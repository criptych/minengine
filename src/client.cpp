////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "engine.hpp"

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

#define GLCheck() _GLCheck(__FILE__, __LINE__)

#define GLChecked(f) (f,GLCheck())
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

    Transform3D(const sf::Transform &transform) {
        const float *a = transform.getMatrix();
        float *b = const_cast<float*>(getMatrix());

        b[ 0] = a[ 0]; b[ 4] = a[ 4]; b[ 8] = a[ 8]; b[12] = a[12];
        b[ 1] = a[ 1]; b[ 5] = a[ 5]; b[ 9] = a[ 9]; b[13] = a[13];
        b[ 2] = a[ 2]; b[ 6] = a[ 6]; b[10] = a[10]; b[14] = a[14];
        b[ 3] = a[ 3]; b[ 7] = a[ 7]; b[11] = a[11]; b[15] = a[15];
    }

    Transform3D& combine(const sf::Transform& transform) {
        const float *a = getMatrix();
        const float *b = transform.getMatrix();

        *this = Transform3D(a[ 0] * b[ 0] + a[ 4] * b[ 1] + a[ 8] * b[ 2] + a[12] * b[ 3],
                            a[ 0] * b[ 4] + a[ 4] * b[ 5] + a[ 8] * b[ 6] + a[12] * b[ 7],
                            a[ 0] * b[ 8] + a[ 4] * b[ 9] + a[ 8] * b[10] + a[12] * b[11],
                            a[ 0] * b[12] + a[ 4] * b[13] + a[ 8] * b[14] + a[12] * b[15],
                            a[ 1] * b[ 0] + a[ 5] * b[ 1] + a[ 9] * b[ 2] + a[13] * b[ 3],
                            a[ 1] * b[ 4] + a[ 5] * b[ 5] + a[ 9] * b[ 6] + a[13] * b[ 7],
                            a[ 1] * b[ 8] + a[ 5] * b[ 9] + a[ 9] * b[10] + a[13] * b[11],
                            a[ 1] * b[12] + a[ 5] * b[13] + a[ 9] * b[14] + a[13] * b[15],
                            a[ 2] * b[ 0] + a[ 6] * b[ 1] + a[10] * b[ 2] + a[14] * b[ 3],
                            a[ 2] * b[ 4] + a[ 6] * b[ 5] + a[10] * b[ 6] + a[14] * b[ 7],
                            a[ 2] * b[ 8] + a[ 6] * b[ 9] + a[10] * b[10] + a[14] * b[11],
                            a[ 2] * b[12] + a[ 6] * b[13] + a[10] * b[14] + a[14] * b[15],
                            a[ 3] * b[ 0] + a[ 7] * b[ 1] + a[11] * b[ 2] + a[15] * b[ 3],
                            a[ 3] * b[ 4] + a[ 7] * b[ 5] + a[11] * b[ 6] + a[15] * b[ 7],
                            a[ 3] * b[ 8] + a[ 7] * b[ 9] + a[11] * b[10] + a[15] * b[11],
                            a[ 3] * b[12] + a[ 7] * b[13] + a[11] * b[14] + a[15] * b[15]);

        return *this;
    }

    Transform3D &frustum(float left, float right, float bottom, float top, float near, float far) {
        Transform3D transform(
            (2 * near) / (right - left), 0.0f, (right + left) / (right - left), 0.0f,
            0.0f, (2 * near) / (top - bottom), (top + bottom) / (top - bottom), 0.0f,
            0.0f, 0.0f, (near + far) / (near - far), (2 * near * far) / (near - far),
            0.0f, 0.0f, -1.0f, 0.0f);

        return combine(transform);
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

        return combine(translation);
    }

    Transform3D &rotate(float angle, const sf::Vector3f &axis) {
        float s = std::sin(angle*PI/180.0f), c = std::cos(angle*PI/180.0f);
        float xx = axis.x * axis.x;
        float xy = axis.x * axis.y;
        float xz = axis.x * axis.z;
        float yy = axis.y * axis.y;
        float yz = axis.y * axis.z;
        float zz = axis.z * axis.z;
        float xs = axis.x * s;
        float ys = axis.y * s;
        float zs = axis.z * s;
        float mc = 1.0f - c;

        Transform3D rotation(xx*mc+ c, xy*mc-zs, xz*mc+ys, 0,
                             xy*mc+zs, yy*mc+ c, yz*mc-xs, 0,
                             xz*mc-ys, yz*mc+xs, zz*mc+ c, 0,
                             0,        0,        0,        1);

        return combine(rotation);
    }

    Transform3D getInverse() const {
        //! @todo
        return Transform3D();
    }
};

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

