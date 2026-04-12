#include "hooks.h"
#include "../Menu/MemoryOperation.h"
#include "../Menu/DumpSdk.h"
#include <algorithm>
#define byte unsigned char
#pragma comment(lib, "shell32.lib")
#pragma execution_character_set("utf-8")



// DirectX设备全局变量
ID3D11Device* pDevice = nullptr;

// 只在内层循环用，性能足够
template<typename T>
inline T SAFE_READ(T* ptr, T def = T()) {
	if (!ptr) return def;
	__try {
		return *ptr;
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
		return def;
	}
}
namespace Hooks
{
	// 着色器属性结构体定义
	struct ShaderProperty {
		string name;
		string type;
		string value;
		ImVec4 colorValue = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		float floatValue = 1.0f;
		int intValue = 1;
		bool boolValue = true;
	};

	bool bDx11Init = false;
	bool bDx11ReInit = false;
	static bool menu_visible = true;

	// 窗口显示状态
	static bool show_global_manager = false;
	static bool show_memory_scanner = false;
	static bool show_memory_drawer = false;
	static bool show_shader_drawer = false;
	static bool show_material_drawer = false;
	static bool show_theme_settings = false;
	static bool show_config_window = false;

	// 窗口原始状态（用于菜单隐藏时保存状态）
	static bool orig_show_global_manager = false;
	static bool orig_show_memory_scanner = false;
	static bool orig_show_memory_drawer = false;
	static bool orig_show_shader_drawer = false;
	static bool orig_show_material_drawer = false;
	static bool orig_show_theme_settings = false;
	static bool orig_show_config_window = false;

	// 窗口前置标志
	static bool focus_global_manager = false;
	static bool focus_memory_scanner = false;
	static bool focus_memory_drawer = false;
	static bool focus_shader_drawer = false;
	static bool focus_material_drawer = false;
	static bool focus_theme_settings = false;
	static bool focus_config_window = false;

	// Dump SDK 进度相关
	static bool dump_sdk_in_progress = false;
	static float dump_sdk_progress = 0.0f;
	static float dump_sdk_target_progress = 0.0f;
	static float dump_sdk_display_progress = 0.0f;
	static bool dump_sdk_complete = false;
	static bool dump_sdk_success = false;
	static std::string dump_sdk_result_path = "";

	Utils::VTableHook Dx11Hook;
	pWndProc oWndProc = nullptr;
	pPresent oPresent = nullptr;

