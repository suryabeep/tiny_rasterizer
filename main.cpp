#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include <time.h>
#include <SDL2/SDL.h>

#include "tgaimage.hpp"
#include "model.hpp"
#include "geometry.hpp"
#include "image.hpp"
#include "our_gl.hpp"

Model *model = NULL;
const int width  = 800;
const int height = 800;
const int depth = 225;

Vec3f light_dir(1, 1, 1);
Vec3f eyePt(0, -1, 3);
Vec3f lookAt(0, 0, 0);
Vec3f up(0, 1, 0);

time_t prev;

Matrix ModelView, Viewport, Projection;

// this shader thing isn't working yet, will get to it soon. For now, just do shading 'manually'
// struct GouraudShader : public IShader {
//     Vec3f varying_intensity; // written by vertex shader, read by fragment shader

//     virtual Vec4f vertex(int iface, int nthvert) {
//         Vec4f gl_Vertex = embed<4>(model->vert(iface, nthvert)); // read the vertex from .obj file
//         gl_Vertex = Viewport*Projection*ModelView*gl_Vertex;     // transform it to screen coordinates
//         varying_intensity[nthvert] = std::max(0.f, model->normal(iface, nthvert)*light_dir); // get diffuse lighting intensity
//         return gl_Vertex;
//     }

//     virtual bool fragment(Vec3f bar, TGAColor &color) {
//         float intensity = varying_intensity*bar;   // interpolate intensity for the current pixel
//         color = TGAColor(255.0f * intensity, 255.0f * intensity, 255.0f * intensity, 255.0f * intensity) ; // well duh
//         return false;                              // no, we do not discard this pixel
//     }
// };

void draw(TGAImage &texture, Image &image, IShader &shader) {
    
    for (int i=0; i<model->nfaces(); i++) {
        Face f = model->face(i);
        Vec3f screen_coords[3];
        Vec3f face_tcs[3];
        Vec3f face_norms[3];
        Vec3f intensities;

        for (int j=0; j<3; j++) {
            Vec3f v = model->vert(i, j);
            Vec4f gl_Vertex = Vec4f(v.x, v.y, v.z, 1.0f);

            screen_coords[j] = m2v(Viewport * Projection * v2m(gl_Vertex.xyz()));

            face_tcs[j] = model->texcoord(i, j);
            face_norms[j] = model->normal(i, j);
            intensities[j] = (model->normal(i, j).normalize() * light_dir.normalize());
        }

        triangle(screen_coords, face_tcs, image, texture, shader, intensities);
    }
}

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("resources/models/african_head.obj");
    }

    Image image(width, height);

    // Initialize SDL
    SDL_Init(SDL_INIT_VIDEO);
    SDL_SetHint(SDL_HINT_VIDEO_X11_NET_WM_BYPASS_COMPOSITOR, "0");
    SDL_Window *window = SDL_CreateWindow("Tiny Rasterizer", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, width, height, SDL_WINDOW_OPENGL);
    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    SDL_Texture * sdl_texture = SDL_CreateTexture(renderer,
            SDL_PIXELFORMAT_ABGR8888, SDL_TEXTUREACCESS_STREAMING, image._width, image._height);
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);


    TGAImage texture;
    texture.read_tga_file("resources/textures/african_head_diffuse.tga");
    texture.flip_vertically();

    // draw loop
    ModelView = lookat(eyePt, lookAt, up);
    Viewport   = viewport(width/8, height/8, width*3/4, height*3/4, depth);
    Projection = projection(-1.0f / (eyePt - lookAt).norm());

    std::cout << ModelView  << std::endl;
    std::cout << Projection << std::endl;
    std::cout << Viewport   <<std::endl;

    GouraudShader shader;

    bool running = true;
    SDL_Event event;
    while(running) {
        // Process events
        while(SDL_PollEvent(&event)) {
            const char * key;
            switch (event.type) {
                case SDL_QUIT:
                    running = false;
                    break;
                case SDL_KEYDOWN:
                    key = SDL_GetKeyName(event.key.keysym.sym);
                    if (strcmp(key, "Escape") == 0) {
                        running = false;
                    }
                    break;
                default:
                break;
            }
        }

        // Clear screen
        SDL_UpdateTexture(sdl_texture, NULL, image.pixels,  image._width * sizeof(unsigned int));
        SDL_RenderClear(renderer);

        // rotate the camera around a circle of radius 2
        // time_t now = time(0);
        // eyePt[0] = cos(now);
        // eyePt[1] = 0;
        // eyePt[2] = sin(now);
        
        // draw
        draw(texture, image, shader);
        SDL_RenderCopy(renderer, sdl_texture, NULL, NULL);

        // Show what was drawn
        SDL_RenderPresent(renderer);
    }

    // Release resources
    SDL_DestroyTexture(sdl_texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    delete model;
    return 0;
}
