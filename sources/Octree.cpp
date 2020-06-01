#include "../headers/Octree.hpp"
#include "../headers/Mesh.hpp"

using namespace std;

Octree::Octree(const Point & o, const Vector & a) : o(o), a(a), nb_children(0) {

    this->bounds[0] = o;
    this->bounds[1] = o + a;

    #if OCTREE_SMART_TREE

        for (uint8_t i = 0; i < 8; i++)
            this->children_idx[i] = -1;

    #else

        for (uint8_t i = 0; i < 8; i++)
            this->children[i] = nullptr;

        // memset(this->children, nullptr, 8 * sizeof(Octree*));

    #endif

    float x = o[0];
    float y = o[1];
    float z = o[2];

    float xa = x + a[0] / 2;
    float ya = y + a[1] / 2;
    float za = z + a[2] / 2;

    // Octree's origin should always be the back-left-bottom point
    this->children_os[0] = o;
    this->children_os[1] = Point(x, y, za);
    this->children_os[2] = Point(xa, y, za);
    this->children_os[3] = Point(xa, y, z);

    this->children_os[4] = Point(x, ya, z);
    this->children_os[5] = Point(x, ya, za);
    this->children_os[6] = Point(xa, ya, za);
    this->children_os[7] = Point(xa, ya, z);
}

Octree::~Octree() {

    #if OCTREE_SMART_TREE

        for (uint8_t i = 0; i < this->nb_children; i++)
            delete this->children[i];

    #else

        for (uint8_t i = 0; i < 8; i++)
            if (this->children[i] != nullptr)
                delete this->children[i];

    #endif
}

// http://people.csail.mit.edu/amy/papers/box-jgt.pdf
// http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
// https://tavianator.com/fast-branchless-raybounding-box-intersections/
// https://tavianator.com/cgit/dimension.git/plain/libdimension/bvh/bvh.c
bool Octree::has_intersection (const Ray & ray) const {

    float invdir[3];
    uint8_t sign[3];
    for (uint8_t i = 0; i < 3; i++) {

        invdir[i] = 1 / ray.direction[i];
        sign[i] = invdir[i] < 0;
    }

    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    tmin = (this->bounds[sign[0]][0] - ray.origin[0]) * invdir[0];
    tmax = (this->bounds[1-sign[0]][0] - ray.origin[0]) * invdir[0];

    tymin = (this->bounds[sign[1]][1] - ray.origin[1]) * invdir[1];
    tymax = (this->bounds[1-sign[1]][1] - ray.origin[1]) * invdir[1];

    if ( tmin > tymax || tymin > tmax )
        return false;

    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;

    tzmin = (this->bounds[sign[2]][2] - ray.origin[2]) * invdir[2];
    tzmax = (this->bounds[1-sign[2]][2] - ray.origin[2]) * invdir[2];

    if ( tmin > tzmax || tzmin > tmax )
        return false;

    if (tzmin > tmin) tmin = tzmin;
    if (tzmax < tmax) tmax = tzmax;

    return 0 <= tmin;
}

void Octree::find_path(vector<const Octree *> & octrees, const Ray & ray) const {

    if (this->has_intersection(ray)) {

        octrees.push_back(this);

        #if OCTREE_SMART_TREE

            for (uint8_t i = 0; i < this->nb_children; i++)
                this->children[i]->find_path(octrees, ray);

        #else

            for (uint8_t i = 0; i < 8; i++)
                if (this->children[i] != nullptr)
                    this->children[i]->find_path(octrees, ray);

        #endif
    }
}

bool Octree::is_contained(const Triangle & t) const {

    return this->is_contained(t.mesh.vertexes[t.p1])
        && this->is_contained(t.mesh.vertexes[t.p2])
        && this->is_contained(t.mesh.vertexes[t.p3]);
}

bool Octree::is_contained(const Point & p) const {

    for (uint8_t i = 0; i < 4; i++)
        if (p[i] < this->o[i] || this->o[i] + this->a[i] < p[i])
            return false;

    return true;
}

// Try to attach a triangle to an octree the deepest possible
// A single triangle is attached to at most one octree and is not shared
bool Octree::append(const Triangle & t) {

    if (!this->is_contained(t))
        return false;

    Vector a_2 = this->a / 2;

    for (uint8_t i = 0; i < 8; i++) {

        #if OCTREE_SMART_TREE

            bool is_new = this->children_idx[i] < 0;
            Octree * child = is_new ? new Octree(this->children_os[i], a_2) : this->children[this->children_idx[i]];

            if (child->append(t)) {

                if (is_new)
                    this->children[this->children_idx[i] = this->nb_children++] = child;

                return true;
            }

            else if (is_new)
                delete child;

        #else

            bool is_new = this->children[i] == nullptr;
            Octree * child = is_new ? new Octree(this->children_os[i], a_2) : this->children[i];

            if (child->append(t)) {

                if (is_new)
                    this->children[i] = child;

                return true;
            }

            else if (is_new)
                delete child;

        #endif
    }

    this->triangles.push_back(&t);
    return true;
}
