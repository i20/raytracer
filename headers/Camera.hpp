#ifndef _CAMERA_HPP
#define _CAMERA_HPP

#include <cstdint>

#include "../headers/Color.hpp"
#include "../headers/Ray.hpp"
#include "../headers/Intersection.hpp"
#include "../headers/Point.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Scene.hpp"
#include "../headers/Matrix.hpp"
#include "../headers/Texture.hpp"

class Camera {

    public:

        enum projection_type { PROJECTION_ORTHOGRAPHIC, PROJECTION_PERSPECTIVE };

        Point eye, look_at;
        Vector up;

        float focale, fov;

        uintmax_t px_width, px_height;

        Camera::projection_type projection;

        uintmax_t tracing_depth;
        uintmax_t aa_depth; /* maximum aa re-computing for one pixel */
        uintmax_t aa_threshold; /* difference above which aa is required */

        Scene & scene;

        // COMPUTED

        Matrix base, inv;

        float proj_width, proj_height;
        float pasx, pasy;

        Texture<Color> image;

        // CONSTRUCTORS

        Camera(

            const Point & eye,
            const Point & look_at,
            const Vector & up,

            const float focale,
            const float fov,

            const uintmax_t px_width,
            const uintmax_t px_height,

            const Camera::projection_type projection,

            const uintmax_t tracing_depth,
            const uintmax_t aa_depth,
            const uintmax_t aa_threshold,

            Scene & scene
        );

        // SETTERS

        void set_focale(const float focale);
        void set_fov(const float fov);

        void set_projection_type(const Camera::projection_type projection);

        void set_tracing_depth(const uintmax_t tracing_depth);
        void set_aa_depth(const uintmax_t aa_depth);
        void set_aa_threshold(const uintmax_t aa_threshold);

        void watch(const Scene & scene);

        void rotate (const Vector & axis, const float pas);
        void rotateScene (const Vector & axis, const float pas);
        void translate (const Vector & translation);

        void reset ();

        // RENDER

        void render();
        void print(const char * file_name) const;

    private:
        void compute_bases();
        void compute_projection_size();

        Color radiate(const Ray & ray) const;
        bool compute_nearest_intersection(Intersection & ninter, const Ray & ray) const;
        // // Tests if a ray intersects with something, especially used to check for object's enlightment
        // //   the parameter "from" allows us to get information about the object
        // // TODO change the name of this function since it is very specific
        void sobel_aa();
};

#endif
