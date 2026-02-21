#include "DumpSdk.h"


void DumpSdk::il2cpp_dump_init(HMODULE Module) {
	hModule = Module;
}
bool DumpSdk::il2cpp_Dump2File(string file)
{
	try {
		ofstream io(file + "\\dump.cs");
		ofstream io2(file + "\\il2cpp.hpp");
		ofstream io3(file + "\\il2cpp-func.hpp");

		if (!io) return false;
		stringstream str;
		stringstream str2;
		stringstream str3;

		vector<Il2CppAssembly*> Assemblys;
		for (size_t i = 0, max = il2cpp_EnummAssembly(Assemblys); i < max; i++)
		{
			auto image = il2cpp_GetImage(Assemblys[i]);
			str << "// Image " << i << ": " << il2cpp_GetImageName(image) << " - " << il2cpp_GetClassCount(image) << endl;
		}
		str << endl << endl << endl;
		for (size_t i = 0, max = Assemblys.size(); i < max; i++)
		{
			auto image = il2cpp_GetImage(Assemblys[i]);
			vector<Il2CppClass*> classes;
			for (size_t i_c = 0, max_c = il2cpp_EnumClasses(image, classes); i_c < max_c; i_c++)
			{
				str << "// Assembly: " << il2cpp_class_get_assemblyname(classes[i_c]) << endl;
				str << "// Namespace: " << il2cpp_GetClassNamespace(classes[i_c]) << "\npublic class " << il2cpp_GetClassName(classes[i_c]) << "\n{\n\t// Fields" << endl;
				str2 << "struct " << il2cpp_GetClassName(classes[i_c]) << "\n" << "{" << endl;

				vector<FieldInfo*> Field;
				for (size_t i_f = 0, max_f = il2cpp_EnumFields(classes[i_c], Field); i_f < max_f; i_f++)
				{
					auto Typename = il2cpp_GetTypeName(il2cpp_GetFieldType(Field[i_f]));
					string Type = Typename;
					if (Typename.find_last_of(".") != string::npos)
					{
						size_t MaxIndex = Typename.find("<");
						if (MaxIndex == string::npos)
						{
							Type = Typename.substr(Typename.find_last_of(".") + 1);
						}
						else
						{
							size_t LastIndex = Typename.find_last_of(".");
							while (LastIndex != string::npos
								&& LastIndex > MaxIndex)
							{
								LastIndex = Typename.find_last_of(".", LastIndex - 1);
							}

							if (LastIndex != string::npos)
								Type = Typename.substr(LastIndex + 1);
						}
					}
					str << "\tpublic " << Type << " " << il2cpp_GetFieldName(Field[i_f]) << "; // 0x" << hex << il2cpp_GetFieldOffset(Field[i_f]) << dec << endl;
					str2 << "\t" << Type << " " << il2cpp_GetFieldName(Field[i_f]) << "; // 0x" << hex << il2cpp_GetFieldOffset(Field[i_f]) << dec << endl;
				}
				str << endl << endl << endl << "\t// Methods" << endl;
				str2 << "\n};" << endl;

				vector<MethodInfo*> Methods;
				for (size_t i_m = 0, max_m = il2cpp_EnumMethods(classes[i_c], Methods); i_m < max_m; i_m++)
				{
					str << "\t// RVA: 0x" << hex << (il2cpp_GetMethodAddress(Methods[i_m]) - (DWORD_PTR)hModule) << dec << endl;

					auto retTypename = il2cpp_GetTypeName(il2cpp_GetMethodRetType(Methods[i_m]));
					string retType = retTypename;
					if (retTypename.find_last_of(".") != string::npos)
					{
						retType = retTypename.substr(retTypename.find_last_of(".") + 1, retTypename.length());
					}
					str << "\tpublic " << retType << " " << il2cpp_GetMethodName(Methods[i_m]) << "(";
					str3 << "DO_API(" << retType << ", " << il2cpp_GetClassName(classes[i_c]) << "_" << il2cpp_GetMethodName(Methods[i_m]) << ", (";

					for (size_t i_p = 0, max_p = il2cpp_GetMethodParamCount(Methods[i_m]); i_p < max_p; i_p++)
					{
						auto Typename = il2cpp_GetTypeName(il2cpp_GetMethodParam(Methods[i_m], i_p));
						string Type = Typename;
						if (Typename.find_last_of(".") != string::npos)
						{
							Type = Typename.substr(Typename.find_last_of(".") + 1, Typename.length());
						}

						str << Type << " " << il2cpp_GetMethodParamName(Methods[i_m], i_p) << (1 == (max_p - i_p) ? "" : ", ");
						str3 << Type << " " << il2cpp_GetMethodParamName(Methods[i_m], i_p) << (1 == (max_p - i_p) ? "" : ", ");

					}
					str << ");\n\n";
					str3 << "));\n";
				}
				str << "}" << endl << endl << endl;
			}
		}
		io << str.str();
		io2 << str2.str();
		io3 << str3.str();
		io.close();
		io2.close();
		io3.close();
		return true;
	}catch (exception& e){
		InfoMesseng::ColorPrint("ERROR", ("Error: " +  string(e.what())).c_str(), 2);
		return false;
	}
}