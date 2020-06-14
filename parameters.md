#CAMERA

    TODO

#COLOR

    c = [R, G, B, A] coordinates, 0 <= x <= 255

        @ To implement to support transparent textures
        A = transparency

#OBJECT

    is_closed = is the object a closed shape ?

        @ Implement is_closed = true for cylinders
        - can be choosen for certain shapes (cylinder)
        - determined for spheres, planes, meshes ...
        - allow back face culling optimisation for meshes

    a = ambient light intensity on each canal, [0;1;inf]

        - values above 1 can be used to simulate fluorescence

    d = diffuse light intensity on each canal, [0;1;inf]

        - values above 1 can be used to simulate fluorescence

    s = specularity intensity on each canal [0;inf]

        s = 0 : no specular response

    r = reflection coefficient

        - r+t = [0;1]
        - part of enlightment due to reflection

    t = transmission coefficient

        - r+t = [0;1]
        - part of enlightment due to transmission

    n = refraction coefficient

    g = specularity, brillance, smoothness, shininess, glossiness

        - 1 <= g < +inf
        - relevant only if s != 0
        - cannot be 0 since pow(x, 0) is always 1 and then does not depends on x anymore
        - under 1, pow tends to act as pow(x, 0) and therefore not good neither
        - the greatest g, the smaller/brightest the light patch
        - a great value simulate metallic material

#CYLINDER

    radius = radius of the cylinder, 0 <= x < +inf

    @ not sure if implemented
    @ isn't it redundant with height ?
    infinite = must cylinder be an infinite one ?

    height = height to shrink the cylinder at

#INTERSECTION

    t = distance from ray origin to intersection

#LIGHT

    i = emission intensity on each canal [ iR, iG, iB ]

        0 <= x < +inf

    c = constant attenuation coefficient

        0 <= c <= 1
        c = 1 : no light, it is completely attenuated by default
        c = 0 : attenuation is only linked to distance with l and q

    l = linear (in distance) attenuation coefficient, the farest the light is, the dimmest it becomes

        0 <= l < +inf
        the greatest l is, the more aggressive is attenuation over distance

    q = quadratic (in distance) attenuation coefficient, similar to l but with square of distance

        0 <= q < +inf
