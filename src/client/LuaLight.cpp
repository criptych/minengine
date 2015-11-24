////////////////////////////////////////////////////////////////////////////////
//
////////////////////////////////////////////////////////////////////////////////

extern "C" {
#include <lua.h>
#include <lauxlib.h>
};

#include "LightInfo.hpp"

#include <vector>

////////////////////////////////////////////////////////////////////////////////

extern std::vector<LightInfo*> gLights;

int ll_Light___new(lua_State *L) {
    luaL_checktype(L, 2, LUA_TTABLE);
    LightInfo **plight = static_cast<LightInfo**>(
        lua_newuserdata(L, sizeof(LightInfo*)));
    LightInfo *light = *plight = new LightInfo();
    gLights.push_back(light);

    static const char *const type_names[] = {
        "point",
        "spot",
        "directional",
        nullptr
    };

    lua_getfield(L, 2, "type");
    light->type = static_cast<LightInfo::LightType>(
        luaL_checkoption(L, -1, "point", type_names));
    lua_pop(L, 1);

    lua_getfield(L, 2, "ambientColor");
    if (!lua_isnoneornil(L, -1)) {
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        lua_rawgeti(L, -3, 3);
        light->ambtColor.r = luaL_checknumber(L, -3);
        light->ambtColor.g = luaL_checknumber(L, -2);
        light->ambtColor.b = luaL_checknumber(L, -1);
        light->ambtColor.a = 1.0f;
        lua_pop(L, 3);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "diffuseColor");
    if (!lua_isnoneornil(L, -1)) {
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        lua_rawgeti(L, -3, 3);
        light->diffColor.r = luaL_checknumber(L, -3);
        light->diffColor.g = luaL_checknumber(L, -2);
        light->diffColor.b = luaL_checknumber(L, -1);
        light->diffColor.a = 1.0f;
        lua_pop(L, 3);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "specularColor");
    if (!lua_isnoneornil(L, -1)) {
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        lua_rawgeti(L, -3, 3);
        light->specColor.r = luaL_checknumber(L, -3);
        light->specColor.g = luaL_checknumber(L, -2);
        light->specColor.b = luaL_checknumber(L, -1);
        light->specColor.a = 1.0f;
        lua_pop(L, 3);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "position");
    if (!lua_isnoneornil(L, -1)) {
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        lua_rawgeti(L, -3, 3);
        lua_rawgeti(L, -4, 4);
        light->position.x = luaL_checknumber(L, -4);
        light->position.y = luaL_checknumber(L, -3);
        light->position.z = luaL_checknumber(L, -2);
        light->position.w = luaL_optnumber(L, -1, 1.0);
        lua_pop(L, 3);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "spotDirection");
    light->spotDirection = glm::vec3(0, 0, -1);
    if (!lua_isnoneornil(L, -1)) {
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        lua_rawgeti(L, -3, 3);
        light->spotDirection.x = luaL_checknumber(L, -3);
        light->spotDirection.y = luaL_checknumber(L, -2);
        light->spotDirection.z = luaL_checknumber(L, -1);
        lua_pop(L, 3);
    }
    lua_pop(L, 1);

    lua_getfield(L, 2, "spotExponent");
    light->spotExponent = luaL_optnumber(L, -1, 1.0f);
    lua_pop(L, 1);

    lua_getfield(L, 2, "spotConeInner");
    light->spotConeInner = luaL_optnumber(L, -1, 180.0f);
    lua_pop(L, 1);

    lua_getfield(L, 2, "spotConeOuter");
    light->spotConeOuter = luaL_optnumber(L, -1, 180.0f);
    lua_pop(L, 1);

    lua_getfield(L, 2, "attenuation");
    if (!lua_isnoneornil(L, -1)) {
        lua_rawgeti(L, -1, 1);
        lua_rawgeti(L, -2, 2);
        lua_rawgeti(L, -3, 3);
        light->attenuation[0] = luaL_optnumber(L, -3, 1.0f);
        light->attenuation[1] = luaL_optnumber(L, -2, 0.0f);
        light->attenuation[2] = luaL_optnumber(L, -1, 0.0f);
        lua_pop(L, 3);
    } else {
        light->attenuation[0] = 1.0f;
        light->attenuation[1] = 0.0f;
        light->attenuation[2] = 0.0f;
    }
    lua_pop(L, 1);

    return 1;
}

static luaL_Reg ll_Light_methods[] = {
    { "__new", ll_Light___new },
    { nullptr, nullptr }
};

int luaopen_Light(lua_State *L) {
    luaL_newmetatable(L, "Light");
    luaL_register(L, nullptr, ll_Light_methods);
    lua_newtable(L);
    lua_getfield(L, -2, "__new");
    lua_setfield(L, -2, "__call");
    lua_setmetatable(L, -2);
    return 1;
}

////////////////////////////////////////////////////////////////////////////////
//  EOF
////////////////////////////////////////////////////////////////////////////////

