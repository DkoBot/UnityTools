#include "class.h"
#include "engine.h"


Il2CppString* create_il2cpp_string(const wchar_t* src) {
	return il2cpp_string_new_utf16(src, static_cast<int32_t>(wcslen(src)));
}

string Engine::il2cppStringToStdString(Il2CppString* il2cppStr)
{
	if (!il2cppStr || il2cppStr->length == 0)
		return {};
	int u8len = WideCharToMultiByte(CP_UTF8, 0,
		il2cppStr->chars, il2cppStr->length,
		nullptr, 0, nullptr, nullptr);
	if (u8len <= 0) return {};
	string u8str(u8len, '\0');
	WideCharToMultiByte(CP_UTF8, 0,
		il2cppStr->chars, il2cppStr->length,
		&u8str[0], u8len, nullptr, nullptr);
	return u8str;
}
Transform* GameObject::GetTransform() {
	return ::GetTransform(this);
};
Vector3 Transform::GetPosition() {
	return ::GetPosition(this);
}
int Transform::GetChildCount() {
	return ::GetChildCount(this);
}
Transform* Transform::GetChild(int index) {
	return ::GetChild(this, index);
}
Vector3 Transform::GetForward() {
	return ::GetForward(this);
}
bool Camera::WorldToScreen(Vector3 position, Vector2& Point)
{
	if (!this) {
		return false;
	}
	uint8_t* temp01 = *(uint8_t**)(((uint8_t*)this) + 0x10);
	D3DXMATRIX* Matrix = (D3DXMATRIX*)(temp01 + 0x100);
	if (Matrix == nullptr) {
		Matrix = (D3DXMATRIX*)(temp01 + 0xDC);
	}
	if (Matrix) {
		float Z = Matrix->_14 * position.x + Matrix->_24 * position.y + Matrix->_34 * position.z + Matrix->_44;

		if (Z > 0)
		{
			float X = Matrix->_11 * position.x + Matrix->_21 * position.y + Matrix->_31 * position.z + Matrix->_41;
			float Y = Matrix->_12 * position.x + Matrix->_22 * position.y + Matrix->_32 * position.z + Matrix->_42;

			float NdcX = X / Z;
			float NdcY = Y / Z;

			Point.x = (NdcX + 1.f) / 2.f * ImGui::GetIO().DisplaySize.x;
			Point.y = (1.f - (NdcY + 1.f) / 2.f) * ImGui::GetIO().DisplaySize.y;

			return true;
		}
	}
	return false;
}
Component* GameObject::GetComponent(string Type) {
	//cout << "\n" << Type.c_str() << ":" << endl;
	if (!this) return NULL;
	uint8_t* Base = *(uint8_t**)((uint8_t*)this + 0x10);
	if (!Base) return NULL;
	uint8_t* Components = *(uint8_t**)(Base + 0x30);
	if (!Components) return NULL;
	uint32_t Count = *(uint32_t*)(Base + 0x40);
	for (size_t i = 0; i < Count; i++) {
		uint8_t* temp = *(uint8_t**)(Components + 0x8 + 0x10 * i);
		if (!temp) continue;
		Component* pComponents = *(Component**)(temp + 0x28);
		if (pComponents && pComponents->pClass && pComponents->pClass->ClassName) {
			/*cout << pComponents->pClass->ClassName << endl;*/
			if (Type.find(pComponents->pClass->ClassName) != string::npos) {
				return pComponents;
			}
		}
	}
	return NULL;
}
string Component::get_tag() {
	Il2CppString* il2Cppname = ::Gettag(this);
	return Engine::il2cppStringToStdString(il2Cppname);
}
Component* GameObject::GetComponentInChildren(string Type) {
	// 先在当前GameObject上查找组件
	Component* result = GetComponent(Type);
	if (result) return result;
	
	// 获取Transform以遍历子对象
	Transform* transform = GetTransform();
	if (!transform) return NULL;
	
	// 遍历所有子对象
	int childCount = transform->GetChildCount();
	for (int i = 0; i < childCount; i++) {
		Transform* childTransform = transform->GetChild(i);
		if (!childTransform) continue;
		
		// Transform继承自Component，可以使用GetGameObject
		GameObject* childGameObject = ::GetGameObject(childTransform);
		if (!childGameObject) continue;
		
		// 递归查找子对象
		Component* childResult = childGameObject->GetComponentInChildren(Type);
		if (childResult) return childResult;
	}
	
	return NULL;
}
void GameObject::SetActive(bool offoron) {
	::SetActive(this, offoron);
}

void Shader::SetGlobalColor(string name, Color value) {
	Il2CppString* nameStr = create_il2cpp_string(wstring(name.begin(), name.end()).c_str());
	::SetGlobalColor(nameStr, value);
}
void Shader::SetGlobalInt(string name, INT32 value) {
	Il2CppString* nameStr = create_il2cpp_string(wstring(name.begin(), name.end()).c_str());
	::SetGlobalInt(nameStr, value);
}
void Shader::SetGlobalFloat(string name, float value) {
	Il2CppString* nameStr = create_il2cpp_string(wstring(name.begin(), name.end()).c_str());
	::SetGlobalFloat(nameStr, value);
}
void Shader::SetGlobalVector(string name, Vector4 value) {
	Il2CppString* nameStr = create_il2cpp_string(wstring(name.begin(), name.end()).c_str());
	::SetGlobalVector(nameStr, value);
}
void Shader::SetGlobalMatrix(string name, Matrix4x4 value) {
	Il2CppString* nameStr = create_il2cpp_string(wstring(name.begin(), name.end()).c_str());
	::SetGlobalMatrix(nameStr, value);
}
void Shader::set_maximumLOD(INT32 value) {
	::SetMaximumLOD(this, value);
}
int Shader::get_passCount() {
	return ::GetPassCount(this);
}