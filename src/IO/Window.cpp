#include <Vonsai/IO/Window.hpp>

#include <Vonsai/IO/Input.hpp>

#include <mutex>

#include <Vonsai/Wraps/_glfw.hpp>

#include <imgui/imgui.h>
#include <imgui/imgui_impl_glfw.h>
#include <imgui/imgui_impl_opengl3.h>

namespace Vonsai {

#define GLFW_PTR static_cast<GLFWwindow *>(m_window)
#define IMGUI_PTR static_cast<ImGuiContext *>(m_gui)

Window::Window(Input *a_input, unsigned int a_width, unsigned int a_height) {

  // Request a new window to GLFW
  m_window = glfwCreateWindow(a_width, a_height, m_title.c_str(), nullptr, nullptr);
  if (!m_window) {
    vo_err("Couldn't create window {}", m_title);
    return;
  }
  m_title += vo_fmt(" :: Window {}", ++s_counter);
  glfwSetWindowTitle(GLFW_PTR, m_title.c_str());

  // Window is valid, load graphic context on it
  glfwMakeContextCurrent(GLFW_PTR);
  if (!GL::loadExtensions(reinterpret_cast<void *>(glfwGetProcAddress))) {
    vo_err("Couldn't load OpenGL extensions");
    glfwDestroyWindow(GLFW_PTR);
    m_window = nullptr;
    return;
  }
  GL::defaultSetup();

  // Window and Graphic context are valid, save variables
  m_valid  = true;
  m_width  = a_width;
  m_height = a_height;
  glfwSwapInterval(0); // Disable V-Sync

  // Attach ImGui
  static std::once_flag initImGUI;
  std::call_once(initImGUI, [&m_gui = m_gui, &m_window = m_window]() {
    m_gui = ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplOpenGL3_Init("#version 410"); // TODO : BASED ON SETTINGS
    ImGui_ImplGlfw_InitForOpenGL(GLFW_PTR, true);
  });

  // Link input callbacks
  InputAttorney::linkCallbacks(a_input, m_window);

  // Set window callbacks
  glfwSetWindowUserPointer(GLFW_PTR, this);
  glfwSetWindowSizeCallback(GLFW_PTR, [](GLFWwindow *ptr, int width, int height) {
    auto curr = static_cast<Window *>(glfwGetWindowUserPointer(ptr));
    onWindowResize(static_cast<float>(width), static_cast<float>(height), *curr);
  });
  glfwSetWindowFocusCallback(GLFW_PTR, [](GLFWwindow *ptr, int focused) {
    auto curr = static_cast<Window *>(glfwGetWindowUserPointer(ptr));
    onWindowFocus(static_cast<bool>(focused), *curr);
  });
  glfwSetWindowCloseCallback(GLFW_PTR, [](GLFWwindow *ptr) {
    auto curr = static_cast<Window *>(glfwGetWindowUserPointer(ptr));
    onWindowDestroy(*curr);
  });
}

bool  Window::isValid() const { return m_valid; }
bool  Window::isFocused() const { return m_focused; }
float Window::getAspectRatio() const { return m_width / m_height; }

bool Window::update(std::function<unsigned int(void)> const &a_onUpdate, std::function<void(void)> const &a_onGui) {

  if (!m_focused) { glfwWaitEvents(); }
  activate();
  glfwPollEvents();

  unsigned int const FPS = a_onUpdate();
  glfwSetWindowTitle(GLFW_PTR, vo_fmt("{} :: {} fps", m_title, FPS).c_str());

  if (ImGui::GetCurrentContext() == IMGUI_PTR) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    a_onGui();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
  }

  glfwSwapBuffers(GLFW_PTR);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  return m_valid;
}

void Window::close() { destroy(); }

void Window::shutdown() {
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();
  glfwTerminate();
}

void Window::activate() {
  glfwMakeContextCurrent(GLFW_PTR);
  // ImGui::SetCurrentContext(IMGUI_PTR);
  // ImGui_ImplGlfw_ChangeMainWindow(GLFW_PTR);
}

void Window::destroy() {
  m_valid = false;
  glfwDestroyWindow(GLFW_PTR);
}

void onWindowDestroy(Window &a_window) { a_window.m_valid = false; }
void onWindowFocus(bool a_focused, Window &a_window) { a_window.m_focused = a_focused; }
void onWindowResize(float a_width, float a_height, Window &a_window) {
  a_window.m_width  = a_width;
  a_window.m_height = a_height;
}

} // namespace Vonsai