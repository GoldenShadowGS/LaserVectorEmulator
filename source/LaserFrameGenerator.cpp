//#include <cmath>
//#include <vector>
//#include <array>
#include <algorithm>
#include <cmath>
#include "LaserFrameGenerator.h"

void LaserFrameGenerator::BlankTo(float x, float y)
{
    float dx = x - m_lastX;
    float dy = y - m_lastY;
    float length = std::sqrt(dx * dx + dy * dy);
    size_t steps = static_cast<size_t>(length / m_pointSpacing * 0.5f);
    for (size_t i = 0; i < steps; i++)
    {
        float t = float(i) / float(steps);
        float ix = m_lastX + dx * t;
        float iy = m_lastY + dy * t;
        LaserFrameGenerator::LaserPoint p {};
        p.x = (int16_t)std::clamp(ix, -m_MaxValue, m_MaxValue);
        p.y = (int16_t)std::clamp(iy, -m_MaxValue, m_MaxValue);
        p.r = 0;
        p.g = 0;
        p.b = 255;
        p.flags = 0;
        m_Frame.push_back(p);
    }
    m_lastX = std::clamp(x, -m_MaxValue, m_MaxValue);
    m_lastY = std::clamp(y, -m_MaxValue, m_MaxValue);


    //LaserFrameGenerator::LaserPoint p {};
    //p.x = (int16_t)x;
    //p.y = (int16_t)y;
    //p.r = 128;
    //p.g = 128;
    //p.b = 0;
    //p.flags = 0;
    //m_Frame.push_back(p);
}

void LaserFrameGenerator::SolidLineTo(float x, float y, LaserColor c)
{
	float dx = x - m_lastX;
    float dy = y - m_lastY;
	float length = std::sqrt(dx * dx + dy * dy);
    size_t steps = std::max<size_t>(1, static_cast<size_t>(length / m_pointSpacing));
	float currentdistance = 0.0f;
    for (size_t i = 0; i <= steps; i++)
    {
        float t = float(i) / float(steps);
        float dxt = dx * t;
		float dyt = dy * t;
		currentdistance  = m_currentdistance + length * t;
        float color_t = currentdistance / m_totaldistance;
        float ix = m_lastX + dxt;
        float iy = m_lastY + dyt;
        LaserFrameGenerator::LaserPoint p {};
        p.x = (int16_t)std::clamp(ix, -m_MaxValue, m_MaxValue);
        p.y = (int16_t)std::clamp(iy, -m_MaxValue, m_MaxValue);
        auto color = c.getRGB(color_t);
        p.r = color.r;
        p.g = color.g;
        p.b = color.b;
        p.flags = 1;
        m_Frame.push_back(p);
	}
    m_currentdistance = currentdistance;
    m_lastX = std::clamp(x, -m_MaxValue, m_MaxValue);
    m_lastY = std::clamp(y, -m_MaxValue, m_MaxValue);
}


void LaserFrameGenerator::AddSquare(float xc, float yc, float size, LaserColor color)
{
    float side = size * m_MaxValue;
    float perimeter = 4.0f * side;
	float x0 = (xc - size / 2.0f) * m_MaxValue;
	float y0 = (yc - size / 2.0f) * m_MaxValue;
	float x1 = (xc + size / 2.0f) * m_MaxValue;
	float y1 = (yc + size / 2.0f) * m_MaxValue;
	m_totaldistance = perimeter;
	m_currentdistance = 0.0f;
	BlankTo(x0, y0);
    SolidLineTo(x1, y0, color);
    SolidLineTo(x1, y1, color);
    SolidLineTo(x0, y1, color);
    SolidLineTo(x0, y0, color);
}

void LaserFrameGenerator::ArcTo(float xc, float yc, float x, float y, LaserColor c0, LaserColor c1, bool ccw)
{

}




















//LaserFrame GeneratePointSequence(float x, float y, float hue)
//{
//    LaserFrame frame;
//
//    RGB color = HSVtoRGB(hue, 1.0f, 1.0f);
//    hue += 15.0f;
//    LaserPoint p {};
//    p.x = (int16_t)x;
//    p.y = (int16_t)y;
//    p.r = color.r;
//    p.g = color.g;
//    p.b = color.b;
//    p.flags = 1;
//    frame.push_back(p);
//
//
//    color = HSVtoRGB(hue, 1.0f, 1.0f);
//    hue += 15.0f;
//    p.x = (int16_t)x+ 30000;
//    p.y = (int16_t)y;
//    p.r = color.r;
//    p.g = color.g;
//    p.b = color.b;
//    p.flags = 1;
//    frame.push_back(p);
//
//    color = HSVtoRGB(hue, 1.0f, 1.0f);
//    hue += 15.0f;
//    p.x = (int16_t)x;
//    p.y = (int16_t)y+30000;
//    p.r = color.r;
//    p.g = color.g;
//    p.b = color.b;
//    p.flags = 1;
//    frame.push_back(p);
//
//    return frame;
//}

