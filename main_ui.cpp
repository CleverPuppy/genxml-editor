#include "main_ui.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include "xml_parser.h"
#include "xml_saver.h"
#include "xml_types.h"
#include <algorithm>
#include <cstddef>
#include <cstdio>
#include <future>
#include <memory>
#include <queue>
#include <stdio.h>
#include <vector>
#define GL_SILENCE_DEPRECATION
#if defined(IMGUI_IMPL_OPENGL_ES2)
#include <GLES2/gl2.h>
#endif
#include <GLFW/glfw3.h> // Will drag system OpenGL headers

constexpr size_t MAX_PATH_SIZE = 4096;

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to
// maximize ease of testing and compatibility with old VS compilers. To link
// with VS2010-era libraries, VS2015+ requires linking with
// legacy_stdio_definitions.lib, which we do using this pragma. Your own project
// should not be affected, as you are likely to link with a newer binary of GLFW
// that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) &&                                 \
    !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

// This example can also compile and run with Emscripten! See
// 'Makefile.emscripten' for details.
#ifdef __EMSCRIPTEN__
#include "../libs/emscripten/emscripten_mainloop_stub.h"
#endif

static void glfw_error_callback(int error, const char *description) {
  fprintf(stderr, "GLFW Error %d: %s\n", error, description);
}

bool MainUI::Init() {
  glfwSetErrorCallback(glfw_error_callback);
  if (!glfwInit())
    return false;

    // Decide GL+GLSL versions
#if defined(IMGUI_IMPL_OPENGL_ES2)
  // GL ES 2.0 + GLSL 100 (WebGL 1.0)
  const char *glsl_version = "#version 100";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(IMGUI_IMPL_OPENGL_ES3)
  // GL ES 3.0 + GLSL 300 es (WebGL 2.0)
  const char *glsl_version = "#version 300 es";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
#elif defined(__APPLE__)
  // GL 3.2 + GLSL 150
  const char *glsl_version = "#version 150";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // 3.2+ only
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // Required on Mac
#else
  // GL 3.0 + GLSL 130
  const char *glsl_version = "#version 130";
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
  // glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+
  // only glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // 3.0+ only
#endif

  // Create window with graphics context
  window = glfwCreateWindow(1280, 720, "Dear ImGui GLFW+OpenGL3 example",
                            nullptr, nullptr);
  if (window == nullptr)
    return false;
  glfwMakeContextCurrent(window);
  glfwSwapInterval(1); // Enable vsync

  // Setup Dear ImGui context
  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGuiIO &io = ImGui::GetIO();
  (void)io;
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
  io.ConfigFlags |=
      ImGuiConfigFlags_NavEnableGamepad; // Enable Gamepad Controls

  // Setup Dear ImGui style
  ImGui::StyleColorsDark();
  // ImGui::StyleColorsLight();

  // Setup Platform/Renderer backends
  ImGui_ImplGlfw_InitForOpenGL(window, true);
#ifdef __EMSCRIPTEN__
  ImGui_ImplGlfw_InstallEmscriptenCallbacks(window, "#canvas");
#endif
  ImGui_ImplOpenGL3_Init(glsl_version);

  // Load Fonts
  // - If no fonts are loaded, dear imgui will use the default font. You can
  // also load multiple fonts and use ImGui::PushFont()/PopFont() to select
  // them.
  // - AddFontFromFileTTF() will return the ImFont* so you can store it if you
  // need to select the font among multiple.
  // - If the file cannot be loaded, the function will return a nullptr. Please
  // handle those errors in your application (e.g. use an assertion, or display
  // an error and quit).
  // - The fonts will be rasterized at a given size (w/ oversampling) and stored
  // into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which
  // ImGui_ImplXXXX_NewFrame below will call.
  // - Use '#define IMGUI_ENABLE_FREETYPE' in your imconfig file to use Freetype
  // for higher quality font rendering.
  // - Read 'docs/FONTS.md' for more instructions and details.
  // - Remember that in C/C++ if you want to include a backslash \ in a string
  // literal you need to write a double backslash \\ !
  // - Our Emscripten build process allows embedding fonts to be accessible at
  // runtime from the "fonts/" folder. See Makefile.emscripten for details.
  // io.Fonts->AddFontDefault();
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\segoeui.ttf", 18.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
  // io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
  // ImFont* font =
  // io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f,
  // nullptr, io.Fonts->GetGlyphRangesJapanese()); IM_ASSERT(font != nullptr);

  return true;
}

void MainUI::Deinit() {
  // Cleanup
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwDestroyWindow(window);
  glfwTerminate();
}

