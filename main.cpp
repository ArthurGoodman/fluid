#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>

#include <iostream>

int main(int, char **) {
    const char *windowTitle = "Fluid";
    bool isFullscreen = false;
    sf::VideoMode videoMode(600, 600);
    sf::Vector2i windowPos;

    sf::RenderWindow window(videoMode, windowTitle, sf::Style::Default);

    windowPos = window.getPosition();

    sf::Shader shader;
    shader.loadFromFile("shader.vert", "shader.frag");

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

        shader.setUniform("resolution", window.getView().getSize());

        sf::RenderStates states;
        states.shader = &shader;

        sf::RectangleShape rect(window.getView().getSize());

        window.draw(rect, states);

        window.display();
    }

    return 0;
}
