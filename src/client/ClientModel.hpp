////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __CLIENTMODEL_HPP__
#define __CLIENTMODEL_HPP__ 1

////////////////////////////////////////////////////////////////////////////////

#include "engine/engine.hpp"
#include "Shader.hpp"
#include <GL/glew.h>

////////////////////////////////////////////////////////////////////////////////

class ClientModel {
    const Model *mModel;
    const Shader *mShader;
    mutable GLuint mVBO;
    mutable GLenum mPrimitive;
    mutable GLuint mCount;

public:
    ClientModel(const Model *model = nullptr, const Shader *shader = nullptr);
    ClientModel(const Model &model);
    ClientModel(const Model &model, const Shader &shader);
    ~ClientModel();

    void setModel(const Model &model);
    void setModel(const Model *model);
    const Model *getModel() const;

    void setShader(const Shader &shader);
    void setShader(const Shader *shader);
    const Shader *getShader() const;

    void render() const;

private:
    void createVBO() const;
    void destroyVBO();
};

////////////////////////////////////////////////////////////////////////////////

#endif // __CLIENTMODEL_HPP__

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

