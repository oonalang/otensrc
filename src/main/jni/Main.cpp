#include "Includes/Logger.h"
#include "Includes/Macros.h"
#include "Includes/obfuscate.h"
#include "Includes/Utils.h"
#include "ImGui/Call_ImGui.h"
#include "IL2CppSDKGenerator/BasicStructs/Call_BasicStructs.h"
#include "IL2CppSDKGenerator/IL2Cpp/Call_IL2Cpp.h"
#include "Hacks/Hacks.h"
#include "IL2CppSDKGenerator/KittyMemory/MemoryPatch.h"
#include "foxcheats/include/ScanEngine.hpp"
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <string>
#include <functional>
#include <cstring>
#include <cfloat>
#include <jni.h>
#include <pthread.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include "oxorany/source/oxorany.h"
#include "oxorany/source/oxorany.cpp"
#include "oxorany/source/oxorany_include.h"
#include "MainFeatureIncludes.h"
#include "ImGui/ethnir_menu.h"
#include "ImGui/runtime_preview_menu.h"

class _BYTE;
class _BOOL4;
class _BOOL8;
class _WORD;
class _DWORD;
class _QWORD;

#define CREATE_COLOR(r, g, b, a) new float[4] {(float)(r) / 255.0f, (float)(g) / 255.0f, (float)(b) / 255.0f, (float)(a) / 255.0f}

bool ClearDisplay = false;
bool ShowFPS;
bool SnowB = false;
float SnowBsize = 0.0f;
bool isSpeedHackEnabled = false;
float speedHackMultiplier = 1.0f;
bool isJumpAdjustmentEnabled = false;
float jumpHeightMultiplier = 1.0f;
bool RedWallhackShow = false;
char logintext[4096];

static bool camoOff       = true;
static bool camoDiamond   = false;
static bool camoRedSprite = false;
#define ID_DIAMOND    0x1D37F758
#define ID_RED_SPRITE 0x1D37F77E

float menu[4] = { 0.0f / 255.0f, 212.0f / 255.0f, 255.0f / 255.0f, 1.0f };

float g_LastLogoOpacity = 1.0f;
float g_LastLogoSize = 1.0f;
int g_LogoHideDelayFrames = 0;
int g_LogoHideDelay = 40;

#define _BYTE uint8_t
#define _WORD uint8_t
#define _DWORD uint64_t
#define _QWORD uint64_t
#define _BOOL4 uint8_t

#include <fstream>
using namespace std;
#include <Substrate/SubstrateHook.h>
#include <Substrate/CydiaSubstrate.h>

ImFont* F50 = nullptr;
ImFont* F107 = nullptr;
ImFont* SOCIAL = nullptr;
ImFont* Bold = nullptr;
JavaVM* jvm = nullptr;
JavaVM* VM = nullptr;

namespace font {
ImFont* icomoon_logo = nullptr;
ImFont* inter_semibold = nullptr;
ImFont* icomoon_page = nullptr;
}

static int g_GlWidth, g_GlHeight;
static bool g_App = false;

struct My_Patches
{
    MemoryPatch A1;
} Patches;

float AVIWA = 119.167f;
bool wallh;
bool showKeyboard = false;
bool active = false;
float AimSmooth = 1.0f;

struct sRegion
{
    uintptr_t start, end;
};

std::chrono::steady_clock::time_point appStartTime = std::chrono::steady_clock::now();
static bool windowCollapsed = false;
static double collapseBarLastActiveTime = 0.0;
static float collapseBarOpacityAnim = 1.0f;
static float collapseBarPressAnim = 0.0f;
static float uncollapseOpenAnim = 1.0f;
static bool dark = true;
static float tabAlpha = 0.0f;
static float tabAdd = 0.0f;
static int page = 1;
static int activeTab = 1;
bool g_LogoPreviewMode = false;
static bool isLogin = true;
static std::string err;
static std::string storedKey = "";
static char s[256];
static bool g_LoginTextLoaded = false;
static float g_OmniTime = 0.0f;

std::vector<sRegion> trapRegions;
uintptr_t address = 0;
std::string md5(std::string s);
uintptr_t g_il2cpp;
static bool isMenuVisible = true;
int TABG = 1;

static void DrawOmniShimmer(ImDrawList* dl, ImVec2 panelMin, ImVec2 panelMax, float radius, float t)
{
    float w = panelMax.x - panelMin.x;
    float shimmerX = panelMin.x + fmodf(t * 60.0f, w + 80.0f) - 40.0f;
    dl->AddRectFilled(
        ImVec2(panelMin.x, panelMin.y),
        ImVec2(panelMax.x, panelMin.y + 1.5f),
        IM_COL32(0, 212, 255, 60),
        radius
    );
    ImVec2 shimMin(ImClamp(shimmerX - 40.0f, panelMin.x, panelMax.x), panelMin.y);
    ImVec2 shimMax(ImClamp(shimmerX + 40.0f, panelMin.x, panelMax.x), panelMin.y + 1.5f);
    if (shimMax.x > shimMin.x)
    {
        dl->AddRectFilledMultiColor(
            shimMin, shimMax,
            IM_COL32(0, 212, 255, 0),
            IM_COL32(100, 240, 255, 200),
            IM_COL32(0, 212, 255, 0),
            IM_COL32(0, 212, 255, 0)
        );
    }
}

