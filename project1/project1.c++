// project1.c++

#include "GLFWController.h"
#include "ModelView.h"

#include <fstream>
#include <iostream>

void readFileIntoController(char* fname, GLFWController* Controller, ShaderIF* sIF);


int main(int argc, char* argv[])
{
	if( argc != 2 )
	{
		std::cerr << "Provide input file name"<< std::endl;
		exit(EXIT_FAILURE);
	}

	GLFWController c(argv[0]);
	c.reportVersions(std::cout);
	ShaderIF* sIF = new ShaderIF("shaders/project1.vsh", "shaders/project1.fsh");

	readFileIntoController( argv[1], &c, sIF );
	// initialize 2D viewing information:
	// Get the overall scene bounding box in Model Coordinates:
	double xyz[6]; // xyz limits, even though this is 2D
	c.getOverallMCBoundingBox(xyz);
	// Tell class ModelView we initially want to see the whole scene:
	ModelView::setMCRegionOfInterest(xyz);

	c.run();

	delete sIF;

	return 0;
}


void readFileIntoController(char* fname, GLFWController* controller, ShaderIF* sIF)
{
	std::fstream data( fname );
	if( !data.good() )
	{
		std::cerr << "Could not open file"<< std::endl;
		exit(EXIT_FAILURE);
		return;
	}

	while( true )
	{
		float a[4];
		float b[4];
		float t[3];
		char i;

		if( !(data>>a[0]) )
		{
			return;
		}
		for(i=1; i < 4; i++)
		{
			data >> a[i];
		}
		for(i=0; i < 4; i++)
		{
			data >> b[i];
		}
		for(i=0; i < 3; i++)
		{
			data >> t[i];
		}

		ModelView* mv = new ModelView(sIF,a,b,t);
		controller->addModel(mv);
	}
	data.close();

}
