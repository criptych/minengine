////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

#include "ShaderCache.hpp"

////////////////////////////////////////////////////////////////////////////////

sf::Shader *ShaderCache::load(sf::Shader *shader, const std::string &name) {
    shader->setAttribLocation("aVertex",   0);
    shader->setAttribLocation("aNormal",   1);
    shader->setAttribLocation("aTexCoord", 2);
    if (!shader->loadFromFile(name+".vert", name+".frag")) {
        return nullptr;
    }
    return shader;
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