void MainUI::Render() {
  // Main loop
#ifdef __EMSCRIPTEN__
  // For an Emscripten build we are disabling file-system access, so let's not
  // attempt to do a fopen() of the imgui.ini file. You may manually call
  // LoadIniSettingsFromMemory() to load settings from your own storage.
  io.IniFilename = nullptr;
  EMSCRIPTEN_MAINLOOP_BEGIN
#else
  while (!glfwWindowShouldClose(window))
#endif
  {
    // Poll and handle events (inputs, window resize, etc.)
    // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to
    // tell if dear imgui wants to use your inputs.
    // - When io.WantCaptureMouse is true, do not dispatch mouse input data to
    // your main application, or clear/overwrite your copy of the mouse data.
    // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input
    // data to your main application, or clear/overwrite your copy of the
    // keyboard data. Generally you may always pass all inputs to dear imgui,
    // and hide them from your application based on those two flags.
    glfwPollEvents();
    if (glfwGetWindowAttrib(window, GLFW_ICONIFIED) != 0) {
      ImGui_ImplGlfw_Sleep(10);
      continue;
    }

    // Start the Dear ImGui frame
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // IMGUI Rendering Commands
    for (const auto &func : imguiRenderingFuncs) {
      func();
    }

    // Rendering
    ImGui::Render();
    int display_w, display_h;
    glfwGetFramebufferSize(window, &display_w, &display_h);
    glViewport(0, 0, display_w, display_h);
    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);
  }
#ifdef __EMSCRIPTEN__
  EMSCRIPTEN_MAINLOOP_END;
#endif
}

void MainUI::AppendRenderFunction(RenderFuncType &&func) {
  imguiRenderingFuncs.emplace_back(func);
}

XMLViewer::XMLViewer() {
  filename.reserve(MAX_PATH_SIZE);
  filename = "../gen4.xml";
  OnFileLoading();
}

static int PathInputChangeCallback(ImGuiInputTextCallbackData *data) {
  *(bool *)(data->UserData) = false;
  return 1;
}

static void XMLValueDataDrawWidget(const XMLValueData &valueData) {
  ImGui::Text("%llu\t%s %s", valueData.value, valueData.name.c_str(),
              (valueData.info ? valueData.info->c_str() : ""));
}

static void XMLVecValueDataTable(const std::vector<XMLValueData> &vecValueData,
                                 const char *strTableName) {
  if (ImGui::BeginTable(strTableName, 3,
                        ImGuiTableFlags_Resizable |
                            ImGuiTableFlags_SizingFixedFit)) {
    ImGui::TableSetupColumn("Name", 0);
    ImGui::TableSetupColumn("Value", 0);
    ImGui::TableSetupColumn("Info", 0);
    ImGui::TableHeadersRow();
    for (const auto &valueData : vecValueData) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Text("%s", valueData.name.c_str());
      ImGui::TableNextColumn();
      ImGui::Text("%llu", valueData.value);
      ImGui::TableNextColumn();
      ImGui::Text("%s", valueData.info ? valueData.info->c_str() : "NA");
    }
    ImGui::EndTable();
  }
}

