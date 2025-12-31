#include "InputManager.h"
#include <Windows.h>
#include <string>
#include "Context.h"

void InputManager::BeginFrame()
{
    prev = curr; // snapshot previous state
}

void InputManager::HandleEvent(const MSG& msg)
{
    switch (msg.message)
    {
    case WM_KEYDOWN: curr[msg.wParam] = true;  break;
    case WM_KEYUP:   curr[msg.wParam] = false; break;

    case WM_LBUTTONDOWN: curr[VK_LBUTTON] = true; break;
    case WM_LBUTTONUP:   curr[VK_LBUTTON] = false; break;
    case WM_RBUTTONDOWN: curr[VK_RBUTTON] = true; break;
    case WM_RBUTTONUP:   curr[VK_RBUTTON] = false; break;
    case WM_MBUTTONDOWN: curr[VK_MBUTTON] = true; break;
    case WM_MBUTTONUP:   curr[VK_MBUTTON] = false; break;
    case WM_XBUTTONDOWN:
        curr[(GET_XBUTTON_WPARAM(msg.wParam) == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2] = true;
        break;
    case WM_XBUTTONUP:
        curr[(GET_XBUTTON_WPARAM(msg.wParam) == XBUTTON1) ? VK_XBUTTON1 : VK_XBUTTON2] = false;
        break;
    }
}

void InputManager::Bind(const std::string& action, int vk)
{
    actionKeys[action] = vk;
}

void InputManager::Update(GameContext& context)
{
    for (auto& [name, key] : actionKeys)
    {
        bool isDown = GetAsyncKeyState(key) & 0x8000;

        states[name].previous = states[name].current;
        states[name].current = isDown;
    }
    for (int vk = 0; vk < 256; ++vk)
        curr[vk] = (GetAsyncKeyState(vk) & 0x8000) != 0;

    for (auto& [actionName, key] : actionKeys)
    {
        if (IsAction(actionName))
        {
            context.events.Emit(actionName);
        }
    }
}


void InputManager::SetMousePos(float x, float y)
{
    m_MousePos = Point2D(((float(x) / m_ScreenWidth) - 0.5f) * 2.6f, ((float(y) / m_ScreenHeight) - 0.5f) * 2.6f);
}

bool InputManager::IsAction(const std::string& name) const
{
    return states.at(name).current; // held
}

bool InputManager::WasPressed(const std::string& name) const
{
    return states.at(name).current && !states.at(name).previous; // single frame press
}

bool InputManager::WasReleased(const std::string& name) const
{
    return !states.at(name).current && states.at(name).previous; // single frame release
}
