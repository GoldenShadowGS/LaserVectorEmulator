#define NTDDI_VERSION NTDDI_WIN7
#define _WIN32_WINNT _WIN32_WINNT_WIN7

#include <windows.h>
#include <windowsx.h>
#include "GalvoSimulator.h"
#include "LaserFrameGenerator.h"
#include "FrameRenderer.h"
#include "LaserColor.h"
#include "Shapes.h"
#pragma comment(lib, "Comctl32.lib")


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

	LaserFrameGenerator frameGenerator(0.9f);
    GalvoSimulator galvoSimulator;
	FrameRenderer frameRenderer(hwnd);
    ShapeGenerator  shapeGenerator(frameGenerator);
    // message + render loop
    MSG msg;
    int mouseX = 0;
    int mouseY = 0;

    float simsteps_per_second = 30000.0f;
	float fps = 60.0f;
    float simsteps = simsteps_per_second / fps;
    float dt = 1.0f / simsteps;
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

                frameRenderer.OnResize(width, height);
            }
        }
        if (!running) break;

        frameGenerator.NewFrame();
        constexpr LaserColor::RGB8 Red{ 255,0,0 };
        constexpr LaserColor::RGB8 Green { 1,255,0 };
        constexpr LaserColor::RGB8 Blue { 0,0,255 };
        LaserColor color(Red, Green);
        float mouseXpos = (float(mouseX) / frameRenderer.getScreenWidth());
        float mouseposY = (float(mouseY) / frameRenderer.getScreenHeight());
        float oppositemouseXpos = ((frameRenderer.getScreenWidth() - float(mouseX)) / frameRenderer.getScreenWidth());
        frameGenerator.LineTo(Point2D(-0.8f, 0.0f), LaserFrameGenerator::LaserState::OFF, LaserFrameGenerator::PointSharpness::SHARP, color);
        frameGenerator.LineTo(Point2D(0.8f, 0.5f), LaserFrameGenerator::LaserState::ON, LaserFrameGenerator::PointSharpness::SHARP, color);
        shapeGenerator.CreateSquare(Point2D(0.0f, 0.0f), 0.5f, color);
        shapeGenerator.CreateSquare(Point2D((mouseXpos - 0.5f) * 2.4f, (mouseposY - 0.5f) * 2.4f), 0.5f, color);
        shapeGenerator.CreateSquare(Point2D((oppositemouseXpos - 0.5f) * 2.4f, (mouseposY - 0.5f) * 2.4f), 0.5f, color);

        galvoSimulator.Simulate(frameGenerator.GetLaserFrame(), dt); 

		frameRenderer.DrawFrame(galvoSimulator.GetSimFrame());

        Sleep(1);
    }

    return 0;
}
