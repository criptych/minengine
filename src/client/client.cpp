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

extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}; // extern "C"

#include <cstdio>
#include <cmath>

#include "GLCheck.hpp"

////////////////////////////////////////////////////////////////////////////////

#include "Transformable3D.hpp"
#include "Camera.hpp"
#include "ClientModel.hpp"
#include "ClientObject.hpp"
#include "ResourceCache.hpp"

////////////////////////////////////////////////////////////////////////////////

class ShaderCache : public ResourceCache<sf::Shader> {
    sf::Shader *load(sf::Shader *shader, const std::string &name) {
        shader->setAttribLocation("aVertex",   0);
        shader->setAttribLocation("aNormal",   1);
        shader->setAttribLocation("aTexCoord", 2);
        if (!shader->loadFromFile(name+".vert", name+".frag")) {
            return nullptr;
        }
        return shader;
    }
};

class TextureCache : public ResourceCache<sf::Texture> {
    sf::Texture *load(sf::Texture *texture, const std::string &name) {
        if (!texture->loadFromFile(name)) {
            return nullptr;
        }
        texture->setSmooth(true);
        texture->setRepeated(true);
        texture->generateMipmap();
        return texture;
    }
};

////////////////////////////////////////////////////////////////////////////////

class Player {
    Camera mCamera;
    float mEyeHeight;
    sf::Vector2f mLookDir;

    Physics::Body mBody;

    mutable sf::Transform3D mTransform;
    mutable bool mNeedsUpdate;

public:
    Player();

    Camera &getCamera();
    const Camera &getCamera() const;

    Physics::Body &getBody();
    const Physics::Body &getBody() const;

    const sf::Transform3D &getTransform() const;

    sf::Vector3f getEyePosition() const;

    void setPosition(const sf::Vector3f &position);
    sf::Vector3f getPosition() const;

    void move(const sf::Vector3f &offset);

    void setLook(const sf::Vector2f &look);
    const sf::Vector2f &getLook() const;

