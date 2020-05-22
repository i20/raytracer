#ifndef _LOADER_HPP
#define _LOADER_HPP

#include <cstdint>

#include <unordered_map>
#include <string>

#include <libjson.h>

#include "../headers/Color.hpp"
#include "../headers/Point.hpp"
#include "../headers/Vector.hpp"

#include "../headers/Texture.hpp"

#include "../headers/Light.hpp"
#include "../headers/AmbientLight.hpp"
#include "../headers/PunctualLight.hpp"

#include "../headers/Object.hpp"
#include "../headers/Sphere.hpp"
#include "../headers/Plane.hpp"
#include "../headers/Cylinder.hpp"
#include "../headers/Mesh.hpp"

#include "../headers/Camera.hpp"
#include "../headers/Scene.hpp"

using namespace std;

class Loader {

    public:

        typedef unordered_map<string, Color> ColorMap;
        typedef unordered_map<string, Point> PointMap;
        typedef unordered_map<string, Vector> VectorMap;

        typedef unordered_map<string, Texture<Color>*> ImageTextureMap;
        typedef unordered_map<string, Texture<Vector>*> NormalsTextureMap;

        Scene * scene;
        Camera * camera;

        Loader (const char * file);
        ~Loader ();

    private:

        // Maps keep track of entities created by the loader in order to alias them
        // And also free dynamically allocated memory upon destruction

        Loader::ColorMap colors;
        Loader::PointMap points;
        Loader::VectorMap vectors;

        Loader::ImageTextureMap image_textures;
        Loader::NormalsTextureMap normals_textures;

        // Base method to get a property of a JSON node
        static const JSONNode * get (const JSONNode & node, const char * name, const char type, const bool required = true);

        // Get methods allow retrieving simple or aliased values from JSON properties

        bool get_bool (const JSONNode & node, const char * name) const;
        float get_float (const JSONNode & node, const char * name) const;
        int get_int (const JSONNode & node, const char * name) const;
        string get_string (const JSONNode & node, const char * name) const;

        Color get_color (const JSONNode & node, const char * name) const;
        Point get_point (const JSONNode & node, const char * name) const;
        Vector get_vector (const JSONNode & node, const char * name) const;

        template <class T>
        Texture<T> * get_texture (const JSONNode & node, const char * name) const;

        // Build methods create and return instances of entities (or pointers for non "primary" types)

        Color build_color (const JSONNode & node) const;
        Point build_point (const JSONNode & node) const;
        Vector build_vector (const JSONNode & node) const;

        template <class T>
        Texture<T> * build_texture (const JSONNode & node) const;

        Object * build_object (const JSONNode & node) const;

        Sphere * build_object_sphere (const JSONNode & node,
            const Color & color,
            const Point & position,
            const bool is_glassy,
            const Texture<Color> * image_texture, const typename Texture<Color>::filtering_type image_texture_filtering,
            const Texture<Vector> * normals_texture, const typename Texture<Vector>::filtering_type normals_texture_filtering,
            const float ar, const float ag, const float ab,
            const float dr, const float dg, const float db,
            const float sr, const float sg, const float sb,
            const float r, const float n, const float g,
            const Vector & z_dir, const Vector & y_dir) const;

        Plane * build_object_plane (const JSONNode & node,
            const Color & color,
            const Point & position,
            const bool is_glassy,
            const Texture<Color> * image_texture, const typename Texture<Color>::filtering_type image_texture_filtering,
            const Texture<Vector> * normals_texture, const typename Texture<Vector>::filtering_type normals_texture_filtering,
            const float ar, const float ag, const float ab,
            const float dr, const float dg, const float db,
            const float sr, const float sg, const float sb,
            const float r, const float n, const float g,
            const Vector & z_dir, const Vector & y_dir) const;

