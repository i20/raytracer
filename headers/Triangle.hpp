#ifndef _TRIANGLE_HPP
#define _TRIANGLE_HPP

#include <cstdint>

#include "../headers/Vector.hpp"

class Mesh;

class Triangle {

    public:
        Mesh & mesh;
        uintmax_t index; // triangle index in the mesh, useful for texture application
        uintmax_t p1, p2, p3; // @todo Why not use an array ?
        Vector u, v, normal;

        Triangle(Mesh & mesh, const uintmax_t index, const uintmax_t p1, const uintmax_t p2, const uintmax_t p3);
};

#endif
