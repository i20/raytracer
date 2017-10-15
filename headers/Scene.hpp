#ifndef _SCENE_HPP
#define _SCENE_HPP

#include <string>
#include <unordered_map>

#include "../headers/Color.hpp"
#include "../headers/Object.hpp"

// Circular reference
class Light;

using namespace std;

class Scene {

    public:

        typedef unordered_map<string, Object*> ObjectMap;
        typedef unordered_map<string, Light*> LightMap;

        // NONE = no boxes used, objects are directly tested
        // SIMPLE = all boxes are level 0 meaning they are all tested for intersection
        // NESTED = boxes can contain other boxes has child to test only needed boxes
        //enum grid_type_enum { NONE, SIMPLE, NESTED };
        //Scene::grid_type_enum grid_type;

        string title;

        Scene::ObjectMap objects;
        Scene::LightMap lights;

        Color bg_color;

        Scene(const char * title, const Color & bg_color);
        Scene(const Scene & scene);

        Scene & operator=(const Scene & scene);

        void add_object(const char * id, Object & object);
        void rm_object(const char * id);
        Object & get_object(const char * id);

        void add_light(const char * id, Light & light);
        void rm_light(const char * id);
        Light & get_light(const char * id);

    protected:

        void copy(const Scene & scene);
};

#endif
