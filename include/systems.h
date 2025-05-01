#ifndef SYSTEMS_H__
#define SYSTEMS_H__

#include <cstdint>
#include <cstdio>
#include <glm/glm.hpp>
#include <lua/lua.hpp>
#include <lua/lauxlib.h>
#include <lua/lualib.h>
#include <scene.h>
#include <ctime>

namespace systems {
    #include <systems/rendering.h>
    #include <systems/transform.h>
    #include <systems/scripting.h>

    
    void start_time();

    float end_time();
};


#endif