#include "Window.h"
#include "Logger.h"

Window::Window(int width, int height, const char *title) {
    if (!glfwInit()) {
        mWindow = nullptr;
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    mWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);

    LOG_DEBUG(std::format("Created window '{}' ({}x{})", title, width, height));
}

Window::~Window() {
    if (mWindow) {
        glfwDestroyWindow(mWindow);
        mWindow = nullptr;
    }
    glfwTerminate();
}

bool Window::shouldClose() const {
    return mWindow ? glfwWindowShouldClose(mWindow) : true;
}

void Window::pollEvents() const { glfwPollEvents(); }

uint32_t Window::Width() const {
    int width, height;
    glfwGetWindowSize(mWindow, &width, &height);
    return static_cast<uint32_t>(width);
}

uint32_t Window::Height() const {
    int width, height;
    glfwGetWindowSize(mWindow, &width, &height);
    return static_cast<uint32_t>(height);
}
