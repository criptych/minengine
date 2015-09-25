////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "engine/engine.hpp"

#include <GL/glew.h>
#include <GL/glu.h>

#include <SFML/System.hpp>
#include <SFML/Window.hpp>
//~ #include <SFML/Graphics.hpp>
#include <SFML/Graphics/Shader.hpp>
#include <SFML/OpenGL.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Network.hpp>

#include <cstdio>
#include <cmath>

#include "GLCheck.hpp"

////////////////////////////////////////////////////////////////////////////////

#include "Transform3D.hpp"

////////////////////////////////////////////////////////////////////////////////

class Transformable3D {
    sf::Vector3f mPosition;
    sf::Vector3f mRotation;
    mutable Transform3D mTransform;
    mutable Transform3D mInverseTransform;
    mutable bool mNeedsUpdate;
    mutable bool mInverseNeedsUpdate;

public:
    Transformable3D(): mNeedsUpdate(true) {
    }

    const Transform3D &getTransform() const {
        if (mNeedsUpdate) {
            mTransform = Transform3D();

            mNeedsUpdate = false;
        }
        return mTransform;
    }

    const Transform3D &getInverseTransform() const {
        if (mInverseNeedsUpdate) {
            mInverseTransform = mTransform.getInverse();

            mInverseNeedsUpdate = false;
        }
        return mInverseTransform;
    }
};

////////////////////////////////////////////////////////////////////////////////

#include "Shader.hpp"

////////////////////////////////////////////////////////////////////////////////

class Camera {
    float mFOV;
    float mAspect;
    float mZNear;
    float mZFar;
    sf::Vector3f mPosition;
    sf::Vector2f mLook;

    mutable Transform3D mTransform;
    mutable bool mNeedsUpdate;

public:
    Camera(
    ): mFOV(75.0), mAspect(1.0), mZNear(0.1), mZFar(100.0), mNeedsUpdate(true) {
    }

    Camera(
        float fov,
        float aspect,
        float zNear,
        float zFar
    ): mFOV(fov), mAspect(aspect), mZNear(zNear), mZFar(zFar), mNeedsUpdate(true) {
    }

    ~Camera() {
    }

    void setFOV(float fov) {
        mFOV = fov;
        mNeedsUpdate = true;
    }

    float getFOV() const {
        return mFOV;
    }

    void setAspect(float aspect) {
        mAspect = aspect;
        mNeedsUpdate = true;
    }

    float getAspect() const {
        return mAspect;
    }

    void setZNear(float zNear) {
        mZNear = zNear;
        mNeedsUpdate = true;
    }

    float getZNear() const {
        return mZNear;
    }

    void setZFar(float zFar) {
        mZFar = zFar;
        mNeedsUpdate = true;
    }

    float getZFar() const {
        return mZFar;
    }

    void setZRange(float zNear, float zFar) {
        mZNear = zNear;
        mZFar = zFar;
        mNeedsUpdate = true;
    }

    void setPosition(const sf::Vector3f &position) {
        mPosition = position;
        mNeedsUpdate = true;
    }

    void setPosition(float x, float y, float z) {
        setPosition(sf::Vector3f(x, y, z));
    }

    const sf::Vector3f &getPosition() const {
        return mPosition;
    }

    void move(const sf::Vector3f &offset) {
        setPosition(mPosition.x + offset.x,
                    mPosition.y + offset.y,
                    mPosition.z + offset.z);
    }

    void move(const sf::Vector3f &offset, float angle) {
        move(Transform3D().rotate(angle, sf::Vector3f(0, 1, 0)).transformPoint(offset));
    }

    void move(float dx, float dy, float dz) {
        move(sf::Vector3f(dx, dy, dz));
    }

    void setLook(const sf::Vector2f &look) {
        mLook = look;
        mNeedsUpdate = true;
    }

    const sf::Vector2f &getLook() const {
        return mLook;
    }

    const Transform3D &getTransform() const {
        if (mNeedsUpdate) {
            mTransform = Transform3D();
            mTransform.perspective(mFOV, mAspect, mZNear, mZFar);
            mTransform.rotate(mLook.y, sf::Vector3f(1.0f, 0.0f, 0.0f));
            mTransform.rotate(mLook.x, sf::Vector3f(0.0f, 1.0f, 0.0f));
            mTransform.translate(-sf::Vector3f(mPosition));
            mNeedsUpdate = false;
        }
        return mTransform;
    }

