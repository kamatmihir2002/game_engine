#ifndef CONTEXT_H__
#define CONTEXT_H__

namespace context {
    void* create_window_handle(char* title, size_t xres, size_t yres);

    void make_current(void* wHandle, size_t xres, size_t yres);

    int is_open(void* wHandle);

    void swap_and_poll(void* wHandle);
};

#endif