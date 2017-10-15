#include <stdint.h>
#include <cstring>

#include "../headers/Color.hpp"
#include "../headers/Intersection.hpp"

#include "../headers/Light.hpp"

using namespace std;

void Light::copy(const Light & light) {

    memcpy(this->i, light.i, 3 * sizeof(float));
}

Light::Light(const float ir, const float ig, const float ib) {

    this->i[0] = ir;
    this->i[1] = ig;
    this->i[2] = ib;
}

Light::Light(const Light & light) {

    this->copy(light);
}

Light & Light::operator=(const Light & light) {

    this->copy(light);
    return *this;
}