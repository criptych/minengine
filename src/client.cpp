////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "engine.hpp"

#define GLEW_STATIC
#include <GL/glew.h>
#include <GL/glu.h>

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>

#include <cstdio>
#include <cmath>

////////////////////////////////////////////////////////////////////////////////

static const double PI = 3.14159265358;

static inline void _GLCheck(const char *file, int line) {
    GLenum errcode;

    while ( (errcode = glGetError()) ) {
        std::fprintf(stderr, "GL Error: %s(%d): %s\n", file, line,
            gluErrorString(errcode)
        );
    }
}

#define GLChecked(f) do{f;_GLCheck(__FILE__, __LINE__);}while(0)
//~ #define GLChecked(f) f

struct Vertex {
    uint8_t r, g, b, a; // color
    uint16_t s, t;      // texcoord
    float u, v, w;      // normal
    float x, y, z;      // vertex
};

GLuint makeVBO(const Vertex *verts, size_t count) {
    GLuint vbo = 0;
    GLChecked(glGenBuffers(1, &vbo));

    if (vbo) {
        GLChecked(glBindBuffer(GL_ARRAY_BUFFER, vbo));
        GLChecked(glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * count, verts, GL_STATIC_DRAW));
    }
    return vbo;
}

void freeVBO(GLuint vbo) {
    GLChecked(glDeleteBuffers(1, &vbo));
}

void drawVBO(GLuint vbo, GLenum mode, GLint first, GLsizei count) {
    GLChecked(glBindBuffer(GL_ARRAY_BUFFER, vbo));

    GLChecked(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE,          sizeof(Vertex), reinterpret_cast<GLvoid*>(offsetof(Vertex, x))));
    GLChecked(glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE,           sizeof(Vertex), reinterpret_cast<GLvoid*>(offsetof(Vertex, u))));
    GLChecked(glVertexAttribPointer(2, 2, GL_UNSIGNED_SHORT, GL_FALSE, sizeof(Vertex), reinterpret_cast<GLvoid*>(offsetof(Vertex, s))));
    GLChecked(glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE,   sizeof(Vertex), reinterpret_cast<GLvoid*>(offsetof(Vertex, r))));

    GLChecked(glEnableVertexAttribArray(0));
    GLChecked(glEnableVertexAttribArray(1));
    GLChecked(glEnableVertexAttribArray(2));
    GLChecked(glEnableVertexAttribArray(3));

    GLChecked(glDrawArrays(mode, first, count));

    GLChecked(glBindBuffer(GL_ARRAY_BUFFER, 0));
}

