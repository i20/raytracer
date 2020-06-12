#include <cstdint>
#include <fstream>

#include "../headers/Texture.hpp"
#include "../headers/Color.hpp"
#include "../headers/Vector.hpp"

// Required by Makefile rules even if empty
// All the code is in the .hpp since it is a template class (except specialized functions)

/*static*/
template <>
Color Texture<Color>::get (std::ifstream & inf) {

    Color c;
    for (uint8_t i = 0; i < 3; i++)
        c[i] = inf.get();

    return c;
}

/*static*/
template <>
Vector Texture<Vector>::get (std::ifstream & inf) {

    // [0;255] => [-1;1]

    // 0 = -1
    // 128 = 0
    // 255 = 1

    // c = c*2/255 -1
    // 255 = 2 -1

    // c = 0 => v = -1
    // c = 128 => v ~= 0
    // c = 255 => v = 1

    Vector v;
    for (uint8_t i = 0; i < 3; i++)
        v[i] = (float)(inf.get() * 2) / 255 - 1;

    return v;
}
