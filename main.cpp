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
    Jacobiscalar,
    Jacobivector,
    Gradient,
    Vorticity,
    VorticityForce,
    Buoyancy,

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
    "buoyancy.frag"};

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

static int next(int i) {
    return (indices[i] + 1) % 2;
}

static void swap(int i) {
    indices[i] = next(i);
}

static sf::RenderTexture &write(int i) {
    return textures[i][indices[i]];
}

static sf::RenderTexture &read(int i) {
    return textures[i][next(i)];
}

static sf::Shader &shader(int i) {
    return shaders[i];
}

static const char *shaderFileName(int i) {
    return shaderFileNames[i];
}

int main(int, char **) {
    const char *windowTitle = "Fluid";

    const float windowDownscale = 1.5;
    const float gridDownscale = 2;

    const int windowWidth = sf::VideoMode::getDesktopMode().width / windowDownscale;
    const int windowHeight = sf::VideoMode::getDesktopMode().height / windowDownscale;

    const int gridWidth = windowWidth / gridDownscale;
    const int gridHeight = windowHeight / gridDownscale;

    const float gridScale = 1.0f;
    const float timestep = 1.0f;

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

        sf::Texture::bind(&write(i).getTexture());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gridWidth, gridHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);

        sf::Texture::bind(&read(i).getTexture());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gridWidth, gridHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);

        glBindTexture(GL_TEXTURE_2D, textureBinding);

        write(i).clear(zeroColor);
        read(i).clear(zeroColor);
    }

    sf::RenderStates states;

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
            sf::RectangleShape rect(sf::Vector2f(gridWidth, gridHeight));

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

            states.shader = &shader(Advect);

            shader(Advect).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
            shader(Advect).setUniform("gridScale", gridScale);
            shader(Advect).setUniform("timestep", timestep);

            shader(Advect).setUniform("velocity", read(Velocity).getTexture());
            shader(Advect).setUniform("advected", read(Velocity).getTexture());
            shader(Advect).setUniform("dissipation", 1.0f);

            write(Velocity).draw(rect, states);
            swap(Velocity);

            shader(Advect).setUniform("velocity", read(Velocity).getTexture());
            shader(Advect).setUniform("advected", read(Density).getTexture());
            shader(Advect).setUniform("dissipation", 0.999f);

            write(Density).draw(rect, states);
            swap(Density);

            shader(Advect).setUniform("velocity", read(Velocity).getTexture());
            shader(Advect).setUniform("advected", read(Temperature).getTexture());
            shader(Advect).setUniform("dissipation", 0.999f);

            write(Temperature).draw(rect, states);
            swap(Temperature);

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
                states.shader = &shader(Splat);

                shader(Splat).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
                shader(Splat).setUniform("read", read(Velocity).getTexture());
                shader(Splat).setUniform("color", sf::Glsl::Vec3(drag.x, -drag.y, 0));
                shader(Splat).setUniform("point", sf::Glsl::Vec2(pos));
                shader(Splat).setUniform("radius", 0.01f);

                write(Velocity).draw(rect, states);
                swap(Velocity);
            }

            if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
                states.shader = &shader(Splat);

                shader(Splat).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
                shader(Splat).setUniform("point", sf::Glsl::Vec2(pos));
                shader(Splat).setUniform("color", sf::Glsl::Vec3(0.8, 0, 0));
                shader(Splat).setUniform("radius", 0.01f);

                shader(Splat).setUniform("read", read(Density).getTexture());

                write(Density).draw(rect, states);
                swap(Density);

                shader(Splat).setUniform("read", read(Temperature).getTexture());

                write(Temperature).draw(rect, states);
                swap(Temperature);
            }

            ////////////////////////////////////////////////////////////////////////////////
            /// Buoyancy
            ////////////////////////////////////////////////////////////////////////////////

            states.shader = &shader(Buoyancy);

            shader(Buoyancy).setUniform("read", read(Velocity).getTexture());
            shader(Buoyancy).setUniform("density", read(Density).getTexture());
            shader(Buoyancy).setUniform("temperature", read(Temperature).getTexture());
            shader(Buoyancy).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
            shader(Buoyancy).setUniform("timestep", timestep);
            shader(Buoyancy).setUniform("k", 0.0005f);
            shader(Buoyancy).setUniform("buoyancyFactor", 0.01f);

            write(Velocity).draw(rect, states);
            swap(Velocity);

            ////////////////////////////////////////////////////////////////////////////////
            /// Vorticity
            ////////////////////////////////////////////////////////////////////////////////

            static const bool useVorticity = true;

            if (useVorticity) {
                states.shader = &shader(Vorticity);

                shader(Vorticity).setUniform("velocity", read(Velocity).getTexture());
                shader(Vorticity).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
                shader(Vorticity).setUniform("gridScale", gridScale);

                write(VelocityVorticity).draw(rect, states);
                swap(VelocityVorticity);

                states.shader = &shader(VorticityForce);

                static const float curl = 0.025f;

                shader(VorticityForce).setUniform("velocity", read(Velocity).getTexture());
                shader(VorticityForce).setUniform("vorticity", read(VelocityVorticity).getTexture());
                shader(VorticityForce).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
                shader(VorticityForce).setUniform("gridScale", gridScale);
                shader(VorticityForce).setUniform("timestep", timestep);
                shader(VorticityForce).setUniform("epsilon", 2.4414e-4f);
                shader(VorticityForce).setUniform("curl", sf::Glsl::Vec2(curl * gridScale, curl * gridScale));

                write(Velocity).draw(rect, states);
                swap(Velocity);
            }

            ////////////////////////////////////////////////////////////////////////////////
            /// Viscosity
            ////////////////////////////////////////////////////////////////////////////////

            static const bool useViscosity = false;

            if (useViscosity) {
                states.shader = &shader(Jacobivector);

                static const float viscosity = 0.05f;

                float alpha = gridScale * gridScale / (viscosity * timestep);

                shader(Jacobivector).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
                shader(Jacobivector).setUniform("alpha", alpha);
                shader(Jacobivector).setUniform("beta", 4.0f + alpha);

                for (int i = 0; i < numJacobiIterations; i++) {
                    shader(Jacobivector).setUniform("x", read(Velocity).getTexture());
                    shader(Jacobivector).setUniform("b", read(Velocity).getTexture());

                    write(Velocity).draw(rect, states);
                    swap(Velocity);
                }
            }

            ////////////////////////////////////////////////////////////////////////////////
            /// Projection operator
            ////////////////////////////////////////////////////////////////////////////////

            states.shader = &shader(Divergence);

            shader(Divergence).setUniform("velocity", read(Velocity).getTexture());
            shader(Divergence).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
            shader(Divergence).setUniform("gridScale", gridScale);

            write(VelocityDivergence).draw(rect, states);
            swap(VelocityDivergence);

            write(Pressure).clear(zeroColor);
            swap(Pressure);

            states.shader = &shader(Jacobiscalar);

            shader(Jacobiscalar).setUniform("b", read(VelocityDivergence).getTexture());
            shader(Jacobiscalar).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
            shader(Jacobiscalar).setUniform("alpha", -gridScale * gridScale);
            shader(Jacobiscalar).setUniform("beta", 4.0f);

            for (int i = 0; i < numJacobiIterations; i++) {
                shader(Jacobiscalar).setUniform("x", read(Pressure).getTexture());

                write(Pressure).draw(rect, states);
                swap(Pressure);
            }

            states.shader = &shader(Gradient);

            shader(Gradient).setUniform("p", read(Pressure).getTexture());
            shader(Gradient).setUniform("w", read(Velocity).getTexture());
            shader(Gradient).setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
            shader(Gradient).setUniform("gridScale", gridScale);

            write(Velocity).draw(rect, states);
            swap(Velocity);
        }

        ////////////////////////////////////////////////////////////////////////////////
        /// Drawing on the window
        ////////////////////////////////////////////////////////////////////////////////

        sf::RectangleShape windowRect(window.getView().getSize());

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
