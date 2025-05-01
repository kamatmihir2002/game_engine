#include <scene.h>
#include <cstdio>
#include <cstdint> 
#include <cstdlib>
#include <cstring>

#include <base.h>

#define STB_JSON_IMPLEMENTATION
#include <stb/stb_json.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

scene::graph* current_scene;

size_t uid = 0;
size_t generate_uid() {
    return uid++;
}

size_t scene::get_current_uid() {
    return uid;
}


scene::resource* current_resources;
int num_current_resources;


glm::vec3 read_vec3(stb_json cursor, char* name) {
    stb_json crs = cursor.MoveCursor(name);
    glm::vec3 vecp;
    vecp[0] = crs.GetDouble(0);
    vecp[1] = crs.GetDouble(1);
    vecp[2] = crs.GetDouble(2);
    return vecp;
}

scene::graph* scene::search_subscene(scene::graph* G, size_t id) {
    if (G->id == id) {
        return G;
    }
    else {
        for (int i = 0; i < G->num_children; i++) {
            scene::graph* Gc = scene::search_subscene(G->children[i], id);
            if (Gc)
                return Gc;
        }
    }
    return NULL;
}


scene::graph* _internal_parse_graph(stb_json cursor, int idx, scene::graph* parent) {
    scene::graph* G = (scene::graph*)malloc(sizeof(scene::graph));
    G->parent = parent;
    stb_json nc = cursor;
    nc = nc.MoveCursor(idx);

    G->id = generate_uid();

    G->position = read_vec3(nc, "position");
    G->scale = read_vec3(nc, "scale");
    G->rotate_axis = read_vec3(nc, "rotation_axis");
    G->rotate_amount = nc.GetDouble("rotation_amount");

    nc.GetString("name", G->object_name, 64);
    nc.GetString("resource", G->resource_name, 64);
    nc.GetString("script", G->script_name, 64);
    
    G->type = scene::GENERIC;

    if (strcmp(G->resource_name, "camera") == 0) {
        G->type = scene::CAMERA;
    }
    if (strcmp(G->resource_name, "light") == 0) {
        G->type = scene::LIGHT;
    }

    nc = nc.MoveCursor("children");
    if (!nc.HasError()) {
        G->num_children = nc.Count();
        G->children = (scene::graph**)malloc(sizeof(scene::graph*) * G->num_children);
        for (int i = 0; i < G->num_children; i++) {
            G->children[i] = _internal_parse_graph(nc, i, G);
        }
    }
    else {
        G->num_children = 0;
        G->children = NULL;
    }
    return G;
}

scene::graph* scene::parse_graph(char* scene_name) {
    size_t sz;
    char* buf = base::io_open_and_read_file(scene_name, &sz, "rb");
    stb_json cursor = stb_json(buf, sz + 1);
    return _internal_parse_graph(cursor, 0, NULL);
}

void scene::make_current_scene(scene::graph* scene) {
    current_scene = scene;

}

scene::graph* scene::get_current_scene() {
    return (scene::graph*)current_scene;
}

#define SUCCESS 1
#define FAIL 0

char* res_folder_path = "data";

typedef enum resource_type {
    MODEL=0,
    TEXTURE,
    VSHADER,
    FSHADER,
    SCRIPT
} resource_type;

int read_mesh(char* path, float** full_buffer, size_t* full_buf_len, unsigned int** indices, size_t* inds_len) {
    char* modelpath = path;

    FILE* f = fopen(modelpath, "rb");
    if (f) {
        
        char eat;
        
        unsigned int numverts;
        unsigned int numnorms;
        unsigned int numinds;
        
        fread(&numverts, sizeof(unsigned int), 1, f);
        fread(&eat, sizeof(char), 1, f);
        fread(&numnorms, sizeof(unsigned int), 1, f);
        fread(&eat, sizeof(char), 1, f);
        fread(&numinds, sizeof(unsigned int), 1, f);
        fread(&eat, sizeof(char), 1, f);
        
        float* verts = (float*)malloc(sizeof(float) * numverts * 3);
        float* norms = (float*)malloc(sizeof(float) * numverts * 3);
        float* tex = (float*)malloc(sizeof(float) * numverts * 2);
        *full_buf_len = (size_t)(numverts * 8);
        *inds_len = (size_t)(numinds);
        (*full_buffer) = (float*)malloc(sizeof(float) * numverts * 8);
        (*indices) = (unsigned int*)malloc(sizeof(unsigned int) * numinds);
        unsigned int* inds = (*indices);
        for (int i = 0; i < numverts; i++) {
            fread(&verts[i * 3], sizeof(float), 1, f);
            fread(&verts[i * 3 + 1], sizeof(float), 1, f);
            fread(&verts[i * 3 + 2], sizeof(float), 1, f);
        }
        for (int i = 0; i < numnorms; i++) {
            fread(&norms[i * 3], sizeof(float), 1, f);
            fread(&norms[i * 3 + 1], sizeof(float), 1, f);
            fread(&norms[i * 3 + 2], sizeof(float), 1, f);
        
        }
        for (int i = 0; i < numverts; i++) {
            fread(&tex[i * 2], sizeof(float), 1, f);
            fread(&tex[i * 2 + 1], sizeof(float), 1, f);
            
        }
        for (int i = 0; i < numnorms; i++) {
            (*full_buffer)[i * 8 + 0] = verts[i * 3];
            (*full_buffer)[i * 8 + 1] = verts[i * 3 + 1];
            (*full_buffer)[i * 8 + 2] = verts[i * 3 + 2];
            (*full_buffer)[i * 8 + 3] = norms[i * 3];
            (*full_buffer)[i * 8 + 4] = norms[i * 3 + 1];
            (*full_buffer)[i * 8 + 5] = norms[i * 3 + 2];
            (*full_buffer)[i * 8 + 6] = tex[i * 2];
            (*full_buffer)[i * 8 + 7] = tex[i * 2 + 1];
           
        }
        free(verts);
        free(norms);
        free(tex);
        for (int i = 0; i < numinds; i++) {
            fread(&inds[i], sizeof(unsigned int), 1, f);
           
        }
        
        
        return SUCCESS;
    }
    return FAIL;
}

