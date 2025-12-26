#include "Shapes.h"
#include "LaserColor.h"

void ShapeGenerator::CreateSquare(Point2D center, float size, LaserColor color)
{
    float x0 = (center.x - size / 2.0f);
    float y0 = (center.y - size / 2.0f);
    float x1 = (center.x + size / 2.0f);
    float y1 = (center.y + size / 2.0f);
    //blank to starting point
	LaserColor debugcolor(180.0f, 180.0f, 0.8f, 0.1f, 0.8f, 0.1f); // dim blue for blanking
    m_LaserGen.LineTo(Point2D(x0, y0), LaserFrameGenerator::LaserState::OFF, LaserFrameGenerator::PointSharpness::SHARP, debugcolor);
	//Laser ON Draw square
    m_LaserGen.LineTo(Point2D(x1, y0), LaserFrameGenerator::LaserState::ON, LaserFrameGenerator::PointSharpness::SHARP, color);
    m_LaserGen.LineTo(Point2D(x1, y1), LaserFrameGenerator::LaserState::ON, LaserFrameGenerator::PointSharpness::SHARP, color);
    m_LaserGen.LineTo(Point2D(x0, y1), LaserFrameGenerator::LaserState::ON, LaserFrameGenerator::PointSharpness::SHARP, color);
    m_LaserGen.LineTo(Point2D(x0, y0), LaserFrameGenerator::LaserState::ON, LaserFrameGenerator::PointSharpness::SHARP, color);
}
