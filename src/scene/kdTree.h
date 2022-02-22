#pragma once
#include <iostream>
// #include "scene.h"
#include "bbox.h"
#include <vector>
#include "ray.h"

class Geometry;

// Note: you can put kd-tree here
// template <typename Geometry>
class KdTree {
public:
    std::vector<Geometry*> objects;
    KdTree* left;
    KdTree* right;
    BoundingBox* objectBounds;
    bool isLeaf;
    void build(int depth);
    bool get_intersection(ray& r, isect& i);
};
