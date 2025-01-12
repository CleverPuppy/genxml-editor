#ifndef __MAIN_UI_H__
#define __MAIN_UI_H__
#include <cstdio>
#include <functional>
#include <vector>
#include <string>
#include <future>
#include "xml_parser.h"
#include "xml_types.h"
typedef struct GLFWwindow GLFWwindow;

class MainUI
{
using RenderFuncType = std::function<void()>;
public:
	MainUI() : window(nullptr) {}
	~MainUI() {
		Deinit();
	}
	MainUI(const MainUI&) = delete;
	MainUI& operator=(const MainUI&) = delete;
	bool Init();
	void Render();

	void AppendRenderFunction(RenderFuncType&& func);
private:
	GLFWwindow *window;
	std::vector<RenderFuncType> imguiRenderingFuncs;
	void Deinit();
};

class XMLViewer
{
public:
	XMLViewer();
	~XMLViewer() = default;
	void Render();
private:
	std::string filename;
	std::string loadingErrorMsg;
	bool isFileOpened = false;
	bool isShowFileDialog = false;
	bool isShowFileLoadError = false;
	bool isFileLoading = false;

	void OnFileLoading();
	XMLDocData xmlDoc;
	std::future<bool> loadingResult;
	std::unique_ptr<XMLParserContext> xmlParserContext;
};

#endif