//LaserFrame GenerateSquare()
//{
//    LaserFrame frame;
//    int16_t max_value = int16_t(32767 * 0.8f);
//    LaserPoint p {};
//    p.x = -max_value;
//    p.y = -max_value;
//    p.r = 255;
//    p.g = 255;
//    p.b = 255;
//    p.flags = 1;
//    frame.push_back(p);
//
//    p.x = max_value;
//    p.y = -max_value;
//    p.r = 255;
//    p.g = 255;
//    p.b = 255;
//    p.flags = 1;
//    frame.push_back(p);
//
//    p.x = max_value;
//    p.y = max_value;
//    p.r = 255;
//    p.g = 255;
//    p.b = 255;
//    p.flags = 1;
//    frame.push_back(p);
//
//    p.x = -max_value;
//    p.y = max_value;
//    p.r = 255;
//    p.g = 255;
//    p.b = 255;
//    p.flags = 1;
//    frame.push_back(p);
//
//    return frame;
//}

//LaserFrame GenerateCubeFrame(float angle)
//{
//    LaserFrame frame;
//
//    // Define cube vertices
//    const float size = 100.0f;
//    struct Vec3 { float x, y, z; };
//    Vec3 vertices[] = {
//        {-size, -size, -size}, {size, -size, -size},
//        {size, size, -size}, {-size, size, -size},
//        {-size, -size, size}, {size, -size, size},
//        {size, size, size}, {-size, size, size}
//    };
//
//    // Define edges (pairs of vertex indices)
//    int edges[12][2] = {
//        {0,1},{1,2},{2,3},{3,0},
//        {4,5},{5,6},{6,7},{7,4},
//        {0,4},{1,5},{2,6},{3,7}
//    };
//
//    // Simple rotation around Y
//    float cosA = std::cos(angle);
//    float sinA = std::sin(angle);
//
//    struct Vec2 { float x, y; };
//    Vec2 proj[8] {};
//
//    for (int i = 0; i < 8; i++)
//    {
//        float x = vertices[i].x * cosA - vertices[i].z * sinA;
//        float z = vertices[i].x * sinA + vertices[i].z * cosA + 400; // simple perspective
//        float y = vertices[i].y;
//        proj[i].x = x * 200 / z + 400;  // scale + center
//        proj[i].y = y * 200 / z + 300;
//    }
//    static float hue = 0.0f;
//    // Convert edges to LaserPoints
//    for (int ei = 0; ei < 12; ei++)
//    {
//        const auto& a = proj[edges[ei][0]];
//        const auto& b = proj[edges[ei][1]];
//        RGB color = HSVtoRGB(hue, 1.0f, 1.0f);
//        hue += 100.0f;
//        LaserPoint p {};
//        p.x = (int16_t)a.x;
//        p.y = (int16_t)a.y;
//        p.r = color.r;
//        p.g = color.g;
//        p.b = color.b;
//        p.flags = 1;
//        
//        frame.push_back(p);
//
//        p.x = (int16_t)b.x;
//        p.y = (int16_t)b.y;
//        frame.push_back(p);
//    }
//
//    return frame;
//}

//LaserFrame GenerateCubeFrame(float angle, float size)
//{
//    LaserFrame frame;
//
//    // Cube vertices
//    struct Vec3 { float x, y, z; };
//    Vec3 vertices[] = {
//        {-size, -size, -size}, { size, -size, -size},
//        { size,  size, -size}, {-size,  size, -size},
//        {-size, -size,  size}, { size, -size,  size},
//        { size,  size,  size}, {-size,  size,  size}
//    };
//
//    // Cube edges (vertex indices)
//    int edges[12][2] = {
//        {0,1},{1,2},{2,3},{3,0},
//        {4,5},{5,6},{6,7},{7,4},
//        {0,4},{1,5},{2,6},{3,7}
//    };
//
//    // Rotation around Y
//    float cosA = std::cos(angle);
//    float sinA = std::sin(angle);
//
//    struct Vec2 { float x, y; };
//    Vec2 proj[8];
//
//    // Perspective camera distance
//    float camDist = size * 4.0f; // adjust 4.0f to control FOV
//
//    // Project vertices
//    for (int i = 0; i < 8; i++)
//    {
//        float x = vertices[i].x * cosA - vertices[i].z * sinA;
//        float z = vertices[i].x * sinA + vertices[i].z * cosA + camDist;
//        float y = vertices[i].y;
//
//        proj[i].x = x / z;
//        proj[i].y = y / z;
//    }
//
//    // Find bounding box
//    float minX = +FLT_MAX, maxX = -FLT_MAX;
//    float minY = +FLT_MAX, maxY = -FLT_MAX;
//    for (int i = 0; i < 8; i++)
//    {
//        minX = std::min(minX, proj[i].x);
//        maxX = std::max(maxX, proj[i].x);
//        minY = std::min(minY, proj[i].y);
//        maxY = std::max(maxY, proj[i].y);
//    }
//
//    // Normalize to -1..1
//    for (int i = 0; i < 8; i++)
//    {
//        proj[i].x = 2.0f * (proj[i].x - minX) / (maxX - minX) - 1.0f;
//        proj[i].y = 2.0f * (proj[i].y - minY) / (maxY - minY) - 1.0f;
//    }
//
//    // Convert edges to LaserPoints
//    for (int ei = 0; ei < 12; ei++)
//    {
//        const auto& a = proj[edges[ei][0]];
//        const auto& b = proj[edges[ei][1]];
//
//		const float maxsize = (32767 * 0.8f);
//        LaserPoint p {};
//        p.x = (int16_t)(a.x * maxsize);
//        p.y = (int16_t)(a.y * maxsize);
//        p.r = 0; p.g = 255; p.b = 192; p.flags = 1;
//        frame.push_back(p);
//
//        p.x = (int16_t)(b.x * maxsize);
//        p.y = (int16_t)(b.y * maxsize);
//        frame.push_back(p);
//    }
//
//    return frame;
//}


