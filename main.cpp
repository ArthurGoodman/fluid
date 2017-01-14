#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#define GL_HALF_FLOAT 0x140b
#define GL_RGBA32F 0x8814
#define GL_RGBA16F 0x881a

enum TextureIndex {
    Velocity = 0,
    Density,
    VelocityDivergence,
    VelocityVorticity,
    Pressure,

    TextureCount
};

static sf::RenderTexture textures[TextureCount][2];
static int indices[TextureCount];

static int next(TextureIndex i) {
    return (indices[i] + 1) % 2;
}

static void swap(TextureIndex i) {
    indices[i] = next(i);
}

static sf::RenderTexture &write(TextureIndex i) {
    return textures[i][indices[i]];
}

static sf::RenderTexture &read(TextureIndex i) {
    return textures[i][next(i)];
}

int main(int, char **) {
    const char *windowTitle = "Fluid";
    const int windowWidth = 1366;
    const int windowHeight = 768;

    const int gridWidth = windowWidth / 2;
    const int gridHeight = windowHeight / 2;

    const float gridScale = 1.0f;
    const float timestep = 1.0f;

    const sf::Color zeroColor;

    const int numJacobiIterations = 50;

    const float velocityScale[] = {0.5, 0.0, 0.0, 0.5,
                                   0.0, 0.5, 0.0, 0.5,
                                   0.0, 0.0, 0.0, 0.5,
                                   0.0, 0.0, 0.0, 1.0};

    const float displayScale[] = {1.0, 0.0, 0.0, 0.0,
                                  1.0, 0.0, 0.0, 0.0,
                                  1.0, 0.0, 0.0, 0.0,
                                  0.0, 0.0, 0.0, 1.0};

    sf::Vector2i mousePos;
    sf::Vector2f lastPos;

    bool isFullscreen = false;
    sf::VideoMode videoMode(windowWidth, windowHeight);
    sf::Vector2i windowPos;

    sf::RenderWindow window(videoMode, windowTitle, sf::Style::Default);
    windowPos = window.getPosition();

    sf::Shader advect;
    advect.loadFromFile("advect.frag", sf::Shader::Fragment);

    sf::Shader splat;
    splat.loadFromFile("splat.frag", sf::Shader::Fragment);

    sf::Shader display;
    display.loadFromFile("display.frag", sf::Shader::Fragment);

    sf::Shader divergence;
    divergence.loadFromFile("divergence.frag", sf::Shader::Fragment);

    sf::Shader jacobiscalar;
    jacobiscalar.loadFromFile("jacobiscalar.frag", sf::Shader::Fragment);

    sf::Shader jacobivector;
    jacobivector.loadFromFile("jacobivector.frag", sf::Shader::Fragment);

    sf::Shader gradient;
    gradient.loadFromFile("gradient.frag", sf::Shader::Fragment);

    sf::Shader vorticity;
    vorticity.loadFromFile("vorticity.frag", sf::Shader::Fragment);

    sf::Shader vorticityForce;
    vorticityForce.loadFromFile("vorticityForce.frag", sf::Shader::Fragment);

    for (int i = 0; i < TextureCount; i++) {
        textures[i][0].create(gridWidth, gridHeight);
        textures[i][1].create(gridWidth, gridHeight);

        GLint textureBinding;
        glGetIntegerv(GL_TEXTURE_BINDING_2D, &textureBinding);

        sf::Texture::bind(&textures[i][0].getTexture());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gridWidth, gridHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);

        sf::Texture::bind(&textures[i][1].getTexture());
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, gridWidth, gridHeight, 0, GL_RGBA, GL_HALF_FLOAT, 0);

        glBindTexture(GL_TEXTURE_2D, textureBinding);

        textures[i][0].clear(zeroColor);
        textures[i][1].clear(zeroColor);
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
                        textures[i][0].clear(zeroColor);
                        textures[i][1].clear(zeroColor);
                    }

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

        sf::RectangleShape rect(sf::Vector2f(gridWidth, gridHeight));
        sf::RectangleShape windowRect(window.getView().getSize());

        sf::CircleShape circle(10);
        circle.setOrigin(circle.getRadius(), circle.getRadius());
        circle.setPosition(gridWidth / 2, gridHeight / 2);

        read(Density).draw(circle);

        sf::RenderStates states;
        states.shader = &advect;

        advect.setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
        advect.setUniform("gridScale", gridScale);
        advect.setUniform("timestep", timestep);

        advect.setUniform("velocity", read(Velocity).getTexture());
        advect.setUniform("advected", read(Velocity).getTexture());
        advect.setUniform("dissipation", 1.0f);

        write(Velocity).draw(rect, states);
        swap(Velocity);

        advect.setUniform("velocity", read(Velocity).getTexture());
        advect.setUniform("advected", read(Density).getTexture());
        advect.setUniform("dissipation", 0.998f);

        write(Density).draw(rect, states);
        swap(Density);

        sf::Vector2f pos(mousePos);

        sf::Vector2f drag = pos - lastPos;
        drag.x = std::max(-1.0f, std::min(drag.x, 1.0f));
        drag.y = std::max(-1.0f, std::min(drag.y, 1.0f));

        lastPos = pos;

        pos.x = (float)pos.x / window.getSize().x * gridWidth;
        pos.y = (float)(window.getSize().y - pos.y) / window.getSize().y * gridHeight;

        if (sf::Mouse::isButtonPressed(sf::Mouse::Left)) {
            states.shader = &splat;

            splat.setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
            splat.setUniform("read", read(Velocity).getTexture());
            splat.setUniform("color", sf::Glsl::Vec3(drag.x, -drag.y, 0));
            splat.setUniform("point", sf::Glsl::Vec2(pos));
            splat.setUniform("radius", 0.01f);

            write(Velocity).draw(rect, states);
            swap(Velocity);
        }

        if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
            states.shader = &splat;

            splat.setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
            splat.setUniform("read", read(Density).getTexture());
            splat.setUniform("color", sf::Glsl::Vec3(0.8, 0, 0));
            splat.setUniform("point", sf::Glsl::Vec2(pos));
            splat.setUniform("radius", 0.01f);

            write(Density).draw(rect, states);
            swap(Density);
        }

        static const bool useVorticity = true;

        if (useVorticity) {
            states.shader = &vorticity;

            vorticity.setUniform("velocity", read(Velocity).getTexture());
            vorticity.setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
            vorticity.setUniform("gridScale", 1.0f);

            write(VelocityVorticity).draw(rect, states);
            swap(VelocityVorticity);

            states.shader = &vorticityForce;

            static const float curl = 0.05f;

            vorticityForce.setUniform("velocity", read(Velocity).getTexture());
            vorticityForce.setUniform("vorticity", read(VelocityVorticity).getTexture());
            vorticityForce.setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
            vorticityForce.setUniform("gridScale", gridScale);
            vorticityForce.setUniform("timestep", timestep);
            vorticityForce.setUniform("epsilon", 2.4414e-4f);
            vorticityForce.setUniform("curl", sf::Glsl::Vec2(curl * gridScale, curl * gridScale));

            write(Velocity).draw(rect, states);
            swap(Velocity);
        }

        static const bool useViscosity = false;

        if (useViscosity) {
            states.shader = &jacobivector;

            static const float viscosity = 0.05f;

            float alpha = gridScale * gridScale / (viscosity * timestep);

            jacobivector.setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
            jacobivector.setUniform("alpha", alpha);
            jacobivector.setUniform("beta", 4.0f + alpha);

            for (int i = 0; i < numJacobiIterations; i++) {
                jacobivector.setUniform("x", read(Velocity).getTexture());
                jacobivector.setUniform("b", read(Velocity).getTexture());

                write(Velocity).draw(rect, states);
                swap(Velocity);
            }
        }

        states.shader = &divergence;

        divergence.setUniform("velocity", read(Velocity).getTexture());
        divergence.setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
        divergence.setUniform("gridScale", gridScale);

        write(VelocityDivergence).draw(rect, states);
        swap(VelocityDivergence);

        write(Pressure).clear(zeroColor);
        swap(Pressure);

        states.shader = &jacobiscalar;

        jacobiscalar.setUniform("b", read(VelocityDivergence).getTexture());
        jacobiscalar.setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
        jacobiscalar.setUniform("alpha", -gridScale * gridScale);
        jacobiscalar.setUniform("beta", 4.0f);

        for (int i = 0; i < numJacobiIterations; i++) {
            jacobiscalar.setUniform("x", read(Pressure).getTexture());

            write(Pressure).draw(rect, states);
            swap(Pressure);
        }

        states.shader = &gradient;

        gradient.setUniform("p", read(Pressure).getTexture());
        gradient.setUniform("w", read(Velocity).getTexture());
        gradient.setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
        gradient.setUniform("gridScale", gridScale);

        write(Velocity).draw(rect, states);
        swap(Velocity);

        states.shader = &display;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::V)) {
            display.setUniform("read", read(Velocity).getTexture());
            display.setUniform("scale", sf::Glsl::Mat4(velocityScale));
        } else {
            display.setUniform("read", read(Density).getTexture());
            display.setUniform("scale", sf::Glsl::Mat4(displayScale));
        }

        display.setUniform("resolution", sf::Glsl::Vec2(window.getSize()));

        window.draw(windowRect, states);

        window.display();
    }

    return 0;
}
