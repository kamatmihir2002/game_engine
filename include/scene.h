#ifndef SCENE_H__
#define SCENE_H__

#include <cstdint>
#include <glm/glm.hpp>

namespace scene {

    typedef enum object_type {
        GENERIC = 0,
        CAMERA,
        LIGHT
    } object_type;
    
    typedef struct graph_t {
        struct graph_t* parent;
        size_t id;
    
        char object_name[64];
        char resource_name[64];
        char script_name[64];

        glm::vec3 position;
        glm::vec3 scale;
        glm::vec3 rotate_axis;

        float rotate_amount;

        char has_script;
        object_type type;
        
        int num_children;
        struct graph_t** children;
    } graph;

    graph* parse_graph(char* scene_name);

    void make_current_scene(graph* scene);

    graph* get_current_scene();

    size_t get_current_uid();

    graph* search_subscene(graph* G, size_t id);

    typedef struct resource_t {
        unsigned int* indices;
        float* buffer;
        size_t numinds;
        size_t full_buffer_len;

        uint8_t* script;
        size_t script_len;
        
        char* v_shader_str;
        size_t v_shader_str_len;

        char* f_shader_str;
        size_t f_shader_str_len;
        
        uint8_t* texture_data;
        size_t texw;
        size_t texh;

        char is_light;
        char is_camera;

        char has_script;
        char hasmodel;
        char hastex;
        char hasvs;
        char hasfs;

    } resource;

    resource* load_resources();

    void make_current_resources(resource* res);

    resource* get_current_resources();

    size_t get_current_num_resources();
};

#endif