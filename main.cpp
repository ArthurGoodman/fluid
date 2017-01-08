#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <iostream>

enum TextureIndex {
    Texture0 = 0,

    TextureCount
};

static sf::RenderTexture textures[TextureCount][2];
static int indices[TextureCount];

static void swapTextures(TextureIndex i) {
    indices[i] = (indices[i] + 1) % 2;
}

static sf::RenderTexture &currentTexture(TextureIndex i) {
    return textures[i][indices[i]];
}

int main(int, char **) {
    const char *windowTitle = "Fluid";
    bool isFullscreen = false;
    sf::VideoMode videoMode(1366, 768);
    sf::Vector2i windowPos;

    sf::RenderWindow window(videoMode, windowTitle, sf::Style::Default);
    windowPos = window.getPosition();

    sf::Shader shader;
    shader.loadFromFile("shader.vert", "shader.frag");

    textures[Texture0][0].create(videoMode.width / 2, videoMode.height / 2);
    textures[Texture0][1].create(videoMode.width / 2, videoMode.height / 2);

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

        shader.setUniform("resolution", currentTexture(Texture0).getView().getSize());

        sf::RenderStates states;
        states.shader = &shader;

        sf::RectangleShape rect(window.getView().getSize());

        currentTexture(Texture0).draw(rect, states);

        sf::Sprite sprite(currentTexture(Texture0).getTexture());
        sprite.scale((float)window.getSize().x / sprite.getTextureRect().width, (float)window.getSize().y / sprite.getTextureRect().height);

        window.draw(sprite);

        window.display();
    }

    return 0;
}
