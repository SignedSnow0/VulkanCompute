#pragma once

#include <GLFW/glfw3.h>

class Window {
  public:
    Window(int width, int height, const char *title);
    ~Window();

    bool shouldClose() const;
    void pollEvents() const;
    [[nodiscard]] inline GLFWwindow *Handle() const { return mWindow; }
    [[nodiscard]] uint32_t Width() const;
    [[nodiscard]] uint32_t Height() const;

  private:
    GLFWwindow *mWindow;
};
