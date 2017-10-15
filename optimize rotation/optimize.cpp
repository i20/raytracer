#include <string>
#include <iostream>

using namespace std;

void set_identity(string * m) {

    for(size_t i = 0; i < 4; i++){

        size_t idx = i * 4;

        for(size_t j = 0; j < 4; j++)
            m[idx + j] = i == j ? "1" : "0";
    }
}

void multiply(string * m, const string * m1, const string * m2) {

    for(size_t i = 0; i < 4; i++){

        size_t idx = i * 4;

        for(size_t j = 0; j < 4; j++){

            string v = "0";

            for(size_t k = 0; k < 4; k++){

                string add;

                if(m1[idx + k] == "0" || m2[k * 4 + j] == "0")
                    add = "0";

                else if(m1[idx + k] == "1")
                    add = m2[k * 4 + j];

                else if(m2[k * 4 + j] == "1")
                    add = m1[idx + k];

                else
                    add = m1[idx + k] + " * " + m2[k * 4 + j];

                if(add != "0"){

                    if(v == "0")
                        v = add;
                    else
                        v += " + " + add;
                }
            }

            if(v.find("+") != string::npos)
                v = "(" + v + ")";

            m[idx + j] = v;
        }
    }
}

void display(const string * m) {

    for(size_t i = 0; i < 4; i++){

        size_t idx = i * 4;

        cout << "[" << endl;

        for(size_t j = 0; j < 4; j++)
            cout << "    { " << m[idx + j] << " }" << endl;

        cout << "]" << endl;
    }
}

int main(const int argc, const char ** argv) {

    string rx[16], ry[16], rz[16], tmp1[16], tmp2[16];

    // ------------------------------------------------------------------------------------------------------------
    // VERSION NORMALE rx -> ry -> rz

    set_identity(rx);
    rx[5] = "cosx";
    rx[6] = "-sinx";
    rx[9] = "sinx";
    rx[10] = "cosx";

    set_identity(ry);
    ry[0] = "cosy";
    ry[2] = "siny";
    ry[8] = "-siny";
    ry[10] = "cosy";

    set_identity(rz);
    rz[0] = "cosz";
    rz[1] = "-sinz";
    rz[4] = "sinz";
    rz[5] = "cosz";

    multiply(tmp1, rx, ry);
    multiply(tmp2, tmp1, rz);

    cout << "[VERSION NORMALE]" << endl;
    display(tmp2);

    // ------------------------------------------------------------------------------------------------------------
    // VERSION INVERSE rz -> ry -> rx

    set_identity(rx);
    rx[5] = "cosx";
    rx[6] = "sinx";
    rx[9] = "-sinx";
    rx[10] = "cosx";

    set_identity(ry);
    ry[0] = "cosy";
    ry[2] = "-siny";
    ry[8] = "siny";
    ry[10] = "cosy";

    set_identity(rz);
    rz[0] = "cosz";
    rz[1] = "sinz";
    rz[4] = "-sinz";
    rz[5] = "cosz";

    multiply(tmp1, rz, ry);
    multiply(tmp2, tmp1, rx);

    cout << "[VERSION INVERSE]" << endl;
    display(tmp2);

    // ------------------------------------------------------------------------------------------------------------

    return 0;
}