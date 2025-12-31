#define NTDDI_VERSION NTDDI_WIN7
#define _WIN32_WINNT _WIN32_WINNT_WIN7

#include <windows.h>
#include <windowsx.h>
#include <sal.h>
#include <chrono>
#include "GalvoSimulator.h"
#include "LaserFrameGenerator.h"
#include "FrameRenderer.h"
#include "LaserColor.h"
#include "Shapes.h"
#include "Point2D.h"
#include "Matrix3X3.h"
#include "InputManager.h"
#include "Object.h"
#include "Context.h"
#pragma comment(lib, "Comctl32.lib")

using Clock = std::chrono::high_resolution_clock;

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

using LS = LaserFrameGenerator::LaserState;
using PS = LaserFrameGenerator::PointSharpness;
using ARC = LaserFrameGenerator::Arc;

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

	float maxAngle = 35.0f;
	LaserFrameGenerator frameGenerator(0.9f, maxAngle);
    GalvoSimulator galvoSimulator(maxAngle);
	FrameRenderer frameRenderer(hwnd);
    ShapeGenerator  shapeGenerator(frameGenerator);
    
    // Input
    InputManager input;
    // Default keybinds
    input.Bind("Thrust", 'W');
    input.Bind("Brake", 'S');
    input.Bind("TurnLeft", 'A');
    input.Bind("TurnRight", 'D');
    input.Bind("Fire", VK_LBUTTON);
    input.Bind("MenuBack", VK_ESCAPE);

    constexpr LaserColor::RGB8 Red { 255,0,0 };
    constexpr LaserColor::RGB8 Green { 0,255,0 };
    constexpr LaserColor::RGB8 Blue { 255,255,255 };
    LaserColor colorredgreen(Red, Green);
    LaserColor colorbluegreen(Blue, Green);
	float angleRads = 0.0f;
	GameContext context(frameGenerator, input, shapeGenerator);

    context.m_ShipPool.Spawn(Ship { Mat3::Scale(1.0f, 1.0f), LaserColor(0.0f, 0.0f, 1.0f), Point2D(0.0f, 0.0f), Point2D(0.0f, 0.0f), 0.0f, 0.0f, 10, true });

    // After creating ships
    context.m_ShipPool.m_PlayerShipIndex = 0; // example
    Ship& playerShip = context.m_ShipPool.ships[context.m_ShipPool.m_PlayerShipIndex];
    playerShip.m_PlayerControlled = true;
    playerShip.BindControls(context);


    // message + render loop
    MSG msg;
    float simsteps_per_second = 30000.0f;
	float fps = 60.0f;
    float simsteps = simsteps_per_second / fps;
    float dt = 1.0f / simsteps;
	bool running = true;
    auto lastTime = Clock::now();
    while (running)
    {
        // Measure current time
        auto currentTime = Clock::now();
        std::chrono::duration<float> elapsed = currentTime - lastTime;
        float deltaT = elapsed.count(); // seconds since last frame
		context.SetDeltaTime(deltaT);
        lastTime = currentTime;

		// INPUT
        input.BeginFrame();
        input.Update(context);
        while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
        {
            if (msg.message == WM_QUIT) { running = false; break; }
            TranslateMessage(&msg);
            DispatchMessage(&msg);
            input.HandleEvent(msg);
            if (msg.message == WM_MOUSEMOVE)
            {
				input.SetMousePos(float(GET_X_LPARAM(msg.lParam)), float(GET_Y_LPARAM(msg.lParam)));
            }
            if (msg.message == WM_SIZE)
            {
                int width = LOWORD(msg.lParam);
                int height = HIWORD(msg.lParam);

                frameRenderer.OnResize(width, height);
				input.SetScreenSize(width, height);
            }
        }
        if (!running) break;

        // UPDATE
        context.UpdatePools();

        // Drawing
        frameGenerator.NewFrame();
        context.DrawPools();
        // Simulate galvo physics
        galvoSimulator.Simulate(frameGenerator.GetLaserFrame(), dt); 

		// Render
		frameRenderer.DrawFrame(galvoSimulator.GetSimFrame());
        input.EndFrame();
        Sleep(1);
    }

    return 0;
}
