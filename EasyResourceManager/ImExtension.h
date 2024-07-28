#pragma once
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include "includes.h"






namespace ImExtension
{
	void LoadFileTypeIcons(ID3D11Device* device, ID3D11DeviceContext* context);
	bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data);
	bool InputTextMultiline(const char* label, std::string* str, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data);
	bool InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data);
	void ShowFileSelector(bool* p_open, std::string& selected_file, ID3D11Device* device, ID3D11DeviceContext* context);
	void ShowResourceList(ID3D11Device* device, ID3D11DeviceContext* context);
}