class Shader {
    mutable GLuint mProgram;
    mutable bool mNeedsUpdate;

    std::string mFragSource;
    std::string mGeomSource;
    std::string mVertSource;

    std::map<std::string, GLint> mUniformLocations;
    std::map<std::string, GLint> mAttribLocations;

public:
    enum class Type {
        Fragment = 1,
        Geometry = 2,
        Vertex   = 4,
    };

    Shader(): mProgram(), mNeedsUpdate() {
    }

    ~Shader() {
        if (mProgram) {
            GLChecked(glDeleteProgram(mProgram));
            mProgram = 0;
        }
    }

    bool loadFromFile(const std::string &vertSource, const std::string &fragSource) {
        sf::FileInputStream file;

        if (file.open(vertSource)) {
            sf::Int64 size = file.getSize();
            std::vector<char> source(size);
            file.read(source.data(), source.size());
            setVertexShaderSource(std::string(source.begin(), source.end()));
        } else {
            std::fprintf(stderr, "error opening \"%s\"\n", vertSource.c_str());
            return false;
        }

        if (file.open(fragSource)) {
            sf::Int64 size = file.getSize();
            std::vector<char> source(size);
            file.read(source.data(), source.size());
            setFragmentShaderSource(std::string(source.begin(), source.end()));
        } else {
            std::fprintf(stderr, "error opening \"%s\"\n", fragSource.c_str());
            return false;
        }

        return true;
    }

    void setFragmentShaderSource(const std::string &source) {
        mFragSource = source;
        mNeedsUpdate = true;
    }

    void setGeometryShaderSource(const std::string &source) {
        mGeomSource = source;
        mNeedsUpdate = true;
    }

    void setVertexShaderSource(const std::string &source) {
        mVertSource = source;
        mNeedsUpdate = true;
    }

    GLint getUniformLocation(const std::string &name) {
        GLint location = -1;
        auto i = mUniformLocations.find(name);
        if (i != mUniformLocations.end()) {
            location = i->second;
        } else {
            TempBinder binder(this);
            GLChecked(location = glGetUniformLocation(mProgram, name.c_str()));
            mUniformLocations[name] = location;
        }
        return location;
    }

    GLint getAttribLocation(const std::string &name) {
        GLint location = -1;
        auto i = mAttribLocations.find(name);
        if (i != mAttribLocations.end()) {
            location = i->second;
        } else {
            TempBinder binder(this);
            GLChecked(location = glGetAttribLocation(mProgram, name.c_str()));
            mAttribLocations[name] = location;
        }
        return location;
    }

    void bindAttribLocation(const std::string &name, GLint location) {
        mAttribLocations[name] = location;
        mNeedsUpdate = true;
    }

    void setParameter(GLint location, int value) {
        TempBinder binder(this);
        GLChecked(glUniform1i(location, value));
    }

    void setParameter(GLint location, const sf::Vector2i &value) {
        TempBinder binder(this);
        GLChecked(glUniform2i(location, value.x, value.y));
    }

    void setParameter(GLint location, const sf::Vector3i &value) {
        TempBinder binder(this);
        GLChecked(glUniform3i(location, value.x, value.y, value.z));
    }

    void setParameter(GLint location, float value) {
        TempBinder binder(this);
        GLChecked(glUniform1f(location, value));
    }

    void setParameter(GLint location, const sf::Vector2f &value) {
        TempBinder binder(this);
        GLChecked(glUniform2f(location, value.x, value.y));
    }

    void setParameter(GLint location, const sf::Vector3f &value) {
        TempBinder binder(this);
        GLChecked(glUniform3f(location, value.x, value.y, value.z));
    }

    void setParameter(GLint location, const sf::Transform &value) {
        TempBinder binder(this);
        GLChecked(glUniformMatrix4fv(location, 1, GL_FALSE, value.getMatrix()));
    }

    void setParameter(const std::string &name, int value) {
        return setParameter(getUniformLocation(name), value);
    }

    void setParameter(const std::string &name, const sf::Vector2i &value) {
        return setParameter(getUniformLocation(name), value);
    }

    void setParameter(const std::string &name, const sf::Vector3i &value) {
        return setParameter(getUniformLocation(name), value);
    }

