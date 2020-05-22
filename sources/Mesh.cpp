#include <sstream>
#include <fstream>
#include <unordered_map>
#include <iostream>

#include "../headers/Mesh.hpp"
#include "../headers/Triangle.hpp"
#include "../headers/Octree.hpp"

using namespace std;

Mesh::Mesh(
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
) :
    Object(
        color,
        position,
        false,
        is_glassy,

        image_texture,
        image_texture_filtering,

        normals_texture,
        normals_texture_filtering,

        ar, ag, ab,
        dr, dg, db,
        sr, sg, sb,
        r, n, g,
        z_dir, y_dir
    ), image_texture_mapping(), normals_texture_mapping(), shading(shading) {

    this->load_file(off_file, use_octree);

    if (this->image_texture != nullptr)
        this->load_texture_mapping(image_texture_mapping, this->image_texture_mapping);

    if (this->normals_texture != nullptr)
        this->load_texture_mapping(normals_texture_mapping, this->normals_texture_mapping);
}

Mesh::Mesh(const Mesh & mesh) :
    Object(mesh),
    triangles(mesh.triangles),
    vertexes(mesh.vertexes),
    normals(mesh.normals),

    image_texture_mapping(mesh.image_texture_mapping),
    normals_texture_mapping(mesh.normals_texture_mapping),

    shading(mesh.shading) {}

Mesh & Mesh::operator=(const Mesh & mesh) {

    this->copy(mesh);
    this->triangles = mesh.triangles;
    this->vertexes = mesh.vertexes;
    this->normals = mesh.normals;

    this->image_texture_mapping = mesh.image_texture_mapping;
    this->normals_texture_mapping = mesh.normals_texture_mapping;

    this->shading = mesh.shading;
    return *this;
}

void Mesh::load_file (const char * off_file, const bool use_octree) {

    ifstream inf(off_file);

    if ( !inf.is_open() ) throw string("Missing or locked texture file for mesh");

    string line;
    getline(inf, line);

    // First line must contain OFF
    if ( line != "OFF" ) throw string("Texture file is malformated");

    uintmax_t nb_points, nb_faces, nb_edges;
    inf >> nb_points >> nb_faces >> nb_edges;

    this->vertexes.reserve(nb_points);
    // Average vertex normals used for interpolation w/ Phong shading
    this->normals.reserve(nb_points);

    float octree_minx,
          octree_miny,
          octree_minz,
          octree_maxx,
          octree_maxy,
          octree_maxz;

    for (uintmax_t i = 0; i < nb_points; i++) {

        float x, y, z;
        inf >> x >> y >> z;

        this->vertexes.push_back( Point(x, y, z) );
        this->normals.push_back( Vector(0, 0, 0) );

        // Compute wrapping octree boundaries in the meantime
        if (use_octree) {

            if (x < octree_minx || i == 0) octree_minx = x;
            if (y < octree_miny || i == 0) octree_miny = y;
            if (z < octree_minz || i == 0) octree_minz = z;

            if (octree_maxx < x || i == 0) octree_maxx = x;
            if (octree_maxy < y || i == 0) octree_maxy = y;
            if (octree_maxz < z || i == 0) octree_maxz = z;
        }
    }

    if (use_octree)
        this->octree = new Octree(Point(octree_minx, octree_miny, octree_minz), Vector(

            octree_maxx - octree_minx,
            octree_maxy - octree_miny,
            octree_maxz - octree_minz
        ));

    this->triangles.reserve(nb_faces);
    // (edge => adjacent faces count) map for closed mesh detection
    unordered_map<string, uint8_t> edge_valence;

    for (uintmax_t i = 0; i < nb_faces; i++) {

        // nb_sides is always 3 in our case
        uintmax_t nb_sides, p1, p2, p3;
        inf >> nb_sides >> p1 >> p2 >> p3;

        this->triangles.push_back(Triangle(*this, i, p1, p2, p3));

        // Do not backup the new triangle in a local var since we need to store a pointer to it in the octree
        const Triangle & face = this->triangles.back();

        if (use_octree)
            this->octree->append(face);

        // Update average normal at each of the tree vertex
        for (uint8_t j = 0; j < 3; j++) {

            this->normals[p1].v[j] += face.normal.v[j];
            this->normals[p2].v[j] += face.normal.v[j];
            this->normals[p3].v[j] += face.normal.v[j];
        }

        // Count edges valence
        Mesh::count_edge_valence_subroutine(edge_valence, p1, p2);
        Mesh::count_edge_valence_subroutine(edge_valence, p1, p3);
        Mesh::count_edge_valence_subroutine(edge_valence, p2, p3);
    }

    // Pay attention to normalize average normal for further interpolation computation
    for (auto & normal : this->normals)
        normal = normal.normalize();

    // Detect if mesh is closed

    this->is_closed = true;

    for (const auto & edge_entry : edge_valence) {

        if (edge_entry.second == 1) {

            // Boundary edge detected
            this->is_closed = false;
            break;
        }
    }

    inf.close();
}

