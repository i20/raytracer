#ifndef _TRIANGLE_HPP
#define _TRIANGLE_HPP

#include "../headers/Vector.hpp"

#include <stdint.h>
#include <string>

using namespace std;

class Mesh;

class Triangle {

    public:
        Mesh & mesh;
        uintmax_t index; // triangle index in the mesh, useful for texture application
        uintmax_t p1, p2, p3;
        Vector u, v, normal;

        // Triangle();
        Triangle(Mesh & mesh, const uintmax_t index, const uintmax_t p1, const uintmax_t p2, const uintmax_t p3);
        Triangle(const Triangle & triangle);

        Triangle & operator=(const Triangle & triangle);

        string to_string() const;
};

#endif