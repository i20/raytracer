#ifndef _PUNCTUALLIGHT_HPP
#define _PUNCTUALLIGHT_HPP

#include "../headers/Light.hpp"
#include "../headers/Color.hpp"
#include "../headers/Intersection.hpp"
#include "../headers/Point.hpp"
#include "../headers/Scene.hpp"

class PunctualLight : public Light {

    private:

        // Attenuation coefficient (constant, linear, quadratic)
        float c, l, q;

    public:

        Point position;

        PunctualLight(const float ir, const float ig, const float ib, const Point & position, const float c, const float l, const float q);

        virtual Color compute_luminosity(const Intersection & inter, const Scene & scene) const override;
        virtual void emit_photons () const override;
};

#endif
