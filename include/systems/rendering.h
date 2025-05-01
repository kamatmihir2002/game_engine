#ifndef RENDERING_H__
#define RENDERING_H__

typedef struct container_texture {
    
    unsigned int fbo;
    unsigned int rbo;

    unsigned int vao;
    unsigned int vbo;
    unsigned int ibo;
    unsigned int shader;
    unsigned int screen_texture;
    unsigned int depth_texture;
    unsigned int sizex;
    unsigned int sizey;
    
} container_texture;


typedef struct camera_t {
    glm::vec3 position;
    glm::vec3 look_direction;
    glm::vec3 up;
    glm::mat4 view;
    glm::mat4 proj;
} camera;

typedef int ui5[5];

typedef struct rendering_t {
    size_t num_objects;
    unsigned int* shader_objects;
    unsigned int* vbos;
    unsigned int* vaos;
    unsigned int* ibos;
    unsigned int* numinds;
    ui5* attribs;

    // transformation
    unsigned int* transform_uniform;
    unsigned int* rotscale_uniform;
    unsigned int* view_uniform;
    unsigned int* proj_uniform;

    // shading
    unsigned int* lightdir_uniform;
    unsigned int* diffuse_uniform;
    unsigned int* specular_uniform;
    unsigned int* shininess_uniform;
    unsigned int* transparency_uniform;
    unsigned int* lsm_uniform;
    unsigned int* shadow_map_uniform;
    unsigned int* albedo_uniform;
    unsigned int* tex;

    //viewing
    unsigned int* campos_uniform;
    float* bound_radius;


    unsigned int* buffers_bound;

    glm::vec3* diffuse;
    glm::vec3* specular;
    glm::vec3* lightpos;

    float* shininess;

    int* attribLen;
    unsigned char* rendering_enabled;

    
    unsigned int pps_vbo;
    

    unsigned int pps_render_texture;
    unsigned int pps_framebuffer;
    unsigned int pps_renderbuffer;
    
    camera cam;
    int cam_id;

    container_texture* c;

} rendering;

rendering* create_rendering_system();

void make_current_rendering_system(rendering* sys);

void init_rendering_system();

rendering* get_current_rendering_system();

void update_rendering_system();

void prepare_rendering_system(size_t sizex, size_t sizey);

typedef struct lighting_t {
    size_t transform_id[8];
    camera props[8];
    float intensities[8];
    int num_lights;

    glm::vec3 transformed_position[8];
    glm::vec3 transformed_look_direction[8];
    glm::vec3 transformed_up_direction[8];

    glm::mat4 lsm[8];

    unsigned int depth_fbo;
    unsigned int depth_map;
    unsigned int depth_shader;
    unsigned int transform_uniform;
    unsigned int lsm_uniform;

} lighting;

lighting* create_lighting_system();

void make_current_lighting_system(lighting* light);

void init_lighting_system();

lighting* get_current_lighting_system();

void update_lighting_system();

#endif