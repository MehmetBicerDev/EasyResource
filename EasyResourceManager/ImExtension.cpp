#include "ImExtension.h"
#include "ui.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <shlobj.h>  // For SHGetKnownFolderPath
#include <stack>
#include <wrl/client.h>
#include <easyresource.h>
#include <unordered_set>

#include <filesystem>
#include <string>

#include <set>


namespace ImExtension
{
    struct InputTextCallback_UserData
    {

        std::string* Str;
        ImGuiInputTextCallback  ChainCallback;
        void* ChainCallbackUserData;
    };

    static int InputTextCallback(ImGuiInputTextCallbackData* data)
    {
        InputTextCallback_UserData* user_data = (InputTextCallback_UserData*)data->UserData;
        if (data->EventFlag == ImGuiInputTextFlags_CallbackResize)
        {
            // Resize string callback
            // If for some reason we refuse the new length (BufTextLen) and/or capacity (BufSize) we need to set them back to what we want.
            std::string* str = user_data->Str;
            IM_ASSERT(data->Buf == str->c_str());
            str->resize(data->BufTextLen);
            data->Buf = (char*)str->c_str();
        }
        else if (user_data->ChainCallback)
        {
            // Forward to user callback, if any
            data->UserData = user_data->ChainCallbackUserData;
            return user_data->ChainCallback(data);
        }
        return 0;
    }

