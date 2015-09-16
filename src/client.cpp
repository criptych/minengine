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

static const float PI = 3.14159265358;

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

////////////////////////////////////////////////////////////////////////////////

class Transform3D : public sf::Transform {
public:
    Transform3D(): Transform() {
    }

    Transform3D(
        float a00, float a01, float a02, float a03,
        float a10, float a11, float a12, float a13,
        float a20, float a21, float a22, float a23,
        float a30, float a31, float a32, float a33
    ) {
        float *m = const_cast<float*>(getMatrix());
        m[ 0] = a00; m[ 4] = a01; m[ 8] = a02; m[12] = a03;
        m[ 1] = a10; m[ 5] = a11; m[ 9] = a12; m[13] = a13;
        m[ 2] = a20; m[ 6] = a21; m[10] = a22; m[14] = a23;
        m[ 3] = a30; m[ 7] = a31; m[11] = a32; m[15] = a33;
    }

    Transform3D &frustum(float left, float right, float bottom, float top, float near, float far) {
        Transform3D t(
            (2 * near) / (right - left), 0.0f, (right + left) / (right - left), 0.0f,
            0.0f, (2 * near) / (top - bottom), (top + bottom) / (top - bottom), 0.0f,
            0.0f, 0.0f, (near + far) / (near - far), (2 * near * far) / (near - far),
            0.0f, 0.0f, -1.0f, 0.0f
        );
        combine(t);
        return *this;
    }

    Transform3D &perspective(float fov, float aspect, float near, float far) {
        float fh = std::tan(fov*PI/360.0)*near;
        float fw = fh * aspect;
        return frustum(-fw, fw, -fh, fh, near, far);
    }

    Transform3D &translate(const sf::Vector3f &offset) {
        Transform3D translation(1, 0, 0, offset.x,
                                0, 1, 0, offset.y,
                                0, 0, 1, offset.z,
                                0, 0, 0, 1);

        combine(translation);
        return *this;
    }
};

////////////////////////////////////////////////////////////////////////////////

class CameraRenderer {
    float mFOV;
    float mAspect;
    float mZNear;
    float mZFar;
    sf::Vector3f mPosition;

    mutable Transform3D mTransform;
    mutable bool mNeedsUpdate;

public:
    CameraRenderer(
    ): mFOV(75.0), mAspect(1.0), mZNear(0.1), mZFar(100.0), mPosition(), mNeedsUpdate(true) {
    }

    CameraRenderer(
        float fov,
        float aspect,
        float zNear,
        float zFar
    ): mFOV(fov), mAspect(aspect), mZNear(zNear), mZFar(zFar), mPosition(), mNeedsUpdate(true) {
    }

    ~CameraRenderer() {
    }

    void setFOV(float fov) {
        mFOV = fov;
        mNeedsUpdate = true;
    }

    void setAspect(float aspect) {
        mAspect = aspect;
        mNeedsUpdate = true;
    }

    void setZNear(float zNear) {
        mZNear = zNear;
        mNeedsUpdate = true;
    }

    void setZFar(float zFar) {
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

    void update() const {
        if (mNeedsUpdate) {
            mTransform = Transform3D();
            mTransform.perspective(mFOV, mAspect, mZNear, mZFar);
            mTransform.translate(-sf::Vector3f(mPosition));
            mNeedsUpdate = false;
        }
    }

    const Transform3D &getProjectionMatrix() const {
        update();
        return mTransform;
    }

    void render() const {
        glMatrixMode(GL_PROJECTION);
        glLoadMatrixf(getProjectionMatrix().getMatrix());
        glMatrixMode(GL_MODELVIEW);
    }
};

class ModelRenderer {
    const Model *mModel;
    const sf::Shader *mShader;
    mutable GLuint mVBO;

public:
    ModelRenderer(
        const Model *model = nullptr,
        const sf::Shader *shader = nullptr
    ): mModel(model), mShader(shader), mVBO() {
    }

    ~ModelRenderer() {
        destroyVBO();
    }

    void setModel(const Model *model) {
        destroyVBO();
        mModel = model;
    }

    void setShader(const sf::Shader *shader) {
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
                sf::Shader::bind(mShader);
            }

            GLChecked(glBindBuffer(GL_ARRAY_BUFFER, mVBO));

#define SizeAndOffset(T, F) sizeof(T), reinterpret_cast<void*>(offsetof(T, F))

