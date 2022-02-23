#include "cubeMap.h"
#include "ray.h"
#include "../ui/TraceUI.h"
#include "../scene/material.h"
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
	int face = 0;

	if (abs_x > abs_y && abs_x > abs_z) {
		if (x > 0) face = 0;
		else face = 1;
	} else if (abs_y > abs_x && abs_y > abs_z) { 
		if (y > 0) face = 2;
		else face = 3;
	} else {
		if (z > 0) face = 4;
		else face = 5;
	}

	return glm::dvec3();
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
