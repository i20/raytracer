#include "../headers/Octree.hpp"
#include "../headers/Mesh.hpp"

#include <sstream>

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

    float x = o.p[0];
    float y = o.p[1];
    float z = o.p[2];

    float xa = x + a.v[0] / 2;
    float ya = y + a.v[1] / 2;
    float za = z + a.v[2] / 2;

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

// bool Octree::has_intersection_face(const Ray & ray, const uint8_t face) const {

//     float middle[3];
//     float normal[3];
//     /**/float tmp = 0;
//     /**/float sca = 0;

//     Vector a_2 = this->a / 2;

//     // Left
//     if (face == 1) {

//         middle[0] = this->o.p[0];
//         middle[1] = this->o.p[1] + a_2.v[1];
//         middle[2] = this->o.p[2] + a_2.v[2];

//         normal[0] = -1; normal[1] = 0; normal[2] = 0;

//         /**/tmp = -(ray.origin.p[0] - middle[0]);
//         /**/sca = -ray.direction.v[0];
//     }

//     // Front
//     else if (face == 2) {

//         middle[0] = this->o.p[0] + a_2.v[0];
//         middle[1] = this->o.p[1] + a_2.v[1];
//         middle[2] = this->o.p[2] + this->a.v[2];

//         normal[0] = 0; normal[1] = 0; normal[2] = 1;

//         /**/tmp = (ray.origin.p[2] - middle[2]);
//         /**/sca = ray.direction.v[2];
//     }

//     // Right
//     else if (face == 3) {

//         middle[0] = this->o.p[0] + this->a.v[0];
//         middle[1] = this->o.p[1] + a_2.v[1];
//         middle[2] = this->o.p[2] + a_2.v[2];

//         normal[0] = 1; normal[1] = 0; normal[2] = 0;

//         /**/tmp = (ray.origin.p[0] - middle[0]);
//         /**/sca = ray.direction.v[0];
//     }

//     // Back
//     else if (face == 4) {

//         middle[0] = this->o.p[0] + a_2.v[0];
//         middle[1] = this->o.p[1] + a_2.v[1];
//         middle[2] = this->o.p[2];

//         normal[0] = 0; normal[1] = 0; normal[2] = -1;

//         /**/tmp = -(ray.origin.p[2] - middle[2]);
//         /**/sca = -ray.direction.v[2];
//     }

//     // Bottom
//     else if (face == 5) {

//         middle[0] = this->o.p[0] + a_2.v[0];
//         middle[1] = this->o.p[1];
//         middle[2] = this->o.p[2] + a_2.v[2];

//         normal[0] = 0; normal[1] = -1; normal[2] = 0;

//         /**/tmp = -(ray.origin.p[1] - middle[1]);
//         /**/sca = -ray.direction.v[1];
//     }

//     // Top
//     else if (face == 6) {

//         middle[0] = this->o.p[0] + a_2.v[0];
//         middle[1] = this->o.p[1] + this->a.v[1];
//         middle[2] = this->o.p[2] + a_2.v[2];

//         normal[0] = 0; normal[1] = 1; normal[2] = 0;

//         /**/tmp = (ray.origin.p[1] - middle[1]);
//         /**/sca = ray.direction.v[1];
//     }

//     /**/if (!sca)
//     /**/    return false;

//     /**///float tmp = 0;
//     /**///float sca = 0;
//     /**///for (uint8_t i = 0; i < 3; i++) {

//         /**///tmp += (ray.origin.p[i] - middle[i]) * normal[i];
//         /**///sca += ray.direction.v[i] * normal[i];
//     /**///}

//     /**///if (!sca)
//     /**///    return false;

//     float t = -tmp / sca;

//     if (t < 0)
//         return false;

//     float inter[3];
//     for (uint8_t i = 0; i < 3; i++)
//         inter[i] = ray.origin.p[i] + t * ray.direction.v[i];

