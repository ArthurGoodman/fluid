#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#define GL_HALF_FLOAT 0x140b
#define GL_RGBA32F 0x8814
#define GL_RGBA16F 0x881a

enum ShaderIndex {
    Advect = 0,
    Splat,
    Display,
    Divergence,
    JacobiScalar,
    JacobiVector,
    Gradient,
    Vorticity,
    VorticityForce,
    Buoyancy,
    Boundary,

    ShaderCount
};

const char *shaderFileNames[] = {
    "advect.frag",
    "splat.frag",
    "display.frag",
    "divergence.frag",
    "jacobiscalar.frag",
    "jacobivector.frag",
    "gradient.frag",
    "vorticity.frag",
    "vorticityForce.frag",
    "buoyancy.frag",
    "boundary.frag"};

enum TextureIndex {
    Velocity = 0,
    Density,
    VelocityDivergence,
    VelocityVorticity,
    Pressure,
    Temperature,

    TextureCount
};

static sf::Shader shaders[ShaderCount];
static sf::RenderTexture textures[TextureCount][2];
static int indices[TextureCount];

static int gridWidth, gridHeight;
sf::RectangleShape rect;

static inline int next(int i) {
    return (indices[i] + 1) % 2;
}

static inline void swap(int i) {
    indices[i] = next(i);
}

static inline sf::RenderTexture &write(int i) {
    return textures[i][indices[i]];
}

static inline sf::RenderTexture &read(int i) {
    return textures[i][next(i)];
}

static inline sf::Shader &shader(int i) {
    return shaders[i];
}

static inline const char *shaderFileName(int i) {
    return shaderFileNames[i];
}

static void render(int shaderIndex, int textureIndex) {
    float scale;

    switch (textureIndex) {
    case Velocity:
        scale = -1;
        break;

    case Pressure:
        scale = 1;
        break;

    default:
        scale = 0;
        break;
    }

    sf::RenderStates states;

    states.shader = &shader(shaderIndex);

    write(textureIndex).draw(rect, states);
    swap(textureIndex);

    states.shader = &shader(Boundary);

    shader(Boundary).setUniform("read", read(textureIndex).getTexture());
    shader(Boundary).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
    shader(Boundary).setUniform("scale", scale);

    write(textureIndex).draw(rect, states);
    swap(textureIndex);
}

