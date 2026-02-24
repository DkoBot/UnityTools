#pragma once
#include "imgui.h"
#include "../Misc/simpleini/SimpleIni.h"
#include <windows.h>
#include <shlobj.h>
#include <string>
#include <filesystem> 
using namespace std;

namespace Config {
	/*---------- 只声明，不定义 ----------*/

	/*配置*/
	extern bool off_imguiAnti_screenshot;
	extern ImGuiKey key_see_menu;
	extern int language_index;

	// Config操作函数
	bool SaveConfig(const char* name);
	bool LoadConfig(const char* name);
	bool DleConfig(const char* name);
	bool CreateConfig(const char* name);
	bool RefreshConfig(vector<string> &config_list);
}