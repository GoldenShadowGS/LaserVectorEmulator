#pragma once
//#include <vector>
//#include <memory>
#include <memory>
#include "Point2D.h"
#include "LaserColor.h"
#include "Matrix3x3.h"
#include "EventManager.h"
#include "Shapes.h"

class LaserFrameGenerator;
class InputManager;
class GameContext;
class ShapeGenerator;

struct Ship
{
    Mat3 m_ObjectMatrix;
    LaserColor m_color;
    Point2D m_Pos = { 0.0f, 0.0f };
    Point2D m_Vel = { 0.0f, 0.0f };
    float m_Angle = 0.0f;
    float m_AngVel = 0.0f; // angular velocity
    int m_HitPoints = 0;  // seconds remaining
	bool m_PlayerControlled = false;
    void Update(GameContext& context);
    void Draw(GameContext& context);
	void Shoot(GameContext& context);
    void BindControls(GameContext& context);
    Point2D ForwardVector() const
    {
        return Point2D(std::cos(m_Angle), std::sin(m_Angle));
	}
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
    Point2D m_Pos = { 0.0f, 0.0f };
    Point2D m_Vel = { 0.0f, 0.0f };
    float m_Angle = 0.0f;
	float m_AngVel = 0.0f; // angular velocity
    float m_Lifetime = 0.0f;  // seconds remaining
};

class BulletPool
{
public:
    BulletPool() {}
    static constexpr int MaxBullets = 256;
    //Bullet bullets[MaxBullets];
    std::unique_ptr<Bullet[]> bullets = std::make_unique<Bullet[]>(MaxBullets);
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
    void DrawAll(GameContext& context);
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
    Point2D m_Pos = { 0.0f, 0.0f };
    Point2D m_Vel = { 0.0f, 0.0f };
    float m_Angle = 0.0f;
    float m_AngVel = 0.0f; // angular velocity
    int m_HitPoints = 0;  // seconds remaining
	AsteroidSize m_Size = AsteroidSize::SMALL;
};

class AsteroidPool
{
public:
    static constexpr int MaxAsteroids = 256;
    //Asteroid asteroids[MaxAsteroids];
    std::unique_ptr<Asteroid[]> asteroids = std::make_unique<Asteroid[]>(MaxAsteroids);
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