int main(int, char **) {
    const char *windowTitle = "Fluid";

    const float windowDownscale = 1.5;
    const float gridDownscale = 2;

    const int windowWidth = sf::VideoMode::getDesktopMode().width / windowDownscale;
    const int windowHeight = sf::VideoMode::getDesktopMode().height / windowDownscale;

    gridWidth = windowWidth / gridDownscale;
    gridHeight = windowHeight / gridDownscale;

    rect = sf::RectangleShape(sf::Vector2f(gridWidth, gridHeight));

    const float gridScale = 1;
    const float timestep = 1;

    const sf::Color zeroColor;

    const int numJacobiIterations = 50;

    const float velocityScale[] = {
        0.5, 0.0, 0.0, 0.5,
        0.0, 0.5, 0.0, 0.5,
        0.0, 0.0, 0.0, 0.5,
        0.0, 0.0, 0.0, 1.0};

    const float displayScale[] = {
        1.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0,
        1.0, 0.0, 0.0, 0.0,
        0.0, 0.0, 0.0, 1.0};

    const float pressureScale[] = {
        0.5, 0.0, 0.0, 0.5,
        0.5, 0.0, 0.0, 0.5,
        0.5, 0.0, 0.0, 0.5,
        0.0, 0.0, 0.0, 1.0};

    bool pause = false;

    sf::Vector2i mousePos;
    sf::Vector2f lastPos;

    bool isFullscreen = false;
    sf::VideoMode videoMode(windowWidth, windowHeight);
    sf::Vector2i windowPos;

    sf::RenderWindow window(videoMode, windowTitle, sf::Style::Default);
    windowPos = window.getPosition();

    for (int i = 0; i < ShaderCount; i++)
        shader(i).loadFromFile(shaderFileName(i), sf::Shader::Fragment);

    for (int i = 0; i < TextureCount; i++) {
        write(i).create(gridWidth, gridHeight);
        read(i).create(gridWidth, gridHeight);

        GLint textureBinding;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBinding);

        static bool useHalfFloats = true;

        GLint internalFormat = useHalfFloats ? GL_RGBA16F : GL_RGBA32F;
        GLenum type = useHalfFloats ? GL_HALF_FLOAT : GL_FLOAT;

        sf::Texture::bind(&write(i).getTexture());
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, gridWidth, gridHeight, 0, GL_RGBA, type, 0);

        sf::Texture::bind(&read(i).getTexture());
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, gridWidth, gridHeight, 0, GL_RGBA, type, 0);

        glBindTexture(GL_TEXTURE_2D, textureBinding);

        write(i).clear(zeroColor);
        read(i).clear(zeroColor);
    }

    while (window.isOpen()) {
        sf::Event event;

        while (window.pollEvent(event))
            switch (event.type) {
            case sf::Event::Closed:
                window.close();
                break;

            case sf::Event::KeyPressed:
                switch (event.key.code) {
                case sf::Keyboard::Escape:
                    if (isFullscreen) {
                        window.create(videoMode, windowTitle, sf::Style::Default);
                        window.setPosition(windowPos);
                        isFullscreen = false;
                    } else
                        window.close();

                    break;

                case sf::Keyboard::F11:
                    if (isFullscreen) {
                        window.create(videoMode, windowTitle, sf::Style::Default);
                        window.setPosition(windowPos);
                    } else {
                        windowPos = window.getPosition();
                        window.create(sf::VideoMode::getDesktopMode(), windowTitle, sf::Style::Fullscreen);
                    }

                    isFullscreen = !isFullscreen;
                    break;

                case sf::Keyboard::R:
                    for (int i = 0; i < TextureCount; i++) {
                        write(i).clear(zeroColor);
                        read(i).clear(zeroColor);
                    }

                    pause = false;
                    break;

                case sf::Keyboard::Space:
                    pause = !pause;
                    break;

                default:
                    break;
                }

                break;

            case sf::Event::MouseButtonPressed:
                mousePos = sf::Vector2i(event.mouseButton.x, event.mouseButton.y);
                break;

            case sf::Event::MouseMoved:
                mousePos = sf::Vector2i(event.mouseMove.x, event.mouseMove.y);
                break;

            case sf::Event::Resized: {
                videoMode = sf::VideoMode(event.size.width, event.size.height);
                sf::Vector2i pos = window.getPosition();
                window.create(videoMode, windowTitle, sf::Style::Default);
                window.setPosition(pos);
                break;
            }

            default:
                break;
            }

        if (!pause) {
            ////////////////////////////////////////////////////////////////////////////////
            /// Permanent source of density and temperature
            ////////////////////////////////////////////////////////////////////////////////

            sf::CircleShape circle(5);
            circle.setOrigin(circle.getRadius(), circle.getRadius());
            circle.setPosition(gridWidth / 2, gridHeight / 2);

            read(Density).draw(circle);
            read(Temperature).draw(circle);

            ////////////////////////////////////////////////////////////////////////////////
            /// Advection
            ////////////////////////////////////////////////////////////////////////////////

            shader(Advect).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
            shader(Advect).setUniform("gridScale", gridScale);
            shader(Advect).setUniform("timestep", timestep);

            shader(Advect).setUniform("velocity", read(Velocity).getTexture());
            shader(Advect).setUniform("advected", read(Velocity).getTexture());
            shader(Advect).setUniform("dissipation", 1.0f);

            render(Advect, Velocity);

            shader(Advect).setUniform("velocity", read(Velocity).getTexture());
            shader(Advect).setUniform("advected", read(Density).getTexture());
            shader(Advect).setUniform("dissipation", 0.999f);

            render(Advect, Density);

            shader(Advect).setUniform("velocity", read(Velocity).getTexture());
            shader(Advect).setUniform("advected", read(Temperature).getTexture());
            shader(Advect).setUniform("dissipation", 0.999f);

            render(Advect, Temperature);

            ////////////////////////////////////////////////////////////////////////////////
            /// Adding forces and densities from mouse movement
            ////////////////////////////////////////////////////////////////////////////////

            sf::Vector2f pos(mousePos);

            sf::Vector2f drag = pos - lastPos;
            drag.x = std::max(-1.0f, std::min(drag.x, 1.0f));
            drag.y = std::max(-1.0f, std::min(drag.y, 1.0f));

            lastPos = pos;

            pos.x = (float)pos.x / window.getSize().x * gridWidth;
            pos.y = (float)(window.getSize().y - pos.y) / window.getSize().y * gridHeight;

            if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
                shader(Splat).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
                shader(Splat).setUniform("read", read(Velocity).getTexture());
                shader(Splat).setUniform("color", sf::Glsl::Vec3(drag.x, -drag.y, 0));
                shader(Splat).setUniform("point", sf::Glsl::Vec2(pos));
                shader(Splat).setUniform("radius", 0.01f);

                render(Splat, Velocity);
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
                shader(Splat).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
                shader(Splat).setUniform("point", sf::Glsl::Vec2(pos));
                shader(Splat).setUniform("color", sf::Glsl::Vec3(0.8, 0, 0));
                shader(Splat).setUniform("radius", 0.01f);

                shader(Splat).setUniform("read", read(Density).getTexture());
                shader(Splat).setUniform("color", sf::Glsl::Vec3(0.8, 0, 0));

                render(Splat, Density);

                shader(Splat).setUniform("read", read(Temperature).getTexture());
                shader(Splat).setUniform("color", sf::Glsl::Vec3(0.5, 0, 0));

                render(Splat, Temperature);
            }

            ////////////////////////////////////////////////////////////////////////////////
            /// Buoyancy
            ////////////////////////////////////////////////////////////////////////////////

            shader(Buoyancy).setUniform("read", read(Velocity).getTexture());
            shader(Buoyancy).setUniform("density", read(Density).getTexture());
            shader(Buoyancy).setUniform("temperature", read(Temperature).getTexture());
            shader(Buoyancy).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
            shader(Buoyancy).setUniform("timestep", timestep);
            shader(Buoyancy).setUniform("k", 0.0005f);
            shader(Buoyancy).setUniform("buoyancyFactor", 0.01f);

            render(Buoyancy, Velocity);

            ////////////////////////////////////////////////////////////////////////////////
            /// Vorticity
            ////////////////////////////////////////////////////////////////////////////////

            static const bool useVorticity = true;

            if (useVorticity) {
                shader(Vorticity).setUniform("velocity", read(Velocity).getTexture());
                shader(Vorticity).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
                shader(Vorticity).setUniform("gridScale", gridScale);

                render(Vorticity, VelocityVorticity);

                static const float curl = 0.025;

                shader(VorticityForce).setUniform("velocity", read(Velocity).getTexture());
                shader(VorticityForce).setUniform("vorticity", read(VelocityVorticity).getTexture());
                shader(VorticityForce).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
                shader(VorticityForce).setUniform("gridScale", gridScale);
                shader(VorticityForce).setUniform("timestep", timestep);
                shader(VorticityForce).setUniform("epsilon", 2.4414e-4f);
                shader(VorticityForce).setUniform("curl", sf::Glsl::Vec2(curl * gridScale, curl * gridScale));

                render(VorticityForce, Velocity);
            }

            ////////////////////////////////////////////////////////////////////////////////
            /// Viscosity
            ////////////////////////////////////////////////////////////////////////////////

            static const bool useViscosity = false;

            if (useViscosity) {
                static const float viscosity = 0.05;

                float alpha = gridScale * gridScale / (viscosity * timestep);

                shader(JacobiVector).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
                shader(JacobiVector).setUniform("alpha", alpha);
                shader(JacobiVector).setUniform("beta", 4.0f + alpha);

                for (int i = 0; i < numJacobiIterations; i++) {
                    shader(JacobiVector).setUniform("x", read(Velocity).getTexture());
                    shader(JacobiVector).setUniform("b", read(Velocity).getTexture());

                    render(JacobiVector, Velocity);
                }
            }

            ////////////////////////////////////////////////////////////////////////////////
            /// Projection operator
            ////////////////////////////////////////////////////////////////////////////////

            shader(Divergence).setUniform("velocity", read(Velocity).getTexture());
            shader(Divergence).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
            shader(Divergence).setUniform("gridScale", gridScale);

            render(Divergence, VelocityDivergence);

            write(Pressure).clear(zeroColor);
            swap(Pressure);

            shader(JacobiScalar).setUniform("b", read(VelocityDivergence).getTexture());
            shader(JacobiScalar).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
            shader(JacobiScalar).setUniform("alpha", -gridScale * gridScale);
            shader(JacobiScalar).setUniform("beta", 4.0f);

            for (int i = 0; i < numJacobiIterations; i++) {
                shader(JacobiScalar).setUniform("x", read(Pressure).getTexture());

                render(JacobiScalar, Pressure);
            }

            shader(Gradient).setUniform("p", read(Pressure).getTexture());
            shader(Gradient).setUniform("w", read(Velocity).getTexture());
            shader(Gradient).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
            shader(Gradient).setUniform("gridScale", gridScale);

            render(Gradient, Velocity);
        }

        ////////////////////////////////////////////////////////////////////////////////
        /// Drawing on the window
        ////////////////////////////////////////////////////////////////////////////////

        sf::RectangleShape windowRect(window.getView().getSize());

        sf::RenderStates states;
        states.shader = &shader(Display);

        shader(Display).setUniform("resolution", sf::Glsl::Vec2(window.getSize()));

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::V)) {
            shader(Display).setUniform("read", read(Velocity).getTexture());
            shader(Display).setUniform("scale", sf::Glsl::Mat4(velocityScale));
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::P)) {
            shader(Display).setUniform("read", read(Pressure).getTexture());
            shader(Display).setUniform("scale", sf::Glsl::Mat4(pressureScale));
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            shader(Display).setUniform("read", read(VelocityDivergence).getTexture());
            shader(Display).setUniform("scale", sf::Glsl::Mat4(pressureScale));
        } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::T)) {
            shader(Display).setUniform("read", read(Temperature).getTexture());
            shader(Display).setUniform("scale", sf::Glsl::Mat4(pressureScale));
        } else {
            shader(Display).setUniform("read", read(Density).getTexture());
            shader(Display).setUniform("scale", sf::Glsl::Mat4(displayScale));
        }

        window.draw(windowRect, states);

        window.display();
    }

    return 0;
}
