#include "Object.h"
#include "InputManager.h"
#include "LaserFrameGenerator.h"
#include "LaserColor.h"
#include "Shapes.h" 
#include "Context.h"




void Ship::Update(GameContext& context)
{
	m_Pos += m_Vel * context.GetDeltaTime();
	m_Angle += m_AngVel * context.GetDeltaTime();
}
void Ship::Draw(GameContext& context)
{
    Mat3 matrix = Mat3::Translation(m_Pos.x, m_Pos.y) * Mat3::Rotation(m_Angle) * Mat3::Scale(1.0f, 1.0f);
    context.m_shapeGen.Ship(matrix, m_color);
}

void Ship::Shoot(GameContext& context)
{
    Bullet b;
    b.m_Pos = m_Pos;
    b.m_Vel = { std::cos(m_Angle) * 5.0f, std::sin(m_Angle) * 5.0f };
    b.m_Lifetime = 2.0f;
    context.m_BulletPool.Spawn(b);
}

void Ship::BindControls(GameContext& context)
{
    context.events.Subscribe("TurnLeft", [&] () { m_AngVel -= 0.01f; });
    context.events.Subscribe("TurnRight", [&] () { m_AngVel += 0.01f; });
    context.events.Subscribe("Thrust", [&] () { m_Vel += ForwardVector() * 0.01f; });
    context.events.Subscribe("Brake", [&] () { m_Vel *= 0.95f; });
    context.events.Subscribe("Fire", [&] () { Shoot(context); });
}

void BulletPool::DrawAll(GameContext& context)
{
    for (int i = 0; i < activeCount; i++)
    {
        Bullet& b = bullets[i];
        ShapeGenerator& shapeGen = context.m_shapeGen;
        Mat3 matrix = Mat3::Translation(b.m_Pos.x, b.m_Pos.y) * Mat3::Rotation(b.m_Angle) * Mat3::Scale(0.01f, 0.01f);
        shapeGen.Square(matrix, b.m_color);
    }
}

//Ship::Ship(float scale, LaserColor color) :
//    m_color(color),
//    m_Pos(0.0f, 0.0f),
//    m_Angle(0.0f)
//{
//    m_ObjectMatrix = Mat3::Scale(scale, scale);
//}


//Bullet::Bullet(float scale, LaserColor color) :
//    m_color(color),
//    m_Pos(0.0f, 0.0f),
//    m_Angle(0.0f)
//{
//    m_ObjectMatrix = Mat3::Scale(scale, scale);
//}
//
//void Bullet::Update(GameContext& context)
//{
//    /* movement, thrust, wrap */
//}
//void Bullet::Draw(GameContext& context)
//{
//    context.m_shapeGen.Square(m_ObjectMatrix, m_color);
//    //Point2D tip = pos + (Point2D(cos(angle), sin(angle)) * 0.05f);
//    //gen.LineTo(tip, LaserColor::Red);
//}
//
//Asteroid::Asteroid(float scale, LaserColor color) :
//    m_color(color),
//    m_Pos(0.0f, 0.0f),
//    m_Angle(0.0f)
//{
//    m_ObjectMatrix = Mat3::Scale(scale, scale);
//}
//
//void Asteroid::Update(GameContext& context)
//{
//    /* movement, thrust, wrap */
//}
//void Asteroid::Draw(GameContext& context)
//{
//    context.m_shapeGen.Square(m_ObjectMatrix, m_color);
//    //Point2D tip = pos + (Point2D(cos(angle), sin(angle)) * 0.05f);
//    //gen.LineTo(tip, LaserColor::Red);
//}