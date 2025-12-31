#include <cmath>
#include <vector>
#include <algorithm>
#include "Shapes.h"
#include "LaserColor.h"
#include "Point2D.h"
#include "LaserFrameGenerator.h"
#include "Matrix3X3.h"

static float constexpr DEG_TO_RAD = 0.01745329251994f;
constexpr float PI = 3.14159265358979323846f;
constexpr float PI2 = 2.0f * PI;

using LS = LaserFrameGenerator::LaserState;
using PS = LaserFrameGenerator::PointSharpness;
using ARC = LaserFrameGenerator::Arc;

Linkage::Linkage(LaserFrameGenerator& generator, Point2D c1, float r1, float r2, float linklength, float barlength) :
    m_LaserGen(generator),
    m_c1(c1),
    m_r1(r1),
    m_r2(r2),
    m_linklength(linklength),
    m_barlength(barlength)
{
    int steps = 360;
    m_linkagepoints.reserve(steps);
    for (int i = 0; i <= steps; i++)
    {
        float t = (float(i) / float(steps)) * PI2;
        m_linkagepoints.push_back(calculateL1(t));
    }
}

void ShapeGenerator::Square(Mat3 matrix, LaserColor color)
{
    float x0 = -1.0f;
    float y0 = -1.0f;
    float x1 = 1.0f;
    float y1 = 1.0f;
    Point2D p0 = matrix.transformPoint(Point2D(x1, y0));
    Point2D p1 = matrix.transformPoint(Point2D(x1, y1));
    Point2D p2 = matrix.transformPoint(Point2D(x0, y1));
    Point2D p3 = matrix.transformPoint(Point2D(x0, y0));
    //blank to starting point
	LaserColor debugcolor(180.0f, 180.0f, 0.8f, 0.1f, 0.8f, 0.1f); // dim blue for blanking
    m_LaserGen.LineTo(p0, LS::OFF, PS::SHARP, debugcolor);
	//Laser ON Draw square
    m_LaserGen.LineTo(p1, LS::ON, PS::SHARP, color);
    m_LaserGen.LineTo(p2, LS::ON, PS::SHARP, color);
    m_LaserGen.LineTo(p3, LS::ON, PS::SHARP, color);
    m_LaserGen.LineTo(p0, LS::ON, PS::SHARP, color);
}

void ShapeGenerator::Ship(Mat3 matrix, LaserColor color)
{
    Point2D shiparray[] = {
        { -0.0497f, -0.0344f },
        { -0.0971f, -0.0657f },
        { 0.0972f, 0.0f },
        { -0.0971f, 0.0657 },
        { -0.0497f, 0.0329 } };
    Point2D transformedarray[5];
    for (int i = 0; i < 5; i++)
    {
        transformedarray[i] = matrix.transformPoint(shiparray[i]);
    }
    m_LaserGen.LineTo(transformedarray[0], LS::OFF, PS::SHARP, color);
    for (int i = 1; i < 5; i++)
    {
        m_LaserGen.LineTo(transformedarray[i], LS::ON, PS::SHARP, color);
    }
    m_LaserGen.LineTo(transformedarray[0], LS::ON, PS::SHARP, color);
}

void ShapeGenerator::SmoothSquare(Point2D center, float size, LaserColor color)
{
    float x0 = (center.x - size / 2.0f);
    float y0 = (center.y - size / 2.0f);
    float x1 = (center.x + size / 2.0f);
    float y1 = (center.y + size / 2.0f);
    //blank to starting point
    LaserColor debugcolor(180.0f, 180.0f, 0.8f, 0.1f, 0.8f, 0.1f); // dim blue for blanking
    m_LaserGen.LineTo(Point2D(x0, y0), LS::OFF, PS::SHARP, debugcolor);
    //Laser ON Draw square
    m_LaserGen.LineTo(Point2D(x1, y0), LS::ON, PS::SMOOTH, color);
    m_LaserGen.LineTo(Point2D(x1, y1), LS::ON, PS::SMOOTH, color);
    m_LaserGen.LineTo(Point2D(x0, y1), LS::ON, PS::SMOOTH, color);
    m_LaserGen.LineTo(Point2D(x0, y0), LS::ON, PS::SHARP, color);
}

