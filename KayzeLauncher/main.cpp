#define _CRT_SECURE_NO_WARNINGS
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <vector>
#include <string>
#include <windows.h>
#include <algorithm>
#include <iostream>
#include <dwmapi.h>
#include <fstream>
#include <urlmon.h>
#include <wininet.h>

#include "font_data.h" 
#include "json.hpp"
using json = nlohmann::json;

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")
#pragma comment(lib, "dwmapi.lib")
#pragma comment(lib, "urlmon.lib")
#pragma comment(lib, "wininet.lib")

#define IDI_ICON1 101 
static bool g_CanDrag = true;

const char* DB_URL = "https://gist.githubusercontent.com/micheleoconedev/3be1241eafb8449b2bebc28ce0a79488/raw/games.json";
const char* DB_FILENAME = "games_cache.temp";

const ImVec4 colTextLight = ImVec4(0.95f, 0.96f, 0.98f, 1.0f);
const ImVec4 colTextGrey = ImVec4(1.00f, 1.00f, 1.00f, 0.40f);
const ImVec4 colCyan = ImVec4(0.06f, 0.72f, 0.83f, 1.0f);
const ImVec4 colBoxBg = ImVec4(0.16f, 0.16f, 0.18f, 1.00f);
const ImVec4 colBorder = ImVec4(1.0f, 1.0f, 1.0f, 0.08f);
const ImVec4 colBtnStop = ImVec4(0.6f, 0.1f, 0.1f, 0.8f);

struct GameInfo {
    std::string name;
    std::string id;
    std::vector<std::string> executables;
};

std::string ToLower(std::string s) {
    std::transform(s.begin(), s.end(), s.begin(), [](unsigned char c) { return std::tolower(c); });
    return s;
}

std::vector<GameInfo> FetchGameDatabase() {
    std::vector<GameInfo> db;
    DeleteUrlCacheEntryA(DB_URL);
    HRESULT hr = URLDownloadToFileA(NULL, DB_URL, DB_FILENAME, 0, NULL);

    SetFileAttributesA(DB_FILENAME, FILE_ATTRIBUTE_HIDDEN);

    std::ifstream f(DB_FILENAME);
    if (f.is_open()) {
        try {
            json data = json::parse(f);
            for (auto& item : data) {
                GameInfo game;
                game.name = item.value("name", "Unknown");
                game.id = item.value("id", "0");
                if (item.contains("executables") && item["executables"].is_array()) {
                    for (auto& exe : item["executables"]) game.executables.push_back(exe.get<std::string>());
                }
                if (!game.executables.empty()) db.push_back(game);
            }
        }
        catch (...) { db.push_back({ "Database Error", "0", {} }); }
        f.close();
    }
    else {
        db.push_back({ "Connection Error", "0", {} });
    }
    return db;
}

void TextCentered(std::string text) {
    auto windowWidth = ImGui::GetContentRegionAvail().x;
    auto windowHeight = ImGui::GetContentRegionAvail().y;
    auto textWidth = ImGui::CalcTextSize(text.c_str()).x;
    auto textHeight = ImGui::CalcTextSize(text.c_str()).y;
    ImGui::SetCursorPosX((windowWidth - textWidth) * 0.5f);
    ImGui::SetCursorPosY((windowHeight - textHeight) * 0.5f);
    ImGui::TextColored(ImVec4(1.0f, 1.0f, 1.0f, 0.4f), "%s", text.c_str());
}

LRESULT CALLBACK GhostProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_DESTROY) PostQuitMessage(0);
    return DefWindowProcA(hwnd, uMsg, wParam, lParam);
}

void RunAsFakeGame(std::string gameName) {
    const char* CLASS_NAME = "KayzeGhostClass";
    WNDCLASSEXA wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEXA);
    wc.lpfnWndProc = GhostProc;
    wc.hInstance = GetModuleHandle(NULL);
    wc.lpszClassName = CLASS_NAME;
    wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));
    wc.hIconSm = LoadIcon(wc.hInstance, MAKEINTRESOURCE(IDI_ICON1));
    if (!wc.hIcon) wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    if (!wc.hIconSm) wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    RegisterClassExA(&wc);
    HWND hwnd = CreateWindowExA(0, CLASS_NAME, gameName.c_str(), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, NULL, NULL, GetModuleHandle(NULL), NULL);
    ShowWindow(hwnd, SW_MINIMIZE);
    MSG msg = { };
    while (GetMessageA(&msg, NULL, 0, 0) > 0) { TranslateMessage(&msg); DispatchMessageA(&msg); }
}

