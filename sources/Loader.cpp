#include <cstdint>

#include <unordered_map>
#include <sstream>
#include <string>
#include <fstream>

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

#include "../headers/Loader.hpp"

using namespace std;

Loader::~Loader () {

    for (auto & light_entry : this->scene->lights)
        delete light_entry.second;

    for (auto & object_entry : this->scene->objects)
        delete object_entry.second;

    delete this->scene;
    delete this->camera;

    for (auto & image_texture_entry : this->image_textures)
        delete image_texture_entry.second;

    for (auto & normals_texture_entry : this->normals_textures)
        delete normals_texture_entry.second;
}

Loader::Loader (const char * file) {

    ifstream inf(file);
    if ( !inf.is_open() ) throw string("Failed opening scene file");

    // Read the entire file into a string
    // Mind the extra parenthesis around arguments to avoid the most vexing parse
    // http://stackoverflow.com/questions/2602013/read-whole-ascii-file-into-c-stdstring
    // http://www.informit.com/guides/content.aspx?g=cplusplus&seqNum=439
    string json( (istreambuf_iterator<char>(inf)),
                 (istreambuf_iterator<char>()) );

    inf.close();

    JSONNode root = libjson::parse(json);

    const JSONNode * colors_node = Loader::get(root, "colors", JSON_NODE, false);
    const JSONNode * positions_node = Loader::get(root, "positions", JSON_NODE, false);
    const JSONNode * vectors_node = Loader::get(root, "vectors", JSON_NODE, false);
    const JSONNode * textures_node = Loader::get(root, "textures", JSON_NODE, false);

    if (colors_node != nullptr)
        for (const auto & node : *colors_node)
            this->colors.insert( Loader::ColorMap::value_type(
                node.name(),
                this->build_color(node)
            ));

    if (positions_node != nullptr)
        for (const auto & node : *positions_node)
            this->points.insert( Loader::PointMap::value_type(
                node.name(),
                this->build_point(node)
            ));

    if (vectors_node != nullptr)
        for (const auto & node : *vectors_node)
            this->vectors.insert( Loader::VectorMap::value_type(
                node.name(),
                this->build_vector(node)
            ));

    if (textures_node != nullptr) {

        const JSONNode * textures_images_node = Loader::get(*textures_node, "images", JSON_NODE, false);
        const JSONNode * textures_normals_node = Loader::get(*textures_node, "normals", JSON_NODE, false);

        if (textures_images_node != nullptr)
            for (const auto & node : *textures_images_node)
                this->image_textures.insert( Loader::ImageTextureMap::value_type(
                    node.name(),
                    this->build_texture<Color>(node)
                ));

        if (textures_normals_node != nullptr)
            for (const auto & node : *textures_normals_node)
                this->normals_textures.insert( Loader::NormalsTextureMap::value_type(
                    node.name(),
                    this->build_texture<Vector>(node)
                ));
    }

    const JSONNode * scene_node = Loader::get(root, "scene", JSON_NODE);
    const JSONNode * camera_node = Loader::get(root, "camera", JSON_NODE);

    this->scene = this->build_scene(*scene_node);
    this->camera = this->build_camera(*camera_node);
}

/*static*/ const JSONNode * Loader::get (const JSONNode & node, const char * name, const char type, const bool required/* = true*/) {

    JSONNode::const_iterator cit = node.find(name);

    if ( cit == node.end() ) {

        if (required) {

            stringstream ss;
            ss << "Missing property : " << name;

            throw ss.str();
        }

        return nullptr;
    }

    char realtype = cit->type();

    // Handle 0000
    if ( (type & realtype) != realtype ) throw string("Invalid property type");

    return &(*cit);
}

// --------------------------------------------------------------------------------------

bool Loader::get_bool (const JSONNode & node, const char * name) const {

    return Loader::get(node, name, JSON_BOOL)->as_bool();
}

float Loader::get_float (const JSONNode & node, const char * name) const {

    return Loader::get(node, name, JSON_NUMBER)->as_float();
}

int Loader::get_int (const JSONNode & node, const char * name) const {

    return Loader::get(node, name, JSON_NUMBER)->as_int();
}

string Loader::get_string (const JSONNode & node, const char * name) const {

    return Loader::get(node, name, JSON_STRING)->as_string();
}

// --------------------------------------------------------------------------------------

// Mind the extra parenthesis with case in switch
// http://stackoverflow.com/questions/5685471/error-jump-to-case-label

