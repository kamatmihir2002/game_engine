#ifndef TRANSFORM_H__
#define TRANSFORM_H__

typedef struct transform_t {
    size_t num_objects;
    glm::mat4* T;
    glm::mat4* localT;
    glm::mat4* RS;
    glm::mat4* localRS;
} transform;

transform* create_transform_system();

void make_current_transform_system(transform* sys);

transform* get_current_transform_system();

void init_transform_system();

void update_transform_system();

void translate_subscene(transform* system, scene::graph* scene, size_t from_id, glm::vec3 translation);

void rotate_subscene(transform* system, scene::graph* scene, size_t from_id, float angle, glm::vec3 axis);

void scale_subscene(transform* system, scene::graph* scene, size_t from_id, glm::vec3 scale);

#endif