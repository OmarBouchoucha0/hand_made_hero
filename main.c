#include <GLFW/glfw3.h>
#include <stdio.h>
#include <stdlib.h>

int main(void) {
  // Optional: force Wayland platform (GLFW 3.4+)
  // glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_WAYLAND);

  if (!glfwInit()) {
    fprintf(stderr, "Failed to initialize GLFW\n");
    return EXIT_FAILURE;
  }

  // Create a windowed mode window and its OpenGL context
  GLFWwindow *window = glfwCreateWindow(640, 480, "Hello Wayland", NULL, NULL);
  if (!window) {
    fprintf(stderr, "Failed to create GLFW window\n");
    glfwTerminate();
    return EXIT_FAILURE;
  }

  // Make the window's context current
  glfwMakeContextCurrent(window);

  // Loop until the user closes the window
  while (!glfwWindowShouldClose(window)) {
    // Render here (clear to dark gray)
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // Swap front and back buffers
    glfwSwapBuffers(window);

    // Poll for and process events
    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return EXIT_SUCCESS;
}