Color Loader::get_color (const JSONNode & node, const char * name) const {

    const JSONNode * c = Loader::get(node, name, JSON_STRING | JSON_NODE | JSON_NULL);

    switch ( c->type() ) {

        // Color is referenced
        case JSON_STRING: {

            string c_alias = c->as_string();

            Loader::ColorMap::const_iterator cit = this->colors.find(c_alias);

            if (cit == this->colors.end())
                throw string("Unknown color \"") + c_alias + "\" in scene file";

            return cit->second;
        }
            break;

        // Color is explicit
        case JSON_NODE:

            return this->build_color(*c);
            break;

        case JSON_NULL:

            return Color::BLACK;
            break;
    }
}

Point Loader::get_point (const JSONNode & node, const char * name) const {

    const JSONNode * p = Loader::get(node, name, JSON_STRING | JSON_NODE);

    switch ( p->type() ) {

        // Position is referenced
        case JSON_STRING: {

            string p_alias = p->as_string();

            Loader::PointMap::const_iterator cit = this->points.find(p_alias);

            if (cit == this->points.end())
                throw string("Unknown position \"") + p_alias + "\" in scene file";

            return cit->second;
        }
            break;

        // Position is explicit
        case JSON_NODE:

            return this->build_point(*p);
            break;
    }
}

Vector Loader::get_vector (const JSONNode & node, const char * name) const {

    const JSONNode * v = Loader::get(node, name, JSON_STRING | JSON_NODE);

    switch ( v->type() ) {

        // Vector is referenced
        case JSON_STRING: {

            string v_alias = v->as_string();

            Loader::VectorMap::const_iterator cit = this->vectors.find(v_alias);

            if (cit == this->vectors.end())
                throw string("Unknown vector \"") + v_alias + "\" in scene file";

            return cit->second;
        }
            break;

        // Vector is explicit
        case JSON_NODE:

            return this->build_vector(*v);
            break;
    }
}

// --------------------------------------------------------------------------------------

template <>
Texture<Color> * Loader::get_texture<Color> (const JSONNode & node, const char * name) const {

    string ref = this->get_string(node, name);

    Loader::ImageTextureMap::const_iterator cit = this->image_textures.find(ref);

    if (cit == this->image_textures.end())
        throw string("Unknown image texture \"") + ref + "\" in scene file";

    return cit->second;
}

template <>
Texture<Vector> * Loader::get_texture<Vector> (const JSONNode & node, const char * name) const {

    string ref = this->get_string(node, name);

    Loader::NormalsTextureMap::const_iterator cit = this->normals_textures.find(ref);

    if (cit == this->normals_textures.end())
        throw string("Unknown normals texture \"") + ref + "\" in scene file";

    return cit->second;
}

// --------------------------------------------------------------------------------------

Color Loader::build_color (const JSONNode & node) const {

    return Color(
        this->get_int(node, "r"),
        this->get_int(node, "g"),
        this->get_int(node, "b")
    );
}

Point Loader::build_point (const JSONNode & node) const {

    return Point(
        this->get_float(node, "x"),
        this->get_float(node, "y"),
        this->get_float(node, "z")
    );
}

Vector Loader::build_vector (const JSONNode & node) const {

    return Vector(
        this->get_float(node, "x"),
        this->get_float(node, "y"),
        this->get_float(node, "z")
    );
}

// --------------------------------------------------------------------------------------

