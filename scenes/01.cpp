// ambient : 0 <= x <= 1 ou +inf coefficient de reflection ambient
// diffuse : 0 <= x <= 1 ou +inf coefficient de reflection ambient
// glossyness : flou du cône de reflection speculaire (très petit pour un miroir)

// diffuse + specular should be <= 1

// TEXTURES
Image texture_mosaic("./textures/mosaic.ppm");
Image texture_grass("./textures/grass.ppm");

// OBJECTS
Plane px(

    Color::BLACK,         // color
    Point::O,           // position
    false,              // glassy
    &texture_mosaic,    // texture
    1, 1, 1,            // ambient
    1, 1, 1,            // diffuse
    0, 0, 0,            // specular
    0,                  // reflection
    0,                  // n
    0,                  // glossyness
    Vector::X,          // height dir
    Vector::Y,          // width dir
    2,                  // height
    2                   // width
);

Plane py(Color::GREEN, Point::O, false, nullptr, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, Vector::Y, Vector::Z, 2, 2);
Plane pz(Color::BLUE, Point::O, false, nullptr, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, Vector::Z, Vector::Y, 2, 2);

Sphere sphere(

    Color::ORANGE,  // color
    Point::O,       // center
    false,          // glassy
    nullptr,           // texture
    1, 1, 1,        // ambient
    1, 1, 1,        // diffuse
    1, 1, 1,        // specular
    1,              // reflection
    1.33,           // n
    100,              // glossyness
    .5,             // radius
    Vector::Z,      // z dir
    Vector::Y       // y dir
);

Cylinder x_axis(

    Color::RED, // color
    Point::O,   // position
    false,      // glassy
    nullptr,       // texture
    1, 1, 1,    // ambient
    1, 1, 1,    // diffuse
    .3, .3, .3, // specular
    1,          // reflection
    0,          // n
    10,         // glossyness
    .025,       // radius
    Vector::X,  // height dir
    Vector::Y,  // width dir
    1.5,        // height
    true        // closed
);

Cylinder y_axis(Color::GREEN, Point::O, false, nullptr, 1, 1, 1, 1, 1, 1, .3, .3, .3, 1, 0, 10, .025, Vector::Y, Vector::Z, 1.5, true);
Cylinder z_axis(Color::BLUE, Point::O, false, nullptr, 1, 1, 1, 1, 1, 1, .3, .3, .3, 1, 0, 10, .025, Vector::Z, Vector::X, 1.5, true);

Cylinder cylinder(

    Color::RED, // color
    Point::O,   // position
    false,      // glassy
    nullptr,       // texture
    1, 1, 1,    // ambient
    1, 1, 1,    // diffuse
    1, 1, 1,    // specular
    1,          // reflection
    0,          // n
    .1,         // glossyness
    .15,        // radius
    Vector::Z,  // height dir
    Vector::X,  // width dir
    .75,        // height
    true        // closed
);

// Mesh cow_mesh(

//     Color::ORANGE,          // color
//     Point::O,               // position
//     false,                  // glassy
//     nullptr,                   // texture
//     1, 1, 1,                // ambient
//     1, 1, 1,                // diffuse
//     1, 1, 1,                // specular
//     1,                      // reflection
//     1.33,                   // n
//     100,                    // glossyness
//     "./meshes/cow.off",     // off file
//     Vector::Z,              // z dir
//     Vector::Y,              // y dir
//     true,                   // octree
//     MESH_PHONG_SHADING      // shading
// );

// Mesh shark_mesh(

//     Color::ORANGE,          // color
//     Point::O,               // position
//     false,                  // glassy
//     nullptr,                   // texture
//     1, 1, 1,                // ambient
//     1, 1, 1,                // diffuse
//     1, 1, 1,                // specular
//     1,                      // reflection
//     1.33,                   // n
//     100,                    // glossyness
//     "./meshes/shark.off",   // off file
//     Vector::Z * -1,         // z dir
//     Vector::Y,              // y dir
//     true,                   // octree
//     MESH_PHONG_SHADING      // shading
// );

Mesh bunny_mesh(

    Color::ORANGE,          // color
    Point::O,               // position
    false,                  // glassy
    nullptr,                   // texture
    1, 1, 1,                // ambient
    1, 1, 1,                // diffuse
    1, 1, 1,                // specular
    1,                      // reflection
    1.33,                   // n
    100,                    // glossyness
    "./meshes/bunny.off",   // off file
    Vector(0, 1, 1.4),      // z dir
    Vector::Z,              // y dir
    true,                   // octree
    MESH_PHONG_SHADING      // shading
);

Mesh sphere_mesh(

    Color::ORANGE,          // color
    Point::O,               // position
    false,                  // glassy
    nullptr,                   // texture
    1, 1, 1,                // ambient
    1, 1, 1,                // diffuse
    1, 1, 1,                // specular
    1,                      // reflection
    1.33,                   // n
    100,                    // glossyness
    "./meshes/sphere.off",  // off file
    Vector::Z,              // z dir
    Vector::Y,              // y dir
    true,                   // octree
    MESH_PHONG_SHADING      // shading
);

// LIGHTS
AmbientLight ambient(.2, .2, .2);
PunctualLight pl1(.5, .5, .5, Point(5, 2, 0), 1, 0, .01);

// SCENE
Scene scene("Scene 01", Color::BLACK);

    scene.ambient_light = &ambient;

    scene.add_object("x-axis", x_axis);
    scene.add_object("y-axis", y_axis);
    scene.add_object("z-axis", z_axis);

    // scene.add_object("x-plane", px);
    // scene.add_object("y-plane", py);
    // scene.add_object("z-plane", pz);

    // scene.add_object("sphere-mesh", sphere_mesh);
    // scene.add_object("bunny-mesh", bunny_mesh);
    // scene.add_object("cow-mesh", cow_mesh);
    // scene.add_object("shark-mesh", shark_mesh);

    // scene.add_object("sphere-plain", sphere);
    // scene.add_object("cylinder-plain", cylinder);

    scene.add_light("light-1", pl1);

// CAMERA
Camera cam(

    Point(5, 5, 5), // eye
    Point::O,       // look at
    Vector::Y,      // up dir
    3,              // focale
    45,             // fov
    VIEWPORT_WIDTH,
    VIEWPORT_HEIGHT,
    Camera::PERSPECTIVE,
    2,              // tracing_depth
    0,              // aa_depth
    50,             // aa_threshold
    scene
);
