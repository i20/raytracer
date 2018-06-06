#ifndef _OCTREE_HPP
#define _OCTREE_HPP

#include <cstdint>

#include <string>
#include <vector>

#include "../headers/Point.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Triangle.hpp"
#include "../headers/Ray.hpp"

// Whether to use packed children or a NULL spanned array
// If false, absent children will be valued to NULL in this->children
#define OCTREE_SMART_TREE true

using namespace std;

class Octree {

    public:

        Point o;
        Vector a;

        // Speed up ray/box intersection detection
        Point bounds[2];

        // Array of child octrees, maximum of children is 8 but can be less
        // Children are packed at the beginning of the array (position are not respected) if OCTREE_SMART_TREE to avoid not null test
        Octree* children[8];

        // Current number of children in the previous array (0 <= x <= 8)
        uint8_t nb_children;

        // Ordered mapping of children to their index in previous array (-1 if not present)
        int8_t children_idx[8];

        vector<const Triangle *> triangles;

        Octree(const Point & o, const Vector & a);
        ~Octree();

        void find_path(vector<const Octree *> & octrees, const Ray & ray) const;

        bool is_contained(const Triangle & t) const;
        bool is_contained(const Point & p) const;
        bool append(const Triangle & t);
        // bool has_intersection_face(const Ray & ray, const uint8_t face) const;
        bool has_intersection(const Ray & ray) const;

        string to_string() const;

    private:

        Point children_os[8];
};

#endif