void ShapeGenerator::ArcTest(Point2D center, float size, LaserColor color)
{
    float x0 = (center.x - size / 2.0f);
    float y0 = (center.y - size / 2.0f);
    float x1 = (center.x + size / 2.0f);
    float y1 = (center.y + size / 2.0f);
    //blank to starting point
    LaserColor debugcolor(180.0f, 180.0f, 0.8f, 0.1f, 0.8f, 0.1f); // dim blue for blanking
    m_LaserGen.LineTo(Point2D(x0, y0), LS::OFF, PS::SHARP, debugcolor);
    //Laser ON Draw square
    m_LaserGen.LineTo(Point2D(x1, y0), LS::ON, PS::SHARP, color);
	Point2D centerArc1 = Point2D(x1, y0 + (y1 - y0) / 2.0f);
    m_LaserGen.ArcTo(centerArc1, Point2D(x1, y1), LS::ON, PS::SHARP, color, LaserFrameGenerator::Arc::CLOCKWISE);
	Point2D centerArc2 = Point2D(x0 + (x1 - x0) / 2.0f, y1);
    m_LaserGen.ArcTo(centerArc2, Point2D(x0, y1), LS::ON, PS::SHARP, color, LaserFrameGenerator::Arc::CLOCKWISE);
    m_LaserGen.LineTo(Point2D(x0, y1), LS::ON, PS::SHARP, color);
    m_LaserGen.LineTo(Point2D(x0, y0), LS::ON, PS::SHARP, color);
}

void Linkage::DrawLinkage(Mat3 matrix, float angle, LaserColor color) const
{
    Point2D A1 = Point2D(m_r1, 0.0f);
    Point2D A2 = Point2D(cosf(angle) * m_r1, sinf(angle) * m_r1);
    Point2D C0 = Point2D(0.0f, 0.0f);
    Point2D C1 = m_c1;
    Point2D B1 = m_c1 + Point2D(m_r2, 0.0f);
    float theta = calculateTheta(angle);
    Point2D B2 = C1 + Point2D(cosf(theta) * m_r2, sinf(theta) * m_r2);
    Point2D L1 = A2 + (B2 - A2).Normalized() * m_barlength;

    Point2D tA1 = matrix.transformPoint(A1);
    Point2D tA2 = matrix.transformPoint(A2);
    Point2D tC0 = matrix.transformPoint(C0);
    Point2D tC1 = matrix.transformPoint(C1);
    Point2D tB1 = matrix.transformPoint(B1);
    Point2D tB2 = matrix.transformPoint(B2);
    Point2D tL1 = matrix.transformPoint(L1);

    m_LaserGen.LineTo(tA1, LS::OFF, PS::SHARP, color);
    m_LaserGen.ArcTo(tC0, tA2, LS::ON, PS::SHARP, color, ARC::COUNTERCLOCKWISE);
    m_LaserGen.LineTo(tC0, LS::OFF, PS::SHARP, color);
    m_LaserGen.LineTo(tA2, LS::ON, PS::SHARP, color);
    m_LaserGen.LineTo(tB1, LS::OFF, PS::SHARP, color);
    m_LaserGen.ArcTo(tC1, tB2, LS::ON, PS::SHARP, color, ARC::COUNTERCLOCKWISE);
    m_LaserGen.LineTo(tC1, LS::OFF, PS::SHARP, color);
    m_LaserGen.LineTo(tB2, LS::ON, PS::SHARP, color);
    m_LaserGen.LineTo(tA2, LS::OFF, PS::SHARP, color);
    m_LaserGen.LineTo(tL1, LS::ON, PS::SHARP, color);

    Point2D p0 = matrix.transformPoint(m_linkagepoints.at(0));
    m_LaserGen.LineTo(p0, LS::OFF, PS::SHARP, color);
    std::vector<Point2D> transformedpoints;
    transformedpoints.reserve(m_linkagepoints.size());
    for (int i = 0; i < m_linkagepoints.size(); i++)
    {
        transformedpoints.push_back(matrix.transformPoint(m_linkagepoints.at(i)));
    }
    float lt = (angle + PI2) / PI2;
    m_LaserGen.DrawShape(transformedpoints, lt, color);
}

Point2D Linkage::calculateL1(float angle) const
{
    float theta = calculateTheta(angle);
    Point2D A2 = Point2D(cosf(angle) * m_r1, sinf(angle) * m_r1);
    Point2D B2 = m_c1 + Point2D(cosf(theta) * m_r2, sinf(theta) * m_r2);
    return A2 + (B2 - A2).Normalized() * m_barlength;
}

float Linkage::calculateTheta(float angle) const
{
    Point2D A2 = Point2D(cosf(angle) * m_r1, sinf(angle) * m_r1);
    Point2D vec = A2 - m_c1;
    float veclength = vec.Length();
    float cosAlpha = (m_r2 * m_r2 + veclength * veclength - m_linklength * m_linklength) / (2.0f * m_r2 * veclength);
    cosAlpha = std::clamp(cosAlpha, -1.0f, 1.0f);
    float alpha = acosf(cosAlpha);
    float base = atan2(vec.y, vec.x);
    float theta = base - alpha;
    return theta;
}
