#include "utils.h"

namespace Utils
{
	void VTableHook::Initizalize(void* pTarget)
	{
		this->pTarget = pTarget;
		this->oVTable = *(void***)pTarget;
		uint32_t Size = CalcVTableSize();
		this->VTable = new void* [Size];
		memcpy(VTable, oVTable, Size * sizeof(void*));
		*(void***)pTarget = VTable;
	}
	void VTableHook::Bind(uint32_t Index, void* Function) {
		VTable[Index] = Function;
	}
	void VTableHook::UnBind(uint32_t Index) {
		VTable[Index] = oVTable[Index];
	}
	void VTableHook::UnAllBind() {
		*(void***)pTarget = oVTable;
	}

	uint32_t VTableHook::CalcVTableSize() {
		int i = 0;
		for (uint32_t i = 0; true; i++) {
			if (oVTable[i] == NULL)
				return i;
		}
	}
}