////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "engine/engine.hpp"

#include <GL/glew.h>

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <SFML/Graphics.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>

#include <cstdio>
#include <cmath>

#include "GLCheck.hpp"

////////////////////////////////////////////////////////////////////////////////

//~ #include "Transform3D.hpp"
#include "Transformable3D.hpp"
#include "Shader.hpp"
#include "Camera.hpp"
#include "ClientModel.hpp"

////////////////////////////////////////////////////////////////////////////////

class Player {
    Camera mCamera;
    float mEyeHeight;
    sf::Vector3f mPosition;
    sf::Vector2f mLookDir;

    Physics::Body mBody;

    mutable sf::Transform3D mTransform;
    mutable bool mNeedsUpdate;

public:
    Player();

    Camera &getCamera();
    const Camera &getCamera() const;

    const sf::Transform3D &getTransform() const;
    sf::Transform3D getViewTransform() const;

    sf::Vector3f getEyePosition() const;

    void setPosition(const sf::Vector3f &position);
    const sf::Vector3f &getPosition() const;

    void move(const sf::Vector3f &offset);

    void setLook(const sf::Vector2f &look);
    const sf::Vector2f &getLook() const;

    void look(const sf::Vector2f &look);

    void update(const sf::Time &delta);
    void render() const;
};

////////////////////////////////////////////////////////////////////////////////

Player::Player(
): mCamera(
    90.0f, 16.0f/9.0f, 0.01f, 100.0f
), mEyeHeight(
    1.7f
), mLookDir(
), mNeedsUpdate(
    true
) {
    mBody.setBounds(BoundingVolume::capsule(0.4f, 1.77f));
}

Camera &Player::getCamera() {
    return mCamera;
}

const Camera &Player::getCamera() const {
    return mCamera;
}

const sf::Transform3D &Player::getTransform() const {
    if (mNeedsUpdate) {
        mTransform = sf::Transform3D();
        mTransform.rotate(mLookDir.y, sf::Vector3f(1.0f, 0.0f, 0.0f));
        mTransform.rotate(mLookDir.x, sf::Vector3f(0.0f, 1.0f, 0.0f));
        mTransform.translate(-getEyePosition());
        mNeedsUpdate = false;
    }
    return mTransform;
}

sf::Transform3D Player::getViewTransform() const {
    return mCamera.getTransform() * getTransform();
}

const sf::Vector3f &Player::getPosition() const {
    return mPosition;
}

void Player::setPosition(const sf::Vector3f &position) {
    mPosition = position;
}

sf::Vector3f Player::getEyePosition() const {
    return sf::Vector3f(mPosition.x, mPosition.y + mEyeHeight, mPosition.z);
}

void Player::setLook(const sf::Vector2f &look) {
    mLookDir = look;
    mNeedsUpdate = true;
}

const sf::Vector2f &Player::getLook() const {
    return mLookDir;
}

void Player::look(const sf::Vector2f &look) {
    mLookDir += look;

    if (mLookDir.y > 90.0f) {
        mLookDir.y = 90.0f;
    } else if (mLookDir.y < -90.0f) {
        mLookDir.y = -90.0f;
    }
}

void Player::move(const sf::Vector3f &move) {
    mPosition += sf::Transform3D().rotate(-getLook().x, sf::Vector3f(0,1,0)) * move;
    mNeedsUpdate = true;
}

void Player::render() const {
}

////////////////////////////////////////////////////////////////////////////////

class GameWindow : protected sf::RenderWindow {
    bool mFullscreen;
    bool mMouseLocked;
    bool mAllowQuit;
    bool mPaused;

    sf::VideoMode mDesktopMode;
    sf::VideoMode mWindowMode;
    sf::String mWindowTitle;
    sf::Uint32 mWindowStyle;
    sf::Vector2i mWindowCenter;

    sf::Font mFont;
    sf::Text mDebugText;

    sf::Time mTickLength;
    sf::Time mPlayTime;
    unsigned int mMaxTicksPerFrame;

    Player mPlayer;
    //~ Camera mCamera;
    mutable sf::Shader mBlockShader;

    sf::Vector3f mLightPos;
    //~ sf::Vector2f mLookDir;
    float mSpinAngle;
    float mSpinSpeed; // degrees/second

    Model mCubeModel;
    ClientModel mCube;

    Model mBallModel;
    ClientModel mBall;

    Model mPlaneModel;
    ClientModel mPlane;

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
    unsigned int contextAttribs = sf::ContextSettings::Default;
    //~ unsigned int contextAttribs = sf::ContextSettings::Core;
    sf::ContextSettings contextSettings(24,8, 0, 3,3, contextAttribs);

    mDesktopMode = sf::VideoMode::getDesktopMode();

    std::fprintf(stderr, "Desktop mode: %dx%d %dbpp\n",
                 mDesktopMode.width, mDesktopMode.height,
                 mDesktopMode.bitsPerPixel);

    create(videoMode, windowTitle, windowStyle, contextSettings);

    const sf::ContextSettings &usedSettings = getSettings();
    sf::err() << "Using OpenGL " << usedSettings.majorVersion << '.' << usedSettings.minorVersion << ' ' <<
        ((usedSettings.attributeFlags & sf::ContextSettings::Core) ? "Core" : "Compat") <<
        ((usedSettings.attributeFlags & sf::ContextSettings::Debug) ? " (Debug)" : "") <<
        ".\n";

    mFont.loadFromFile("fonts/Vera.ttf");
    mDebugText.setFont(mFont);
    mDebugText.setCharacterSize(16);

    mPlayer.setPosition(sf::Vector3f(0,0,5));

