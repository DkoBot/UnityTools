#pragma once
#include "../Menu/globalmanagement.h"
#include "../Menu/globalshader.h"
#include "../Menu/globalmaterial.h"
#include "../Misc/ColorInfo/InfoMesseng.h"
#include "base.h"
#include "utils.h"
#include "engine.h"
#include "Conini.h"
#include "../Src/font_fa-solid-900.c"
#include <windows.h>
#include <shlobj.h>
#include <atomic>
#include <chrono>
#include <vector>
#include <cctype>
#include <cstdint>
#include <ctime>
#include <iomanip>
#include <sstream>
#include <thread>
namespace Hooks
{
	void initialize(kiero::RenderType::Enum renderType);
	HRESULT WINAPI hkPresent(IDXGISwapChain* pSwapChain, UINT SyncInterval, UINT Flags);
	HRESULT WINAPI hkResize(IDXGISwapChain* pSwapChain, UINT BufferCount, UINT Width, UINT Height, DXGI_FORMAT NewFormat, UINT SwapChainFlags);
	LRESULT WINAPI hkWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
}

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);