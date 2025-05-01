#include <engine.h>
#include <cstdlib>

engine::application* engine::application_create(char* scene_name, size_t xres, size_t yres) {
    engine::application* app = (engine::application*)malloc(sizeof(engine::application));

    app->xres = xres;
    app->yres = yres;
    
    app->wHandle = context::create_window_handle(scene_name, xres, yres);
    context::make_current(app->wHandle, xres, yres);

    app->scene = scene::parse_graph(scene_name);
    scene::make_current_scene(app->scene);

    app->resources = scene::load_resources();
    scene::make_current_resources(app->resources);

    app->transform_system   =   systems::create_transform_system();
    systems::make_current_transform_system(app->transform_system);

    app->render_system      =   systems::create_rendering_system();
    systems::make_current_rendering_system(app->render_system);

    app->lighting_system    =   systems::create_lighting_system();
    systems::make_current_lighting_system(app->lighting_system);

    app->script_system      =   systems::create_scripting_system();
    systems::make_current_scripting_system(app->script_system);

    return app;
}

void engine::application_init(engine::application* app) {
    scene::make_current_scene(app->scene);
    scene::make_current_resources(app->resources);
    systems::make_current_transform_system(app->transform_system);
    systems::make_current_rendering_system(app->render_system);
    systems::make_current_lighting_system(app->lighting_system);
    systems::make_current_scripting_system(app->script_system);
    systems::init_transform_system();
    systems::init_lighting_system();
    systems::init_rendering_system();
    systems::init_scripting_system();
}

void engine::application_run(engine::application* app) {

    while (context::is_open(app->wHandle)) {
        systems::start_time();

        systems::update_lighting_system();
        // systems::prepare_rendering_system(app->xres, app->yres);
        systems::update_rendering_system();
        
        systems::set_frame_time(systems::end_time());

        systems::update_scripting_system();
        context::swap_and_poll(app->wHandle);
    }

}
