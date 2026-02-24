#include "Conini.h"


namespace fs = std::filesystem;
CSimpleIniA ini;

string GetHackDataDir()
{
	/* 1. 取公共文档路径（UTF-8） */
	PWSTR path = nullptr;
	if (FAILED(SHGetKnownFolderPath(FOLDERID_Documents, 0, nullptr, &path)))
		return {};
	char buf[MAX_PATH];
	WideCharToMultiByte(CP_UTF8, 0, path, -1, buf, sizeof(buf), nullptr, nullptr);
	CoTaskMemFree(path);
	fs::path dir = fs::path(buf) / "UnityTools";

	/* 3. 不存在就创建（多级） */
	error_code ec;
	fs::create_directories(dir, ec);
	if (ec) return {};
	return dir.string() + '\\';
}

namespace Config {
	/*---------- 真正定义 + 初始值 ----------*/
	/*配置*/
	ImGuiKey key_see_menu = ImGuiKey_Insert;
	bool off_imguiAnti_screenshot = false;
	int language_index = 0;
	

	// -------------- 变量元信息表 --------------
	enum class VarType { Bool, Float3, Float, Int, String };
	struct Entry {
		void* addr;
		VarType      type;
		const char* section;
		const char* key;
	};


	static const Entry g_tbl[] = {
		// 配置
		{&off_imguiAnti_screenshot, VarType::Bool,   "Imgui", "AntiScreenshot"},
		{&key_see_menu,            VarType::Int,    "Imgui", "KeySeeMenu"},
		{&language_index,          VarType::Int,    "General", "Language"},
	};


