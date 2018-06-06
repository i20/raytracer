#ifndef _COLOR_HPP
#define _COLOR_HPP

#include <cstdint>

#include <string>

using namespace std;

class Color {

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

        uint8_t c[4];

        Color(); // Needed since Texture contains a vector<Color> TODO see if cannot be removed
        Color(const uint8_t red, const uint8_t green, const uint8_t blue);
        Color(const Color & color);

        uint8_t & operator[] (const uint8_t i);

        Color & operator=(const Color & color);

        Color operator+(const Color & color) const;
        Color operator-(const Color & color) const;

        Color operator*(const Color & color) const;
        Color operator*(const float v) const;

        uint32_t get_rgba() const;

        string to_string() const;

    private:
        void copy (const Color & color);
};

#endif
