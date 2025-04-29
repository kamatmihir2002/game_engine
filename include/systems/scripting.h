#ifndef SCRIPTING_H__
#define SCRIPTING_H__

typedef struct scripting_t {
    lua_State* global_state;
} scripting;

scripting* create_scripting_system();

void make_current_scripting_system(scripting* sys);

scripting* get_current_scripting_system();

void init_scripting_system();

void update_scripting_system();



#endif