static void RenderSkinsTabContent(float contentWidth, float contentHeight)
{
    (void)contentWidth;
    (void)contentHeight;
    ethnir::EqBeginColumns();
    ethnir::SectionLabel("SKINS");
    RenderSkinCategoryContent(skinSubTab, true);
    ethnir::EqNextColumn();
    ethnir::SectionLabel("CAMO MODIFIER");
    ethnir::BeginGroupCard("eth_camo");
    if (ethnir::RowToggle(nullptr, "Default / OFF", &camoOff)) {
        if (camoOff) {
            camoDiamond = false;
            camoRedSprite = false;
            for (const auto& getitem : itemData) {
                if (getitem.itemName.find("[MYTHIC]") != std::string::npos ||
                    getitem.itemName.find("[LEGENDARY]") != std::string::npos) {
                    for (auto conf : weaponConfInstance) {
                        if (!conf) continue;
                        weaponconfFields = (WeaponConfFields*)((uintptr_t)conf + 0x20);
                        if (weaponconfFields->ID == getitem.WeaponConf[2])
                            weaponconfFields->DefWeaponSkinID = 0;
                    }
                }
            }
        }
    }
    if (ethnir::RowToggle(nullptr, "Diamond Camo", &camoDiamond)) {
        if (camoDiamond) {
            camoOff = false; camoRedSprite = false;
            for (const auto& getitem : itemData) {
                if (getitem.itemName.find("[MYTHIC]") != std::string::npos ||
                    getitem.itemName.find("[LEGENDARY]") != std::string::npos) {
                    for (auto conf : weaponConfInstance) {
                        if (!conf) continue;
                        weaponconfFields = (WeaponConfFields*)((uintptr_t)conf + 0x20);
                        if (weaponconfFields->ID == getitem.WeaponConf[2])
                            weaponconfFields->DefWeaponSkinID = ID_DIAMOND;
                    }
                }
            }
        } else { camoOff = true; }
    }
    if (ethnir::RowToggle(nullptr, "Red Sprite Camo", &camoRedSprite)) {
        if (camoRedSprite) {
            camoOff = false; camoDiamond = false;
            for (const auto& getitem : itemData) {
                if (getitem.itemName.find("[MYTHIC]") != std::string::npos ||
                    getitem.itemName.find("[LEGENDARY]") != std::string::npos) {
                    for (auto conf : weaponConfInstance) {
                        if (!conf) continue;
                        weaponconfFields = (WeaponConfFields*)((uintptr_t)conf + 0x20);
                        if (weaponconfFields->ID == getitem.WeaponConf[2])
                            weaponconfFields->DefWeaponSkinID = ID_RED_SPRITE;
                    }
                }
            }
        } else { camoOff = true; }
    }
    ImGui::Dummy(ImVec2(0, 4));
    ImGui::TextDisabled("Only applies to [M] Mythic and [L] Legendary weapon skins.");
    ethnir::EndGroupCard();
    ethnir::EqEndColumns();
}
// Tab ids follow ethnir::kTabs: 0 Players, 1 AimBot, 2 World, 3 Skins,
// 4 Misc, 5 Config. Every tab lays itself out with the shell's own column and
// card helpers, so all six read the same.
static void EthnirDrawTab(int tab)
{
    const ImVec2 region = ImGui::GetContentRegionAvail();
    const float contentWidth = ImMax(0.0f, region.x);
    const float contentHeight = ImMax(0.0f, region.y);
    switch (tab)
    {
    case 0: runtime_preview_menu::RenderEspTab(contentWidth, contentHeight); break;
    case 1: runtime_preview_menu::RenderAimTab(contentWidth, contentHeight); break;
    case 2: runtime_preview_menu::RenderMemoryTab(contentWidth, contentHeight); break;
    case 3: RenderSkinsTabContent(contentWidth, contentHeight); break;
    case 4: runtime_preview_menu::RenderMiscTab(contentWidth, contentHeight); break;
    case 5: runtime_preview_menu::RenderSettingsTab(contentWidth, contentHeight); break;
    default: break;
    }
}

EGLBoolean (*old_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);

