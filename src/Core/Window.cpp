#include "Window.h"

#include "Core/Logger.h"

Window::Window(uint32_t width, uint32_t height, const char *title) {
    if (!glfwInit()) {
        mWindow = nullptr;
        return;
    }

    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);
    mWindow =
        glfwCreateWindow(static_cast<int>(width), static_cast<int>(height),
                         title, nullptr, nullptr);

    LOG_INFO("Created window '{}' ({}x{})", title, width, height);
}

Window::~Window() {
    glfwDestroyWindow(mWindow);
    glfwTerminate();
}

bool Window::ShouldClose() const {
    return mWindow ? glfwWindowShouldClose(mWindow) : true;
}

void Window::PollEvents() const { glfwPollEvents(); }

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

bool Window::IsKeyPressed(int key) {
    if (key < 0 || key >= 512) {
        return false;
    }
    bool lastState = mKeys[key];
    mKeys[key] = glfwGetKey(mWindow, key) == GLFW_PRESS;

    return !lastState && (glfwGetKey(mWindow, key) == GLFW_PRESS);
}

bool Window::IsKeyReleased(int key) {
    if (key < 0 || key >= 512) {
        return false;
    }
    bool lastState = mKeys[key];
    mKeys[key] = glfwGetKey(mWindow, key) == GLFW_PRESS;

    return lastState && (glfwGetKey(mWindow, key) == GLFW_RELEASE);
}

bool Window::IsKeyDown(int key) {
    return glfwGetKey(mWindow, key) == GLFW_PRESS;
}