	inline string il2CppStringToGbk(Il2CppString* src)
	{
		if (!src || !src->length) return {};

		// 1. 拿到 UTF-16 缓冲区
		const wchar_t* utf16 = reinterpret_cast<const wchar_t*>(src->chars);

		// 2. UTF-16 → GBK
		int gbLen = WideCharToMultiByte(936, 0, utf16, src->length,
			nullptr, 0, nullptr, nullptr);
		string gbk(gbLen, 0);
		WideCharToMultiByte(936, 0, utf16, src->length,
			&gbk[0], gbLen, nullptr, nullptr);
		return gbk;
	}
	// UTF-8 转 Wide String 辅助函数
	static wstring Utf8ToWide(const string& utf8Str) {
		if (utf8Str.empty()) {
			return wstring();
		}
		int len = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), static_cast<int>(utf8Str.length()), NULL, 0);
		if (len == 0) {
			return wstring();
		}
		wstring wideStr(len, L'\0');
		int result = MultiByteToWideChar(CP_UTF8, 0, utf8Str.c_str(), static_cast<int>(utf8Str.length()), &wideStr[0], len);
		if (result == 0) {
			return wstring();
		}
		return wideStr;
	}
	pResize oResize = nullptr;
	static ID3D11DeviceContext* pDeviceContext = nullptr;
	static RECT lockRect{};
	static ID3D11RenderTargetView* pRenderTargetView = nullptr;
	static bool inSizeMove = false;
	static bool isDraggingWindow = false;
	static HWND hwnd = nullptr; // 窗口句柄，用于反截图功能

	void initialize(kiero::RenderType::Enum renderType) {
		kiero::init(renderType);
		kiero::bind(8, (void**)&oPresent, hkPresent);
		kiero::bind(13, (void**)&oResize, hkResize);
	}
	Il2CppString* create_il2cpp_string(const wchar_t* src) {
		return il2cpp_string_new_utf16(src, static_cast<int32_t>(wcslen(src)));
	}
	// 优化的搜索函数，使用预计算的小写版本
	inline bool contains_optimized(size_t index, const string& search_term) {
		if (index >= globalmanagement::classes_lower.size()) return false;
		if (search_term.empty()) return true;

		// 将搜索词转换为小写（只需要转换一次）
		static string last_search_term;
		static string search_lower;
		if (last_search_term != search_term) {
			last_search_term = search_term;
			search_lower = search_term;
			// 使用循环代替transform，避免编译器兼容性问题
			for (size_t i = 0; i < search_lower.size(); ++i) {
				search_lower[i] = ::tolower(search_lower[i]);
			}
		}

		// 在预计算的小写类名中搜索
		return globalmanagement::classes_lower[index].find(search_lower) != string::npos;
	}

	inline bool contains(const string& a, const string& b) {
		// 保留原函数用于其他用途（如Assembly搜索）
		string a_lower = a;
		string b_lower = b;
		// 使用循环代替transform，避免编译器兼容性问题
		for (size_t i = 0; i < a_lower.size(); ++i) {
			a_lower[i] = ::tolower(a_lower[i]);
		}
		for (size_t i = 0; i < b_lower.size(); ++i) {
			b_lower[i] = ::tolower(b_lower[i]);
		}
		return a_lower.find(b_lower) != string::npos;
	}

	HRESULT WINAPI hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags) {
		// 1. 初始化 DirectX11 和 ImGui
		{
			if (!bDx11Init) {
				pSwapChain->GetDevice(__uuidof(ID3D11Device), (void**)&pDevice);
				pDevice->GetImmediateContext(&pDeviceContext);
				ID3D11Texture2D* pBackBuffer;
				pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
				pDevice->CreateRenderTargetView(pBackBuffer, nullptr, &pRenderTargetView);
				pBackBuffer->Release();
				oWndProc = (WNDPROC)SetWindowLongPtr(Engine::hWnd, GWLP_WNDPROC, (LONG_PTR)hkWndProc);
				hwnd = Engine::hWnd; // 初始化窗口句柄
				if (!bDx11ReInit) {
					ImGui::CreateContext();
					ImGui::StyleColorsDark();
					ImGui_ImplWin32_Init(Engine::hWnd);
					ImGuiIO& Io = ImGui::GetIO();
					Io.IniFilename = nullptr; // 保存配置文件
					Io.LogFilename = nullptr;
					//Io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\msyh.ttc", 20.0f, NULL, Io.Fonts->GetGlyphRangesChineseFull());
					float baseFontSize = 22.0f;
					ImFont* font = Io.Fonts->AddFontFromFileTTF
					(
						"c:\\Windows\\Fonts\\msyh.ttc",
						baseFontSize,
						nullptr,
						Io.Fonts->GetGlyphRangesChineseFull()
					);
					IM_ASSERT(font != nullptr);
					
					// 设置默认字体为中文字体
					Io.FontDefault = font;
					//SetWindowDisplayAffinity(hwnd, WDA_EXCLUDEFROMCAPTURE);
					float iconFontSize = baseFontSize * 2.0f / 3.0f;
					static const ImWchar icons_ranges[] = { ICON_MIN_FA, ICON_MAX_16_FA,0 };
					ImFontConfig icons_config;
					icons_config.MergeMode = true;
					icons_config.PixelSnapH = true;
					icons_config.GlyphMinAdvanceX = iconFontSize;
					Io.Fonts->AddFontFromMemoryCompressedBase85TTF(fa_solid_900_compressed_data_base85, iconFontSize, &icons_config, icons_ranges);
					il2cpp_thread_attach(il2cpp_domain_get());
				}
				ImGui_ImplDX11_Init(pDevice, pDeviceContext);
				bDx11Init = true;
				bDx11ReInit = true;
			}
			// 2. 控制鼠标光标显示
			static int originalCursorCount = 0;
			static bool initialized = false;

			// 初始化时保存原始光标计数
			if (!initialized) {
				// 通过临时增加计数来获取当前计数，然后恢复
				int temp = ShowCursor(TRUE);
				originalCursorCount = temp - 1;
				ShowCursor(FALSE);
				initialized = true;
			}

			if (menu_visible) {
				// 菜单显示时强制显示鼠标光标
				// 使用激进的方法：多次调用确保光标可见
				for (int i = 0; i < 10; i++) {
					ShowCursor(TRUE);
				}
			}
			else {
				// 菜单隐藏时恢复原始状态
				// 确保计数回到原始值
				int current = ShowCursor(TRUE);
				while (current > originalCursorCount) {
					current = ShowCursor(FALSE);
				}
				while (current < originalCursorCount) {
					current = ShowCursor(TRUE);
				}
			}

			// 获取当前光标可见状态
			int currentCount = ShowCursor(TRUE);
			ShowCursor(FALSE); // 恢复计数
			bool cursorVisible = currentCount >= 0;
			// 开始绘制
			ImGui_ImplDX11_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		}

		// 2. 绘制菜单和窗口
		{
			// 检查显示隐藏快捷键
			if (ImGui::IsKeyPressed(Config::key_see_menu, false)) {
				bool new_menu_visible = !menu_visible;
				if (new_menu_visible != menu_visible) {
					if (!new_menu_visible) {
						// 隐藏菜单时保存当前窗口状态
						orig_show_global_manager = show_global_manager;
						orig_show_memory_scanner = show_memory_scanner;
						orig_show_memory_drawer = show_memory_drawer;
						orig_show_shader_drawer = show_shader_drawer;
						orig_show_material_drawer = show_material_drawer;
						orig_show_theme_settings = show_theme_settings;
						orig_show_config_window = show_config_window;
						// 隐藏所有窗口
						show_global_manager = false;
						show_memory_scanner = false;
						show_memory_drawer = false;
						show_shader_drawer = false;
						show_material_drawer = false;
						show_theme_settings = false;
						show_config_window = false;
					}
					else {
						// 显示菜单时恢复窗口状态
						show_global_manager = orig_show_global_manager;
						show_memory_scanner = orig_show_memory_scanner;
						show_memory_drawer = orig_show_memory_drawer;
						show_shader_drawer = orig_show_shader_drawer;
						show_material_drawer = orig_show_material_drawer;
						show_theme_settings = orig_show_theme_settings;
						show_config_window = orig_show_config_window;
					}
					menu_visible = new_menu_visible;
				}
			}

			// 1. 全局主菜单条
			if (menu_visible && ImGui::BeginMainMenuBar()) {
				/* ---------- 彩虹渐变标题 ---------- */
				{
					const char* txt = "Unity Tools 2026.04.12";
					float speed = 0.3f;                 // 色相滚动速度（弧度/秒）
					float step = 1.0f / strlen(txt);   // 每字色相步进

					ImDrawList* draw = ImGui::GetWindowDrawList();
					ImVec2 pos = ImGui::GetCursorScreenPos();

					/* 时间驱动的色相基线 */
					float baseHue = fmodf((float)ImGui::GetTime() * speed, 1.0f);

					/* 逐字绘制 */
					float x = pos.x;
					for (int i = 0; txt[i]; ++i) {
						/* 当前字色相 */
						float hue = fmodf(baseHue + i * step, 1.0f);
						ImVec4 rgb;
						ImGui::ColorConvertHSVtoRGB(hue, 0.85f, 1.0f, rgb.x, rgb.y, rgb.z);
						rgb.w = 1.0f;

						/* 字宽 */
						char buf[2] = { txt[i], 0 };
						float w = ImGui::CalcTextSize(buf).x;

						/* 绘制 */
						draw->AddText(ImVec2(x, pos.y + 4), ImColor(rgb), buf);
						x += w;
					}

					/* 占住 ImGui 光标，后续控件不会重叠 */
					ImGui::Dummy(ImVec2(x - pos.x, ImGui::GetTextLineHeight()));
				}
				if (ImGui::MenuItem("全局管理")) {
					show_global_manager = true;
					focus_global_manager = true;
				}
				if (ImGui::MenuItem("内存扫描")) {
					show_memory_scanner = true;
					focus_memory_scanner = true;
				}
				if (ImGui::MenuItem("内存绘制")) {
					show_memory_drawer = true;
					focus_memory_drawer = true;
				}
				if (ImGui::MenuItem("着色器管理器")) {
					show_shader_drawer = true;
					focus_shader_drawer = true;
				}
				if (ImGui::MenuItem("材质管理器")) {
					show_material_drawer = true;
					focus_material_drawer = true;
				}
				if (ImGui::MenuItem("主题")) {
					show_theme_settings = true;
					focus_theme_settings = true;
				}
				if (ImGui::MenuItem("配置")) {
					show_config_window = true;
					focus_config_window = true;
				}
				// 右侧
				{
					// 在右侧显示FPS、时间和Unity版本
					static float displayFps = 0.0f;
					static chrono::steady_clock::time_point lastFpsUpdate = chrono::steady_clock::now();
					const float fpsUpdateInterval = 0.5f; // 0.5秒更新一次

					auto fpsNow = chrono::steady_clock::now();
					float elapsed = chrono::duration<float>(fpsNow - lastFpsUpdate).count();

					if (elapsed >= fpsUpdateInterval) {
						displayFps = ImGui::GetIO().Framerate;
						lastFpsUpdate = fpsNow;
					}

					float fps = displayFps;

					// 获取当前时间
					auto timeNow = chrono::system_clock::now();
					auto time_t = chrono::system_clock::to_time_t(timeNow);
					tm tm;
					localtime_s(&tm, &time_t);

					char timeStr[20];
					strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", &tm);

					string unityVersion = "Not Find Version";
					if ((void*)GetUnityVersion()) {
						unityVersion = il2CppStringToGbk(GetUnityVersion());
					}
					// 构建完整文本并计算宽度
					char fpsText[32];
					char dateTimeText[64];
					char unityVersionText[64];
					sprintf_s(fpsText, "FPS: %.0f", fps);  // 显示为整数
					sprintf_s(dateTimeText, "DateTime: %s", timeStr);
					sprintf_s(unityVersionText, "Unity: %s", unityVersion.c_str());

					float fpsWidth = ImGui::CalcTextSize(fpsText).x;
					float dateTimeWidth = ImGui::CalcTextSize(dateTimeText).x;
					float unityVersionWidth = ImGui::CalcTextSize(unityVersionText).x;
					float totalTextWidth = fpsWidth + dateTimeWidth + unityVersionWidth + 2 * ImGui::GetStyle().ItemSpacing.x; // 加上间距

					float windowWidth = ImGui::GetWindowWidth();
					float leftOffset = 10.0f; // 向左侧移动10像素
					ImGui::SameLine(windowWidth - totalTextWidth - leftOffset);
					if (fps >= 100) {
						ImGui::TextColored(ImVec4(0, 255, 98, 255), "%s", fpsText);
					}
					else if (fps >= 60 && fps <= 100) {
						ImGui::TextColored(ImVec4(255, 251, 0, 255), "%s", fpsText);
					}
					else {
						ImGui::TextColored(ImVec4(255, 51, 0, 255), "%s", fpsText);
					}
					ImGui::SameLine();
					ImGui::Text("%s", dateTimeText);
					ImGui::SameLine();
					ImGui::TextColored(ImVec4(0, 191, 255, 255), "%s", unityVersionText);
				}

				ImGui::EndMainMenuBar();
			}
		}
		// 绘制自定义鼠标指针
		if (menu_visible) {
			ImDrawList* drawList = ImGui::GetForegroundDrawList();
			ImVec2 mousePos = ImGui::GetMousePos();

			// 检测鼠标按键状态
			bool leftClick = ImGui::IsMouseDown(ImGuiMouseButton_Left);
			bool rightClick = ImGui::IsMouseDown(ImGuiMouseButton_Right);

			// 根据点击状态调整外观
			float radius = leftClick ? 10.0f : (rightClick ? 12.0f : 8.0f); // 点击时变大
			ImU32 color;

			if (leftClick) {
				color = IM_COL32(255, 100, 100, 255); // 左键：红色
			}
			else if (rightClick) {
				color = IM_COL32(100, 100, 255, 255); // 右键：蓝色
			}
			else {
				color = IM_COL32(0, 242, 255, 255); // 默认：青色
			}

			// 外圆
			drawList->AddCircleFilled(mousePos, radius, color, 16);

			// 内圆（稍微小一点的白色圆）
			float innerRadius = radius * 0.6f;
			ImU32 innerColor = IM_COL32(255, 255, 255, 255);
			drawList->AddCircleFilled(mousePos, innerRadius, innerColor, 16);

			// 添加边框
			ImU32 borderColor = IM_COL32(255, 255, 255, 200);
			drawList->AddCircle(mousePos, radius, borderColor, 16, 2.0f);

			// 点击时的额外效果
			if (leftClick || rightClick) {
				// 添加光晕效果
				float glowRadius = radius * 1.5f;
				ImU32 glowColor = leftClick ?
					IM_COL32(255, 100, 100, 100) : // 左键光晕：红色半透明
					IM_COL32(100, 100, 255, 100);  // 右键光晕：蓝色半透明
				drawList->AddCircleFilled(mousePos, glowRadius, glowColor, 16);

				// 添加点击时的脉冲效果
				static float pulseTime = 0.0f;
				pulseTime += ImGui::GetIO().DeltaTime * 10.0f;
				float pulseRadius = radius + sinf(pulseTime) * 2.0f;
				ImU32 pulseColor = leftClick ?
					IM_COL32(255, 150, 150, 150) :
					IM_COL32(150, 150, 255, 150);
				drawList->AddCircle(mousePos, pulseRadius, pulseColor, 16, 1.0f);
			}
		}

		// 3. 功能实现
		{
			if (menu_visible) {
				static ImFont* smallFont = nullptr;
				/* 2. 把下面整块替换进你现有的“全局管理窗口”即可 */
				if (show_global_manager) {
					if (focus_global_manager) { ImGui::SetNextWindowFocus(); focus_global_manager = false; }

					// 初始化标记
					static bool is_first_open = false;

					/* ---- 紧凑样式 ---- */
					ImGui::PushFont(smallFont);                       // 14 px
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 3));
					ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2, 2)); // 表格再扁
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5);
					ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);

					/* 关键：最小宽度 520（原来是 640）*/
					ImGui::SetNextWindowSizeConstraints(ImVec2(520, 300), ImVec2(FLT_MAX, FLT_MAX));

					ImGui::Begin("全局管理", &show_global_manager);

					/* ---------- 数据缓存（同你旧代码，略） ---------- */
					// Ass
					static vector<string> g_assemblies = { };
					static vector<string> g_namespaces, g_classes;
					static string         g_selAssembly, g_selNamespace, g_selClass;
					static char           asmFilter[128] = "", clsFilter[128] = "", varFilter[128] = "";
					static vector<string> g_searchResults;
					static vector<void*>  g_objectPointers; // 存储转换后的对象指针
					static string         g_currentTag = ""; // 存储当前Tag
					static int            clsFilterType = 0; // 0=All, 1=System, 2=Unity, 3=Custom
					static int            curResultIdx = -1;
					static char           comboPreview[128] = "";
					static char           g_dumpAddr[256] = "";
					static char           g_instanceAddr[256] = ""; // 结构分析器实例地址
					static bool           show_large_file_confirm = false; // 大文件确认弹窗
					static int32_t        pending_dump_size = 0; // 待确认的dump大小
					static uintptr_t      pending_metadata_addr = 0; // 待确认的metadata地址
					static std::string    pending_save_path = ""; // 待确认的保存路径
					static bool           show_structure_analyzer = false; // 结构分析器弹窗标志
					static bool           parsed = false; // 是否已解析
					static std::string    parsedAssembly; // 解析的程序集
					static std::string    parsedNamespace; // 解析的命名空间
					static std::string    parsedClass; // 解析的类名
					static uintptr_t      parsedAddr = 0; // 解析的实例地址
					static bool           show_find_refs = false; // 显示查找引用窗口
					static std::vector<std::string> refClasses; // 引用该类的类列表
					static bool           searching_refs = false; // 是否正在搜索引用
					static bool           refs_searched = false; // 是否已完成搜索
					if (!is_first_open) {
						if (globalmanagement::initialization()) {
							g_assemblies = globalmanagement::assemblies;
							InfoMesseng::ColorPrint("SUCCESS", "globalmanagement init Success!", 0);
						}
						else {
							InfoMesseng::ColorPrint("ERROR", "globalmanagement init error!", 1);
						}
						is_first_open = true;
					}

					// 计算每一列的理想高度
					float availableHeight = ImGui::GetContentRegionAvail().y;
					float col1Height = 0, col2Height = 0, col3Height = 0;

					// 临时计算第一列高度
					{
						float lineH = ImGui::GetTextLineHeightWithSpacing();
						float btnH = lineH + ImGui::GetStyle().FramePadding.y * 2;
						float spacing = ImGui::GetStyle().ItemSpacing.y;
						float searchBoxH = ImGui::GetFrameHeightWithSpacing();
						float titleH = lineH + ImGui::GetStyle().ItemSpacing.y;
						col1Height = titleH + searchBoxH + (availableHeight - titleH - searchBoxH - btnH - spacing * 2);
					}

					// 临时计算第二列高度
					{
						float searchBoxH = ImGui::GetFrameHeightWithSpacing();
						float titleH = ImGui::GetTextLineHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
						col2Height = titleH + searchBoxH + availableHeight - titleH - searchBoxH;
					}

					// 临时计算第三列高度
					{
						float infoHeight = ImGui::GetTextLineHeightWithSpacing() * 4 + ImGui::GetStyle().ItemSpacing.y * 4; // Ns, Asm, Cls, Spacing
						float comboHeight = ImGui::GetFrameHeightWithSpacing();
						float remainingHeight = availableHeight - infoHeight - comboHeight;
						col3Height = infoHeight + comboHeight + max(remainingHeight, 200.0f); // 至少200px用于内容
					}

					// 取最大高度作为统一列高
					float unifiedHeight = max(max(col1Height, col2Height), col3Height);

					// 创建自适应三列布局
					ImGui::Columns(3, "GMColumns", false); // false = 不等宽，让内容决定宽度

					/* -------- 第一列：Assembly -------- */
					{
						ImGui::BeginChild("Col1", ImVec2(0, unifiedHeight), false);
						ImGui::Text("Assembly");
						ImGui::Separator();

						/* 搜索框 */
						ImGui::SetNextItemWidth(-FLT_MIN);
						ImGui::InputTextWithHint("##asmFilter", "搜索...", asmFilter, sizeof(asmFilter));

						/* 计算列表框高度 */
						float lineH = ImGui::GetTextLineHeightWithSpacing();
						float btnH = lineH + ImGui::GetStyle().FramePadding.y * 2;
						float spacing = ImGui::GetStyle().ItemSpacing.y;
						float listH = ImGui::GetContentRegionAvail().y - btnH - spacing;

						ImGui::SetNextItemWidth(-FLT_MIN);
						if (ImGui::BeginListBox("##asmList", ImVec2(-FLT_MIN, listH))) {
							int asmIndex = 0;
							for (auto& n : globalmanagement::sorted_assemblies) {
								if (asmFilter[0] && !contains(n, asmFilter)) {
									asmIndex++;
									continue;
								}
								bool sel = (n == g_selAssembly);
								char label[256];
								sprintf_s(label, "%s##asm_%d", n.c_str(), asmIndex);
								if (ImGui::Selectable(label, sel)) {
									g_selAssembly = n;
									// 获取该程序集的所有类
									if (globalmanagement::get_Assemblies_Class(n)) {
										string msg = "Loaded classes for assembly: " + n;
										InfoMesseng::ColorPrint("INFO", msg.c_str(), 2);
									}
									else {
										string msg = "Loaded classes for assembly: " + n + " Error!";
										InfoMesseng::ColorPrint("ERROR", msg.c_str(), 1);
									}
								}
								if (sel) ImGui::SetItemDefaultFocus();
								asmIndex++;
							}
							ImGui::EndListBox();
						}

						/* 刷新按钮独占一行，宽度 100% */
						ImGui::SetNextItemWidth(-FLT_MIN);
						if (ImGui::Button("刷新##asm", ImVec2(-FLT_MIN, 0))) {
							if (globalmanagement::initialization()) {
								g_assemblies = globalmanagement::assemblies;
								InfoMesseng::ColorPrint("SUCCESS", "globalmanagement init Success!", 0);
							}
							else {
								InfoMesseng::ColorPrint("ERROR", "globalmanagement init error!", 1);
							}
						}
						ImGui::EndChild();
						ImGui::NextColumn();
					}

					/* -------- 第二列 -------- */
					{
						ImGui::BeginChild("Col2", ImVec2(0, unifiedHeight), false);
						ImGui::Text("类名");
						ImGui::Separator();
						ImGui::SetNextItemWidth(-FLT_MIN);
						ImGui::InputTextWithHint("##clsFilter", "搜索...", clsFilter, sizeof(clsFilter));

						// 类名筛选下拉框
						const char* filterTypes[] = { "All", "System", "Unity", "Custom" };
						ImGui::SetNextItemWidth(-FLT_MIN);
						if (ImGui::BeginCombo("##clsFilterType", filterTypes[clsFilterType])) {
							for (int i = 0; i < IM_ARRAYSIZE(filterTypes); i++) {
								bool isSelected = (clsFilterType == i);
								if (ImGui::Selectable(filterTypes[i], isSelected)) {
									clsFilterType = i;
									// 筛选类型改变时重置搜索结果
									globalmanagement::search_results_valid = false;
								}
								if (isSelected) {
									ImGui::SetItemDefaultFocus();
								}
							}
							ImGui::EndCombo();
						}

						// 更新搜索结果（只在搜索词或筛选类型改变时）
						globalmanagement::update_class_search_results(clsFilter, clsFilterType);

						ImGui::SetNextItemWidth(-FLT_MIN);
						if (ImGui::BeginListBox("##clsList", ImVec2(-FLT_MIN, -FLT_MIN))) {
							int visibleIndex = 0;
							for (size_t resultIdx = 0; resultIdx < globalmanagement::class_search_results.size(); ++resultIdx) {
								size_t i = globalmanagement::class_search_results[resultIdx];
								const auto& c = globalmanagement::sorted_classes[i];
								bool sel = (c == g_selClass);
								char label[512];
								sprintf_s(label, "%s##cls_%d", c.c_str(), visibleIndex);
								if (ImGui::Selectable(label, sel)) {
									g_selClass = c;

									// 解析类名和命名空间
									size_t dotCount = 0;
									size_t lastDotPos = string::npos;

									// 统计点号数量并记录最后一个点号位置
									for (size_t i = 0; i < c.length(); i++) {
										if (c[i] == '.') {
											dotCount++;
											lastDotPos = i;
										}
									}

									if (lastDotPos != string::npos) {
										// 有点号，通过最后一个点号分割
										g_selNamespace = c.substr(0, lastDotPos);  // 命名空间部分
										g_selClass = c.substr(lastDotPos + 1);    // 类名部分
									}
									else {
										// 没有点号，说明是全局类（没有命名空间）
										g_selNamespace = "";
										// g_selClass 保持不变（就是完整的类名）
									}

									// 获取选中类的变量和方法信息
									if (globalmanagement::get_Class_Variable_Function(c)) {
										string msg = "Loaded variables and methods for class: " + c;
										InfoMesseng::ColorPrint("INFO", msg.c_str(), 2);
									}
									else {
										string msg = "Failed to load variables and methods for class: " + c;
										InfoMesseng::ColorPrint("WARING", msg.c_str(), 2);
									}
								}
								if (sel) ImGui::SetItemDefaultFocus();
								visibleIndex++;
							}
							ImGui::EndListBox();
						}
						ImGui::EndChild();
						ImGui::NextColumn();
					}

					/* -------- 第三列 -------- */
					{
						ImGui::BeginChild("Col3", ImVec2(0, unifiedHeight), false);
						ImGui::Text("变量 & 方法");
						ImGui::Separator();
						ImGui::Text("Ns: %s", g_selNamespace.empty() ? "—" : g_selNamespace.c_str());
						ImGui::Text("Asm: %s", g_selAssembly.empty() ? "—" : g_selAssembly.c_str());
						ImGui::Text("Cls: %s", g_selClass.empty() ? "—" : g_selClass.c_str());
						ImGui::Spacing();

						/* 搜索行：下拉 + 按钮 整行填满 */
						float avail = ImGui::GetContentRegionAvail().x;
						float btnW = 48; // 按钮再窄 2 px
						ImGui::SetNextItemWidth(avail - btnW - ImGui::GetStyle().ItemSpacing.x);
						// 双击编辑功能
						static bool isEditing = false;
						static char editBuffer[128] = "";

						// 确定combo的预览文本
						const char* previewText = "待寻找实例";
						if (!g_searchResults.empty()) {
							// 如果有搜索结果，显示第一个实例
							previewText = g_searchResults[0].c_str();
						}
						else if (comboPreview[0]) {
							// 否则显示当前选中的预览
							previewText = comboPreview;
						}

						if (isEditing) {
							// 编辑模式：显示输入框
							ImGui::SetNextItemWidth(avail);
							if (ImGui::InputText("##editCombo", editBuffer, sizeof(editBuffer), ImGuiInputTextFlags_EnterReturnsTrue)) {
								// 按Enter保存
								strncpy_s(comboPreview, sizeof(comboPreview), editBuffer, sizeof(comboPreview) - 1);
								isEditing = false;
							}

							// 下面一行：保存、取消、寻找按钮，各占约33%宽度
							float editButtonWidth = (avail - ImGui::GetStyle().ItemSpacing.x * 2) / 3.0f;
							if (ImGui::Button("保存", ImVec2(editButtonWidth, 0))) {
								strncpy_s(comboPreview, sizeof(comboPreview), editBuffer, sizeof(comboPreview) - 1);
								isEditing = false;
							}
							ImGui::SameLine();
							if (ImGui::Button("取消", ImVec2(editButtonWidth, 0))) {
								isEditing = false;
							}
							ImGui::SameLine();
							if (ImGui::Button("寻找", ImVec2(editButtonWidth, 0))) {
								g_searchResults = globalmanagement::get_Class_object(g_selNamespace + "." + g_selClass, g_selAssembly);
								g_objectPointers.clear();
								for (const auto& addrStr : g_searchResults) {
									if (!addrStr.empty()) {
										uintptr_t addr = 0;
										if (sscanf_s(addrStr.c_str(), "%llx", &addr) == 1)
											g_objectPointers.push_back(reinterpret_cast<void*>(addr));
										else
											g_objectPointers.push_back(nullptr);
									}
									else
										g_objectPointers.push_back(nullptr);
								}
								curResultIdx = -1; comboPreview[0] = 0;
								isEditing = false; // 退出编辑模式
							}
						}
						else {
							// 正常模式：显示combo（占满整行宽度）
							ImGui::SetNextItemWidth(avail);
							if (ImGui::BeginCombo("##res", previewText)) {
								if (g_searchResults.empty()) ImGui::TextDisabled("暂无");
								else {
									for (int i = 0; i < g_searchResults.size(); ++i) {
										bool s = (i == curResultIdx);
										if (ImGui::Selectable(g_searchResults[i].c_str(), s)) {
											curResultIdx = i;
											snprintf(comboPreview, sizeof(comboPreview), "%s", g_searchResults[i].c_str());
										}
										if (s) ImGui::SetItemDefaultFocus();
									}
								}
								ImGui::EndCombo();
							}
							// 这里不放编辑按钮
						}

						// 下面一行：编辑和寻找按钮，各占50%宽度（仅在非编辑模式时显示）
						if (!isEditing) {
							float buttonWidth = (avail - ImGui::GetStyle().ItemSpacing.x) * 0.5f;
							if (ImGui::Button("编辑", ImVec2(buttonWidth, 0))) {
								// 进入编辑模式
								isEditing = true;
								strncpy_s(editBuffer, sizeof(editBuffer), previewText, sizeof(editBuffer) - 1);
							}
							ImGui::SameLine();
							if (ImGui::Button("寻找", ImVec2(buttonWidth, 0))) {
								g_searchResults = globalmanagement::get_Class_object(g_selNamespace + "." + g_selClass, g_selAssembly);

								// 将字符串地址转换回void*指针
								g_objectPointers.clear();
								for (const auto& addrStr : g_searchResults) {
									if (!addrStr.empty()) {
										// 从十六进制字符串转换回指针
										uintptr_t addr = 0;
										if (sscanf_s(addrStr.c_str(), "%llx", &addr) == 1) {
											g_objectPointers.push_back(reinterpret_cast<void*>(addr));
										}
										else {
											g_objectPointers.push_back(nullptr); // 转换失败设为null
										}
									}
									else {
										g_objectPointers.push_back(nullptr);
									}
								}
								curResultIdx = -1; comboPreview[0] = 0;
							}
						}

						// 变量折叠栏
						if (ImGui::CollapsingHeader("变量")) {
							// 根据行数计算总高度，超过最大值时限制高度并显示滚动条
							const float varRowHeight = ImGui::GetTextLineHeightWithSpacing();
							const int varRowCount = 1 + (int)globalmanagement::variables.size(); // 表头 + 数据行
							const float varTotalHeight = varRowCount * varRowHeight;
							const float varMaxHeight = 600.0f;
							float varChildHeight = (varTotalHeight > varMaxHeight) ? varMaxHeight : varTotalHeight;
							if (varChildHeight < varRowHeight) varChildHeight = varRowHeight;
							// 最小高度：无数据时也能显示表头且无垂直滚动条
							const float varMinHeight = 60.0f;
							if (varChildHeight < varMinHeight) varChildHeight = varMinHeight;
							float varChildW = ImGui::GetContentRegionAvail().x;
							const float varMinWidth = 400.0f;
							if (varChildW < varMinWidth) varChildW = varMinWidth;
							ImGui::BeginChild("VarScroll", ImVec2(varChildW, varChildHeight), true);
							if (ImGui::BeginTable("varT", 4,
								ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
								ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame)) {
								/* 列可左右拉伸，设置初始比例与最小宽度 */
								ImGui::TableSetupColumn("偏移", ImGuiTableColumnFlags_WidthFixed, 70.0f);
								ImGui::TableSetupColumn("变量名", ImGuiTableColumnFlags_WidthStretch, 0.30f);
								ImGui::TableSetupColumn("类型", ImGuiTableColumnFlags_WidthStretch, 0.35f);
								ImGui::TableSetupColumn("值", ImGuiTableColumnFlags_WidthStretch, 0.10f);
								ImGui::TableHeadersRow();

								// 确定用于读取值的对象基址：选中项或第一个
								void* valueBase = nullptr;
								if (!g_objectPointers.empty()) {
									if (curResultIdx >= 0 && (size_t)curResultIdx < g_objectPointers.size() && g_objectPointers[curResultIdx])
										valueBase = g_objectPointers[curResultIdx];
									else if (g_objectPointers[0])
										valueBase = g_objectPointers[0];
								}
								static MemoryOperation memOp;
								// 先按类型读取各变量值，避免单缓冲导致 tooltip 错乱
								static vector<string> varValueStrings;
								varValueStrings.resize(globalmanagement::variables.size());
								if (valueBase) {
									for (size_t i = 0; i < globalmanagement::variables.size(); ++i) {
										const auto& var = globalmanagement::variables[i];
										void* fieldAddr = (char*)valueBase + var.offset;
										const string& t = var.type;
										if (t == "System.Int32" || t == "System.UInt32" || t == "int" || t == "uint")
											varValueStrings[i] = std::to_string(memOp.Read_int(fieldAddr));
										else if (t == "System.Single" || t == "float") {
											char buf[64];
											sprintf_s(buf, sizeof(buf), "%.4g", memOp.Read_flaot(fieldAddr));
											varValueStrings[i] = buf;
										}
										else if (t == "System.Double" || t == "double") {
											char buf[64];
											sprintf_s(buf, sizeof(buf), "%.4g", memOp.Read_double(fieldAddr));
											varValueStrings[i] = buf;
										}
										else if (t == "System.Boolean" || t == "bool")
											varValueStrings[i] = memOp.Read_bool(fieldAddr) ? "true" : "false";
										else if (t == "System.Byte" || t == "byte")
											varValueStrings[i] = std::to_string((unsigned)memOp.Read_byte(fieldAddr));
										else if (t == "System.String" || t == "string") {
											void* strPtr = *(void**)fieldAddr;
											if (strPtr)
												varValueStrings[i] = memOp.Read_il2cppString(strPtr);
											else
												varValueStrings[i] = "(null)";
										}
										else {
											char buf[32];
											sprintf_s(buf, sizeof(buf), "0x%llX", (unsigned long long)(uintptr_t) * (void**)fieldAddr);
											varValueStrings[i] = buf;
										}
									}
								}
								else {
									for (size_t i = 0; i < globalmanagement::variables.size(); ++i)
										varValueStrings[i] = "—";
								}
								// 双击值列进入编辑
								static int editingVarIndex = -1;
								static char valueEditBuf[512] = "";

								// 显示所有变量数据（使用滚动条浏览）
								for (size_t vi = 0; vi < globalmanagement::variables.size(); ++vi) {
									const auto& var = globalmanagement::variables[vi];
									ImGui::TableNextRow();
									char offsetStr[32];
									sprintf_s(offsetStr, sizeof(offsetStr), "0x%X", var.offset);
									ImGui::TableNextColumn(); ImGui::Text("%s", offsetStr);
									if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", offsetStr);

									ImGui::TableNextColumn(); ImGui::Text("%s", var.name.c_str());
									if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", var.name.c_str());

									ImGui::TableNextColumn(); ImGui::Text("%s", var.type.c_str());
									if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", var.type.c_str());

									// 值列：bool 用下拉框 true/false，其它类型双击进入编辑
									ImGui::TableNextColumn();
									const char* valueDisplay = (vi < varValueStrings.size()) ? varValueStrings[vi].c_str() : "—";
									const string& vtype = var.type;
									bool isBoolType = (vtype == "System.Boolean" || vtype == "bool");
									bool canEdit = (valueBase != nullptr) && (
										vtype == "System.Int32" || vtype == "System.UInt32" || vtype == "int" || vtype == "uint" ||
										vtype == "System.Single" || vtype == "float" || vtype == "System.Double" || vtype == "double" ||
										isBoolType || vtype == "System.Byte" || vtype == "byte" ||
										vtype == "System.String" || vtype == "string");

									if (isBoolType && canEdit) {
										void* fieldAddr = (char*)valueBase + var.offset;
										bool currentBool = (vi < varValueStrings.size() && (varValueStrings[vi] == "true" || varValueStrings[vi] == "1"));
										char idBuf[64];
										sprintf_s(idBuf, sizeof(idBuf), "##boolCombo_%zu", vi);
										ImGui::SetNextItemWidth(-FLT_MIN);
										if (ImGui::BeginCombo(idBuf, currentBool ? "true" : "false")) {
											if (ImGui::Selectable("false", !currentBool)) {
												memOp.write_bool(fieldAddr, false);
												if (vi < varValueStrings.size()) varValueStrings[vi] = "false";
											}
											if (ImGui::Selectable("true", currentBool)) {
												memOp.write_bool(fieldAddr, true);
												if (vi < varValueStrings.size()) varValueStrings[vi] = "true";
											}
											ImGui::EndCombo();
										}
									}
									else if (editingVarIndex == (int)vi) {
										char idBuf[64];
										sprintf_s(idBuf, sizeof(idBuf), "##varValueEdit_%zu", vi);
										ImGui::SetNextItemWidth(-FLT_MIN);
										bool enter = ImGui::InputText(idBuf, valueEditBuf, sizeof(valueEditBuf), ImGuiInputTextFlags_EnterReturnsTrue);
										if (ImGui::IsItemDeactivatedAfterEdit() || enter) {
											void* fieldAddr = (char*)valueBase + var.offset;
											bool ok = false;
											try {
												if (vtype == "System.Int32" || vtype == "System.UInt32" || vtype == "int" || vtype == "uint") {
													int v = std::stoi(valueEditBuf);
													ok = memOp.write_int(fieldAddr, v);
												}
												else if (vtype == "System.Single" || vtype == "float") {
													float v = std::stof(valueEditBuf);
													ok = memOp.write_float(fieldAddr, v);
												}
												else if (vtype == "System.Double" || vtype == "double") {
													double v = std::stod(valueEditBuf);
													ok = memOp.write_double(fieldAddr, v);
												}
												else if (vtype == "System.Byte" || vtype == "byte") {
													int v = std::stoi(valueEditBuf);
													if (v < 0) v = 0; if (v > 255) v = 255;
													ok = memOp.write_byte(fieldAddr, (uint8_t)v);
												}
												else if (vtype == "System.String" || vtype == "string") {
													ok = memOp.write_il2cppString(fieldAddr, string(valueEditBuf));
												}
											}
											catch (...) { ok = false; }
											if (ok && vi < varValueStrings.size()) varValueStrings[vi] = valueEditBuf;
											editingVarIndex = -1;
										}

									}
									else {
										ImGui::Text("%s", valueDisplay);
										if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", valueDisplay);
										if (canEdit && !isBoolType && ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {
											editingVarIndex = (int)vi;
											strncpy_s(valueEditBuf, sizeof(valueEditBuf), valueDisplay, _TRUNCATE);
										}
									}
								}
								ImGui::EndTable();
							}
							ImGui::EndChild();
						}

						// 方法折叠栏
						if (ImGui::CollapsingHeader("方法")) {
							// 根据行数计算总高度，超过最大值时限制高度并显示滚动条
							const float methRowHeight = ImGui::GetTextLineHeightWithSpacing();
							const int methRowCount = 1 + (int)globalmanagement::sorted_methods.size(); // 表头 + 数据行
							const float methTotalHeight = methRowCount * methRowHeight;
							const float methMaxHeight = 600.0f;
							float methChildHeight = (methTotalHeight > methMaxHeight) ? methMaxHeight : methTotalHeight;
							if (methChildHeight < methRowHeight) methChildHeight = methRowHeight;
							const float methMinHeight = 60.0f;
							if (methChildHeight < methMinHeight) methChildHeight = methMinHeight;
							float methChildW = ImGui::GetContentRegionAvail().x;
							const float methMinWidth = 400.0f;
							if (methChildW < methMinWidth) methChildW = methMinWidth;
							ImGui::BeginChild("MethScroll", ImVec2(methChildW, methChildHeight), true);
							if (ImGui::BeginTable("methT", 3,
								ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg |
								ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchSame)) {
								ImGui::TableSetupColumn("方法名", ImGuiTableColumnFlags_WidthStretch, 0.40f);
								ImGui::TableSetupColumn("返回值", ImGuiTableColumnFlags_WidthStretch, 0.25f);
								ImGui::TableSetupColumn("参数", ImGuiTableColumnFlags_WidthStretch, 0.35f);
								ImGui::TableHeadersRow();

								// 显示预排序的方法数据（使用滚动条浏览）
								for (size_t i = 0; i < globalmanagement::sorted_methods.size(); ++i) {
									const auto& method = globalmanagement::sorted_methods[i];
									ImGui::TableNextRow();

									// 为静态方法设置蓝色背景
									if (method.isStatic) {
										ImGui::TableSetBgColor(ImGuiTableBgTarget_RowBg0, IM_COL32(100, 149, 237, 255)); // 蓝色背景
									}

									ImGui::TableNextColumn(); ImGui::Text("%s", method.name.c_str());
									if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", method.name.c_str());

									ImGui::TableNextColumn(); ImGui::Text("%s", method.returnType.c_str());
									if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", method.returnType.c_str());

									// 使用预排序的参数字符串缓存
									ImGui::TableNextColumn(); ImGui::Text("%s", globalmanagement::sorted_method_params[i].c_str());
									if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s", globalmanagement::sorted_method_params[i].c_str());
								}
								ImGui::EndTable();
							}
							ImGui::EndChild();
						}

						// 其他折叠栏
						if (ImGui::CollapsingHeader("其他")) {
							// Tag搜索区域
							{
								ImGui::Text("Tag 标签寻找");

								// 当前Tag显示（不可编辑的输入框）
								ImGui::Text("当前Tag:");
								ImGui::SameLine();
								ImGui::SetNextItemWidth(-FLT_MIN);
								ImGui::InputText("##currentTag", (char*)g_currentTag.c_str(), g_currentTag.size() + 1, ImGuiInputTextFlags_ReadOnly);

								// 当前实例搜索按钮
								bool hasObjects = !g_objectPointers.empty();
								if (hasObjects) {
									if (ImGui::Button("当前实例搜索", ImVec2(-FLT_MIN, 0))) {
										if ((int)g_objectPointers.size() > 0) {
											// TODO: 实现Tag搜索逻辑
											Component* temp_object = (Component*)g_objectPointers[0];
											string temp_tags = temp_object->get_tag();
											if (temp_tags != "Untagged") {
												g_currentTag = temp_tags;
											}
											else {
												g_currentTag = "nullptr";
											}
										}
									}
								}
								else {
									ImGui::BeginDisabled();
									ImGui::Button("当前实例搜索", ImVec2(-FLT_MIN, 0));
									ImGui::EndDisabled();
								}
							}
							ImGui::Separator();
							// Copy代码
							{
								ImGui::Text("结构生成:");
								if (ImGui::Button("复制类结构到剪贴板", ImVec2(-FLT_MIN, 0))) {
									string Structure_text = globalmanagement::get_Class_Structure(g_selNamespace + "." + g_selClass, g_selAssembly);

									// 复制到剪贴板
									if (!Structure_text.empty() && Structure_text.find("// Error:") == string::npos) {
										if (OpenClipboard(NULL)) {
											EmptyClipboard();
											size_t size = (Structure_text.length() + 1) * sizeof(wchar_t);
											HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
											if (hGlobal) {
												wchar_t* pGlobal = (wchar_t*)GlobalLock(hGlobal);
												if (pGlobal) {
													// 转换为宽字符
													size_t converted = 0;
													mbstowcs_s(&converted, pGlobal, size / sizeof(wchar_t), Structure_text.c_str(), _TRUNCATE);
													GlobalUnlock(hGlobal);
													SetClipboardData(CF_UNICODETEXT, hGlobal);
												}
												GlobalFree(hGlobal);
											}
											CloseClipboard();
										}
									}
								}
								ImGui::Separator();
								if (ImGui::Button("结构解析器", ImVec2(-FLT_MIN, 0))) {
									show_structure_analyzer = true;
								}

								// 结构分析器窗口
								if (ImGui::Begin("结构分析器", &show_structure_analyzer, ImGuiWindowFlags_AlwaysAutoResize)) {
									// 显示当前选择的类，处理没有命名空间的情况
									std::string displayClass = g_selClass.empty() ? "未选择" : g_selClass;
									if (!g_selNamespace.empty() && g_selNamespace != "None") {
										displayClass = g_selNamespace + "." + g_selClass;
									}
									
									// 使用紧凑的样式
									ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));
									ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 2));
									
									ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.9f, 1.0f), "当前类:");
									ImGui::SameLine();
									ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "%s", displayClass.c_str());
									
									ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.9f, 1.0f), "程序集:");
									ImGui::SameLine();
									ImGui::Text("%s", g_selAssembly.empty() ? "未选择" : g_selAssembly.c_str());
									
									ImGui::Separator();
									
									// 输入区域
									ImGui::Text("实例地址 (Hex):");
									ImGui::SameLine();
									ImGui::SetNextItemWidth(250);
									ImGui::InputText("##instanceAddr", g_instanceAddr, IM_ARRAYSIZE(g_instanceAddr));
									
									// 帮助提示
									ImGui::SameLine();
									ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "(如: 0x12345678)");
									
									// 按钮区域
									ImGui::SetNextItemWidth(120);
									if (ImGui::Button("解析")) {
										// 解析地址
										uintptr_t addr = 0;
										const char* addrStr = g_instanceAddr;
										if (addrStr && strlen(addrStr) > 0) {
											if (strstr(addrStr, "0x") == addrStr || strstr(addrStr, "0X") == addrStr) {
												addr = strtoull(addrStr, nullptr, 16);
											}
											else {
												addr = strtoull(addrStr, nullptr, 16);
											}
										}

										// 查找程序集
										const Il2CppAssembly* assembly = nullptr;
										if (!g_selAssembly.empty()) {
											Il2CppDomain* domain = il2cpp_domain_get();
											if (domain) {
												assembly = il2cpp_domain_assembly_open(domain, g_selAssembly.c_str());
											}
										}

										if (!assembly) {
											ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "请选择有效的程序集");
										}
										else if (addr == 0) {
											ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "请输入有效的实例地址");
										}
										else if (g_selClass.empty()) {
											ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "请选择一个类");
										}
										else {
											// 处理命名空间为空或为 "None" 的情况
											std::string ns = g_selNamespace;
											if (ns == "None") ns = "";

											// 保存解析参数
											parsedAssembly = g_selAssembly;
											parsedNamespace = ns;
											parsedClass = g_selClass;
											parsedAddr = addr;
											parsed = true;
										}
									}

									ImGui::SameLine();
									ImGui::SetNextItemWidth(80);
									if (ImGui::Button("关闭")) {
										memset(g_instanceAddr, 0, sizeof(g_instanceAddr));
										parsed = false;
										show_structure_analyzer = false;
									}
									
									ImGui::SameLine();
									ImGui::SetNextItemWidth(100);
									if (ImGui::Button("查找引用")) {
										// 开始搜索引用
										refClasses.clear();
										searching_refs = true;
										refs_searched = false;
										show_find_refs = true;
										
										// 调试：保存前几个找到的typeName用于分析
										std::vector<std::string> debugTypeNames;
										
										// 搜索所有程序集中哪些类的字段引用了当前类
										Il2CppDomain* domain = il2cpp_domain_get();
										if (domain) {
											size_t asmCount = 0;
											const Il2CppAssembly** assemblies = il2cpp_domain_get_assemblies(domain, &asmCount);
											
											for (size_t a = 0; a < asmCount; a++) {
												const Il2CppAssembly* asmPtr = assemblies[a];
												if (!asmPtr) continue;
												
												const Il2CppImage* image = il2cpp_assembly_get_image(asmPtr);
												if (!image) continue;
												
												size_t classCount = il2cpp_image_get_class_count(image);
												for (size_t c = 0; c < classCount; c++) {
													Il2CppClass* klass = (Il2CppClass*)il2cpp_image_get_class(image, c);
													if (!klass) continue;
												
													// 遍历该类的所有字段
													void* fieldIter = nullptr;
													FieldInfo* field = nullptr;
													while ((field = il2cpp_class_get_fields(klass, &fieldIter)) != nullptr) {
														if (!field || !field->name) continue;
														
														Il2CppType* fieldType = (Il2CppType*)field->type;
														if (!fieldType) continue;
														
												const char* typeName = il2cpp_type_get_name(fieldType);
												if (!typeName) continue;
												
												// 检查字段类型是否匹配目标类
												std::string fieldTypeName(typeName);
												
												// 简单匹配逻辑：
												// 1. 直接比较类名
												// 2. 或者类型名以 ".类名" 结尾
												// 3. 或者类型名以 "<类名" 结尾（泛型）
												bool isMatch = false;
												
												// 精确匹配类名（字段类型名就是类名）
												if (fieldTypeName == parsedClass) {
													isMatch = true;
												}
												// 检查是否以 ".类名" 或 "<类名" 结尾
												else {
													std::string suffixDot = "." + parsedClass;
													std::string suffixAngle = "<" + parsedClass;
													if (fieldTypeName.length() > parsedClass.length()) {
														if (fieldTypeName.find(suffixDot) != std::string::npos ||
														    fieldTypeName.find(suffixAngle) != std::string::npos) {
															isMatch = true;
														}
													}
												}
												
												if (isMatch) {
															// 找到引用
															const char* refNs = il2cpp_class_get_namespace(klass);
															const char* refName = il2cpp_class_get_name(klass);
															if (refName) {
																std::string refFullName;
																if (refNs && strlen(refNs) > 0) {
																	refFullName = std::string(refNs) + "." + std::string(refName);
																} else {
																	refFullName = std::string(refName);
																}
																// 避免重复添加
																bool alreadyExists = false;
																for (const auto& existing : refClasses) {
																	if (existing == refFullName) {
																		alreadyExists = true;
																		break;
																	}
																}
																if (!alreadyExists) {
																	refClasses.push_back(refFullName);
																}
															}
														}
													}
												}
											}
										}
										searching_refs = false;
										refs_searched = true;
									}
									
									ImGui::PopStyleVar(2);

									// 如果已经解析过，显示结果
									if (parsed) {
										ImGui::Separator();
										ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "=== 解析结果 ===");
										
										// 使用滚动区域 - 自适应大小
										float contentHeight = ImGui::GetContentRegionAvail().y;
										if (contentHeight < 200) contentHeight = 200;
										if (contentHeight > 600) contentHeight = 600;
										
										ImGui::BeginChild("ParseResult", ImVec2(600, contentHeight), true);
										
										// 查找程序集
										const Il2CppAssembly* assembly = nullptr;
										Il2CppDomain* domain = il2cpp_domain_get();
										if (domain) {
											assembly = il2cpp_domain_assembly_open(domain, parsedAssembly.c_str());
										}

										if (assembly) {
											globalmanagement::DisplayInstanceStructure(const_cast<Il2CppAssembly*>(assembly), parsedNamespace, parsedClass, parsedAddr, 0);
										}
										ImGui::EndChild();
										
										// 显示帮助信息
										ImGui::Separator();
										ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 
											"提示: 点击树形节点可以展开/折叠嵌套对象");
									}
									ImGui::End();
								}
								
								// 查找引用结果窗口
								if (show_find_refs) {
									if (ImGui::Begin("查找引用结果", &show_find_refs, ImGuiWindowFlags_AlwaysAutoResize)) {
										ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.9f, 1.0f), "搜索引用: %s", parsedClass.c_str());
										ImGui::Separator();
										
										if (searching_refs) {
											ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "正在搜索...");
										}
										else if (refs_searched) {
											ImGui::Text("找到 %zu 个类引用了该类:", refClasses.size());
											ImGui::Separator();
											
											// 显示所有引用类
											for (size_t i = 0; i < refClasses.size(); i++) {
												const auto& refClass = refClasses[i];
												char label[512];
												sprintf_s(label, "##ref_%zu", i);
												
												// 解析命名空间和类名
												size_t lastDot = refClass.find_last_of('.');
												std::string refNs, refName;
												if (lastDot != std::string::npos) {
													refNs = refClass.substr(0, lastDot);
													refName = refClass.substr(lastDot + 1);
												} else {
													refNs = "";
													refName = refClass;
												}
												
												char treeLabel[512];
												sprintf_s(treeLabel, "%s (%zu)", refClass.c_str(), i + 1);
												
												if (ImGui::TreeNode(treeLabel)) {
													ImGui::TextColored(ImVec4(0.6f, 0.8f, 0.6f, 1.0f), "类名: %s", refName.c_str());
													if (!refNs.empty()) {
														ImGui::TextColored(ImVec4(0.6f, 0.6f, 0.8f, 1.0f), "命名空间: %s", refNs.c_str());
													}
													
													// 显示该引用类的结构
													ImGui::Separator();
													ImGui::Text("类结构:");
													
													// 查找程序集并显示结构
													Il2CppDomain* domain = il2cpp_domain_get();
													if (domain) {
														// 遍历所有程序集查找该类
														size_t asmCount = 0;
														const Il2CppAssembly** assemblies = il2cpp_domain_get_assemblies(domain, &asmCount);
														
														for (size_t a = 0; a < asmCount; a++) {
															const Il2CppAssembly* asmPtr = assemblies[a];
															if (!asmPtr) continue;
															
															const Il2CppImage* image = il2cpp_assembly_get_image(asmPtr);
															if (!image) continue;
															
															// 尝试在该程序集中查找类
															Il2CppClass* klass = il2cpp_class_from_name(image, refNs.empty() ? "" : refNs.c_str(), refName.c_str());
															if (klass) {
																// 找到类，显示其所有实例（这里需要用户输入地址，或者显示静态字段）
																ImGui::TextColored(ImVec4(0.5f, 0.8f, 0.5f, 1.0f), "程序集: %s", il2cpp_image_get_name(image));
																ImGui::Text("提示: 需要实例地址才能查看详细结构");
																break;
															}
														}
													}
													
													ImGui::TreePop();
												}
											}
											
											if (refClasses.empty()) {
												ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.8f, 1.0f), "未找到引用该类的其他类");
											}
										}
										
										ImGui::Separator();
										if (ImGui::Button("关闭")) {
											show_find_refs = false;
										}
										ImGui::End();
									}
								}

								// 大文件确认弹窗
								if (ImGui::BeginPopupModal("大文件确认", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
									float pendingSizeMB = pending_dump_size / (1024.0f * 1024.0f);
									ImGui::TextColored(ImVec4(1.0f, 0.3f, 0.3f, 1.0f), "警告: 文件大小超过1GB!");
									ImGui::Text("即将Dump的大小: %d MB (%d GB)", (int)pendingSizeMB, (int)(pendingSizeMB / 1024.0f));
									ImGui::Text("这可能需要较长时间且占用大量磁盘空间。");
									ImGui::Separator();
									if (ImGui::Button("确认Dump", ImVec2(120, 0))) {
										if (globalmanagement::DumpMetadataToFile(pending_metadata_addr, pending_dump_size, pending_save_path.c_str())) {
											InfoMesseng::ColorPrint("SUCCESS", ("Dump saved to: " + pending_save_path).c_str(), 0);
										}
										else {
											InfoMesseng::ColorPrint("ERROR", ("Failed to save dump to: " + pending_save_path).c_str(), 1);
										}
										ImGui::CloseCurrentPopup();
									}
									ImGui::SameLine();
									if (ImGui::Button("取消", ImVec2(120, 0))) {
										ImGui::CloseCurrentPopup();
									}
									ImGui::EndPopup();
								}
							}
							ImGui::Separator();
							// Dump dat 区域
							{
								ImGui::Text("Dump global-metadata.dat");
								ImGui::Text("输入赋值地址:");
								ImGui::SameLine();
								ImGui::SetNextItemWidth(-FLT_MIN);
								ImGui::InputText("##dumpPath", g_dumpAddr, IM_ARRAYSIZE(g_dumpAddr));
								ImGui::Separator();
								if (ImGui::Button("Dump", ImVec2(-FLT_MIN, 0))) {
									// 解析 g_dumpAddr，格式1：模块名+偏移量，例如 GameAssembly.dll+0x1000
									// 格式2：直接地址，例如 0x12345678
									string addrStr = g_dumpAddr;
									string moduleName;
									string offsetStr;
									if (!addrStr.empty()) {
										// 获取当前程序路径用于保存文件
										char exePath[MAX_PATH] = { 0 };
										GetModuleFileNameA(NULL, exePath, MAX_PATH);
										// 去掉文件名，只保留目录
										char* lastSlash = strrchr(exePath, '\\');
										if (lastSlash) *(lastSlash + 1) = '\0';
										string savePath = string(exePath) + "global-metadata.dat";

										// 使用 + 分隔，最多分隔两个
										size_t plusPos = addrStr.find('+');
										if (plusPos != string::npos) {
											// 格式1：模块名+偏移量
											moduleName = addrStr.substr(0, plusPos);
											offsetStr = addrStr.substr(plusPos + 1);
											HMODULE hModule = GetModuleHandleA(moduleName.c_str());
											if (hModule) {
												uintptr_t offset = 0;
												if (offsetStr.find("0x") == 0 || offsetStr.find("0X") == 0) {
													offset = std::stoull(offsetStr, nullptr, 16);
												}
												else {
													offset = std::stoull(offsetStr, nullptr, 10);
												}
												uintptr_t metadataAddr = (uintptr_t)hModule + offset;
												Il2CppGlobalMetadataHeader* header = (Il2CppGlobalMetadataHeader*)metadataAddr;

												// 调用 CalculateMetadataSize
												int32_t size = globalmanagement::CalculateMetadataSize(header);
												float sizeMB = size / (1024.0f * 1024.0f);
												InfoMesseng::ColorPrint("SUCCESS", ("Metadata Size: " + std::to_string(size) + " [" + std::to_string((int)sizeMB) + " MB]").c_str(), 0);

												// 大于1GB时弹出确认框
												if (sizeMB > 1000.0f) {
													pending_dump_size = size;
													pending_metadata_addr = metadataAddr;
													pending_save_path = savePath;
													ImGui::OpenPopup("大文件确认");
												}
												else if (globalmanagement::DumpMetadataToFile(metadataAddr, size, savePath.c_str())) {
													InfoMesseng::ColorPrint("SUCCESS", ("Dump saved to: " + savePath).c_str(), 0);
												}
												else {
													InfoMesseng::ColorPrint("ERROR", ("Failed to save dump to: " + savePath).c_str(), 1);
												}
											}
											else {
												InfoMesseng::ColorPrint("ERROR", ("Failed to get module: " + moduleName).c_str(), 1);
											}
										}
										else {
											uintptr_t metadataAddr = std::stoull(addrStr, nullptr, 16);
											Il2CppGlobalMetadataHeader* header = (Il2CppGlobalMetadataHeader*)metadataAddr;
											int32_t size = globalmanagement::CalculateMetadataSize(header);
											float sizeMB = size / (1024.0f * 1024.0f);
											InfoMesseng::ColorPrint("SUCCESS", ("Metadata Size: " + std::to_string(size) + " [" + std::to_string((int)sizeMB) + " MB]").c_str(), 0);

											// 大于1GB时弹出确认框
											if (sizeMB > 1000.0f) {
												pending_dump_size = size;
												pending_metadata_addr = metadataAddr;
												pending_save_path = savePath;
												ImGui::OpenPopup("大文件确认");
											}
											else if (globalmanagement::DumpMetadataToFile(metadataAddr, size, savePath.c_str())) {
												InfoMesseng::ColorPrint("SUCCESS", ("Dump saved to: " + savePath).c_str(), 0);
											}
											else {
												InfoMesseng::ColorPrint("ERROR", ("Failed to save dump to: " + savePath).c_str(), 1);
											}
										}
									}
								}
								ImGui::Separator();
								if (ImGui::Button("扫描MetadataHeader赋值地址", ImVec2(-FLT_MIN, 0))) {
									unsigned char pattern[] = { 0x48, 0xB8, 0xCD, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC, 0xCC,0x48, 0xF7, 0xE1, 0x48, 0xC1, 0xEA, 0x05 };
									unsigned char pattern2[] = { 0x48,0x8B,0x5C,0x24,0x30,0x48,0x83,0xC4,0x20,0x5F,0xC3,0x48,0x63,0x88 }; // 备用特征码 取 mov r8,rax 签名的mov [xxx],xxx 值
									vector<uintptr_t> addrs;
									vector<uintptr_t> OutPutaddrs;
									int count = MemoryOperation::ScanFeature("GameAssembly.dll", pattern, sizeof(pattern), addrs);
									int expectedCount = 0;
									if (count == 1) {
										for (int i = 0; i > -500; i--) {
											uintptr_t check = addrs[0] + i;
											if (check < (uintptr_t)GetModuleHandleA("GameAssembly.dll"))
												break;
											if (*(unsigned char*)check == 0x48 && *(unsigned char*)(check + 1) == 0x8B && *(unsigned char*)(check + 2) == 0x05) {
												// 计算 RVA 差值验证
												size_t diff = addrs[0] - check;
												uintptr_t val = MemoryOperation::ReadRipRelativeValue(check);
												if (val != 0 && val <= 0x7FFE00000000) {
													OutPutaddrs.push_back(val);
												}
												if (expectedCount == 2) {
													break;
												}
												else {
													expectedCount++;
												}
											}
										}
										if (OutPutaddrs.size() > 0) {
											if (OutPutaddrs.size() > 1) {
												InfoMesseng::ColorPrint("WARING", "Multiple Metadata Header addresses were found during the search", 2);
											}
											for (int i = 0; i < OutPutaddrs.size(); i++) {
												uintptr_t metadataAddr = OutPutaddrs[i];
												char hexAddr[32];
												sprintf_s(hexAddr, "%llX", (unsigned long long)metadataAddr);
												InfoMesseng::ColorPrint("SUCCESS", ("Auto Find MetadataHeader Addr : " + string(hexAddr)).c_str(), 0);
											}
										}
										else {
											InfoMesseng::ColorPrint("ERROR", "Failed to find valid MetadataHeader address", 1);
										}
									}
									else {
										InfoMesseng::ColorPrint("ERROR", "The game's Metadata Header feature code was not found", 1);
									}
								}
							}
							ImGui::Separator();
							// Dump dump.cs
							{
								ImGui::Text("Dump SDK");
								ImGui::Separator();

								// 显示进度对话框
								if (dump_sdk_in_progress || dump_sdk_complete) {
									ImGui::OpenPopup("Dump SDK");
									if (ImGui::BeginPopupModal("Dump SDK", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
										if (dump_sdk_in_progress) {
											// 动画进度条
											if (dump_sdk_display_progress < dump_sdk_target_progress) {
												dump_sdk_display_progress += 0.005f;
												if (dump_sdk_display_progress > dump_sdk_target_progress) {
													dump_sdk_display_progress = dump_sdk_target_progress;
												}
											}
											ImGui::Text("正在Dump SDK，请稍候...");
											ImGui::Separator();
											ImGui::ProgressBar(dump_sdk_display_progress, ImVec2(300, 20), "Dumping...");
										}
										else if (dump_sdk_complete) {
											ImGui::Separator();
											if (dump_sdk_success) {
												ImGui::TextColored(ImVec4(0, 255, 0, 255), "Dump SDK 成功!");
												ImGui::Text("文件已保存至: %s", dump_sdk_result_path.c_str());
												if (ImGui::Button("打开文件所在目录", ImVec2(200, 0))) {
													ShellExecuteA(NULL, "open", dump_sdk_result_path.substr(0, dump_sdk_result_path.find_last_of("\\")).c_str(), NULL, NULL, SW_SHOWDEFAULT);
												}
											}
											else {
												ImGui::TextColored(ImVec4(255, 0, 0, 255), "Dump SDK 失败!");
											}
											ImGui::Separator();
											if (ImGui::Button("确定", ImVec2(120, 0))) {
												dump_sdk_in_progress = false;
												dump_sdk_complete = false;
												dump_sdk_progress = 0.0f;
												dump_sdk_target_progress = 0.0f;
												dump_sdk_display_progress = 0.0f;
												dump_sdk_success = false;
												dump_sdk_result_path = "";
												ImGui::CloseCurrentPopup();
											}
										}
										ImGui::EndPopup();
									}
								}

								if (ImGui::Button("Start Dump", ImVec2(-FLT_MIN, 0))) {
									ImGui::OpenPopup("确认 Dump SDK");
								}

								// 确认对话框
								if (ImGui::BeginPopupModal("确认 Dump SDK", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
									ImGui::Text("确定要Dump SDK吗?\n此操作可能需要一些时间。");
									ImGui::Separator();
									if (ImGui::Button("确定", ImVec2(120, 0))) {
										DumpSdk::il2cpp_dump_init(Engine::hGameAssembly);
										char exePath[MAX_PATH] = { 0 };
										GetModuleFileNameA(NULL, exePath, MAX_PATH);
										char* lastSlash = strrchr(exePath, '\\');
										if (lastSlash) *(lastSlash + 1) = '\0';
										string savePath = string(exePath);
										dump_sdk_in_progress = true;
										dump_sdk_progress = 0.0f;
										dump_sdk_target_progress = 0.0f;
										dump_sdk_display_progress = 0.0f;
										dump_sdk_complete = false;

										// 创建线程执行Dump
										thread([savePath]() {
											dump_sdk_target_progress = 0.1f;
											std::this_thread::sleep_for(std::chrono::milliseconds(100));
											bool success = DumpSdk::il2cpp_Dump2File(savePath);
											dump_sdk_target_progress = 1.0f;

											dump_sdk_success = success;
											dump_sdk_result_path = savePath + "dump.cs";
											dump_sdk_in_progress = false;
											dump_sdk_complete = true;
											}).detach();

										ImGui::CloseCurrentPopup();
									}
									ImGui::SameLine();
									if (ImGui::Button("取消", ImVec2(120, 0))) {
										ImGui::CloseCurrentPopup();
									}
									ImGui::EndPopup();
								}
							}
						}
						ImGui::EndChild();
					}

					ImGui::Columns(1);
					ImGui::End();
					ImGui::PopStyleVar(6);
					ImGui::PopFont();
				}

				// 内存扫描窗口
				if (show_memory_scanner) {
					if (focus_memory_scanner) {
						ImGui::SetNextWindowFocus();
						focus_memory_scanner = false;
					}
					// 设置窗口样式：微圆角，自适应大小
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
					ImGui::Begin("内存扫描", &show_memory_scanner, ImGuiWindowFlags_AlwaysAutoResize);
					ImGui::Text("这里是内存扫描功能");
					// TODO: 添加内存扫描相关功能
					ImGui::End();
					ImGui::PopStyleVar();
				}

				// 内存绘制窗口
				if (show_memory_drawer) {
					if (focus_memory_drawer) {
						ImGui::SetNextWindowFocus();
						focus_memory_drawer = false;
					}

					/* ---- 紧凑样式 ---- */
					ImGui::PushFont(smallFont);                       // 14 px
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 3));
					ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2, 2)); // 表格再扁
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5);
					ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);

					/* 关键：最小宽度 520（原来是 640）*/
					ImGui::SetNextWindowSizeConstraints(ImVec2(520, 300), ImVec2(FLT_MAX, FLT_MAX));

					ImGui::Begin("内存绘制", &show_memory_drawer);
					{

					}
					// 计算列高，留出按钮区域的空间
					float availableHeight = ImGui::GetContentRegionAvail().y;
					float buttonAreaHeight = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y; // 按钮高度 + 间距
					float unifiedHeight = availableHeight - buttonAreaHeight;

					// 创建自适应两列布局
					ImGui::Columns(2, "MemoryDrawerColumns", false); // false = 不等宽，让内容决定宽度

					/* -------- 第一列：内存地址 -------- */
					{
						ImGui::BeginChild("MDCol1", ImVec2(0, unifiedHeight), false);
						ImGui::Text("内存地址");
						ImGui::Separator();

						/* 地址输入框 */
						ImGui::SetNextItemWidth(-FLT_MIN);
						static char addressInput[256] = "";
						ImGui::InputTextWithHint("##addressInput", "输入内存地址 (0x...):", addressInput, sizeof(addressInput));

						/* 动态地址列表 - 填满剩余空间 */
						static vector<string> addressList;
						static int selectedAddressIndex = -1;

						ImGui::SetNextItemWidth(-FLT_MIN);
						if (ImGui::BeginListBox("##addressList", ImVec2(-FLT_MIN, -FLT_MIN))) {
							for (int i = 0; i < (int)addressList.size(); ++i) {
								bool isSelected = (selectedAddressIndex == i);
								if (ImGui::Selectable(addressList[i].c_str(), isSelected)) {
									selectedAddressIndex = i;
								}
								if (isSelected) {
									ImGui::SetItemDefaultFocus();
								}
							}
							ImGui::EndListBox();
						}

						ImGui::EndChild();

						/* 按钮区域 - 在child外面 */
						float buttonWidth = (ImGui::GetContentRegionAvail().x - ImGui::GetStyle().ItemSpacing.x) * 0.5f;

						if (ImGui::Button("添加地址", ImVec2(buttonWidth, 0))) {
							if (strlen(addressInput) > 0) {
								string newAddress = addressInput;

								// 检查是否已存在
								auto it = find(addressList.begin(), addressList.end(), newAddress);
								if (it != addressList.end()) {
									// 如果存在，选中它
									selectedAddressIndex = static_cast<int>(distance(addressList.begin(), it));
								}
								else {
									// 如果不存在，添加它并选中
									addressList.push_back(newAddress);
									selectedAddressIndex = (int)addressList.size() - 1;
								}

								// 清空输入框
								addressInput[0] = '\0';
							}
						}

						ImGui::SameLine();
						if (ImGui::Button("移除地址", ImVec2(buttonWidth, 0))) {
							if (selectedAddressIndex >= 0 && selectedAddressIndex < (int)addressList.size()) {
								addressList.erase(addressList.begin() + selectedAddressIndex);
								if (selectedAddressIndex >= (int)addressList.size()) {
									selectedAddressIndex = (int)addressList.size() - 1;
								}
							}
						}

						ImGui::NextColumn();
					}

					/* -------- 第二列：绘制设置 -------- */
					{
						ImGui::BeginChild("MDCol2", ImVec2(0, unifiedHeight), false);
						ImGui::Text("绘制设置");
						ImGui::Separator();

						/* 类型选择 */
						const char* drawTypes[] = { "矩形", "圆形", "线条", "文本" };
						static int selectedDrawType = 0;
						ImGui::SetNextItemWidth(-FLT_MIN);
						if (ImGui::BeginCombo("绘制类型##drawType", drawTypes[selectedDrawType])) {
							for (int i = 0; i < IM_ARRAYSIZE(drawTypes); i++) {
								bool isSelected = (selectedDrawType == i);
								if (ImGui::Selectable(drawTypes[i], isSelected)) {
									selectedDrawType = i;
								}
								if (isSelected) {
									ImGui::SetItemDefaultFocus();
								}
							}
							ImGui::EndCombo();
						}

						ImGui::Spacing();

						/* 颜色设置 */
						static ImVec4 drawColor = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
						ImGui::ColorEdit4("绘制颜色##drawColor", (float*)&drawColor);

						ImGui::Spacing();

						/* 大小设置 */
						static float drawSize = 10.0f;
						ImGui::SetNextItemWidth(-FLT_MIN);
						ImGui::SliderFloat("##drawSize", &drawSize, 1.0f, 100.0f, "大小: %.1f");

						ImGui::Spacing();

						/* 绘制按钮 */
						ImGui::SetNextItemWidth(-FLT_MIN);
						if (ImGui::Button("开始绘制", ImVec2(-FLT_MIN, 0))) {
							// TODO: 开始绘制逻辑
						}

						ImGui::Spacing();

						/* 清除按钮 */
						ImGui::SetNextItemWidth(-FLT_MIN);
						if (ImGui::Button("清除绘制", ImVec2(-FLT_MIN, 0))) {
							// TODO: 清除绘制逻辑
						}

						ImGui::Spacing();


						if (ImGui::CollapsingHeader("工具")) {
							ImGui::Text("这里是一些工具选项");
							ImGui::Separator();

							// 是否可视选中框
							static bool isVisible = true;
							ImGui::Checkbox("是否可视", &isVisible);

							ImGui::Spacing();

							// 获取坐标按钮和显示逻辑
							static bool showCoordinates = false;
							static chrono::steady_clock::time_point showStartTime;

							if (ImGui::Button("获取坐标", ImVec2(-FLT_MIN, 0))) {
								showCoordinates = true;
								showStartTime = chrono::steady_clock::now();
							}

							// 检查是否应该隐藏坐标显示（3秒后）
							if (showCoordinates) {
								auto currentTime = chrono::steady_clock::now();
								auto elapsed = chrono::duration_cast<chrono::seconds>(currentTime - showStartTime);
								if (elapsed.count() >= 3) {
									showCoordinates = false;
								}
							}

							// 显示坐标信息
							if (showCoordinates) {
								ImGui::Text("实体坐标: xx,xx,xx");
								ImGui::Separator();
							}
						}

						ImGui::EndChild();
					}

					ImGui::Columns(1);
					ImGui::End();
					ImGui::PopStyleVar(6);
					ImGui::PopFont();
				}

				// 着色器管理器窗口
				if (show_shader_drawer) {
					static bool is_first_open = false;
					static vector<string> shaderList = { };
					static Shader* currentShader = nullptr;

					if (focus_shader_drawer) {
						ImGui::SetNextWindowFocus();
						focus_shader_drawer = false;
					}

					if (!is_first_open) {
						if (globalshader::init()) {
							InfoMesseng::ColorPrint("SUCCESS", "globalshader init Success!", 0);
							shaderList = globalshader::shaderNames;
						}
						else {
							InfoMesseng::ColorPrint("ERROR", "globalshader init error!", 1);
						}
						is_first_open = true;
					}

					/* ---- 紧凑样式 ---- */
					ImGui::PushFont(smallFont);                       // 14 px
					ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5, 5));
					ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 2));
					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 3));
					ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2, 2)); // 表格再扁
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5);
					ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);

					/* 关键：最小宽度 600（着色器需要更多空间）*/
					ImGui::SetNextWindowSizeConstraints(ImVec2(600, 400), ImVec2(FLT_MAX, FLT_MAX));

					ImGui::Begin("着色器管理器", &show_shader_drawer);

					// 计算列高，留出按钮区域的空间
					float availableHeight = ImGui::GetContentRegionAvail().y;
					float buttonAreaHeight = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y; // 按钮高度 + 间距
					float unifiedHeight = availableHeight - buttonAreaHeight;

					// 创建自适应两列布局
					ImGui::Columns(2, "ShaderDrawerColumns", false); // false = 不等宽，让内容决定宽度

					/* 选中的着色器索引（需要在两列之间共享） */
					static int selectedShaderIndex = -1;

					/* -------- 第一列：着色器列表 -------- */
					{
						// 存储新出现的着色器名称（在刷新按钮中更新，在列表渲染中使用）
						static vector<string> newShaderNames;

						ImGui::BeginChild("SDCol1", ImVec2(0, unifiedHeight), false);
						ImGui::Text("着色器列表");
						ImGui::Separator();

						/* 搜索框 */
						static char shaderFilter[256] = "";
						ImGui::SetNextItemWidth(-FLT_MIN);
						ImGui::InputTextWithHint("##shaderFilter", "搜索着色器...", shaderFilter, sizeof(shaderFilter));

						/* 对列表进行排序和过滤 */
						static vector<string> sortedShaderList;
						static string lastShaderListHash = "";
						static string lastFilter = "";

						// 生成当前列表的哈希（用于检测是否需要重新排序）
						string currentHash = "";
						for (const auto& s : shaderList) {
							currentHash += s;
						}

						// 如果列表或过滤器发生变化，重新排序和过滤
						if (currentHash != lastShaderListHash || string(shaderFilter) != lastFilter) {
							sortedShaderList.clear();
							for (const auto& s : shaderList) {
								// 转换为小写进行不区分大小写的搜索
								string lowerS = s;
								string lowerFilter = shaderFilter;
								for (char& c : lowerS) c = (char)::tolower(c);
								for (char& c : lowerFilter) c = (char)::tolower(c);

								// 如果搜索框为空或名称包含搜索文本，则添加到列表
								if (lowerFilter.empty() || lowerS.find(lowerFilter) != string::npos) {
									sortedShaderList.push_back(s);
								}
							}
							// 按字母顺序排序（A~Z）
							std::sort(sortedShaderList.begin(), sortedShaderList.end(),
								[](const string& a, const string& b) {
									string lowerA = a, lowerB = b;
									for (char& c : lowerA) c = (char)::tolower(c);
									for (char& c : lowerB) c = (char)::tolower(c);
									return lowerA < lowerB;
								});
							lastShaderListHash = currentHash;
							lastFilter = shaderFilter;
						}

						ImGui::Spacing();

						/* 动态着色器列表 - 填满剩余空间 */
						ImGui::SetNextItemWidth(-FLT_MIN);
						if (ImGui::BeginListBox("##shaderList", ImVec2(-FLT_MIN, -FLT_MIN))) {
							for (int i = 0; i < (int)sortedShaderList.size(); ++i) {
								// 在原始列表中查找索引
								int originalIndex = -1;
								for (int j = 0; j < (int)shaderList.size(); ++j) {
									if (shaderList[j] == sortedShaderList[i]) {
										originalIndex = j;
										break;
									}
								}

								// 检查是否为新出现的着色器
								bool isNewShader = false;
								for (const auto& newName : newShaderNames) {
									if (sortedShaderList[i] == newName) {
										isNewShader = true;
										break;
									}
								}

								bool isSelected = (selectedShaderIndex == originalIndex);

								// 如果是新着色器，设置淡蓝色背景
								if (isNewShader) {
									// 使用更高的 alpha 值使背景更明显
									ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.4f, 0.6f, 0.9f, 0.7f));  // 淡蓝色背景（提高透明度）
									ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.6f, 0.9f, 0.9f));  // 悬停时更深
									ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.3f, 0.5f, 0.8f, 1.0f));  // 激活时更深
								}

								// 添加唯一标识符避免 ID 冲突
								char label[512];
								sprintf_s(label, sizeof(label), "%s##shader_%d", sortedShaderList[i].c_str(), i);
								// 如果是新着色器，使用 Highlight 标志强制显示背景，并设置为选中状态以确保背景显示
								if (ImGui::Selectable(label, isSelected || isNewShader, isNewShader ? ImGuiSelectableFlags_Highlight : ImGuiSelectableFlags_None)) {
									selectedShaderIndex = originalIndex;
									// 清空属性列表
									globalshader::cachedProperties.clear();
									globalshader::cachedSelectedShader = nullptr;
									// 点击时调用 get_shader_by_name 获取属性
									if (globalshader::get_shader_by_name(sortedShaderList[i])) {
										currentShader = globalshader::temp_sle_Shader;
										InfoMesseng::ColorPrint("SUCCESS", ("Loaded properties for shader: " + sortedShaderList[i]).c_str(), 0);
										globalshader::GetShader();

									}
									else {
										InfoMesseng::ColorPrint("ERROR", ("Failed to load properties for shader: " + sortedShaderList[i]).c_str(), 1);
									}
								}

								// 如果是新着色器且未选中，手动绘制背景以确保可见
								if (isNewShader && !isSelected) {
									ImVec2 itemMin = ImGui::GetItemRectMin();
									ImVec2 itemMax = ImGui::GetItemRectMax();
									ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, IM_COL32(102, 153, 230, 180), 0.0f);  // 淡蓝色背景，RGBA(102,153,230,180)
								}

								// 恢复样式（如果设置了）
								if (isNewShader) {
									ImGui::PopStyleColor(3);  // 恢复 3 个颜色设置
								}

								if (isSelected) {
									ImGui::SetItemDefaultFocus();
								}
							}
							ImGui::EndListBox();
						}

						ImGui::EndChild();
						if (ImGui::Button("刷新着色器", ImVec2(-FLT_MIN, 0))) {
							if (globalshader::init()) {
								InfoMesseng::ColorPrint("SUCCESS", "globalshader refurbish Success!", 0);
								// 找出新出现的着色器
								newShaderNames.clear();
								for (const auto& name : globalshader::shaderNames) {
									bool isNew = true;
									for (const auto& cachedName : globalshader::cachedShaderNames) {
										if (name == cachedName) {
											isNew = false;
											break;
										}
									}
									if (isNew) {
										newShaderNames.push_back(name);
									}
								}
								shaderList = globalshader::shaderNames;
								// 清空缓存，强制重新排序
								lastShaderListHash = "";
							}
							else {
								InfoMesseng::ColorPrint("ERROR", "globalshader refurbish error!", 1);
							}
						}

						ImGui::NextColumn();
					}

					/* -------- 第二列：着色器属性 -------- */
					{
						ImGui::BeginChild("SDCol2", ImVec2(0, unifiedHeight), false);
						ImGui::Text("着色器属性");
						ImGui::Separator();

						// 显示当前选中着色器名称
						if (selectedShaderIndex >= 0 && selectedShaderIndex < (int)shaderList.size()) {
							ImGui::Text("当前着色器: %s", shaderList[selectedShaderIndex].c_str());
						}
						else {
							ImGui::Text("当前着色器: 未选中");
						}

						// 获取属性列表
						static int selectedPropertyIndex = -1;
						std::vector<ShaderPropertyInfo> properties = globalshader::cachedProperties;

						if (!properties.empty()) {
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();

							ImGui::Text("属性列表 :");

							ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
							for (int i = 0; i < (int)properties.size(); ++i) {
								float baseY = ImGui::GetCursorPosY();
								const char* typeNames[] = { "Color", "Vector", "Float", "Range", "Texture", "Int" };
								const char* typeName = (properties[i].type >= 0 && properties[i].type <= 5) ? typeNames[properties[i].type] : "Unknown";
								float centerY = baseY + (ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) / 2;
								ImGui::SetCursorPosY(centerY);
								ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "[%s]", typeName);
								ImGui::SameLine();
								ImGui::Text("%s:", properties[i].name.c_str());
								ImGui::SameLine();

								ImGui::PushID(i);
								ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
								ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);

								// 根据类型显示对应的只读控件
								switch (properties[i].type) {
								case ShaderType_Float:
								case ShaderType_Range:
								{
									float val = properties[i].floatValue;
									char buf[64];
									sprintf_s(buf, sizeof(buf), "%.4f", val);
									ImGui::InputText("##val", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
									break;
								}
								case ShaderType_Int:
								{
									int val = properties[i].intValue;
									char buf[64];
									sprintf_s(buf, sizeof(buf), "%d", val);
									ImGui::InputText("##val", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
									break;
								}
								case ShaderType_Vector:
								{
									ImVec4 val(properties[i].vectorValue.x, properties[i].vectorValue.y,
										properties[i].vectorValue.z, properties[i].vectorValue.w);
									char buf[128];
									sprintf_s(buf, sizeof(buf), "(%.4f, %.4f, %.4f, %.4f)", val.x, val.y, val.z, val.w);
									ImGui::InputText("##val", buf, sizeof(buf), ImGuiInputTextFlags_ReadOnly);
									break;
								}
								case ShaderType_Color:
								{
									ImVec4 val(properties[i].colorValue.r, properties[i].colorValue.g,
										properties[i].colorValue.b, properties[i].colorValue.a);
									ImGui::ColorEdit4("##val", (float*)&val, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
									break;
								}
								case ShaderType_Texture:
								{
									ImGui::Text("%s", properties[i].textureValue.c_str());
									break;
								}
								default:
									ImGui::Text("Unknown");
									break;
								}

								ImGui::PopStyleColor();
								ImGui::PopItemFlag();
								ImGui::PopID();

								// 重置 Y 位置到基准线，确保下一行对齐
								ImGui::SetCursorPosY(baseY + ImGui::GetFrameHeightWithSpacing());

								if (i < (int)properties.size() - 1) {
									ImGui::Spacing();
								}
							}
							ImGui::PopStyleVar();
						}
						else {
							ImGui::Spacing();
							ImGui::Text("没有可用的属性");
							ImGui::Text("请先选择一个着色器");
						}

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::Text("全局操作:");
						static bool temp_bool = false;
						ImGui::Checkbox("开启修改", &temp_bool);
						if (temp_bool) {
							static int temp_int = 0;
							if (ImGui::DragInt("设定画质等级##batchInt", &temp_int, 1)) {
								::SetGlobalMaximumLOD(temp_int);
							}
						}
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::EndChild();
					}

					ImGui::Columns(1);
					ImGui::End();
					ImGui::PopStyleVar(6);
					ImGui::PopFont();
				}

				// 材料绘制窗口
				if (show_material_drawer) {
					if (focus_material_drawer) {
						ImGui::SetNextWindowFocus();
						focus_material_drawer = false;
					}

					ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 3));
					ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(2, 2));
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 5);
					ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3);
					ImGui::PushFont(smallFont);

					ImGui::SetNextWindowSizeConstraints(ImVec2(600, 400), ImVec2(FLT_MAX, FLT_MAX));

					ImGui::Begin("材质管理器", &show_material_drawer);

					// 首次打开时初始化
					static bool material_init_done = false;
					if (!material_init_done) {
						globalmaterial::init();
						material_init_done = true;
					}

					// 计算列高，留出按钮区域的空间
					float availableHeight = ImGui::GetContentRegionAvail().y;
					float buttonAreaHeight = ImGui::GetFrameHeightWithSpacing() + ImGui::GetStyle().ItemSpacing.y;
					float unifiedHeight = availableHeight - buttonAreaHeight;

					// 创建自适应两列布局
					ImGui::Columns(2, "MaterialDrawerColumns", false);

					// 用于追踪选中的材质名称（map 无法用索引，所以用名称）
					static string selectedMaterialName = "";

					// 第一列：材质列表
					{
						static vector<string> newMaterialNames;

						ImGui::BeginChild("MDCol1", ImVec2(0, unifiedHeight), false);
						ImGui::Text("材质列表");
						ImGui::Separator();

						// 搜索框
						static char materialFilter[256] = "";
						ImGui::SetNextItemWidth(-FLT_MIN);
						ImGui::InputTextWithHint("##materialFilter", "搜索材质...", materialFilter, sizeof(materialFilter));

						// 对列表进行排序和过滤
						static vector<string> sortedMaterialList;
						static string lastMaterialListHash = "";
						static string lastMaterialFilter = "";

						string currentHash = "";
						for (const auto& kv : globalmaterial::materialNames) {
							currentHash += kv.first;
						}

						if (currentHash != lastMaterialListHash || string(materialFilter) != lastMaterialFilter) {
							sortedMaterialList.clear();
							for (const auto& kv : globalmaterial::materialNames) {
								const string& s = kv.first;
								string lowerS = s;
								string lowerFilter = materialFilter;
								for (char& c : lowerS) c = (char)::tolower(c);
								for (char& c : lowerFilter) c = (char)::tolower(c);

								if (lowerFilter.empty() || lowerS.find(lowerFilter) != string::npos) {
									sortedMaterialList.push_back(s);
								}
							}
							std::sort(sortedMaterialList.begin(), sortedMaterialList.end(),
								[](const string& a, const string& b) {
									string lowerA = a, lowerB = b;
									for (char& c : lowerA) c = (char)::tolower(c);
									for (char& c : lowerB) c = (char)::tolower(c);
									return lowerA < lowerB;
								});
							lastMaterialListHash = currentHash;
							lastMaterialFilter = materialFilter;
						}

						ImGui::Spacing();
						ImGui::SetNextItemWidth(-FLT_MIN);
						if (ImGui::BeginListBox("##materialList", ImVec2(-FLT_MIN, -FLT_MIN))) {
							for (int i = 0; i < (int)sortedMaterialList.size(); ++i) {
								const string& itemName = sortedMaterialList[i];

								bool isNewMaterial = false;
								for (const auto& newName : newMaterialNames) {
									if (itemName == newName) {
										isNewMaterial = true;
										break;
									}
								}
								bool isSelected = (selectedMaterialName == itemName);

								if (isNewMaterial) {
									ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.4f, 0.6f, 0.9f, 0.7f));
									ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.4f, 0.6f, 0.9f, 0.9f));
									ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.3f, 0.5f, 0.8f, 1.0f));
								}

								char label[512];
								sprintf_s(label, sizeof(label), "%s##material_%d", itemName.c_str(), i);
								if (ImGui::Selectable(label, isSelected || isNewMaterial, isNewMaterial ? ImGuiSelectableFlags_Highlight : ImGuiSelectableFlags_None)) {
									selectedMaterialName = itemName;
									if (globalmaterial::get_material_by_name(itemName)) {

										globalmaterial::cachedProperties = globalmaterial::GetMaterialProperties();
										InfoMesseng::ColorPrint("SUCCESS", ("Loaded properties for material: " + sortedMaterialList[i]).c_str(), 0);
									}
									else {
										InfoMesseng::ColorPrint("ERROR", ("Failed to load properties for material: " + sortedMaterialList[i]).c_str(), 1);
									}
								}
								if (isNewMaterial && !isSelected) {
									ImVec2 itemMin = ImGui::GetItemRectMin();
									ImVec2 itemMax = ImGui::GetItemRectMax();
									ImGui::GetWindowDrawList()->AddRectFilled(itemMin, itemMax, IM_COL32(102, 153, 230, 180), 0.0f);
								}

								if (isNewMaterial) {
									ImGui::PopStyleColor(3);
								}

								if (isSelected) {
									ImGui::SetItemDefaultFocus();
								}
							}
							ImGui::EndListBox();
						}

						ImGui::EndChild();
						if (ImGui::Button("刷新材质", ImVec2(-FLT_MIN, 0))) {
							if (globalmaterial::init()) {
								InfoMesseng::ColorPrint("SUCCESS", "Material refurbish Success!", 0);
								newMaterialNames.clear();
								for (const auto& kv : globalmaterial::materialNames) {
									const string& name = kv.first;
									bool isNew = true;
									for (const auto& cachedKv : globalmaterial::cachedMaterialNames) {
										if (name == cachedKv.first) {
											isNew = false;
											break;
										}
									}
									if (isNew) {
										newMaterialNames.push_back(name);
									}
								}
							}
							else {
								InfoMesseng::ColorPrint("ERROR", "Material refurbish error!", 1);
							}
						}

						ImGui::NextColumn();
					}

					// 第二列：材质属性
					{
						ImGui::BeginChild("MDCol2", ImVec2(0, unifiedHeight), false);
						ImGui::Text("材质属性");
						ImGui::Separator();

						if (!selectedMaterialName.empty()) {
							ImGui::Text("当前材质: %s", selectedMaterialName.c_str());
						}
						else {
							ImGui::Text("当前材质: 未选中");
						}

						// 获取属性列表
						static int selectedPropertyIndex = -1;
						static string prevSelectedMaterial = "";  // 追踪上一个材质
						std::vector<MaterialPropertyInfo> properties = globalmaterial::cachedProperties;
						static bool vectorAsColor = false;

						// 用于存储每个属性的编辑值（使用属性名称作为key，避免索引错乱）
						static std::map<string, float> floatValues;
						static std::map<string, int> intValues;
						static std::map<string, ImVec4> vectorValues;
						static std::map<string, ImVec4> colorValues;

						// 材质切换时清理旧值
						if (selectedMaterialName != prevSelectedMaterial) {
							floatValues.clear();
							intValues.clear();
							vectorValues.clear();
							colorValues.clear();
							prevSelectedMaterial = selectedMaterialName;
						}

						// 初始化新属性值（使用属性名称作为key）
						for (int i = 0; i < (int)properties.size(); ++i) {
							string propKey = properties[i].name;
							if (floatValues.find(propKey) == floatValues.end()) {
								floatValues[propKey] = properties[i].floatValue;
							}
							if (intValues.find(propKey) == intValues.end()) {
								intValues[propKey] = properties[i].intValue;
							}
							if (vectorValues.find(propKey) == vectorValues.end()) {
								vectorValues[propKey] = ImVec4(properties[i].vectorValue.x, properties[i].vectorValue.y,
									properties[i].vectorValue.z, properties[i].vectorValue.w);
							}
							if (colorValues.find(propKey) == colorValues.end()) {
								colorValues[propKey] = ImVec4(properties[i].colorValue.r, properties[i].colorValue.g,
									properties[i].colorValue.b, properties[i].colorValue.a);
							}
						}

						if (!properties.empty()) {
							ImGui::Spacing();
							ImGui::Separator();
							ImGui::Spacing();

							ImGui::Text("属性列表 :");
							ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(4, 4));
							for (int i = 0; i < (int)properties.size(); ++i) {
								float baseY = ImGui::GetCursorPosY();
								const char* typeNames[] = { "Color", "Vector", "Float", "Range", "Texture", "Int" };
								const char* typeName = (properties[i].type >= 0 && properties[i].type <= 5) ? typeNames[properties[i].type] : "Unknown";
								float centerY = baseY + (ImGui::GetFrameHeight() - ImGui::GetTextLineHeight()) / 2;
								ImGui::SetCursorPosY(centerY);
								ImGui::TextColored(ImVec4(0.4f, 0.7f, 1.0f, 1.0f), "[%s]", typeName);
								ImGui::SameLine();
								ImGui::Text("%s:", properties[i].name.c_str());
								ImGui::SameLine();

								ImGui::PushID(i);
								ImGui::SetNextItemWidth(-FLT_MIN);

								string propKey = properties[i].name;
								switch (properties[i].type) {
								case ShaderType_Float:
								{
									if (ImGui::DragFloat("##val", &floatValues[propKey], 0.01f)) {
										if (globalmaterial::temp_material) {
											Il2CppString* propName = Engine::create_il2cpp_string(
												wstring(properties[i].name.begin(), properties[i].name.end()).c_str());
											globalmaterial::temp_material->SetFloat(propName, floatValues[propKey]);
										}
									}
									break;
								}
								case ShaderType_Int:
								{
									if (ImGui::DragInt("##val", &intValues[propKey], 1)) {
										if (globalmaterial::temp_material) {
											Il2CppString* propName = Engine::create_il2cpp_string(
												wstring(properties[i].name.begin(), properties[i].name.end()).c_str());
											globalmaterial::temp_material->SetInt(propName, intValues[propKey]);
										}
									}
									break;
								}

								case ShaderType_Vector:
								{
									if (vectorAsColor) {
										if (ImGui::ColorEdit4("##val", (float*)&vectorValues[propKey], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
											if (globalmaterial::temp_material) {
												Il2CppString* propName = Engine::create_il2cpp_string(
													wstring(properties[i].name.begin(), properties[i].name.end()).c_str());
												Color col(vectorValues[propKey].x, vectorValues[propKey].y, vectorValues[propKey].z, vectorValues[propKey].w);
												globalmaterial::temp_material->SetColor(propName, &col);
											}
										}
									}
									else {
										if (ImGui::InputFloat4("##val", (float*)&vectorValues[propKey], "%.3f")) {
											if (globalmaterial::temp_material) {
												Il2CppString* propName = Engine::create_il2cpp_string(
													wstring(properties[i].name.begin(), properties[i].name.end()).c_str());
												Vector4 vec(vectorValues[propKey].x, vectorValues[propKey].y, vectorValues[propKey].z, vectorValues[propKey].w);
												globalmaterial::temp_material->SetVector(propName, vec);
											}
										}
									}
									break;
								}
								case ShaderType_Color:
								{
									string propKey = properties[i].name;
									if (colorValues.find(propKey) == colorValues.end()) {
										colorValues[propKey] = ImVec4(properties[i].colorValue.r, properties[i].colorValue.g,
											properties[i].colorValue.b, properties[i].colorValue.a);
									}
									if (ImGui::ColorEdit4("##val", (float*)&colorValues[propKey], ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
										if (globalmaterial::temp_material) {
											Il2CppString* propName = Engine::create_il2cpp_string(
												wstring(properties[i].name.begin(), properties[i].name.end()).c_str());
											Color col(colorValues[propKey].x, colorValues[propKey].y, colorValues[propKey].z, colorValues[propKey].w);
											globalmaterial::temp_material->SetColor(propName, &col);
										}
									}
									break;
								}
								case ShaderType_Texture:
								{
									ImGui::Text("%s", properties[i].textureValue.c_str());
									break;
								}
								default:
									ImGui::Text("Unknown");
									break;
								}

								ImGui::PopID();
								ImGui::SetCursorPosY(baseY + ImGui::GetFrameHeightWithSpacing());

								if (i < (int)properties.size() - 1) {
									ImGui::Spacing();
								}
							}
							ImGui::PopStyleVar();
						}
						else {
							ImGui::Spacing();
							ImGui::Text("没有可用的属性");
							ImGui::Text("请先选择一个材质");
						}

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();
						ImGui::Text("配置:");
						ImGui::Checkbox("Vector转Color##vectorAsColor", &vectorAsColor);

						ImGui::EndChild();
					}



					ImGui::Columns(1);
					ImGui::End();
					ImGui::PopStyleVar(4);
					ImGui::PopFont();
				}

				// 主题设置窗口
				if (show_theme_settings) {
					if (focus_theme_settings) {
						ImGui::SetNextWindowFocus();
						focus_theme_settings = false;
					}
					// 设置窗口样式：微圆角，自适应大小
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);

					// 主题选择变量声明
					static int selected_theme = 0;
					const char* themes[] = { "默认", "明亮", "紫色", "自定义" };

					// 让ImGui完全根据内容自适应窗口大小
					// 不设置任何大小约束，让窗口能够自由调整

					ImGui::Begin("主题设置", &show_theme_settings, ImGuiWindowFlags_AlwaysAutoResize);

					// 自定义颜色结构（基于经典主题的默认值）
					struct CustomColors {
						ImVec4 WindowBg = ImVec4(0.94f, 0.94f, 0.94f, 1.00f);       // 经典主题窗口背景
						ImVec4 TitleBg = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);         // 经典主题标题背景
						ImVec4 TitleBgActive = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);  // 经典主题活动标题背景
						ImVec4 TitleBgCollapsed = ImVec4(0.85f, 0.85f, 0.85f, 0.51f); // 经典主题折叠标题背景
						ImVec4 Header = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);          // 经典主题标题栏
						ImVec4 HeaderHovered = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);   // 经典主题标题栏悬停
						ImVec4 HeaderActive = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);    // 经典主题标题栏激活
						ImVec4 Button = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);          // 经典主题按钮
						ImVec4 ButtonHovered = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);   // 经典主题按钮悬停
						ImVec4 ButtonActive = ImVec4(0.64f, 0.64f, 0.64f, 1.00f);    // 经典主题按钮激活
						ImVec4 FrameBg = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);         // 经典主题输入框背景
						ImVec4 FrameBgHovered = ImVec4(0.85f, 0.85f, 0.85f, 1.00f);  // 经典主题输入框悬停
						ImVec4 FrameBgActive = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);   // 经典主题输入框激活
						ImVec4 Text = ImVec4(0.00f, 0.00f, 0.00f, 1.00f);            // 经典主题文本
						ImVec4 TextDisabled = ImVec4(0.60f, 0.60f, 0.60f, 1.00f);    // 经典主题禁用文本
					};
					static CustomColors customColors;

					ImGui::Text("选择主题:");
					ImGui::Spacing();

					if (ImGui::Combo("##主题选择", &selected_theme, themes, IM_ARRAYSIZE(themes))) {
						// 应用选择的主题
						switch (selected_theme) {
						case 0: // 默认（深色）
							ImGui::StyleColorsDark();
							break;
						case 1: // 明亮
							ImGui::StyleColorsLight();
							break;
						case 2: // 紫色
							ImGui::StyleColorsClassic();
							break;
						case 3: // 自定义
						{
							// 应用自定义颜色
							ImGuiStyle& style = ImGui::GetStyle();
							style.Colors[ImGuiCol_WindowBg] = customColors.WindowBg;
							style.Colors[ImGuiCol_TitleBg] = customColors.TitleBg;
							style.Colors[ImGuiCol_TitleBgActive] = customColors.TitleBgActive;
							style.Colors[ImGuiCol_TitleBgCollapsed] = customColors.TitleBgCollapsed;
							style.Colors[ImGuiCol_Header] = customColors.Header;
							style.Colors[ImGuiCol_HeaderHovered] = customColors.HeaderHovered;
							style.Colors[ImGuiCol_HeaderActive] = customColors.HeaderActive;
							style.Colors[ImGuiCol_Button] = customColors.Button;
							style.Colors[ImGuiCol_ButtonHovered] = customColors.ButtonHovered;
							style.Colors[ImGuiCol_ButtonActive] = customColors.ButtonActive;
							style.Colors[ImGuiCol_FrameBg] = customColors.FrameBg;
							style.Colors[ImGuiCol_FrameBgHovered] = customColors.FrameBgHovered;
							style.Colors[ImGuiCol_FrameBgActive] = customColors.FrameBgActive;
							style.Colors[ImGuiCol_Text] = customColors.Text;
							style.Colors[ImGuiCol_TextDisabled] = customColors.TextDisabled;
							break;
						}
						}
					}

					ImGui::Spacing();
					ImGui::Separator();
					ImGui::Spacing();

					ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "当前主题: %s", themes[selected_theme]);

					// 自定义颜色选择器（仅在选择自定义主题时显示）
					if (selected_theme == 3) {
						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						ImGui::Text("自定义颜色设置:");
						ImGui::Spacing();

						// 实时应用自定义颜色的辅助函数
						auto applyCustomColors = [&]() {
							ImGuiStyle& style = ImGui::GetStyle();
							style.Colors[ImGuiCol_WindowBg] = customColors.WindowBg;
							style.Colors[ImGuiCol_TitleBg] = customColors.TitleBg;
							style.Colors[ImGuiCol_TitleBgActive] = customColors.TitleBgActive;
							style.Colors[ImGuiCol_TitleBgCollapsed] = customColors.TitleBgCollapsed;
							style.Colors[ImGuiCol_Header] = customColors.Header;
							style.Colors[ImGuiCol_HeaderHovered] = customColors.HeaderHovered;
							style.Colors[ImGuiCol_HeaderActive] = customColors.HeaderActive;
							style.Colors[ImGuiCol_Button] = customColors.Button;
							style.Colors[ImGuiCol_ButtonHovered] = customColors.ButtonHovered;
							style.Colors[ImGuiCol_ButtonActive] = customColors.ButtonActive;
							style.Colors[ImGuiCol_FrameBg] = customColors.FrameBg;
							style.Colors[ImGuiCol_FrameBgHovered] = customColors.FrameBgHovered;
							style.Colors[ImGuiCol_FrameBgActive] = customColors.FrameBgActive;
							style.Colors[ImGuiCol_Text] = customColors.Text;
							style.Colors[ImGuiCol_TextDisabled] = customColors.TextDisabled;
							};

						// 窗口背景色
						if (ImGui::ColorEdit3("窗口背景##WindowBg", (float*)&customColors.WindowBg, ImGuiColorEditFlags_NoAlpha)) {
							applyCustomColors();
						}
						if (ImGui::ColorEdit3("标题背景##TitleBg", (float*)&customColors.TitleBg, ImGuiColorEditFlags_NoAlpha)) {
							applyCustomColors();
						}
						if (ImGui::ColorEdit3("活动标题背景##TitleBgActive", (float*)&customColors.TitleBgActive, ImGuiColorEditFlags_NoAlpha)) {
							applyCustomColors();
						}
						if (ImGui::ColorEdit3("折叠标题背景##TitleBgCollapsed", (float*)&customColors.TitleBgCollapsed, ImGuiColorEditFlags_NoAlpha)) {
							applyCustomColors();
						}
						if (ImGui::ColorEdit3("标题栏##Header", (float*)&customColors.Header, ImGuiColorEditFlags_NoAlpha)) {
							applyCustomColors();
						}
						if (ImGui::ColorEdit3("标题栏悬停##HeaderHovered", (float*)&customColors.HeaderHovered, ImGuiColorEditFlags_NoAlpha)) {
							applyCustomColors();
						}
						if (ImGui::ColorEdit3("标题栏激活##HeaderActive", (float*)&customColors.HeaderActive, ImGuiColorEditFlags_NoAlpha)) {
							applyCustomColors();
						}

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						if (ImGui::ColorEdit3("按钮##Button", (float*)&customColors.Button, ImGuiColorEditFlags_NoAlpha)) {
							applyCustomColors();
						}
						if (ImGui::ColorEdit3("按钮悬停##ButtonHovered", (float*)&customColors.ButtonHovered, ImGuiColorEditFlags_NoAlpha)) {
							applyCustomColors();
						}
						if (ImGui::ColorEdit3("按钮激活##ButtonActive", (float*)&customColors.ButtonActive, ImGuiColorEditFlags_NoAlpha)) {
							applyCustomColors();
						}

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						if (ImGui::ColorEdit3("输入框背景##FrameBg", (float*)&customColors.FrameBg, ImGuiColorEditFlags_NoAlpha)) {
							applyCustomColors();
						}
						if (ImGui::ColorEdit3("输入框悬停##FrameBgHovered", (float*)&customColors.FrameBgHovered, ImGuiColorEditFlags_NoAlpha)) {
							applyCustomColors();
						}
						if (ImGui::ColorEdit3("输入框激活##FrameBgActive", (float*)&customColors.FrameBgActive, ImGuiColorEditFlags_NoAlpha)) {
							applyCustomColors();
						}

						ImGui::Spacing();
						ImGui::Separator();
						ImGui::Spacing();

						if (ImGui::ColorEdit3("文本颜色##Text", (float*)&customColors.Text)) {
							applyCustomColors();
						}
						if (ImGui::ColorEdit3("禁用文本##TextDisabled", (float*)&customColors.TextDisabled)) {
							applyCustomColors();
						}
					}

					ImGui::End();
					ImGui::PopStyleVar();
				}

				// 配置窗口
				if (show_config_window) {
					if (focus_config_window) {
						ImGui::SetNextWindowFocus();
						focus_config_window = false;
					}
					// 设置窗口样式：微圆角，自适应大小
					ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
					ImGui::Begin("配置", &show_config_window, ImGuiWindowFlags_AlwaysAutoResize);

					ImGui::Text("语言设置");
					const char* languages[] = { "中文", "English" };
					ImGui::SetNextItemWidth(200.0f);
					if (ImGui::BeginCombo("语言##language", languages[Config::language_index])) {
						for (int i = 0; i < IM_ARRAYSIZE(languages); i++) {
							bool isSelected = (Config::language_index == i);
							if (ImGui::Selectable(languages[i], isSelected)) {
								Config::language_index = i;
							}
							if (isSelected) {
								ImGui::SetItemDefaultFocus();
							}
						}
						ImGui::EndCombo();
					}

					ImGui::Separator();
					ImGui::Text("快捷键设置");

					// 显示当前快捷键
					const char* keyName = "";
					switch (Config::key_see_menu) {
					case ImGuiKey_Insert: keyName = "Insert"; break;
					case ImGuiKey_Delete: keyName = "Delete"; break;
					case ImGuiKey_Home: keyName = "Home"; break;
					case ImGuiKey_End: keyName = "End"; break;
					case ImGuiKey_PageUp: keyName = "PageUp"; break;
					case ImGuiKey_PageDown: keyName = "PageDown"; break;
					case ImGuiKey_F1: keyName = "F1"; break;
					case ImGuiKey_F2: keyName = "F2"; break;
					case ImGuiKey_F3: keyName = "F3"; break;
					case ImGuiKey_F4: keyName = "F4"; break;
					case ImGuiKey_F5: keyName = "F5"; break;
					case ImGuiKey_F6: keyName = "F6"; break;
					case ImGuiKey_F7: keyName = "F7"; break;
					case ImGuiKey_F8: keyName = "F8"; break;
					case ImGuiKey_F9: keyName = "F9"; break;
					case ImGuiKey_F10: keyName = "F10"; break;
					case ImGuiKey_F11: keyName = "F11"; break;
					case ImGuiKey_F12: keyName = "F12"; break;
					default: keyName = "未知按键"; break;
					}

					ImGui::Text("菜单显示/隐藏快捷键: %s", keyName);

					static bool waitingForKey = false;
					if (waitingForKey) {
						ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "请按下想要设置的按键...");
					}

					if (ImGui::Button(waitingForKey ? "取消设置" : "设置快捷键")) {
						waitingForKey = !waitingForKey;
					}

					// 处理按键输入
					if (waitingForKey) {
						for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key++) {
							if (ImGui::IsKeyPressed((ImGuiKey)key)) {
								Config::key_see_menu = (ImGuiKey)key;
								waitingForKey = false;
								break;
							}
						}
					}

					ImGui::Separator();
					if (ImGui::Button("保存配置")) {
						Config::SaveConfig("Cofig1.ini");
						ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "配置已保存!");
					}

					// TODO: 添加更多配置相关功能
					ImGui::End();
					ImGui::PopStyleVar();
				}
			}
		}
		ImGui::Render();
		pDeviceContext->OMSetRenderTargets(1, &pRenderTargetView, nullptr);
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

		// 在渲染后再次确保鼠标光标显示
		if (menu_visible) {
			ClipCursor(nullptr);
			// 隐藏系统鼠标指针
			for (int i = 0; i < 10; i++) {
				ShowCursor(FALSE);
			}

			// 解除鼠标剪切区域（如果有的话）
			ClipCursor(NULL);
		}
		return oPresent(pSwapChain, SyncInterval, Flags);
	}
	HRESULT WINAPI hkResize(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags) {
		if (bDx11Init) {
			if (pRenderTargetView) {
				pRenderTargetView->Release();
				pRenderTargetView = nullptr;
			}
			if (pDeviceContext) {
				pDeviceContext->Release();
				pDeviceContext = nullptr;
			}
			if (pDevice) {
				pDevice->Release();
				pDevice = nullptr;
			}
			ImGui_ImplDX11_Shutdown();
			bDx11Init = false;
		}
		return Dx11Hook.GetOriginal<decltype(&hkResize)>(13)
			(pSwapChain, BufferCount, Width, Height, NewFormat, SwapChainFlags);
	}
	LRESULT WINAPI hkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
	{
		switch (msg)
		{
		case WM_ENTERSIZEMOVE:
			inSizeMove = true;
			break;
		case WM_EXITSIZEMOVE:
			inSizeMove = false;
			isDraggingWindow = false;
			break;
		case WM_NCLBUTTONDOWN:
			// 检查是否点击了标题栏 (HTCAPTION)
			if (wParam == HTCAPTION)
			{
				isDraggingWindow = true;
			}
			break;
		case WM_NCLBUTTONUP:
			if (isDraggingWindow)
			{
				isDraggingWindow = false;
			}
			break;
		}
		if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
			return true;

		// 当菜单显示时，阻止游戏接收键盘和鼠标输入
		if (menu_visible)
		{
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}

		ImGuiIO& io = ImGui::GetIO();
		if (io.WantCaptureMouse || io.WantCaptureKeyboard)
		{
			return DefWindowProc(hWnd, msg, wParam, lParam);
		}
		return CallWindowProcA(oWndProc, hWnd, msg, wParam, lParam);
	}
}
