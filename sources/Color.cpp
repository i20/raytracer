#include <sstream>

#include <string>
#include <stdint.h>

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

void Color::copy(const Color & color) {

    for (uint8_t i = 0; i < 3; i++)
        this->c[i] = color.c[i];
}

Color::Color() : c() {}

Color::Color(const uint8_t red, const uint8_t green, const uint8_t blue) {

    this->c[0] = red;
    this->c[1] = green;
    this->c[2] = blue;
}

Color::Color(const Color & color) {

    this->copy(color);
}

uint8_t & Color::operator[] (const uint8_t i) {

    return this->c[i];
}

Color & Color::operator=(const Color & color) {

    this->copy(color);
    return *this;
}

Color Color::operator+(const Color & color) const {

    uint8_t c[3];
    for(uint8_t i = 0; i < 3; i++){

        // 0 <= sum <= 510 (255 * 2)
        // 0 <= uint16_t <= 65 535
        uint16_t sum = this->c[i] + color.c[i];
        c[i] = 255 < sum ? 255 : sum;
    }

    return Color(c[0], c[1], c[2]);
}

Color Color::operator-(const Color & color) const {

    uint8_t c[3];

    for(uint8_t i = 0; i < 3; i++){

        // -255 <= sub <= 255
        // -32 768 <= int16_t <= 32 767
        int16_t sub = this->c[i] - color.c[i];
        c[i] = sub < 0 ? 0 : sub;
    }

    return Color(c[0], c[1], c[2]);
}

Color Color::operator*(const Color & color) const {

    uint8_t c[3];

    for(uint8_t i = 0; i < 3; i++){

        // 0 <= mul <= 65 025 (255 * 255)
        // 0 <= uint16_t <= 65 535
        uint16_t mul = this->c[i] * color.c[i];
        c[i] = mul > 255 ? 255 : mul;
    }

    return Color(c[0], c[1], c[2]);
}

// 0 <= v <= 1
Color Color::operator*(const float v) const {

    uint8_t c[3];

    for(uint8_t i = 0; i < 3; i++)
        c[i] = this->c[i] * v;

    return Color(c[0], c[1], c[2]);
}

uint32_t Color::get_rgba() const {

    return (this->c[0] << 24) + (this->c[1] << 16) + (this->c[2] << 8);
}

string Color::to_string() const {

    stringstream ss;
    ss << "Color[red=" << (int)this->c[0] << " green=" << (int)this->c[1] << " blue=" << (int)this->c[2] << "]";

    return ss.str();
}