    void setParameter(const std::string &name, float value) {
        return setParameter(getUniformLocation(name), value);
    }

    void setParameter(const std::string &name, const sf::Vector2f &value) {
        return setParameter(getUniformLocation(name), value);
    }

    void setParameter(const std::string &name, const sf::Vector3f &value) {
        return setParameter(getUniformLocation(name), value);
    }

    void setParameter(const std::string &name, const sf::Transform &value) {
        return setParameter(getUniformLocation(name), value);
    }

    GLuint getProgramID() const {
        return mProgram;
    }

protected:
    class ShaderCompileException {
    public:
        ShaderCompileException() {}
        ShaderCompileException(const std::string &msg) {}
    };

    bool compile(GLenum shaderType, const std::string &source) const {
        GLuint shader;
        GLChecked(shader = glCreateShader(shaderType));
        const GLchar *sources[] = { source.data() };
        GLint lengths[] = { static_cast<GLint>(source.size()) };
        GLChecked(glShaderSource(shader, 1, sources, lengths));
        GLChecked(glCompileShader(shader));

        GLint logLength = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &logLength);
        std::vector<char> log(logLength);
        glGetShaderInfoLog(shader, log.size(), nullptr, log.data());
        std::fprintf(stderr, "Shader compile log:\n%s\n", &log[0]);

        GLint status = GL_FALSE;

        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

        if (status) {
            GLChecked(glAttachShader(mProgram, shader));
        }

        GLChecked(glDeleteShader(shader));

        return status;
    }

    GLuint compile() const {
        if (mProgram) {
            GLChecked(glDeleteProgram(mProgram));
        }

        mNeedsUpdate = false;

        try {
            GLChecked(mProgram = glCreateProgram());

            if (!mVertSource.empty()) {
                if (!compile(GL_VERTEX_SHADER, mVertSource)) {
                    throw ShaderCompileException("vertex shader");
                }
            }

            if (!mGeomSource.empty()) {
                if (!compile(GL_GEOMETRY_SHADER, mGeomSource)) {
                    throw ShaderCompileException("geometry shader");
                }
            }

            if (!mFragSource.empty()) {
                if (!compile(GL_FRAGMENT_SHADER, mFragSource)) {
                    throw ShaderCompileException("fragment shader");
                }
            }

            for (auto i : mAttribLocations) {
                GLChecked(glBindAttribLocation(mProgram, i.second, i.first.c_str()));
            }

            GLChecked(glLinkProgram(mProgram));

            GLint logLength = 0;

            glGetProgramiv(mProgram, GL_INFO_LOG_LENGTH, &logLength);
            std::vector<char> log(logLength);
            glGetProgramInfoLog(mProgram, log.size(), nullptr, log.data());
            std::fprintf(stderr, "Program link log:\n%s\n", &log[0]);

            GLint status = GL_FALSE;

            glGetProgramiv(mProgram, GL_LINK_STATUS, &status);

            if (!status) {
                throw ShaderCompileException("link program");
            }
        }

        catch (ShaderCompileException&) {
            GLChecked(glDeleteProgram(mProgram));
            mProgram = 0;
        }

        return mProgram;
    }

public:
    static void bind(const Shader *shader) {
        if (shader != mBound) {
            if (shader) {
                if (shader->mNeedsUpdate) {
                    shader->compile();
                }

                GLChecked(glUseProgram(shader->getProgramID()));

            } else {
                GLChecked(glUseProgram(0));
            }

            mBound = shader;
        }
    }

private:
    static const Shader *mBound;

    class TempBinder {
        const Shader *mOldShader;

    public:
        TempBinder(Shader *shader = nullptr): mOldShader(mBound) {
            if (shader) {
                Shader::bind(shader);
            }
        }

        ~TempBinder() {
            Shader::bind(mOldShader);
        }
    };
};

const Shader *Shader::mBound = nullptr;

////////////////////////////////////////////////////////////////////////////////

class CameraRenderer {
    float mFOV;
    float mAspect;
    float mZNear;
    float mZFar;
    sf::Vector3f mPosition;
    sf::Vector2f mLook;

    mutable Transform3D mTransform;
    mutable bool mNeedsUpdate;

public:
    CameraRenderer(
    ): mFOV(75.0), mAspect(1.0), mZNear(0.1), mZFar(100.0), mNeedsUpdate(true) {
    }

