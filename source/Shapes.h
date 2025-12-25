#pragma once
#include "LaserColor.h"
#include "LaserFrameGenerator.h"

class ShapeGenerator
{
public:
	ShapeGenerator(LaserFrameGenerator& generator) : m_LaserGen(generator) {}
	void CreateSquare(float xc, float yc, float size, LaserColor color);
private:
	LaserFrameGenerator& m_LaserGen;
};