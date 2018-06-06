#ifndef _MESH_HPP
#define _MESH_HPP

#include <cstdint>
#include <cmath>

#include <vector>
#include <array>
#include <unordered_map>
#include <string>

#include "../headers/Object.hpp"
#include "../headers/Ray.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"
#include "../headers/Color.hpp"
#include "../headers/Texture.hpp"

using namespace std;

class Triangle;

class Mesh : public Object {

    public:

        enum shading_type { SHADING_NONE, SHADING_GOURAUD, SHADING_PHONG };
        typedef vector< array< array<float, 2> , 3> > TextureMapping;

        vector<Triangle> triangles;
        vector<Point> vertexes;
        // Separate average normals from vertexes due to loading order
        // We cannot afford having dummy (0, 0, 0) vectors at initialization
        vector<Vector> normals;
        Mesh::TextureMapping image_texture_mapping;
        Mesh::TextureMapping normals_texture_mapping;
        uint8_t shading;

        Mesh(
            const Color & color,
            const Point & position,
            const bool is_glassy,

            const Texture<Color> * image_texture,
            const char * image_texture_mapping,
            const typename Texture<Color>::filtering_type image_texture_filtering,

            const Texture<Vector> * normals_texture,
            const char * normals_texture_mapping,
            const typename Texture<Vector>::filtering_type normals_texture_filtering,

            const float ar, const float ag, const float ab,
            const float dr, const float dg, const float db,
            const float sr, const float sg, const float sb,
            const float r, const float n, const float g,
            const char * off_file,
            const Vector & z_dir, const Vector & y_dir,
            const bool use_octree, const uint8_t shading
        );

        Mesh(const Mesh & mesh);

        Mesh & operator=(const Mesh & mesh);

        ~Mesh();

        virtual string to_string() const override;

    private:

        virtual TTPairList compute_intersection_ts(const vector<const Octree *> & octrees, const Ray & ray_object) const override;
        virtual bool compute_intersection_final(Vector & normal_object, const Point & point_object, const Triangle * t) const override;
        virtual Color compute_color_shape(const Point & point_object, const Triangle * triangle) const override;

        static void count_edge_valence_subroutine(unordered_map<string, uint8_t> & edge_valence, const uintmax_t pa, const uintmax_t pb);

        // Inline function
        void compute_intersection_ts_subroutine(TTPairList & ts, const Triangle & triangle, const Ray & ray_object) const;

        void load_file (const char * off_file, const bool use_octree);
        void load_texture_mapping (const char * texture_mapping_file, Mesh::TextureMapping & texture_mapping) const;

        template <class T>
        T compute_texture_texel (const Point & point_object, const Texture<T> & texture, const Triangle * triangle) const;
};

inline void Mesh::compute_intersection_ts_subroutine (TTPairList & ts, const Triangle & triangle, const Ray & ray_object) const {

    // Every triangle is expressed in mesh's base
    // #REF http://geomalgorithms.com/a06-_intersect-2.html

    float a = 0, b = 0;
    for (uint8_t i = 0; i < 3; i++) {

        a += triangle.normal.v[i] * (this->vertexes[triangle.p1].p[i] - ray_object.origin.p[i]);
        b += triangle.normal.v[i] * ray_object.direction.v[i];
    }

    // b != 0 -> ray does not lie in triangle's plane so there is exactly one intersection point
    // At this point there is an intersection with the plane but we are not sure if the point is inside or outside the triangle

    // Avoid triangle that are obviously hidden because behind (only for closed meshes)
    // Reminder (http://graphics.stanford.edu/courses/cs468-10-fall/LectureSlides/02_Basics.pdf)
    // - boundary edge : adjacent to 1 face
    // - regular edge : adjacent to 2 faces
    // - singular edge : adjacent to 3 and more faces
    // - closed mesh : mesh with no boundary edge
    if (this->is_closed ? (ray_object.in ? b > 0 : b < 0) : b != 0)
        Object::insert_t(a / b, &triangle, ts);
}

template <class T>
T Mesh::compute_texture_texel (const Point & point_object, const Texture<T> & texture, const Triangle * triangle) const {

    // TODO handle texture mapping for normals texture (property this->normals_texture_mapping)
    auto & coords = this->image_texture_mapping[triangle->index];

    float uu = 0,
          uv = 0,
          vv = 0,
          wu = 0,
          wv = 0;

    for (uint8_t i = 0; i < 3; i++) {

        float w_v = point_object.p[i] - this->vertexes[triangle->p1].p[i];
        uu += triangle->u.v[i] * triangle->u.v[i];
        uv += triangle->u.v[i] * triangle->v.v[i];
        vv += triangle->v.v[i] * triangle->v.v[i];
        wu += w_v * triangle->u.v[i];
        wv += w_v * triangle->v.v[i];
    }

    float det = uv * uv - uu * vv;

    // r = coordinate on p1->p2 axis
    // s = coordinate on p1->p3 axis
    float r, s;
    r = (uv * wv - vv * wu) / det;
    s = (uv * wu - uu * wv) / det;

    float uv_tex[2];
    // Bilinear interpolation
    for (uint8_t i = 0; i < 2; i++)
        uv_tex[i] = (1 - (r+s)) * coords[0][i] + r * coords[1][i] + s * coords[2][i];

    return texture.get_texel_by_uv(uv_tex[0], uv_tex[1], this->image_texture_filtering == Texture<Color>::FILTERING_BILINEAR);
}

#endif
