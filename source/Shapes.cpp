#include "Shapes.h"
#include "LaserColor.h"

void ShapeGenerator::CreateSquare(float centerX, float centerY, float size, LaserColor color)
{
    const float& max = m_LaserGen.GetMaxValue();
    float side = size * max;
    float perimeter = 4.0f * side;
    float x0 = (centerX - size / 2.0f) * max;
    float y0 = (centerY - size / 2.0f) * max;
    float x1 = (centerX + size / 2.0f) * max;
    float y1 = (centerY + size / 2.0f) * max;
    //blank to starting point
	LaserColor debugcolor(0.0f, 0.0f, 0.1f, 0.1f, 0.1f, 0.1f); // dim blue for blanking
    m_LaserGen.LineTo(x0, y0, true, debugcolor);
	//Laser ON Draw square
    m_LaserGen.LineTo(x1, y0, true, color);
    m_LaserGen.LineTo(x1, y1, true, color);
    m_LaserGen.LineTo(x0, y1, true, color);
    m_LaserGen.LineTo(x0, y0, true, color);
}