        Cylinder * build_object_cylinder (const JSONNode & node,
            const Color & color,
            const Point & position,
            const bool is_glassy,
            const Texture<Color> * image_texture, const typename Texture<Color>::filtering_type image_texture_filtering,
            const Texture<Vector> * normals_texture, const typename Texture<Vector>::filtering_type normals_texture_filtering,
            const float ar, const float ag, const float ab,
            const float dr, const float dg, const float db,
            const float sr, const float sg, const float sb,
            const float r, const float n, const float g,
            const Vector & z_dir, const Vector & y_dir) const;

        Mesh * build_object_mesh (const JSONNode & node,
            const Color & color,
            const Point & position,
            const bool is_glassy,
            const Texture<Color> * image_texture, const char * image_texture_mapping, const typename Texture<Color>::filtering_type image_texture_filtering,
            const Texture<Vector> * normals_texture, const char * normals_texture_mapping, const typename Texture<Vector>::filtering_type normals_texture_filtering,
            const float ar, const float ag, const float ab,
            const float dr, const float dg, const float db,
            const float sr, const float sg, const float sb,
            const float r, const float n, const float g,
            const Vector & z_dir, const Vector & y_dir) const;

        Light * build_light (const JSONNode & node) const;
        AmbientLight * build_light_ambient (const JSONNode & node, const float ir, const float ig, const float ib) const;
        PunctualLight * build_light_punctual (const JSONNode & node, const float ir, const float ig, const float ib) const;

        Scene * build_scene (const JSONNode & node) const;
        Camera * build_camera (const JSONNode & node) const;

        // Some additionnal methods to help parsing complex structures

        Camera::projection_type get_camera_projection_type (const JSONNode & node, const char * name) const;
        Mesh::shading_type get_object_mesh_shading_type (const JSONNode & node, const char * name) const;

        template <class T>
        typename Texture<T>::filtering_type get_texture_filtering_type (const JSONNode & node, const char * name) const;

        template <class T>
        void parse_texture (const JSONNode & node, const char * name, const bool with_mapping, Texture<T> ** texture, typename Texture<T>::filtering_type * filtering, string * mapping) const;
};

// http://www.cplusplus.com/forum/general/76656/
template <>
Texture<Color> * Loader::get_texture<Color> (const JSONNode & node, const char * name) const;

template <>
Texture<Vector> * Loader::get_texture<Vector> (const JSONNode & node, const char * name) const;

template <class T>
Texture<T> * Loader::build_texture (const JSONNode & node) const {

    return new Texture<T>( node.as_string().c_str() );
}

template <class T>
typename Texture<T>::filtering_type Loader::get_texture_filtering_type (const JSONNode & node, const char * name) const {

    const JSONNode * filtering_node = Loader::get(node, name, JSON_STRING | JSON_NULL);

    switch ( filtering_node->type() ) {

        case JSON_STRING: {

            string filtering = filtering_node->as_string();

            if (filtering == "bilinear")
                return Texture<T>::FILTERING_BILINEAR;

            else if (filtering == "trilinear")
                return Texture<T>::FILTERING_TRILINEAR;

            else if (filtering == "anisotropic")
                return Texture<T>::FILTERING_ANISOTROPIC;

            else if (filtering == "point")
                return Texture<T>::FILTERING_NONE;

            throw string("Unknown filtering type");
        }
            break;

        case JSON_NULL:
            return Texture<T>::FILTERING_NONE;
            break;
    }
}

template <class T>
void Loader::parse_texture (const JSONNode & node, const char * name, const bool with_mapping,
    Texture<T> ** texture,
    typename Texture<T>::filtering_type * filtering,
    string * mapping
) const {

    const JSONNode * texture_node = Loader::get(node, name, JSON_NODE, false);

    if (texture_node != nullptr) {

        *texture = this->get_texture<T>(*texture_node, "ref");
        *filtering = this->get_texture_filtering_type<T>(*texture_node, "filtering");

        if (with_mapping)
            *mapping = this->get_string(*texture_node, "mapping");
    }
}

#endif
