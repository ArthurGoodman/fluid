#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

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

    for (int i = 0; i < TextureCount; i++) {
        textures[i][0].create(gridWidth, gridHeight);
        textures[i][1].create(gridWidth, gridHeight);
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
                        textures[i][0].clear();
                        textures[i][1].clear();
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

        sf::RenderStates states;
        states.shader = &advect;

        advect.setUniform("velocity", read(Velocity).getTexture());
        advect.setUniform("advected", read(Velocity).getTexture());
        advect.setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
        advect.setUniform("gridScale", 1.0f);
        advect.setUniform("timestep", 1.0f);
        advect.setUniform("dissipation", 1.0f);

        write(Velocity).draw(rect, states);
        swap(Velocity);

        advect.setUniform("velocity", read(Velocity).getTexture());
        advect.setUniform("advected", read(Density).getTexture());
        advect.setUniform("gridSize", sf::Glsl::Vec2(gridWidth, gridHeight));
        advect.setUniform("gridScale", 1.0f);
        advect.setUniform("timestep", 1.0f);
        advect.setUniform("dissipation", 0.998f);

        write(Density).draw(rect, states);
        swap(Density);

        sf::Vector2f pos(mousePos);
        sf::Vector2f drag = pos - lastPos;
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

        states.shader = &display;

        display.setUniform("read", read(Velocity).getTexture());
        display.setUniform("bias", sf::Glsl::Vec3(0, 0, 0));
        display.setUniform("scale", sf::Glsl::Vec3(1, 1, 1));
        display.setUniform("resolution", sf::Glsl::Vec2(window.getSize()));

        window.draw(windowRect, states);

        window.display();
    }

    return 0;
}
