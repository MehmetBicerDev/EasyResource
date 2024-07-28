#pragma once
#include <imgui.h>
#include <imgui_impl_dx11.h>
#include <imgui_impl_win32.h>
#include <Windows.h>
#include <d3d11.h>
#include "includes.h"



namespace gui
{
    static POINTS					guiPosition = { };
    static ImVec2					size = { 900 , 500 };
    static int                      title_height = 20;
    static int                      sidebar_width = 0;
}


namespace ui {
    bool init();
    void update();
    extern ID3D11Device* g_pd3dDevice;
    extern ID3D11DeviceContext* g_pd3dDeviceContext;
}