void Mesh::load_texture_mapping (const char * texture_mapping_file, Mesh::TextureMapping & texture_mapping) const {

    ifstream inf(texture_mapping_file);

    if ( !inf.is_open() ) throw string("Missing or locked texture mapping file for mesh");

    string line;
    getline(inf, line);

    // First line must contain TEX
    if ( line != "TEX" ) throw string("Texture mapping file is malformated");

    uintmax_t nb_faces = this->triangles.size();
    texture_mapping.reserve(nb_faces);

    for (uintmax_t i = 0; i < nb_faces; i++) {

        // /!\ two adjacent faces may be mapped with non adjacent parts of the texture
        // <texture u p1> <texture v p1> <texture u p2> <texture v p3> <texture u p3>

        array< array<float, 2> , 3> coords;

        for (uint8_t k = 0; k < 3; k++)
            inf >> coords[k][0] >> coords[k][1];

        texture_mapping.push_back( coords );
    }

    inf.close();
}

void Mesh::count_edge_valence_subroutine(unordered_map<string, uint8_t> & edge_valence, const uintmax_t pa, const uintmax_t pb) {

    // Be careful, edge can be pa-pb or pb-pa in the map

    stringstream ss;
    ss << pa << "-" << pb;
    string k1 = ss.str();

    auto k1it = edge_valence.find(k1);

    if (k1it != edge_valence.end())
        k1it->second++;

    else {

        ss.str(""); // reset stream content
        ss << pb << "-" << pa;
        string k2 = ss.str();

        auto k2it = edge_valence.find(k2);

        if (k2it != edge_valence.end())
            k2it->second++;

        else
            edge_valence.insert( unordered_map<string, uint8_t>::value_type(k1, 1) );
    }
}

Mesh::~Mesh() {

    if (this->octree != nullptr)
        delete this->octree;
}

TTPairList Mesh::compute_intersection_ts(const vector<const Octree *> & octrees, const Ray & ray_object) const {

    TTPairList ts;

    if (octrees.size() != 0) {

        // Foreach octree crossed by the ray
        for (const auto poctree : octrees) {

            // Foreach triangle attached to the octree
            for (const auto ptriangle : poctree->triangles)
                this->compute_intersection_ts_subroutine(ts, *ptriangle, ray_object);
        }
    }

    else {

        // Foreach triangle composing the mesh
        for (const auto & triangle : this->triangles)
            this->compute_intersection_ts_subroutine(ts, triangle, ray_object);
    }

    return ts;
}

bool Mesh::compute_intersection_final(Vector & normal_object, const Point & point_object, const Triangle * t) const {

    float uu = 0,
          uv = 0,
          vv = 0,
          wu = 0,
          wv = 0;

    for (uint8_t i = 0; i < 3; i++) {

        float w_v = point_object.p[i] - this->vertexes[t->p1].p[i];
        uu += t->u.v[i] * t->u.v[i];
        uv += t->u.v[i] * t->v.v[i];
        vv += t->v.v[i] * t->v.v[i];
        wu += w_v * t->u.v[i];
        wv += w_v * t->v.v[i];
    }

    float det = uv * uv - uu * vv;

    // Coordinate on p1->p2 axis
    float r = (uv * wv - vv * wu) / det;
    if (0 <= r && r <= 1) {

        // Coordinate on p1->p3 axis
        float s = (uv * wu - uu * wv) / det;
        if (0 <= s && s <= 1 - r) {

            // Intersection point is on the triangle

            if (this->shading == Mesh::SHADING_PHONG) {

                // @todo Use ponderate average to take into consideration triangle's area when interpolating
                // @todo Phong shading seems to generate enlightened artifacts on hidden face edge @img(artifacts/shading)

                float v[4];
                // Bilinear interpolation
                for (uint8_t i = 0; i < 4; i++)
                    v[i] = (1 - (r+s)) * this->normals[t->p1].v[i] + r * this->normals[t->p2].v[i] + s * this->normals[t->p3].v[i];

                normal_object = Vector(v);
            }

            // @todo Implement Gouraud shading

            // Flat shading
            else
                normal_object = t->normal;

            if (this->normals_texture != nullptr)
                normal_object = normal_object + this->compute_texture_texel<Vector>(point_object, *this->normals_texture, t);

            return true;
        }
    }

    return false;
}

Color Mesh::compute_color_shape (const Point & point_object, const Triangle * triangle) const {

    return this->color + this->compute_texture_texel<Color>(point_object, *this->image_texture, triangle);
}

// @todo Clean up
string Mesh::to_string() const {

    stringstream ss;
    ss << "Mesh{triangles=>[";

    /*vector<Triangle>::const_iterator cbegin = this->triangles.begin(), cend = this->triangles.end();
    for (vector<Triangle>::const_iterator cit = cbegin; cit != cend; cit++) {

        if (cit != cbegin)
            ss << ",";

        ss << cit->to_string();
    }*/

    ss << "]}";
    return ss.str();
}