// //--------------------------- 3D cube generator ------------------------------
// //Returns LaserPoint vector using int16 XY coordinates. Adds small blanked jumps.
//LaserFrame GenerateRotatingCubeFrameInterpolated(float t)
//{
//    // base cube vertices (size)
//    const float half = 1.0f;
//    std::array<std::array<float, 3>, 8> verts = { {
//        {-half,-half,-half},{ half,-half,-half},{ half, half,-half},{-half, half,-half},
//        {-half,-half, half},{ half,-half, half},{ half, half, half},{-half, half, half}
//    } };
//
//    // rotations (Y and X)
//    float cy = std::cos(t); float sy = std::sin(t);
//    float cx = std::cos(t * 0.6f); float sx = std::sin(t * 0.6f);
//
//    auto rotate = [&] (const std::array<float, 3>& p) {
//        // yaw around Y
//        float x1 = p[0] * cy - p[2] * sy;
//        float z1 = p[0] * sy + p[2] * cy;
//        float y1 = p[1];
//        // pitch around X
//        float y2 = y1 * cx - z1 * sx;
//        float z2 = y1 * sx + z1 * cx;
//        return std::array<float, 3>{ x1, y2, z2 };
//        };
//
//    // project to 2D
//    std::vector<std::pair<float, float>> proj;
//    proj.reserve(8);
//    const float focal = 1.2f;
//    for (auto& v : verts)
//    {
//        auto r = rotate(v);
//        float z = r[2] + 2.8f; // shift forward so z>0
//        float px = (r[0] * focal) / z;
//        float py = (r[1] * focal) / z;
//        proj.push_back({ px, py });
//    }
//
//    // cube edges
//    const int edges[12][2] = {
//        {0,1},{1,2},{2,3},{3,0},
//        {4,5},{5,6},{6,7},{7,4},
//        {0,4},{1,5},{2,6},{3,7}
//    };
//
//    LaserFrame frame;
//    frame.reserve(size_t(12) * 64);
//
//    // For each edge, interpolate 12-32 points
//    for (int ei = 0; ei < 12; ++ei)
//    {
//        const auto& a = proj[edges[ei][0]];
//        const auto& b = proj[edges[ei][1]];
//        // pick interpolation count proportional to projected length
//        float dx = b.first - a.first;
//        float dy = b.second - a.second;
//        float len = std::sqrt(dx * dx + dy * dy);
//        int samples = (int)clampf(16.0f + len * 80.0f, 12.0f, 80.0f);
//        for (int s = 0; s <= samples; ++s)
//        {
//            float u = float(s) / float(samples);
//            float nx = a.first * (1.0f - u) + b.first * u;
//            float ny = a.second * (1.0f - u) + b.second * u;
//            // map from approx normalized (-1..1) to int16 range
//            float fx = clampf(nx, -1.0f, 1.0f) * 32767.0f;
//            float fy = clampf(ny, -1.0f, 1.0f) * 32767.0f;
//            LaserPoint p { (int16_t)fx, (int16_t)fy, 0, 255, 192, 1 };
//            frame.push_back(p);
//        }
//        // blank jump between edges
//        LaserPoint blank {};
//        blank.x = 0; blank.y = 0; blank.r = blank.g = blank.b = 0; blank.flags = 0;
//        frame.push_back(blank);
//    }
//
//    return frame;
//}