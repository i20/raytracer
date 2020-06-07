#include <string>
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

            this->normals[p1][j] += face.normal[j];
            this->normals[p2][j] += face.normal[j];
            this->normals[p3][j] += face.normal[j];
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

    const uintmax_t nb_faces = this->triangles.size();
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

bool Mesh::compute_intersection_final(Vector & true_normal_object, Vector & normal_object, const Point & point_object, const Triangle * t, const Ray & ray_object) const {

    float uu = 0,
          uv = 0,
          vv = 0,
          wu = 0,
          wv = 0;

    for (uint8_t i = 0; i < 3; i++) {

        const float w_v = point_object[i] - this->vertexes[t->p1][i];
        uu += t->u[i] * t->u[i];
        uv += t->u[i] * t->v[i];
        vv += t->v[i] * t->v[i];
        wu += w_v * t->u[i];
        wv += w_v * t->v[i];
    }

    const float det = uv * uv - uu * vv;

    // Coordinate on p1->p2 axis
    const float r = (uv * wv - vv * wu) / det;
    if (0 <= r && r <= 1) {

        // Coordinate on p1->p3 axis
        const float s = (uv * wu - uu * wv) / det;
        if (0 <= s && s <= 1 - r) {

            // Intersection point is on the triangle

            // In the particular case of meshes there is one more layer of normal, the raw triangle one
            // For other primitive shapes true normal (shaded) = raw normal (flat)
            // Light computation must always take place on true normal (shaded)
            // BUT normal correction must use raw normal (flat) as it is the only one to carry full geometry information
            const Vector raw_normal_object = t->normal;

            if (this->shading == Mesh::SHADING_PHONG) {

                // @todo Use ponderate average to take into consideration triangle's area when interpolating

                // Bilinear interpolation
                for (uint8_t i = 0; i < 4; i++)
                    true_normal_object[i] = (1 - (r+s)) * this->normals[t->p1][i] + r * this->normals[t->p2][i] + s * this->normals[t->p3][i];

                // @todo Maybe it is unnecessary here, remove if redundant
                true_normal_object = true_normal_object.normalize();
            }

            // @todo Implement Gouraud shading

            // Flat shading
            else true_normal_object = raw_normal_object; // ok normalized

            // Bump mapping
            if (this->normals_texture != nullptr) {
                // Bump texel is expressed in normal base which is different from object base
                // Notice that we use normal_object and not true_normal_object here to compute
                // the normal base, that way bump base is coherent with shading

                // @todo Compute bump_base, notice that when flat shaded then base x, y can be simply deduced from triangle plane
                //       But when Phong shaded, base x, y must be deduced with cross product from triangle (as in sphere bump_base)
                const Matrix bump_base = Matrix::TRANSFER(point_object, Vector::X /* @todo */, Vector::Y /* @todo */, true_normal_object);
                normal_object = bump_base * this->compute_texture_texel<Vector>(point_object, *this->normals_texture, t).normalize();
            }

            else normal_object = true_normal_object;

            // Detection of wether final normal should be corrected must be done on true normal as
            // bump mapping looses the information of object true geometry and leads to false positives
            // Phong shading also looses true geometry and therefore should not be taken into account
            // otherwise generates enlightened artifacts on hidden face edge @img(artifacts/shading)
            if (0 < raw_normal_object * ray_object.direction) {
                true_normal_object = true_normal_object * -1;
                normal_object = normal_object * -1;
            }

            return true;
        }
    }

    return false;
}

Color Mesh::compute_color_shape (const Point & point_object, const Triangle * triangle) const {

    return this->color + this->compute_texture_texel<Color>(point_object, *this->image_texture, triangle);
}
