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

#include <GL/glut.h>
#include "math.h"

#include <stdio.h>
#include <vector>
#include <assert.h>

#include "SoccerBallGL.h"

static double rotate_y = 0; 
static double rotate_x = 0;      

static std::vector<Hextagon> pentagons, hexagons;

static void Init (void)
{
    float position[] = { -5.0, 5.0, 5.0, 0.0 };
    float local_view[] = { 0.0 };

    glEnable (GL_DEPTH_TEST);
    glDepthFunc (GL_LESS);

    glLightfv (GL_LIGHT0, GL_POSITION, position);
    glLightModelfv (GL_LIGHT_MODEL_LOCAL_VIEWER, local_view);

    glFrontFace (GL_CW);
    glEnable (GL_LIGHTING);
    glEnable (GL_LIGHT0);
    glEnable (GL_AUTO_NORMAL);
    glEnable (GL_NORMALIZE);

    glClearColor (0.0, 0.65, 0.1, 1.0); // green
}

static void setPentColors ()
{
    float ambient[] = { 0.01175, 0.01175, 0.01175 };
    float diffuse[] = { 0.04136, 0.04136, 0.04136 };
    float specular[] = { 0.626959, 0.626959, 0.626959 };

    glMaterialfv (GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv (GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv (GL_FRONT, GL_SPECULAR, specular);
    glMaterialf (GL_FRONT, GL_SHININESS, 0.6*128.0);
}

static void setHexColors ()
{
    float ambient[] = { 1.2175, 1.2175, 1.2175 };
    float diffuse[] = { 0.7136, 0.7136, 1.2136 };
    float specular[] = { 0.626959, 0.626959, 0.626959 };

    glMaterialfv (GL_FRONT, GL_AMBIENT, ambient);
    glMaterialfv (GL_FRONT, GL_DIFFUSE, diffuse);
    glMaterialfv (GL_FRONT, GL_SPECULAR, specular);
    glMaterialf (GL_FRONT, GL_SHININESS, 0.7*128.0);
}

/////////////////////////////////////////////////////////////////////////////
//
//  recursive call.
//  when level > 0, split triangle into four smaller triangles and call this
//  function recursively for each of them.
//  norm is normal to "seam" line, which is used to "dent" the surface
//
/////////////////////////////////////////////////////////////////////////////

static void DisplayTriangle (int level, DPoint a, DPoint b, DPoint c, DPoint & norm)
{
    const int ADDED_DETAIL_ALONG_SEAMS = 1; // keep this number between [0 .. 2] inclusive.

    if (level <= 0)
    {
        const double R = 0.022; // this value controls how deep and thick seams are.

        double coef_a = 1, coef_b = 1, coef_c = 1;
        bool is_edge = false;

        double angle = a.getAngleAsin (norm);

        if (angle < R)
        {
            coef_a = 1 - R + sqrt (R * R - (angle - R) * (angle - R));

            is_edge = true;
        }

        angle = b.getAngleAsin (norm);

        if (angle < R)
        {
            coef_b = 1 - R + sqrt (R * R - (angle - R) * (angle - R));

            is_edge = true;
        }

        angle = c.getAngleAsin (norm);

        if (angle < R)
        {
            coef_c = 1 - R + sqrt (R * R - (angle - R) * (angle - R));

            is_edge = true;
        }

        if (is_edge && (level > -ADDED_DETAIL_ALONG_SEAMS))
        {
            DPoint ab = a.midpointTo (b);
            DPoint ac = a.midpointTo (c);
            DPoint bc = b.midpointTo (c);

            DisplayTriangle (level - 1, a, ab, ac, norm);
            DisplayTriangle (level - 1, b, ab, bc, norm);
            DisplayTriangle (level - 1, c, ac, bc, norm);
            DisplayTriangle (level - 1, ab, ac, bc, norm);
            return;
        }
        else // level < 0 or is_edge is false
        {
            if (level == -ADDED_DETAIL_ALONG_SEAMS)
            {
                a.MultiplyBy (coef_a);
                b.MultiplyBy (coef_b);
                c.MultiplyBy (coef_c);
            }

            DPoint norm = a.getNormal (b, c);

            glNormal3f (norm.x, norm.y, norm.z);

            glVertex3f ( a.x, a.y, a.z );

            glVertex3f (b.x, b.y, b.z);

            glVertex3f (c.x, c.y, c.z);
        }
    }
    else 
    {
        // create 4 smaller triangles.
        // compute median points on all three sides

        DPoint ab = a.midpointTo (b);
        DPoint ac = a.midpointTo (c);
        DPoint bc = b.midpointTo (c);

        DisplayTriangle (level - 1, a, ab, ac, norm);
        DisplayTriangle (level - 1, b, ab, bc, norm);
        DisplayTriangle (level - 1, c, ac, bc, norm);
        DisplayTriangle (level - 1, ab, ac, bc, norm);
    }
}

static void DisplayBall ()
{
    glClear (GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    glLoadIdentity ();

    glColorMaterial (GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);

    glRotatef (rotate_x, 1.0, 0.0, 0.0);
    glRotatef (rotate_y, 0.0, 1.0, 0.0);

    const int DETAIL_LEVEL = 5;

    setPentColors ();

    glBegin (GL_TRIANGLES);

    for (auto & pent : pentagons )
    {
        for (size_t i = 0; i < pent.ordered_vertices.size(); i++)
        {
            auto & a = pent.ordered_vertices[i];
            auto & b = pent.ordered_vertices[ (i + 1) % 5];

            DPoint seam_normal (0.0,0.0,0.0);

            seam_normal = seam_normal.getNormal (a, b);

            DisplayTriangle (DETAIL_LEVEL, pent.center, a, b, seam_normal);
        }
    } 

    setHexColors ();

    for (auto & pent : hexagons )
    {
        for (size_t i = 0; i < pent.ordered_vertices.size(); i++)
        {
            auto & a = pent.ordered_vertices [i];
            auto & b = pent.ordered_vertices [ (i + 1) % 6];

            DPoint seam_normal (0.0,0.0,0.0);

            seam_normal = seam_normal.getNormal (a, b);

            DisplayTriangle (DETAIL_LEVEL, pent.center, a, b, seam_normal);
        }
    }

    glEnd ();

    glutSwapBuffers ();  // glFlush is done implicitly inside this call.
}

// ----------------------------------------------------------
// specialKeys() Callback Function
// ----------------------------------------------------------
static void specialKeys( int key, int x, int y ) 
{ 
    //  Right arrow - increase rotation by 5 degree
    if (key == GLUT_KEY_RIGHT)
    {
        rotate_y += 5;
    }
 
    //  Left arrow - decrease rotation by 5 degree
    else if (key == GLUT_KEY_LEFT)
    {
        rotate_y -= 5;
    }
 
    else if (key == GLUT_KEY_UP)
    {
        rotate_x += 5;
    }
 
    else if (key == GLUT_KEY_DOWN)
    {
        rotate_x -= 5;
    }
 
    //  Request display update
    glutPostRedisplay();
}

static void Reshape(int w, int h)
{
    glViewport (0, 0, (GLint)w, (GLint)h);

    glMatrixMode (GL_PROJECTION);

    glLoadIdentity ();

    // this is to avoid stretching the image:

    float aspect = (float)w / (float)h;

    glOrtho(-2 * aspect, 2 * aspect, -2.0, 2.0, -1.0, 6.0);

    glMatrixMode (GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv);
    glutInitWindowSize(800, 800);

    GLenum type = GLUT_DEPTH | GLUT_RGB | GLUT_DOUBLE | GLUT_MULTISAMPLE;

    glutInitDisplayMode (type);

    glutCreateWindow("Soccer ball. Use keyboard arrow keys to rotate");

    Init ();

    SoccerBallGL soccerBall;

    soccerBall.Init (pentagons, hexagons);

    glutReshapeFunc (Reshape);

    glutDisplayFunc (DisplayBall);

    glutSpecialFunc (specialKeys);

    glutMainLoop ();

    return EXIT_SUCCESS;
}
