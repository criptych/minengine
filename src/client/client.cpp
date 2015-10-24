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

#include "Transformable3D.hpp"
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

class ChunkRenderer {
public:
    void render(sf::RenderTarget &target, const Chunk &chunk);
};

////////////////////////////////////////////////////////////////////////////////

void ChunkRenderer::render(sf::RenderTarget &target, const Chunk &chunk) {
}

////////////////////////////////////////////////////////////////////////////////

class GameWindow : protected sf::RenderWindow {
    bool mFullscreen;
    bool mMouseLocked;
    bool mAllowQuit;
    bool mPaused;

    int mViewMode;

    sf::VideoMode mDesktopMode;
    sf::VideoMode mWindowMode;
    sf::String mWindowTitle;
    sf::Uint32 mWindowStyle;
    sf::Vector2i mWindowCenter;
    sf::ContextSettings mContextSettings;

    sf::Font mFont;
    sf::Text mDebugText;

    sf::Time mMinFrameLength;
    sf::Time mFrameDelay;
    sf::Time mTickLength;
    sf::Time mPlayTime;
    unsigned int mMaxTicksPerFrame;

    float mTicksPerSecond;
    float mFramesPerSecond;
    sf::Time mInputLength;
    sf::Time mUpdateLength;
    sf::Time mRenderLength;
    sf::Time mIdleLength;
    sf::Time mFrameLength;

    Player mPlayer;
    mutable sf::Shader mBlockShader;

    sf::Vector3f mLightPos;
    float mSpinAngle;
    float mSpinSpeed; // degrees/second

    sf::Texture mDiffMap;
    sf::Texture mSpecMap;
    sf::Texture mGlowMap;
    sf::Texture mBumpMap;

    sf::Texture mWhiteTex;
    sf::Texture mBlackTex;
    sf::Texture mClearTex;

    Model mBallModel;
    ClientModel mBall;

    Model mPlaneModel;
    ClientModel mPlane;

    ChunkData mTestChunkData;
    Chunk mTestChunk;

    ChunkRenderer mChunkRenderer;

public:
    GameWindow();

    void run();
    void quit();

protected:
    void init();
    void quit(bool internal);

    bool loadShaders();
    bool loadTextures();

    void handleEvents();

    void handleInput(const sf::Time &delta);
    void handleEvent(const sf::Event &event);
    void update(const sf::Time &delta);
    void render();

    void setMousePosition(const sf::Vector2i &position);
    sf::Vector2i getMousePosition() const;
    void lockMouse();
    void unlockMouse();
};

////////////////////////////////////////////////////////////////////////////////

GameWindow::GameWindow(
): mFullscreen(
    false
), mMouseLocked(
    false
), mAllowQuit(
    true
), mPaused(
    false
), mViewMode(
    0
), mWindowMode(
    1280, 720
), mWindowTitle(
    L"MinEngine Client"
), mWindowStyle(
    sf::Style::Default
), mMinFrameLength(
    sf::microseconds(4167) // ~240fps
), mFrameDelay(
    //~ sf::microseconds(1000)
), mTickLength(
    sf::microseconds(20000) // 50000
), mMaxTicksPerFrame(
    5
), mLightPos(
    0, 2, 10
), mSpinAngle(
), mSpinSpeed(
    12
), mTestChunkData(
), mTestChunk(
    Position(),
    &mTestChunkData
) {
}

void GameWindow::run() {
    sf::Clock clock;
    sf::Time tickAccum;

    size_t tickCount = 0;
    size_t frameCount = 0;
    sf::Time fpsAccum;
    sf::Time fpsInterval(sf::microseconds(1000000));

    init();

    mFrameLength = sf::Time::Zero;

    while (isOpen()) {
        sf::Time delta = clock.restart();

        handleEvents();

        handleInput(delta);

        sf::Time inputTime = clock.getElapsedTime();

        tickAccum += delta;
        fpsAccum += delta;

        unsigned int frameTicks = mMaxTicksPerFrame;

        while (tickAccum >= mTickLength) {
            tickAccum -= mTickLength;

            if (frameTicks > 0) {
                update(mTickLength);
                tickCount += 1;
                frameTicks -= 1;
            }
        }

        sf::Time updateTime = clock.getElapsedTime();

        render();

        sf::Time renderTime = clock.getElapsedTime();

        display();

        sf::sleep(mFrameDelay);

        sf::Time endTime = clock.getElapsedTime();

        static const float lastFrameRatio = 0.4f;
        static const float nextFrameRatio = 0.6f;

        mInputLength = lastFrameRatio * mInputLength + nextFrameRatio * inputTime;
        mUpdateLength = lastFrameRatio * mUpdateLength + nextFrameRatio * (updateTime - inputTime);
        mRenderLength = lastFrameRatio * mRenderLength + nextFrameRatio * (renderTime - updateTime);
        mIdleLength = lastFrameRatio * mIdleLength + nextFrameRatio * (endTime - renderTime);
        mFrameLength = lastFrameRatio * mFrameLength + nextFrameRatio * endTime;

        frameCount += 1;

        if (fpsAccum >= fpsInterval) {
            mTicksPerSecond = float(tickCount) / fpsAccum.asSeconds();
            mFramesPerSecond = float(frameCount) / fpsAccum.asSeconds();
            tickCount = frameCount = 0;
            fpsAccum = sf::Time::Zero;

            mFrameDelay = mMinFrameLength - mInputLength - mUpdateLength - mRenderLength;
        }
    }

    quit(true);
}

