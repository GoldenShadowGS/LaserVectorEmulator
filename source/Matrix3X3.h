#pragma once
#include <cmath>
#include "Point2D.h"



class Mat3
{
public:
    float m[3][3];

    // --- Constructors ---
    Mat3() { setIdentity(); }

    static Mat3 Identity()
    {
        Mat3 r;
        r.setIdentity();
        return r;
    }

    void setIdentity()
    {
        *this = Mat3 {
            {1,0,0},
            {0,1,0},
            {0,0,1}
        };
    }

    Mat3(std::initializer_list<std::initializer_list<float>> values)
    {
        int r = 0, c = 0;
        for (auto& row : values)
        {
            c = 0;
            for (auto& val : row)
            {
                m[r][c++] = val;
            }
            r++;
        }
    }

    // --- Factory Methods ---
    static Mat3 Translation(float tx, float ty)
    {
        return Mat3 {
            {1, 0, tx},
            {0, 1, ty},
            {0, 0, 1}
        };
    }

    static Mat3 Rotation(float radians)
    {
        float c = std::cos(radians);
        float s = std::sin(radians);

        return Mat3 {
            {c, -s, 0},
            {s,  c, 0},
            {0,  0, 1}
        };
    }

    static Mat3 Scale(float sx, float sy)
    {
        return Mat3 {
            {sx, 0, 0},
            {0, sy, 0},
            {0, 0, 1}
        };
    }

    // --- Matrix Multiplication ---
    Mat3 operator*(const Mat3& o) const
    {
        Mat3 r;
        for (int row = 0; row < 3; ++row)
            for (int col = 0; col < 3; ++col)
                r.m[row][col] = m[row][0] * o.m[0][col]
                + m[row][1] * o.m[1][col]
                + m[row][2] * o.m[2][col];
        return r;
    }

    // --- Transform a Vec2 ---
    Point2D transformPoint(const Point2D& v) const
    {
        return {
            v.x * m[0][0] + v.y * m[0][1] + m[0][2],
            v.x * m[1][0] + v.y * m[1][1] + m[1][2]
        };
    }
};