            GLChecked(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, SizeAndOffset(Vertex, x)));
            GLChecked(glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE, SizeAndOffset(Vertex, u)));
            GLChecked(glVertexAttribPointer(2, 2, GL_SHORT, GL_FALSE, SizeAndOffset(Vertex, s)));
            GLChecked(glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, SizeAndOffset(Vertex, r)));

            GLChecked(glEnableVertexAttribArray(0));
            GLChecked(glEnableVertexAttribArray(1));
            GLChecked(glEnableVertexAttribArray(2));
            GLChecked(glEnableVertexAttribArray(3));

            GLChecked(glVertexPointer(3, GL_FLOAT, SizeAndOffset(Vertex, x)));
            GLChecked(glNormalPointer(GL_FLOAT, SizeAndOffset(Vertex, u)));
            GLChecked(glTexCoordPointer(2, GL_SHORT, SizeAndOffset(Vertex, s)));
            GLChecked(glColorPointer(4, GL_UNSIGNED_BYTE, SizeAndOffset(Vertex, r)));

            GLChecked(glEnableClientState(GL_VERTEX_ARRAY));
            GLChecked(glEnableClientState(GL_NORMAL_ARRAY));
            GLChecked(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
            GLChecked(glEnableClientState(GL_COLOR_ARRAY));

            GLChecked(glDrawArrays(mModel->getPrimitive(), 0,
                mModel->getVertices().size()));

            GLChecked(glBindBuffer(GL_ARRAY_BUFFER, 0));

            if (mShader) {
                sf::Shader::bind(nullptr);
            }
        }
    }

