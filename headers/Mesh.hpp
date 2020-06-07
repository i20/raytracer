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

class Triangle;

class Mesh : public Object {

    public:

        enum shading_type { SHADING_NONE, SHADING_GOURAUD, SHADING_PHONG };
        typedef std::vector< std::array< std::array<float, 2> , 3> > TextureMapping;

        std::vector<Triangle> triangles;
        std::vector<Point> vertexes;
        // Separate average normals from vertexes due to loading order
        // We cannot afford having dummy (0, 0, 0) vectors at initialization
        std::vector<Vector> normals;
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

        ~Mesh();

    private:

        virtual TTPairList compute_intersection_ts(const std::vector<const Octree *> & octrees, const Ray & ray_object) const override;
        virtual bool compute_intersection_final(Vector & true_normal_object, Vector & normal_object, const Point & point_object, const Triangle * t, const Ray & ray_object) const override;
        virtual Color compute_color_shape(const Point & point_object, const Triangle * triangle) const override;

        static void count_edge_valence_subroutine(std::unordered_map<std::string, uint8_t> & edge_valence, const uintmax_t pa, const uintmax_t pb);

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

        a += triangle.normal[i] * (this->vertexes[triangle.p1][i] - ray_object.origin[i]);
        b += triangle.normal[i] * ray_object.direction[i];
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
        Object::insert_t(a / b, &triangle, ts, ray_object);
}

template <class T>
T Mesh::compute_texture_texel (const Point & point_object, const Texture<T> & texture, const Triangle * triangle) const {

    // @todo handle texture mapping for normals texture (property this->normals_texture_mapping)
    auto & coords = this->image_texture_mapping[triangle->index];

    float uu = 0,
          uv = 0,
          vv = 0,
          wu = 0,
          wv = 0;

    for (uint8_t i = 0; i < 3; i++) {

        const float w_v = point_object[i] - this->vertexes[triangle->p1][i];
        uu += triangle->u[i] * triangle->u[i];
        uv += triangle->u[i] * triangle->v[i];
        vv += triangle->v[i] * triangle->v[i];
        wu += w_v * triangle->u[i];
        wv += w_v * triangle->v[i];
    }

    const float det = uv * uv - uu * vv;

    // r = coordinate on p1->p2 axis
    // s = coordinate on p1->p3 axis
    const float r = (uv * wv - vv * wu) / det;
    const float s = (uv * wu - uu * wv) / det;

    float uv_tex[2];
    // Bilinear interpolation
    for (uint8_t i = 0; i < 2; i++)
        uv_tex[i] = (1 - (r+s)) * coords[0][i] + r * coords[1][i] + s * coords[2][i];

    return texture.get_texel_by_uv(uv_tex[0], uv_tex[1], this->image_texture_filtering == Texture<Color>::FILTERING_BILINEAR);
}

#endif
