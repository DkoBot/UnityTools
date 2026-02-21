// assembly
DO_API(const Il2CppImage*, il2cpp_assembly_get_image, (const Il2CppAssembly* assembly));

// image
DO_API(const char*, il2cpp_image_get_name, (const Il2CppImage* image));
DO_API(size_t, il2cpp_image_get_class_count, (const Il2CppImage* image));
DO_API(const Il2CppClass*, il2cpp_image_get_class, (const Il2CppImage* image, size_t index));

DO_API(Il2CppString*, il2cpp_string_new_utf16, (const wchar_t* text, int32_t len));
// class
DO_API(Il2CppClass*, il2cpp_class_from_name, (const Il2CppImage* image, const char* namespaze, const char* name));
DO_API(const MethodInfo*, il2cpp_class_get_methods, (Il2CppClass* klass, void** iter));
DO_API(const MethodInfo*, il2cpp_class_get_method_from_name, (Il2CppClass* klass, const char* name, int argsCount));
DO_API(FieldInfo*, il2cpp_class_get_field_from_name, (Il2CppClass* klass, const char* name));
DO_API(const char*, il2cpp_class_get_name, (const Il2CppClass* klass));
DO_API(const char*, il2cpp_class_get_namespace, (const Il2CppClass* klass));
DO_API(const Il2CppClass*, il2cpp_class_get_parent, (const Il2CppClass* klass));
DO_API(const Il2CppClass*, il2cpp_class_get_declaring_type, (const Il2CppClass* klass));
DO_API(FieldInfo*, il2cpp_class_get_fields, (Il2CppClass* klass, void** iter));
DO_API(int32_t, il2cpp_field_get_offset, (FieldInfo* field));
DO_API(const char*, il2cpp_field_get_name, (FieldInfo* field));

// field
DO_API(void, il2cpp_field_static_get_value, (FieldInfo* field, void* value));
DO_API(const Il2CppType*, il2cpp_field_get_type, (FieldInfo* field));

// domain
DO_API(Il2CppDomain*, il2cpp_domain_get, ());
DO_API(const Il2CppAssembly*, il2cpp_domain_assembly_open, (Il2CppDomain* domain, const char* name));
DO_API(const Il2CppAssembly**, il2cpp_domain_get_assemblies, (const Il2CppDomain* domain, size_t* size));

// method
DO_API(const Il2CppType*, il2cpp_method_get_return_type, (const MethodInfo* method));
DO_API(const char*, il2cpp_method_get_name, (const MethodInfo* method));
DO_API(uint32_t, il2cpp_method_get_param_count, (const MethodInfo* method));
DO_API(const Il2CppType*, il2cpp_method_get_param, (const MethodInfo* method, uint32_t index));
DO_API(const char*, il2cpp_method_get_param_name, (const MethodInfo* method, uint32_t index));
DO_API(uint32_t, il2cpp_method_get_flags, (const MethodInfo* method, uint32_t* iflags));
DO_API(Il2CppObject*, il2cpp_runtime_invoke, (const MethodInfo* method, void* obj, void** params, Il2CppObject** exc));

// thread
DO_API(Il2CppThread*, il2cpp_thread_attach, (Il2CppDomain* domain));

// type
DO_API(char*, il2cpp_type_get_name, (const Il2CppType* type));
DO_API(const char*, il2cpp_class_get_assemblyname, (const Il2CppClass* klass));