int read_bin(char* path, uint8_t** buffer, size_t* size, char* mode) {
    FILE* f = fopen(path, mode);
    if (f) {
        (*buffer) = (uint8_t*)base::io_read_file(f, size, mode);
        return SUCCESS;
    }
    return FAIL;
}

int read_tex(char* path, uint8_t** texdata, size_t* texw, size_t* texh) {
    int channels = 0;
    int w = 0, h = 0;
    FILE* f = fopen(path, "rb");
    if (f) {
        fclose(f);
        stbi_set_flip_vertically_on_load(1);
        (*texdata) = stbi_load(path, &w, &h, &channels, 0);
        *texw = w;
        *texh = h;
        return SUCCESS;
    }
    return FAIL;
}

int read_resource(char* path, scene::resource* res, resource_type type) {

    int status = SUCCESS;
    switch (type) {
        case MODEL:
            return read_mesh(path, &(res->buffer), &(res->full_buffer_len), &(res->indices), &(res->numinds));
        case TEXTURE:
            return read_tex(path, &(res->texture_data), &(res->texw), &(res->texh));
        case VSHADER:
            status = read_bin(path, (uint8_t**)&(res->v_shader_str), &(res->v_shader_str_len), "rb");
            if (status)
                res->v_shader_str[res->v_shader_str_len] = 0;
            return status;
        case FSHADER:
            status = read_bin(path, (uint8_t**)&(res->f_shader_str), &(res->f_shader_str_len), "rb");
            if (status)
                res->f_shader_str[res->f_shader_str_len] = 0;
            return status;
        case SCRIPT:
            status = read_bin(path, (uint8_t**)&(res->script), &(res->script_len), "rb");
            if (status)
                res->script[res->script_len] = 0;
            return status;
    }
    return FAIL;
}

void _internal_load_resource(scene::resource* res, char* object_name, char* res_name, char* script_name) {
    
    char path[128];
    char scripts_folder_path[128];
    char mdlpath[128];
    char texpath[128];
    char vspath[128];
    char fspath[128];
    char scriptpath[128];

    snprintf(path, 128, "%s/res/%s/", res_folder_path, res_name);
    snprintf(scripts_folder_path, 128, "%s/scripts/", res_folder_path);
    snprintf(mdlpath, 128, "%s%s.bin", path, res_name);
    snprintf(texpath, 128, "%s%s.png", path, res_name);
    snprintf(vspath, 128, "%s%s.vs", path, res_name);
    snprintf(fspath, 128, "%s%s.fs", path, res_name);
    snprintf(scriptpath, 128, "%s%s", scripts_folder_path, script_name);
    
    if (res_name[0] == 0) {
        res->hasmodel = 0;
        res->hastex = 0;
        res->hasvs = 0;
        res->hasfs = 0;
        res->has_script = 0;
    }
    else {
        res->hasmodel = 1;
        res->hastex = 1;
        res->hasvs = 1;
        res->hasfs = 1;
        res->has_script = 1;
        if (!read_resource(mdlpath, res, MODEL)) {
            printf("WARNING: for object %s\n", object_name);
            printf("No model found.\n");
            res->hasmodel = 0;
        }
        if (!read_resource(texpath, res, TEXTURE)) {
            printf("WARNING: for object %s\n", object_name);
            printf("No texture found.\n");
            res->hastex = 0;
        }
        if (!read_resource(vspath, res, VSHADER)) {
            printf("WARNING: for object %s\n", object_name);
            printf("No vertex shader found.\n");
            res->hasvs = 0;
        }
        if (!read_resource(fspath, res, FSHADER)) {
            printf("WARNING: for object %s\n", object_name);
            printf("No fragment shader found.\n");
            res->hasfs = 0;
        }
        if (!read_resource(scriptpath, res, SCRIPT)) {
            printf("WARNING: for object %s\n", object_name);
            printf("No script found.\n");
            res->has_script= 0;
        }
    }
}

void _internal_load_resource_recursive(scene::graph* G, scene::resource* res_array) {
    size_t idx = G->id;
    scene::resource* rp = &res_array[idx];
    rp->is_camera = (G->type == scene::CAMERA);
    rp->is_light = (G->type == scene::LIGHT);
    _internal_load_resource(rp, G->object_name, G->resource_name, G->script_name);
    for (int i = 0; i < G->num_children; i++) {
        _internal_load_resource_recursive(G->children[i], res_array);
    }
}

scene::resource* scene::load_resources() {
    scene::resource* res_array = (scene::resource*)malloc(sizeof(scene::resource) * get_current_uid());
    scene::graph* G = scene::get_current_scene();
    _internal_load_resource_recursive(G, res_array);
    return res_array;
}

void scene::make_current_resources(scene::resource* res) {
    current_resources = res;
    num_current_resources = get_current_uid();
}

scene::resource* scene::get_current_resources() {
    return (scene::resource*)current_resources;
}

size_t scene::get_current_num_resources() {
    return get_current_uid();
}