//     if (face == 1 || face == 3)
//         return this->o.p[1] <= inter[1] && inter[1] <= this->o.p[1] + this->a.v[1]
//             && this->o.p[2] <= inter[2] && inter[2] <= this->o.p[2] + this->a.v[2];

//     else if (face == 2 || face == 4)
//         return this->o.p[0] <= inter[0] && inter[0] <= this->o.p[0] + this->a.v[0]
//             && this->o.p[1] <= inter[1] && inter[1] <= this->o.p[1] + this->a.v[1];

//     else if (face == 5 || face == 6)
//         return this->o.p[0] <= inter[0] && inter[0] <= this->o.p[0] + this->a.v[0]
//             && this->o.p[2] <= inter[2] && inter[2] <= this->o.p[2] + this->a.v[2];
// }

// Naive method
// bool Octree::has_intersection(const Ray & ray) const {

//     return this->has_intersection_face(ray, 1)
//         || this->has_intersection_face(ray, 2)
//         || this->has_intersection_face(ray, 3)
//         || this->has_intersection_face(ray, 4)
//         || this->has_intersection_face(ray, 5)
//         || this->has_intersection_face(ray, 6);
// }

// http://people.csail.mit.edu/amy/papers/box-jgt.pdf
// http://www.scratchapixel.com/lessons/3d-basic-rendering/minimal-ray-tracer-rendering-simple-shapes/ray-box-intersection
// https://tavianator.com/fast-branchless-raybounding-box-intersections/
// https://tavianator.com/cgit/dimension.git/plain/libdimension/bvh/bvh.c
bool Octree::has_intersection (const Ray & ray) const {

    float invdir[3];
    uint8_t sign[3];
    for (uint8_t i = 0; i < 3; i++) {

        invdir[i] = 1 / ray.direction.v[i];
        sign[i] = invdir[i] < 0;
    }

    float tmin, tmax, tymin, tymax, tzmin, tzmax;

    tmin = (this->bounds[sign[0]].p[0] - ray.origin.p[0]) * invdir[0];
    tmax = (this->bounds[1-sign[0]].p[0] - ray.origin.p[0]) * invdir[0];

    tymin = (this->bounds[sign[1]].p[1] - ray.origin.p[1]) * invdir[1];
    tymax = (this->bounds[1-sign[1]].p[1] - ray.origin.p[1]) * invdir[1];

    if ( tmin > tymax || tymin > tmax )
        return false;

    if (tymin > tmin) tmin = tymin;
    if (tymax < tmax) tmax = tymax;

    tzmin = (this->bounds[sign[2]].p[2] - ray.origin.p[2]) * invdir[2];
    tzmax = (this->bounds[1-sign[2]].p[2] - ray.origin.p[2]) * invdir[2];

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

    bool r = true;
    for (uint8_t i = 0; i < 4; i++)
        r = r && this->o.p[i] <= p.p[i] && p.p[i] <= this->o.p[i] + this->a.v[i];

    return r;
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

// #TODO
string Octree::to_string() const {

    stringstream ss;

    ss << "Octree{o=>" << this->o.to_string() << ",a=>" << this->a.to_string() << ",triangles=>[";

    /*vector<Triangle>::const_iterator cbegin = this->triangles.begin(), cend = this->triangles.end();
    for (vector<Triangle>::const_iterator cit = cbegin; cit != cend; cit++) {

        if (cit != cbegin)
            ss << ",";

        ss << cit->to_string();
    }*/

    ss << "],children=>[";
    bool coma = false;

    for (uint8_t i = 0; i < 8; i++) {

        #if OCTREE_SMART_TREE

            if (0 <= this->children_idx[i]) {

                if (coma) ss << ",";
                else coma = true;

                ss << (int)i << "=>" << this->children[this->children_idx[i]]->to_string();
            }

        #else

            if (this->children[i] != nullptr) {

                if (coma) ss << ",";
                else coma = true;

                ss << (int)i << "=>" << this->children[i]->to_string();
            }

        #endif
    }

    ss << "]}";

    return ss.str();
}