struct RunningGame {
    GameInfo info;
    std::string activeExe;
    PROCESS_INFORMATION pi = { 0 };
    bool isRunning = false;
};

bool StartGhostProcess(RunningGame& game, std::string chosenExe) {
    char currentPath[MAX_PATH]; GetModuleFileNameA(NULL, currentPath, MAX_PATH);
    std::string currentDir = currentPath;
    size_t lastSlash = currentDir.find_last_of("\\/");
    if (lastSlash != std::string::npos) currentDir = currentDir.substr(0, lastSlash);
    std::string destPath = currentDir + "\\" + chosenExe;
    if (!CopyFileA(currentPath, destPath.c_str(), FALSE)) return false;
    STARTUPINFOA si; ZeroMemory(&si, sizeof(si)); si.cb = sizeof(si);
    ZeroMemory(&game.pi, sizeof(game.pi));
    std::string cmdLine = "\"" + destPath + "\" -ghost";
    if (CreateProcessA(NULL, (LPSTR)cmdLine.c_str(), NULL, NULL, FALSE, 0, NULL, NULL, &si, &game.pi)) {
        game.isRunning = true; game.activeExe = chosenExe; return true;
    } return false;
}

void StopGhostProcess(RunningGame& game) {
    if (game.isRunning) {
        TerminateProcess(game.pi.hProcess, 0); WaitForSingleObject(game.pi.hProcess, 2000);
        CloseHandle(game.pi.hProcess); CloseHandle(game.pi.hThread);
        for (int i = 0; i < 10; i++) { if (DeleteFileA(game.activeExe.c_str())) break; Sleep(100); }
        game.isRunning = false;
    }
}

void SetupKayzeStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    float roundness = 12.0f;
    style.WindowRounding = roundness;
    style.FrameRounding = roundness;
    style.ChildRounding = roundness;
    style.PopupRounding = roundness;
    style.ScrollbarRounding = roundness;
    style.GrabRounding = roundness;
    style.WindowPadding = ImVec2(15, 15);
    style.ItemSpacing = ImVec2(12, 12);
    style.ButtonTextAlign = ImVec2(0.5f, 0.5f);
    style.WindowBorderSize = 0.0f;
    style.ChildBorderSize = 1.0f;
    style.FrameBorderSize = 0.0f;

    ImVec4* colors = style.Colors;
    colors[ImGuiCol_WindowBg] = ImVec4(0, 0, 0, 0);
    colors[ImGuiCol_FrameBg] = colBoxBg;
    colors[ImGuiCol_ChildBg] = colBoxBg;
    colors[ImGuiCol_PopupBg] = ImVec4(0.12f, 0.12f, 0.14f, 0.98f);
    colors[ImGuiCol_Border] = colBorder;
    colors[ImGuiCol_Text] = colTextLight;
    colors[ImGuiCol_TextDisabled] = ImVec4(0.5f, 0.5f, 0.5f, 1.0f);
    colors[ImGuiCol_Button] = ImVec4(0.11f, 0.50f, 0.87f, 0.6f);
    colors[ImGuiCol_ButtonHovered] = ImVec4(0.06f, 0.72f, 0.83f, 0.8f);
    colors[ImGuiCol_ButtonActive] = ImVec4(0.06f, 0.72f, 0.83f, 1.0f);
    colors[ImGuiCol_Header] = ImVec4(0.11f, 0.50f, 0.87f, 0.5f);
    colors[ImGuiCol_HeaderHovered] = colCyan;
}

static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

