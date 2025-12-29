#include "Object.h"
#include "InputManager.h"
#include "LaserFrameGenerator.h"
#include "LaserColor.h"
#include "Shapes.h" 


GameContext::GameContext(LaserFrameGenerator& laserGen, InputManager& inputManager, ShapeGenerator& shapeGen) :
    m_laserGen(laserGen),
    m_inputManager(inputManager),
    m_shapeGen(shapeGen),
    m_WorldMatrix(),
    m_MousePos(Point2D(0.0f, 0.0f)),
    m_deltaT(0.0f)
{

}

void GameContext::UpdatePools()
{
    m_BulletPool.UpdateAll(m_deltaT);
    m_AsteroidPool.UpdateAll(m_deltaT);
    m_ShipPool.UpdateAll(*this);
}
void GameContext::DrawPools()
{
    m_ShipPool.DrawAll(*this);
}

void Ship::Update(GameContext& context)
{

}
void Ship::Draw(GameContext& context)
{
    context.m_shapeGen.Square(m_ObjectMatrix, m_color);
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