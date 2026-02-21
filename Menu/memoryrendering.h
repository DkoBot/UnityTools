#pragma once
#include "engine.h"
#include "base.h"
#include <vector>
#include <string>

class memoryrendering {
	public:
		static GameObject* Comp2GameObject(void* comp)
        {
            if (!comp) return nullptr;
            uintptr_t* p = (uintptr_t*)comp;
            // 64 位常见两处偏移
            if (p[3]) return (GameObject*)p[3];   // +0x18
            if (p[4]) return (GameObject*)p[4];   // +0x20
            return nullptr;
        }
};