Object * Loader::build_object (const JSONNode & node) const {

    string type = this->get_string(node, "type");
    Color color = this->get_color(node, "color");
    Point position = this->get_point(node, "position");

    // ----------------

    Texture<Color> * textures_image = nullptr;
    Texture<Vector> * textures_normals = nullptr;

    typename Texture<Color>::filtering_type textures_image_filtering;
    typename Texture<Vector>::filtering_type textures_normals_filtering;

    string textures_image_mapping, textures_normals_mapping;

    const JSONNode * textures_node = Loader::get(node, "textures", JSON_NODE, false);

    if (textures_node != nullptr) {

        bool with_mapping = type == "mesh";

        this->parse_texture<Color>(*textures_node, "image", with_mapping, &textures_image, &textures_image_filtering, &textures_image_mapping);
        this->parse_texture<Vector>(*textures_node, "normals", with_mapping, &textures_normals, &textures_normals_filtering, &textures_normals_mapping);
    }

    // ----------------

    const JSONNode * coeff = Loader::get(node, "coeff", JSON_NODE);
    const JSONNode * ambient_ref = Loader::get(*coeff, "ambient", JSON_NODE);
    const JSONNode * diffuse_ref = Loader::get(*coeff, "diffuse", JSON_NODE);
    const JSONNode * specular_ref = Loader::get(*coeff, "specular", JSON_NODE);

    float ar = this->get_float(*ambient_ref, "r");
    float ag = this->get_float(*ambient_ref, "g");
    float ab = this->get_float(*ambient_ref, "b");

    float dr = this->get_float(*diffuse_ref, "r");
    float dg = this->get_float(*diffuse_ref, "g");
    float db = this->get_float(*diffuse_ref, "b");

    float sr = this->get_float(*specular_ref, "r");
    float sg = this->get_float(*specular_ref, "g");
    float sb = this->get_float(*specular_ref, "b");

    float r = this->get_float(node, "reflection");
    float t = this->get_float(node, "transmission");

    float n = this->get_float(node, "n");
    float g = this->get_float(node, "glossyness");

    Vector z_dir = this->get_vector(node, "z-dir");
    Vector y_dir = this->get_vector(node, "y-dir");

    if (type == "sphere")
        return this->build_object_sphere(node,
            color,
            position,
            textures_image, textures_image_filtering,
            textures_normals, textures_normals_filtering,
            ar, ag, ab,
            dr, dg, db,
            sr, sg, sb,
            r, t, n, g,
            z_dir, y_dir
        );

    else if (type == "cylinder")
        return this->build_object_cylinder(node,
            color,
            position,
            textures_image, textures_image_filtering,
            textures_normals, textures_normals_filtering,
            ar, ag, ab,
            dr, dg, db,
            sr, sg, sb,
            r, t, n, g,
            z_dir, y_dir
        );

    else if (type == "plane")
        return this->build_object_plane(node,
            color,
            position,
            textures_image, textures_image_filtering,
            textures_normals, textures_normals_filtering,
            ar, ag, ab,
            dr, dg, db,
            sr, sg, sb,
            r, t, n, g,
            z_dir, y_dir
        );

    else if (type == "mesh")
        return this->build_object_mesh(node,
            color,
            position,
            textures_image, textures_image_mapping.c_str(), textures_image_filtering,
            textures_normals, textures_normals_mapping.c_str(), textures_normals_filtering,
            ar, ag, ab,
            dr, dg, db,
            sr, sg, sb,
            r, t, n, g,
            z_dir, y_dir
        );

    throw string("Unknown object type \"") + type + "\" when parsing scene file";
}

Sphere * Loader::build_object_sphere (const JSONNode & node,
    const Color & color,
    const Point & position,
    const Texture<Color> * image_texture, const typename Texture<Color>::filtering_type image_texture_filtering,
    const Texture<Vector> * normals_texture, const typename Texture<Vector>::filtering_type normals_texture_filtering,
    const float ar, const float ag, const float ab,
    const float dr, const float dg, const float db,
    const float sr, const float sg, const float sb,
    const float r, const float t,
    const float n, const float g,
    const Vector & z_dir, const Vector & y_dir
) const {

    return new Sphere(

        color,
        position,

        image_texture, image_texture_filtering,
        normals_texture, normals_texture_filtering,

        ar, ag, ab,
        dr, dg, db,
        sr, sg, sb,
        r, t, n, g,

        this->get_float(node, "radius"),

        z_dir, y_dir
    );
}

Plane * Loader::build_object_plane (const JSONNode & node,
    const Color & color,
    const Point & position,
    const Texture<Color> * image_texture, const typename Texture<Color>::filtering_type image_texture_filtering,
    const Texture<Vector> * normals_texture, const typename Texture<Vector>::filtering_type normals_texture_filtering,
    const float ar, const float ag, const float ab,
    const float dr, const float dg, const float db,
    const float sr, const float sg, const float sb,
    const float r,  const float t,
    const float n, const float g,
    const Vector & z_dir, const Vector & y_dir
) const {

    return new Plane(

        color,
        position,

        image_texture, image_texture_filtering,
        normals_texture, normals_texture_filtering,

        ar, ag, ab,
        dr, dg, db,
        sr, sg, sb,
        r, t, n, g,

        z_dir, y_dir,

        this->get_float(node, "height"),
        this->get_float(node, "width")
    );
}

Cylinder * Loader::build_object_cylinder (const JSONNode & node,
    const Color & color,
    const Point & position,
    const Texture<Color> * image_texture, const typename Texture<Color>::filtering_type image_texture_filtering,
    const Texture<Vector> * normals_texture, const typename Texture<Vector>::filtering_type normals_texture_filtering,
    const float ar, const float ag, const float ab,
    const float dr, const float dg, const float db,
    const float sr, const float sg, const float sb,
    const float r,  const float t,
    const float n, const float g,
    const Vector & z_dir, const Vector & y_dir
) const {

    return new Cylinder(

        color,
        position,

        image_texture, image_texture_filtering,
        normals_texture, normals_texture_filtering,

        ar, ag, ab,
        dr, dg, db,
        sr, sg, sb,
        r, t, n, g,

        this->get_float(node, "radius"),

        z_dir, y_dir,

        this->get_float(node, "height"),
        false
    );
}

