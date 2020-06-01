#include <cstdint>

#include "../headers/Color.hpp"

using namespace std;

/*static*/ const Color Color::BLACK(0, 0, 0);
/*static*/ const Color Color::WHITE(255, 255, 255);
/*static*/ const Color Color::RED(255, 0, 0);
/*static*/ const Color Color::GREEN(0, 255, 0);
/*static*/ const Color Color::BLUE(0, 0, 255);
/*static*/ const Color Color::PURPLE(128, 0, 128);
/*static*/ const Color Color::ORANGE(255, 128, 0);
/*static*/ const Color Color::YELLOW(255, 255, 0);
/*static*/ const Color Color::PINK(255, 105, 180);

Color::Color() : c() {}

Color::Color(const uint8_t red, const uint8_t green, const uint8_t blue) {

    (*this)[0] = red;
    (*this)[1] = green;
    (*this)[2] = blue;
}

uint8_t & Color::operator[] (const uint8_t i) {
    return this->c[i];
}

const uint8_t & Color::operator[] (const uint8_t i) const {
    return this->c[i];
}

Color Color::operator+(const Color & color) const {

    Color c;

    for (uint8_t i = 0; i < 3; i++) {

        // 0 <= sum <= 510 (255 * 2)
        // 0 <= uint16_t <= 65 535
        uint16_t sum = (*this)[i] + color[i];
        c[i] = 255 < sum ? 255 : sum;
    }

    return c;
}

Color Color::operator-(const Color & color) const {

    Color c;

    for (uint8_t i = 0; i < 3; i++) {
        // -255 <= sub <= 255
        // -32 768 <= int16_t <= 32 767
        int16_t sub = (*this)[i] - color[i];
        c[i] = sub < 0 ? 0 : sub;
    }

    return c;
}

Color Color::operator*(const Color & color) const {

    Color c;

    for (uint8_t i = 0; i < 3; i++) {

        // 0 <= mul <= 65 025 (255 * 255)
        // 0 <= uint16_t <= 65 535
        uint16_t mul = (*this)[i] * color[i];
        c[i] = mul > 255 ? 255 : mul;
    }

    return c;
}

// 0 <= v <= 1
Color Color::operator*(const float v) const {

    Color c;

    for (uint8_t i = 0; i < 3; i++)
        c[i] = (*this)[i] * v;

    return c;
}

uint32_t Color::get_rgba() const {

    return ((*this)[0] << 24) + ((*this)[1] << 16) + ((*this)[2] << 8);
}