int main(int argc, char** argv) {
    std::string cmdLine = GetCommandLineA();
    if (cmdLine.find("-ghost") != std::string::npos) {
        char myPath[MAX_PATH]; GetModuleFileNameA(NULL, myPath, MAX_PATH);
        std::string myName = myPath;
        size_t lastSlash = myName.find_last_of("\\/");
        if (lastSlash != std::string::npos) myName = myName.substr(lastSlash + 1);
        RunAsFakeGame(myName); return 0;
    }

    WNDCLASSEXW wc = { sizeof(wc), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, L"KAYZEQuestCompleter", nullptr };
    wc.hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
    wc.hIconSm = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
    ::RegisterClassExW(&wc);

    int w = 550, h = 850;
    int x = (GetSystemMetrics(SM_CXSCREEN) - w) / 2;
    int y = (GetSystemMetrics(SM_CYSCREEN) - h) / 2;
    HWND hwnd = ::CreateWindowW(wc.lpszClassName, L"KAYZE Discord Quest Completer", WS_POPUP, x, y, w, h, nullptr, nullptr, wc.hInstance, nullptr);
    MARGINS margins = { -1 }; DwmExtendFrameIntoClientArea(hwnd, &margins);

    if (!CreateDeviceD3D(hwnd)) { CleanupDeviceD3D(); return 1; }
    ::ShowWindow(hwnd, SW_SHOWDEFAULT);
    ::UpdateWindow(hwnd);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    io.IniFilename = NULL;

    SetupKayzeStyle();

    ImFontConfig fontConfig;
    fontConfig.FontDataOwnedByAtlas = false;

    ImFont* fontRegular = io.Fonts->AddFontFromMemoryTTF((void*)font_regular_data, sizeof(font_regular_data), 20.0f, &fontConfig);
    ImFont* fontBold = io.Fonts->AddFontFromMemoryTTF((void*)font_bold_data, sizeof(font_bold_data), 20.0f, &fontConfig);

    if (!fontRegular) fontRegular = io.Fonts->AddFontDefault();
    if (!fontBold) fontBold = io.Fonts->AddFontDefault();

    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    auto db = FetchGameDatabase();

    std::vector<RunningGame> activeGames;
    static char searchBuf[128] = "";
    GameInfo* currentSelectedGame = nullptr;
    bool done = false;

    while (!done) {
        MSG msg;
        while (::PeekMessage(&msg, nullptr, 0U, 0U, PM_REMOVE)) {
            ::TranslateMessage(&msg); ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT) done = true;
        }
        if (done) break;

        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
        ImGuiStyle& style = ImGui::GetStyle();

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::Begin("Main", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
        g_CanDrag = !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive();

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddRectFilled(ImVec2(0, 0), ImVec2((float)w, (float)h), IM_COL32(18, 18, 18, 255), 12.0f);

        ImGui::PushFont(fontBold);
        ImGui::AlignTextToFramePadding();
        ImGui::TextColored(ImVec4(1, 1, 1, 0.5f), "1.0 Stable");
        ImGui::SameLine();
        ImGui::TextColored(ImVec4(1, 1, 1, 0.5f), "|");
        ImGui::SameLine();

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0.15f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.25f));
        if (ImGui::Button("GITHUB")) {
            ShellExecuteA(NULL, "open", "https://github.com/micheleoconedev", NULL, NULL, SW_SHOWNORMAL);
        }
        ImGui::PopStyleColor(2);
        ImGui::PopFont();

        float smallBtnSize = 18.0f;
        ImGui::SameLine(ImGui::GetWindowWidth() - (smallBtnSize * 2) - 30.0f);

        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(1, 1, 1, 0.1f));
        if (ImGui::Button("-", ImVec2(smallBtnSize, smallBtnSize))) ShowWindow(hwnd, SW_MINIMIZE);
        ImGui::SameLine();
        if (ImGui::Button("X", ImVec2(smallBtnSize, smallBtnSize))) done = true;
        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
        ImGui::Separator();

        ImGui::Spacing();
        float winWidth = ImGui::GetWindowWidth();

        ImGui::PushFont(fontBold);
        ImGui::SetWindowFontScale(3.5f);
        std::string title = "KAYZE";
        float titleW = ImGui::CalcTextSize(title.c_str()).x;
        ImGui::SetCursorPosX((winWidth - titleW) * 0.5f);
        ImGui::TextColored(colCyan, title.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopFont();

        ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 25.0f);

        ImGui::PushFont(fontRegular);
        ImGui::SetWindowFontScale(1.2f);
        std::string sub = "DISCORD QUEST COMPLETER";
        float subW = ImGui::CalcTextSize(sub.c_str()).x;
        ImGui::SetCursorPosX((winWidth - subW) * 0.5f);
        ImGui::TextColored(colTextGrey, sub.c_str());
        ImGui::SetWindowFontScale(1.0f);
        ImGui::PopFont();

        ImGui::Spacing(); ImGui::Spacing();

        ImGui::PushFont(fontRegular);
        ImGui::SetCursorPosX(30);
        ImGui::PushItemWidth(winWidth - 60);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(10, 10));
        ImGui::InputTextWithHint("##search", "Search game...", searchBuf, IM_ARRAYSIZE(searchBuf));
        ImGui::PopStyleVar();
        ImGui::PopItemWidth();
        ImGui::PopFont();
        ImGui::Spacing();

        ImGui::PushFont(fontBold);
        ImGui::TextColored(ImVec4(1, 1, 1, 0.8f), "Search Results:");
        ImGui::PopFont();

        ImGui::BeginChild("ResultsBox", ImVec2(0, 180), true);
        std::string searchQ = ToLower(searchBuf);

        if (searchQ.length() > 0) {
            bool found = false;
            for (auto& game : db) {
                if (ToLower(game.name).find(searchQ) != std::string::npos) {
                    found = true;
                    ImGui::BeginChild(game.name.c_str(), ImVec2(0, 55), true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

                    float textH = ImGui::GetTextLineHeight();
                    ImGui::SetCursorPosY((55.0f - textH) * 0.5f);
                    ImGui::SetCursorPosX(15.0f);

                    ImGui::PushFont(fontBold);
                    ImGui::Text("%s", game.name.c_str());
                    ImGui::PopFont();

                    float btnH = 30.0f;
                    float btnW = 90.0f;
                    ImGui::SetCursorPosY((55.0f - btnH) * 0.5f);
                    ImGui::SetCursorPosX(ImGui::GetWindowWidth() - btnW - 15.0f);

                    ImGui::PushFont(fontBold);
                    if (ImGui::Button("SELECT", ImVec2(btnW, btnH))) currentSelectedGame = &game;
                    ImGui::PopFont();
                    ImGui::EndChild();
                }
            }
            if (!found) TextCentered("No results found.");
        }
        else {
            TextCentered("Type a game name above...");
        }
        ImGui::EndChild();

        ImGui::Spacing(); ImGui::Separator(); ImGui::Spacing();

        float footerHeight = 40.0f;
        float bottomMargin = 10.0f;

        if (currentSelectedGame != nullptr) {
            ImGui::PushFont(fontBold);
            ImGui::TextColored(colCyan, "SELECTED GAME:");
            ImGui::PopFont();

            float boxHeight = ImGui::GetContentRegionAvail().y - footerHeight - bottomMargin;

            ImGui::BeginChild("SelectionBox", ImVec2(0, boxHeight), true);

            ImGui::PushFont(fontBold);
            ImGui::SetWindowFontScale(1.2f);
            ImGui::Text("%s", currentSelectedGame->name.c_str());
            ImGui::SetWindowFontScale(1.0f);
            ImGui::PopFont();

            ImGui::PushFont(fontRegular);
            ImGui::TextDisabled("ID: %s", currentSelectedGame->id.c_str());
            ImGui::Separator(); ImGui::Spacing();
            ImGui::Text("Available versions:");
            ImGui::PopFont();

            float availWidth = ImGui::GetContentRegionAvail().x;
            int numExecutables = (int)currentSelectedGame->executables.size();
            float spacing = style.ItemSpacing.x;
            float halfWidth = (availWidth - spacing) * 0.5f;

            ImGui::PushFont(fontBold);
            for (int i = 0; i < numExecutables; ++i) {
                const auto& exe = currentSelectedGame->executables[i];

                bool alreadyRunning = false;
                std::vector<RunningGame>::iterator activeGameIt;
                for (auto it = activeGames.begin(); it != activeGames.end(); ++it) {
                    if (it->activeExe == exe && it->isRunning) {
                        alreadyRunning = true;
                        activeGameIt = it;
                        break;
                    }
                }

                float currentBtnWidth = halfWidth;
                if (numExecutables % 2 != 0 && i == numExecutables - 1) {
                    currentBtnWidth = availWidth;
                }

                if (alreadyRunning) {
                    ImGui::PushStyleColor(ImGuiCol_Button, colBtnStop);
                    if (ImGui::Button(("STOP: " + exe).c_str(), ImVec2(currentBtnWidth, 30))) {
                        StopGhostProcess(*activeGameIt);
                        activeGames.erase(activeGameIt);
                    }
                    ImGui::PopStyleColor();
                }
                else {
                    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.06f, 0.72f, 0.83f, 0.5f));
                    if (ImGui::Button(("START: " + exe).c_str(), ImVec2(currentBtnWidth, 30))) {
                        RunningGame rg; rg.info = *currentSelectedGame;
                        if (StartGhostProcess(rg, exe)) activeGames.push_back(rg);
                    }
                    ImGui::PopStyleColor();
                }

                if (i % 2 == 0 && i < numExecutables - 1) {
                    ImGui::SameLine();
                }
            }
            ImGui::PopFont();
            ImGui::EndChild();
        }
        else {
            float boxHeight = ImGui::GetContentRegionAvail().y - footerHeight - bottomMargin;
            ImGui::BeginChild("SelectionBox", ImVec2(0, boxHeight), true);
            TextCentered("No game selected.");
            ImGui::EndChild();
        }

        float windowHeight = ImGui::GetWindowHeight();
        float footerStartY = windowHeight - 40.0f;

        ImGui::SetCursorPosY(footerStartY);
        ImGui::Separator();

        float textLineHeight = ImGui::GetTextLineHeight();
        float textCenterY = footerStartY + (40.0f - textLineHeight) * 0.5f;

        std::string t1 = "dev by ";
        std::string t2 = "KAYZE";

        float totalW = ImGui::CalcTextSize(t1.c_str()).x + ImGui::CalcTextSize(t2.c_str()).x;
        float textX = (ImGui::GetWindowWidth() - totalW) * 0.5f;

        ImGui::SetCursorPos(ImVec2(textX, textCenterY));
        ImGui::PushFont(fontRegular);
        ImGui::TextDisabled("%s", t1.c_str());
        ImGui::PopFont();

        ImGui::SameLine(0, 0);

        ImGui::PushFont(fontBold);
        ImGui::TextColored(colCyan, "%s", t2.c_str());
        ImGui::PopFont();

        ImGui::End();
        ImGui::Render();
        const float clear_color[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, nullptr);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        g_pSwapChain->Present(1, 0);
    }

    for (auto& g : activeGames) StopGhostProcess(g);
    ImGui_ImplDX11_Shutdown(); ImGui_ImplWin32_Shutdown(); ImGui::DestroyContext();
    CleanupDeviceD3D(); ::DestroyWindow(hwnd); ::UnregisterClassW(wc.lpszClassName, wc.hInstance);
    return 0;
}

