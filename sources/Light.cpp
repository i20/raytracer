#include "../headers/Color.hpp"
#include "../headers/Light.hpp"

using namespace std;

Light::Light(const float ir, const float ig, const float ib) {

    (*this)[0] = ir;
    (*this)[1] = ig;
    (*this)[2] = ib;
}

float & Light::operator[] (const uint8_t i) {
    return this->i[i];
}

const float & Light::operator[] (const uint8_t i) const {
    return this->i[i];
}
