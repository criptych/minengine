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
#include "ClientObject.hpp"

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
    bool mQuitting;
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

    sf::Texture mCubeDiffMap;
    sf::Texture mCubeSpecMap;
    sf::Texture mCubeGlowMap;
    sf::Texture mCubeBumpMap;

    Model mBallModel;
    Model mPlaneModel;
    Model mCubeModel;

    ClientModel mBall;
    ClientModel mPlane;
    ClientModel mCube;

    MaterialInfo mBallMtl;
    MaterialInfo mPlaneMtl;
    MaterialInfo mCubeMtl;

    ClientObject mBallObj;
    ClientObject mPlaneObj;
    ClientObject mCubeObj;

    ChunkData mTestChunkData;
    Chunk mTestChunk;

    ChunkRenderer mChunkRenderer;

public:
    GameWindow();

    void run();
    void quit();

protected:
    bool init();
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
), mQuitting(
    false
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
    //~ sf::microseconds(2083) // ~480fps
    sf::microseconds(4167) // ~240fps
    //~ sf::microseconds(8333) // ~120fps
    //~ sf::microseconds(16667) // ~60fps
    //~ sf::microseconds(33333) // ~30fps
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
), mBallObj(
    mBall, mBlockShader, mBallMtl
), mPlaneObj(
    mPlane, mBlockShader, mPlaneMtl
), mCubeObj(
    mCube, mBlockShader, mCubeMtl
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

    mQuitting = false;

    if (!init()) {
        return;
    }

    mFrameLength = sf::Time::Zero;

    while (!mQuitting) {
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

    close();
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

        const sf::Uint8 *rgb = rgbImage.getPixelsPtr();
        const sf::Uint8 *alpha = alphaImage.getPixelsPtr();

        std::vector<sf::Uint8> outData(size.x * size.y * 4);
        sf::Uint8 *out = outData.data();

        for (p.y = 0; p.y < size.y; p.y++) {
            for (p.x = 0; p.x < size.x; p.x++) {
                *out++ = *rgb++;
                *out++ = *rgb++;
                *out++ = *rgb++;
                rgb++;
                *out++ = *alpha++;
                alpha++;
                alpha++;
                alpha++;
            }
        }

        texture.create(size.x, size.y);
        texture.update(outData.data());
    }

    return true;
}

