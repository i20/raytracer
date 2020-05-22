MyRayTracer

INSTALL
- download SDL and SDL_ttf .dmg from official site
- mount .dmg and copy .framework folder to ./libraries
- make

EXECUTE
$ ./raytracer <scene name>



IDEAS

- Pre-projection to determine pixels needing a ray before radiation
- oriented object (ex with plane, visible on one side not on the other)
- Photon Mapping
- Bump Mapping (Perlin)
- Voxel (3D Grid)
- scene description file
- use cuda computing
- use SIMD
- use MPI

Voxels must be used in an intersection tree
The voxel tree has a root voxel, each tree node has exactly eight or no child


TODO

- move Vector/Point to macros, see if Matrix can be moved too
- handle refracted ray when coming out from refracting object, what is outside n?
- implement scenario programming (ex: load scene + wait 2 sec + move ...)
- implement "include" syntax in json to include scene parts
- strange illumination when punctual light is very near a surface

some triangles are flat shaded on shadow edge : image 1456688795
    - called terminator problem in http://geekshavefeelings.com/x/wp-content/uploads/2010/03/Its-Really-Not-a-Rendering-Bug-You-see....pdf
    - it is not a bug
    - this is due to the way shadow rays are traced
    - shadow rays start from the intersection point which is on flat triangles
    - then ray cannot reach the light since curvature is not interpolated, only normals
    ? is there a way to interpolate curvature based on triangle normals to correct this
    refs:
        https://people.mpi-inf.mpg.de/~rzayer/pappe/PG04.pdf
        http://www.ijesit.com/Volume%202/Issue%203/IJESIT201303_48.pdf
        http://www.math.zju.edu.cn/yxn/papers/normalshading.pdf
        http://ljk.imag.fr/membres/Stefanie.Hahmann/PUBLICATIONS/HB03.pdf
        ./pdf/201112-Curvature Estimation for Meshes Based on Vertex-Normal Triangles.pdf

AVAILABLE OPTIMISATIONS:

- SIMD computing
- octree containment
- hidden triangles avoiding for meshes
- Sobel edges detection w/ antialiasing

AVAILABLE OPTIONS:

- Phong or flat shading for meshes
- antialiasing

