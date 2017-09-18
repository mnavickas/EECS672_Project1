// ModelView.h - a basic combined Model and View for OpenGL

#ifndef MODELVIEW_H
#define MODELVIEW_H

#include "ShaderIF.h"

#include <string>

#ifdef __APPLE_CC__
#include "GLFW/glfw3.h"
#else
#include <GL/gl.h>
#endif

typedef float vec2[2];
typedef float vec3[3];

class ModelView
{

public:
	// NOTE: You will likely want to modify the ModelView constructor to
	//       take additional parameters.
	ModelView
		(
			ShaderIF* sIF,
			float a[4],
			float b[4],
			float t[3]
		);

	virtual ~ModelView();

	// xyzLimits: {mcXmin, mcXmax, mcYmin, mcYmax, mcZmin, mcZmax}
	void getMCBoundingBox(double* xyzLimits) const;
	bool handleCommand(unsigned char anASCIIChar, double ldsX, double ldsY);
	void render() const;

	// Viewing controls common to 2D and 3D:
	static void setAspectRatioPreservationEnabled(bool b)
		{ aspectRatioPreservationEnabled = b; }
	static void setMCRegionOfInterest(double xyz[6]);

private:
	// TODO: VAO(s), VBO(s), and other relevant INSTANCE variables

	ShaderIF* shaderIF;
	GLuint vao[1];
	GLuint vbo[1];
	float* paramA;
	float* paramB;
	float* paramT;
	static vec3 colors[5];
	static int colorIndex;

	// TODO: add uniform and attribute variable location CLASS variables

	// Routines for computing parameters necessary to map from arbitrary
	// model coordinate ranges into OpenGL's -1..+1 Logical Device Space.
	// 1. linearMap determines the scale and translate parameters needed in
	//    order to map a value, f (fromMin <= f <= fromMax) to its corresponding
	//    value, t (toMin <= t <= toMax). Specifically: t = scale*f + trans.
	static void linearMap(double fromMin, double fromMax,
		double toMin, double toMax, double& scale, double& trans);
	// 2. matchAspectRatio modifies (xmin,xmax) OR (ymin,ymax) to center that
	//    region in an expanded window that matches the given aspect ratio.
	static void matchAspectRatio(double& xmin, double& xmax,
		double& ymin, double& ymax, double vAR);
	// 3. compute2DScaleTrans uses the current model coordinate region of
	//    interest - modified as necessary to preserve aspect ratios - and
	//    then passes the modified limits to linearMap to compute the scale
	//    and translation needed to map MC to LDS.
	//    (The scales and translationes are returned in float[] because
	//    glUniform currently allows only float[].)
	static void compute2DScaleTrans(float* scaleTrans);

	static double mcRegionOfInterest[6];
	static bool aspectRatioPreservationEnabled;

	float computeX(float t);
	float computeY(float t);
	void initModelGeometry();
	float yMin;
	float yMax;
	float xMin;
	float xMax;
	int numPoints;
	int color;
	vec2* coords;
};

#endif
