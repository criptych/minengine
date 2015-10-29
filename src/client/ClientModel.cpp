////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "ClientModel.hpp"

#include <GL/glew.h>
#include "GLCheck.hpp"

////////////////////////////////////////////////////////////////////////////////

ClientModel::ClientModel(
    const Model *model
): mModel(model), mVAO(), mVBO(), mIBO(), mPrimitive(), mCount() {
}

ClientModel::ClientModel(
    const Model &model
): mModel(&model), mVAO(), mVBO(), mIBO(), mPrimitive(), mCount() {
}

ClientModel::~ClientModel() {
    destroyVertexArrays();
}

void ClientModel::setModel(const Model *model) {
    destroyVertexArrays();
    mModel = model;
    if (mModel) {
        mPrimitive = mModel->getPrimitive();
    }
}

void ClientModel::setModel(const Model &model) {
    setModel(&model);
}

const Model *ClientModel::getModel() const {
    return mModel;
}

void ClientModel::render() const {
    if (mModel) {
        if (mVAO == 0) {
            createVertexArrays();
            if (mVAO == 0) {
                return;
            }
        }

        GLClearErrors();

        GLChecked(glBindVertexArray(mVAO));

        if (mIBO) {
            GLChecked(glDrawElements(mPrimitive, mCount, GL_UNSIGNED_SHORT, nullptr));
        } else {
            GLChecked(glDrawArrays(mPrimitive, 0, mCount));
        }

        GLChecked(glBindVertexArray(0));
    }
}

void ClientModel::createVertexArrays() const {
    if (mModel) {
        size_t numVerts = mModel->getVertices().size();
        size_t numIndex = mModel->getIndices().size();

        mCount = numVerts;

        if (mCount == 0) {
            return;
        }

        sf::err() << "numVerts = " << numVerts;
        sf::err() << ", numIndex = " << numIndex;

        GLClearErrors();

        if (!mVAO) {
            if (!glGenVertexArrays) {
                sf::err() << "\n\nglGenVertexArrays == NULL!\n";
            }

            GLChecked(glGenVertexArrays(1, &mVAO));

            sf::err() << ", VAO = " << mVAO;

            if (!mVAO) {
                return;
            }
        }

        GLChecked(glBindVertexArray(mVAO));

        if (!mVBO) {
            GLChecked(glGenBuffers(1, &mVBO));

            sf::err() << ", VBO = " << mVBO;

            if (!mVBO) {
                return;
            }
        }

        sf::err() << "glGenBuffers(VBO) -> " << mVBO;
        sf::err() << std::flush;

        if (numIndex > 0) {
            mCount = numIndex;

            if (!mIBO) {
                GLChecked(glGenBuffers(1, &mIBO));

                sf::err() << ", IBO = " << mIBO;

                if (!mIBO) {
                    return;
                }
            }

            sf::err() << ", glGenBuffers(IBO) -> " << mIBO;
            sf::err() << std::flush;

            size_t size = sizeof(uint16_t) * numIndex;
            const void *data = mModel->getIndices().data();
            GLChecked(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
            GLChecked(glBufferData(GL_ELEMENT_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));
        }

        sf::err() << ", mPrimitive = " << mPrimitive;
        sf::err() << ", mCount = " << mCount;
        sf::err() << std::flush;

        size_t size = sizeof(Vertex) * numVerts;
        const void *data = mModel->getVertices().data();
        sf::err() << ", size == " << size;
        sf::err() << std::flush;
        GLChecked(glBindBuffer(GL_ARRAY_BUFFER, mVBO));
        GLChecked(glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW));

        GLChecked(glBindBuffer(GL_ARRAY_BUFFER, mVBO));

        if (mIBO) {
            GLChecked(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIBO));
        }

#define SizeAndOffset(T, F) sizeof(T), reinterpret_cast<const void*>(offsetof(T, F))

        GLChecked(glEnableVertexAttribArray(0));
        GLChecked(glEnableVertexAttribArray(1));
        GLChecked(glEnableVertexAttribArray(2));
        GLChecked(glEnableVertexAttribArray(3));

        GLChecked(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, SizeAndOffset(Vertex, position)));
        GLChecked(glVertexAttribPointer(1, 3, GL_FLOAT, GL_TRUE,  SizeAndOffset(Vertex, normal)));
        GLChecked(glVertexAttribPointer(2, 2, GL_SHORT, GL_TRUE,  SizeAndOffset(Vertex, texCoord)));
        GLChecked(glVertexAttribPointer(3, 4, GL_UNSIGNED_BYTE, GL_TRUE, SizeAndOffset(Vertex, color)));

        GLChecked(glBindVertexArray(0));

        sf::err() << std::endl;
    }
}

void ClientModel::destroyVertexArrays() {
    if (mVAO) {
        GLChecked(glDeleteVertexArrays(1, &mVAO));
        mVAO = 0;
    }

    if (mVBO) {
        GLChecked(glDeleteBuffers(1, &mVBO));
        mVBO = 0;
    }

    if (mIBO) {
        GLChecked(glDeleteBuffers(1, &mIBO));
        mIBO = 0;
    }

    mPrimitive = 0;
    mCount = 0;
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

