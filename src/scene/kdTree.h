#pragma once

// Note: you can put kd-tree here
template <typename Obj>
class KdTree {
public:
    std::vector<Geometry> objects;
    KdTree<Geometry>* left;
    KdTree<Geometry>* right;
    BoundingBox objectBounds;
    bool isLeaf;

    void build();
};

template<typename Obj>
void KdTree<Obj>::build() {
    // sort(typename Obj::beginObjects(), Obj::endObjects(), Obj::getBoundingBox().getMin());
    
    // Step 1: partition the elements into halves
    //         -> whether that is arbitrarily or through sort
    //            we should be decreasing the objects.size() for each subtree
    // Step 2: recurse on left/right subtrees, continuing to build out until
    //         there is only a single Geometry in objects
    // Step 3: include a depth to determine the sort of the objects (to get median over x/y/z)
    // Step 4: store bounding box in KdTree class
    // Step 5: alter scene->intersect(&ray) to cull checks by recursively looking at bboxes of KdTree
    if (!isLeaf) {
        std::vector<Geometry> left_objects;
        std::vector<Geometry> right_objects;
        int midpoint = objects.size()/2;
        std::cout << midpoint << std::endl;

        for(int i = 0; i < objects.size(); i++) {
            if (i < midpoint) left_objects.push_back(objects[i]);
            else right_objects.push_back(objects[i]);
        }

        left->objects = left_objects;
        right->objects = right_objects;
        if (left_objects.size() == 1) {
            left->isLeaf = true;
        } else {
            left->isLeaf = false;
            left->build();
        }

        if (right_objects.size() == 1) {
            right->isLeaf = true;
        } else {
            right->isLeaf = false;
            right->build();
        }
    }

    // Obj* it = Obj::beginObjects();
    // while (!it == Obj::endObjects()) {
    //     it++;
    //     std::cout << it << std::endl;
    // }
}