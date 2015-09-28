////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "Shader.hpp"
#include "GLCheck.hpp"

////////////////////////////////////////////////////////////////////////////////

class TempBinder {
    const Shader *mOldShader;

public:
    TempBinder(Shader *shader = nullptr): mOldShader(Shader::getBound()) {
        if (shader) {
            Shader::bind(shader);
        }
    }

    ~TempBinder() {
        Shader::bind(mOldShader);
    }
};

////////////////////////////////////////////////////////////////////////////////

const Shader *Shader::mBound = nullptr;

Shader::Shader(): mProgram(), mNeedsUpdate() {
}

Shader::~Shader() {
    if (mProgram) {
        GLChecked(glDeleteProgram(mProgram));
        mProgram = 0;
    }
}

bool Shader::loadFromFile(const std::string &vertSource, const std::string &fragSource) {
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

void Shader::setFragmentShaderSource(const std::string &source) {
    mFragSource = source;
    mNeedsUpdate = true;
}

void Shader::setGeometryShaderSource(const std::string &source) {
    mGeomSource = source;
    mNeedsUpdate = true;
}

void Shader::setVertexShaderSource(const std::string &source) {
    mVertSource = source;
    mNeedsUpdate = true;
}

GLint Shader::getUniformLocation(const std::string &name) {
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

GLint Shader::getAttribLocation(const std::string &name) {
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

void Shader::bindAttribLocation(const std::string &name, GLint location) {
    mAttribLocations[name] = location;
    mNeedsUpdate = true;
}

void Shader::setParameter(GLint location, int value) {
    TempBinder binder(this);
    GLChecked(glUniform1i(location, value));
}

void Shader::setParameter(GLint location, const sf::Vector2i &value) {
    TempBinder binder(this);
    GLChecked(glUniform2i(location, value.x, value.y));
}

void Shader::setParameter(GLint location, const sf::Vector3i &value) {
    TempBinder binder(this);
    GLChecked(glUniform3i(location, value.x, value.y, value.z));
}

void Shader::setParameter(GLint location, float value) {
    TempBinder binder(this);
    GLChecked(glUniform1f(location, value));
}

void Shader::setParameter(GLint location, const sf::Vector2f &value) {
    TempBinder binder(this);
    GLChecked(glUniform2f(location, value.x, value.y));
}

void Shader::setParameter(GLint location, const sf::Vector3f &value) {
    TempBinder binder(this);
    GLChecked(glUniform3f(location, value.x, value.y, value.z));
}

void Shader::setParameter(GLint location, const Transform3D &value) {
    TempBinder binder(this);
    GLChecked(glUniformMatrix4fv(location, 1, GL_FALSE, value.getMatrix()));
}

void Shader::setParameter(const std::string &name, int value) {
    return setParameter(getUniformLocation(name), value);
}

void Shader::setParameter(const std::string &name, const sf::Vector2i &value) {
    return setParameter(getUniformLocation(name), value);
}

void Shader::setParameter(const std::string &name, const sf::Vector3i &value) {
    return setParameter(getUniformLocation(name), value);
}

void Shader::setParameter(const std::string &name, float value) {
    return setParameter(getUniformLocation(name), value);
}

void Shader::setParameter(const std::string &name, const sf::Vector2f &value) {
    return setParameter(getUniformLocation(name), value);
}

void Shader::setParameter(const std::string &name, const sf::Vector3f &value) {
    return setParameter(getUniformLocation(name), value);
}

void Shader::setParameter(const std::string &name, const Transform3D &value) {
    return setParameter(getUniformLocation(name), value);
}

GLuint Shader::getProgramID() const {
    return mProgram;
}

bool Shader::compile(GLenum shaderType, const std::string &source) const {
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

GLuint Shader::compile() const {
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

const Shader *Shader::getBound() {
    return mBound;
}

void Shader::bind(const Shader *shader) {
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

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

