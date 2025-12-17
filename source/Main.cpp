#define NTDDI_VERSION NTDDI_WIN7
#define _WIN32_WINNT _WIN32_WINNT_WIN7

#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <d2d1.h>
#include <functional>
#include "D2DRenderTarget.h"
#include "GalvoSimulator.h"
#include "LaserFrameGenerator.h"
#pragma comment(lib, "Comctl32.lib")


LaserFrameGenerator g_laserFrameGen;
D2DRenderTarget g_renderTarget;
LaserRenderer laserRenderer;
SimFrame g_simFrame;
GalvoSimulator g_GalvoSim(g_laserFrameGen.GetLaserFrame(), g_simFrame);

struct SliderControl
{
    HWND hwndSlider;        // Handle to the trackbar
    HWND hwndLabel;         // Label for the slider
    HWND hwndValue;         // Label showing numeric value
    float scale;            // Scale factor to convert slider int → float
    std::function<void(float)> onChange; // Callback when value changes

    int minVal;
    int maxVal;
    const wchar_t* labelText;

    void Create(HWND parent, HINSTANCE hInst, int x, int y, int width)
    {
        // Label
        hwndLabel = CreateWindow(L"STATIC", labelText, WS_CHILD | WS_VISIBLE,
            x, y, width, 20, parent, nullptr, hInst, nullptr);

        // Slider
        hwndSlider = CreateWindowEx(0, TRACKBAR_CLASS, L"",
            WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
            x, y + 20, width - 50, 30, parent, nullptr, hInst, nullptr);

        SendMessage(hwndSlider, TBM_SETRANGE, TRUE, MAKELONG(minVal, maxVal));
        SendMessage(hwndSlider, TBM_SETPOS, TRUE, minVal + (maxVal - minVal) / 2);

        // Value display
        hwndValue = CreateWindow(L"STATIC", L"", WS_CHILD | WS_VISIBLE | SS_CENTER,
            x + width - 45, y + 20, 45, 30, parent, nullptr, hInst, nullptr);

        UpdateValue(); // Set initial text
    }

    void UpdateValue() const
    {
        int pos = (int)SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
        float val = pos * scale;

        // Update numeric display
        wchar_t buf[32];
        swprintf(buf, 32, L"%.2f", val);
        SetWindowText(hwndValue, buf);

        // Call the callback
        if (onChange)
            onChange(val);
    }
};


std::vector<SliderControl> sliders;

#ifdef _DEBUG
static LRESULT CALLBACK ControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        int yPos = 10;
        HINSTANCE hInst = (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE);

        // Max Angle slider
        sliders.push_back({
            nullptr, nullptr, nullptr, 1.0f,
            [&] (float val) { g_GalvoSim.SetMaxAngle(val); },
            25, 35, L"Max Angle"
            });

        // Speed slider
        sliders.push_back({
            nullptr, nullptr, nullptr, 1.0f,
            [&] (float val) { g_GalvoSim.maxSpeed = val; },
            25, 75, L"Speed"
            });

        // Damping slider
        sliders.push_back({
            nullptr, nullptr, nullptr, 1.0f,
            [&] (float val) { g_GalvoSim.damping = val; },
            10, 30, L"Damping"
            });

        // Stiffness slider
        sliders.push_back({
            nullptr, nullptr, nullptr, 100.0f,
            [&] (float val) { g_GalvoSim.stiffness = val; },
            5, 15, L"Stiffness"
            });

        for (auto& s : sliders)
        {
            s.Create(hwnd, hInst, 10, yPos, 260);
            yPos += 60;
        }

        break;
    }
    case WM_HSCROLL:
    {
        HWND hCtrl = (HWND)lParam;
        for (auto& s : sliders)
        {
            if (s.hwndSlider == hCtrl)
            {
                s.UpdateValue();
                // You can now use the updated variable via s.pValue
                break;
            }
        }
        return 0;
    }
    case WM_CLOSE:
        DestroyWindow(hwnd);
        return 0;

    case WM_DESTROY:
        return 0;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