    CameraRenderer(
        float fov,
        float aspect,
        float zNear,
        float zFar
    ): mFOV(fov), mAspect(aspect), mZNear(zNear), mZFar(zFar), mNeedsUpdate(true) {
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

    void move(const sf::Vector3f &offset) {
        setPosition(mPosition.x + offset.x,
                    mPosition.y + offset.y,
                    mPosition.z + offset.z);
    }

    void move(float dx, float dy, float dz) {
        move(sf::Vector3f(dx, dy, dz));
    }

    void setLook(const sf::Vector2f &look) {
        mLook = look;
        mNeedsUpdate = true;
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

public:
    ModelRenderer(
        const Model *model = nullptr,
        const Shader *shader = nullptr
    ): mModel(model), mShader(shader), mVBO() {
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

            GLChecked(glVertexPointer(3, GL_FLOAT, SizeAndOffset(Vertex, position)));
            GLChecked(glNormalPointer(GL_FLOAT, SizeAndOffset(Vertex, normal)));
            GLChecked(glTexCoordPointer(2, GL_SHORT, SizeAndOffset(Vertex, texCoord)));
            GLChecked(glColorPointer(4, GL_UNSIGNED_BYTE, SizeAndOffset(Vertex, color)));

            GLChecked(glEnableClientState(GL_VERTEX_ARRAY));
            GLChecked(glEnableClientState(GL_NORMAL_ARRAY));
            GLChecked(glEnableClientState(GL_TEXTURE_COORD_ARRAY));
            GLChecked(glEnableClientState(GL_COLOR_ARRAY));

            GLChecked(glDrawArrays(mModel->getPrimitive(), 0,
                mModel->getVertices().size()));

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

void makeBox(Vertex *verts, const sf::Vector3f &center, const sf::Vector3f &size) {
    sf::Vector3f mx = center + size;
    sf::Vector3f mn = center - size;
    size_t i = 0;
    verts[i++] = Vertex(0x7fff,0x0000, +1.0, 0.0, 0.0, mx.x,mx.y,mn.z);
    verts[i++] = Vertex(0x7fff,0x7fff, +1.0, 0.0, 0.0, mx.x,mx.y,mx.z);
    verts[i++] = Vertex(0x0000,0x7fff, +1.0, 0.0, 0.0, mx.x,mn.y,mx.z);
    verts[i++] = Vertex(0x0000,0x0000, +1.0, 0.0, 0.0, mx.x,mn.y,mn.z);
    verts[i++] = Vertex(0x0000,0x0000, -1.0, 0.0, 0.0, mn.x,mn.y,mn.z);
    verts[i++] = Vertex(0x0000,0x7fff, -1.0, 0.0, 0.0, mn.x,mn.y,mx.z);
    verts[i++] = Vertex(0x7fff,0x7fff, -1.0, 0.0, 0.0, mn.x,mx.y,mx.z);
    verts[i++] = Vertex(0x7fff,0x0000, -1.0, 0.0, 0.0, mn.x,mx.y,mn.z);
    verts[i++] = Vertex(0x0000,0x0000,  0.0,+1.0, 0.0, mn.x,mx.y,mn.z);
    verts[i++] = Vertex(0x0000,0x7fff,  0.0,+1.0, 0.0, mn.x,mx.y,mx.z);
    verts[i++] = Vertex(0x7fff,0x7fff,  0.0,+1.0, 0.0, mx.x,mx.y,mx.z);
    verts[i++] = Vertex(0x7fff,0x0000,  0.0,+1.0, 0.0, mx.x,mx.y,mn.z);
    verts[i++] = Vertex(0x7fff,0x0000,  0.0,-1.0, 0.0, mx.x,mn.y,mn.z);
    verts[i++] = Vertex(0x7fff,0x7fff,  0.0,-1.0, 0.0, mx.x,mn.y,mx.z);
    verts[i++] = Vertex(0x0000,0x7fff,  0.0,-1.0, 0.0, mn.x,mn.y,mx.z);
    verts[i++] = Vertex(0x0000,0x0000,  0.0,-1.0, 0.0, mn.x,mn.y,mn.z);
    verts[i++] = Vertex(0x7fff,0x0000,  0.0, 0.0,+1.0, mx.x,mn.y,mx.z);
    verts[i++] = Vertex(0x7fff,0x7fff,  0.0, 0.0,+1.0, mx.x,mx.y,mx.z);
    verts[i++] = Vertex(0x0000,0x7fff,  0.0, 0.0,+1.0, mn.x,mx.y,mx.z);
    verts[i++] = Vertex(0x0000,0x0000,  0.0, 0.0,+1.0, mn.x,mn.y,mx.z);
    verts[i++] = Vertex(0x0000,0x0000,  0.0, 0.0,-1.0, mn.x,mn.y,mn.z);
    verts[i++] = Vertex(0x0000,0x7fff,  0.0, 0.0,-1.0, mn.x,mx.y,mn.z);
    verts[i++] = Vertex(0x7fff,0x7fff,  0.0, 0.0,-1.0, mx.x,mx.y,mn.z);
    verts[i++] = Vertex(0x7fff,0x0000,  0.0, 0.0,-1.0, mx.x,mn.y,mn.z);
}

void makeBox(Vertex *verts) {
    makeBox(verts, sf::Vector3f(0,0,0), sf::Vector3f(1,1,1));
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

    CameraRenderer camera(90.0f, 16.0f/9.0f, 0.1f, 100.0f);
    camera.setPosition(0.0, 0.0, 5.0);
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

    Vertex cubeVerts[24];
    makeBox(cubeVerts, sf::Vector3f(0,0,0), sf::Vector3f(0.5,0.5,0.5));

    Model cubeModel(GL_QUADS, cubeVerts);

    cubeModel.calcNormals();

    //~ ModelRenderer cube(&cubeModel);
    ModelRenderer cube(&cubeModel, &shader);

    //~ ModelRenderer cube(&cubeModel);
    ModelRenderer cube2(&cubeModel, &shader);

    float spin = 0, spinSpeed = 45; // degrees/second

    sf::Vector2i lastMousePos = sf::Mouse::getPosition(window);
    sf::Vector2f look;

    GLChecked(glEnable(GL_DEPTH_TEST));
    //~ GLChecked(glDepthMask(GL_TRUE));
    GLChecked(glDepthFunc(GL_LESS));

    GLChecked(glEnable(GL_CULL_FACE));
    GLChecked(glEnable(GL_COLOR_MATERIAL));
    GLChecked(glEnable(GL_LIGHTING));
    GLChecked(glEnable(GL_LIGHT0));

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

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::W)) {
            camera.move(0,0,-delta.asSeconds());
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::S)) {
            camera.move(0,0,delta.asSeconds());
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::A)) {
            camera.move(-delta.asSeconds(),0,0);
        }
        if (sf::Keyboard::isKeyPressed(sf::Keyboard::D)) {
            camera.move(delta.asSeconds(),0,0);
        }

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
        GLChecked(glEnable(GL_LIGHTING));
        GLChecked(glEnable(GL_LIGHT0));
        GLChecked(glEnable(GL_COLOR_MATERIAL));