void XMLViewer::Render() {
  if (!isFileOpened) {
    ImGui::Text("No file opened.");
    if (ImGui::Button("Open")) {
      isShowFileDialog = true;
      ImGui::OpenPopup("File Selector");
    }

    if (isShowFileDialog) {
      ImGui::BeginPopupModal("File Selector", NULL,
                             ImGuiWindowFlags_AlwaysAutoResize);
      if (isFileLoading) {
        if (filename.empty()) {
          ImGui::Text("Unknow error");
        }
        ImGui::Text("Loading file of %s ...", filename.c_str());
      } else {
        ImGui::InputText(
            "Please input the absolute path of the target xml file", &filename,
            ImGuiInputTextFlags_CallbackEdit, &PathInputChangeCallback,
            &isShowFileLoadError);
        if (ImGui::Button("Load")) {
          OnFileLoading();
        }
        if (isShowFileLoadError) {
          ImGui::Text("Load '%s' failed.", filename.c_str());
        }
      }
      ImGui::EndPopup();
    }
  } else {
    ImGui::Text("Opened file %s", filename.c_str());
    const XMLDocData &docData = xmlParserContext->parsedDoc;
    if (ImGui::TreeNode("enum(s)")) {
      for (const auto &enumData : docData.enumerates) {
        if (ImGui::TreeNode(enumData.name.c_str())) {
          XMLVecValueDataTable(enumData.values, "Values");
          ImGui::TreePop();
        }
      }
      ImGui::TreePop();
    }
    if (ImGui::TreeNode("struct(s)")) {
      for (const auto &structData : docData.structures) {
        if (ImGui::TreeNode(structData.name.c_str())) {
          for (const auto &fields : structData.fields) {
            if (!fields.choices) {
              ImGui::BulletText("%s", fields.name.c_str());
            } else {
              if (ImGui::TreeNode(fields.name.c_str())) {
                XMLVecValueDataTable(fields.choices.value(), "Choices");
                ImGui::TreePop();
              }
            }
          }
          ImGui::TreePop();
        }
      }
      ImGui::TreePop();
    }

    if (ImGui::Button("Close")) {
      OnFileClose();
    }

    if (ImGui::Button("Save")) {
      if (toSaveFilename.empty() && !filename.empty()) {
        toSaveFilename = filename;
      }
      toSaveFilename.reserve(FILENAME_MAX);
      savingMsg.clear();
      ImGui::OpenPopup("Save");
    }
    if (ImGui::BeginPopupModal("Save", NULL,
                               ImGuiWindowFlags_AlwaysAutoResize)) {

      if (!savingResult) {
        ImGui::InputText("File", toSaveFilename.data(),
                         toSaveFilename.capacity());
        if (ImGui::Button("Save")) {
          OnFileSave();
        }
      } else {
        if (savingResult->valid()) {
          savingMsg += (savingResult.get() ? " Sucess" : " Fail");
          savingResult.reset();
        }
      }
      if (!savingMsg.empty()) {
        ImGui::Text("%s.", savingMsg.c_str());
      }
      // no begin save or finished save
      if (!savingResult || savingResult->valid()) {
        if (ImGui::Button("Cancel")) {
          savingResult.reset();
          ImGui::CloseCurrentPopup();
        }
      }
      ImGui::EndPopup();
    }
    if (ImGui::Button("Add enum")) {
      if (!xmlEditEnumUI) {
        xmlEditEnumUI = std::make_unique<XMLEditEnumUI>();
      }
      ImGui::OpenPopup("Edit enum" );
    }
    if (ImGui::BeginPopupModal("Edit enum")) {

      xmlEditEnumUI->Render();
      ImGui::NewLine();
      if (ModalOKButton()) {
        xmlParserContext->parsedDoc.enumerates.emplace_back(
          xmlEditEnumUI->currentEditing
        );
        xmlEditEnumUI.reset();
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();

      if (ModalCancelButton()) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    if (ImGui::Button("Add Struct")) {
      if (!xmlEditStructUI) xmlEditStructUI = std::make_unique<XMLEditStructUI>();
      ImGui::OpenPopup("Edit Struct");
    }
    if (ImGui::BeginPopupModal("Edit Struct")) {
      xmlEditStructUI->Render();
      if (ModalOKButton()) {
        xmlParserContext->parsedDoc.structures.emplace_back(xmlEditStructUI->currentEditing);
        xmlEditStructUI.reset();
        ImGui::CloseCurrentPopup();
      }
      ImGui::SameLine();
      if (ModalCancelButton()) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

  }
}

void XMLViewer::OnFileLoading() {
  isFileLoading = true;
  isShowFileLoadError = false;
  if (loadingResult)
    loadingResult->wait();
  auto newLoadingResult = std::make_unique<std::future<bool>>(
      std::async(std::launch::async, [this]() {
        auto parserContextPtr =
            std::make_unique<XMLParserContext>(this->filename);
        bool parseResult = parserContextPtr->init();
        if (parseResult) {
          this->xmlParserContext.swap(parserContextPtr);
          isFileOpened = true;
          isShowFileDialog = false;
        } else {
          isShowFileLoadError = true;
        }

        isFileLoading = false;
        return parseResult;
      }));
  loadingResult.swap(newLoadingResult);
}

void XMLViewer::OnFileClose() {
  if (isFileLoading) {
    loadingResult->wait();
  }
  xmlParserContext.reset();
  isFileLoading = false;
  isFileOpened = false;
}

void XMLViewer::OnFileSave() {
  savingResult = std::make_unique<std::future<bool>>(std::async([this]() {
    savingMsg = "Saving ...";
    bool saveResult = SaveToFile(xmlParserContext->parsedDoc, toSaveFilename.c_str());
    return saveResult;
  }));
}

// Main code
int main(int, char **) {
  MainUI mainUI{};
  mainUI.Init();

  XMLViewer viewer;

  mainUI.AppendRenderFunction([&viewer]() { viewer.Render(); });

  mainUI.Render();

  return 0;
}
