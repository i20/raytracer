#include <cstdlib>

#include <iostream>

using namespace std;

int main () {

    int active_gpu_id;
    int gpus_count;
    cudaDeviceProp gpu_props;

    cudaGetDeviceCount(&gpus_count);

    if (gpus_count) {

        cudaGetDevice(&active_gpu_id);

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
    }

    else
        cout << "Sorry but no GPU available on your machine" << endl;

    return EXIT_SUCCESS;
}