    void look(const sf::Vector2f &look);

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

Physics::Body &Player::getBody() {
    return mBody;
}

const Physics::Body &Player::getBody() const {
    return mBody;
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

sf::Vector3f Player::getPosition() const {
    return sf::Vector3f(mBody.getPosition()) / 256.0f;
}

void Player::setPosition(const sf::Vector3f &position) {
    mBody.setPosition(Position(position * 256.0f));
    mNeedsUpdate = true;
}

sf::Vector3f Player::getEyePosition() const {
    sf::Vector3f position(getPosition());
    return sf::Vector3f(position.x, position.y + mEyeHeight, position.z);
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
    setPosition(getPosition() + sf::Transform3D().rotate(-getLook().x, sf::Vector3f(0,1,0)) * move);
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

static ShaderCache sShaderCache;
static TextureCache sTextureCache;
static std::vector<ClientObject*> sObjects;

int ll_Object___new(lua_State *L) {
    luaL_checktype(L, 2, LUA_TTABLE);
    ClientObject **pobj = static_cast<ClientObject**>(
        lua_newuserdata(L, sizeof(ClientObject*)));
    ClientObject *obj = *pobj = new ClientObject();
    sObjects.push_back(obj);

    sf::err() << sObjects.size() << " objects to draw\n";

    lua_getfield(L, 2, "shader");
    if (!lua_isnoneornil(L, -1)) {
        const char *name = luaL_checkstring(L, -1);
        sf::Shader *shader = sShaderCache.acquire(name);
        obj->setShader(shader);

        sf::err() << "Shader: " << shader << "\n";
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "material");
    if (!lua_isnoneornil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        MaterialInfo *material = new MaterialInfo();
        obj->setMaterial(material);

        lua_getfield(L, -1, "diffMap");
        if (!lua_isnoneornil(L, -1)) {
            const char *name = luaL_checkstring(L, -1);
            material->diffMap = sTextureCache.acquire(name);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "specMap");
        if (!lua_isnoneornil(L, -1)) {
            const char *name = luaL_checkstring(L, -1);
            material->specMap = sTextureCache.acquire(name);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "glowMap");
        if (!lua_isnoneornil(L, -1)) {
            const char *name = luaL_checkstring(L, -1);
            material->glowMap = sTextureCache.acquire(name);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "bumpMap");
        if (!lua_isnoneornil(L, -1)) {
            const char *name = luaL_checkstring(L, -1);
            material->bumpMap = sTextureCache.acquire(name);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "specPower");
        if (!lua_isnoneornil(L, -1)) {
            material->specPower = luaL_checknumber(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "bumpScale");
        if (!lua_isnoneornil(L, -1)) {
            material->bumpScale = luaL_checknumber(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "bumpBias");
        if (!lua_isnoneornil(L, -1)) {
            material->bumpBias = luaL_checknumber(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "fresnelPower");
        if (!lua_isnoneornil(L, -1)) {
            material->fresnelPower = luaL_checknumber(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "fresnelScale");
        if (!lua_isnoneornil(L, -1)) {
            material->fresnelScale = luaL_checknumber(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "fresnelBias");
        if (!lua_isnoneornil(L, -1)) {
            material->fresnelBias = luaL_checknumber(L, -1);
        }
        lua_pop(L, 1);

        sf::err() << "Material:\n"
            "\t diffMap == " << material->diffMap << "\n"
            "\t specMap == " << material->specMap << "\n"
            "\t glowMap == " << material->glowMap << "\n"
            "\t bumpMap == " << material->bumpMap << "\n"
            "\t specPower == " << material->specPower << "\n"
            "\t bumpScale == " << material->bumpScale << "\n"
            "\t bumpBias == " << material->bumpBias << "\n"
            "\t fresnelPower == " << material->fresnelPower << "\n"
            "\t fresnelScale == " << material->fresnelScale << "\n"
            "\t fresnelBias == " << material->fresnelBias << "\n"
            ;
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "model");
    if (!lua_isnoneornil(L, -1)) {

        static const char *const primitive_names[] = {
            "points",
            "lines",
            "lineloop",
            "linestrip",
            "triangles",
            "trianglestrip",
            "trianglefan",
            //~ "quads",
            //~ "quadstrip",
            //~ "polygon",
            nullptr
        };

        static const char *const shape_names[] = {
            "box",
            "plane",
            "sphere",
            nullptr
        };

        Model *model = new Model();
        obj->setModel(new ClientModel(model));

        lua_getfield(L, -1, "primitive");
        int primitive = luaL_checkoption(L, -1, "triangles", primitive_names);
        lua_pop(L, 1);

        lua_getfield(L, -1, "shape");
        int shape = luaL_checkoption(L, -1, "box", shape_names);
        lua_pop(L, 1);

        lua_getfield(L, -1, "radius");
        float radius = 1.0f;
        if (!lua_isnoneornil(L, -1)) {
            radius = luaL_checknumber(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "steps");
        int step = 5, rstep = 6;
        if (!lua_isnoneornil(L, -1)) {
            if (lua_type(L, -1) == LUA_TTABLE) {
                lua_rawgeti(L, -1, 1);
                lua_rawgeti(L, -2, 2);
                step = luaL_checkinteger(L, -2);
                rstep = luaL_checkinteger(L, -1);
                lua_pop(L, 2);
            } else {
                step = rstep = luaL_checkinteger(L, -1);
            }
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "size");
        sf::Vector3f size(1.0f, 1.0f, 1.0f);
        if (!lua_isnoneornil(L, -1)) {
            lua_rawgeti(L, -1, 1);
            lua_rawgeti(L, -2, 2);
            lua_rawgeti(L, -3, 3);
            size.x = luaL_checknumber(L, -3);
            size.y = luaL_checknumber(L, -2);
            size.z = luaL_checknumber(L, -1);
            lua_pop(L, 3);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "center");
        sf::Vector3f center;
        if (!lua_isnoneornil(L, -1)) {
            lua_rawgeti(L, -1, 1);
            lua_rawgeti(L, -2, 2);
            lua_rawgeti(L, -3, 3);
            center.x = luaL_checknumber(L, -3);
            center.y = luaL_checknumber(L, -2);
            center.z = luaL_checknumber(L, -1);
            lua_pop(L, 3);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "a");
        sf::Vector3f a(1.0f, 1.0f, 1.0f);
        if (!lua_isnoneornil(L, -1)) {
            lua_rawgeti(L, -1, 1);
            lua_rawgeti(L, -2, 2);
            lua_rawgeti(L, -3, 3);
            a.x = luaL_checknumber(L, -3);
            a.y = luaL_checknumber(L, -2);
            a.z = luaL_checknumber(L, -1);
            lua_pop(L, 3);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "b");
        sf::Vector3f b(1.0f, 1.0f, 1.0f);
        if (!lua_isnoneornil(L, -1)) {
            lua_rawgeti(L, -1, 1);
            lua_rawgeti(L, -2, 2);
            lua_rawgeti(L, -3, 3);
            b.x = luaL_checknumber(L, -3);
            b.y = luaL_checknumber(L, -2);
            b.z = luaL_checknumber(L, -1);
            lua_pop(L, 3);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "c");
        sf::Vector3f c(1.0f, 1.0f, 1.0f);
        if (!lua_isnoneornil(L, -1)) {
            lua_rawgeti(L, -1, 1);
            lua_rawgeti(L, -2, 2);
            lua_rawgeti(L, -3, 3);
            c.x = luaL_checknumber(L, -3);
            c.y = luaL_checknumber(L, -2);
            c.z = luaL_checknumber(L, -1);
            lua_pop(L, 3);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "texRect");
        sf::FloatRect texRect(0, 0, 1, 1);
        if (!lua_isnoneornil(L, -1)) {
            lua_rawgeti(L, -1, 1);
            lua_rawgeti(L, -2, 2);
            lua_rawgeti(L, -3, 3);
            lua_rawgeti(L, -4, 4);
            texRect.left   = luaL_checknumber(L, -4);
            texRect.top    = luaL_checknumber(L, -3);
            texRect.width  = luaL_checknumber(L, -2);
            texRect.height = luaL_checknumber(L, -1);
            lua_pop(L, 4);
        }
        lua_pop(L, 1);

        model->setPrimitive(primitive);

        switch (shape) {
            case 0: {
                sf::err() << "box(<" <<
                    size.x << ',' << size.y << ',' << size.z << ">, <" <<
                    center.x << ',' << center.y << ',' << center.z << ">, [" <<
                    texRect.left << ',' << texRect.top << ',' <<
                    texRect.width << ',' << texRect.height << "])\n";
                model->makeBox(size, center, texRect);
                break;
            }

            case 1: {
                sf::err() << "plane(<...>)\n";
                model->makePlane(a, b, c, texRect);
                break;
            }

            case 2: {
                sf::err() << "sphere(<...>)\n";
                model->makeBall(radius, step, rstep, center /*, texRect*/);
                break;
            }

            default: {
                sf::err() << "unknown shape " << shape << "???\n";
                break;
            }
        }

    }
    lua_pop(L, 1);
    return 1;
}

static luaL_Reg ll_Object_methods[] = {
    { "__new", ll_Object___new },
    { nullptr, nullptr }
};

int luaopen_Object(lua_State *L) {
    luaL_newmetatable(L, "Object");
    luaL_register(L, nullptr, ll_Object_methods);
    lua_newtable(L);
    lua_getfield(L, -2, "__new");
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////

class GameWindow : protected sf::RenderWindow {
    bool mFullscreen;
    bool mMouseLocked;
    bool mAllowQuit;
    bool mQuitting;
    bool mPaused;

    enum class ViewMode {
        Normal,
        InsideWireframe,
        Wireframe,
    };
    ViewMode mViewMode;

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

    sf::Vector3f mLightPos;
    float mSpinAngle;
    float mSpinSpeed; // degrees/second

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

    bool initScene();

    void handleEvents();

    void handleInput(const sf::Time &delta);
    void handleEvent(const sf::Event &event);
    void update(const sf::Time &delta);
    void render();

    void start3D();
    void end3D();
    void start2D();
    void end2D();

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
    ViewMode::Normal
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

    mFont.loadFromFile("data/fonts/VeraMono.ttf");
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

    if (!initScene()) {
        return false;
    }

    if (hasFocus()) {
        lockMouse();
    }

    return true;
}

bool GameWindow::initScene() {
    lua_State *L = luaL_newstate();

    if (!L) {
        return false;
    }

    luaL_openlibs(L);

    lua_pushcfunction(L, luaopen_Object);
    lua_pushliteral(L, "Object");

    if (lua_pcall(L, 1, 1, 0)) {
        const char *msg = lua_tostring(L, -1);
        if (msg) {
            sf::err() << "Lua error: " << msg << std::endl;
        }
        lua_close(L);
        return false;
    } else {
        if (lua_isnoneornil(L, -1)) {
            lua_pop(L, 1);
        } else {
            lua_setglobal(L, "Object");
        }
    }

    if (luaL_loadfile(L, "data/scripts/main.lua") || lua_pcall(L, 0, 0, 0)) {
        const char *msg = lua_tostring(L, -1);
        if (msg) {
            sf::err() << "Lua error: " << msg << std::endl;
        }
        lua_close(L);
        return false;
    }

    lua_close(L);

    return true;
}

void GameWindow::quit(bool internal) {
    if (internal || mAllowQuit) {
        mQuitting = true;
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
            sf::err() << "Window resized to " << size.x << 'x' << size.y << '\n';
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
                    switch (mViewMode) {
                        case ViewMode::Normal: {
                            mViewMode = ViewMode::InsideWireframe;
                            break;
                        }

                        case ViewMode::InsideWireframe: {
                            mViewMode = ViewMode::Wireframe;
                            break;
                        }

                        default:
                        case ViewMode::Wireframe: {
                            mViewMode = ViewMode::Normal;
                            break;
                        }
                    }
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

                    mFullscreen = !mFullscreen;

                    if (wasLocked) {
                        lockMouse();
                    }

                    break;
                }

                case sf::Keyboard::R: {
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
                        sTextureCache.reloadAll();
                    } else if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
                        sShaderCache.reloadAll();
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

        float deltaSeconds = delta.asSeconds();

        sf::Vector2f look;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
            look.x -= 180*deltaSeconds;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
            look.x += 180*deltaSeconds;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
            look.y -= 180*deltaSeconds;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
            look.y += 180*deltaSeconds;
        }

        sf::Vector3f move;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            move.z -= 2*deltaSeconds;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            move.z += 2*deltaSeconds;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            move.x -= 2*deltaSeconds;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            move.x += 2*deltaSeconds;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Z)) {
            move.y -= 2*deltaSeconds;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Q)) {
            move.y += 2*deltaSeconds;
        }

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
            look *= 0.25f;
            move *= 0.25f;
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::LControl)) {
            look *= 4.0f;
            move *= 4.0f;
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

        mPlayer.look(look);
        mPlayer.move(move);
    }
}

void GameWindow::update(const sf::Time &delta) {
    mPlayTime += delta;

    if (not mPaused) {
        mSpinAngle += delta.asSeconds() * mSpinSpeed;
    }
}

void GameWindow::start3D() {
    GLChecked(glEnable(GL_DEPTH_TEST));

    switch (mViewMode) {
        default:
        case ViewMode::Normal: {
            GLChecked(glEnable(GL_CULL_FACE));
            break;
        }

        case ViewMode::InsideWireframe: {
            GLChecked(glPolygonMode(GL_BACK, GL_LINE));
            break;
        }

        case ViewMode::Wireframe: {
            GLChecked(glPolygonMode(GL_FRONT_AND_BACK, GL_LINE));
            break;
        }
    }
}

void GameWindow::end3D() {
    // nothing to do here
}

void GameWindow::start2D() {
    GLChecked(glDisable(GL_DEPTH_TEST));
    GLChecked(glDisable(GL_CULL_FACE));
    GLChecked(glPolygonMode(GL_FRONT_AND_BACK, GL_FILL));
    GLChecked(glBindBuffer(GL_ARRAY_BUFFER, 0));

    GLChecked(pushGLStates());
}

void GameWindow::end2D() {
    GLChecked(popGLStates());
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

    GLChecked(glClearColor(1,0,1,0));
    GLChecked(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

    ////////////////////////////////////////////////////////////
    //  3D setup
    ////////////////////////////////////////////////////////////

    start3D();

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

    sf::Transform3D spinLight;
    spinLight.rotate(mSpinAngle, sf::Vector3f(0,1,0));
    sf::Vector3f spinLightPos = spinLight.transformPoint(mLightPos);

    static const sf::Color lightAmbt(25,25,25);
    static const sf::Color lightDiff(230,230,230);
    static const sf::Color lightSpec(255,255,255);

    for (ClientObject *obj : sObjects) {
        sf::Shader *shader = obj->getShader();
        if (shader) {
            shader->setParameter("uProjMatrix", projectionTransform);
            shader->setParameter("uViewMatrix", modelViewTransform);
            shader->setParameter("uLights[0].position", modelViewTransform * spinLightPos);
            shader->setParameter("uLights[0].ambtColor", lightAmbt);
            shader->setParameter("uLights[0].diffColor", lightDiff);
            shader->setParameter("uLights[0].specColor", lightSpec);
        }
        obj->render();
    }

    end3D();

    ////////////////////////////////////////////////////////////
    //  end 3D
    ////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////
    //  2D setup
    ////////////////////////////////////////////////////////////

    start2D();

    // draw 2D overlay

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

    end2D();

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

