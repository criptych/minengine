////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

extern "C" {
#include <lua.h>
#include <lauxlib.h>
};

#include "ClientObject.hpp"
#include "ShaderCache.hpp"
#include "TextureCache.hpp"

#include <vector>

////////////////////////////////////////////////////////////////////////////////

extern std::vector<ClientObject*> gObjects;
extern ShaderCache gShaderCache;
extern TextureCache gTextureCache;

int ll_Object___new(lua_State *L) {
    luaL_checktype(L, 2, LUA_TTABLE);
    ClientObject **pobject = static_cast<ClientObject**>(
        lua_newuserdata(L, sizeof(ClientObject*)));
    ClientObject *object = *pobject = new ClientObject();
    gObjects.push_back(object);

    sf::err() << gObjects.size() << " objects to draw\n";

    lua_getfield(L, 2, "shader");
    if (!lua_isnoneornil(L, -1)) {
        const char *name = luaL_checkstring(L, -1);
        sf::Shader *shader = gShaderCache.acquire(name);
        object->setShader(shader);

        sf::err() << "Shader: " << shader << "\n";
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "material");
    if (!lua_isnoneornil(L, -1)) {
        luaL_checktype(L, -1, LUA_TTABLE);
        MaterialInfo *material = new MaterialInfo();
        object->setMaterial(material);

        lua_getfield(L, -1, "diffMap");
        if (!lua_isnoneornil(L, -1)) {
            const char *name = luaL_checkstring(L, -1);
            material->diffMap = gTextureCache.acquire(name);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "specMap");
        if (!lua_isnoneornil(L, -1)) {
            const char *name = luaL_checkstring(L, -1);
            material->specMap = gTextureCache.acquire(name);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "glowMap");
        if (!lua_isnoneornil(L, -1)) {
            const char *name = luaL_checkstring(L, -1);
            material->glowMap = gTextureCache.acquire(name);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "bumpMap");
        if (!lua_isnoneornil(L, -1)) {
            const char *name = luaL_checkstring(L, -1);
            material->bumpMap = gTextureCache.acquire(name);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "roughness");
        if (!lua_isnoneornil(L, -1)) {
            material->roughness = luaL_checknumber(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "bumpScale");
        if (!lua_isnoneornil(L, -1)) {
            material->bumpScale = luaL_checknumber(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "bumpBias");
        if (!lua_isnoneornil(L, -1)) {
            material->bumpBias = luaL_checknumber(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "fresnelPower");
        if (!lua_isnoneornil(L, -1)) {
            material->fresnelPower = luaL_checknumber(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "fresnelScale");
        if (!lua_isnoneornil(L, -1)) {
            material->fresnelScale = luaL_checknumber(L, -1);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "fresnelBias");
        if (!lua_isnoneornil(L, -1)) {
            material->fresnelBias = luaL_checknumber(L, -1);
        }
        lua_pop(L, 1);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "model");
    if (!lua_isnoneornil(L, -1)) {

        static const char *const primitive_names[] = {
            "points",
            "lines",
            "lineloop",
            "linestrip",
            "triangles",
            "trianglestrip",
            "trianglefan",
            //~ "quads",
            //~ "quadstrip",
            //~ "polygon",
            nullptr
        };

        static const char *const shape_names[] = {
            "box",
            "plane",
            "sphere",
            nullptr
        };

        Model *model = new Model();
        object->setModel(new ClientModel(model));

        lua_getfield(L, -1, "primitive");
        int primitive = luaL_checkoption(L, -1, "triangles", primitive_names);
        lua_pop(L, 1);

        lua_getfield(L, -1, "shape");
        int shape = luaL_checkoption(L, -1, "box", shape_names);
        lua_pop(L, 1);

        lua_getfield(L, -1, "radius");
        float radius = luaL_optnumber(L, -1, 1.0f);
        lua_pop(L, 1);

        lua_getfield(L, -1, "steps");
        int step = 5, rstep = 6;
        if (!lua_isnoneornil(L, -1)) {
            if (lua_type(L, -1) == LUA_TTABLE) {
                lua_rawgeti(L, -1, 1);
                lua_rawgeti(L, -2, 2);
                step = luaL_checkinteger(L, -2);
                rstep = luaL_checkinteger(L, -1);
                lua_pop(L, 2);
            } else {
                step = luaL_checkinteger(L, -1);
                rstep = 2 * step;
            }
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "size");
        glm::vec3 size(1.0f, 1.0f, 1.0f);
        if (!lua_isnoneornil(L, -1)) {
            lua_rawgeti(L, -1, 1);
            lua_rawgeti(L, -2, 2);
            lua_rawgeti(L, -3, 3);
            size.x = luaL_checknumber(L, -3);
            size.y = luaL_checknumber(L, -2);
            size.z = luaL_checknumber(L, -1);
            lua_pop(L, 3);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "center");
        glm::vec3 center;
        if (!lua_isnoneornil(L, -1)) {
            lua_rawgeti(L, -1, 1);
            lua_rawgeti(L, -2, 2);
            lua_rawgeti(L, -3, 3);
            center.x = luaL_checknumber(L, -3);
            center.y = luaL_checknumber(L, -2);
            center.z = luaL_checknumber(L, -1);
            lua_pop(L, 3);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "a");
        glm::vec3 a(1.0f, 1.0f, 1.0f);
        if (!lua_isnoneornil(L, -1)) {
            lua_rawgeti(L, -1, 1);
            lua_rawgeti(L, -2, 2);
            lua_rawgeti(L, -3, 3);
            a.x = luaL_checknumber(L, -3);
            a.y = luaL_checknumber(L, -2);
            a.z = luaL_checknumber(L, -1);
            lua_pop(L, 3);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "b");
        glm::vec3 b(1.0f, 1.0f, 1.0f);
        if (!lua_isnoneornil(L, -1)) {
            lua_rawgeti(L, -1, 1);
            lua_rawgeti(L, -2, 2);
            lua_rawgeti(L, -3, 3);
            b.x = luaL_checknumber(L, -3);
            b.y = luaL_checknumber(L, -2);
            b.z = luaL_checknumber(L, -1);
            lua_pop(L, 3);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "c");
        glm::vec3 c(1.0f, 1.0f, 1.0f);
        if (!lua_isnoneornil(L, -1)) {
            lua_rawgeti(L, -1, 1);
            lua_rawgeti(L, -2, 2);
            lua_rawgeti(L, -3, 3);
            c.x = luaL_checknumber(L, -3);
            c.y = luaL_checknumber(L, -2);
            c.z = luaL_checknumber(L, -1);
            lua_pop(L, 3);
        }
        lua_pop(L, 1);

        lua_getfield(L, -1, "texRect");
        glm::vec4 texRect(0, 0, 1, 1);
        if (!lua_isnoneornil(L, -1)) {
            lua_rawgeti(L, -1, 1);
            lua_rawgeti(L, -2, 2);
            lua_rawgeti(L, -3, 3);
            lua_rawgeti(L, -4, 4);
            texRect.x  = luaL_checknumber(L, -4);
            texRect.y = luaL_checknumber(L, -3);
            texRect.z = luaL_checknumber(L, -2);
            texRect.w = luaL_checknumber(L, -1);
            lua_pop(L, 4);
        }
        lua_pop(L, 1);

        model->setPrimitive(primitive);

        switch (shape) {
            case 0: {
                model->makeBox(size, center, texRect);
                break;
            }

            case 1: {
                model->makePlane(a, b, c, texRect);
                break;
            }

            case 2: {
                model->makeBall(radius, step, rstep, center /*, texRect*/);
                break;
            }

            default: {
                sf::err() << "unknown shape " << shape << "???\n";
                break;
            }
        }

    }
    lua_pop(L, 1);
    return 1;
}

static luaL_Reg ll_Object_methods[] = {
    { "__new", ll_Object___new },
    { nullptr, nullptr }
};

int luaopen_Object(lua_State *L) {
    luaL_newmetatable(L, "Object");
    luaL_register(L, nullptr, ll_Object_methods);
    lua_newtable(L);
    lua_getfield(L, -2, "__new");
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

