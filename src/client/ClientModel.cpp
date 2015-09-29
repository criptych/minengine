////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "ClientModel.hpp"

#include <GL/glew.h>
#include "GLCheck.hpp"

////////////////////////////////////////////////////////////////////////////////

ClientModel::ClientModel(
    const Model *model,
    const Shader *shader
): mModel(model), mShader(shader), mVBO(), mPrimitive(), mCount() {
}

ClientModel::ClientModel(
    const Model &model
): mModel(&model), mShader(nullptr), mVBO(), mPrimitive(), mCount() {
}

ClientModel::ClientModel(
    const Model &model,
    const Shader &shader
): mModel(&model), mShader(&shader), mVBO(), mPrimitive(), mCount() {
}

ClientModel::~ClientModel() {
    destroyVBO();
}

void ClientModel::setModel(const Model *model) {
    destroyVBO();
    mModel = model;
}

void ClientModel::setModel(const Model &model) {
    setModel(&model);
}

const Model *ClientModel::getModel() const {
    return mModel;
}

void ClientModel::setShader(const Shader *shader) {
    mShader = shader;
}

void ClientModel::setShader(const Shader &shader) {
    setShader(&shader);
}

const Shader *ClientModel::getShader() const {
    return mShader;
}

void ClientModel::render() const {
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

#define SizeAndOffset(T, F) sizeof(T), reinterpret_cast<const void*>(offsetof(T, F))

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

        GLChecked(glDisableVertexAttribArray(0));
        GLChecked(glDisableVertexAttribArray(1));
        GLChecked(glDisableVertexAttribArray(2));
        GLChecked(glDisableVertexAttribArray(3));

        GLChecked(glBindBuffer(GL_ARRAY_BUFFER, 0));

        if (mShader) {
            Shader::bind(nullptr);
        }
    }
}

void ClientModel::createVBO() const {
    if (mModel && !mVBO) {
        GLChecked(glGenBuffers(1, &mVBO));

        sf::err() << "glGenBuffers -> " << mVBO;
        sf::err() << std::flush;

        mPrimitive = mModel->getPrimitive();
        mCount = mModel->getVertices().size();

        sf::err() << ", mPrimitive = " << mPrimitive;
        sf::err() << ", mCount = " << mCount;
        sf::err() << std::flush;

        if (mCount && mVBO) {
            size_t size = sizeof(Vertex) * mCount;
            const void *data = mModel->getVertices().data();
            sf::err() << ", size == " << size;
            sf::err() << std::flush;
            GLChecked(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
            GLChecked(glBufferData(GL_ARRAY_BUFFER, size, data,
                GL_STATIC_DRAW));
            GLChecked(glBindBuffer(GL_ARRAY_BUFFER, 0));
        }

        sf::err() << std::endl;
    }
}

void ClientModel::destroyVBO() {
    if (mVBO) {
        GLChecked(glDeleteBuffers(1, &mVBO));
        mVBO = 0;
        mPrimitive = 0;
        mCount = 0;
    }
}

////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