private:
    void createVBO() const {
        if (mModel && !mVBO) {
            GLChecked(glGenBuffers(1, &mVBO));

            if (mVBO) {
                GLChecked(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
                GLChecked(glBufferData(GL_ARRAY_BUFFER,
                    sizeof(Vertex) * mModel->getVertices().size(),
                    &mModel->getVertices()[0], GL_STATIC_DRAW));
                GLChecked(glBindBuffer(GL_ARRAY_BUFFER, 0));
            }
        }
    }

    void destroyVBO() {
        if (mVBO) {
            GLChecked(glDeleteBuffers(1, &mVBO));
            mVBO = 0;
        }
    }
};

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

    CameraRenderer camera; //(90.0, 16.0/9.0, 0.1, 100.0);
    camera.setFOV(90.0);
    camera.setAspect(16.0/9.0);
    camera.setZNear(0.1);
    camera.setZFar(100.0);
    camera.setPosition(0.0, 0.0, 5.0);
    camera.render();

    glLoadIdentity();

    sf::Shader shader;
    shader.loadFromFile("shaders/default.330.vert", "shaders/default.330.frag");
    shader.setParameter("uViewMatrix", sf::Transform::Identity);
    shader.setParameter("uProjMatrix", camera.getProjectionMatrix());

    Model cubeModel(GL_QUADS);
    cubeModel.addVertex(Vertex(0xff,0xff,0xff,0xff, 0xffff,0x0000, +1.0, 0.0, 0.0, +1.0,+1.0,-1.0));
    cubeModel.addVertex(Vertex(0xff,0xff,0xff,0xff, 0xffff,0x0000, +1.0, 0.0, 0.0, +1.0,+1.0,+1.0));
    cubeModel.addVertex(Vertex(0xff,0xff,0xff,0xff, 0x0000,0x0000, +1.0, 0.0, 0.0, +1.0,-1.0,+1.0));
    cubeModel.addVertex(Vertex(0xff,0xff,0xff,0xff, 0x0000,0x0000, +1.0, 0.0, 0.0, +1.0,-1.0,-1.0));
    cubeModel.addVertex(Vertex(0xff,0x00,0xff,0xff, 0x0000,0x0000, -1.0, 0.0, 0.0, -1.0,-1.0,-1.0));
    cubeModel.addVertex(Vertex(0xff,0x00,0xff,0xff, 0x0000,0x0000, -1.0, 0.0, 0.0, -1.0,-1.0,+1.0));
    cubeModel.addVertex(Vertex(0xff,0x00,0xff,0xff, 0xffff,0x0000, -1.0, 0.0, 0.0, -1.0,+1.0,+1.0));
    cubeModel.addVertex(Vertex(0xff,0x00,0xff,0xff, 0xffff,0x0000, -1.0, 0.0, 0.0, -1.0,+1.0,-1.0));
    cubeModel.addVertex(Vertex(0xff,0xff,0x00,0xff, 0x0000,0x0000,  0.0,+1.0, 0.0, -1.0,+1.0,-1.0));
    cubeModel.addVertex(Vertex(0xff,0xff,0x00,0xff, 0x0000,0x0000,  0.0,+1.0, 0.0, -1.0,+1.0,+1.0));
    cubeModel.addVertex(Vertex(0xff,0xff,0x00,0xff, 0xffff,0x0000,  0.0,+1.0, 0.0, +1.0,+1.0,+1.0));
    cubeModel.addVertex(Vertex(0xff,0xff,0x00,0xff, 0xffff,0x0000,  0.0,+1.0, 0.0, +1.0,+1.0,-1.0));
    cubeModel.addVertex(Vertex(0x00,0xff,0xff,0xff, 0xffff,0x0000,  0.0,-1.0, 0.0, +1.0,-1.0,-1.0));
    cubeModel.addVertex(Vertex(0x00,0xff,0xff,0xff, 0xffff,0x0000,  0.0,-1.0, 0.0, +1.0,-1.0,+1.0));
    cubeModel.addVertex(Vertex(0x00,0xff,0xff,0xff, 0x0000,0x0000,  0.0,-1.0, 0.0, -1.0,-1.0,+1.0));
    cubeModel.addVertex(Vertex(0x00,0xff,0xff,0xff, 0x0000,0x0000,  0.0,-1.0, 0.0, -1.0,-1.0,-1.0));
    cubeModel.addVertex(Vertex(0xff,0x00,0x00,0xff, 0xffff,0x0000,  0.0, 0.0,+1.0, +1.0,-1.0,+1.0));
    cubeModel.addVertex(Vertex(0xff,0x00,0x00,0xff, 0xffff,0x0000,  0.0, 0.0,+1.0, +1.0,+1.0,+1.0));
    cubeModel.addVertex(Vertex(0xff,0x00,0x00,0xff, 0x0000,0x0000,  0.0, 0.0,+1.0, -1.0,+1.0,+1.0));
    cubeModel.addVertex(Vertex(0xff,0x00,0x00,0xff, 0x0000,0x0000,  0.0, 0.0,+1.0, -1.0,-1.0,+1.0));
    cubeModel.addVertex(Vertex(0x00,0xff,0x00,0xff, 0x0000,0x0000,  0.0, 0.0,-1.0, -1.0,-1.0,-1.0));
    cubeModel.addVertex(Vertex(0x00,0xff,0x00,0xff, 0x0000,0x0000,  0.0, 0.0,-1.0, -1.0,+1.0,-1.0));
    cubeModel.addVertex(Vertex(0x00,0xff,0x00,0xff, 0xffff,0x0000,  0.0, 0.0,-1.0, +1.0,+1.0,-1.0));
    cubeModel.addVertex(Vertex(0x00,0xff,0x00,0xff, 0xffff,0x0000,  0.0, 0.0,-1.0, +1.0,-1.0,-1.0));

    ModelRenderer cube(&cubeModel, &shader);

    int spin = 0, spinSpeed = 32; // degrees/second

    GLChecked(glEnable(GL_DEPTH_TEST));
    GLChecked(glDepthMask(GL_TRUE));
    GLChecked(glDepthFunc(GL_LESS));

    //~ GLChecked(glEnable(GL_CULL_FACE));

    GLChecked(glEnable(GL_COLOR_MATERIAL));
    GLChecked(glEnable(GL_LIGHTING));
    GLChecked(glEnable(GL_LIGHT0));

    const sf::Time tickLength(sf::microseconds(50000));
    const unsigned int maxFrameTicks = 5;

    sf::Clock clock;
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

        unsigned int frameTicks = maxFrameTicks;

        while (tickCount >= tickLength) {
            tickCount -= tickLength;
            if (frameTicks > 0) {
                //~ engine.tick();

                /*
                 * update blocks, entities, etc.
                 */

                spin += tickLength.asSeconds() * spinSpeed;
                frameTicks -= 1;
            }
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
        glRotated(std::sin(spin*PI/128.0)*30.0, 1.0,0.0,0.0);
        glRotated(spin*180.0/128.0, 0.0,1.0,0.0);
        cube.render();
        glPopMatrix();

        window.display();
    }
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

