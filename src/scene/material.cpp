#include "material.h"
#include "../ui/TraceUI.h"
#include "light.h"
#include "ray.h"
extern TraceUI* traceUI;
#include "glm/gtx/string_cast.hpp"
#include <glm/gtx/io.hpp>
#include <iostream>
#include "../fileio/images.h"

using namespace std;
extern bool debugMode;

Material::~Material()
{
}

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
glm::dvec3 Material::shade(Scene* scene, const ray& r, const isect& i) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
	// You will need to call both distanceAttenuation() and
	// shadowAttenuation()
	// somewhere in your code in order to compute shadows and light falloff.
	//	if( debugMode )
	//		std::cout << "Debugging Phong code..." << std::endl;

	// When you're iterating through the lights,
	// you'll want to use code that looks something
	// like this:
	//
	// for ( const auto& pLight : scene->getAllLights() )
	// {
	//              // pLight has type unique_ptr<Light>
	// 		.
	// 		.
	// 		.did 
	// 

	glm::dvec3 i_coords = r.getPosition() + r.getDirection() * i.getT();

	glm::dvec3 return_light(0.0, 0.0, 0.0);

	// TODO : verify lighting model
	return_light += scene->ambient() * ka(i);

	for (const auto& pLight : scene->getAllLights()) {
		glm::dvec3 vec_l = pLight->getDirection(i_coords);
		glm::dvec3 vec_n = i.getN();
		glm::dvec3 vec_r = glm::reflect(-vec_l, vec_n);
		glm::dvec3 vec_v = r.getDirection();

		double dist_atten = pLight->distanceAttenuation(i_coords);
		glm::dvec3 shadow_atten = pLight->shadowAttenuation(r, i_coords + vec_n * .0001); // r is ignored lol

		glm::dvec3 diffuse = kd(i) * glm::max(0.0, glm::dot(vec_n, vec_l));
		glm::dvec3 specular = ks(i) * (glm::pow(glm::max(0.0, glm::dot(-vec_v, vec_r)), shininess(i)));
		return_light += shadow_atten * dist_atten * (diffuse + specular);
	}

	//emissive light
	return_light += ke(i);

	return return_light;
}

TextureMap::TextureMap(string filename)
{
	data = readImage(filename.c_str(), width, height);
	if (data.empty()) {
		width = 0;
		height = 0;
		string error("Unable to load texture map '");
		error.append(filename);
		error.append("'.");
		throw TextureMapException(error);
	}
}

// input:   coord = (u,v)
// returns: return (R, G, B)
glm::dvec3 TextureMap::getMappedValue(const glm::dvec2& coord) const
{
	// YOUR CODE HERE
	//
	// In order to add texture mapping support to the
	// raytracer, you need to implement this function.
	// What this function should do is convert from
	// parametric space which is the unit square
	// [0, 1] x [0, 1] in 2-space to bitmap coordinates,
	// and use these to perform bilinear interpolation
	// of the values.

	double u = coord[0];
	double v = coord[1];
	double x = coord[0] * getWidth();
	double y = coord[1] * getHeight();
	
	glm::dvec3 a_11 = 1/256.0 * getPixelAt(std::floor(x), std::floor(y));
	glm::dvec3 a_12 = 1/256.0 * getPixelAt(std::floor(x), std::ceil(y));
	glm::dvec3 a_21 = 1/256.0 * getPixelAt(std::ceil(x), std::floor(y));
	glm::dvec3 a_22 = 1/256.0 * getPixelAt(std::ceil(x), std::ceil(y));

	return (1 - v) * ((1 - u) * a_11 + u * a_21) + v * ((1 - u) * a_21 + u * a_22);
	// return getPixelAt((int)u, (int)v);
}

glm::dvec3 TextureMap::getPixelAt(int x, int y) const
{
	// YOUR CODE HERE
	//
	// In order to add texture mapping support to the
	// raytracer, you need to implement this function.
	
	return glm::dvec3(data[y * getWidth() * 3 + 3*x], data[y * getWidth() * 3 + 3*x + 1], data[y * getWidth() * 3 + 3*x + 2]);
}

glm::dvec3 MaterialParameter::value(const isect& is) const
{
	if (0 != _textureMap) {
		return _textureMap->getMappedValue(is.getUVCoordinates());
	}
	else {
		return _value;
	}
}

double MaterialParameter::intensityValue(const isect& is) const
{
	if (0 != _textureMap) {
		glm::dvec3 value(
		        _textureMap->getMappedValue(is.getUVCoordinates()));
		return (0.299 * value[0]) + (0.587 * value[1]) +
		       (0.114 * value[2]);
	} else
		return (0.299 * _value[0]) + (0.587 * _value[1]) +
		       (0.114 * _value[2]);
}