    void render() const {
        GLChecked(glMatrixMode(GL_PROJECTION));
        GLChecked(glLoadMatrixf(getTransform().getMatrix()));
        GLChecked(glMatrixMode(GL_MODELVIEW));
    }
};

////////////////////////////////////////////////////////////////////////////////

class ModelRenderer {
    const Model *mModel;
    const Shader *mShader;
    mutable GLuint mVBO;
    mutable GLenum mPrimitive;
    mutable GLuint mCount;

public:
    ModelRenderer(
        const Model *model = nullptr,
        const Shader *shader = nullptr
    ): mModel(model), mShader(shader), mVBO(), mPrimitive(), mCount() {
    }

    ~ModelRenderer() {
        destroyVBO();
    }

    void setModel(const Model *model) {
        destroyVBO();
        mModel = model;
    }

    void setShader(const Shader *shader) {
        mShader = shader;
    }

    void render() const {
        if (mModel) {
            if (mVBO == 0) {
                createVBO();
                if (mVBO == 0) {
                    return;
                }
            }

            if (mShader) {
                Shader::bind(mShader);
            }

            GLChecked(glBindBuffer(GL_ARRAY_BUFFER, mVBO));

#define SizeAndOffset(T, F) sizeof(T), reinterpret_cast<void*>(offsetof(T, F))

            GLChecked(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, SizeAndOffset(Vertex, position)));
            GLChecked(glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE,  SizeAndOffset(Vertex, normal)));
            GLChecked(glVertexAttribPointer(2, 2, GL_SHORT, GL_TRUE,  SizeAndOffset(Vertex, texCoord)));
            GLChecked(glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, SizeAndOffset(Vertex, color)));

            GLChecked(glEnableVertexAttribArray(0));
            GLChecked(glEnableVertexAttribArray(1));
            GLChecked(glEnableVertexAttribArray(2));
            GLChecked(glEnableVertexAttribArray(3));

            //~ GLChecked(glVertexPointer(3, GL_FLOAT, SizeAndOffset(Vertex, position)));
            //~ GLChecked(glNormalPointer(GL_FLOAT, SizeAndOffset(Vertex, normal)));
            //~ GLChecked(glTexCoordPointer(2, GL_SHORT, SizeAndOffset(Vertex, texCoord)));
            //~ GLChecked(glColorPointer(4, GL_UNSIGNED_BYTE, SizeAndOffset(Vertex, color)));

            //~ GLChecked(glEnableClientState(GL_VERTEX_ARRAY));
            //~ GLChecked(glEnableClientState(GL_NORMAL_ARRAY));
            //~ GLChecked(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
            //~ GLChecked(glEnableClientState(GL_COLOR_ARRAY));

            GLChecked(glDrawArrays(mPrimitive, 0, mCount));

            GLChecked(glBindBuffer(GL_ARRAY_BUFFER, 0));

            if (mShader) {
                Shader::bind(nullptr);
            }
        }
    }

private:
    void createVBO() const {
        if (mModel && !mVBO) {
            GLChecked(glGenBuffers(1, &mVBO));

            sf::err() << "glGenBuffers -> " << mVBO;

            mPrimitive = mModel->getPrimitive();
            mCount = mModel->getVertices().size();

            sf::err() << ", mPrimitive = " << mPrimitive;
            sf::err() << ", mCount = " << mCount;

            if (mCount && mVBO) {
                size_t size = sizeof(Vertex) * mCount;
                const void *data = mModel->getVertices().data();
                sf::err() << ", size == " << size;
                GLChecked(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
                GLChecked(glBufferData(GL_ARRAY_BUFFER, size, data,
                    GL_STATIC_DRAW));
                GLChecked(glBindBuffer(GL_ARRAY_BUFFER, 0));
            }

            sf::err() << std::endl;
        }
    }

    void destroyVBO() {
        if (mVBO) {
            GLChecked(glDeleteBuffers(1, &mVBO));
            mVBO = 0;
            mPrimitive = 0;
            mCount = 0;
        }
    }
};

////////////////////////////////////////////////////////////////////////////////

