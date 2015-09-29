////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "engine/engine.hpp"

#include <GL/glew.h>

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>

#include <cstdio>
#include <cmath>

#include "GLCheck.hpp"

////////////////////////////////////////////////////////////////////////////////

#include "Transform3D.hpp"
#include "Transformable3D.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "ClientModel.hpp"

////////////////////////////////////////////////////////////////////////////////

class GameWindow : protected sf::Window {
    bool mFullscreen;
    bool mMouseLocked;
    bool mAllowQuit;
    bool mPaused;

    sf::VideoMode mDesktopMode;
    sf::VideoMode mWindowMode;
    sf::String mWindowTitle;
    sf::Uint32 mWindowStyle;
    sf::Vector2i mWindowCenter;

    sf::Time mTickLength;
    sf::Time mPlayTime;
    unsigned int mMaxTicksPerFrame;

    Camera mCamera;
    Shader mBlockShader;

    sf::Vector3f mLightPos;
    sf::Vector2f mLookDir;
    float mSpinAngle;
    float mSpinSpeed; // degrees/second

    Model mCubeModel;
    ClientModel mCube;

    Model mBallModel;
    ClientModel mBall;


public:
    GameWindow();

    void run();
    void quit();

protected:
    void init();
    void quit(bool internal);

    void handleEvents();
    void handleInput(const sf::Time &delta);

    void handleEvent(const sf::Event &event);
    void update(const sf::Time &delta);
    void render();

    void setMousePosition(const sf::Vector2i &position);
    sf::Vector2i getMousePosition() const;
};

////////////////////////////////////////////////////////////////////////////////

GameWindow::GameWindow(
): mFullscreen(
    false
), mMouseLocked(
    true
), mAllowQuit(
    true
), mPaused(
    false
), mTickLength(
    sf::microseconds(20000) // 50000
), mMaxTicksPerFrame(
    5
), mLightPos(
    1, 5, 5
), mSpinAngle(
), mSpinSpeed(
    45
) {
}

void GameWindow::run() {
    sf::Clock clock;
    sf::Time tickCount;

    init();

    while (isOpen()) {
        handleEvents();

        sf::Time delta = clock.restart();

        handleInput(delta);

        tickCount += delta;

        unsigned int frameTicks = mMaxTicksPerFrame;

        while (tickCount >= mTickLength) {
            tickCount -= mTickLength;

            if (frameTicks > 0) {
                update(mTickLength);
                frameTicks -= 1;
            }
        }

        render();

        display();
    }

    quit(true);
}

void GameWindow::quit() {
    quit(false);
}

void GameWindow::init() {
    sf::VideoMode videoMode(960, 540);
    sf::String windowTitle(L"MinEngine Client");
    sf::Uint32 windowStyle(sf::Style::Default);
    sf::ContextSettings contextSettings(24,8, 0, 3,3);

    mDesktopMode = sf::VideoMode::getDesktopMode();

    std::fprintf(stderr, "Desktop mode: %dx%d %dbpp\n",
                 mDesktopMode.width, mDesktopMode.height,
                 mDesktopMode.bitsPerPixel);

    create(videoMode, windowTitle, windowStyle, contextSettings);

    mWindowCenter = sf::Vector2i(getSize()) / 2;

    glewInit();

    GLChecked(glEnable(GL_DEPTH_TEST));
    GLChecked(glDepthFunc(GL_LESS));
    GLChecked(glEnable(GL_CULL_FACE));
    GLChecked(glClearColor(0.200,0.267,0.333,0.0));

    mCamera.setFOV(90.0f);
    mCamera.setAspect(16.0f/9.0f);
    mCamera.setZRange(0.01f, 100.0f);
    mCamera.setPosition(0.0, 1.7, 5.0);

    if (!mBlockShader.loadFromFile(
        "shaders/default.330.vert", "shaders/default.330.frag"
    )) {
        quit(true);
    }

    mBlockShader.bindAttribLocation("aVertex",   0);
    mBlockShader.bindAttribLocation("aNormal",   1);
    mBlockShader.bindAttribLocation("aTexCoord", 2);
    mBlockShader.bindAttribLocation("aColor",    3);
    mBlockShader.setParameter("uResolution", sf::Vector2f(getSize()));

    mCubeModel.makeBox(sf::Vector3f(0.5,0.5,0.5), sf::Vector3f(0.5,0.5,0.5));
    mCube.setModel(mCubeModel);
    mCube.setShader(mBlockShader);

    mBallModel.makeBall(0.5, 6);
    mBall.setModel(mBallModel);
    mBall.setShader(mBlockShader);

    setMousePosition(mWindowCenter);
}

