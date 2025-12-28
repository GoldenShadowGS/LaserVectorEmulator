#pragma once
#include "LaserColor.h"
#include "LaserFrameGenerator.h"
#include "Matrix3x3.h"
#include "Point2D.h"

class ShapeGenerator
{
public:
	ShapeGenerator(LaserFrameGenerator& generator) : m_LaserGen(generator) {}
	void Square(Mat3 matrix, LaserColor color);
	void SmoothSquare(Point2D center, float size, LaserColor color);
	void ArcTest(Point2D center, float size, LaserColor color);
private:
	LaserFrameGenerator& m_LaserGen;
};

class Linkage
{
public:
	//c1 is the offset of 2nd center from the origin
	Linkage(LaserFrameGenerator& generator, Point2D c1, float r1, float r2, float linklength, float barlength) :
		m_LaserGen(generator),
		m_c1(c1),
		m_r1(r1),
		m_r2(r2),
		m_linklength(linklength),
		m_barlength(barlength)
	{}
	void DrawLinkage(Mat3 matrix, float angle, LaserColor color) const;
private:
	float calculateTheta(float angle) const;
	Point2D calculateL1(Mat3 matrix, float angle) const;
	LaserFrameGenerator& m_LaserGen;
	Point2D m_c1;
	float m_r1;
	float m_r2;
	float m_linklength;
	float m_barlength;
};