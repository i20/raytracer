#ifndef _COLOR_HPP
#define _COLOR_HPP

#include <cstdint>

class Color {

    private:

        // @todo Handle alpha transparency
        uint8_t c[3];

    public:

        static const Color BLACK;
        static const Color WHITE;
        static const Color RED;
        static const Color GREEN;
        static const Color BLUE;
        static const Color PURPLE;
        static const Color ORANGE;
        static const Color YELLOW;
        static const Color PINK;

        Color() = default;
        Color(const uint8_t red, const uint8_t green, const uint8_t blue);

        uint8_t & operator[](const uint8_t i);
        const uint8_t & operator[](const uint8_t i) const;

        Color operator+(const Color & color) const;
        Color operator-(const Color & color) const;

        Color operator*(const Color & color) const;
        Color operator*(const float v) const;

        uint32_t get_rgba() const;
};

#endif
