#pragma once
#include <array>
#include <unordered_map>
#include <string>
#include <Windows.h>
#include "Point2D.h"

class GameContext;

class InputManager
{
public:
    void BeginFrame();              // call at start of frame
    void EndFrame() {}                // call at end of frame
    void HandleEvent(const MSG& msg);

    bool KeyDown(int vk) const { return  curr[vk]; } // held
    bool KeyPressed(int vk) const { return  curr[vk] && !prev[vk]; } // went down this frame
    bool KeyReleased(int vk) const { return !curr[vk] && prev[vk]; } // went up this frame

    // Action mapping for gameplay or UI
    bool IsAction(const std::string& name) const;
    bool WasPressed(const std::string& name) const;
    bool WasReleased(const std::string& name) const;
    void Bind(const std::string& action, int vk);
    void Update(GameContext& context);
    void SetMousePos(float x, float y);
	void SetScreenSize(int width, int height) { m_ScreenWidth = width; m_ScreenHeight = height; }
	Point2D GetMousePos() const { return m_MousePos; }

private:
    struct ActionState
    {
        bool current = false;
        bool previous = false;
    };
    std::array<bool, 256> curr {};
    std::array<bool, 256> prev {};
    //std::unordered_map<std::string, int> bindings;
    std::unordered_map<std::string, int> actionKeys;
    std::unordered_map<std::string, ActionState> states;
    Point2D m_MousePos;
    int m_ScreenWidth = 0;
	int m_ScreenHeight = 0;
};
