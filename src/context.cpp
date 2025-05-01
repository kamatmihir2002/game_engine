#include <cstddef>
#include <context.h>

#include <glad/glad.h>
#include <glfw/glfw3.h>


void* context::create_window_handle(char* title, size_t xres, size_t yres) {
    
    glfwInit();
    GLFWwindow* w = glfwCreateWindow(xres, yres, title, NULL, NULL);
    if (w == NULL) {
        
        glfwTerminate();
        
    }
    return (void*)w;
}

void context::make_current(void* wHandle, size_t xres, size_t yres) {
    glfwMakeContextCurrent((GLFWwindow*)wHandle);
    gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
    glViewport(0, 0, xres, yres);
}

int context::is_open(void* wHandle) {
    return !glfwWindowShouldClose((GLFWwindow*)wHandle);
}

void context::swap_and_poll(void* wHandle) {
    glfwSwapBuffers((GLFWwindow*)wHandle);
    glfwPollEvents();
}