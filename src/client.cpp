////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "engine.hpp"

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>

////////////////////////////////////////////////////////////////////////////////

void drawVBO(GLuint vbo) {
    glBindVertexBuffer(GL_VERTEX_ARRAY_BUFFER, vbo);
    glVertexAttrPointer(0);
}

////////////////////////////////////////////////////////////////////////////////

extern "C"
int main(int argc, char **argv) {

    // mine::Engine engine;

    ChunkGenerator chunkGen;
    ChunkCache chunkCache(chunkGen);

    for (size_t i = 0; i < 4098; i++) {
        chunkCache.getChunk(Position(0,0,0));
    }

    Chunk *chunk = chunkCache.getChunk(Position(0,0,0));

    printf("chunk == %p, chunk->getData() == %p\n", chunk, chunk->getData());

    sf::RenderWindow window;

    window.create(sf::VideoMode(960, 540), L"MinEngine Client");

    sf::Clock clock;
    const sf::Time tickLength(sf::microseconds(50000));
    sf::Time tickCount;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed: {
                    window.close();
                    break;
                }

                case sf::Event::Resized: {
                    break;
                }

                case sf::Event::LostFocus:
                case sf::Event::GainedFocus: {
                    break;
                }

                case sf::Event::TextEntered: {
                    break;
                }

                case sf::Event::KeyPressed:
                case sf::Event::KeyReleased: {

                    /// @note for debugging
                    if (event.key.code == sf::Keyboard::Escape) {
                        window.close();
                    }

                    break;
                }

                case sf::Event::MouseWheelMoved:
                case sf::Event::MouseWheelScrolled: {
                    break;
                }

                case sf::Event::MouseButtonPressed:
                case sf::Event::MouseButtonReleased: {
                    break;
                }

                case sf::Event::MouseMoved: {
                    break;
                }

                case sf::Event::MouseEntered:
                case sf::Event::MouseLeft: {
                    break;
                }

                case sf::Event::JoystickButtonPressed:
                case sf::Event::JoystickButtonReleased: {
                    break;
                }

                case sf::Event::JoystickMoved: {
                    break;
                }

                case sf::Event::JoystickConnected:
                case sf::Event::JoystickDisconnected: {
                    break;
                }

                case sf::Event::TouchBegan:
                case sf::Event::TouchMoved:
                case sf::Event::TouchEnded: {
                    break;
                }

                case sf::Event::SensorChanged: {
                    break;
                }

                // silence "enumeration value not handled" warning
                case sf::Event::Count: break;
            }
        }

        tickCount += clock.restart();

        while (tickCount >= tickLength) {
            tickCount -= tickLength;
            //~ engine.tick();

            /*
             * update blocks, entities, etc.
             */
        }

        //~ render(engine)

        /*
         * for each visible chunk:
         *      create vbo
         *      convert blocks to polys
         *      render vbo
         */

        window.display();
    }
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

