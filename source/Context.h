#pragma once
//#include "Point2D.h"
//#include "Matrix3x3.h"
//#include "EventManager.h"
#include "Object.h"

//class LaserFrameGenerator;
//class InputManager;
//class GameContext;
//class ShapeGenerator;

class GameContext
{
public:
    GameContext(LaserFrameGenerator& laserGen, InputManager& inputManager, ShapeGenerator& shapeGen);
    LaserFrameGenerator& m_laserGen;
    InputManager& m_inputManager;
    ShapeGenerator& m_shapeGen;
    void SetWorldMatrix(const Mat3& matrix) { m_WorldMatrix = matrix; }
    const Mat3& GetWorldMatrix() const { return m_WorldMatrix; }
    void SetDeltaTime(float dt) { m_deltaT = dt; }
    float GetDeltaTime() const { return m_deltaT; }
    void SetMousePos(float mouseX, float mouseY) { m_MousePos = Point2D(mouseX, mouseY); }
    const Point2D& GetMousePos() const { return m_MousePos; }
    void UpdatePools();
    void DrawPools();

    BulletPool m_BulletPool;
    AsteroidPool m_AsteroidPool;
    ShipPool m_ShipPool;
    EventManager events;


private:
    Mat3 m_WorldMatrix;
    Point2D m_MousePos;
    float m_deltaT;
};