#include <algorithm>

#include "../headers/Color.hpp"
#include "../headers/Object.hpp"
#include "../headers/PunctualLight.hpp"

#include "../headers/Scene.hpp"

using namespace std;

void Scene::copy(const Scene & scene) {

    this->title = scene.title;

    this->objects = scene.objects;
    this->lights = scene.lights;

    this->bg_color = scene.bg_color;
}

Scene & Scene::operator=(const Scene & scene) {

    this->copy(scene);
    return *this;
}

Scene::Scene(const char * title, const Color & bg_color) :
    title(title),
    bg_color(bg_color) {}

Scene::Scene(const Scene & scene) :
    title(scene.title),

    objects(scene.objects),
    lights(scene.lights),

    bg_color(scene.bg_color) {}

// --------------------------------------------------------------------------------------

void Scene::add_object(const char * id, Object & object) {

    if (this->objects.find(id) != this->objects.end())
        throw string("ID already used when inserting object in scene");

    this->objects.insert( Scene::ObjectMap::value_type(id, &object) );
}

void Scene::rm_object(const char * id) {

    auto it = this->objects.find(id);

    if (it == this->objects.end())
        throw string("ID not found in scene for object");

    this->objects.erase(it);
}

Object & Scene::get_object(const char * id) {

    auto it = this->objects.find(id);

    if (it == this->objects.end())
        throw string("ID not found in scene for object");

    return *(it->second);
}

// --------------------------------------------------------------------------------------

void Scene::add_light(const char * id, Light & light) {

    if (this->lights.find(id) != this->lights.end())
        throw string("ID already used when inserting light in scene");

    this->lights.insert( Scene::LightMap::value_type(id, &light) );
}

void Scene::rm_light(const char * id) {

    auto it = this->lights.find(id);

    if (it == this->lights.end())
        throw string("ID not found in scene for light");

    this->lights.erase(it);
}

Light & Scene::get_light(const char * id) {

    auto it = this->lights.find(id);

    if (it == this->lights.end())
        throw string("ID not found in scene for light");

    return *(it->second);
}
