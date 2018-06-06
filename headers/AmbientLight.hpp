#ifndef _AMBIENTLIGHT_HPP
#define _AMBIENTLIGHT_HPP

#include "../headers/Light.hpp"
#include "../headers/Color.hpp"
#include "../headers/Intersection.hpp"
#include "../headers/Scene.hpp"

using namespace std;

class AmbientLight : public Light {

    public:

        AmbientLight (const float ir, const float ig, const float ib);
        AmbientLight (const AmbientLight & light);

        AmbientLight & operator= (const AmbientLight & light);

        virtual Color compute_luminosity (const Intersection & inter, const Scene & scene) const override;
};

#endif
