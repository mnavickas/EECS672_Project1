// ModelView.c++ - a basic combined Model and View for OpenGL

#include <iostream>

#include "ModelView.h"
#include "Controller.h"
#include "ShaderIF.h"

double ModelView::mcRegionOfInterest[6] = { -1.0, 1.0, -1.0, 1.0, -1.0, 1.0 };
bool ModelView::aspectRatioPreservationEnabled = true;

vec3 ModelView::colors[5] =
	{
		{0.8,0.8,0.8},
		{0.8,0.0,0.0},
		{0.0,0.8,0.0},
		{0.0,0.0,0.8},
		{0.8,0.0,0.8}
	};

int ModelView::colorIndex = 0;

// NOTE: You will likely want to modify the ModelView constructor to
//       take additional parameters.
ModelView::ModelView
					(
						ShaderIF* sIF,
						float a[4],
						float b[4],
						float t[3]
					)
					: shaderIF(sIF)
					, paramA(a)
					, paramB(b)
					, paramT(t)
					, color( (colorIndex++)%5 )
{
	initModelGeometry();
}

void ModelView::initModelGeometry()
{
	glGenVertexArrays(1, vao);
	glGenBuffers(1, vbo);

	glBindVertexArray(vao[0]);
	glBindBuffer(GL_ARRAY_BUFFER, vbo[0]);

	//generate data
	numPoints = paramT[2];
	coords = new vec2[numPoints];

	const float delta = (paramT[1] - paramT[0]) / (numPoints - 1);
	float t = paramT[0];

	for( int i = 0; i < numPoints; i++, t+=delta )
	{
		coords[i][0] = computeX(t);
		coords[i][1] = computeY(t);
	}

	// make note of the min/max coordinates
	xMin = xMax = coords[0][0];
	yMin = yMax = coords[0][1];
	for (int i=1 ; i<numPoints ; i++)
	{
		if (coords[i][0] < xMin)
			xMin = coords[i][0];
		else if (coords[i][0] > xMax)
			xMax = coords[i][0];
		if (coords[i][1] < yMin)
			yMin = coords[i][1];
		else if (coords[i][1] > yMax)
			yMax = coords[i][1];
	}

	const int numBytesCoordinateData = numPoints * sizeof(vec2);
	glBufferData(GL_ARRAY_BUFFER, numBytesCoordinateData, coords, GL_STATIC_DRAW);
	glVertexAttribPointer(shaderIF->pvaLoc("mcPosition"), 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(shaderIF->pvaLoc("mcPosition"));

}
float ModelView::computeX(float t)
{
	return paramA[0] + paramA[1]*(t) + paramA[2]*(t*t) + paramA[3]*(t*t*t);
}
float ModelView::computeY(float t)
{
	return paramB[0] + paramB[1]*(t) + paramB[2]*(t*t) + paramB[3]*(t*t*t);
}

ModelView::~ModelView()
{
	glDeleteBuffers(1, vbo);
	glDeleteVertexArrays(1, vao);
	delete coords;
}

void ModelView::compute2DScaleTrans(float* scaleTransF) // CLASS METHOD
{
	double xmin = mcRegionOfInterest[0];
	double xmax = mcRegionOfInterest[1];
	double ymin = mcRegionOfInterest[2];
	double ymax = mcRegionOfInterest[3];

	if (aspectRatioPreservationEnabled)
	{
		// preserve aspect ratio. Make "region of interest" wider or taller to
		// match the Controller's viewport aspect ratio.
		double vAR = Controller::getCurrentController()->getViewportAspectRatio();
		matchAspectRatio(xmin, xmax, ymin, ymax, vAR);
	}

    // We are only concerned with the xy extents for now, hence we will
    // ignore mcRegionOfInterest[4] and mcRegionOfInterest[5].
    // Map the overall limits to the -1..+1 range expected by the OpenGL engine:
	double scaleTrans[4];
	linearMap(xmin, xmax, -1.0, 1.0, scaleTrans[0], scaleTrans[1]);
	linearMap(ymin, ymax, -1.0, 1.0, scaleTrans[2], scaleTrans[3]);
	for (int i=0 ; i<4 ; i++)
		scaleTransF[i] = static_cast<float>(scaleTrans[i]);
}

// xyzLimits: {mcXmin, mcXmax, mcYmin, mcYmax, mcZmin, mcZmax}
void ModelView::getMCBoundingBox(double* xyzLimits) const
{
	xyzLimits[0] = xMin; xyzLimits[1] = xMax;
	xyzLimits[2] = yMin; xyzLimits[3] = yMax;
	xyzLimits[4] = -1;   xyzLimits[5] = 1;
}

bool ModelView::handleCommand(unsigned char anASCIIChar, double ldsX, double ldsY)
{
	return true;
}

// linearMap determines the scale and translate parameters needed in
// order to map a value, f (fromMin <= f <= fromMax) to its corresponding
// value, t (toMin <= t <= toMax). Specifically: t = scale*f + trans.
void ModelView::linearMap(double fromMin, double fromMax, double toMin, double toMax,
					  double& scale, double& trans) // CLASS METHOD
{
	scale = (toMax - toMin) / (fromMax - fromMin);
	trans = toMin - scale*fromMin;
}

void ModelView::matchAspectRatio(double& xmin, double& xmax,
        double& ymin, double& ymax, double vAR)
{
	double wHeight = ymax - ymin;
	double wWidth = xmax - xmin;
	double wAR = wHeight / wWidth;
	if (wAR > vAR)
	{
		// make window wider
		wWidth = wHeight / vAR;
		double xmid = 0.5 * (xmin + xmax);
		xmin = xmid - 0.5*wWidth;
		xmax = xmid + 0.5*wWidth;
	}
	else
	{
		// make window taller
		wHeight = wWidth * vAR;
		double ymid = 0.5 * (ymin + ymax);
		ymin = ymid - 0.5*wHeight;
		ymax = ymid + 0.5*wHeight;
	}
}

void ModelView::render() const
{
	// save the current GLSL program in use
	GLint pgm;
	glGetIntegerv(GL_CURRENT_PROGRAM, &pgm);

	// draw the triangles using our vertex and fragment shaders
	glUseProgram(shaderIF->getShaderPgmID());

	float scaleTrans[4];
	compute2DScaleTrans(scaleTrans);
	glUniform4fv(shaderIF->ppuLoc("scaleTrans"), 1, scaleTrans);

	// Establish the color
	glUniform3fv(shaderIF->ppuLoc("color"), 1, colors[color] );

	glBindVertexArray(vao[0]); // reestablishes all buffer settings as noted above

	glDrawArrays(GL_LINE_STRIP, 0, numPoints);

	// restore the previous program
	glUseProgram(pgm);
}

void ModelView::setMCRegionOfInterest(double xyz[6])
{
	for (int i=0 ; i<6 ; i++)
		mcRegionOfInterest[i] = xyz[i];
}
