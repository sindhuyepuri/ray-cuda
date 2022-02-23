#include "cubeMap.h"
#include "ray.h"
#include "../ui/TraceUI.h"
#include "../scene/material.h"
#include <iostream>
#include <glm/gtx/io.hpp>
#include "glm/gtx/string_cast.hpp"


extern TraceUI* traceUI;

glm::dvec3 CubeMap::getColor(ray r) const
{
	// YOUR CODE HERE
	// FIXME: Implement Cube Map here

	// 1. Figure out which face we're intersecting with
		// Face with highest dot product btw ray and normal
	// 2. Intersect with the face
	// 3. Get color based on getMappedCoordinates() of intersection

	glm::dvec3 ray_dir = r.getDirection();
	double x = ray_dir[0];
	double y = ray_dir[1];
	double z = ray_dir[2];
	double abs_x = std::abs(x);
	double abs_y = std::abs(y);
	double abs_z = std::abs(z);
	// int face = 0;
	glm::dvec3 color(0.0, 0.0, 0.0);

	glm::dvec2 coord;
	if (abs_x > abs_y && abs_x > abs_z) {
		double norm_z = ((double)z)/abs_x;
		double norm_y = ((double)y)/abs_x;
		if (x > 0) {
			coord[0] = 0.5 - norm_z;
			coord[1] = 0.5 + norm_y;
			color = tMap[0]->getMappedValue(coord);
		} else {
			coord[0] = 0.5 + norm_z;
			coord[1] = 0.5 + norm_y;
			color = tMap[1]->getMappedValue(coord);
		}
	} else if (abs_y > abs_x && abs_y > abs_z) { 
		double norm_x = ((double)x)/abs_y;
		double norm_z = ((double)z)/abs_y;
		if (y > 0) {
			coord[0] = 0.5 + norm_x;
			coord[1] = 0.5 - norm_z;
			color = tMap[2]->getMappedValue(coord);
		} else {
			coord[0] = 0.5 + norm_x;
			coord[1] = 0.5 + norm_z;
			color = tMap[3]->getMappedValue(coord);
		}
	} else {
		double norm_x = ((double)x)/abs_z;
		double norm_y = ((double)y)/abs_z;
		if (z > 0) {
			coord[0] = 0.5 + norm_x;
			coord[1] = 0.5 - norm_y;
			color = tMap[4]->getMappedValue(coord);
		} else {
			coord[0] = 0.5 + norm_x;
			coord[1] = 0.5 + norm_y;
			color = tMap[5]->getMappedValue(coord);
		}
	}
	// std::cout << glm::to_string(color) << std::endl;
	return color;
}

CubeMap::CubeMap()
{
}

CubeMap::~CubeMap()
{
}

void CubeMap::setNthMap(int n, TextureMap* m)
{
	if (m != tMap[n].get())
		tMap[n].reset(m);
}
