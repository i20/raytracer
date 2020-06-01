#include "../headers/Triangle.hpp"
#include "../headers/Mesh.hpp"

using namespace std;

//Triangle::Triangle() {}

Triangle::Triangle(Mesh & mesh, const uintmax_t index, const uintmax_t p1, const uintmax_t p2, const uintmax_t p3) : mesh(mesh), index(index), p1(p1), p2(p2), p3(p3) {

    this->u = mesh.vertexes[p2] - mesh.vertexes[p1];
    this->v = mesh.vertexes[p3] - mesh.vertexes[p1];

    this->normal = (this->u ^ this->v).normalize();
}