    bool InputText(const char* label, std::string* str, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = 0, void* user_data = 0)
    {
        IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
        flags |= ImGuiInputTextFlags_CallbackResize;

        InputTextCallback_UserData cb_user_data;
        cb_user_data.Str = str;
        cb_user_data.ChainCallback = callback;
        cb_user_data.ChainCallbackUserData = user_data;
        return ImGui::InputText(label, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
    }

    bool InputTextMultiline(const char* label, std::string* str, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
    {
        IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
        flags |= ImGuiInputTextFlags_CallbackResize;

        InputTextCallback_UserData cb_user_data;
        cb_user_data.Str = str;
        cb_user_data.ChainCallback = callback;
        cb_user_data.ChainCallbackUserData = user_data;
        return ImGui::InputTextMultiline(label, (char*)str->c_str(), str->capacity() + 1, size, flags, InputTextCallback, &cb_user_data);
    }

    bool InputTextWithHint(const char* label, const char* hint, std::string* str, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data)
    {
        IM_ASSERT((flags & ImGuiInputTextFlags_CallbackResize) == 0);
        flags |= ImGuiInputTextFlags_CallbackResize;

        InputTextCallback_UserData cb_user_data;
        cb_user_data.Str = str;
        cb_user_data.ChainCallback = callback;
        cb_user_data.ChainCallbackUserData = user_data;
        return ImGui::InputTextWithHint(label, hint, (char*)str->c_str(), str->capacity() + 1, flags, InputTextCallback, &cb_user_data);
    }

    bool should_run = true;

    // FileInfo struct as provided
    struct FileInfo {
        std::string name;
        std::string type;
        std::uintmax_t size;
        std::filesystem::file_time_type last_write_time;
        bool is_directory;

        bool operator<(const FileInfo& other) const {
            return name < other.name;
        }
    };

    // Convert file_time_type to time_t
    std::time_t to_time_t(const std::filesystem::file_time_type& ftime) {
        auto sctp = std::chrono::time_point_cast<std::chrono::system_clock::duration>(
            ftime - std::filesystem::file_time_type::clock::now() + std::chrono::system_clock::now()
        );
        return std::chrono::system_clock::to_time_t(sctp);
    }

    // Function to get known folder path
    std::string GetKnownFolderPath(REFKNOWNFOLDERID folderId) {
        PWSTR path = nullptr;
        if (SUCCEEDED(SHGetKnownFolderPath(folderId, 0, NULL, &path))) {
            std::wstring ws(path);
            std::string result(ws.begin(), ws.end());
            CoTaskMemFree(path);
            return result;
        }
        return "";
    }

    // DriveInfo struct
    struct DriveInfo {
        std::string drive;
        std::string label;
    };

    // Get available drives
    std::vector<DriveInfo> GetDrives() {
        std::vector<DriveInfo> drives;
        char driveStrings[256];
        DWORD len = GetLogicalDriveStringsA(sizeof(driveStrings), driveStrings);
        if (len > 0) {
            char* drive = driveStrings;
            while (*drive) {
                DriveInfo info;
                info.drive = drive;

                char volumeName[MAX_PATH];
                if (GetVolumeInformationA(drive, volumeName, sizeof(volumeName), NULL, NULL, NULL, NULL, 0)) {
                    info.label = volumeName;
                }
                else {
                    info.label = "Local Disk";
                }
                drives.push_back(info);
                drive += strlen(drive) + 1;
            }
        }
        return drives;
    }

    // IconTexture struct
    struct IconTexture {
        Microsoft::WRL::ComPtr<ID3D11Texture2D> texture;
        Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
    };


    // Create a D3D11 texture from an HICON
    bool CreateTextureFromIcon(HICON hIcon, IconTexture& out_texture, ID3D11Device* device, ID3D11DeviceContext* context) {
        ICONINFO iconInfo;
        BITMAP bm;
        GetIconInfo(hIcon, &iconInfo);
        GetObject(iconInfo.hbmColor, sizeof(bm), &bm);

        int width = bm.bmWidth;
        int height = bm.bmHeight;
        HDC hdc = GetDC(nullptr);
        HDC hdcMem = CreateCompatibleDC(hdc);
        HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, iconInfo.hbmColor);

        unsigned char* pixels = new unsigned char[width * height * 4];
        BITMAPINFO bmi = { 0 };
        bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        bmi.bmiHeader.biWidth = width;
        bmi.bmiHeader.biHeight = -height; // top-down
        bmi.bmiHeader.biPlanes = 1;
        bmi.bmiHeader.biBitCount = 32;
        bmi.bmiHeader.biCompression = BI_RGB;
        GetDIBits(hdcMem, iconInfo.hbmColor, 0, height, pixels, &bmi, DIB_RGB_COLORS);

        D3D11_TEXTURE2D_DESC desc;
        ZeroMemory(&desc, sizeof(desc));
        desc.Width = width;
        desc.Height = height;
        desc.MipLevels = 1;
        desc.ArraySize = 1;
        desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
        desc.SampleDesc.Count = 1;
        desc.Usage = D3D11_USAGE_DYNAMIC;
        desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
        desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

        D3D11_SUBRESOURCE_DATA subResource;
        subResource.pSysMem = pixels;
        subResource.SysMemPitch = width * 4;
        subResource.SysMemSlicePitch = 0;
        if (FAILED(device->CreateTexture2D(&desc, nullptr, out_texture.texture.GetAddressOf()))) {
            delete[] pixels;
            return false;
        }

        D3D11_MAPPED_SUBRESOURCE mappedResource;
        if (SUCCEEDED(context->Map(out_texture.texture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource))) {
            memcpy(mappedResource.pData, pixels, width * height * 4);
            context->Unmap(out_texture.texture.Get(), 0);
        }
        else {
            delete[] pixels;
            return false;
        }

        D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
        ZeroMemory(&srvDesc, sizeof(srvDesc));
        srvDesc.Format = desc.Format;
        srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
        srvDesc.Texture2D.MostDetailedMip = 0;
        srvDesc.Texture2D.MipLevels = 1;
        if (FAILED(device->CreateShaderResourceView(out_texture.texture.Get(), &srvDesc, out_texture.srv.GetAddressOf()))) {
            delete[] pixels;
            return false;
        }

        delete[] pixels;
        SelectObject(hdcMem, hbmOld);
        DeleteDC(hdcMem);
        ReleaseDC(nullptr, hdc);
        DestroyIcon(hIcon);

        return true;
    }

    // Function to get icon for a file or directory
    HICON GetFileIcon(const std::string& path, bool is_directory) {
        SHFILEINFOA shFileInfo;
        UINT flags = SHGFI_ICON | SHGFI_LARGEICON;

        if (is_directory) {
            flags |= SHGFI_USEFILEATTRIBUTES;
            if (!SHGetFileInfoA(path.c_str(), FILE_ATTRIBUTE_DIRECTORY, &shFileInfo, sizeof(shFileInfo), flags)) {
                std::cerr << "Failed to get directory icon for path: " << path << std::endl;
                return nullptr;
            }
        }
        else {
            if (!SHGetFileInfoA(path.c_str(), 0, &shFileInfo, sizeof(shFileInfo), flags)) {
                std::cerr << "Failed to get file icon for path: " << path << std::endl;
                return nullptr;
            }
        }
        return shFileInfo.hIcon;
    }

    // Helper function to draw icon in ImGui
    void DrawIcon(HICON hIcon, ID3D11Device* device, ID3D11DeviceContext* context, std::unordered_map<std::string, IconTexture>& texture_map, const std::string& path) {
        if (hIcon) {
            if (texture_map.find(path) == texture_map.end()) {
                IconTexture iconTexture;
                if (CreateTextureFromIcon(hIcon, iconTexture, device, context)) {
                    texture_map[path] = iconTexture;
                }
            }
            ImGui::Image((void*)texture_map[path].srv.Get(), ImVec2(32, 32));
        }
    }

    std::string selected = "";

    // Function to display the file selector
    // Function to display the file selector
    void ShowFileSelector(bool* p_open, std::string& selected_file, ID3D11Device* device, ID3D11DeviceContext* context) {
        if (!*p_open) return;

        ImGui::Begin("File Selector", p_open, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoScrollbar);

        ImGui::SetWindowPos("File Selector", ImVec2(0, 20));
        ImGui::SetWindowSize("File Selector", ImVec2(gui::size.x, gui::size.y - 20));

        static std::string current_path = std::filesystem::current_path().string();
        static std::string input_path = current_path;
        static std::vector<FileInfo> items;
        static bool refresh = true;

        static int sort_column = 0;
        static bool sort_ascending = true;

        static std::vector<std::pair<std::string, std::string>> quick_access = {
            {"Desktop", GetKnownFolderPath(FOLDERID_Desktop)},
            {"Documents", GetKnownFolderPath(FOLDERID_Documents)},
            {"Downloads", GetKnownFolderPath(FOLDERID_Downloads)},
            {"Music", GetKnownFolderPath(FOLDERID_Music)},
            {"Pictures", GetKnownFolderPath(FOLDERID_Pictures)},
            {"Videos", GetKnownFolderPath(FOLDERID_Videos)},
            {"Home", GetKnownFolderPath(FOLDERID_Profile)}
        };

        static std::vector<DriveInfo> drives = GetDrives();

        static std::stack<std::string> back_stack;
        static std::stack<std::string> forward_stack;

        static std::unordered_map<std::string, IconTexture> texture_map;

        auto navigate_to_path = [&](const std::string& path, bool add_to_history = true) {
            if (std::filesystem::exists(path) && std::filesystem::is_directory(path)) {
                if (add_to_history && current_path != path) {
                    back_stack.push(current_path);
                    while (!forward_stack.empty()) forward_stack.pop();
                }
                current_path = path;
                input_path = current_path;
                refresh = true;
            }
            else {
                std::cerr << "Error: Path does not exist or is not a directory." << std::endl;
            }
            };

        auto go_back = [&]() {
            if (!back_stack.empty()) {
                forward_stack.push(current_path);
                navigate_to_path(back_stack.top(), false);
                back_stack.pop();
            }
            else {
                std::filesystem::path parent_path = std::filesystem::path(current_path).parent_path();
                if (parent_path != current_path) {
                    forward_stack.push(current_path);
                    navigate_to_path(parent_path.string(), false);
                }
            }
            };

        auto go_forward = [&]() {
            if (!forward_stack.empty()) {
                back_stack.push(current_path);
                navigate_to_path(forward_stack.top(), false);
                forward_stack.pop();
            }
            };

        if (refresh) {
            items.clear();
            try {
                for (const auto& entry : std::filesystem::directory_iterator(current_path)) {
                    try {
                        FileInfo info;
                        info.name = entry.path().filename().string();
                        info.is_directory = entry.is_directory();
                        info.type = info.is_directory ? "Folder" : entry.path().extension().string();
                        info.size = info.is_directory ? 0 : entry.file_size();
                        info.last_write_time = entry.last_write_time();
                        items.push_back(info);
                    }
                    catch (const std::filesystem::filesystem_error& e) {
                        std::cerr << "Skipping: " << entry.path().string() << " due to error: " << e.what() << std::endl;
                    }
                }

                std::sort(items.begin(), items.end(), [](const FileInfo& a, const FileInfo& b) {
                    if (sort_column == 0)
                        return sort_ascending ? a.name < b.name : a.name > b.name;
                    if (sort_column == 1)
                        return sort_ascending ? a.type < b.type : a.type > b.type;
                    if (sort_column == 2)
                        return sort_ascending ? a.size < b.size : a.size > b.size;
                    if (sort_column == 3)
                        return sort_ascending ? a.last_write_time < b.last_write_time : a.last_write_time > b.last_write_time;
                    return false;
                    });
            }
            catch (const std::filesystem::filesystem_error& e) {
                std::cerr << "Error accessing directory: " << e.what() << std::endl;
                *p_open = false;
            }
            refresh = false;
        }

        ImGui::BeginChild("Sidebar", ImVec2(200, 0), true);
        if (ImGui::CollapsingHeader("Quick Access")) {
            for (const auto& [name, path] : quick_access) {
                if (ImGui::Selectable(name.c_str())) {
                    navigate_to_path(path);
                }
            }
        }
        if (ImGui::CollapsingHeader("This PC")) {
            for (const auto& drive : drives) {
                std::string drive_label = drive.label + " (" + drive.drive + ")";
                if (ImGui::Selectable(drive_label.c_str())) {
                    navigate_to_path(drive.drive);
                }
            }
        }
        ImGui::EndChild();

        ImGui::SameLine();

        ImGui::BeginGroup();


        ImGui::Text("Current Path");
        ImGui::SameLine();
        if (InputText("##input_path", &input_path, ImGuiInputTextFlags_EnterReturnsTrue)) {
            navigate_to_path(input_path);
        }

        ImGui::BeginGroup();
        if (ImGui::Button("<")) {
            go_back();
        }
        ImGui::SameLine();
        if (ImGui::Button(">")) {
            go_forward();
        }
        ImGui::EndGroup();

        ImGui::BeginChild("##FileGroup");
        if (ImGui::BeginTable("file_list_table", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 200.0f);
            ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.0f);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Date Modified", ImGuiTableColumnFlags_WidthFixed, 200.0f);
            ImGui::TableHeadersRow();

            for (const auto& item : items) {
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                std::string full_path = current_path + "\\" + item.name;
                HICON hIcon = GetFileIcon(full_path, item.is_directory);
                if (hIcon) {
                    DrawIcon(hIcon, device, context, texture_map, full_path);
                    DestroyIcon(hIcon); // Ensure the icon is destroyed after use
                }
                ImGui::SameLine();
                if (ImGui::Selectable(item.name.c_str(), false, ImGuiSelectableFlags_AllowDoubleClick)) {
                    auto selected_path = std::filesystem::path(current_path) / item.name;
                    if (item.is_directory) {
                        navigate_to_path(selected_path.string());
                    }
                    else {
                        selected_file = selected_path.string();
                        *p_open = false;  // Close the dialog after a file is selected
                    }
                }
                ImGui::TableNextColumn();
                ImGui::Text("%s", item.type.c_str());
                ImGui::TableNextColumn();
                if (!item.is_directory)
                    ImGui::Text("%llu", item.size);
                else
                    ImGui::Text("-");
                ImGui::TableNextColumn();
                auto ftime = to_time_t(item.last_write_time);
                ImGui::Text("%s", std::ctime(&ftime));
            }
            ImGui::EndTable();
        }

        ImGui::EndChild();
        ImGui::EndGroup();

        ImGui::End();
    }
    // Function to get icon for a file type based on extension
    HICON GetFileTypeIcon(const std::string& extension) {
        SHFILEINFOA shFileInfo;
        UINT flags = SHGFI_USEFILEATTRIBUTES | SHGFI_ICON | SHGFI_LARGEICON;

        std::string fakePath = "C:\\fake_path\\file." + extension;

        if (SHGetFileInfoA(fakePath.c_str(), FILE_ATTRIBUTE_NORMAL, &shFileInfo, sizeof(shFileInfo), flags)) {
            return shFileInfo.hIcon;
        }
        return nullptr;
    }

