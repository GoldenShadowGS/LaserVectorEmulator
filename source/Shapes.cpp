#include <cmath>
#include "Shapes.h"
#include "LaserColor.h"
#include "Point2D.h"

static float constexpr DEG_TO_RAD = 0.01745329251994f;
constexpr float PI = 3.14159265358979323846f;
constexpr float PI2 = 2.0f * PI;

using LS = LaserFrameGenerator::LaserState;
using PS = LaserFrameGenerator::PointSharpness;
using ARC = LaserFrameGenerator::Arc;

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
    Point2D A1 = matrix.transformPoint(Point2D(m_r1, 0.0f));
    m_LaserGen.LineTo(A1, LS::OFF, PS::SHARP, color);
    Point2D A2 = matrix.transformPoint(Point2D(cosf(angle) * m_r1, sinf(angle) * m_r1));
    Point2D C = matrix.transformPoint(Point2D(0.0f, 0.0f));
    m_LaserGen.ArcTo(C, A2, LS::ON, PS::SHARP, color, ARC::COUNTERCLOCKWISE);
    m_LaserGen.LineTo(Point2D(0.0f, 0.0f), LS::OFF, PS::SHARP, color);
    m_LaserGen.LineTo(A2, LS::ON, PS::SHARP, color);

    Point2D B1 = matrix.transformPoint(m_c1 + Point2D(m_r2, 0.0f));
    m_LaserGen.LineTo(B1, LS::OFF, PS::SHARP, color);

    float theta = calculateTheta(angle);
    Point2D B2 = matrix.transformPoint(m_c1 + Point2D(cosf(theta) * m_r2, sinf(theta) * m_r2));
    Point2D L1 = matrix.transformPoint(A2 + (B2 - A2).Normalized() * m_barlength);
    m_LaserGen.ArcTo(m_c1, B2, LS::ON, PS::SHARP, color, ARC::COUNTERCLOCKWISE);
    m_LaserGen.LineTo(m_c1, LS::OFF, PS::SHARP, color);
    m_LaserGen.LineTo(B2, LS::ON, PS::SHARP, color);
    m_LaserGen.LineTo(A2, LS::OFF, PS::SHARP, color);
    m_LaserGen.LineTo(L1, LS::ON, PS::SHARP, color);
    int steps = 200;
    Point2D p0 = calculateL1(matrix, 0.0f);
    m_LaserGen.LineTo(p0, LS::OFF, PS::SMOOTH, color);
    std::vector<Point2D> linkagepoints;
    linkagepoints.reserve(steps);
    for (int i = 0; i <= steps; i++)
    {
        float t = (float(i) / float(steps) ) * PI2;
        linkagepoints.push_back(calculateL1(matrix, t));
    }
    m_LaserGen.DrawShape(linkagepoints, color);
}

Point2D Linkage::calculateL1(Mat3 matrix, float angle) const
{
    float theta = calculateTheta(angle);
    Point2D A2 = matrix.transformPoint(Point2D(cosf(angle) * m_r1, sinf(angle) * m_r1));
    Point2D B2 = matrix.transformPoint(m_c1 + Point2D(cosf(theta) * m_r2, sinf(theta) * m_r2));
    return matrix.transformPoint(A2 + (B2 - A2).Normalized() * m_barlength);
}

float Linkage::calculateTheta(float angle) const
{
    Point2D A2 = Point2D(cosf(angle) * m_r1, sinf(angle) * m_r1);
    Point2D vec = A2 - m_c1;
    float veclength = vec.Length();
    float cosAlpha = (m_r2 * m_r2 + veclength * veclength - m_linklength * m_linklength) / (2.0 * m_r2 * veclength);
    cosAlpha = std::clamp(cosAlpha, -1.0f, 1.0f);
    double alpha = acosf(cosAlpha);
    double base = atan2(vec.y, vec.x);
    float theta = base - alpha;
    return theta;
}