void GameWindow::quit() {
    quit(false);
}

static bool mergeImages(const std::string &rgbFN, const std::string &alphaFN, sf::Texture &texture) {
    sf::Image rgbImage, alphaImage;

    if (!rgbImage.loadFromFile(rgbFN)) {
        return false;
    }

    if (!alphaImage.loadFromFile(alphaFN)) {
        return false;
    }

    sf::Vector2u size(rgbImage.getSize()), p;

    if (size == alphaImage.getSize()) {
        sf::err() << "merging \"" << rgbFN << "\" and \"" << alphaFN << "\"...\n";
        for (p.y = 0; p.y < size.y; p.y++) {
            for (p.x = 0; p.x < size.x; p.x++) {
                sf::Color c = rgbImage.getPixel(p.x, p.y);
                sf::Color a = alphaImage.getPixel(p.x, p.y);
                c.a = (a.r + a.g + a.b) / 3;
                rgbImage.setPixel(p.x, p.y, c);
            }
        }
    }

    texture.loadFromImage(rgbImage);

    return true;
}

void GameWindow::init() {
    mContextSettings = sf::ContextSettings(
        24, 8,  // depth and stencil bits
        8,      // antialiasing level
        3,3,    // OpenGL version (major, minor)
        sf::ContextSettings::Default
        //~ sf::ContextSettings::Core
    );

    mDesktopMode = sf::VideoMode::getDesktopMode();

    std::fprintf(stderr, "Desktop mode: %dx%d %dbpp\n",
                 mDesktopMode.width, mDesktopMode.height,
                 mDesktopMode.bitsPerPixel);

    create(mWindowMode, mWindowTitle, mWindowStyle, mContextSettings);

    const sf::ContextSettings &usedSettings = getSettings();
    sf::err() << "Using OpenGL " << usedSettings.majorVersion << '.' << usedSettings.minorVersion << ' ' <<
        ((usedSettings.attributeFlags & sf::ContextSettings::Core) ? "Core" : "Compat") <<
        ((usedSettings.attributeFlags & sf::ContextSettings::Debug) ? " (Debug)" : "") <<
        ".\n";

    mFont.loadFromFile("fonts/VeraMono.ttf");
    mDebugText.setFont(mFont);
    mDebugText.setCharacterSize(16);

    mPlayer.getCamera().setFOV(75.0f);
    mPlayer.setPosition(sf::Vector3f(0,0,0));

    sf::Vector3f eye = mPlayer.getEyePosition();
    sf::err() << "mPlayer.getEyePosition() == " << eye.x << ',' << eye.y << ',' << eye.z << '\n';

    GLChecked(glewInit());

    GLChecked(glEnable(GL_DEPTH_TEST));
    GLChecked(glDepthFunc(GL_LESS));
    GLChecked(glEnable(GL_CULL_FACE));
    GLChecked(glClearColor(0.200,0.267,0.333,0.0));

    if (!loadShaders()) {
        quit(true);
    }

    if (!loadTextures()) {
        quit(true);
    }

    sf::Image white;
    white.create(16, 16, sf::Color::White);
    mWhiteTex.loadFromImage(white);

    sf::Image black;
    black.create(16, 16, sf::Color::Black);
    mBlackTex.loadFromImage(black);

    sf::Image clear;
    clear.create(16, 16, sf::Color::Transparent);
    mClearTex.loadFromImage(clear);

    mBallModel.makeBall(0.5f, 8, 16);
    mBall.setModel(mBallModel);
    mBall.setShader(mBlockShader);

    mPlaneModel.setPrimitive(GL_TRIANGLES);

    sf::Vector3f size(2, 0, 2), pos(0, 0, 0);

    sf::Vector3f field(50.0, 0, 50.0);
    sf::Vector3f max(field.x - size.x, 0, field.z - size.z);
    sf::Vector3f min(-max);

    size_t n = (2 * field.x) / (max.x - min.x);
    size_t m = (2 * field.z) / (max.z - min.z);
    mPlaneModel.reserveVertices(24 * n * m);
    mPlaneModel.reserveIndices(36 * n * m);

    sf::err() << "min = <" << min.x << ',' << min.y << ',' << min.z << ">\n";
    sf::err() << "max = <" << max.x << ',' << max.y << ',' << max.z << ">\n";

    for (pos.z = min.z; pos.z <= max.z; pos.z += size.z * 2) {
        for (pos.x = min.x; pos.x <= max.x; pos.x += size.x * 2) {
            mPlaneModel.addBox(size, pos);
        }
    }

    //~ mPlaneModel.setColor(sf::Color::Green);
    mPlane.setModel(mPlaneModel);
    mPlane.setShader(mBlockShader);

    lockMouse();
}