        // draw 3D scene

        //~ camera.render();

        look.x = std::fmod(look.x + 180.0f, 360.0f) - 180.0f;

        if (look.y > 89.9f) {
            look.y = 89.9f;
        }
        if (look.y < -89.9f) {
            look.y = -89.9f;
        }

        camera.setLook(look);
        Transform3D projectionTransform(camera.getTransform());
        //~ projectionTransform.rotate(look.y, sf::Vector3f(1.0f, 0.0f, 0.0f));
        //~ projectionTransform.rotate(look.x, sf::Vector3f(0.0f, 1.0f, 0.0f));

        shader.setParameter("uTime", playTime.asSeconds());
        shader.setParameter("uProjMatrix", projectionTransform);

        Transform3D modelViewTransform;
        modelViewTransform.rotate(std::sin(spin*PI/360.0f)*30.0f,
                                  sf::Vector3f(1.0f,0.0f,0.0f));
        modelViewTransform.rotate(spin,
                                  sf::Vector3f(0.0f,1.0f,0.0f));

        shader.setParameter("uViewMatrix", modelViewTransform);

        //~ GLChecked(glPushMatrix());
        //~ GLChecked(glLoadMatrixf(modelViewTransform.getMatrix()));
        cube.render();
        //~ GLChecked(glPopMatrix());

        modelViewTransform.translate(sf::Vector3f(1.0f,0.0f,0.0f));
        shader.setParameter("uViewMatrix", modelViewTransform);
        cube.render();

        // end 3D

        ////////////////////////////////////////////////////////////

        // 2D setup

        GLChecked(glDisable(GL_DEPTH_TEST));
        GLChecked(glDisable(GL_CULL_FACE));
        GLChecked(glDisable(GL_LIGHTING));

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