    sf::Vector3f eye = mPlayer.getEyePosition();
    sf::err() << "mPlayer.getEyePosition() == " << eye.x << ',' << eye.y << ',' << eye.z << '\n';

    mWindowCenter = sf::Vector2i(getSize()) / 2;

    GLChecked(glewInit());

    GLChecked(glEnable(GL_DEPTH_TEST));
    GLChecked(glDepthFunc(GL_LESS));
    GLChecked(glEnable(GL_CULL_FACE));
    GLChecked(glClearColor(0.200,0.267,0.333,0.0));

    mBlockShader.setAttribLocation("aVertex",   0);
    mBlockShader.setAttribLocation("aNormal",   1);
    mBlockShader.setAttribLocation("aTexCoord", 2);
    mBlockShader.setAttribLocation("aColor",    3);

    if (!mBlockShader.loadFromFile(
        "shaders/default.330.vert", "shaders/default.330.frag"
    )) {
        quit(true);
    }

    mBlockShader.setParameter("uResolution", sf::Vector2f(getSize()));

    mCubeModel.makeBox(sf::Vector3f(0.5f,0.5f,0.5f), sf::Vector3f(0.0f,0.0f,0.5f));
    mCube.setModel(mCubeModel);
    mCube.setShader(mBlockShader);

    mBallModel.makeBall(0.5, 6);
    //~ mBallModel.setPrimitive(GL_LINE_STRIP);
    mBall.setModel(mBallModel);
    mBall.setShader(mBlockShader);

    mPlaneModel.makeBox(sf::Vector3f(5.0f,1.0f,5.0f));
    mPlane.setModel(mPlaneModel);
    mPlane.setShader(mBlockShader);

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
            mPlayer.getCamera().setAspect(size.x / size.y);
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
                        //~ mPlayer.move(event.joystickMove.position * 0.01f, 0, 0);
                        break;
                    }

                    case sf::Joystick::Y: {
                        //~ mPlayer.move(0, 0, event.joystickMove.position * 0.01f);
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
        mPlayer.look(sf::Vector2f(mouseDelta));
        setMousePosition(mWindowCenter);
    }

    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
        mPlayer.look(sf::Vector2f(-180*delta.asSeconds(),0));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
        mPlayer.look(sf::Vector2f(180*delta.asSeconds(),0));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
        mPlayer.look(sf::Vector2f(0,-180*delta.asSeconds()));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
        mPlayer.look(sf::Vector2f(0,180*delta.asSeconds()));
    }
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::R)) {
        mPlayer.setLook(sf::Vector2f(0,0));
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

    //~ mLookDir.x = std::fmod(mLookDir.x + 180.0f, 360.0f) - 180.0f;
    //~ mLookDir.y = std::min(89.9f, std::max(-89.9f, mLookDir.y));

    //~ mPlayer.getCamera().setLook(mLookDir);
    //~ mPlayer.getCamera().move(move, -mLookDir.x);

    mPlayer.move(move);
}

void GameWindow::update(const sf::Time &delta) {
    mPlayTime += delta;

    if (not mPaused) {
        mSpinAngle += delta.asSeconds() * mSpinSpeed;
    }

    char temp[256];
    sf::Vector3f p = mPlayer.getPosition();
    sf::Vector3f e = mPlayer.getEyePosition();
    sf::Vector2f o = mPlayer.getLook();
    snprintf(temp, sizeof(temp), "%g,%g,%g (%g,%g,%g)\n%g,%g",
             p.x, p.y, p.z, e.x, e.y, e.z, o.x, o.y);
    mDebugText.setString(temp);
}

void GameWindow::render() {
    GLChecked(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    ////////////////////////////////////////////////////////////
    //  3D setup
    ////////////////////////////////////////////////////////////

    GLChecked(glEnable(GL_DEPTH_TEST));
    GLChecked(glEnable(GL_CULL_FACE));
    //~ GLChecked(glPolygonMode(GL_BACK, GL_LINE));

    // draw 3D scene

    /**
     * @todo
     * for each visible chunk:
     *      create vbo
     *      convert blocks to polys
     *      render vbo
     */

    //! camera.render();

    mPlayer.render();

    sf::Transform3D projectionTransform(mPlayer.getViewTransform());

    mBlockShader.setParameter("uTime", mPlayTime.asSeconds());
    mBlockShader.setParameter("uProjMatrix", projectionTransform);

    sf::Transform3D spinLight;
    spinLight.rotate(mSpinAngle, sf::Vector3f(0,0,1));
    sf::Vector3f spinLightPos = spinLight.transformPoint(mLightPos);

    mBlockShader.setParameter("uLightPos", spinLightPos);
    mBlockShader.setParameter("uEyePos", mPlayer.getEyePosition());

    sf::Transform3D lightBallTransform;
    lightBallTransform.translate(spinLightPos);
    mBlockShader.setParameter("uViewMatrix", lightBallTransform);
    mBall.render();

    sf::Transform3D modelViewTransform;

    mBlockShader.setParameter("uViewMatrix", modelViewTransform);

    mCube.render();

    modelViewTransform.translate(sf::Vector3f(1.0f,0.0f,0.0f));
    mBlockShader.setParameter("uViewMatrix", modelViewTransform);
    mCube.render();

    mPlane.render();

    ////////////////////////////////////////////////////////////
    //  end 3D
    ////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////
    //  2D setup
    ////////////////////////////////////////////////////////////

    GLChecked(glDisable(GL_DEPTH_TEST));
    GLChecked(glDisable(GL_CULL_FACE));

    // draw 2D overlay

    pushGLStates();

    draw(mDebugText);

    popGLStates();

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