    std::unordered_map<std::string, IconTexture> icon_map;

    // Function to get default icon for a file extension from the registry
    HICON GetDefaultIconFromRegistry(const std::string& extension) {
        HKEY hKey;
        std::string regPath = "Software\\Classes\\" + extension;
        if (RegOpenKeyExA(HKEY_CURRENT_USER, regPath.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, regPath.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
                return nullptr;
            }
        }

        char className[256];
        DWORD classNameSize = sizeof(className);
        if (RegQueryValueExA(hKey, nullptr, nullptr, nullptr, (LPBYTE)className, &classNameSize) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return nullptr;
        }
        RegCloseKey(hKey);

        std::string defaultIconPath = "Software\\Classes\\" + std::string(className) + "\\DefaultIcon";
        if (RegOpenKeyExA(HKEY_CURRENT_USER, defaultIconPath.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
            if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, defaultIconPath.c_str(), 0, KEY_READ, &hKey) != ERROR_SUCCESS) {
                return nullptr;
            }
        }

        char iconPath[256];
        DWORD iconPathSize = sizeof(iconPath);
        if (RegQueryValueExA(hKey, nullptr, nullptr, nullptr, (LPBYTE)iconPath, &iconPathSize) != ERROR_SUCCESS) {
            RegCloseKey(hKey);
            return nullptr;
        }
        RegCloseKey(hKey);