size_t makeBox(std::vector<Vertex> &verts, GLenum mode, const sf::Vector3f &center, const sf::Vector3f &size) {
    sf::Vector3f mx = center + size;
    sf::Vector3f mn = center - size;

    size_t i = 0;

    //~ verts[i++] = Vertex(0x7fff,0x0000, +1.0, 0.0, 0.0, mx.x,mx.y,mn.z);
    //~ verts[i++] = Vertex(0x7fff,0x7fff, +1.0, 0.0, 0.0, mx.x,mx.y,mx.z);
    //~ verts[i++] = Vertex(0x0000,0x7fff, +1.0, 0.0, 0.0, mx.x,mn.y,mx.z);
    //~ verts[i++] = Vertex(0x0000,0x0000, +1.0, 0.0, 0.0, mx.x,mn.y,mn.z);
    //~ verts[i++] = Vertex(0x0000,0x0000, -1.0, 0.0, 0.0, mn.x,mn.y,mn.z);
    //~ verts[i++] = Vertex(0x0000,0x7fff, -1.0, 0.0, 0.0, mn.x,mn.y,mx.z);
    //~ verts[i++] = Vertex(0x7fff,0x7fff, -1.0, 0.0, 0.0, mn.x,mx.y,mx.z);
    //~ verts[i++] = Vertex(0x7fff,0x0000, -1.0, 0.0, 0.0, mn.x,mx.y,mn.z);
    //~ verts[i++] = Vertex(0x0000,0x0000,  0.0,+1.0, 0.0, mn.x,mx.y,mn.z);
    //~ verts[i++] = Vertex(0x0000,0x7fff,  0.0,+1.0, 0.0, mn.x,mx.y,mx.z);
    //~ verts[i++] = Vertex(0x7fff,0x7fff,  0.0,+1.0, 0.0, mx.x,mx.y,mx.z);
    //~ verts[i++] = Vertex(0x7fff,0x0000,  0.0,+1.0, 0.0, mx.x,mx.y,mn.z);
    //~ verts[i++] = Vertex(0x7fff,0x0000,  0.0,-1.0, 0.0, mx.x,mn.y,mn.z);
    //~ verts[i++] = Vertex(0x7fff,0x7fff,  0.0,-1.0, 0.0, mx.x,mn.y,mx.z);
    //~ verts[i++] = Vertex(0x0000,0x7fff,  0.0,-1.0, 0.0, mn.x,mn.y,mx.z);
    //~ verts[i++] = Vertex(0x0000,0x0000,  0.0,-1.0, 0.0, mn.x,mn.y,mn.z);
    //~ verts[i++] = Vertex(0x7fff,0x0000,  0.0, 0.0,+1.0, mx.x,mn.y,mx.z);
    //~ verts[i++] = Vertex(0x7fff,0x7fff,  0.0, 0.0,+1.0, mx.x,mx.y,mx.z);
    //~ verts[i++] = Vertex(0x0000,0x7fff,  0.0, 0.0,+1.0, mn.x,mx.y,mx.z);
    //~ verts[i++] = Vertex(0x0000,0x0000,  0.0, 0.0,+1.0, mn.x,mn.y,mx.z);
    //~ verts[i++] = Vertex(0x0000,0x0000,  0.0, 0.0,-1.0, mn.x,mn.y,mn.z);
    //~ verts[i++] = Vertex(0x0000,0x7fff,  0.0, 0.0,-1.0, mn.x,mx.y,mn.z);
    //~ verts[i++] = Vertex(0x7fff,0x7fff,  0.0, 0.0,-1.0, mx.x,mx.y,mn.z);
    //~ verts[i++] = Vertex(0x7fff,0x0000,  0.0, 0.0,-1.0, mx.x,mn.y,mn.z);

    switch (mode) {
        case GL_TRIANGLES:
            verts.resize(36);

            verts[i++] = Vertex(0x7fff,0x0000, mx.x,mx.y,mn.z);
            verts[i++] = Vertex(0x7fff,0x7fff, mx.x,mx.y,mx.z);
            verts[i++] = Vertex(0x0000,0x7fff, mx.x,mn.y,mx.z);
            verts[i++] = Vertex(0x7fff,0x0000, mx.x,mx.y,mn.z);
            verts[i++] = Vertex(0x0000,0x7fff, mx.x,mn.y,mx.z);
            verts[i++] = Vertex(0x0000,0x0000, mx.x,mn.y,mn.z);

            verts[i++] = Vertex(0x0000,0x0000, mn.x,mn.y,mn.z);
            verts[i++] = Vertex(0x0000,0x7fff, mn.x,mn.y,mx.z);
            verts[i++] = Vertex(0x7fff,0x7fff, mn.x,mx.y,mx.z);
            verts[i++] = Vertex(0x0000,0x0000, mn.x,mn.y,mn.z);
            verts[i++] = Vertex(0x7fff,0x7fff, mn.x,mx.y,mx.z);
            verts[i++] = Vertex(0x7fff,0x0000, mn.x,mx.y,mn.z);

            verts[i++] = Vertex(0x0000,0x0000, mn.x,mx.y,mn.z);
            verts[i++] = Vertex(0x0000,0x7fff, mn.x,mx.y,mx.z);
            verts[i++] = Vertex(0x7fff,0x7fff, mx.x,mx.y,mx.z);
            verts[i++] = Vertex(0x0000,0x0000, mn.x,mx.y,mn.z);
            verts[i++] = Vertex(0x7fff,0x7fff, mx.x,mx.y,mx.z);
            verts[i++] = Vertex(0x7fff,0x0000, mx.x,mx.y,mn.z);

            verts[i++] = Vertex(0x7fff,0x0000, mx.x,mn.y,mn.z);
            verts[i++] = Vertex(0x7fff,0x7fff, mx.x,mn.y,mx.z);
            verts[i++] = Vertex(0x0000,0x7fff, mn.x,mn.y,mx.z);
            verts[i++] = Vertex(0x7fff,0x0000, mx.x,mn.y,mn.z);
            verts[i++] = Vertex(0x0000,0x7fff, mn.x,mn.y,mx.z);
            verts[i++] = Vertex(0x0000,0x0000, mn.x,mn.y,mn.z);

            verts[i++] = Vertex(0x7fff,0x0000, mx.x,mn.y,mx.z);
            verts[i++] = Vertex(0x7fff,0x7fff, mx.x,mx.y,mx.z);
            verts[i++] = Vertex(0x0000,0x7fff, mn.x,mx.y,mx.z);
            verts[i++] = Vertex(0x7fff,0x0000, mx.x,mn.y,mx.z);
            verts[i++] = Vertex(0x0000,0x7fff, mn.x,mx.y,mx.z);
            verts[i++] = Vertex(0x0000,0x0000, mn.x,mn.y,mx.z);

            verts[i++] = Vertex(0x0000,0x0000, mn.x,mn.y,mn.z);
            verts[i++] = Vertex(0x0000,0x7fff, mn.x,mx.y,mn.z);
            verts[i++] = Vertex(0x7fff,0x7fff, mx.x,mx.y,mn.z);
            verts[i++] = Vertex(0x0000,0x0000, mn.x,mn.y,mn.z);
            verts[i++] = Vertex(0x7fff,0x7fff, mx.x,mx.y,mn.z);
            verts[i++] = Vertex(0x7fff,0x0000, mx.x,mn.y,mn.z);
            break;

        case GL_QUADS:
            verts.resize(24);

            verts[i++] = Vertex(0x7fff,0x0000, mx.x,mx.y,mn.z);
            verts[i++] = Vertex(0x7fff,0x7fff, mx.x,mx.y,mx.z);
            verts[i++] = Vertex(0x0000,0x7fff, mx.x,mn.y,mx.z);
            verts[i++] = Vertex(0x0000,0x0000, mx.x,mn.y,mn.z);

            verts[i++] = Vertex(0x0000,0x0000, mn.x,mn.y,mn.z);
            verts[i++] = Vertex(0x0000,0x7fff, mn.x,mn.y,mx.z);
            verts[i++] = Vertex(0x7fff,0x7fff, mn.x,mx.y,mx.z);
            verts[i++] = Vertex(0x7fff,0x0000, mn.x,mx.y,mn.z);

            verts[i++] = Vertex(0x0000,0x0000, mn.x,mx.y,mn.z);
            verts[i++] = Vertex(0x0000,0x7fff, mn.x,mx.y,mx.z);
            verts[i++] = Vertex(0x7fff,0x7fff, mx.x,mx.y,mx.z);
            verts[i++] = Vertex(0x7fff,0x0000, mx.x,mx.y,mn.z);

            verts[i++] = Vertex(0x7fff,0x0000, mx.x,mn.y,mn.z);
            verts[i++] = Vertex(0x7fff,0x7fff, mx.x,mn.y,mx.z);
            verts[i++] = Vertex(0x0000,0x7fff, mn.x,mn.y,mx.z);
            verts[i++] = Vertex(0x0000,0x0000, mn.x,mn.y,mn.z);

            verts[i++] = Vertex(0x7fff,0x0000, mx.x,mn.y,mx.z);
            verts[i++] = Vertex(0x7fff,0x7fff, mx.x,mx.y,mx.z);
            verts[i++] = Vertex(0x0000,0x7fff, mn.x,mx.y,mx.z);
            verts[i++] = Vertex(0x0000,0x0000, mn.x,mn.y,mx.z);

            verts[i++] = Vertex(0x0000,0x0000, mn.x,mn.y,mn.z);
            verts[i++] = Vertex(0x0000,0x7fff, mn.x,mx.y,mn.z);
            verts[i++] = Vertex(0x7fff,0x7fff, mx.x,mx.y,mn.z);
            verts[i++] = Vertex(0x7fff,0x0000, mx.x,mn.y,mn.z);
            break;
    }

    return i;
}

