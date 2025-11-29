#include <windows.h>
#include <windowsx.h>
#include <d2d1.h>
#include "Helpers.h"
#include "FrameRenderer.h"
#include "GalvoSimulator.h"

std::atomic<bool> running = true;
GalvoSimulator simulator;
LaserFrame GeneratePointSequence(float x, float y);

void GalvoThread()
{
    // 30,000 Hz -> 33.333 microseconds per sample
    const float dt = 1.0f / 30000.0f;

    while (running)
    {
        simulator.Step(dt);
        Sleep(0); // yield (Windows can't sleep 33 µs, but it's okay)
    }
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

    // Create window
    HWND hwnd = CreateWindowEx(0, wc.lpszClassName, L"Laser Emulator (Direct2D)",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1000, 1000,
        nullptr, nullptr, hInstance, nullptr);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    std::thread galvo(GalvoThread);
    galvo.detach();


    // message + render loop
    FrameRenderer renderer(hwnd);
    GalvoSimulator galvoSim;
    MSG msg;
    bool running = true;
    int mouseX = 0;
    int mouseY = 0;
    LaserFrame prevframe;
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

        LaserFrame lframe = GeneratePointSequence(mouseX, mouseY);
        galvoSim.LoadFrame(lframe);

        // render frame
        renderer.RenderFrame(lframe, prevframe);
        prevframe = lframe;

        // simple frame cap ~60Hz (cooperative)
        Sleep(1);
    }

    return 0;
}