#include <SFML/Graphics.hpp>

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

static void swapTextures(TextureIndex i) {
    indices[i] = (indices[i] + 1) % 2;
}

int main(int, char **) {
    const char *windowTitle = "Fluid";
    const int windowWidth = 1366;
    const int windowHeight = 768;

    const int gridWidth = windowWidth / 2;
    const int gridHeight = windowHeight / 2;

    bool isFullscreen = false;
    sf::VideoMode videoMode(windowWidth, windowHeight);
    sf::Vector2i windowPos;

    sf::RenderWindow window(videoMode, windowTitle, sf::Style::Default);
    windowPos = window.getPosition();

    sf::Shader shader;
    shader.loadFromFile("shader.vert", "shader.frag");

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

                default:
                    break;
                }
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

        shader.setUniform("resolution", sf::Glsl::Vec2(gridWidth, gridHeight));

        sf::RenderStates states;
        states.shader = &shader;

        sf::RectangleShape windowRect(window.getView().getSize());

        textures[0][0].draw(windowRect, states);

        sf::Sprite sprite(textures[0][0].getTexture());
        sprite.scale((float)window.getSize().x / gridWidth, (float)window.getSize().y / gridHeight);

        window.draw(sprite);

        window.display();
    }

    return 0;
}
