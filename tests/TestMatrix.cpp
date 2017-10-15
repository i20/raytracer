#include <iostream>

#include "../headers/Matrix.hpp"
#include "../headers/Vector.hpp"
#include "../headers/Point.hpp"

using namespace std;

#define PI 3.14159265359

int main (const int argc, const char ** argv) {

    Point pi(1, 2, 0);

    // ------------------------------------------------------------------------------------------------------------
    // IDENTITY

    Matrix mi = Matrix::IDENTITY();

    Point pii1 = mi * pi;

    cout << pi.to_string() << " has been transformed by identity giving " << pii1.to_string() << endl;

    // ------------------------------------------------------------------------------------------------------------
    // TRANSLATION

    Vector vt(2, -1, -6);

    Matrix mt = Matrix::TRANSLATION(vt);

    Point pt = mt * pi;

    Point pii2 = mt.invert() * pt;

    cout << pi.to_string() << " has been translated by " << vt.to_string() << " giving " << pt.to_string() << " reversion gives " << pii2.to_string() << endl;

    // ------------------------------------------------------------------------------------------------------------
    // ROTATION

    Vector vr(0, 1, 0);
    double angle = 90;

    Matrix mr = Matrix::ROTATION(vr, angle);

    Point pr = mr * pi;

    Point pii3 = mr.invert() * pr;

    cout << pi.to_string() << " has been rotated by " << angle << "° around " << vr.to_string() << " giving " << pr.to_string() << " reversion gives " << pii3.to_string() << endl;

    // ------------------------------------------------------------------------------------------------------------
    // HOMOTHETY

    Vector vh(2, .5, 1);

    Matrix mh = Matrix::HOMOTHETY(vh);

    Point ph = mh * pi;

    Point pii4 = mh.invert() * ph;

    cout << pi.to_string() << " has been scaled by " << vh.to_string() << " giving " << ph.to_string() << " reversion gives " << pii4.to_string() << endl;

    // ------------------------------------------------------------------------------------------------------------
    // BASE CHANGE

    Point po(0, 0, 0);

    Vector vx(0, -1, 0);
    Vector vy(1, 0, 0);
    Vector vz(0, 0, 1);

    Matrix mp = Matrix::TRANSFER(po, vx, vy, vz);

    Point pp = mp * pi;

    Point pii5 = mp.invert() * pp;

    cout << pi.to_string() << " has been projected into base {" << po.to_string() << " " << vx.to_string() << " " << vy.to_string() << " " << vz.to_string() << "} giving " << pp.to_string() << " reversion gives " << pii5.to_string() << endl;

    // ------------------------------------------------------------------------------------------------------------

    return 0;
}