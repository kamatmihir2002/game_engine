#include <systems.h>
#include <scene.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

systems::transform* current_transform_system;

void create_transform(systems::transform* transform, size_t id, glm::vec3 translate, glm::vec3 rot_axis, float angle, glm::vec3 scale, glm::mat4 T_parent, glm::mat4 R_parent) {
    transform->localT[id] = transform->T[id];
    transform->localT[id] = glm::translate(transform->localT[id], translate);
    transform->localT[id] = glm::rotate(transform->localT[id], angle, rot_axis);
    transform->localT[id] = glm::scale(transform->localT[id], scale);
    transform->T[id] = T_parent * transform->T[id];
    transform->T[id] = glm::translate(transform->T[id], translate);
    transform->T[id] = glm::rotate(transform->T[id], angle, rot_axis);
    transform->T[id] = glm::scale(transform->T[id], scale);
    transform->localRS[id] = transform->RS[id];
    transform->localRS[id] = glm::rotate(transform->localRS[id], angle, rot_axis);
    transform->RS[id] = R_parent * transform->RS[id];
    transform->RS[id] = glm::rotate(transform->RS[id], angle, rot_axis);
    
}

void _internal_create_transform_system_recursive(systems::transform* transform, scene::graph* graph, glm::mat4 parentT, glm::mat4 parentR) {
    create_transform(transform, graph->id, graph->position, graph->rotate_axis, graph->rotate_amount, graph->scale, parentT, parentR);
    for (int i = 0; i < graph->num_children; i++) {
        _internal_create_transform_system_recursive(transform, graph->children[i], transform->T[graph->id], transform->RS[graph->id]);
    }
}

systems::transform* systems::create_transform_system() {
    scene::graph* G = scene::get_current_scene();
    systems::transform* transform = (systems::transform*)malloc(sizeof(systems::transform));
    size_t num_nodes = scene::get_current_uid();
    transform->num_objects = num_nodes;
    transform->T = (glm::mat4*)malloc(sizeof(glm::mat4) * num_nodes);
    transform->localT = (glm::mat4*)malloc(sizeof(glm::mat4) * num_nodes);
    for (int i = 0; i < transform->num_objects; i++) {
        transform->T[i] = glm::mat4(1.0);
        transform->localT[i] = glm::mat4(1.0);
    }

    transform->RS = (glm::mat4*)malloc(sizeof(glm::mat4) * num_nodes);
    transform->localRS = (glm::mat4*)malloc(sizeof(glm::mat4) * num_nodes);
    for (int i = 0; i < transform->num_objects; i++) {
        transform->RS[i] = glm::mat4(1.0);
        transform->localRS[i] = glm::mat4(1.0);
    }
    _internal_create_transform_system_recursive(transform, G, glm::mat4(1.0), glm::mat4(1.0));
    return transform;
}

void systems::make_current_transform_system(systems::transform* sys) {
    current_transform_system = sys;
}

void systems::init_transform_system() {

}

systems::transform* systems::get_current_transform_system() {
    return (systems::transform*)current_transform_system;
}


typedef enum TTYPE {
    TRANSLATE = 0,
    ROTATE,
    SCALE
} transform_type;

void do_subscene_rec(systems::transform* system, scene::graph* scene, size_t from_id, glm::vec3 TRS, float angle, glm::mat4 parentT, glm::mat4 parentRS, transform_type trans_type) {
    if (from_id == scene->id) {
        switch(trans_type) {
            case TRANSLATE:
                system->T[from_id] = glm::translate(system->T[from_id], TRS);
                system->localT[from_id] = glm::translate(system->localT[from_id], TRS);
                break;
            case ROTATE:
                system->T[from_id] = glm::rotate(system->T[from_id], angle, TRS);
                system->RS[from_id] = glm::rotate(system->RS[from_id], angle, TRS);
                system->localT[from_id] = glm::rotate(system->localT[from_id], angle, TRS);
                system->localRS[from_id] = glm::rotate(system->localRS[from_id], angle, TRS);
                break;
            case SCALE:
                system->T[from_id] = glm::scale(system->T[from_id], TRS);
                system->localT[from_id] = glm::scale(system->localT[from_id], TRS);
                break;
        }
    }
    else {
        system->T[scene->id] = parentT * system->localT[scene->id];
        
        if (trans_type == ROTATE)
            system->RS[scene->id] = parentRS * system->localRS[scene->id];
    }
    for (int i = 0; i < scene->num_children; i++) {
        do_subscene_rec(system, scene->children[i], from_id, TRS, angle, system->T[from_id], system->RS[from_id], trans_type);
    }
}

void systems::translate_subscene(systems::transform* system, scene::graph* scene, size_t from_id, glm::vec3 translation) {
    scene::graph* Gc = scene::search_subscene(scene, from_id);
    do_subscene_rec(system, Gc, from_id, translation, 0.0, glm::mat4(1.0), glm::mat4(1.0), TRANSLATE);
}

void systems::rotate_subscene(systems::transform* system, scene::graph* scene, size_t from_id, float angle, glm::vec3 axis) {
    scene::graph* Gc = scene::search_subscene(scene, from_id);
    do_subscene_rec(system, Gc, from_id, axis, angle, glm::mat4(1.0), glm::mat4(1.0), ROTATE);
}

void systems::scale_subscene(systems::transform* system, scene::graph* scene, size_t from_id, glm::vec3 scale) {
    scene::graph* Gc = scene::search_subscene(scene, from_id);
    do_subscene_rec(system, Gc, from_id, scale, 0.0, glm::mat4(1.0), glm::mat4(1.0), SCALE);
}

void systems::update_transform_system() {

}