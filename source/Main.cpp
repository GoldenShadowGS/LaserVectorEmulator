#define NTDDI_VERSION NTDDI_WIN7
#define _WIN32_WINNT _WIN32_WINNT_WIN7

#include <windows.h>
#include <commctrl.h>
#include <windowsx.h>
#include <d2d1.h>
#include "Helpers.h"
#include "FrameRenderer.h"
#include "GalvoSimulator.h"
#pragma comment(lib, "Comctl32.lib")

std::atomic<bool> running = true;
GalvoSimulator simulator;
LaserFrame GeneratePointSequence(float x, float y, float hue);

//void GalvoThread()
//{
//    // 30,000 Hz -> 33.333 microseconds per sample
//    const float dt = 1.0f / 300.0f;
//
//    while (running)
//    {
//        simulator.Step(dt);
//        Sleep(0); // yield (Windows can't sleep 33 µs, but it's okay)
//    }
//}

void UpdateHue(float& hue, float amount)
{
    hue += amount;              // increment
    if (hue >= 360.0f)        // wrap around
        hue -= 360.0f;
}

struct SliderControl
{
    HWND hwndSlider;        // Handle to the trackbar
    HWND hwndLabel;         // Optional: handle to the label
    float* pValue;          // Pointer to the variable to update
    float scale;            // How to scale integer slider to float value
    const wchar_t* labelText;  // Label text
    int minVal;             // Slider min
    int maxVal;             // Slider max

    void Create(HWND parent, HINSTANCE hInst, int x, int y, int width)
    {
        // Create the label
        hwndLabel = CreateWindow(L"STATIC", labelText, WS_CHILD | WS_VISIBLE,
            x, y, width, 20, parent, nullptr, hInst, nullptr);

        // Create the slider
        hwndSlider = CreateWindowEx(0, TRACKBAR_CLASS, L"",
            WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS,
            x, y + 20, width, 30, parent, nullptr, hInst, nullptr);

        SendMessage(hwndSlider, TBM_SETRANGE, TRUE, MAKELONG(minVal, maxVal));
        SendMessage(hwndSlider, TBM_SETPOS, TRUE, (minVal + maxVal) / 2);
    }

    void UpdateValue()
    {
        int pos = SendMessage(hwndSlider, TBM_GETPOS, 0, 0);
        if (pValue)
            *pValue = pos * scale;
    }
};

std::vector<SliderControl> sliders;


static LRESULT CALLBACK ControlWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg)
    {
    case WM_CREATE:
    {
        // Add sliders to the vector
        sliders.push_back({ nullptr, nullptr, &simulator.maxAngle, 1.0f, L"Max Angle", 10, 45 });
        sliders.push_back({ nullptr, nullptr, &simulator.maxSpeed, 0.5f, L"Max Speed", 1, 25 });
        sliders.push_back({ nullptr, nullptr, &simulator.damping, 1.0f, L"Damping", 10, 30 });
        sliders.push_back({ nullptr, nullptr, &simulator.stiffness, 100.0f, L"Stiffness", 1, 20 });

        // Create the controls
        int yPos = 10;
        for (auto& s : sliders)
        {
            s.Create(hwnd, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), 10, yPos, 260);
            yPos += 60; // space for label + slider
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


    // Create window
    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, L"Laser Emulator (Direct2D)",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1000, 1000,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

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


    //std::thread galvo(GalvoThread);
    //galvo.detach();


    // message + render loop
    FrameRenderer renderer(hwnd);
    MSG msg;
    //bool running = true;
    int mouseX = 0;
    int mouseY = 0;
	float hue = 0.0f;
    RenderFrame renderFrame;
    RenderFrame prevframe;
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

                renderer.OnResize(width, height);
            }
        }
        if (!running) break;

        // generate frame (replace with UDP receiver later)
        UpdateHue(hue, 1.0f);
        LaserFrame lframe = GeneratePointSequence(mouseX, mouseY, hue);
        simulator.LoadFrame(std::move(lframe));
        for(int i = 0; i < 1000; i++)
			simulator.Step(1.0f / 1000.0f);

        // render frame
        renderer.DrawFrame(simulator.getRenderFrame(), prevframe);
        prevframe = simulator.getRenderFrame();

        // simple frame cap ~60Hz (cooperative)
        Sleep(1);
    }

    return 0;
}