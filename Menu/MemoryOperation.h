#pragma once
#include "base.h"
#include "engine.h"
#include "../Misc/ColorInfo/InfoMesseng.h"
#include <vector>
class MemoryOperation
{
public:
	// 读取
	float Read_flaot(void* addr);
	int Read_int(void* addr);
	bool Read_bool(void* addr);
	double Read_double(void* addr);
	uint8_t Read_byte(void* addr);
	string Read_il2cppString(void* il2cppStringAddr);
	// 写入
	bool write_float(void* addr, float value);
	bool write_double(void* addr, double value);
	bool write_int(void* addr, int value);
	bool write_bool(void* addr, bool value);
	bool write_byte(void* addr, uint8_t value);
	bool write_il2cppString(void* addr, string value);

	// 特征码扫描MetadataHeader操作
	static int ScanFeature(const char* moduleName, const unsigned char* pattern, size_t patternLen, vector<uintptr_t>& results);
	static uintptr_t ReadRipRelativeValue(uintptr_t movAddr);
};