EGLBoolean hook_eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
    eglQuerySurface(dpy, surface, EGL_WIDTH, &g_GlWidth);
    eglQuerySurface(dpy, surface, EGL_HEIGHT, &g_GlHeight);

    if (!g_App)
    {
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.IniFilename = NULL;
        io.LogFilename = NULL;
        io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;
        io.MouseDoubleClickTime = 0.3f;
        io.MouseDragThreshold = 2.f;
        ImGui_ImplOpenGL3_Init("#version 300 es");

        ImFontConfig icomoon_logo_config;
        icomoon_logo_config.MergeMode = false;
        icomoon_logo_config.PixelSnapH = true;
        icomoon_logo_config.FontDataOwnedByAtlas = false;
        font::icomoon_logo = io.Fonts->AddFontFromMemoryTTF(
            (void*)icomoon_page,
            sizeof(icomoon_page),
            20.f,
            &icomoon_logo_config
        );

        ImFontConfig inter_config;
        inter_config.MergeMode = false;
        inter_config.PixelSnapH = true;
        inter_config.FontDataOwnedByAtlas = false;
        font::inter_semibold = io.Fonts->AddFontFromMemoryTTF(
            (void*)inter_semibold,
            sizeof(inter_semibold),
            16.f,
            &inter_config
        );

        ImFontConfig page_config;
        page_config.MergeMode = false;
        page_config.PixelSnapH = true;
        page_config.FontDataOwnedByAtlas = false;
        font::icomoon_page = io.Fonts->AddFontFromMemoryTTF(
            (void*)icomoon_page,
            sizeof(icomoon_page),
            18.f,
            &page_config
        );

        static const ImWchar icons_ranges[] = { 0xe000, 0xf8ff, 0 };
        ImFontConfig iconsConfig;
        iconsConfig.MergeMode = true;
        iconsConfig.PixelSnapH = true;
        iconsConfig.OversampleH = 2.5f;
        iconsConfig.OversampleV = 2.5f;
        iconsConfig.FontDataOwnedByAtlas = false;
        F107 = io.Fonts->AddFontFromMemoryCompressedTTF(
            (void*)font_awesome_data1,
            (int)font_awesome_size1,
            25.0f,
            &iconsConfig,
            icons_ranges
        );
        F50 = io.Fonts->AddFontFromMemoryTTF((void*)F50_data, F50_size, 30.0f, NULL, io.Fonts->GetGlyphRangesDefault());
        if (!F107) {
            F107 = font::inter_semibold;
        }
        if (font::inter_semibold) {
            io.FontDefault = font::inter_semibold;
        }
        io.Fonts->Build();
        ImGui_ImplOpenGL3_CreateFontsTexture();

        memset(&Config, 0, sizeof(sConfig));
        Config.sColorsESPPLAYER.LinePLAYER = CREATE_COLOR(0, 212, 255, 255);
        Config.sColorsESPPLAYER.BoxPLAYER = CREATE_COLOR(0, 212, 255, 255);
        Config.sColorsESPPLAYER.NamePLAYER = CREATE_COLOR(180, 245, 255, 255);
        Config.sColorsESPPLAYER.DistancePLAYER = CREATE_COLOR(140, 230, 255, 200);
        Config.sColorsESPPLAYER.HealthPLAYER = CREATE_COLOR(80, 220, 140, 255);
        Config.sColorsESPPLAYER.SkeletonPLAYER = CREATE_COLOR(0, 212, 255, 200);
        Config.sColorsESPBOT.LineBOT = CREATE_COLOR(0, 210, 120, 180);
        Config.sColorsESPBOT.BoxBOT = CREATE_COLOR(0, 210, 120, 180);
        Config.sColorsESPBOT.NameBOT = CREATE_COLOR(0, 220, 140, 200);
        Config.sColorsESPBOT.HealthBOT = CREATE_COLOR(0, 220, 140, 200);
        Config.sColorsESPBOT.DistanceBOT = CREATE_COLOR(0, 210, 120, 160);
        Config.sColorsESPBOT.SkeletonBOT = CREATE_COLOR(0, 210, 120, 160);
        Config.sColorsESPOTHERS.PovOTHERS = CREATE_COLOR(255, 60, 80, 180);
        Config.Aim.AimAssistSize = 0.0f;
        Config.Aim.Cross = 45.0f;
        Config.Aim.Target = EAimTarget::Heads;
        Config.Aim.Trigger = EAimTrigger::None;
        Config.Aim.By = EAim::Distance;
        Config.Bline = 2.0f;
        Config.Pline = 2.0f;
        g_App = true;
    }

    g_OmniTime += ImGui::GetIO().DeltaTime;

    ImGuiIO *io = &ImGui::GetIO();
    screenWidth = (float)g_GlWidth;
    screenHeight = (float)g_GlHeight;
    io->DisplaySize = ImVec2((float)g_GlWidth, (float)g_GlHeight);

    io->MouseWheel = 0.0f;
    io->MouseWheelH = 0.0f;
    io->MouseDown[0] = false;
    if (m_unity != 0 && Config.ImGuiMenu.thiz != 0)
    {
        const int uniW = get_width(), uniH = get_height();
        const float sx = (uniW > 0) ? (float)g_GlWidth  / (float)uniW : 1.0f;
        const float sy = (uniH > 0) ? (float)g_GlHeight / (float)uniH : 1.0f;
        auto Input_get_touchCount = (int (*)())(Class_Input_get_touchCount);
        auto Input_get_mousePosition = (Vector3(*)(uintptr_t))(Class_Input_get_mousePosition);
        if (Input_get_touchCount() > 0)
        {
            Vector3 touchPos = Input_get_mousePosition(Config.ImGuiMenu.thiz);
            io->MousePos = ImVec2(touchPos.x * sx, g_GlHeight - touchPos.y * sy);
            auto Input_GetTouch = (Touch(*)(uintptr_t, int))(Class_Input_GetTouch);
            switch (Input_GetTouch(Config.ImGuiMenu.thiz, 0).m_Phase)
            {
                case TouchPhase::Began:
                case TouchPhase::Stationary:
                case TouchPhase::Moved:
                    io->MouseDown[0] = true;
                    break;
                case TouchPhase::Ended:
                case TouchPhase::Canceled:
                default:
                    io->MouseDown[0] = false;
                    break;
            }
        }
    }

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    ImDrawList *draw = ImGui::GetBackgroundDrawList();
    DrawESP(ImGui::GetBackgroundDrawList(), screenWidth, screenHeight, get_dpi());
    floating_info::Render(draw, screenWidth, screenHeight);

    if (windowCollapsed)
    {
        static ImVec2 collapsedLogoPos = ImVec2(-1, -1);
        static bool collapsedPosInitialized = false;
        static bool collapsedWasDragging = false;
        static ImVec2 collapsedDragLastMousePos = ImVec2(0.0f, 0.0f);
        static float collapsedDragDistance = 0.0f;
        const float collapsedAlphaSetting = ImClamp(GetLogoOpacity(), 0.0f, 1.0f);
        const float collapsedScaleSetting = ImClamp(GetLogoSizeMultiplier(), 0.1f, 2.0f);
        const float pill_w = 158.0f * c::scale * collapsedScaleSetting;
        const float pill_h = 60.0f * c::scale * collapsedScaleSetting;
        float line_w = pill_w;
        float click_h = pill_h;
        if (!collapsedPosInitialized)
        {
            ImVec2 vp = ImGui::GetMainViewport()->Pos;
            collapsedLogoPos = ImVec2(vp.x + 10.0f * c::scale, vp.y + 10.0f * c::scale);
            collapsedPosInitialized = true;
        }
        collapseBarOpacityAnim = 1.0f;
        ImGui::SetNextWindowPos(collapsedLogoPos, ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(line_w, click_h), ImGuiCond_Always);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
        auto getCollapsedTouchPos = [&]() -> ImVec2
        {
            ImVec2 currentMousePos = ImGui::GetIO().MousePos;
            if (Class_Input_get_touchCount != 0 && Class_Input_get_mousePosition != 0 && Config.ImGuiMenu.thiz != 0)
            {
                auto Input_get_touchCount = (int (*)())(Class_Input_get_touchCount);
                if (Input_get_touchCount() > 0)
                {
                    auto Input_get_mousePosition = (Vector3(*)(uintptr_t))(Class_Input_get_mousePosition);
                    Vector3 pos = Input_get_mousePosition(Config.ImGuiMenu.thiz);
                    currentMousePos = ImVec2(pos.x, get_height() - pos.y);
                }
            }
            return currentMousePos;
        };
        if (ImGui::Begin("##indicator", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize |
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoSavedSettings))
        {
            ImVec2 windowPos = ImGui::GetWindowPos();
            ImGui::SetCursorPos(ImVec2(0, 0));
            ImGui::InvisibleButton("##restoreclick", ImVec2(line_w, click_h));
            bool barHovered = ImGui::IsItemHovered();
            bool barHeld = ImGui::IsItemActive();
            collapseBarPressAnim = ImLerp(collapseBarPressAnim, barHeld ? 1.0f : 0.0f,
                                          ImGui::GetIO().DeltaTime * 18.0f);
            if (ImGui::IsItemActivated())
            {
                collapsedWasDragging = false;
                collapsedDragDistance = 0.0f;
                collapsedDragLastMousePos = getCollapsedTouchPos();
            }
            if (ImGui::IsItemActive())
            {
                ImVec2 currentMousePos = getCollapsedTouchPos();
                ImVec2 delta = ImVec2(
                    currentMousePos.x - collapsedDragLastMousePos.x,
                    currentMousePos.y - collapsedDragLastMousePos.y
                );
                collapsedLogoPos.x += delta.x;
                collapsedLogoPos.y += delta.y;
                collapsedDragDistance += sqrtf((delta.x * delta.x) + (delta.y * delta.y));
                collapsedDragLastMousePos = currentMousePos;
                if (collapsedDragDistance > (6.0f * c::scale))
                    collapsedWasDragging = true;
            }
            if (ImGui::IsItemDeactivated())
            {
                if (!collapsedWasDragging && windowCollapsed)
                {
                    windowCollapsed = false;
                    isMenuVisible = true;
                    uncollapseOpenAnim = 0.0f;
                }
                collapsedWasDragging = false;
                collapsedDragDistance = 0.0f;
            }
            float drawAlpha = ImClamp(collapseBarOpacityAnim * collapsedAlphaSetting, 0.0f, 1.0f);
            ImDrawList* indicatorDraw = ImGui::GetWindowDrawList();
            const float pillR = click_h * 0.5f;
            ImVec2 pillMin = windowPos;
            ImVec2 pillMax = ImVec2(windowPos.x + line_w, windowPos.y + click_h);
            float breathe = 0.5f + 0.5f * sinf(g_OmniTime * 1.8f);
            int glowAlpha = (int)((55.0f + 30.0f * breathe) * drawAlpha);
            indicatorDraw->AddRectFilled(
                ImVec2(pillMin.x - 4, pillMin.y - 4),
                ImVec2(pillMax.x + 4, pillMax.y + 4),
                IM_COL32(0, 180, 255, glowAlpha),
                pillR + 4.0f
            );
            indicatorDraw->AddRectFilledMultiColor(
                pillMin, pillMax,
                IM_COL32(8, 18, 38, (int)(200 * drawAlpha)),
                IM_COL32(6, 12, 28, (int)(200 * drawAlpha)),
                IM_COL32(4, 8, 18, (int)(200 * drawAlpha)),
                IM_COL32(6, 12, 28, (int)(200 * drawAlpha))
            );
            indicatorDraw->AddRectFilled(pillMin, pillMax,
                IM_COL32(8, 16, 32, (int)(180 * drawAlpha)), pillR);
            DrawOmniShimmer(indicatorDraw, pillMin, pillMax, pillR, g_OmniTime);
            indicatorDraw->AddRect(
                pillMin, pillMax,
                IM_COL32(0, 180, 255, (int)(100 * drawAlpha)),
                pillR, 0, 1.2f
            );
            GLuint collapsedLogo = LoadAstralTexture(astral_data, sizeof(astral_data));
            const float logoPad = 6.0f * c::scale * collapsedScaleSetting;
            const float logoSize = 48.0f * c::scale * collapsedScaleSetting;
            const ImVec2 logoMin(windowPos.x + logoPad,
                                  windowPos.y + (click_h - logoSize) * 0.5f);
            const ImVec2 logoMax(logoMin.x + logoSize, logoMin.y + logoSize);
            if (collapsedLogo != 0) {
                indicatorDraw->AddImageRounded(
                    (ImTextureID)(intptr_t)collapsedLogo,
                    logoMin, logoMax,
                    ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f),
                    IM_COL32(255, 255, 255, (int)(255.0f * drawAlpha)),
                    logoSize * 0.5f
                );
            }
            char fpsText[32] = {};
            std::snprintf(fpsText, sizeof(fpsText), "%d FPS",
                          (int)std::lround(ImGui::GetIO().Framerate));
            ImFont* fpsFont = F50 ? F50 : (font::inter_semibold ? font::inter_semibold : ImGui::GetFont());
            const float fpsSize = ((fpsFont == F50) ? (16.0f * c::scale) : (18.0f * c::scale))
                                  * collapsedScaleSetting;
            const ImVec2 fpsSizeVec = fpsFont->CalcTextSizeA(fpsSize, FLT_MAX, 0.0f, fpsText);
            int fpsVal = (int)std::lround(ImGui::GetIO().Framerate);
            ImU32 fpsColor = (fpsVal >= 55) ? IM_COL32(100, 220, 120, 255)
                            : (fpsVal >= 40) ? IM_COL32(0, 212, 255, 255)
                            : IM_COL32(255, 180, 100, 255);
            indicatorDraw->AddText(
                fpsFont, fpsSize,
                ImVec2(logoMax.x + 14.0f * c::scale * collapsedScaleSetting,
                        windowPos.y + (click_h - fpsSizeVec.y) * 0.5f),
                fpsColor,
                fpsText
            );
        }
        ImGui::End();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar(3);
    }

    if (isMenuVisible && !windowCollapsed)
    {
        if (!g_LoginTextLoaded && VM != nullptr)
        {
            if (LoadTextFromFile() && logintext[0] != '\0')
            {
                strncpy(s, logintext, sizeof(s) - 1);
                s[sizeof(s) - 1] = '\0';
                g_LoginTextLoaded = true;
            }
        }

        runtime_preview_menu::EnsureTexturesLoaded();
        main_runtime_theme::ApplyThemeState();
        ImVec2 viewportCenter = ImGui::GetMainViewport()->GetCenter();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));

        if (!isLogin && !ui_loading::IsActive())
        {
            const ImVec2 login_size = ImVec2(500, 560);
            ImGui::SetNextWindowPos(viewportCenter, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            ImGui::SetNextWindowSize(login_size, ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.0f);
            if (ImGui::Begin(OBFUSCATE("Kaelex Login"), nullptr,
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoCollapse))
            {
                const ImVec2 pos = ImGui::GetWindowPos();
                ImDrawList* dl = ImGui::GetWindowDrawList();
                const float R  = 18.0f;
                const float Ri = 14.0f;
                const float inset = 10.0f;
                const ImVec2 outerMin = pos;
                const ImVec2 outerMax = pos + login_size;
                const ImVec2 innerMin = outerMin + ImVec2(inset, inset);
                const ImVec2 innerMax = outerMax - ImVec2(inset, inset);
                float breathe = 0.5f + 0.5f * sinf(g_OmniTime * 1.4f);
                int outerGlowA = (int)(40.0f + 25.0f * breathe);
                dl->AddRectFilled(
                    outerMin - ImVec2(8, 8),
                    outerMax + ImVec2(8, 8),
                    IM_COL32(0, 160, 220, outerGlowA),
                    R + 8.0f
                );
                dl->AddRectFilled(
                    outerMin - ImVec2(4, 4),
                    outerMax + ImVec2(4, 4),
                    IM_COL32(0, 190, 255, outerGlowA + 20),
                    R + 4.0f
                );
                dl->AddRectFilled(outerMin, outerMax,
                    IM_COL32(8, 18, 38, 210), R);
                dl->AddRectFilledMultiColor(
                    outerMin,
                    ImVec2(outerMax.x, outerMin.y + login_size.y * 0.55f),
                    IM_COL32(0, 180, 255, 45),
                    IM_COL32(0, 140, 200, 25),
                    IM_COL32(0, 0, 0, 0),
                    IM_COL32(0, 0, 0, 0)
                );
                dl->AddRect(outerMin, outerMax,
                    IM_COL32(0, 180, 255, 80), R, 0, 1.5f);
                DrawOmniShimmer(dl, outerMin, outerMax, R, g_OmniTime);
                dl->AddRectFilled(innerMin, innerMax,
                    IM_COL32(12, 28, 48, 240), Ri);
                dl->AddRectFilledMultiColor(
                    innerMin,
                    ImVec2(innerMax.x, innerMin.y + (innerMax.y - innerMin.y) * 0.5f),
                    IM_COL32(0, 180, 255, 35),
                    IM_COL32(0, 140, 200, 18),
                    IM_COL32(0, 0, 0, 0),
                    IM_COL32(0, 0, 0, 0)
                );
                dl->AddRect(innerMin, innerMax,
                    IM_COL32(0, 200, 255, 50), Ri, 0, 1.0f);
                float iconCX = pos.x + login_size.x * 0.5f;
                float iconCY = pos.y + 56.0f;
                dl->AddCircleFilled(ImVec2(iconCX, iconCY), 30.0f,
                    IM_COL32(0, 180, 255, (int)(40 + 20 * breathe)), 32);
                dl->AddCircleFilled(ImVec2(iconCX, iconCY), 24.0f,
                    IM_COL32(12, 28, 48, 250), 32);
                dl->AddCircle(ImVec2(iconCX, iconCY), 24.0f,
                    IM_COL32(0, 200, 255, 120), 32, 1.5f);
                {
                    const char* titleText = "Kaelex";
                    const float titleSize = 42.0f;
                    ImVec2 tsz = F50
                        ? F50->CalcTextSizeA(titleSize, FLT_MAX, 0.0f, titleText)
                        : ImGui::CalcTextSize(titleText);
                    float tx = pos.x + (login_size.x - tsz.x) * 0.5f;
                    float ty = pos.y + 90.0f;
                    if (F50)
                        dl->AddText(F50, titleSize, ImVec2(tx + 2, ty + 2),
                            IM_COL32(0, 100, 160, 100), titleText);
                    if (F50)
                        dl->AddText(F50, titleSize, ImVec2(tx, ty),
                            IM_COL32(200, 240, 255, 255), titleText);
                    else
                        dl->AddText(ImVec2(tx, ty),
                            IM_COL32(200, 240, 255, 255), titleText);
                }
                {
                    const char* sub = "PREMIUM ACCESS PORTAL";
                    ImVec2 ssz = ImGui::CalcTextSize(sub);
                    dl->AddText(
                        ImVec2(pos.x + (login_size.x - ssz.x) * 0.5f, pos.y + 135.0f),
                        IM_COL32(120, 200, 255, 140),
                        sub
                    );
                }
                {
                    const char* hint1 = "Paste your license key or type it manually";
                    const char* hint2 = "to continue.";
                    ImVec2 h1sz = ImGui::CalcTextSize(hint1);
                    ImVec2 h2sz = ImGui::CalcTextSize(hint2);
                    dl->AddText(
                        ImVec2(pos.x + (login_size.x - h1sz.x) * 0.5f, pos.y + 158.0f),
                        IM_COL32(160, 200, 230, 160), hint1
                    );
                    dl->AddText(
                        ImVec2(pos.x + (login_size.x - h2sz.x) * 0.5f, pos.y + 176.0f),
                        IM_COL32(160, 200, 230, 160), hint2
                    );
                }
                {
                    const float inputW = 440.0f;
                    const float inputH = 56.0f;
                    const float inputX = (login_size.x - inputW) * 0.5f;
                    ImVec2 inputScreenMin = ImVec2(pos.x + inputX, pos.y + 200.0f);
                    ImVec2 inputScreenMax = inputScreenMin + ImVec2(inputW, inputH);
                    dl->AddRectFilled(
                        inputScreenMin - ImVec2(2, 2),
                        inputScreenMax + ImVec2(2, 2),
                        IM_COL32(0, 180, 255, 35), 14.0f
                    );
                    ImGui::SetCursorPos(ImVec2(inputX, 200.0f));
                    ImGui::AstralInput("##key_login", s, sizeof(s), ImVec2(inputW, inputH));
                }
                bool loginInputClicked = ImGui::IsItemClicked();
                bool loginInputActive  = ImGui::IsItemActive();
                bool loginInputHovered = ImGui::IsItemHovered();
                if (loginInputClicked || loginInputActive)
                    showKeyboard = true;
                if (showKeyboard && !loginInputActive && !loginInputHovered && ImGui::IsMouseClicked(0))
                {
                    float kbH = ImGui::GetIO().DisplaySize.y * 0.60f;
                    if (ImGui::GetMousePos().y < ImGui::GetIO().DisplaySize.y - kbH)
                        showKeyboard = false;
                }
                {
                    const float btnW = 440.0f;
                    const float btnH = 52.0f;
                    const float btnX = (login_size.x - btnW) * 0.5f;
                    ImGui::SetCursorPos(ImVec2(btnX, 274.0f));
                    ImVec2 bMin = ImVec2(pos.x + btnX, pos.y + 274.0f);
                    ImVec2 bMax = bMin + ImVec2(btnW, btnH);
                    dl->AddRectFilled(bMin, bMax, IM_COL32(8, 24, 48, 220), 12.0f);
                    dl->AddRect(bMin, bMax, IM_COL32(0, 180, 255, 90), 12.0f, 0, 1.2f);
                    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.2f, 0.3f, 0.5f, 0.6f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.25f, 0.35f, 0.55f, 0.8f));
                    ImGui::PushStyleColor(ImGuiCol_Border,        ImVec4(0.3f, 0.7f, 1.0f, 0.5f));
                    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.8f, 0.94f, 1.0f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 12.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
                    if (F50) ImGui::PushFont(F50);
                    if (ImGui::Button("PASTE", ImVec2(btnW, btnH)))
                    {
                        auto key = getClipboard();
                        strncpy(s, key.c_str(), sizeof(s) - 1);
                        s[sizeof(s) - 1] = '\0';
                    }
                    if (F50) ImGui::PopFont();
                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor(5);
                }
                {
                    const float btnW = 300.0f;
                    const float btnH = 54.0f;
                    const float btnX = (login_size.x - btnW) * 0.5f;
                    ImGui::SetCursorPos(ImVec2(btnX, 342.0f));
                    ImVec2 bMin = ImVec2(pos.x + btnX, pos.y + 342.0f);
                    ImVec2 bMax = bMin + ImVec2(btnW, btnH);
                    dl->AddRectFilledMultiColor(
                        bMin, bMax,
                        IM_COL32(0, 200, 255, 255),
                        IM_COL32(0, 150, 220, 255),
                        IM_COL32(0, 120, 200, 255),
                        IM_COL32(0, 180, 245, 255)
                    );
                    dl->AddRectFilledMultiColor(
                        bMin,
                        ImVec2(bMax.x, bMin.y + btnH * 0.45f),
                        IM_COL32(255, 255, 255, 35),
                        IM_COL32(255, 255, 255, 35),
                        IM_COL32(255, 255, 255, 0),
                        IM_COL32(255, 255, 255, 0)
                    );
                    dl->AddRect(bMin, bMax,
                        IM_COL32(100, 240, 255, (int)(100 + 50 * breathe)), 14.0f, 0, 1.5f);
                    dl->AddRect(
                        bMin - ImVec2(2, 2), bMax + ImVec2(2, 2),
                        IM_COL32(0, 200, 255, (int)(35 + 20 * breathe)), 16.0f, 0, 1.8f
                    );
                    ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(1, 1, 1, 0.1f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0, 0, 0, 0.15f));
                    ImGui::PushStyleColor(ImGuiCol_Border,        ImVec4(0, 0, 0, 0));
                    ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 14.0f);
                    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 0.0f);
                    if (F50) ImGui::PushFont(F50);
                    if (ImGui::Button("LOGIN", ImVec2(btnW, btnH)))
                    {
                        err = Login(s);
                        if (err == "OK")
                        {
                            showKeyboard = false;
                            strncpy(logintext, s, sizeof(logintext) - 1);
                            logintext[sizeof(logintext) - 1] = '\0';
                            SaveLoginTextToFile(s);
                            g_LoginTextLoaded = true;
                            err.clear();
                            ui_loading::Start(7.0f);
                        }
                    }
                    if (F50) ImGui::PopFont();
                    ImGui::PopStyleVar(2);
                    ImGui::PopStyleColor(5);
                }
                if (!err.empty() && err != "OK")
                {
                    ImVec2 errMin = ImVec2(pos.x + 30.0f, pos.y + 414.0f);
                    ImVec2 errMax = ImVec2(pos.x + login_size.x - 30.0f, pos.y + 444.0f);
                    dl->AddRectFilled(errMin, errMax,
                        IM_COL32(255, 80, 100, 30), 8.0f);
                    dl->AddRect(errMin, errMax,
                        IM_COL32(255, 100, 120, 80), 8.0f, 0, 1.0f);
                    ImGui::SetCursorPos(ImVec2(38.0f, 422.0f));
                    ImGui::TextColored(ImColor(255, 140, 160, 255), "  Error: %s", err.c_str());
                }
                {
                    float divY = pos.y + login_size.y - 52.0f;
                    dl->AddLine(
                        ImVec2(pos.x + 30.0f, divY),
                        ImVec2(pos.x + login_size.x - 30.0f, divY),
                        IM_COL32(0, 180, 255, 50)
                    );
                    const char* contactText = "discord.gg/rodsmodz";
                    ImVec2 ctsz = ImGui::CalcTextSize(contactText);
                    dl->AddText(
                        ImVec2(pos.x + (login_size.x - ctsz.x) * 0.5f, divY + 10.0f),
                        IM_COL32(120, 180, 220, 120),
                        contactText
                    );
                    const char* versionText = "v2.4.1";
                    ImVec2 vtsz = ImGui::CalcTextSize(versionText);
                    dl->AddText(
                        ImVec2(pos.x + login_size.x - vtsz.x - 22.0f, divY + 10.0f),
                        IM_COL32(80, 140, 180, 100),
                        versionText
                    );
                    dl->AddCircleFilled(
                        ImVec2(pos.x + 22.0f, divY + 17.0f),
                        4.5f,
                        IM_COL32(80, 220, 140, 220), 10
                    );
                }
                if (showKeyboard)
                    RenderVirtualKeyboard("##VirtualKeyboardLogin", s, sizeof(s), &showKeyboard);
            }
            ImGui::End();
        }
        else if (!isLogin && ui_loading::IsActive())
        {
            if (ui_loading::RenderWindow((ImTextureID)(intptr_t)runtime_preview_menu::g_menuBackground.id))
            {
                isLogin = true;
            }
        }
        else
        {
            uncollapseOpenAnim = ImClamp(
                uncollapseOpenAnim + ImGui::GetIO().DeltaTime * 5.0f, 0.0f, 1.0f);
            float openEase = uncollapseOpenAnim * uncollapseOpenAnim
                             * (3.0f - 2.0f * uncollapseOpenAnim);
            float openAlpha = 0.2f + 0.8f * openEase;
            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, openAlpha);

            main_runtime_theme::ApplyAccentFromHue();
            c::ApplyMainWindowStyle(ImGui::GetStyle());
            c::UpdateTheme(dark, menu, ImGui::GetIO().DeltaTime);
            main_runtime_theme::ApplyThemeState();

            static ethnir::MenuState menuState;
            // forces the floating info overlay off every frame
            Config.ExtraMenu.ClearDisplay = true;

            // the shell paints its own backdrop instead of the game's wallpaper art
            menuState.Backdrop = nullptr;
            menuState.DrawTab = EthnirDrawTab;
            // debounced 500ms auto save, never mid-drag; writes are atomic
            menuState.OnSave = []() { SaveConfiguration("ethnir"); };

            // touch input is already GL-space; do not rescale again
            menuState.DragScaleX = 1.0f;
            menuState.DragScaleY = 1.0f;

            ethnir::EqRender(menuState);

            if (menuState.HeaderPressed == 0)
            {
                SaveConfiguration("ethnir");
            }

            if (menuState.TrafficPressed == 0)
            {
                runtime_preview_menu::StateRefs refs{
                    dark, tabAlpha, tabAdd, page, activeTab,
                    windowCollapsed, isMenuVisible,
                    collapseBarLastActiveTime,
                    collapseBarOpacityAnim,
                    collapseBarPressAnim
                };
                runtime_preview_menu::CollapseMenu(refs);
            }

            dark = menuState.Dark;

            if (Config.ExtraMenu.WallHack) {
                Patches.A1.Modify();
            } else {
                Patches.A1.Restore();
            }
            ImGui::PopStyleVar();
        }
        ImGui::PopStyleVar();
    }

    ImGui::EndFrame();
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    return old_eglSwapBuffers(dpy, surface);
}