bool GameWindow::init() {
    mContextSettings = sf::ContextSettings(
        24, 8,  // depth and stencil bits
        8,      // antialiasing level
        3, 3,   // OpenGL version (major, minor)
        sf::ContextSettings::Default
        //~ sf::ContextSettings::Core
    );

    mDesktopMode = sf::VideoMode::getDesktopMode();

    std::fprintf(stderr, "Desktop mode: %dx%d %dbpp\n",
                 mDesktopMode.width, mDesktopMode.height,
                 mDesktopMode.bitsPerPixel);

    create(mWindowMode, mWindowTitle, mWindowStyle, mContextSettings);

    setVerticalSyncEnabled(true);

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

    glewExperimental = true;
    GLChecked(glewInit());

    GLChecked(glEnable(GL_DEPTH_TEST));
    GLChecked(glDepthFunc(GL_LESS));
    GLChecked(glEnable(GL_CULL_FACE));
    GLChecked(glClearColor(0.200,0.267,0.333,0.0));

    if (!loadShaders()) {
        return false;
    }

    if (!loadTextures()) {
        return false;
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

    mPlane.setModel(mPlaneModel);

    mCubeModel.makeBox(sf::Vector3f(0.5f, 0.5f, 0.5f), sf::Vector3f(0, 0.5f, 0));
    mCube.setModel(mCubeModel);

    mBallMtl.diffMap = &mWhiteTex;
    mBallMtl.specMap = &mWhiteTex;
    mBallMtl.glowMap = &mClearTex;
    mBallMtl.bumpMap = &mClearTex;
    mBallMtl.specPower = 100.0f;
    mBallMtl.bumpScale = 0.00f;
    mBallMtl.bumpBias = 0.00f;
    mBallMtl.fresnelPower = 5.0f;
    mBallMtl.fresnelScale = 1.0f;
    mBallMtl.fresnelBias = 0.0f;

    mPlaneMtl.diffMap = &mDiffMap;
    mPlaneMtl.specMap = &mSpecMap;
    mPlaneMtl.glowMap = &mGlowMap;
    mPlaneMtl.bumpMap = &mBumpMap;
    mPlaneMtl.specPower = 100.0f;
    mPlaneMtl.bumpScale = 0.02f;
    mPlaneMtl.bumpBias = 0.00f;
    mPlaneMtl.fresnelPower = 5.0f;
    mPlaneMtl.fresnelScale = 1.0f;
    mPlaneMtl.fresnelBias = 0.0f;

    mCubeMtl.diffMap = &mCubeDiffMap;
    mCubeMtl.specMap = &mCubeSpecMap;
    mCubeMtl.glowMap = &mCubeGlowMap;
    mCubeMtl.bumpMap = &mCubeBumpMap;
    mCubeMtl.specPower = 1000.0f;
    mCubeMtl.bumpScale = 0.05f;
    mCubeMtl.bumpBias = -0.02f;
    mCubeMtl.fresnelPower = 5.0f;
    mCubeMtl.fresnelScale = 1.0f;
    mCubeMtl.fresnelBias = 0.0f;

    if (hasFocus()) {
        lockMouse();
    }

    return true;
}

void GameWindow::quit(bool internal) {
    if (internal || mAllowQuit) {
        mQuitting = true;
    }
}

bool GameWindow::loadShaders() {
    mBlockShader.setAttribLocation("aVertex",   0);
    mBlockShader.setAttribLocation("aNormal",   1);
    mBlockShader.setAttribLocation("aTexCoord", 2);

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
    mDiffMap.generateMipmap();

    if (!mergeImages("textures/Scifi_Hex_Wall_specular.jpg", "textures/Scifi_Hex_Wall_glossiness.jpg", mSpecMap)) {
        return false;
    }
    mSpecMap.setSmooth(true);
    mSpecMap.setRepeated(true);
    mSpecMap.generateMipmap();

    if (!mergeImages("textures/Scifi_Hex_Wall_glow.jpg", "textures/Scifi_Hex_Wall_Ambient_Occlusion.jpg", mGlowMap)) {
        return false;
    }
    mGlowMap.setSmooth(true);
    mGlowMap.setRepeated(true);
    mGlowMap.generateMipmap();

    if (!mergeImages("textures/Scifi_Hex_Wall_normal.jpg", "textures/Scifi_Hex_Wall_Displacement.jpg", mBumpMap)) {
        return false;
    }
    mBumpMap.setSmooth(true);
    mBumpMap.setRepeated(true);
    mBumpMap.generateMipmap();

    if (!mCubeDiffMap.loadFromFile("textures/cube_albedo.png")) {
        return false;
    }
    mCubeDiffMap.setSmooth(true);
    mCubeDiffMap.setRepeated(true);
    mCubeDiffMap.generateMipmap();

    if (!mCubeSpecMap.loadFromFile("textures/cube_specular.png")) {
        return false;
    }
    mCubeSpecMap.setSmooth(true);
    mCubeSpecMap.setRepeated(true);
    mCubeSpecMap.generateMipmap();

    if (!mCubeGlowMap.loadFromFile("textures/cube_glow.png")) {
        return false;
    }
    mCubeGlowMap.setSmooth(true);
    mCubeGlowMap.setRepeated(true);
    mCubeGlowMap.generateMipmap();

    if (!mCubeBumpMap.loadFromFile("textures/cube_normal.png")) {
        return false;
    }
    mCubeBumpMap.setSmooth(true);
    mCubeBumpMap.setRepeated(true);
    mCubeBumpMap.generateMipmap();

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
            sf::err() << "Window resized to " << size.x << 'x' << size.y << '\n';
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

        float fov = mPlayer.getCamera().getFOV();

        if (sf::Mouse::isButtonPressed(sf::Mouse::Right)) {
            if (fov > 30.0f) {
                fov -= 100.0f * delta.asSeconds();
            }
        } else {
            if (fov < 75.0f) {
                fov += 100.0f * delta.asSeconds();
            }
        }

        mPlayer.getCamera().setFOV(fov);

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

    sf::Transform3D projectionTransform(mPlayer.getCamera().getTransform());
    sf::Transform3D modelViewTransform(mPlayer.getTransform());

    mBlockShader.setParameter("uTime", mPlayTime.asSeconds());
    mBlockShader.setParameter("uProjMatrix", projectionTransform);
    mBlockShader.setParameter("uViewMatrix", modelViewTransform);

    sf::Transform3D spinLight;
    spinLight.rotate(mSpinAngle, sf::Vector3f(0,1,0));
    sf::Vector3f spinLightPos = spinLight.transformPoint(mLightPos);

    static const sf::Color lightAmbt(51,51,51);
    static const sf::Color lightDiff(204,204,204);
    static const sf::Color lightSpec(255,255,255);

    mBlockShader.setParameter("uLights[0].position", modelViewTransform * spinLightPos);
    mBlockShader.setParameter("uLights[0].ambtColor", lightAmbt);
    mBlockShader.setParameter("uLights[0].diffColor", lightDiff);
    mBlockShader.setParameter("uLights[0].specColor", lightSpec);
    mBlockShader.setParameter("uEyePos", mPlayer.getEyePosition());

    mPlaneObj.render();
    mCubeObj.render();

    sf::Transform3D lightBallTransform;
    lightBallTransform.translate(spinLightPos);
    mBlockShader.setParameter("uViewMatrix", modelViewTransform * lightBallTransform);

    mBallObj.render();

    ////////////////////////////////////////////////////////////
    //  end 3D
    ////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////
    //  2D setup
    ////////////////////////////////////////////////////////////

    GLChecked(glDisable(GL_DEPTH_TEST));
    GLChecked(glDisable(GL_CULL_FACE));
    GLChecked(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));

    GLChecked(glBindBuffer(GL_ARRAY_BUFFER, 0));

    // draw 2D overlay

    GLChecked(pushGLStates());

    float frameLength = mFrameLength.asSeconds();
    float inputFrac = mInputLength.asSeconds() / frameLength;
    float updateFrac = mUpdateLength.asSeconds() / frameLength;
    float renderFrac = mRenderLength.asSeconds() / frameLength;
    float idleFrac = 1.0f - inputFrac - updateFrac - renderFrac;

    sf::RectangleShape rect(sf::Vector2f(32, 32));
    rect.setFillColor(sf::Color::White);
    GLChecked(draw(rect));

    rect.setPosition(sf::Vector2f(0, 32.0f * (idleFrac)));
    rect.setSize(sf::Vector2f(32, 32.0f * (inputFrac + updateFrac + renderFrac)));
    rect.setFillColor(sf::Color::Red);
    GLChecked(draw(rect));

    rect.setPosition(sf::Vector2f(0, 32.0f * (idleFrac + renderFrac)));
    rect.setSize(sf::Vector2f(32, 32.0f * (inputFrac + updateFrac)));
    rect.setFillColor(sf::Color::Green);
    GLChecked(draw(rect));

    rect.setPosition(sf::Vector2f(0, 32.0f * (idleFrac + renderFrac + updateFrac)));
    rect.setSize(sf::Vector2f(32, 32.0f * (inputFrac)));
    rect.setFillColor(sf::Color::Blue);
    GLChecked(draw(rect));

    mDebugText.setPosition(sf::Vector2f(32, 0));
    GLChecked(draw(mDebugText));

    GLChecked(popGLStates());

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