        // Extract the path and index
        std::string iconPathStr(iconPath);
        size_t commaPos = iconPathStr.find(',');
        int index = 0;
        if (commaPos != std::string::npos) {
            index = std::stoi(iconPathStr.substr(commaPos + 1));
            iconPathStr = iconPathStr.substr(0, commaPos);
        }

        HICON hIcon = nullptr;
        ExtractIconExA(iconPathStr.c_str(), index, nullptr, &hIcon, 1);
        return hIcon;
    }

    void LoadFileTypeIcons(ID3D11Device* device, ID3D11DeviceContext* context) {
        std::set<std::string> extensions = {
            "jpg", "png", "bmp", "dib", "jpeg", "jfif", "jpe", "gif", "tif", "tiff", "heic",
            "exe", "bat", "com"
        };

        for (const auto& ext : extensions) {
            HICON hIcon = GetDefaultIconFromRegistry("." + ext);
            if (hIcon) {
                IconTexture texture;
                if (CreateTextureFromIcon(hIcon, texture, device, context)) {
                    icon_map[ext] = texture;
                    std::cout << "Loaded icon for extension: " << ext << std::endl;
                }
                else {
                    std::cerr << "Failed to create texture for extension: " << ext << std::endl;
                }
                DestroyIcon(hIcon);
            }
            else {
                std::cerr << "Failed to load icon for extension: " << ext << std::endl;
            }
        }

        // Load default binary icon
        HICON hIcon = GetDefaultIconFromRegistry(".bin");
        if (hIcon) {
            IconTexture texture;
            if (CreateTextureFromIcon(hIcon, texture, device, context)) {
                icon_map["bin"] = texture;
                std::cout << "Loaded default binary icon." << std::endl;
            }
            else {
                std::cerr << "Failed to create texture for default binary icon." << std::endl;
            }
            DestroyIcon(hIcon);
        }
    }

    std::string GetFileTypeIconKey(const std::string& extension) {
        static const std::unordered_set<std::string> image_extensions = {
            "jpg", "png", "bmp", "dib", "jpeg", "jfif", "jpe", "gif", "tif", "tiff", "heic"
        };
        static const std::unordered_set<std::string> executable_extensions = {
            "exe", "bat", "com"
        };

        if (image_extensions.find(extension) != image_extensions.end()) {
            return extension; // Specific image extension
        }
        if (executable_extensions.find(extension) != executable_extensions.end()) {
            return "exe"; // General executable type
        }
        return "bin"; // Default binary type
    }

    uint32_t selected_resource_index = 0;
    void ShowResourceList(ID3D11Device* device, ID3D11DeviceContext* context) {
        ImGui::BeginChild("Resource List");
        if (ImGui::BeginTable("resource_list_table", 4, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Icon", ImGuiTableColumnFlags_WidthFixed, 40.0f);
            ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthFixed, 200.0f);
            ImGui::TableSetupColumn("Size", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableSetupColumn("Extension", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < easyresource::mainresource->m_resources.size(); ++i) {
                const auto& resource = easyresource::mainresource->m_resources[i];
                ImGui::TableNextRow();
                ImGui::TableNextColumn();

                std::string extension = resource.m_name.substr(resource.m_name.find_last_of('.') + 1);
                std::string icon_key = GetFileTypeIconKey(extension);

                ImGui::PushID(static_cast<int>(i));
                if (ImGui::Selectable("", selected_resource_index == static_cast<int>(i), ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap)) {
                    selected_resource_index = static_cast<int>(i);
                }
                ImGui::SameLine();

                if (icon_map.find(icon_key) != icon_map.end()) {
                    // Center the icon
                    float icon_size = 32.0f;
                    float cell_height = ImGui::GetTextLineHeightWithSpacing();
                    float icon_pos_y = (cell_height - icon_size) / 2.f;
                    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + icon_pos_y);
                    ImGui::Image((void*)icon_map[icon_key].srv.Get(), ImVec2(icon_size, icon_size));
                }
                else {
                    ImGui::Text("No Icon");
                }

                ImGui::TableNextColumn();
                ImGui::Text("%s", resource.m_name.c_str());

                ImGui::TableNextColumn();
                ImGui::Text("%llu", resource.m_size);

                ImGui::TableNextColumn();
                ImGui::Text("%s", extension.c_str());

                if (ImGui::BeginPopupContextItem("ResourceContextMenu")) {
                    if (ImGui::MenuItem("Delete")) {
                        if (selected_resource_index != -1) {
                            easyresource::delete_resource(selected_resource_index);
                        }
                    }

                    ImGui::EndPopup();
                }
                ImGui::PopID();
            }

            ImGui::EndTable();
        }
        ImGui::EndChild();
    }
}