#endif

static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_PAINT:
        ValidateRect(hwnd, nullptr);
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    break;
    }
    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int WINAPI wWinMain(
    _In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ PWSTR lpCmdLine,
    _In_ int nCmdShow
)
{
    // Register class
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"LaserEmuWndClass";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClass(&wc);

#ifdef _DEBUG
    WNDCLASSEX wc2 = {};
    wc2.cbSize = sizeof(wc2);
    wc2.lpfnWndProc = ControlWndProc;
    wc2.hInstance = hInstance;
    wc2.lpszClassName = L"ControlWindowClass";
    wc2.hCursor = LoadCursor(nullptr, IDC_ARROW);
    RegisterClassEx(&wc2);

    INITCOMMONCONTROLSEX icc;
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_BAR_CLASSES;   // Needed for trackbars (sliders)
    InitCommonControlsEx(&icc);
#endif


    // Create window
    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, L"Laser Emulator (Direct2D)",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 600, 600,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

#ifdef _DEBUG
    HWND hControlWnd = CreateWindowEx(
        0,
        L"ControlWindowClass",
        L"Laser Simulation Controls",
        WS_OVERLAPPEDWINDOW,
        100, 100, 300, 300,
        nullptr,
        nullptr,
        hInstance,
        nullptr
    );
    ShowWindow(hControlWnd, SW_SHOW);
    UpdateWindow(hControlWnd);
#endif

    // message + render loop
    g_renderTarget.Initialize(hwnd);
	laserRenderer.Initialize(g_renderTarget.GetD2DRenderTarget());
    laserRenderer.Resize(g_renderTarget.getScreenWidth(), g_renderTarget.getScreenHeight());
    MSG msg;
    int mouseX = 0;
    int mouseY = 0;
	float hue = 0.0f;
	float angle = 0.0f;
    const float simsteps_per_second = 30000.0f;
	const float fps = 60.0f;
    const float simsteps = simsteps_per_second / fps;
    float dt = 1.0f / simsteps;
	LaserFrameGenerator frameGen;
    bool running = true;
    while (running)
    {
        // pump messages
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) { running = false; break; }
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_MOUSEMOVE)
            {
                mouseX = GET_X_LPARAM(msg.lParam);
                mouseY = GET_Y_LPARAM(msg.lParam);
            }
            if (msg.message == WM_SIZE)
            {
                int width = LOWORD(msg.lParam);
                int height = HIWORD(msg.lParam);

                g_renderTarget.OnResize(width, height);
                laserRenderer.Resize(width, height);
            }
        }
        if (!running) break;

        // generate frame (replace with UDP receiver later)

        //LaserFrame lframe = GenerateRotatingCubeFrameInterpolated(angle);

        g_laserFrameGen.NewFrame();
		LaserColor color(0.0f, 180.0f, 1.0f, 1.0f, 1.0f, 1.0f);
        g_laserFrameGen.AddSquare(0.125f, 0.125f, 0.8f, color);
		float mouseXpos = (float(mouseX) / g_renderTarget.getScreenWidth());
		float mouseposY = (float(mouseY) / g_renderTarget.getScreenHeight());
        g_laserFrameGen.AddSquare((mouseXpos-0.5f) * 2.4f, (mouseposY-0.5f) * 2.4f, 0.5f, color);
        //Draw here...
		//Draw a square that changes size with mouse Y

        // TODO:
        // Create UDP packet from Frame


        // Instead, we simulate and render in our window
        g_GalvoSim.Simulate(dt);

        laserRenderer.Clear();
        laserRenderer.Accumulate(g_simFrame);

        auto* rt = g_renderTarget.GetD2DRenderTarget();
        rt->BeginDraw();
        rt->Clear(D2D1::ColorF(D2D1::ColorF::Black));

        laserRenderer.Present();

        rt->EndDraw();

        // simple frame cap ~60Hz (cooperative)
        Sleep(1);
    }

    return 0;
}