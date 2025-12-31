#include "Context.h"
#include "LaserFrameGenerator.h"
#include "InputManager.h"
#include "Shapes.h"
#include "Object.h"
#include "Point2D.h"
#include "Matrix3x3.h"
#include "EventManager.h"

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
    m_BulletPool.DrawAll(*this);
}