/* Soccerball with OpenGL, Copyright (c) 2020 Yuriy Yakimenko
 *
 * Permission to use, copy, modify, distribute, and sell this software and its
 * documentation for any purpose is hereby granted without fee, provided that
 * the above copyright notice appear in all copies and that both that
 * copyright notice and this permission notice appear in supporting
 * documentation.  No representations are made about the suitability of this
 * software for any purpose.  It is provided "as is" without express or 
 * implied warranty.
 */

#pragma once

#include "math.h"
#include <vector>

class DPoint
{
	public:
	double x,y,z;

	public:

	DPoint () : x(0), y(0), z(0) { }

	DPoint (double _x, double _y, double _z)
	{
		x = _x; 
		y = _y;
		z = _z;
	}
	
	void Copy (const DPoint & point)
	{
		x = point.x;
		y = point.y;
		z = point.z;
	}  

	void Assign (double _x, double _y, double _z)
	{
		x = _x; 
		y = _y;
		z = _z;
	}

	void Normalize ()
	{
		double k = 1.0 / sqrt(x*x + y*y + z*z);
		x = x * k;
		y = y * k;
		z = z * k;
	}

    void MultiplyBy (double coef)
    {
        x *= coef;
        y *= coef;
        z *= coef;
    }

    DPoint getNormal (DPoint & a, DPoint & b) const 
    {
        double a1 = this->x - a.x;
        double a2 = this->y - a.y;
        double a3 = this->z - a.z;

        double b1 = this->x - b.x;
        double b2 = this->y - b.y;
        double b3 = this->z - b.z;

        DPoint ret ;

        ret.x = a2 * b3 - a3 * b2 ;
        ret.y = a3 * b1 - a1 * b3 ;
        ret.z = a1 * b2 - a2 * b1;

        ret.Normalize();

        if (x * ret.x + y * ret.y + z * ret.z < 0)
        {
            ret.x = -ret.x;
            ret.y = -ret.y;
            ret.z = -ret.z;
        }

        return ret;
    }

    DPoint midpointTo (const DPoint & other) const
    {
        DPoint pt (x + other.x, y + other.y, z + other.z);

        pt.Normalize();

        return pt;
    }

    double getAngleAsin (const DPoint & other) const 
    {
        double v = abs (x * other.x + y * other.y + z * other.z);

        return asin (v);
    }

    double SquaredDistance (const DPoint & other) const
    {
	    return	(x - other.x) * (x - other.x) +
				(y - other.y) * (y - other.y) +
				(z - other.z) * (z - other.z) ;
    }

};

//////////////////////////////////////////////////////////////////////////////////////////

struct Hextagon // pentagon or hexagon 
{
    DPoint center;
    std::vector<DPoint> ordered_vertices;
};

class SoccerBallGL 
{
    private:

    static double SquaredDistance (const DPoint & pt1, const DPoint & pt2);
    static DPoint * initVertices ();

    static DPoint * initHexagonCenters (const DPoint *);

    static int findClosest (const DPoint & from, int start, const DPoint * points, int cnt, double min);
    static void FindIntermediatePoint (const DPoint & pt1, const DPoint & pt2, DPoint & out, double fraction);

    static DPoint * FindPentagonPoints (const DPoint * vertex);

    void initPentagons (const DPoint *vertex, const DPoint * pentagon_points, std::vector<Hextagon> & pentagons);

    void initHexagons (const DPoint *hex_centers, const DPoint * pentagon_points, std::vector<Hextagon> & hexagons);

    public:

    void Init ( std::vector<Hextagon> & pentagons, std::vector<Hextagon> & hexagons );

};
