#ifndef _LIGHT_HPP
#define _LIGHT_HPP

#include "../headers/Color.hpp"
#include "../headers/Intersection.hpp"

// Circular reference
class Scene;

using namespace std;

class Light {

    public:

        float i[3];

        Light (const float ir, const float ig, const float ib);
        Light (const Light & light);

        Light & operator= (const Light & light);

        virtual Color compute_luminosity (const Intersection & inter, const Scene & scene) const = 0;

        // Photon mapping required methods
        // virtual void emit_photons () const = 0;

    protected:

        void copy (const Light & light);
};

#endif