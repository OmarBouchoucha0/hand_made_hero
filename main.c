#include <GLFW/glfw3.h>

int main(void) {

  if (!glfwInit()) {
    return 1;
  }

  GLFWwindow *window = glfwCreateWindow(640, 480, "handmae", NULL, NULL);
  if (!window) {
    glfwTerminate();
    return 1;
  }

  glfwMakeContextCurrent(window);

  while (!glfwWindowShouldClose(window)) {
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glfwSwapBuffers(window);

    glfwPollEvents();
  }

  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
