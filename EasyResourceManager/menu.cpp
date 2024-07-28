#include "menu.h"
#include <imgui.h>
#include <filesystem>
#include <string>
#include <easyresource.h>
#include "ui.h"
#include "ImExtension.h"
#pragma comment(lib, "EasyResource")
#define GUI_TITLE "EasyResourceGUI"



std::string resource_load_file = "";
bool once = false;

bool show_1 = false;
bool show_2 = false;
uint32_t id = 0;
std::string resource_add_file = "";
bool should_run = false;
void menu::update()
{
    if (!once)
    {
        ImExtension::LoadFileTypeIcons(ui::g_pd3dDevice, ui::g_pd3dDeviceContext);
        once = true;
    }
    ImGui_ImplDX11_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    {
        ImGuiWindowFlags main_window_flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::SetNextWindowPos({ 0, 0 });
        ImGui::SetNextWindowSize(gui::size);
        std::string selected2 = "";
        
        ImGui::Begin(GUI_TITLE, &should_run, main_window_flags);
        auto BackgroundDrawList = ImGui::GetBackgroundDrawList();
        auto ForegroundDrawList = ImGui::GetForegroundDrawList();
        auto WindowDrawList = ImGui::GetWindowDrawList();
        auto WindowSize = ImGui::GetWindowSize();
        auto WindowPos = ImGui::GetWindowPos();
        auto CursorPos = ImGui::GetCursorPos();

        WindowDrawList->AddRectFilled(ImVec2(0, 0), ImVec2(900, 19), 0xFFFFFFFF);
        WindowDrawList->AddText(ImVec2(3, 3.6), 0xFF151515, "EasyResource Resource Manager");

        ForegroundDrawList->AddLine(ImVec2(0, 0), ImVec2(0, 500), 0xFFFFFFFF);
        ForegroundDrawList->AddLine(ImVec2(0, 499), ImVec2(899, 499), 0xFFFFFFFF);
        ForegroundDrawList->AddLine(ImVec2(899, 499), ImVec2(899, 0), 0xFFFFFFFF);

      

        auto style = ImGui::GetStyle();
        BackgroundDrawList->AddRectFilled(WindowPos, ImVec2(WindowPos.x + WindowSize.x, WindowPos.y + WindowSize.y), IM_COL32(15, 14, 14, 255), style.WindowRounding); // MainBg
        
        ImGui::SetCursorPosY(20);

        if (ImGui::Button("Resource Editor", ImVec2(445 * 2, 20)))
            id = 0;
    
        ImGui::Separator();
     
        

        if (id == 0)
        {
            if (ImGui::Button("Select File"))
                show_1 = true;
            ImGui::SameLine();
            ImGui::Text(resource_load_file.size() ? ("Selected File: %s \n", resource_load_file.c_str()) : "Please select a file to load.");

            if (resource_load_file.size())
            {
                if (ImGui::Button("Load File"))
                {
                    std::filesystem::path file_path(resource_load_file);
                    std::string directory_path = file_path.parent_path().string();

                    easyresource::initialize(directory_path);

                    easyresource::load_resources();
                }         
                ImGui::SameLine();
                if (ImGui::Button("Create Resource"))
                {
                    std::filesystem::path file_path(resource_load_file);
                    std::string directory_path = file_path.parent_path().string();

                    easyresource::initialize(directory_path);
                    printf("%s \n", directory_path.c_str());
                    easyresource::save_resources();
                }

                ImGui::SameLine();

                if (ImGui::Button("Extract Resource"))
                {
                    easyresource::extract_resources();
                }
                ImGui::SameLine();
                if (ImGui::Button("Save"))
                {
                    easyresource::save_resources();
                }
                ImGui::SameLine();
                if (ImGui::Button("Add File"))
                {
                    easyresource::add_file_to_resources(resource_load_file, resource_add_file);
                    easyresource::save_resources();
                }
                ImGui::SameLine();
                ImExtension::InputTextWithHint("##ResourceName", "<resource name>", &resource_add_file, 0, 0, 0);
            }
            
            ImExtension::ShowFileSelector(&show_1, resource_load_file, ui::g_pd3dDevice, ui::g_pd3dDeviceContext);

            if (easyresource::mainresource && resource_load_file.size())
                ImExtension::ShowResourceList(ui::g_pd3dDevice, ui::g_pd3dDeviceContext);
        }
       
        ImGui::End();
    }

    // Rendering
    ImGui::Render();
}