Mesh * Loader::build_object_mesh (const JSONNode & node,
    const Color & color,
    const Point & position,
    const Texture<Color> * image_texture, const char * image_texture_mapping, const typename Texture<Color>::filtering_type image_texture_filtering,
    const Texture<Vector> * normals_texture, const char * normals_texture_mapping, const typename Texture<Vector>::filtering_type normals_texture_filtering,
    const float ar, const float ag, const float ab,
    const float dr, const float dg, const float db,
    const float sr, const float sg, const float sb,
    const float r,  const float t,
    const float n, const float g,
    const Vector & z_dir, const Vector & y_dir
) const {

    return new Mesh(

        color,
        position,

        image_texture, image_texture_mapping, image_texture_filtering,
        normals_texture, normals_texture_mapping, normals_texture_filtering,

        ar, ag, ab,
        dr, dg, db,
        sr, sg, sb,
        r, t, n, g,

        this->get_string(node, "file").c_str(),

        z_dir, y_dir,

        this->get_bool(node, "use-octree"),
        this->get_object_mesh_shading_type(node, "shading")
    );
}

// --------------------------------------------------------------------------------------

Light * Loader::build_light (const JSONNode & node) const {

    string type = this->get_string(node, "type");

    const JSONNode * intensity = Loader::get(node, "intensity", JSON_NODE);
    float ir = this->get_float(*intensity, "r");
    float ig = this->get_float(*intensity, "g");
    float ib = this->get_float(*intensity, "b");

    if (type == "ambient")
        return this->build_light_ambient(node, ir, ig, ib);
    else if (type == "punctual")
        return this->build_light_punctual(node, ir, ig, ib);

    throw string("Unknown light type \"") + type + "\" when parsing scene file";
}

AmbientLight * Loader::build_light_ambient (const JSONNode & node, const float ir, const float ig, const float ib) const {

    return new AmbientLight(ir, ig, ib);
}

PunctualLight * Loader::build_light_punctual (const JSONNode & node, const float ir, const float ig, const float ib) const {

    const JSONNode * attenuation = Loader::get(node, "attenuation", JSON_NODE);

    return new PunctualLight(

        ir, ig, ib,

        this->get_point(node, "position"),

        this->get_float(*attenuation, "constant"),
        this->get_float(*attenuation, "linear"),
        this->get_float(*attenuation, "quadratic")
    );
}

Scene * Loader::build_scene (const JSONNode & node) const {

    Scene * scene = new Scene(
        this->get_string(node, "title").c_str(),
        this->get_color(node, "background-color")
    );

    const JSONNode * objects_node = Loader::get(node, "objects", JSON_NODE, false);
    const JSONNode * lights_node = Loader::get(node, "lights", JSON_NODE, false);

    if (objects_node != nullptr)
        for (const auto & node : *objects_node)
            scene->add_object(
                node.name().c_str(),
                *(this->build_object(node))
            );

    if (lights_node != nullptr)
        for (const auto & node : *lights_node)
            scene->add_light(
                node.name().c_str(),
                *(this->build_light(node))
            );

    return scene;
}

Camera * Loader::build_camera (const JSONNode & node) const {

    const JSONNode * viewport_node = Loader::get(node, "viewport", JSON_NODE);

    return new Camera(

        this->get_point(node, "eye"),
        this->get_point(node, "look-at"),
        this->get_vector(node, "up-dir"),

        this->get_float(node, "focale"),
        this->get_float(node, "fov"),

        this->get_int(*viewport_node, "width"),
        this->get_int(*viewport_node, "height"),

        this->get_camera_projection_type(node, "projection-type"),

        this->get_int(node, "trace-depth"),
        this->get_int(node, "aa-depth"),
        this->get_int(node, "aa-threshold"),

        *(this->scene)
    );
}

Camera::projection_type Loader::get_camera_projection_type (const JSONNode & node, const char * name) const {

    string projection = this->get_string(node, name);

    if (projection == "perspective")
        return Camera::PROJECTION_PERSPECTIVE;

    else if (projection == "orthographic")
        return Camera::PROJECTION_ORTHOGRAPHIC;

    throw string("Invalid projection type");
}

Mesh::shading_type Loader::get_object_mesh_shading_type (const JSONNode & node, const char * name) const {

    const JSONNode * shading_node = Loader::get(node, name, JSON_STRING | JSON_NULL);

    switch ( shading_node->type() ) {

        case JSON_STRING: {

            string shading = shading_node->as_string();

            if (shading == "gouraud" || shading == "color")
                return Mesh::SHADING_GOURAUD;
            else if (shading == "phong" || shading == "normal")
                return Mesh::SHADING_PHONG;
            else if (shading == "flat")
                return Mesh::SHADING_NONE;

            throw string("Invalid shading type");
        }
            break;

        case JSON_NULL:
            return Mesh::SHADING_NONE;
            break;
    }
}
