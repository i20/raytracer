#include <cstdlib>
#include <ctime>
#include <cstdint>

#include <iostream>
#include <sstream>

#include <omp.h>
#include <SDL2/SDL.h>
#include <SDL2_ttf/SDL_ttf.h>

#include "../headers/Vector.hpp"
#include "../headers/Term.hpp"
#include "../headers/Loader.hpp"

#define VIEWPORT_WIDTH 500
#define VIEWPORT_HEIGHT 500

#define STAT_WIDTH 100
#define STAT_HEIGHT 200

using namespace std;

struct rendering_data_s {

    double spf;
    float eye_x, eye_y, eye_z;
};

static double timer (const double t) {

    return omp_get_wtime() - t;
}

static void display_stats (SDL_Renderer * renderer, TTF_Font * font, const struct rendering_data_s & data) {

    static SDL_Rect stats_r = {

        /* x */ 10,
        /* y */ 10
    };

    static SDL_Color stats_c = {

        /* r */ 255,
        /* g */ 0,
        /* b */ 0,
        /* a */ 255
    };

    stringstream ss;
    ss << data.spf << " sec/frame" << " - ";
    ss << "eye at (" << data.eye_x << ", " << data.eye_y << ", " << data.eye_z << ")";

    SDL_Surface * stats_s = TTF_RenderText_Solid(font, ss.str().c_str(), stats_c);
    SDL_Texture * stats_t = SDL_CreateTextureFromSurface(renderer, stats_s);

    stats_r.w = stats_s->w;
    stats_r.h = stats_s->h;

    SDL_FreeSurface(stats_s);

    SDL_RenderCopy(renderer, stats_t, nullptr, &stats_r);
    SDL_DestroyTexture(stats_t);
}

