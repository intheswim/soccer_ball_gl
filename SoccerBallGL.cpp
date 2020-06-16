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

#include <assert.h>
#include "SoccerBallGL.h"

DPoint * SoccerBallGL::initVertices ()
{
    DPoint * vertex = new DPoint[12];

	double alpha;
	double beta = atan (2.0);

	int i;

	vertex [0].x = 0;
	vertex [0].y = 1;
	vertex [0].z = 0;

	for (i = 1; i <= 5; i++)
	{
		alpha = 0.4 * M_PI * (i-1); 
		vertex[i].x = sin(beta) * cos(alpha);
		vertex[i].y = cos(beta);
		vertex[i].z = sin(beta) * sin(alpha);
	}

	for (i = 6; i < 12; i++)
	{
		vertex[i].x = -vertex[i-6].x;
		vertex[i].y = -vertex[i-6].y;
		vertex[i].z = -vertex[i-6].z;
	}	

    return vertex;    
}

DPoint * SoccerBallGL::initHexagonCenters (const DPoint * vertex) // result should be 20 points.
{
    DPoint * hex_centers = new DPoint [20];

    int cnt = 0;

    for (int i = 0; i < 12; i++)
    {
        auto & pt = vertex[i];

        for (int j = i + 1; j < 12; j++)
        {
            auto & pt2 = vertex[j];

            if (pt.SquaredDistance(pt2) < 1.2)
            {
                for (int k = j + 1; k < 12; k++)
                {
                    auto & pt3 = vertex[k];

                    if (pt.SquaredDistance(pt3) < 1.2 && pt2.SquaredDistance(pt3) < 1.2)
                    {
                        DPoint hex_center;
                        hex_center.x = vertex[i].x + vertex[j].x + vertex[k].x;
                        hex_center.y = vertex[i].y + vertex[j].y + vertex[k].y;
                        hex_center.z = vertex[i].z + vertex[j].z + vertex[k].z;

                        hex_center.Normalize();

                        hex_centers[cnt] = hex_center;
                        cnt++;
                    }
                }
            }
        }
    }

    assert (20 == cnt);

    return hex_centers;
}


double SoccerBallGL::SquaredDistance (const DPoint & pt1, const DPoint & pt2)
{
    return pt1.SquaredDistance (pt2);
}


int SoccerBallGL::findClosest (const DPoint & from, int start, const DPoint * points, int cnt, double min)
{
	double dist = 0;

	for (int i = start; i < cnt; i++)
	{
		dist = from.SquaredDistance (points[i]); 

		if (dist < min)	return i;
	}
	return -1;
}

void SoccerBallGL::FindIntermediatePoint (const DPoint & pt1, const DPoint & pt2, DPoint & out, double fraction)
{
	out.x = pt1.x + (pt2.x - pt1.x) * fraction;
	out.y = pt1.y + (pt2.y - pt1.y) * fraction;
	out.z = pt1.z + (pt2.z - pt1.z) * fraction;

	out.Normalize();
}

DPoint * SoccerBallGL::FindPentagonPoints (const DPoint *vertex)
{
    DPoint * pentagon_points = new DPoint[60];

	int cnt = 0;
	DPoint pent1, pent2;
		
	for (int i=0; i < 12; i++)
	{
			int closest = i;
			while (0 <= (closest = findClosest (vertex[i], closest + 1, vertex, 12, 1.2)))
			{
				FindIntermediatePoint (vertex[i], vertex[closest], pent1, 1/3.0);
				FindIntermediatePoint (vertex[i], vertex[closest], pent2, 2/3.0);

				if (cnt <= 58)
				{
					pentagon_points[cnt].x = pent1.x;
					pentagon_points[cnt].y = pent1.y;
					pentagon_points[cnt].z = pent1.z;

					cnt++;

					pentagon_points[cnt].x = pent2.x;
					pentagon_points[cnt].y = pent2.y;
					pentagon_points[cnt].z = pent2.z;

					cnt++;

				}
			} 
	}

    return pentagon_points;
}


void SoccerBallGL::initPentagons (const DPoint *vertex, const DPoint * pentagon_points, std::vector<Hextagon> & pentagons)
{
	DPoint pent_set [5];
	DPoint temp;

	for (int i = 0; i < 12; i++)
	{
		int closest = 0;
		int cnt = 0;
		while (0 <= (closest = findClosest (vertex[i], closest, pentagon_points, 60, 0.4)))
		{
			pent_set [cnt] = pentagon_points[closest]; 
			closest++;
			cnt++;
		}

		// safety net
		assert (cnt == 5) ;

		for (cnt = 0; cnt < 4; cnt++)
		{
			for (int j = cnt + 1; j < 5; )
			{
				if (SquaredDistance(pent_set [cnt], pent_set [j]) > 0.4) j++;					
				else 
				{
					if (j != cnt+1) // we reorder points to make sure the 5 form a polygon
					{
						temp = pent_set [cnt + 1];
						pent_set [cnt + 1] = pent_set [j];
						pent_set [j] = temp;
					}
					break;
				}
			}
		} 

        Hextagon p;
        p.center = vertex[i];

        for (int j = 0; j < 5; j++)
        {
            p.ordered_vertices.push_back (pent_set [j]);
        }

        pentagons.push_back (p);
    }

}

void SoccerBallGL::initHexagons (const DPoint * hex_centers, const DPoint * pentagon_points, std::vector<Hextagon> & hexagons)
{
	DPoint pent_set [6];
	DPoint temp;

	for (int i = 0; i < 20; i++)
	{
		int closest = 0;
		int cnt = 0;
		while (0 <= (closest = findClosest (hex_centers[i], closest, pentagon_points, 60, 0.4)))
		{
			pent_set [cnt] = pentagon_points[closest]; 
			closest++;
			cnt++;
		}

		assert (cnt == 6);

		for (cnt = 0; cnt < 5; cnt++)
		{
			for (int j = cnt + 1; j < 6; )
			{
                double d = SquaredDistance (pent_set [cnt], pent_set [j]);

				if (d > 0.2) j++;					
				else 
				{
					if (j != cnt + 1) // we reorder points to make sure the 6 form a polygon
					{
						temp = pent_set [cnt + 1];
						pent_set [cnt + 1] = pent_set [j];
						pent_set [j] = temp;
					}
					break;
				}
			}
		}

        Hextagon p;
        p.center = hex_centers[i];

        for (int j=0; j < 6; j++)
        {
            p.ordered_vertices.push_back (pent_set [j]);
        }

        hexagons.push_back (p);
    }

}

void SoccerBallGL::Init( std::vector<Hextagon> & pentagons, std::vector<Hextagon> & hexagons)
{
    DPoint * vertex = initVertices();

    DPoint * hex_centers = initHexagonCenters (vertex);

    DPoint * pentagon_points = FindPentagonPoints (vertex);

    initPentagons (vertex, pentagon_points, pentagons);

    initHexagons (hex_centers, pentagon_points, hexagons);

    delete[] hex_centers;
    delete[] vertex; 
    delete[] pentagon_points;
}
