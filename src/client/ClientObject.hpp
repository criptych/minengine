////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __CLIENTOBJECT_HPP__
#define __CLIENTOBJECT_HPP__ 1

////////////////////////////////////////////////////////////////////////////////

#include "ClientModel.hpp"
#include "MaterialInfo.hpp"
#include <SFML/Graphics/Shader.hpp>

////////////////////////////////////////////////////////////////////////////////

class ClientObject {
    const ClientModel *mModel;
    sf::Shader *mShader;
    const MaterialInfo *mMaterial;

public:
    ClientObject(
        const ClientModel *model = nullptr,
        sf::Shader *shader = nullptr,
        const MaterialInfo *material = nullptr
    );

    ClientObject(
        const ClientModel &model
    );

    ClientObject(
        const ClientModel &model,
        sf::Shader &shader
    );

    ClientObject(
        const ClientModel &model,
        sf::Shader &shader,
        const MaterialInfo &material
    );

    ~ClientObject();

    void setModel(const ClientModel &model);
    void setModel(const ClientModel *model);
    const ClientModel *getModel() const;

    void setShader(sf::Shader &shader);
    void setShader(sf::Shader *shader);
    const sf::Shader *getShader() const;

    void setMaterial(const MaterialInfo &material);
    void setMaterial(const MaterialInfo *material);
    const MaterialInfo *getMaterial() const;

    void render() const;
};

////////////////////////////////////////////////////////////////////////////////

#endif // __CLIENTOBJECT_HPP__

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

