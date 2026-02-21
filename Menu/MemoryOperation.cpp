#include "MemoryOperation.h"

// --- Read ---

float MemoryOperation::Read_flaot(void* addr) {
	if (!addr) return 0.0f;
	return *(float*)addr;
}

int MemoryOperation::Read_int(void* addr) {
	if (!addr) return 0;
	return *(int*)addr;
}

bool MemoryOperation::Read_bool(void* addr) {
	if (!addr) return false;
	return *(bool*)addr;
}

double MemoryOperation::Read_double(void* addr) {
	if (!addr) return 0.0;
	return *(double*)addr;
}

uint8_t MemoryOperation::Read_byte(void* addr) {
	if (!addr) return 0;
	return *(uint8_t*)addr;
}

string MemoryOperation::Read_il2cppString(void* il2cppStringAddr) {
	if (!il2cppStringAddr) return {};
	return Engine::il2cppStringToStdString((Il2CppString*)il2cppStringAddr);
}

// --- Write ---

bool MemoryOperation::write_float(void* addr, float value) {
	if (!addr) return false;
	*(float*)addr = value;
	return true;
}

bool MemoryOperation::write_double(void* addr, double value) {
	if (!addr) return false;
	*(double*)addr = value;
	return true;
}

bool MemoryOperation::write_int(void* addr, int value) {
	if (!addr) return false;
	*(int*)addr = value;
	return true;
}

bool MemoryOperation::write_bool(void* addr, bool value) {
	if (!addr) return false;
	*(bool*)addr = value;
	return true;
}

bool MemoryOperation::write_byte(void* addr, uint8_t value) {
	if (!addr) return false;
	*(uint8_t*)addr = value;
	return true;
}

bool MemoryOperation::write_il2cppString(void* addr, string value) {
	if (!addr) return false;
	wstring wval(value.begin(), value.end());
	Il2CppString* newStr = Engine::create_il2cpp_string(wval.c_str());
	*(Il2CppString**)addr = newStr;
	return true;
}


int MemoryOperation::ScanFeature(const char* moduleName, const unsigned char* pattern, size_t patternLen,vector<uintptr_t>& results) {
	HMODULE hMod = GetModuleHandleA(moduleName);
	if (!hMod) return 0;

	// 获取模块大小（简化版，实际应使用 Module32First 或 GetModuleInformation）
	PIMAGE_DOS_HEADER dos = (PIMAGE_DOS_HEADER)hMod;
	PIMAGE_NT_HEADERS nt = (PIMAGE_NT_HEADERS)((BYTE*)hMod + dos->e_lfanew);
	size_t modSize = nt->OptionalHeader.SizeOfImage;

	results.clear();
	uintptr_t base = (uintptr_t)hMod;

	for (uintptr_t i = 0; i < modSize - patternLen; i++) {
		if (memcmp((void*)(base + i), pattern, patternLen) == 0) {
			results.push_back(base + i);
		}
	}

	return (int)results.size();
}

uintptr_t MemoryOperation::ReadRipRelativeValue(uintptr_t movAddr) {
	// 验证指令
	if (*(unsigned char*)movAddr != 0x48 ||
		*(unsigned char*)(movAddr + 1) != 0x8B ||
		*(unsigned char*)(movAddr + 2) != 0x05) {
		return 0;
	}

	// 读取偏移
	int32_t offset = *(int32_t*)(movAddr + 3);
	uintptr_t nextInsn = movAddr + 7;
	uintptr_t targetAddr = nextInsn + offset;

	// 读取目标地址中的 qword 值
	return *(uintptr_t*)targetAddr;
}