void perspectiveView(double fov, double aspect, double znear, double zfar) {
    double fh = std::tan(fov*PI/360.0)*znear;
    double fw = fh * aspect;
    glFrustum(-fw, fw, -fh, fh, znear, zfar);
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

    sf::VideoMode videoMode(960, 540);
    sf::String windowTitle(L"MinEngine Client");
    sf::Uint32 windowStyle(sf::Style::Default);
    sf::ContextSettings contextSettings;

    sf::RenderWindow window(videoMode, L"MinEngine Client", windowStyle, contextSettings);

    glewInit();

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    perspectiveView(90.0, 16.0/9.0, 0.1, 100.0);
    glTranslated(0.0, 0.0, -5.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //~ uint8_t r, g, b, a; // color
    //~ uint16_t s, t;      // texcoord
    //~ float u, v, w;      // normal
    //~ float x, y, z;      // vertex
    Vertex cubedata[24] = {
        { 0xff,0xff,0xff,0xff, 0xffff,0x0000, +1.0, 0.0, 0.0, +1.0,+1.0,-1.0 },
        { 0xff,0xff,0xff,0xff, 0xffff,0x0000, +1.0, 0.0, 0.0, +1.0,+1.0,+1.0 },
        { 0xff,0xff,0xff,0xff, 0x0000,0x0000, +1.0, 0.0, 0.0, +1.0,-1.0,+1.0 },
        { 0xff,0xff,0xff,0xff, 0x0000,0x0000, +1.0, 0.0, 0.0, +1.0,-1.0,-1.0 },
        { 0xff,0x00,0xff,0xff, 0x0000,0x0000, -1.0, 0.0, 0.0, -1.0,-1.0,-1.0 },
        { 0xff,0x00,0xff,0xff, 0x0000,0x0000, -1.0, 0.0, 0.0, -1.0,-1.0,+1.0 },
        { 0xff,0x00,0xff,0xff, 0xffff,0x0000, -1.0, 0.0, 0.0, -1.0,+1.0,+1.0 },
        { 0xff,0x00,0xff,0xff, 0xffff,0x0000, -1.0, 0.0, 0.0, -1.0,+1.0,-1.0 },
        { 0xff,0xff,0x00,0xff, 0x0000,0x0000,  0.0,+1.0, 0.0, -1.0,+1.0,-1.0 },
        { 0xff,0xff,0x00,0xff, 0x0000,0x0000,  0.0,+1.0, 0.0, -1.0,+1.0,+1.0 },
        { 0xff,0xff,0x00,0xff, 0xffff,0x0000,  0.0,+1.0, 0.0, +1.0,+1.0,+1.0 },
        { 0xff,0xff,0x00,0xff, 0xffff,0x0000,  0.0,+1.0, 0.0, +1.0,+1.0,-1.0 },
        { 0x00,0xff,0xff,0xff, 0xffff,0x0000,  0.0,-1.0, 0.0, +1.0,-1.0,-1.0 },
        { 0x00,0xff,0xff,0xff, 0xffff,0x0000,  0.0,-1.0, 0.0, +1.0,-1.0,+1.0 },
        { 0x00,0xff,0xff,0xff, 0x0000,0x0000,  0.0,-1.0, 0.0, -1.0,-1.0,+1.0 },
        { 0x00,0xff,0xff,0xff, 0x0000,0x0000,  0.0,-1.0, 0.0, -1.0,-1.0,-1.0 },
        { 0xff,0x00,0x00,0xff, 0xffff,0x0000,  0.0, 0.0,+1.0, +1.0,-1.0,+1.0 },
        { 0xff,0x00,0x00,0xff, 0xffff,0x0000,  0.0, 0.0,+1.0, +1.0,+1.0,+1.0 },
        { 0xff,0x00,0x00,0xff, 0x0000,0x0000,  0.0, 0.0,+1.0, -1.0,+1.0,+1.0 },
        { 0xff,0x00,0x00,0xff, 0x0000,0x0000,  0.0, 0.0,+1.0, -1.0,-1.0,+1.0 },
        { 0x00,0xff,0x00,0xff, 0x0000,0x0000,  0.0, 0.0,-1.0, -1.0,-1.0,-1.0 },
        { 0x00,0xff,0x00,0xff, 0x0000,0x0000,  0.0, 0.0,-1.0, -1.0,+1.0,-1.0 },
        { 0x00,0xff,0x00,0xff, 0xffff,0x0000,  0.0, 0.0,-1.0, +1.0,+1.0,-1.0 },
        { 0x00,0xff,0x00,0xff, 0xffff,0x0000,  0.0, 0.0,-1.0, +1.0,-1.0,-1.0 },
    };

    GLuint cube = makeVBO(cubedata, 24);

    fprintf(stderr, "cube == %#x\n", cube);

    double spin = 0.0, spinSpeed = 60.0; // degrees/second

    GLChecked(glEnable(GL_DEPTH_TEST));
    GLChecked(glDepthMask(GL_TRUE));
    GLChecked(glDepthFunc(GL_LESS));

    GLChecked(glEnable(GL_CULL_FACE));



    sf::Clock clock;
    const sf::Time tickLength(sf::microseconds(5000));
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

            spin += tickLength.asSeconds() * spinSpeed;
        }

        //~ render(engine)

        /*
         * for each visible chunk:
         *      create vbo
         *      convert blocks to polys
         *      render vbo
         */


        glClear(GL_COLOR_BUFFER_BIT);
        glPushMatrix();
        glRotated(std::sin(spin*PI/180.0)*30.0, 1.0,0.0,0.0);
        glRotated(spin, 0.0,1.0,0.0);
        drawVBO(cube, GL_QUADS, 0, 24);
        glPopMatrix();

        window.display();
    }

    freeVBO(cube);
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

