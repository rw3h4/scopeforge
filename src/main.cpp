#include <GLFW/glfw3.h>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "implot.h"

#include <cmath>
#include <cstdio>
#include <chrono>
#include <vector>
#include <iostream>
#include <random>

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

struct SignalParams {
  float frequency = 50.0f;        // Hz, mains fundamental
  float amplitude = 230.0f;       // Vrms of fundamental
  float h3_percentage = 0.0f;     // 3rd harmonic, % of fundamental 
  float h5_percentage = 0.0f;     // 5th harmonic, % of fundamental
  float h7_percentage = 0.0f;     // 7th harmonic, % of fundamental
  float noise_amplitude = 0.0f;   // Vrms of gaussian noise
  bool paused = false;
};

// One RNG for noise. Fine as a file scope object for now. Becomes a member 
// of the generator when threaded 
static std::mt19937 rng{std::random_device{}()};
static std::normal_distribution<float> gaussian{0.0f, 1.0f};

float generateSample(const SignalParams& prm, float t) {
  const float w = 2.0f * static_cast<float>(M_PI) * prm.frequency; // angular frequency 
  const float peak = prm.amplitude * std::sqrt(2.0f);

  float value = peak * std::sin(w * t);
  value += peak * (prm.h3_percentage / 100.0f) * std::sin(3.0f * w * t);
  value += peak * (prm.h5_percentage / 100.0f) * std::sin(5.0f * w * t);
  value += peak * (prm.h7_percentage / 100.0f) * std::sin(7.0f * w * t);

  if (prm.noise_amplitude > 0.0f) {
    value += prm.noise_amplitude * std::sqrt(2.0f) * gaussian(rng);
  }

  return value;
}

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
  ScrollingBuffer reference(4000);
  SignalParams params;

  const float sample_rate = 10000.0f;   // 10kHz, stays constant, not a UI knob 

  auto start_time = std::chrono::steady_clock::now();
  double last_sample_time = 0.0;

  while (!glfwWindowShouldClose(window)) {
    glfwPollEvents();

    // Generate new samples since the last frame 
    // TODO: Will move this to a thread
    auto now = std::chrono::steady_clock::now();
    double current_time = std::chrono::duration<double>(now - start_time).count();
    const double sample_period = 1.0 / sample_rate;

    // Clamp backlog to 100ms to avoid a catch-up spin after OS suspend
    if (current_time - last_sample_time > 0.1)
      last_sample_time = current_time - 0.1;

    while (last_sample_time + sample_period < current_time) {
      last_sample_time += sample_period;
      if (!params.paused) {
        // Distorted composite, fundamental + harmonics + noise 
        waveform.add(last_sample_time, generateSample(params, last_sample_time));

        // Clean reference, fundamental only 
        float clean = params.amplitude * std::sqrt(2.0f) *
          std::sin(2.0f * static_cast<float>(M_PI) * params.frequency * last_sample_time
            );
        reference.add(last_sample_time, clean);
      }
    }

    // ImGui frame 
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("ScopeForge - Real-time Oscilloscope");
    ImGui::Text("Synthetic mains generator - adjust and watch the waveform respond");
    ImGui::Text("Samples buffered: %d", (int)waveform.t.size());
    ImGui::Spacing();

    ImGui::SeparatorText("Fundamental");
    ImGui::SliderFloat("Frequency (Hz)", &params.frequency, 40.0f, 60.0f, "%.1f");
    ImGui::SliderFloat("Amplitude (Vrms)", &params.amplitude, 0.0f, 300.0f, "%.1f");

    ImGui::SeparatorText("Harmonic distortion (% of fundamental)");
    ImGui::SliderFloat("3rd harmonic", &params.h3_percentage, 0.0f, 40.0f, "%.1f%%");
    ImGui::SliderFloat("5th harmonic", &params.h5_percentage, 0.0f, 40.0f, "%.1f%%");
    ImGui::SliderFloat("7th harmonic", &params.h7_percentage, 0.0f, 40.0f, "%.1f%%");

    ImGui::SeparatorText("Noise");
    ImGui::SliderFloat("Gaussian noise (Vrms)", &params.noise_amplitude, 0.0f, 30.0f, "%.1f");

    ImGui::Spacing();
    ImGui::Checkbox("Pause", &params.paused);
    ImGui::SameLine();
    if (ImGui::Button("Reset to clean 50Hz")) {
      params = SignalParams{};
      waveform = ScrollingBuffer(4000);
      reference = ScrollingBuffer(4000);
      last_sample_time = current_time;
    }

    if (ImPlot::BeginPlot("Voltage Waveform", ImVec2(-1, 500))) {
      ImPlot::SetupAxes("Time (s)", "Voltage (V)", ImPlotAxisFlags_AutoFit, 0);
      ImPlot::SetupAxisLimits(ImAxis_X1, current_time - 0.1,current_time, ImGuiCond_Always);
      ImPlot::SetupAxisLimits(ImAxis_Y1, -400, 400, ImGuiCond_Once);

      if (!reference.t.empty()) {
        char ref_label[32];
        std::snprintf(ref_label, sizeof(ref_label), "Clean %.0f Hz", (double)params.frequency);
        ImPlot::PlotLine(ref_label,
                          reference.t.data(),
                          reference.y.data(),
                          (int)reference.t.size(),
                          ImPlotSpec(ImPlotProp_Offset, reference.offset));
      }
      if (!waveform.t.empty()) {
        ImPlot::PlotLine("Distorted mains",
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
