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

#if defined(SFML_SYSTEM_WINDOWS)
# define GL_GLEXT_PROTOTYPES 1
# include <GL/glext.h>
#endif

////////////////////////////////////////////////////////////////////////////////

void drawVBO(GLuint vbo) {
    glBindVertexArray(vbo);
    /*
     * 3*float      vertex
     * 3*float      normal
     * 2*uint16     texcoord
     * 4*uint8      color
     */
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,          32, reinterpret_cast<GLvoid*>( 0));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE,          32, reinterpret_cast<GLvoid*>(12));
    glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_FALSE, 32, reinterpret_cast<GLvoid*>(24));
    glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE,   32, reinterpret_cast<GLvoid*>(28));
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

