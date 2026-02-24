#pragma once
#include "base.h"

namespace Utils
{

	class VTableHook {
	public:
		// 初始化虚表Hook,替换虚表
		void Initizalize(void* pTarget);
		// Hook虚表上指定Idx的函数
		void Bind(uint32_t Index, void* Function);
		void UnBind(uint32_t Index);
		void UnAllBind();
		template <typename T>
		T GetOriginal(uint32_t Index) {
			return (T)oVTable[Index];
		}
		// 析构函数，释放VTable数组
		~VTableHook() {
			if (VTable) {
				delete[] VTable;
				VTable = NULL;
			}
		}
	private:
		uint32_t CalcVTableSize();
		void* pTarget = NULL;
		void** VTable = NULL; // 执行修改虚表
		void** oVTable = NULL; // 执行原始虚表
	};
}
