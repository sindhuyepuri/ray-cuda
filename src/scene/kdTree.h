#pragma once
#include <iostream>
// #include "scene.h"
#include "bbox.h"
#include <vector>

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
    void build();
};
