#include <engine.h>

int main(int argc, char** argv) {
    engine::application* app = engine::application_create(argv[1],  1920, 1080);
    engine::application_init(app);
    engine::application_run(app);
}