void GameWindow::quit(bool internal) {
    if (internal || mAllowQuit) {
        close();
    }
}

bool GameWindow::loadShaders() {
    mBlockShader.setAttribLocation("aVertex",   0);
    mBlockShader.setAttribLocation("aNormal",   1);
    mBlockShader.setAttribLocation("aTexCoord", 2);
    mBlockShader.setAttribLocation("aColor",    3);

    return mBlockShader.loadFromFile(
        "shaders/default.330.vert", "shaders/default.330.frag"
    );
}

bool GameWindow::loadTextures() {
    if (!mDiffMap.loadFromFile("textures/Scifi_Hex_Wall_Albedo.jpg")) {
        return false;
    }
    mDiffMap.setSmooth(true);
    mDiffMap.setRepeated(true);

    if (!mergeImages("textures/Scifi_Hex_Wall_specular.jpg", "textures/Scifi_Hex_Wall_glossiness.jpg", mSpecMap)) {
        return false;
    }
    mSpecMap.setSmooth(true);
    mSpecMap.setRepeated(true);

    if (!mergeImages("textures/Scifi_Hex_Wall_glow.jpg", "textures/Scifi_Hex_Wall_Ambient_Occlusion.jpg", mGlowMap)) {
        return false;
    }
    mGlowMap.setSmooth(true);
    mGlowMap.setRepeated(true);

    if (!mergeImages("textures/Scifi_Hex_Wall_normal.jpg", "textures/Scifi_Hex_Wall_Displacement.jpg", mBumpMap)) {
        return false;
    }
    mBumpMap.setSmooth(true);
    mBumpMap.setRepeated(true);

    return true;
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
            if (size.y > 0) {
                mPlayer.getCamera().setAspect(size.x / size.y);
            }
            mWindowCenter = sf::Vector2i(size) / 2;
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
                    if (mMouseLocked) {
                        unlockMouse();
                    } else {
                        lockMouse();
                    }
                    break;
                }

                case sf::Keyboard::Space: {
                    mPaused = not mPaused;
                    break;
                }

                case sf::Keyboard::F1: {
                    ++mViewMode %= 3;
                    break;
                }

                case sf::Keyboard::F11: {
                    bool wasLocked = mMouseLocked;

                    if (wasLocked) {
                        unlockMouse();
                    }

                    if (mFullscreen) {
                        create(mWindowMode, mWindowTitle, mWindowStyle, mContextSettings);
                    } else {
                        create(mDesktopMode, mWindowTitle, mWindowStyle | sf::Style::Fullscreen, mContextSettings);
                    }

                    GLChecked(glClearColor(0.200,0.267,0.333,0.0));

                    mFullscreen = !mFullscreen;

                    if (wasLocked) {
                        lockMouse();
                    }

                    break;
                }

                case sf::Keyboard::R: {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                        loadTextures();
                    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
                        loadShaders();
                    }
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
    if (hasFocus()) {
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
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
            move *= 2.0f;
        }

        mPlayer.move(move);
    }
}

void GameWindow::update(const sf::Time &delta) {
    mPlayTime += delta;

    if (not mPaused) {
        mSpinAngle += delta.asSeconds() * mSpinSpeed;
    }
}