bool CreateDeviceD3D(HWND hWnd) {
    DXGI_SWAP_CHAIN_DESC sd; ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2; sd.BufferDesc.Width = 0; sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60; sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH; sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd; sd.SampleDesc.Count = 1; sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE; sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
    UINT createDeviceFlags = 0; D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    if (D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext) != S_OK) return false;
    CreateRenderTarget(); return true;
}
void CleanupDeviceD3D() { CleanupRenderTarget(); if (g_pSwapChain) g_pSwapChain->Release(); if (g_pd3dDeviceContext) g_pd3dDeviceContext->Release(); if (g_pd3dDevice) g_pd3dDevice->Release(); }
void CreateRenderTarget() { ID3D11Texture2D* pBackBuffer; g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer)); g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView); pBackBuffer->Release(); }
void CleanupRenderTarget() { if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; } }
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam)) return true;
    switch (msg) {
    case WM_SIZE: if (g_pd3dDevice != nullptr && wParam != SIZE_MINIMIZED) { CleanupRenderTarget(); g_pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0); CreateRenderTarget(); } return 0;
    case WM_NCHITTEST: { LRESULT hit = DefWindowProc(hWnd, msg, wParam, lParam); if (hit == HTCLIENT && g_CanDrag) return HTCAPTION; return hit; }
    case WM_DESTROY: ::PostQuitMessage(0); return 0;
    } return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}