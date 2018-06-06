#include <string>
#include <sstream>
#include <iostream>

#include "../headers/Triangle.hpp"
#include "../headers/Mesh.hpp"

using namespace std;

//Triangle::Triangle() {}

Triangle::Triangle(Mesh & mesh, const uintmax_t index, const uintmax_t p1, const uintmax_t p2, const uintmax_t p3) : mesh(mesh), index(index), p1(p1), p2(p2), p3(p3) {

    float u_v[3], v_v[3];
    for (uint8_t i = 0; i < 3; i++) {

        u_v[i] = mesh.vertexes[p2].p[i] - mesh.vertexes[p1].p[i];
        v_v[i] = mesh.vertexes[p3].p[i] - mesh.vertexes[p1].p[i];
    }

    this->u = Vector(u_v[0], u_v[1], u_v[2]);
    this->v = Vector(v_v[0], v_v[1], v_v[2]);

    this->normal = (this->u ^ this->v).normalize();
}

Triangle::Triangle(const Triangle & triangle) :
    mesh(triangle.mesh),
    index(triangle.index),
    p1(triangle.p1),
    p2(triangle.p2),
    p3(triangle.p3),
    u(triangle.u),
    v(triangle.v),
    normal(triangle.normal) {}

Triangle & Triangle::operator=(const Triangle & triangle) {

    this->mesh = triangle.mesh;
    this->index = triangle.index;
    this->p1 = triangle.p1;
    this->p2 = triangle.p2;
    this->p3 = triangle.p3;
    this->u = triangle.u;
    this->v = triangle.v;
    this->normal = triangle.normal;
    return *this;
}

string Triangle::to_string() const {

    stringstream ss;
    ss << "Triangle init " << this->p1 << " " << this->p2 << " " << this->p3;
    return ss.str();
}
