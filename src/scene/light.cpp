#include <cmath>
#include <iostream>

#include "light.h"
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include "glm/gtx/string_cast.hpp"

#include "../RayTracer.h"
extern bool debugMode;

using namespace std;

double DirectionalLight::distanceAttenuation(const glm::dvec3& P) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


glm::dvec3 DirectionalLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.
	Scene* scene = getScene();
	
	bool has_isect; 
	glm::dvec3 isect_point; // intersection with objects
	// double current_dist = 0; // isect_point -> light distance (start as zero so we enter the while loop)
	// double original_dist = glm::distance(p, position); // distance between intersection point and light
	glm::dvec3 atten (1, 1, 1); 
	double epsilon = .001;
	ray shadow_ray(p, getDirection(p), glm::dvec3(1, 1, 1), ray::RayType::SHADOW);
	glm::dvec3 current_pos = p;

	while (true) { // while we haven't passed the light
		ray shadow_ray(current_pos, getDirection(p), glm::dvec3(1, 1, 1), ray::RayType::SHADOW);
		isect i; // keep track of the place where the ray intersects each object
		has_isect = scene->intersect(shadow_ray, i);
		if (!has_isect) return atten; 
		glm::dvec3 first_isect_point = shadow_ray.at(i.getT() + epsilon);
		
		current_pos = first_isect_point;
		ray new_shadow_ray(current_pos, getDirection(p), glm::dvec3(1, 1, 1), ray::RayType::SHADOW);
		// shadow_ray.setPosition(isect_point + epsilon);

		isect j;
		bool exit_isect = scene->intersect(new_shadow_ray, j); // get intersection when exiting object
		if (!exit_isect) return atten; // shouldn't happen but just in case
		glm::dvec3 second_isect_point = new_shadow_ray.at(j.getT() + epsilon);
		
		double dist_in_obj = glm::distance(first_isect_point, second_isect_point);
		// std::cout << dist_in_obj << std::endl;
		glm::dvec3 dist_vec (dist_in_obj, dist_in_obj, dist_in_obj);
		atten *= glm::pow(i.getMaterial().kt(i), dist_vec);

		current_pos = second_isect_point;
		// current_dist = glm::distance(p, current_pos);
		//std::cout << current_dist << std::endl;
	}
	return atten;

}

glm::dvec3 DirectionalLight::getColor() const
{
	return color;
}

glm::dvec3 DirectionalLight::getDirection(const glm::dvec3& P) const
{
	return -orientation;
}

double PointLight::distanceAttenuation(const glm::dvec3& P) const
{

	// YOUR CODE HERE

	// You'll need to modify this method to attenuate the intensity 
	// of the light based on the distance between the source and the 
	// point P.  For now, we assume no attenuation and just return 1.0

	double dist = glm::distance(P, position);
	return glm::min(1.0, 1/(constantTerm + linearTerm * dist + quadraticTerm * dist * dist));
}

glm::dvec3 PointLight::getColor() const
{
	return color;
}

glm::dvec3 PointLight::getDirection(const glm::dvec3& P) const
{
	return glm::normalize(position - P);
}


glm::dvec3 PointLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.

	// if the ray hits another object, return (0, 0, 0)
	// isect i;
	// if (scene->intersect(r, i)) {
	// 	return glm::dvec3(0,0,0);
	// }

	// std::cout << "position: " << shadow_ray.getPosition() << std::endl;
	
	// if (scene->intersect(shadow_ray, i)) {
	// 	// get the intersection point
	// 	// cast another ray from that point (+epsilon)
	// 	// find the next intpoint
	// 	// get distance
	// 	return glm::dvec3(0,0,0);
	// }
	// return glm::dvec3(1,1,1);

	Scene* scene = getScene();
	
	bool has_isect; 
	glm::dvec3 isect_point; // intersection with objects
	double current_dist = 0; // isect_point -> light distance (start as zero so we enter the while loop)
	double original_dist = glm::distance(p, position); // distance between intersection point and light
	glm::dvec3 atten (1, 1, 1); 
	double epsilon = .001;
	ray shadow_ray(p, getDirection(p), glm::dvec3(1, 1, 1), ray::RayType::SHADOW);
	glm::dvec3 current_pos = p;

	while (current_dist < original_dist) { // while we haven't passed the light
		ray shadow_ray(current_pos, getDirection(p), glm::dvec3(1, 1, 1), ray::RayType::SHADOW);
		isect i; // keep track of the place where the ray intersects each object
		has_isect = scene->intersect(shadow_ray, i);
		if (!has_isect) return atten; 
		glm::dvec3 first_isect_point = shadow_ray.at(i.getT() + epsilon);
		
		current_pos = first_isect_point;
		ray new_shadow_ray(current_pos, getDirection(p), glm::dvec3(1, 1, 1), ray::RayType::SHADOW);
		// shadow_ray.setPosition(isect_point + epsilon);

		isect j;
		bool exit_isect = scene->intersect(new_shadow_ray, j); // get intersection when exiting object
		if (!exit_isect) return atten; // shouldn't happen but just in case
		glm::dvec3 second_isect_point = new_shadow_ray.at(j.getT() + epsilon);
		
		double dist_in_obj = glm::distance(first_isect_point, second_isect_point);
		// std::cout << dist_in_obj << std::endl;
		glm::dvec3 dist_vec (dist_in_obj, dist_in_obj, dist_in_obj);
		atten *= glm::pow(i.getMaterial().kt(i), dist_vec);

		current_pos = second_isect_point;
		current_dist = glm::distance(p, current_pos);
		//std::cout << current_dist << std::endl;
	}
	return atten;
}

#define VERBOSE 0

