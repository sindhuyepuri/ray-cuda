#include "kdTree.h"
#include "scene.h"
#include <float.h>

void KdTree::build() {
    // sort(typename Obj::beginObjects(), Obj::endObjects(), Obj::getBoundingBox().getMin());
    
    // Step 1: partition the elements into halves
    //         -> whether that is arbitrarily or through sort
    //            we should be decreasing the objects.size() for each subtree
    // Step 2: recurse on left/right subtrees, continuing to build out until
    //         there is only a single Geometry in objects
    // Step 3: include a depth to determine the sort of the objects (to get median over x/y/z)
    // Step 4: store bounding box in KdTree class
    // Step 5: alter scene->intersect(&ray) to cull checks by recursively looking at bboxes of KdTree
    std::vector<Geometry*> left_objects;
    std::vector<Geometry*> right_objects;

    objectBounds = new BoundingBox(glm::dvec3(DBL_MAX, DBL_MAX, DBL_MAX), glm::dvec3(DBL_MIN, DBL_MIN, DBL_MIN));

    // if (!objects.empty()) {
    //     std::cout << objects.size() << std::endl;
    // }

    if (objects.size() <= 1) {
        isLeaf = true;
        return;
    }

    int midpoint = objects.size()/2;
    // std::cout << midpoint << std::endl;

    for(int i = 0; i < objects.size(); i++) {
        objectBounds->merge(objects[i]->getBoundingBox());
        if (i < midpoint) {
            left_objects.push_back(objects[i]);
        }
        else {
            right_objects.push_back(objects[i]);
        }
    }

    left = new KdTree();
    right = new KdTree();

    left->objects = left_objects;
    right->objects = right_objects;
    left->build();
    right->build();
}

bool KdTree::get_intersection(ray& r, isect& i) {
    if (isLeaf) return objects[0]->intersect(r, i);
    return left->get_intersection(r, i) || right->get_intersection(r, i);
}