#include <systems.h>
#include <scene.h>
#include <cstdlib>


systems::scripting* current_scripting_system;


int _internal_translate(lua_State* L) {
    size_t id = lua_tonumber(L, 1);
    float x = lua_tonumber(L, 2);
    float y = lua_tonumber(L, 3);
    float z = lua_tonumber(L, 4);
    systems::translate_subscene(systems::get_current_transform_system(), scene::get_current_scene(), id, glm::vec3(x, y, z));
    return 1;
}

int _internal_rotate(lua_State* L) {
    size_t id = lua_tonumber(L, 1);
    float w = lua_tonumber(L, 2);
    float x = lua_tonumber(L, 3);
    float y = lua_tonumber(L, 4);
    float z = lua_tonumber(L, 5);
    systems::rotate_subscene(systems::get_current_transform_system(), scene::get_current_scene(), id, w, glm::vec3(x, y, z));
    return 1;
}

int _internal_scale(lua_State* L) {
    size_t id = lua_tonumber(L, 1);
    float x = lua_tonumber(L, 2);
    float y = lua_tonumber(L, 3);
    float z = lua_tonumber(L, 4);
    systems::scale_subscene(systems::get_current_transform_system(), scene::get_current_scene(), id, glm::vec3(x, y, z));
    return 1;
}

void lua_loadscripts(systems::scripting* scr, scene::graph* G, scene::resource* res_array) {
    
    G->has_script = res_array[G->id].has_script;
    if (res_array[G->id].has_script) {
        luaL_dostring(scr->global_state, (char*)res_array[G->id].script);
        char strp[128];
        sprintf(strp, "scene[%d] = {properties = properties, init = init, update = update}", G->id + 1);
        luaL_dostring(scr->global_state, strp);
    }
    for (int i = 0; i < G->num_children; i++) {
        lua_loadscripts(scr, G->children[i], res_array);
    }
}

systems::scripting* systems::create_scripting_system() {
    systems::scripting* scr = (systems::scripting*)malloc(sizeof(systems::scripting));
    scr->global_state = lua_open();
    luaL_openlibs(scr->global_state);
    lua_pushcfunction(scr->global_state, _internal_translate);
    lua_setglobal(scr->global_state,"_internal_translate");
    lua_pushcfunction(scr->global_state, _internal_rotate);
    lua_setglobal(scr->global_state,"_internal_rotate");
    lua_pushcfunction(scr->global_state, _internal_scale);
    lua_setglobal(scr->global_state,"_internal_scale");
    luaL_dostring(scr->global_state, "scene = {}");
    scene::graph* G = scene::get_current_scene();
    systems::transform* T = systems::get_current_transform_system();
    scene::resource* res_array = scene::get_current_resources();
    lua_loadscripts(scr, G, res_array);
    luaL_dostring(scr->global_state, "function sceneprop(id) return scene[id+1].properties end");
    return (systems::scripting*)scr;
}

void systems::make_current_scripting_system(systems::scripting* sys) {
    current_scripting_system = sys;
}

void lua_do_init_script(systems::scripting* sg, scene::graph* G) {
    if (G->has_script) {
        lua_getglobal(sg->global_state, "scene");
        lua_rawgeti(sg->global_state, -1, G->id + 1);
        lua_getfield(sg->global_state, -1, "init");
        lua_pushnumber(sg->global_state, G->id);
        lua_pcall(sg->global_state, 1, 0, 0);
    }
    // lua_dumpstack();
    for (int i = 0; i < G->num_children; i++) {
        lua_do_init_script(sg, G->children[i]);
    }
}

void lua_do_update_script(systems::scripting* sg, scene::graph* G) {
    if (G->has_script) {
        lua_getglobal(sg->global_state, "scene");
        lua_rawgeti(sg->global_state, -1, G->id + 1);
        lua_getfield(sg->global_state, -1, "update");
        lua_pushnumber(sg->global_state, G->id);
        lua_pushnumber(sg->global_state, 0.02);
        lua_pcall(sg->global_state, 2, 0, 0);
    }
    // lua_dumpstack(global_state);
    for (int i = 0; i < G->num_children; i++) {
        lua_do_update_script(sg, G->children[i]);
    }
}

void systems::init_scripting_system() {

    systems::scripting* sg = systems::get_current_scripting_system();
    scene::graph* G = scene::get_current_scene();
    lua_do_init_script(sg, G);

}

void lua_dumpstack (systems::scripting* sg) {
    lua_State* L = sg->global_state;
    int top=lua_gettop(L);
    for (int i=1; i <= top; i++) {
        printf("%d\t%s\t", i, luaL_typename(L,i));
        switch (lua_type(L, i)) {
        case LUA_TNUMBER:
            printf("%g\n",lua_tonumber(L,i));
            break;
        case LUA_TSTRING:
            printf("%s\n",lua_tostring(L,i));
            break;
        case LUA_TBOOLEAN:
            printf("%s\n", (lua_toboolean(L, i) ? "true" : "false"));
            break;
        case LUA_TNIL:
            printf("%s\n", "nil");
            break;
        default:
            printf("%p\n",lua_topointer(L,i));
            break;
        }
    }
}


void systems::update_scripting_system() {
    systems::scripting* sg = systems::get_current_scripting_system();
    scene::graph* G = scene::get_current_scene();
    lua_do_update_script(sg, G);
    // lua_dumpstack(sg);

}

systems::scripting* systems::get_current_scripting_system() {
    return (systems::scripting*)current_scripting_system;
}
