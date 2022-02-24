#pragma once
#include <iostream>
// #include "scene.h"
#include "bbox.h"
#include <vector>
#include "ray.h"

// class Geometry;

// Note: you can put kd-tree here
template <typename T>
class KdTree {
public:
    void build(int depth) {
        // Step 1: partition the elements into halves
        //         -> whether that is arbitrarily or through sort
        //            we should be decreasing the objects.size() for each subtree
        // Step 2: recurse on left/right subtrees, continuing to build out until
        //         there is only a single Geometry in objects
        // Step 3: include a depth to determine the sort of the objects (to get median over x/y/z)
        // Step 4: store bounding box in KdTree class
        // Step 5: alter scene->intersect(&ray) to cull checks by recursively looking at bboxes of KdTree

        // std::cout << "sad" << std::endl;

        std::vector<T*> left_objects;
        std::vector<T*> right_objects;

        objectBounds = new BoundingBox();

        // sort the objects along the specified axis (depth % 3)
        sort(objects.begin(), objects.end(), [depth](T* a, T* b) -> bool {
            return a->getBoundingBox().getMin()[depth % 3] > b->getBoundingBox().getMin()[depth % 3];
        });

        // sort(objects.begin(), objects.end(), [depth](Geometry* a, Geometry* b) -> bool {
        //     return a->getBoundingBox().getMin()[0] > b->getBoundingBox().getMin()[0];
        // });

        // if (!objects.empty()) {
        //     std::cout << objects.size() << std::endl;
        // }

        if (objects.size() <= 1) {
            isLeaf = true;
            // objectBounds = objects[0]->getBoundingBox();
            return;
        }

        int midpoint = objects.size()/2;

        // std::cout << depth << std::endl;
        // std::cout << midpoint << std::endl;
        // std::cout << "\n" << std::endl;

        for(int i = 0; i < objects.size(); i++) {
            objectBounds->merge(objects[i]->getBoundingBox());
            if (i < midpoint) {
                left_objects.push_back(objects[i]);
            }
            else {
                right_objects.push_back(objects[i]);
            }
        }
        // std::cout << objectBounds->volume() << std::endl;
        left = new KdTree();
        right = new KdTree();
        left->objects = left_objects;
        right->objects = right_objects;
        if (!left_objects.empty()) left->build(depth + 1);
        if (!right_objects.empty()) right->build(depth + 1);
    }

    bool get_intersection(ray& r, isect& i) {
        if (objects.empty()) return false;
        if (isLeaf) return objects[0]->intersect(r, i);
        double min, max;
        if (!objectBounds->intersect(r, min, max)) return false;

        isect left_i;
        isect right_i;
        bool left_has = left->get_intersection(r, left_i);
        bool right_has = right->get_intersection(r, right_i);
        if (!right_has && !left_has) return false;
        if (right_has && !left_has) {
            i = right_i;
            return true;
        }
        if (left_has && !right_has) {
            i = left_i;
            return true;
        }
        if (left_i.getT() < right_i.getT()) {
            i = left_i;
        } else {
            i = right_i;
        }
        return true;
    }

    std::vector<T*> objects;
    KdTree<T>* left;
    KdTree<T>* right;
    BoundingBox* objectBounds;
    bool isLeaf;
};

