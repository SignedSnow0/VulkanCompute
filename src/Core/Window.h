#pragma once

#include <GLFW/glfw3.h>

/**
 * @brief Class representing a window.
 */
class Window {
public:
    /**
     * @brief Construct a new Window object
     *
     * @param width The width of the window.
     * @param height The height of the window.
     * @param title The title of the window.
     */
    Window(uint32_t width, uint32_t height, const char *title);
    ~Window();

    /**
     * @brief Polls the operating system for window events.
     *
     * Should be called at the beginning of each frame.
     */
    void PollEvents() const;

    /**
     *@brief Checks if the window should close.
     *
     * @return true if the window should close, false otherwise.
     */
    [[nodiscard]] bool ShouldClose() const;
    [[nodiscard]] inline GLFWwindow *Handle() const { return mWindow; }
    /**
     * @brief Gets the width of the window.
     *
     * @return The width of the window.
     */
    [[nodiscard]] uint32_t Width() const;
    /**
     * @brief Gets the height of the window.
     *
     * @return The height of the window.
     */
    [[nodiscard]] uint32_t Height() const;

    /**
     * @brief Checks if a specific key is currently pressed.
     *
     * Holding down the key will return true each time the method is called.
     *
     * @param key The key to check (GLFW key code).
     * @return true if the key is pressed, false otherwise.
     */
    [[nodiscard]] bool IsKeyPressed(int key);
    [[nodiscard]] bool IsKeyReleased(int key);
    [[nodiscard]] bool IsKeyDown(int key);

private:
    GLFWwindow* mWindow;

    bool mKeys[512]{ false };
};