void GameWindow::render() {
    char temp[256];
    sf::Vector3f p = mPlayer.getPosition();
    sf::Vector3f e = mPlayer.getEyePosition();
    sf::Vector2f o = mPlayer.getLook();
    snprintf(temp, sizeof(temp),
             "%.2ffps (%lldus/f, %lldus delay) / %.2ftps\n"
             "%8.4f,%8.4f,%8.4f (%8.4f,%8.4f,%8.4f)\n"
             "%8.4f,%8.4f",
             mFramesPerSecond, mFrameLength.asMicroseconds(), mFrameDelay.asMicroseconds(), mTicksPerSecond,
             p.x, p.y, p.z, e.x, e.y, e.z, o.x, o.y);
    mDebugText.setString(temp);

    GLChecked(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    ////////////////////////////////////////////////////////////
    //  3D setup
    ////////////////////////////////////////////////////////////

    GLChecked(glEnable(GL_DEPTH_TEST));

    switch (mViewMode) {
        case 0: {
            // normal
            GLChecked(glEnable(GL_CULL_FACE));
            break;
        }

        case 1: {
            // wireframe from back
            GLChecked(glPolygonMode(GL_BACK, GL_LINE));
            break;
        }

        case 2: {
            // wireframe
            GLChecked(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
            break;
        }
    }

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
    spinLight.rotate(mSpinAngle, sf::Vector3f(0,1,0));
    sf::Vector3f spinLightPos = spinLight.transformPoint(mLightPos);

    mBlockShader.setParameter("uLightPos", spinLightPos);
    mBlockShader.setParameter("uEyePos", mPlayer.getEyePosition());

    sf::Transform3D lightBallTransform;
    lightBallTransform.translate(spinLightPos);
    mBlockShader.setParameter("uViewMatrix", lightBallTransform);

    mBlockShader.setParameter("uDiffMap", mWhiteTex);
    mBlockShader.setParameter("uSpecMap", mWhiteTex);
    mBlockShader.setParameter("uGlowMap", mClearTex);
    mBlockShader.setParameter("uBumpMap", mClearTex);
    mBall.render();

    sf::Transform3D modelViewTransform;
    mBlockShader.setParameter("uViewMatrix", modelViewTransform);

    mBlockShader.setParameter("uDiffMap", mDiffMap);
    mBlockShader.setParameter("uSpecMap", mSpecMap);
    mBlockShader.setParameter("uGlowMap", mGlowMap);
    mBlockShader.setParameter("uBumpMap", mBumpMap);
    mPlane.render();

    ////////////////////////////////////////////////////////////
    //  end 3D
    ////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////
    //  2D setup
    ////////////////////////////////////////////////////////////

    GLChecked(glDisable(GL_DEPTH_TEST));
    GLChecked(glDisable(GL_CULL_FACE));
    GLChecked(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    // draw 2D overlay

    pushGLStates();

    float frameLength = mFrameLength.asSeconds();
    float inputFrac = mInputLength.asSeconds() / frameLength;
    float updateFrac = mUpdateLength.asSeconds() / frameLength;
    float renderFrac = mRenderLength.asSeconds() / frameLength;
    float idleFrac = 1.0f - inputFrac - updateFrac - renderFrac;

    sf::RectangleShape rect(sf::Vector2f(32, 32));
    rect.setFillColor(sf::Color::White);
    draw(rect);

    rect.setPosition(sf::Vector2f(0, 32.0f * (idleFrac)));
    rect.setSize(sf::Vector2f(32, 32.0f * (inputFrac + updateFrac + renderFrac)));
    rect.setFillColor(sf::Color::Red);
    draw(rect);

    rect.setPosition(sf::Vector2f(0, 32.0f * (idleFrac + renderFrac)));
    rect.setSize(sf::Vector2f(32, 32.0f * (inputFrac + updateFrac)));
    rect.setFillColor(sf::Color::Green);
    draw(rect);

    rect.setPosition(sf::Vector2f(0, 32.0f * (idleFrac + renderFrac + updateFrac)));
    rect.setSize(sf::Vector2f(32, 32.0f * (inputFrac)));
    rect.setFillColor(sf::Color::Blue);
    draw(rect);

    mDebugText.setPosition(sf::Vector2f(32, 0));
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

void GameWindow::lockMouse() {
    mMouseLocked = true;
    mWindowCenter = sf::Vector2i(getSize()) / 2;
    setMousePosition(mWindowCenter);
}

void GameWindow::unlockMouse() {
    mMouseLocked = false;
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