void GameWindow::quit(bool internal) {
    if (internal || mAllowQuit) {
        close();
    }
}

void GameWindow::handleEvents() {
    sf::Event event;

    while (pollEvent(event)) {
        handleEvent(event);
    }
}

void GameWindow::handleEvent(const sf::Event &event) {
    switch (event.type) {
        case sf::Event::Closed: {
            quit();
            break;
        }

        case sf::Event::Resized: {
            sf::Vector2f size(getSize());
            mBlockShader.setParameter("uResolution", size);
            GLChecked(glViewport(0, 0, size.x, size.y));
            mCamera.setAspect(size.x / size.y);
            break;
        }

        case sf::Event::LostFocus: {
            break;
        }

        case sf::Event::GainedFocus: {
            break;
        }

        case sf::Event::TextEntered: {
            break;
        }

        case sf::Event::KeyPressed: {
            /// @note for debugging
            switch (event.key.code) {
                case sf::Keyboard::Escape: {
                    quit();
                    break;
                }

                case sf::Keyboard::Tab: {
                    mMouseLocked = !mMouseLocked;
                    if (mMouseLocked) {
                        setMousePosition(mWindowCenter);
                    }
                    break;
                }

                case sf::Keyboard::Space: {
                    mPaused = not mPaused;
                    break;
                }

                case sf::Keyboard::F11: {
                    if (mFullscreen) {
                        create(mWindowMode, mWindowTitle, mWindowStyle, getSettings());
                    } else {
                        create(mDesktopMode, mWindowTitle, mWindowStyle | sf::Style::Fullscreen, getSettings());
                    }

                    GLChecked(glClearColor(0.200,0.267,0.333,0.0));

                    mFullscreen = !mFullscreen;

                    break;
                }

                default: {
                    break;
                }
            }
            break;
        }

        case sf::Event::KeyReleased: {
            break;
        }

        case sf::Event::MouseWheelMoved: {
            break;
        }

        case sf::Event::MouseWheelScrolled: {
            break;
        }

        case sf::Event::MouseButtonPressed: {
            break;
        }

        case sf::Event::MouseButtonReleased: {
            break;
        }

        case sf::Event::MouseMoved: {
            break;
        }

        case sf::Event::MouseEntered: {
            break;
        }

        case sf::Event::MouseLeft: {
            break;
        }

        case sf::Event::JoystickButtonPressed: {
            break;
        }

        case sf::Event::JoystickButtonReleased: {
            break;
        }

        case sf::Event::JoystickMoved: {
            if (std::fabs(event.joystickMove.position) >= 10.0f) {
                fprintf(stderr, "%d:%d: %.2f\n",
                        event.joystickMove.joystickId,
                        event.joystickMove.axis,
                        event.joystickMove.position);

                switch (event.joystickMove.axis) {
                    case sf::Joystick::X: {
                        mCamera.move(event.joystickMove.position * 0.01f, 0, 0);
                        break;
                    }

                    case sf::Joystick::Y: {
                        mCamera.move(0, 0, event.joystickMove.position * 0.01f);
                        break;
                    }

                    case sf::Joystick::Z: {
                        break;
                    }

                    case sf::Joystick::R: {
                        break;
                    }

                    case sf::Joystick::U: {
                        break;
                    }

                    case sf::Joystick::V: {
                        break;
                    }

                    case sf::Joystick::PovX: {
                        break;
                    }

                    case sf::Joystick::PovY: {
                        break;
                    }
                }
            }
            break;
        }

        case sf::Event::JoystickConnected: {
            break;
        }

        case sf::Event::JoystickDisconnected: {
            break;
        }

        case sf::Event::TouchBegan: {
            sf::err() << "T-down " << event.touch.finger << ':' <<
                event.touch.x << ',' << event.touch.y << std::endl;
            break;
        }

        case sf::Event::TouchMoved: {
            sf::err() << "T-move " << event.touch.finger << ':' <<
                event.touch.x << ',' << event.touch.y << std::endl;
            break;
        }

        case sf::Event::TouchEnded: {
            sf::err() << "T-up   " << event.touch.finger << ':' <<
                event.touch.x << ',' << event.touch.y << std::endl;
            break;
        }

        case sf::Event::SensorChanged: {
            break;
        }

        // silence "enumeration value not handled" warning
        default: {
            break;
        }
    }
}

