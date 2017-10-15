#include <cstdlib>
#include <iostream>

#define N 10

using namespace std;

__global__
void saxpy_cuda (int n, float a, float * x, float * y) {

    int i = blockIdx.x * blockDim.x + threadIdx.x;

    /*if (i < n)*/ y[i] = a * x[i] + y[i];
}

int main () {

    int size = N * sizeof(float);

    int active_gpu_id;
    int gpus_count;
    cudaDeviceProp gpu_props;

    cudaGetDeviceCount(&gpus_count);

    if (gpus_count) {

        cudaGetDevice(&active_gpu_id);

        // char* name
        // int maxThreadsPerBlock;
        // int maxThreadsDim[3];
        // int maxGridSize[3];
        // int integrated
        // int pciBusID
        // int pciDeviceID

        cout << "There is " << gpus_count << " GPUs available on your machine which are :" << endl;

        for (int i = 0; i < gpus_count; i++) {

            cudaGetDeviceProperties(&gpu_props, i);
            cout << "- " << gpu_props.name << " (id=" << i << ")";
            if (gpu_props.integrated) cout << " [INTEGRATED]";
            if (i == active_gpu_id) cout << " [ACTIVE]";
            cout << endl;
            cout << "---> maxThreadsPerBlock = " << gpu_props.maxThreadsPerBlock << endl;
            cout << "---> maxThreadsDim = (" << gpu_props.maxThreadsDim[0] << ", " << gpu_props.maxThreadsDim[1] << ", " << gpu_props.maxThreadsDim[2] << ")" << endl;
            cout << "---> maxGridSize = (" << gpu_props.maxGridSize[0] << ", " << gpu_props.maxGridSize[1] << ", " << gpu_props.maxGridSize[2] << ")" << endl;
        }

        // cout << endl;
        // cout << "Wanna change GPU?" << endl;

        // cudaSetDevice(int device);

        // Host
        float x_h[N];
        float y_h[N];

        for (int i = 0; i < N; i++) {

            x_h[i] = i;
            y_h[i] = i + N;
        }

        // Device
        float * x_d;
        float * y_d;

        // (void **)
        cudaMalloc(&x_d, size);
        cudaMalloc(&y_d, size);

        cudaMemcpy(x_d, x_h, size, cudaMemcpyHostToDevice);
        cudaMemcpy(y_d, y_h, size, cudaMemcpyHostToDevice);

        saxpy_cuda <<<N, 1>>> (N, 2.0, x_d, y_d);

        cudaMemcpy(y_h, y_d, size, cudaMemcpyDeviceToHost);

        cudaFree(x_d);
        cudaFree(y_d);

        cout << "RESULT :";

        for (int i = 0; i < N; i++)
            cout << " " << y_h[i];

        cout << endl;
    }

    else
        cout << "Sorry but no GPU available on your machine" << endl;

    return EXIT_SUCCESS;
}