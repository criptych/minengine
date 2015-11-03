////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "ClientObject.hpp"

////////////////////////////////////////////////////////////////////////////////

ClientObject::ClientObject(
    const ClientModel *model,
    sf::Shader *shader,
    const MaterialInfo *material
): mModel(model), mShader(shader), mMaterial(material) {
}

ClientObject::ClientObject(
    const ClientModel &model
): mModel(&model), mShader(nullptr), mMaterial(nullptr) {
}

ClientObject::ClientObject(
    const ClientModel &model,
    sf::Shader &shader
): mModel(&model), mShader(&shader), mMaterial(nullptr) {
}

ClientObject::ClientObject(
    const ClientModel &model,
    sf::Shader &shader,
    const MaterialInfo &material
): mModel(&model), mShader(&shader), mMaterial(&material) {
}

ClientObject::~ClientObject() {
}

void ClientObject::setModel(const ClientModel *model) {
    mModel = model;
}

void ClientObject::setModel(const ClientModel &model) {
    setModel(&model);
}

const ClientModel *ClientObject::getModel() const {
    return mModel;
}

void ClientObject::setShader(sf::Shader *shader) {
    mShader = shader;
}

void ClientObject::setShader(sf::Shader &shader) {
    setShader(&shader);
}

const sf::Shader *ClientObject::getShader() const {
    return mShader;
}

void ClientObject::setMaterial(const MaterialInfo *material) {
    mMaterial = material;
}

void ClientObject::setMaterial(const MaterialInfo &material) {
    setMaterial(&material);
}

const MaterialInfo *ClientObject::getMaterial() const {
    return mMaterial;
}

void ClientObject::render() const {
    if (mModel) {
        if (mShader) {
            if (mMaterial) {
                if (mMaterial->diffMap) {
                    mShader->setParameter("uMaterial.diffMap", *mMaterial->diffMap);
                }
                if (mMaterial->specMap) {
                    mShader->setParameter("uMaterial.specMap", *mMaterial->specMap);
                }
                if (mMaterial->glowMap) {
                    mShader->setParameter("uMaterial.glowMap", *mMaterial->glowMap);
                }
                if (mMaterial->bumpMap) {
                    mShader->setParameter("uMaterial.bumpMap", *mMaterial->bumpMap);
                }
                mShader->setParameter("uMaterial.specPower",    mMaterial->specPower);
                mShader->setParameter("uMaterial.bumpScale",    mMaterial->bumpScale);
                mShader->setParameter("uMaterial.bumpBias",     mMaterial->bumpBias);
                mShader->setParameter("uMaterial.fresnelPower", mMaterial->fresnelPower);
                mShader->setParameter("uMaterial.fresnelScale", mMaterial->fresnelScale);
                mShader->setParameter("uMaterial.fresnelBias",  mMaterial->fresnelBias);
            }
        }

        sf::Shader::bind(mShader);

        mModel->render();

        if (mShader) {
            sf::Shader::bind(nullptr);
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

