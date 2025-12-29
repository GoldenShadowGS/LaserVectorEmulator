#pragma once
//#include <vector>
//#include <memory>
#include <array>
#include "Point2D.h"
#include "LaserColor.h"
#include "Matrix3x3.h"

class LaserFrameGenerator;
class InputManager;
class GameContext;
class ShapeGenerator;

struct Ship
{
    Mat3 m_ObjectMatrix;
    LaserColor m_color;
    Point2D m_Pos;
    Point2D m_Vel;
    float m_Angle;
    float m_AngVel; // angular velocity
    int m_HitPoints;  // seconds remaining
	bool m_PlayerControlled;
    void Update(GameContext& context);
    void Draw(GameContext& context);
};

class ShipPool
{
public:
    static constexpr int MaxShips = 10;
    int m_PlayerShipIndex = -1;
    Ship ships[MaxShips];
    int activeCount = 0;

    void Spawn(const Ship& s)
    {
        if (activeCount >= MaxShips) return;
        ships[activeCount++] = s;
    }

    void Deactivate(int index)
    {
        if (index < 0 || index >= activeCount) return;
        ships[index] = ships[activeCount - 1];
        activeCount--;
    }

    void UpdateAll(GameContext& context)
    {
        for (int i = 0; i < activeCount; )
        {
            Ship& s = ships[i];
			s.Update(context);
            if (s.m_HitPoints <= 0)
                Deactivate(i); // swaps last active in
            else
                ++i; // only increment if we didn’t remove the bullet
        }
    }
    void DrawAll(GameContext& context)
    {
        for (int i = 0; i < activeCount;i++)
        {
            Ship& s = ships[i];
			s.Draw(context);
        }
    }
};

struct Bullet
{
    Mat3 m_ObjectMatrix;
    LaserColor m_color;
    Point2D m_Pos;
    Point2D m_Vel;
    float m_Angle;
	float m_AngVel; // angular velocity
    float m_Lifetime;  // seconds remaining
};

class BulletPool
{
public:
    BulletPool() {}
    static constexpr int MaxBullets = 256;
    Bullet bullets[MaxBullets];
    int activeCount = 0;

    void Spawn(const Bullet& b)
    {
        if (activeCount >= MaxBullets) return;
        bullets[activeCount++] = b;
    }

    void Deactivate(int index)
    {
        if (index < 0 || index >= activeCount) return;
        bullets[index] = bullets[activeCount - 1];
        activeCount--;
    }

    void UpdateAll(float dt)
    {
        for (int i = 0; i < activeCount; )
        {
            Bullet& b = bullets[i];
			b.m_Pos += b.m_Vel * dt;
			b.m_Angle += b.m_AngVel * dt;
            b.m_Lifetime -= dt;

            if (b.m_Lifetime <= 0)
            {
                Deactivate(i); // swaps last active in
            }
            else
            {
                ++i; // only increment if we didn’t remove the bullet
            }
        }
    }
};


struct Asteroid
{
    Asteroid() : m_color(LaserColor(0.0f, 0.0f, 1.0f)) {}
    enum class AsteroidSize
    {
        SMALL,
        MEDIUM,
        LARGE
    };
    Mat3 m_ObjectMatrix;
    LaserColor m_color;
    Point2D m_Pos;
    Point2D m_Vel;
    float m_Angle;
    float m_AngVel; // angular velocity
    int m_HitPoints;  // seconds remaining
	AsteroidSize m_Size;
};

class AsteroidPool
{
public:
    static constexpr int MaxAsteroids = 256;
    Asteroid asteroids[MaxAsteroids];
    int activeCount = 0;

    void Spawn(const Asteroid& a)
    {
        if (activeCount >= MaxAsteroids) return;
        asteroids[activeCount++] = a;
    }

    void Deactivate(int index)
    {
        if (index < 0 || index >= activeCount) return;
        asteroids[index] = asteroids[activeCount - 1];
        activeCount--;
    }

    void UpdateAll(float dt)
    {
        for (int i = 0; i < activeCount; )
        {
            Asteroid& a = asteroids[i];
            a.m_Pos += a.m_Vel * dt;
            a.m_Angle += a.m_AngVel * dt;

            if (a.m_HitPoints <= 0)
            {
                Deactivate(i); // swaps last active in
            }
            else
            {
                ++i; // only increment if we didn’t remove the bullet
            }
        }
    }
};



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

private:
	Mat3 m_WorldMatrix;
    Point2D m_MousePos;
    float m_deltaT;
};