#ifndef _TEXTURE_HPP
#define _TEXTURE_HPP

#include <stdint.h>
#include <vector>
#include <fstream>
#include <cmath>

using namespace std;

// Texture must be read left to right, top to bottom
// i >
// j v

template <class T>
class Texture {

    public:

        enum filtering_type { FILTERING_NONE, FILTERING_BILINEAR, FILTERING_TRILINEAR, FILTERING_ANISOTROPIC };

        bool ok;
        uintmax_t width, height;
        vector<T> map;

        Texture(const char * file_name);
        Texture(const uintmax_t width, const uintmax_t height);
        Texture(const Texture<T> & texture);

        Texture<T> & operator=(const Texture<T> & texture);

        void set_texel(const uintmax_t i, const uintmax_t j, const T & texel);
        T get_texel(const uintmax_t i, const uintmax_t j) const;
        T get_texel_by_uv (const float u, const float v, const bool smooth) const;

        bool print(const char * file_name) const;
        bool print(const char * file_name, const uintmax_t istart, const uintmax_t jstart, const uintmax_t iend, const uintmax_t jend) const;

    private:
        uintmax_t get_uint(ifstream & inf) const;
        void ignore_comments(ifstream & inf) const;
};

template <class T>
void Texture<T>::ignore_comments (ifstream & inf) const {

    string line;

    while (inf.peek() == '#')
        getline(inf, line);
}

// Does not support comments at the end of a non-comment line
template <class T>
Texture<T>::Texture (const char * file_name) {

    ifstream inf(file_name);

    if ( !inf.is_open() ) {

        this->ok = false;
        return;
    }

    string line;
    getline(inf, line);

    // PNM : Portable Anymap, global name for all formats below
    //   PBM : Portable Bitmap, binary = P4, ASCII = P1, white/black
    //   PGM : Portable Graymap, binary = P5, ASCII = P2, shades of grey
    //   PPM : Portable Pixmap, binary = P6, ASCII = P3, color

    // Handle only P6
    if ( line != "P6" ) {

        this->ok = false;
        return;
    }

    this->ignore_comments(inf);

    this->width = this->get_uint(inf);

    inf.ignore();

    this->height = this->get_uint(inf);

    inf.ignore();

    uintmax_t intensity = this->get_uint(inf);

    if (intensity != 255) {

        this->ok = false;
        return;
    }

    inf.ignore();

    this->ignore_comments(inf);

    uintmax_t nb_texels = this->width * this->height;
    this->map.reserve(nb_texels);

    uint8_t t[3];
    for (uintmax_t i = 0; i < nb_texels; i++) {

        for (uint8_t k = 0; k < 3; k++)
            t[k] = inf.get();

        this->map.push_back( T(t[0], t[1], t[2]) );
    }

    inf.close();

    this->ok = true;
}

template <class T>
Texture<T>::Texture(const uintmax_t width, const uintmax_t height) : ok(true), width(width), height(height), map(width * height) {}

template <class T>
Texture<T>::Texture(const Texture<T> & texture) : ok(texture.ok), width(texture.width), height(texture.height), map(texture.map) {}

template <class T>
Texture<T> & Texture<T>::operator=(const Texture<T> & texture) {

    this->ok = texture.ok;
    this->width = texture.width;
    this->height = texture.height;
    this->map = texture.map;

    return *this;
}

template <class T>
void Texture<T>::set_texel(const uintmax_t i, const uintmax_t j, const T & texel) {

    this->map[i + j * this->width] = texel;
}

template <class T>
T Texture<T>::get_texel(const uintmax_t i, const uintmax_t j) const {

    return this->map[i + j * this->width];
}

