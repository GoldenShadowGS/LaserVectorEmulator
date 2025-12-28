#define NTDDI_VERSION NTDDI_WIN7
#define _WIN32_WINNT _WIN32_WINNT_WIN7

#include <windows.h>
#include <windowsx.h>
#include "GalvoSimulator.h"
#include "LaserFrameGenerator.h"
#include "FrameRenderer.h"
#include "LaserColor.h"
#include "Shapes.h"
#include "Point2D.h"
#include "Matrix3X3.h"
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
    // message + render loop
    MSG msg;
    int mouseX = 0;
    int mouseY = 0;
    //Linkage(LaserFrameGenerator & generator, Point2D c1, float r1, float r2, float linklength, float barlength) :
    Linkage linkage(frameGenerator, Point2D(0.1f, -0.3f), 0.2f, 0.3f, 0.4f, 0.8f);
    float simsteps_per_second = 30000.0f;
	float fps = 60.0f;
    float simsteps = simsteps_per_second / fps;
    float dt = 1.0f / simsteps;
	bool running = true;
    float angleRads = 0.0f;
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
        angleRads += 0.1f;
        frameGenerator.NewFrame();
        constexpr LaserColor::RGB8 Red{ 255,0,0 };
        constexpr LaserColor::RGB8 Green { 1,255,0 };
        constexpr LaserColor::RGB8 Blue { 0,0,255 };
        LaserColor colorredgreen(Red, Green);
        LaserColor colorbluegreen(Blue, Green);
        float mouseXpos = ((float(mouseX) / frameRenderer.getScreenWidth()) - 0.5f) * 2.6f;
        float mouseposY = ((float(mouseY) / frameRenderer.getScreenHeight()) - 0.5f) * 2.6f;
        float mouseangle = atan2f(mouseposY, mouseXpos);
        Mat3 spinmatrix = Mat3::Rotation(angleRads);
        Mat3 mousepositionMatrix = Mat3::Translation(mouseXpos, mouseposY);
        Mat3 scaling = Mat3::Scale(0.1f, 0.1f);
        Mat3 transform = mousepositionMatrix * spinmatrix * Mat3::Translation(0.1f, 0.1f) * scaling;
        float oppositemouseXpos = ((frameRenderer.getScreenWidth() - float(mouseX)) / frameRenderer.getScreenWidth());
        //shapeGenerator.Square(transform, color);
        //std::string debugMsg = "Debug message: Angle: " + std::to_string(mouseangle / 0.01745329251994f) + "\n";
        //OutputDebugStringA(debugMsg.c_str());
        Mat3 linkscaling = Mat3::Scale(1.0f, 1.0f);
        linkage.DrawLinkage(linkscaling, mouseangle, colorredgreen);
        //Point2D A1 = Point2D(0.35f, 0.0f);
        //frameGenerator.LineTo(A1, LS::OFF, PS::SHARP, colorbluegreen);
        //Point2D mousePos = Point2D(mouseXpos, mouseposY);
        //Point2D C = Point2D(0.0f, 0.0f);
        //frameGenerator.ArcTo(C, mousePos, LS::ON, PS::SHARP, colorredgreen, ARC::COUNTERCLOCKWISE);
        //frameGenerator.LineTo(Point2D(0.8f, 0.5f), LaserFrameGenerator::LaserState::ON, LaserFrameGenerator::PointSharpness::SHARP, color);
        //shapeGenerator.ArcTest(Point2D((mouseXpos - 0.5f) * 2.4f, (mouseposY - 0.5f) * 2.4f), 0.5f, color);
        //shapeGenerator.Square(Point2D((oppositemouseXpos - 0.5f) * 2.4f, (mouseposY - 0.5f) * 2.4f), 0.5f, color);
        //frameGenerator.LineTo(Point2D(0.0f, 0.0f), LaserFrameGenerator::LaserState::OFF, LaserFrameGenerator::PointSharpness::SHARP, color);

        galvoSimulator.Simulate(frameGenerator.GetLaserFrame(), dt); 

		frameRenderer.DrawFrame(galvoSimulator.GetSimFrame());

        Sleep(1);
    }

    return 0;
}