int main (const int argc, const char ** argv) {

    try {

        // Require a command line argument
        if (argc < 2)
            throw string("Missing scene to load name as first argument");

        // Do not use clock() to calculate execution time since it does not see parallelized gain
        // http://stackoverflow.com/questions/10727849/no-performance-gain-after-using-openmp-on-a-program-optimize-for-sequential-runn?answertab=votes#tab-top

        double t;

        stringstream ss_scene_path;
        ss_scene_path << "./scenes/" << argv[1] << ".json";

        cout << Term::SC << "Loading scene \"" << argv[1] << "\" ..." << endl;
        t = omp_get_wtime();
        Loader loader(ss_scene_path.str().c_str());
        cout << Term::RC << Term::CLR << "Scene loading took " << Term::FGC_GREEN << timer(t) << Term::R << " seconds" << endl;

        SDL_Init(SDL_INIT_VIDEO);

        SDL_Window * window = SDL_CreateWindow(

            loader.scene->title.c_str(),
            SDL_WINDOWPOS_CENTERED,
            SDL_WINDOWPOS_CENTERED,
            loader.camera->px_width,
            loader.camera->px_height,
            0
        );

        SDL_Surface * icon = SDL_LoadBMP("icon.bmp");
        // http://www.gamedev.net/topic/463077-sdl---transparent-icons/
        // SDL_SetColorKey(icon, SDL_SRCCOLORKEY, SDL_MapRGB(icon->format, 255, 0, 255));
        SDL_SetWindowIcon(window, icon);

        SDL_Renderer * renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
        SDL_Texture * texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, loader.camera->px_width, loader.camera->px_height);

        SDL_Event event;

        TTF_Init();
        TTF_Font * font = TTF_OpenFont("Sathu.ttf", 12);

        uint32_t * pixels = new uint32_t[loader.camera->px_width * loader.camera->px_height];

        struct rendering_data_s data;

        // Does the scene need to be re-rendered? Initialized to true for first rendering
        bool render = true;
        bool stats = false;

        float translation_pas = 1;
        float rotation_pas = 10;

        while (true) {

            if (render) {

                // http://ascii-table.com/ansi-escape-sequences.php
                // http://www.cplusplus.com/forum/unices/36461/s
                // http://stackoverflow.com/questions/25879183/can-terminal-app-be-made-to-respect-ansi-escape-codes
                // http://wiki.bash-hackers.org/scripting/terminalcodes

                cout << Term::SC << "Processing rendering..." << endl;

                t = omp_get_wtime();
                loader.camera->render();
                data.spf = timer(t);
                data.eye_x = loader.camera->eye[0];
                data.eye_y = loader.camera->eye[1];
                data.eye_z = loader.camera->eye[2];
                cout << Term::RC << Term::CLR << "Scene rendering took " << Term::FGC_GREEN << data.spf << Term::R << " seconds" << endl;

                uintmax_t w = 0;

                //@ Can it be parallelized with openmp ?
                for (uintmax_t j = 1; j < loader.camera->px_height + 1; j++)
                    for (uintmax_t i = 1; i < loader.camera->px_width + 1; i++)
                        pixels[w++] = loader.camera->image.get_texel(i, j).get_rgba();

                // sizeof(uint32_t) = 4
                SDL_UpdateTexture(texture, nullptr, pixels, loader.camera->px_width << 2);
                SDL_RenderCopy(renderer, texture, nullptr, nullptr);

                if (stats)
                    display_stats(renderer, font, data);

                SDL_RenderPresent(renderer);

                render = false;
            }

            SDL_WaitEvent(&event);

            // Get modifier keys states
            SDL_Keymod modState = SDL_GetModState();

            // #REF https://wiki.libsdl.org/SDL_Keycode
            if (event.type == SDL_KEYDOWN) {

                // Camera rotation controls

                if (event.key.keysym.sym == SDLK_LEFT) {
                    cout << "Left arrow key was pressed" << endl;

                    if (modState & KMOD_LSHIFT)
                        loader.camera->rotateScene(Vector::Y, -rotation_pas);
                    else
                        loader.camera->rotate(Vector::Y, rotation_pas);

                    render = true;
                }

                else if (event.key.keysym.sym == SDLK_RIGHT) {
                    cout << "Right arrow key was pressed" << endl;

                    if (modState & KMOD_LSHIFT)
                        loader.camera->rotateScene(Vector::Y, rotation_pas);
                    else
                        loader.camera->rotate(Vector::Y, -rotation_pas);

                    render = true;
                }

                else if (event.key.keysym.sym == SDLK_UP) {
                    cout << "Up arrow key was pressed" << endl;

                    if (modState & KMOD_LSHIFT)
                        loader.camera->rotateScene(Vector::X, -rotation_pas);
                    else
                        loader.camera->rotate(Vector::X, -rotation_pas);

                    render = true;
                }

                else if (event.key.keysym.sym == SDLK_DOWN) {
                    cout << "Down arrow key was pressed" << endl;

                    if (modState & KMOD_LSHIFT)
                        loader.camera->rotateScene(Vector::X, rotation_pas);
                    else
                        loader.camera->rotate(Vector::X, rotation_pas);

                    render = true;
                }

                // Camera translation controls

                else if (event.key.keysym.sym == SDLK_q) {
                    cout << "Q key was pressed" << endl;
                    loader.camera->translate(Vector::X * translation_pas);
                    render = true;
                }

                else if (event.key.keysym.sym == SDLK_d) {
                    cout << "D key was pressed" << endl;
                    loader.camera->translate(Vector::X * -translation_pas);
                    render = true;
                }

                else if (event.key.keysym.sym == SDLK_z) {
                    cout << "Z key was pressed" << endl;
                    loader.camera->translate(Vector::Z * translation_pas);
                    render = true;
                }

                else if (event.key.keysym.sym == SDLK_s) {
                    cout << "S key was pressed" << endl;
                    loader.camera->translate(Vector::Z * -translation_pas);
                    render = true;
                }

                // Camera reset control

                else if (event.key.keysym.sym == SDLK_o) {
                    cout << "O key was pressed" << endl;
                    loader.camera->reset();
                    render = true;
                }

                // Steps controls

                else if (event.key.keysym.sym == SDLK_t) {
                    cout << "T key was pressed" << endl;
                    translation_pas += .1;
                }

                else if (event.key.keysym.sym == SDLK_g) {

                    cout << "G key was pressed" << endl;

                    if (.2 < translation_pas)
                        translation_pas -= .1;
                    else
                        translation_pas /= 2; // Avoid negative values
                }

                else if (event.key.keysym.sym == SDLK_y) {
                    cout << "Y key was pressed" << endl;
                    rotation_pas += 1;
                }

                else if (event.key.keysym.sym == SDLK_h) {

                    cout << "H key was pressed" << endl;

                    if (2 < rotation_pas)
                        rotation_pas -= 1;
                    else
                        rotation_pas /= 2; // Avoid negative values
                }

                // Focale controls

                //@ Does not seem to work, to fix

                else if (event.key.keysym.sym == SDLK_u) {
                    cout << "U key was pressed" << endl;
                    loader.camera->set_focale( loader.camera->focale + 1 );
                    render = true;
                }

                else if (event.key.keysym.sym == SDLK_j) {
                    cout << "J key was pressed" << endl;
                    //@ avoid negative values
                    loader.camera->set_focale( loader.camera->focale - 1 );
                    render = true;
                }

                // General controls

                // Display or not rendering info
                else if (event.key.keysym.sym == SDLK_i) {

                    stats = !stats;

                    // Need to recopy current image since SDL_RenderPresent invalid previous buffer
                    SDL_RenderCopy(renderer, texture, nullptr, nullptr);

                    if (stats)
                        display_stats(renderer, font, data);

                    SDL_RenderPresent(renderer);
                }

                // Prints the scene currently displayed to a file
                else if (event.key.keysym.sym == SDLK_p) {

                    cout << "P key was pressed" << endl;

                    stringstream ss;
                    ss << "./screenshots/" << time(nullptr) << ".ppm";

                    cout << Term::SC << "Processing printing..." << endl;

                    t = omp_get_wtime();
                    loader.camera->print(ss.str().c_str());
                    cout << Term::RC << Term::CLR << "Scene printing took " << Term::FGC_GREEN << timer(t) << Term::R << " seconds" << endl;
                }

                //@todo Implement scene reload
                // // Reload scene from file
                // else if (event.key.keysym.sym == SDLK_r) {

                //     cout << "R key was pressed" << endl;

                //     SDL_SetWindowTitle
                //     SDL_SetWindowSize
                //     SDL_SetWindowData

                //     cout << Term::SC << "Reloading scene file..." << endl;

                //     t = omp_get_wtime();
                //     loader.camera->print(ss.str().c_str());
                //     cout << Term::RC << Term::CLR << "Scene loading took " << Term::FGC_GREEN << timer(t) << Term::R << " seconds" << endl;
                // }

                else if (event.key.keysym.sym == SDLK_ESCAPE) {
                    cout << "Escape key was pressed" << endl;
                    break;
                }
            }

            else if (event.type == SDL_QUIT) {
                cout << "Window was closed" << endl;
                break;
            }
        }

        TTF_CloseFont(font);
        TTF_Quit();

        SDL_DestroyTexture(texture);
        SDL_DestroyRenderer(renderer);
        SDL_FreeSurface(icon);
        SDL_DestroyWindow(window);
        SDL_Quit();

        delete [] pixels;

        cout << "Exit normally, goodbye" << endl;

        return EXIT_SUCCESS;
    }

    catch (const string & e) {

        cerr << "Exception : " << e << endl;

        return EXIT_FAILURE;
    }
}
