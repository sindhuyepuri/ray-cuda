// The main ray tracer.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

#include "ui/TraceUI.h"
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include "glm/gtx/string_cast.hpp"

#include <string.h> // for memset
#include <thread>
#include <pthread.h>

#include <iostream>
#include <fstream>

using namespace std;
extern TraceUI* traceUI;

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = true;

// Trace a top-level ray through pixel(i,j), i.e. normalized window coordinates (x,y),
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.

// glm::dvec3 RayTracer::trace(double x, double y)
// {
// 	// Clear out the ray cache in the scene for debugging purposes,
// 	if (TraceUI::m_debug)
// 	{
// 		scene->clearIntersectCache();		
// 	}
// 	ray r(glm::dvec3(0,0,0), glm::dvec3(0,0,0), glm::dvec3(1,1,1), ray::VISIBILITY);
// 	scene->getCamera().rayThrough(x,y,r);
// 	double dummy;
// 	glm::dvec3 ret = traceRay(r, glm::dvec3(1.0,1.0,1.0), traceUI->getDepth(), dummy);
// 	ret = glm::clamp(ret, 0.0, 1.0);
// 	return ret;
// }

void RayTracer::tracePixel(int i, int j) {
	// int i = threadIdx.x + blockIdx.x * blockDim.x
	// int j = threadIdx.y + blockIdx.y * blockDim.y;
	// if ((i >= max_x) || (j >= max_y)) return;
	
	glm::dvec3 col(0,0,0);

	if( ! sceneLoaded() ) return;

	double x = double(i)/double(buffer_width);
	double y = double(j)/double(buffer_height);

	unsigned char *pixel = buffer.data() + ( i + j * buffer_width ) * 3;
	ray r(glm::dvec3(0,0,0), glm::dvec3(0,0,0), glm::dvec3(1,1,1), ray::VISIBILITY);
	scene->getCamera().rayThrough(x,y,r);
	double dummy;
	col = traceRay(r, glm::dvec3(1.0,1.0,1.0), traceUI->getDepth(), dummy);
	col = glm::clamp(col, 0.0, 1.0);

	pixel[0] = (int)( 255.0 * col[0]);
	pixel[1] = (int)( 255.0 * col[1]);
	pixel[2] = (int)( 255.0 * col[2]);
}

#define VERBOSE 0

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
glm::dvec3 RayTracer::traceRay(ray& r, const glm::dvec3& thresh, int depth, double& t )
{
	isect i;
	glm::dvec3 colorC(0.0, 0.0, 0.0);

#if VERBOSE
	std::cerr << "== current depth: " << depth << std::endl;
#endif

	if(depth >= 0 && scene->intersect(r, i)) {
		

		const Material& m = i.getMaterial();
		colorC = m.shade(scene.get(), r, i);

		if (depth > 0) {
			glm::dvec3 w_in = r.getDirection();
			glm::dvec3 n = i.getN();
			glm::dvec3 reflect_vec(w_in - 2.0 * (glm::dot(w_in, n) * n));
			ray reflected_ray(r.at(i.getT()), reflect_vec, glm::dvec3(1, 1, 1), ray::RayType::REFLECTION);
			colorC += m.kr(i) * traceRay(reflected_ray, thresh, depth - 1, t); // what are thresh and t for?
		}
		glm::dvec3 normal = i.getN();
		glm::dvec3 incident = r.getDirection();
		double n_i;
		double n_t;
		if (glm::dot(normal, incident) > 0) { // entering
			n_i = 1.0003;
			n_t = m.index(i);
		} else { // exiting
			n_i = m.index(i);
			n_t = 1.0003;
			normal = -1.0 * normal;
		}
		double curly_n = n_i/n_t;
		double c_1 = glm::dot(normal, incident);
		double c_2_sqrd = 1 - glm::pow(curly_n, 2.0) * (1 - glm::pow(c_1, 2.0));
		if (c_2_sqrd >= 0) {
			double c_2 = glm::sqrt(c_2_sqrd);
			glm::dvec3 trans_vec = glm::normalize(curly_n * incident + (curly_n * c_1 - c_2) * normal);
			double mis_length = glm::length(curly_n * (incident + c_1 * normal));
			// glm::dvec3 t_vec = curly_n * incident + (curly_n * c_1 - glm::sqrt(1 - glm::pow(mis_length, 2.0))) * normal;
			ray refracted_ray(r.at(i.getT()), glm::normalize(trans_vec), glm::dvec3(1, 1, 1), ray::RayType::REFRACTION);
			colorC += m.kt(i) * traceRay(refracted_ray, thresh, depth - 1, t);
		}
		
	} else {
		colorC = glm::dvec3(0.0, 0.0, 0.0);
	}		
#if VERBOSE
	std::cerr << "== depth: " << depth+1 << " done, returning: " << colorC << std::endl;
#endif
	return colorC;
}