size_t hook_strlen(const char *thread)
{
    if (strstr(thread, "eglSwapBuffers"))
    {
    }
    return strlen(thread);
}

#include <unistd.h>
#include <cstdint>

uintptr_t m_Anogs = 0;

void Init_Thread2() {
    while (!m_Anogs) {
        m_Anogs = Tools::GetBaseAddress("libanogs.so");
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    LOGI("libanogs.so: %p", m_Anogs);
}

void Init_Thread()
{
    while (!m_unity)
    {
        m_unity = Tools::GetBaseAddress("libunity.so");
        sleep(1);
    }
    LOGI("libunity.so: %p", m_unity);
    UpdateAllOffset();
    // these offsets are version-locked: invalid ones are skipped, not hooked
    if (IsOffsetInLibrary("libunity.so", 0x5755800))
        MemoryPatch::createWithHex("libunity.so", 0x5755800, "00 00 80 D2 C0 03 5F D6").Modify();
    if (IsOffsetInLibrary("libunity.so", 0x9FEC8AC))
        MemoryPatch::createWithHex("libunity.so", 0x9FEC8AC, "00 00 80 D2 C0 03 5F D6").Modify();
    if (IsOffsetInLibrary("libunity.so", 0x8D781DC))
        Patches.A1 = MemoryPatch::createWithHex("libunity.so", 0x8D781DC,
            "1F 20 03 D5 E0 03 13 AA");
    if (IsOffsetInLibrary("libunity.so", 0xC9B6F90))
        DobbyHook((void*)getAbsoluteAddress("libunity.so", 0xC9B6F90),
            (void*)&WeaponFireComponent_Instant_CreateBulletLine,
            (void**)&oWeaponFireComponent_Instant_CreateBulletLine);
    if (IsOffsetInLibrary("libunity.so", 0xC9C33A4))
        DobbyHook((void*)getAbsoluteAddress("libunity.so", 0xC9C33A4),
            (void*)&WeaponFireComponent_Instant_CreateBulletProjectile,
            (void**)&oWeaponFireComponent_Instant_CreateBulletProjectile);
    InitializeAllHooks();
    auto swapBuffers = ((uintptr_t)DobbySymbolResolver(
        OBFUSCATE("libunity.so"), OBFUSCATE("eglSwapBuffers")));
    LOGI("eglSwapBuffers: %p", (void*)swapBuffers);
    if (swapBuffers)
        KittyMemory::ProtectAddr((void*)swapBuffers, sizeof(swapBuffers),
            PROT_READ | PROT_WRITE | PROT_EXEC);
    xhook_enable_debug(0);
    xhook_register(OBFUSCATE(".*libunity\\.so$"),
        OBFUSCATE("eglSwapBuffers"),
        (void*)hook_eglSwapBuffers,
        (void**)&old_eglSwapBuffers);
    if (xhook_refresh(0) == 0)
        xhook_clear();
}

__attribute__((constructor))
void native_Init(JNIEnv *env, jclass clazz, jobject mContext) {
}

JNIEXPORT jint JNICALL JNI_OnLoad(JavaVM* vm, void* reserved)
{
    jvm = vm;
    VM  = vm;
    std::thread(Init_Thread).detach();
    std::thread(Init_Thread2).detach();
    std::thread(Skins_Thread).detach();
    return JNI_VERSION_1_6;
}
