#ifndef _PUNCTUALLIGHT_HPP
#define _PUNCTUALLIGHT_HPP

#include "../headers/Light.hpp"
#include "../headers/Color.hpp"
#include "../headers/Intersection.hpp"
#include "../headers/Point.hpp"
#include "../headers/Scene.hpp"

using namespace std;

class PunctualLight : public Light {

    public:

        Point position;

        PunctualLight(const float ir, const float ig, const float ib, const Point & position, const float c, const float l, const float q);

        PunctualLight(const PunctualLight & light);

        PunctualLight & operator=(const PunctualLight & light);

        virtual Color compute_luminosity(const Intersection & inter, const Scene & scene) const override;
        // virtual void emit_photons () const override;

    private:

        // Attenuation coefficient (constant, linear, quadratic)
        float c, l, q;
};

#endif