size_t makeBox(std::vector<Vertex> &verts, GLenum mode) {
    return makeBox(verts, mode, sf::Vector3f(0,0,0), sf::Vector3f(1,1,1));
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

    bool fullscreen = false;
    sf::VideoMode videoMode(960, 540);
    sf::String windowTitle(L"MinEngine Client");
    sf::Uint32 windowStyle(sf::Style::Default);
    sf::ContextSettings contextSettings(24,8, 0, 3,3);

    sf::Window window(videoMode, L"MinEngine Client", windowStyle, contextSettings);

    sf::VideoMode desktopMode(sf::VideoMode::getDesktopMode());

    std::fprintf(stderr, "Desktop mode: %dx%d %dbpp\n",
                 desktopMode.width, desktopMode.height, desktopMode.bitsPerPixel);

    glewInit();

    Camera camera(90.0f, 16.0f/9.0f, 0.1f, 100.0f);
    camera.setPosition(0.0, 1.7, 5.0);
    //~ camera.render();

    Shader shader;

    if (!
        shader.loadFromFile("shaders/default.330.vert", "shaders/default.330.frag")
    ) {
        return -1;
    }

    shader.bindAttribLocation("aVertex",   0);
    shader.bindAttribLocation("aNormal",   1);
    shader.bindAttribLocation("aTexCoord", 2);
    shader.bindAttribLocation("aColor",    3);

    shader.setParameter("uResolution", sf::Vector2f(window.getSize()));

    std::vector<Vertex> cubeVerts;
    makeBox(cubeVerts, GL_TRIANGLES, sf::Vector3f(0.5,0.5,0.5), sf::Vector3f(0.5,0.5,0.5));

    Model cubeModel(GL_TRIANGLES, cubeVerts);

    cubeModel.calcNormals();

    ModelRenderer cube(&cubeModel, &shader);

    float spin = 0, spinSpeed = 45; // degrees/second

    sf::Vector2i lastMousePos = sf::Mouse::getPosition(window);
    sf::Vector2f look;

    GLChecked(glEnable(GL_DEPTH_TEST));
    //~ GLChecked(glDepthMask(GL_TRUE));
    GLChecked(glDepthFunc(GL_LESS));

    GLChecked(glEnable(GL_CULL_FACE));
    //~ GLChecked(glEnable(GL_COLOR_MATERIAL));
    //~ GLChecked(glEnable(GL_LIGHTING));
    //~ GLChecked(glEnable(GL_LIGHT0));

    GLChecked(glClearColor(0.200,0.267,0.333,0.0));

    const sf::Time tickLength(sf::microseconds(50000)); // 20000
    const unsigned int maxFrameTicks = 5;

    sf::Clock clock;
    sf::Time tickCount;

    sf::Time playTime;

    bool paused = false;

    while (window.isOpen()) {
        sf::Event event;
        while (window.pollEvent(event)) {
            switch (event.type) {
                case sf::Event::Closed: {
                    window.close();
                    break;
                }

                case sf::Event::Resized: {
                    sf::Vector2f size(window.getSize());
                    shader.setParameter("uResolution", size);
                    GLChecked(glViewport(0, 0, size.x, size.y));
                    camera.setAspect(size.x / size.y);
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
                            window.close();
                            break;
                        }

                        case sf::Keyboard::Space: {
                            paused = not paused;
                            break;
                        }

                        case sf::Keyboard::F11: {
                            if (fullscreen) {
                                window.create(videoMode, windowTitle, windowStyle, contextSettings);
                            } else {
                                window.create(desktopMode, windowTitle, windowStyle | sf::Style::Fullscreen, contextSettings);
                            }

                            GLChecked(glClearColor(0.200,0.267,0.333,0.0));

                            fullscreen = !fullscreen;

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
                    sf::Vector2i mousePos(event.mouseMove.x, event.mouseMove.y);
                    sf::Vector2i mouseDelta = mousePos - lastMousePos;
                    look.x += mouseDelta.x;
                    look.y += mouseDelta.y;
                    lastMousePos = mousePos;
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
                                camera.move(event.joystickMove.position * 0.01f, 0, 0);
                                break;
                            }

                            case sf::Joystick::Y: {
                                camera.move(0, 0, event.joystickMove.position * 0.01f);
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

                            //~ default: {
                                //~ break;
                            //~ }
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
                    break;
                }

                case sf::Event::TouchMoved: {
                    break;
                }

                case sf::Event::TouchEnded: {
                    break;
                }

                case sf::Event::SensorChanged: {
                    break;
                }

                // silence "enumeration value not handled" warning
                //~ case sf::Event::Count: break;
                default: {
                    break;
                }
            }
        }

        sf::Time delta = clock.restart();
        playTime += delta;
        tickCount += delta;

        unsigned int frameTicks = maxFrameTicks;

        look.x = std::fmod(look.x + 180.0f, 360.0f) - 180.0f;

        if (look.y > 89.9f) {
            look.y = 89.9f;
        }
        if (look.y < -89.9f) {
            look.y = -89.9f;
        }

        sf::Vector3f move;

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            move.z -= delta.asSeconds();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            move.z += delta.asSeconds();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            move.x -= delta.asSeconds();
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            move.x += delta.asSeconds();
        }

        camera.setLook(look);
        camera.move(move, -look.x);

        while (tickCount >= tickLength) {
            tickCount -= tickLength;
            if (frameTicks > 0) {
                /*
                 * update blocks, entities, etc.
                 */

                //~ engine.tick();

                if (not paused) {
                    spin += tickLength.asSeconds() * spinSpeed;
                }

                frameTicks -= 1;
            }
        }

        /*
         * for each visible chunk:
         *      create vbo
         *      convert blocks to polys
         *      render vbo
         */

        //~ render(engine)

        GLChecked(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

        ////////////////////////////////////////////////////////////

        // 3D setup

        GLChecked(glEnable(GL_DEPTH_TEST));
        GLChecked(glEnable(GL_CULL_FACE));
        //~ GLChecked(glEnable(GL_LIGHTING));
        //~ GLChecked(glEnable(GL_LIGHT0));
        //~ GLChecked(glEnable(GL_COLOR_MATERIAL));

        // draw 3D scene

        //~ camera.render();

        Transform3D projectionTransform(camera.getTransform());

        shader.setParameter("uTime", playTime.asSeconds());
        shader.setParameter("uProjMatrix", projectionTransform);

        Transform3D modelViewTransform;

        //~ const float Pi = 3.14159265358;

        //~ modelViewTransform.rotate(std::sin(spin*Pi/360.0f)*30.0f,
                                  //~ sf::Vector3f(1.0f,0.0f,0.0f));
        //~ modelViewTransform.rotate(spin,
                                  //~ sf::Vector3f(0.0f,1.0f,0.0f));

        shader.setParameter("uViewMatrix", modelViewTransform);

        //~ GLChecked(glPushMatrix());
        //~ GLChecked(glLoadMatrixf(modelViewTransform.getMatrix()));
        cube.render();
        //~ GLChecked(glPopMatrix());

        modelViewTransform.translate(sf::Vector3f(1.5f,0.0f,0.0f));
        shader.setParameter("uViewMatrix", modelViewTransform);
        cube.render();

        // end 3D

        ////////////////////////////////////////////////////////////

        // 2D setup

        GLChecked(glDisable(GL_DEPTH_TEST));
        GLChecked(glDisable(GL_CULL_FACE));
        //~ GLChecked(glDisable(GL_LIGHTING));

        // draw 2D overlay


        // end 2D

        ////////////////////////////////////////////////////////////

        window.display();
    }

    return 0;
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

