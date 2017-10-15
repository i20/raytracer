#include <cstdlib>
#include <iostream>
#include <ctime>
#include <sstream>
#include <string>
#include <stdio.h>
#include <stdint.h>

#include <xmmintrin.h>

#define N 500000000

using namespace std;

// It is very important to use __mm_load_ps and __mm_store_ps when manipulating __m128 registers
// The cast from float* to __m128* theorically works but only if float* is 16 byte aligned
// Also it seems that it is not possible to force 16 byte alignment for local array vars (http://stackoverflow.com/questions/841433/gcc-attribute-alignedx-explanation) therefore it compromises the whole thing
// __m128 are specific SSE registers and we HAVE TO use it, but keep in mind that they are just a few then we need to retrieve the result in the main memory afterwards
// http://www.gamedev.net/topic/526505-simd-sse-intrinsics-code-wont-work/

float timer (const clock_t t) {

    return ((float)(clock() - t)) / CLOCKS_PER_SEC;
}

void dot_sisd (const float v1[4], const float v2[4]) {

    float sum = 0;

    for (uint8_t i = 0; i < 4; i++)
        sum += v1[i] * v2[i];

    //return sum;
}

void dot_simd_dirty (const float v1[4], const float v2[4]) {

    __m128 r = _mm_mul_ps(*(__m128*)v1, *(__m128*)v2);
    r = _mm_add_ps(r, _mm_shuffle_ps(r, r, _MM_SHUFFLE(0, 1, 2, 3)));
    r = _mm_add_ps(r, _mm_shuffle_ps(r, r, _MM_SHUFFLE(2, 3, 0, 1)));

    //return *(float*)&r;
}

void dot_simd_clean (__m128 v1, __m128 v2) {

    float sum;

    __m128 r = _mm_mul_ps(v1, v2);
    r = _mm_add_ps(r, _mm_shuffle_ps(r, r, _MM_SHUFFLE(0, 1, 2, 3)));
    r = _mm_add_ps(r, _mm_shuffle_ps(r, r, _MM_SHUFFLE(2, 3, 0, 1)));

    // _mm_store_ss(&sum, r);

    // return sum;
}

int main (void) {

    clock_t t;

    float v1_sisd[4] = {1., 2., 3., 4.};
    float v2_sisd[4] = {4., 3., 2., 1.};

    __m128 v1_simd = _mm_loadu_ps(v1_sisd);
    __m128 v2_simd = _mm_loadu_ps(v2_sisd);

    t = clock();
    for (int i = 0; i < N; i++) dot_sisd(v1_sisd, v2_sisd);
    cout << "SISD = " << timer(t) << endl;

    t = clock();
    for (int i = 0; i < N; i++) dot_simd_dirty(v1_sisd, v2_sisd);
    cout << "SIMD DIRTY = " << timer(t) << endl;

    t = clock();
    for (int i = 0; i < N; i++) dot_simd_clean(v1_simd, v2_simd);
    cout << "SIMD CLEAN = " << timer(t) << endl;

    return EXIT_SUCCESS;
}