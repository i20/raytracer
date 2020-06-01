#ifndef _LIGHT_HPP
#define _LIGHT_HPP

#include "../headers/Color.hpp"
#include "../headers/Intersection.hpp"

class Scene;

class Light {

    private:

        // 0 <= i < +inf
        float i[3];

    public:

        Light (const float ir, const float ig, const float ib);

        float & operator[] (const uint8_t i);
        const float & operator[] (const uint8_t i) const;

        virtual Color compute_luminosity (const Intersection & inter, const Scene & scene) const = 0;
        virtual void emit_photons () const = 0;
};

#endif