	// 配置函数区
	bool SaveConfig(const char* fileName)
	{
		string name(fileName);
		if (name.size() < 4 || name.compare(name.size() - 4, 4, ".ini") != 0)
			name += ".ini";

		string fullPath = GetHackDataDir() + name;
		for (auto& e : g_tbl)
		{
			switch (e.type)
			{
			case VarType::Bool:
				ini.SetBoolValue(e.section, e.key, *static_cast<bool*>(e.addr));
				break;

			case VarType::Float:
				ini.SetDoubleValue(e.section, e.key, *static_cast<float*>(e.addr));
				break;

			case VarType::Float3: {
				float* v = static_cast<float*>(e.addr);
				ini.SetDoubleValue(e.section, e.key, v[0], "#R");
				ini.SetDoubleValue(e.section, (string(e.key) + "G").c_str(), v[1], "#G");
				ini.SetDoubleValue(e.section, (string(e.key) + "B").c_str(), v[2], "#B");
				break;
			}
			case VarType::Int:
				ini.SetLongValue(e.section, e.key, *static_cast<int*>(e.addr));
				break;

			case VarType::String:
				ini.SetValue(e.section, e.key, static_cast<std::string*>(e.addr)->c_str());
				break;
			}
		}
		SI_Error rc = ini.SaveFile(fullPath.c_str());
		printf("[SaveConfig] %s  %s\n", fullPath.c_str(), rc == SI_OK ? "成功" : "失败");
		return rc == SI_OK;
	}
	bool LoadConfig(const char* fileName)
	{
		string name(fileName);
		if (name.size() < 4 || name.compare(name.size() - 4, 4, ".ini") != 0)
			name += ".ini";

		string fullPath = GetHackDataDir() + name;
		if (!fs::exists(fullPath) || !fs::is_regular_file(fullPath))
		{
			printf("[LoadConfig] 文件不存在:%s\n", fullPath.c_str());
			return false;
		}

		SI_Error rc = ini.LoadFile(fullPath.c_str());
		if (rc < 0)
		{
			printf("[LoadConfig] 加载失败:%s\n", fullPath.c_str());
			return false;
		}
		for (auto& e : g_tbl)
		{
			switch (e.type)
			{
			case VarType::Bool:
				*static_cast<bool*>(e.addr) = ini.GetBoolValue(e.section, e.key, *static_cast<bool*>(e.addr));
				break;

			case VarType::Float:
				*static_cast<float*>(e.addr) = static_cast<float>(ini.GetDoubleValue(e.section, e.key, *static_cast<float*>(e.addr)));
				break;

			case VarType::Float3: {
				float* v = static_cast<float*>(e.addr);
				v[0] = static_cast<float>(ini.GetDoubleValue(e.section, e.key, v[0]));
				v[1] = static_cast<float>(ini.GetDoubleValue(e.section, (string(e.key) + "G").c_str(), v[1]));
				v[2] = static_cast<float>(ini.GetDoubleValue(e.section, (string(e.key) + "B").c_str(), v[2]));
				break;
			}

			case VarType::Int:
				*static_cast<int*>(e.addr) = static_cast<int>(ini.GetLongValue(e.section, e.key, *static_cast<int*>(e.addr)));
				break;

			case VarType::String:
				*static_cast<std::string*>(e.addr) = ini.GetValue(e.section, e.key, static_cast<std::string*>(e.addr)->c_str());
				break;
			}
		}
		printf("[LoadConfig] 已加载:%s\n", fullPath.c_str());
		return true;
	}
	bool DleConfig(const char* fileName) {
		string name(fileName);
		if (name.size() < 4 ||
			name.compare(name.size() - 4, 4, ".ini") != 0)
			name += ".ini";
		string fullPath = GetHackDataDir() + name;  // 修正：使用name而不是fileName
		if (!fs::exists(fullPath)) return false;  // 修正：如果文件不存在，返回false
		if (fs::remove(fullPath)) {
			return true;
		}
		else {
			return false;
		}
	}
	bool CreateConfig(const char* fileName)
	{
		std::string name(fileName);
		if (name.empty()) return false;  // 检查空文件名
		if (name.size() < 4 || name.compare(name.size() - 4, 4, ".ini") != 0)
			name += ".ini";
		string fullPath = GetHackDataDir() + name;
		if (fs::exists(fullPath)) return false;
		for (auto& e : g_tbl) {
			switch (e.type) {
			case VarType::Bool:
				ini.SetBoolValue(e.section, e.key, *static_cast<bool*>(e.addr));
				break;
			case VarType::Float:
				ini.SetDoubleValue(e.section, e.key, *static_cast<float*>(e.addr));
				break;
			case VarType::Float3: {
				float* v = static_cast<float*>(e.addr);
				ini.SetDoubleValue(e.section, e.key, v[0], "#R");
				ini.SetDoubleValue(e.section, (std::string(e.key) + "G").c_str(), v[1], "#G");
				ini.SetDoubleValue(e.section, (std::string(e.key) + "B").c_str(), v[2], "#B");
				break;
			}
			case VarType::Int:
				ini.SetLongValue(e.section, e.key, *static_cast<int*>(e.addr));
				break;

			case VarType::String:
				ini.SetValue(e.section, e.key, static_cast<std::string*>(e.addr)->c_str());
				break;
			}
		}
		SI_Error rc = ini.SaveFile(fullPath.c_str());
		printf("[CreateConfig] %s  %s\n", fullPath.c_str(), rc == SI_OK ? "成功" : "失败");
		return rc == SI_OK;
	}
	bool RefreshConfig(std::vector<std::string>& config_list)
	{
		config_list.clear();
		std::string dir = GetHackDataDir();
		if (dir.empty()) return false;
		try {
			for (const auto& entry : fs::directory_iterator(dir))
			{
				if (!entry.is_regular_file()) continue;

				std::string fname = entry.path().filename().string();
				if (fname.size() >= 4 &&
					fname.compare(fname.size() - 4, 4, ".ini") == 0)
				{
					fname.resize(fname.size() - 4);
					config_list.emplace_back(fname);
				}
			}
			return !config_list.empty();
		}
		catch (const fs::filesystem_error&) {
			return false; 
		}
	}
}