void GameWindow::handleInput(const sf::Time &delta) {
    if (mMouseLocked) {
        sf::Vector2i mousePos = getMousePosition();
        sf::Vector2i mouseDelta = mousePos - mWindowCenter;
        mLookDir += sf::Vector2f(mouseDelta);
        setMousePosition(mWindowCenter);
    }

    mLookDir.x = std::fmod(mLookDir.x + 180.0f, 360.0f) - 180.0f;

    if (mLookDir.y > 89.9f) {
        mLookDir.y = 89.9f;
    }
    if (mLookDir.y < -89.9f) {
        mLookDir.y = -89.9f;
    }

    sf::Vector3f move;

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
        move.z -= 2*delta.asSeconds();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
        move.z += 2*delta.asSeconds();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
        move.x -= 2*delta.asSeconds();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
        move.x += 2*delta.asSeconds();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
        move.y -= 2*delta.asSeconds();
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
        move.y += 2*delta.asSeconds();
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
        move *= 0.25f;
    }

    mCamera.setLook(mLookDir);
    mCamera.move(move, -mLookDir.x);

}

void GameWindow::update(const sf::Time &delta) {
    mPlayTime += delta;

    if (not mPaused) {
        mSpinAngle += delta.asSeconds() * mSpinSpeed;
    }
}

void GameWindow::render() {
    GLChecked(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    ////////////////////////////////////////////////////////////
    //  3D setup
    ////////////////////////////////////////////////////////////

    GLChecked(glEnable(GL_DEPTH_TEST));
    GLChecked(glEnable(GL_CULL_FACE));

    // draw 3D scene

    /**
     * @todo
     * for each visible chunk:
     *      create vbo
     *      convert blocks to polys
     *      render vbo
     */

    //! camera.render();

    Transform3D projectionTransform(mCamera.getTransform());

    mBlockShader.setParameter("uTime", mPlayTime.asSeconds());
    mBlockShader.setParameter("uProjMatrix", projectionTransform);

    Transform3D spinLight;
    spinLight.rotate(mSpinAngle, sf::Vector3f(0,0,1));
    sf::Vector3f spinLightPos = spinLight.transformPoint(mLightPos);

    mBlockShader.setParameter("uLightPos", spinLightPos);
    mBlockShader.setParameter("uEyePos", mCamera.getPosition());

    Transform3D lightBallTransform;
    lightBallTransform.translate(spinLightPos);
    mBlockShader.setParameter("uViewMatrix", lightBallTransform);
    mBall.render();

    Transform3D modelViewTransform;

    mBlockShader.setParameter("uViewMatrix", modelViewTransform);

    mCube.render();

    modelViewTransform.translate(sf::Vector3f(1.5f,0.0f,0.0f));
    mBlockShader.setParameter("uViewMatrix", modelViewTransform);
    mCube.render();

    ////////////////////////////////////////////////////////////
    //  end 3D
    ////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////
    //  2D setup
    ////////////////////////////////////////////////////////////

    GLChecked(glDisable(GL_DEPTH_TEST));
    GLChecked(glDisable(GL_CULL_FACE));

    // draw 2D overlay


    ////////////////////////////////////////////////////////////
    //  end 2D
    ////////////////////////////////////////////////////////////
}

void GameWindow::setMousePosition(const sf::Vector2i &position) {
    sf::Mouse::setPosition(position, *this);
}

sf::Vector2i GameWindow::getMousePosition() const {
    return sf::Mouse::getPosition(*this);
}

////////////////////////////////////////////////////////////////////////////////

extern "C"
int main(int argc, char **argv) {

    // mine::Engine engine;

    //~ ChunkGenerator chunkGen;
    //~ ChunkCache chunkCache(chunkGen);

    //~ for (size_t i = 0; i < 4098; i++) {
        //~ chunkCache.getChunk(Position(0,0,0));
    //~ }

    //~ Chunk *chunk = chunkCache.getChunk(Position(0,0,0));

    //~ printf("chunk == %p, chunk->getData() == %p\n", chunk, chunk->getData());

    GameWindow gameWindow;

    gameWindow.run();

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