template <class T>
T Texture<T>::get_texel_by_uv (const float u, const float v, const bool smooth) const {

    // TODO Enable setting smooth from the scene's json

    // -------------------------------------
    // |           |           |           |
    // |           |           |           |
    // |    0,0 . .|. . 1,0 . .|. . 2,0    |
    // |     .     |     .     |     .     |
    // |     .     |     .     |     .     |
    // ------------A-----------B------------
    // |     .     |     .     |     .     |
    // |     .     |     .     |     .     |
    // |    0,1 . .|. . 1,1 . .|. . 2,1    |
    // |     .     |     .     |     .     |
    // |     .     |     .     |     .     |
    // ------------C-----------D------------
    // |     .     |     .     |     .     |
    // |     .     |     .     |     .     |
    // |    0,2 . .|. . 1,2 . .|. . 2,2    |
    // |           |           |           |
    // |           |           |           |
    // -------------------------------------

    // p1 p2 p3
    // p4 p5 p6
    // p7 p8 p9

    // The idea is to first determine the ghost square we are in and then apply the correct bilinear interpolation to find the smoothed color

    // u = 0      fi = 0.5   i = 0   di = .5 center of first texel
    // u = 1      fi = 2.5   i = 2   di = .5 center of last texel
    // u = 0.5    fi = 1.5   i = 1   di = .5 center of mid texel
    // u = 0.75   fi = 2     i = 2   di = 0  start edge of last texel

    // Real coords of the texel
    float fi, fj;
    fi = u * (this->width - 1) + .5;
    fj = v * (this->height - 1) + .5;

    // Raw texel coords, the basic approach would be to return this texel without interpolation
    uintmax_t i, j;
    i = (uintmax_t)fi;
    j = (uintmax_t)fj;

    T p5 = this->get_texel(i, j);

    if (!smooth) return p5;

    // Decimal part of the real coords
    float di, dj;
    di = modf(fi, &fi);
    dj = modf(fj, &fj);

    // Corrected interpolation coords depending on considered ghost square
    // 0 <= r, s < .5
    float r, s;

    T & a1 = p5; // Alias for interpolation calculus semantic
    T a2, b1, b2;

    // A or C
    if (di <= .5) {

        r = i != 0 ? .5 - di : 0; // <

        // A
        if (dj <= .5) {

            s = j != 0 ? .5 - dj : 0; // ^

            if (i > 0)                                      /* p4 */ a2 = this->get_texel(i-1, j);
            if (j > 0)                                      /* p2 */ b1 = this->get_texel(i, j-1);
            if (i > 0 && j > 0)                             /* p1 */ b2 = this->get_texel(i-1, j-1);
        }
        // C
        else {

            s = j != this->height - 1 ? dj - .5 : 0; // v

            if (i > 0)                                      /* p4 */ a2 = this->get_texel(i-1, j);
            if (j < this->height - 1)                       /* p8 */ b1 = this->get_texel(i, j+1);
            if (i > 0 && j < this->height - 1)              /* p7 */ b2 = this->get_texel(i-1, j+1);
        }
    }
    // B or D
    else {

        r = i != this->width - 1 ? di - .5 : 0; // >

        // B
        if (dj <= .5) {

            s = j != 0 ? .5 - dj : 0; // ^

            if (i < this->width - 1)                        /* p6 */ a2 = this->get_texel(i+1, j);
            if (j > 0)                                      /* p2 */ b1 = this->get_texel(i, j-1);
            if (i < this->width - 1 && j > 0)               /* p3 */ b2 = this->get_texel(i+1, j-1);
        }
        // D
        else {

            s = j != this->height - 1 ? dj - .5 : 0; // v

            if (i < this->width - 1)                         /* p6 */ a2 = this->get_texel(i+1, j);
            if (j < this->height - 1)                        /* p8 */ b1 = this->get_texel(i, j+1);
            if (i < this->width - 1 && j < this->height - 1) /* p9 */ b2 = this->get_texel(i+1, j+1);
        }
    }

    // Interpolation recipient
    // TODO Handle interpolation with Texture<Vector> (cannot store in uint8_t)
    uint8_t c[3];
    for (uint8_t k = 0; k < 3; k++) {

        uint8_t ak, bk;
        ak = (1 - r) * a1[k] + r * a2[k];
        bk = (1 - r) * b1[k] + r * b2[k];
        c[k] = (1 - s) * ak + s * bk;
    }

    return T(c[0], c[1], c[2]);
}

template <class T>
uintmax_t Texture<T>::get_uint(ifstream & inf) const {

    uintmax_t i = 0;
    char t;

    while (true) {

        t = inf.peek();

        if ( t < '0' || '9' < t ) break;

        i = i * 10 + (t - '0');

        inf.ignore();
    }

    return i;
}

// http://netpbm.sourceforge.net/doc/ppm.html
// http://www.tomdalling.com/blog/modern-opengl/07-more-lighting-ambient-specular-attenuation-gamma/

template <class T>
bool Texture<T>::print (const char * file_name) const {

    ofstream outf(file_name);

    if ( !outf.is_open() )
        return false;

    // Meta data in ASCII
    outf << "P6" << endl;
    outf << this->width << " " << this->height << endl;
    outf << 255 << endl;

    for (const auto & texel : this->map)
        for (uint8_t i = 0; i < 3; i++)
            outf.put(texel[i]);

    outf.close();

    return true;
}

template <class T>
bool Texture<T>::print(const char * file_name, const uintmax_t istart, const uintmax_t jstart, const uintmax_t iend, const uintmax_t jend) const {

    ofstream outf(file_name);

    if ( !outf.is_open() )
        return false;

    // Meta data in ASCII
    outf << "P6" << endl;
    outf << (iend - istart + 1) << " " << (jend - jstart + 1) << endl;
    outf << 255 << endl;

    for (uintmax_t j = jstart; j < jend + 1; j++) {
        for (uintmax_t i = istart; i < iend + 1; i++) {

            T t = this->map[i + j * this->width];

            // TODO handle .c if T is Vector
            for (uint8_t k = 0; k < 3; k++)
                outf.put(t[k]);
        }
    }

    outf.close();

    return true;
}

#endif