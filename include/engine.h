
#ifndef ENGINE_H__
#define ENGINE_H__

#include <cstddef>

#include <context.h>
#include <scene.h>
#include <systems.h>

namespace engine {

    typedef struct application_t {
        size_t  xres;
        size_t  yres;
        void*   wHandle;
        scene::graph*       scene;
        scene::resource*    resources;
        systems::transform* transform_system;
        systems::rendering* render_system;
        systems::lighting*  lighting_system;
        systems::scripting* script_system;

    } application;

    application* application_create(char* scene_name, size_t xres, size_t yres);

    void application_init(application* app);

    void application_run(application* app);

};

#endif