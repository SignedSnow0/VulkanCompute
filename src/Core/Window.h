#pragma once

#include <GLFW/glfw3.h>

class Window {
public:
    Window(uint32_t width, uint32_t height, const char* title);
    ~Window();

    bool shouldClose() const;
    void pollEvents() const;
    [[nodiscard]] inline GLFWwindow* Handle() const { return mWindow; }
    [[nodiscard]] uint32_t Width() const;
    [[nodiscard]] uint32_t Height() const;

    bool IsKeyPressed(int key) const;

private:
    GLFWwindow* mWindow;
};