RayTracer::RayTracer()
	: scene(nullptr), buffer(0), thresh(0), buffer_width(0), buffer_height(0), m_bBufferReady(false)
{
}

RayTracer::~RayTracer()
{
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer.data();
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene(const char* fn)
{
	ifstream ifs(fn);
	if( !ifs ) {
		string msg( "Error: couldn't read scene file " );
		msg.append( fn );
		traceUI->alert( msg );
		return false;
	}

	// Strip off filename, leaving only the path:
	string path( fn );
	if (path.find_last_of( "\\/" ) == string::npos)
		path = ".";
	else
		path = path.substr(0, path.find_last_of( "\\/" ));

	// Call this with 'true' for debug output from the tokenizer
	Tokenizer tokenizer( ifs, false );
	Parser parser( tokenizer, path );
	try {
		scene.reset(parser.parseScene());
	}
	catch( SyntaxErrorException& pe ) {
		traceUI->alert( pe.formattedMessage() );
		return false;
	} catch( ParserException& pe ) {
		string msg( "Parser: fatal exception " );
		msg.append( pe.message() );
		traceUI->alert( msg );
		return false;
	} catch( TextureMapException e ) {
		string msg( "Texture mapping exception: " );
		msg.append( e.message() );
		traceUI->alert( msg );
		return false;
	}

	if (!sceneLoaded())
		return false;

	return true;
}

void RayTracer::traceSetup(int w, int h)
{
	size_t newBufferSize = w * h * 3;
	if (newBufferSize != buffer.size()) {
		bufferSize = newBufferSize;
		buffer.resize(bufferSize);
	}
	buffer_width = w;
	buffer_height = h;
	std::fill(buffer.begin(), buffer.end(), 0);
	m_bBufferReady = true;

	/*
	 * Sync with TraceUI
	 */

	threads = traceUI->getThreads();
	block_size = traceUI->getBlockSize();
	thresh = traceUI->getThreshold();
	samples = traceUI->getSuperSamples();
	aaThresh = traceUI->getAaThreshold();

	// YOUR CODE HERE
	// FIXME: Additional initializations
	scene->kdtree = new KdTree<Geometry>();
	scene->kdtree->objects = scene->objects;
	scene->kdtree->build(0);
	cout << scene->objects.size() << endl;
	for (auto &geo : scene->objects) {
		geo->BuildKdTree();
	}
}

struct worker_args {
	int start_height;
	int end_height;
	int width;
	int id;
	RayTracer* rt;
};

void* work(void* arguments) {
	struct worker_args* args = (struct worker_args*)arguments;
	int w = args->width;
	int start_h = args->start_height;
	int end_h = args->end_height;
	for (int i = 0; i < w; i++) {
		for (int j = start_h; j <= end_h; j++) {
			args->rt->tracePixel(i, j);
		}
	}
}

/*
 * RayTracer::traceImage
 *
 *	Trace the image and store the pixel data in RayTracer::buffer.
 *
 *	Arguments:
 *		w:	width of the image buffer
 *		h:	height of the image buffer
 *
 */
void RayTracer::traceImage(int w, int h)
{
	traceSetup(w,h);
	// int tx, ty = 8;

	// dim3 blocks(w/tx + 1, h/ty + 1);
	// dim3 threads(tx, ty);

	// for (int i = 0; i < w; i++) {
	// 	for (int j = 0; j < h; j++) {
	// 		tracePixel(i, j);
	// 	}
	// }
	pthread_t* workers = new pthread_t[threads];

	int incr = h / threads;
	for (int i = 0; i < threads; i++) {
		struct worker_args* args = new struct worker_args();
		args->start_height = i * incr;
		args->end_height = i * incr + incr - 1;
		args->width = w;
		args->id = i;
		args->rt = this;
		pthread_create(&workers[i], NULL, work, (void*)args);
	}

	for (int i = 0; i < threads; i++) {
		pthread_join(workers[i], NULL);
	}
}

// std::vector<std::pair<int, int>> aliased_pixels;

// double max_dist = DBL_MIN;

// bool RayTracer::findAliasedPixel(int i, int j) {
// 	int num_pixels = 0;
// 	unsigned char *curr_pixel = buffer.data() + ( i + j * buffer_width ) * 3;
// 	glm::dvec3 curr_color(curr_pixel[0], curr_pixel[1], curr_pixel[2]);

// 	glm::dvec3 avg_color(0.0, 0.0, 0.0);
// 	for (int x = i - 1; x <= i + 1; x++) {
// 		for (int y = j - 1; y <= j + 1; y++) {
// 			if (x >= 0 && x < buffer_width && y >= 0 && y < buffer_height) {
// 				num_pixels++;
// 				unsigned char *pixel = buffer.data() + ( x + y * buffer_width ) * 3;
// 				avg_color += glm::dvec3(pixel[0], pixel[1], pixel[2]);
// 			}
// 		}
// 	}

// 	avg_color = 1/((double)num_pixels) * avg_color;

// 	double dist = glm::distance(1/256.0 * curr_color, 1/256.0 * avg_color);
// 	if (dist > max_dist) {
// 		max_dist = dist;
// 	}
// 	std::cout << dist << std::endl;
// 	if (glm::distance(1/256.0 * curr_color, 1/256.0 * avg_color) > .1) {
// 		std::cout << dist << std::endl;
// 		std::pair <int, int> pixel (i, j);
// 		aliased_pixels.push_back(pixel);
// 		return true;
// 	}
// 	return false;
// }

int RayTracer::aaImage()
{
// 	// YOUR CODE HERE
// 	// FIXME: Implement Anti-aliasing here
// 	//
// 	// TIP: samples and aaThresh have been synchronized with TraceUI by
// 	//      RayTracer::traceSetup() function
// 	for (int i = 0; i < buffer_width; i++) {
// 		for (int j = 0; j < buffer_height; j++) {
// 			if(findAliasedPixel(i, j)) {
// 				setPixel(i, j, glm::dvec3(1, 1, 1));
// 			} else {
// 				setPixel(i, j, glm::dvec3(0, 0, 0));
// 			}
// 		}
// 	}
// 	// std::cout << max_dist << std::endl;
// 	// for (std::pair<int, int> &element : aliased_pixels) {
// 	// 	setPixel(element.first, element.second, aaPixel(element.first, element.second, 0, 1));
// 	// }

	return 0;
}

// // recurses until the difference between the original color and the antialiased color is less than thresh
// glm::dvec3 RayTracer::aaPixel (double i, double j, int depth, double pixel_width) {
// 	glm::dvec3 initial_col = trace(i / double(buffer_width), j / double(buffer_height));
// 	glm::dvec3 final_col(0, 0, 0);
// 	double incr = pixel_width/double(samples);

// 	for (double i_incr = 0.0; i_incr < pixel_width; i_incr += incr) {
// 		for (double j_incr = 0.0; j_incr < pixel_width; j_incr += incr) {

// 			glm::dvec3 col(0,0,0);

// 			double x = (double(i) + i_incr)/double(buffer_width);
// 			double y = (double(j) + j_incr)/double(buffer_height);
// 			col = trace(x, y);
// 			if (glm::distance(initial_col, col) > aaThresh && depth < 5) {
// 				// col = glm::dvec3(1, 1, 1);
// 				col = aaPixel(i + i_incr, j + j_incr, depth + 1, pixel_width / samples);
// 			} 
// 			// else {
// 				// col = glm::dvec3(0, 0, 0);
// 			// }
// 			final_col += col;
// 		}
// 	}

// 	final_col /= double(samples * samples);
	
// 	return final_col;
// }

bool RayTracer::checkRender()
{
	// YOUR CODE HERE
	// FIXME: Return true if tracing is done.
	//        This is a helper routine for GUI.
	//
	// TIPS: Introduce an array to track the status of each worker thread.
	//       This array is maintained by the worker threads.
	return true;
}

void RayTracer::waitRender()
{
	// YOUR CODE HERE
	// FIXME: Wait until the rendering process is done.
	//        This function is essential if you are using an asynchronous
	//        traceImage implementation.
	//
	// TIPS: Join all worker threads here.
}


glm::dvec3 RayTracer::getPixel(int i, int j)
{
	unsigned char *pixel = buffer.data() + ( i + j * buffer_width ) * 3;
	return glm::dvec3((double)pixel[0]/255.0, (double)pixel[1]/255.0, (double)pixel[2]/255.0);
}

void RayTracer::setPixel(int i, int j, glm::dvec3 color)
{
	unsigned char *pixel = buffer.data() + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * color[0]);
	pixel[1] = (int)( 255.0 * color[1]);
	pixel[2] = (int)( 255.0 * color[2]);
}

