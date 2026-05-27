#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include <cmath>
#include <chrono>
#include <vector>
#include <iostream>

// Circular buffer for the waveform data
struct ScrollingBuffer {
  int max_size;
  int offset = 0;
  std::vector<float> t;  // time 
  std::vector<float> y;

  ScrollingBuffer(int max = 2000) : max_size(max) {
    t.reserve(max_size);
    y.reserve(max_size);
  }

  void add(float time, float value) {
    if ((int)t.size() < max_size) {
      t.push_back(time);
      y.push_back(value);
    } else {
      t[offset] = time;
      y[offset] = value;
      offset = (offset + 1) % max_size;
    }
  }
};

int main() {
  // GLFW + OpenGL window setup 
  if (!glfwInit()) {
    std::cerr << "GLFW init failed\n";
    return 1;
  }
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow* window = glfwCreateWindow(1400, 800, "ScopeForge", nullptr, nullptr);
  if (!window) {
    std::cerr << "Failed to create GLFW window\n";
    glfwTerminate();
    return 1;
  }
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1);  // vsync 
  
  // ImGui setup
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImPlot::CreateContext();
  ImGui::StyleColorsDark();
  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");

  ScrollingBuffer waveform(4000);

  // Simulation parameters 
  const float sample_rate = 10000.0f; // 10kHz
  const float frequency = 50.0f;
  const float amplitude = 230.0f * std::sqrt(2.0f); // 230V RMS peak 

  auto start_time = std::chrono::steady_clock::now();
  float last_sample_time = 0.0f;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Generate new samples since the last frame 
    // TODO: Will move this to a thread
    auto now = std::chrono::steady_clock::now();
    float current_time = std::chrono::duration<float>(now - start_time).count();
    const float sample_period = 1.0f / sample_rate;

    while (last_sample_time + sample_period < current_time) {
      last_sample_time += sample_period;
      float value = amplitude * std::sin(2.0f * M_PI * frequency * last_sample_time);
      waveform.add(last_sample_time, value);
    }

    // ImGui frame 
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("ScopeForge - Real-time Oscilloscope");
    ImGui::Text("Synthetic 50Hz / 230Vrms mains signal");
    ImGui::Text("Samples buffered: %zu", waveform.t.size());

    if (ImPlot::BeginPlot("Voltage Waveform", ImVec2(-1, 500))) {
      ImPlot::SetupAxes("Time (s)", "Voltage (V)", ImPlotAxisFlags_AutoFit, 0);
      ImPlot::SetupAxisLimits(ImAxis_X1, current_time - 0.1,current_time, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, -400, 400, ImGuiCond_Once);

      if (!waveform.t.empty()) {
        ImPlot::PlotLine("V(t)",
                          waveform.t.data(),
                          waveform.y.data(),
                          (int)waveform.t.size(),
                          ImPlotSpec(ImPlotProp_Offset, waveform.offset)
        );
      }
      ImPlot::EndPlot();
    }
    ImGui::End();

    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(window);
  }

  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImPlot::DestroyContext();
  ImGui::DestroyContext();
  glfwDestroyWindow(window);
  glfwTerminate();
  return 0;
}
