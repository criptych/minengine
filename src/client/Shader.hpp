////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __SHADER_HPP__
#define __SHADER_HPP__ 1

////////////////////////////////////////////////////////////////////////////////

#include <map>
#include <string>

#include <GL/glew.h>

#include <SFML/Graphics/Transform.hpp>
#include <SFML/System.hpp>

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

    Shader();
    ~Shader();

    bool loadFromFile(const std::string &vertSource, const std::string &fragSource);
    void setFragmentShaderSource(const std::string &source);
    void setGeometryShaderSource(const std::string &source);
    void setVertexShaderSource(const std::string &source);

    GLint getUniformLocation(const std::string &name);
    GLint getAttribLocation(const std::string &name);
    void bindAttribLocation(const std::string &name, GLint location);

    void setParameter(GLint location, int value);
    void setParameter(GLint location, const sf::Vector2i &value);
    void setParameter(GLint location, const sf::Vector3i &value);
    void setParameter(GLint location, float value);
    void setParameter(GLint location, const sf::Vector2f &value);
    void setParameter(GLint location, const sf::Vector3f &value);
    void setParameter(GLint location, const sf::Transform &value);
    void setParameter(const std::string &name, int value);
    void setParameter(const std::string &name, const sf::Vector2i &value);
    void setParameter(const std::string &name, const sf::Vector3i &value);
    void setParameter(const std::string &name, float value);
    void setParameter(const std::string &name, const sf::Vector2f &value);
    void setParameter(const std::string &name, const sf::Vector3f &value);
    void setParameter(const std::string &name, const sf::Transform &value);

    GLuint getProgramID() const;

protected:
    class ShaderCompileException {
    public:
        ShaderCompileException() {}
        ShaderCompileException(const std::string &msg) {}
    };

    bool compile(GLenum shaderType, const std::string &source) const;
    GLuint compile() const;

private:
    static const Shader *mBound;

public:
    static const Shader *getBound();
    static void bind(const Shader *shader);
};

////////////////////////////////////////////////////////////////////////////////

#endif // __SHADER_HPP__

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

