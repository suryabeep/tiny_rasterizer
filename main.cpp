#include <vector>
#include <cmath>
#include <cstdlib>
#include <limits>
#include "tgaimage.hpp"
#include "model.hpp"
#include "geometry.hpp"

const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0,   0,   255);
const TGAColor green = TGAColor(0,   255, 0,   255);
Model *model = NULL;
const int width  = 800;
const int height = 800;

void line(Vec2i p0, Vec2i p1, TGAImage &image, TGAColor color) {
    bool steep = false;
    if (std::abs(p0.x-p1.x)<std::abs(p0.y-p1.y)) {
        std::swap(p0.x, p0.y);
        std::swap(p1.x, p1.y);
        steep = true;
    }
    if (p0.x>p1.x) {
        std::swap(p0, p1);
    }

    for (int x=p0.x; x<=p1.x; x++) {
        float t = (x-p0.x)/(float)(p1.x-p0.x);
        int y = p0.y*(1.-t) + p1.y*t;
        if (steep) {
            image.set(y, x, color);
        } else {
            image.set(x, y, color);
        }
    }
}

Vec3f barycentric(Vec3f A, Vec3f B, Vec3f C, Vec3f P) {
    Vec3f s[2];
    for (int i=2; i--; ) {
        s[i][0] = C[i]-A[i];
        s[i][1] = B[i]-A[i];
        s[i][2] = A[i]-P[i];
    }
    Vec3f u = cross(s[0], s[1]);
    if (std::abs(u[2])>1e-2) // dont forget that u[2] is integer. If it is zero then triangle ABC is degenerate
        return Vec3f(1.f-(u.x+u.y)/u.z, u.y/u.z, u.x/u.z);
    return Vec3f(-1,1,1); // in this case generate negative coordinates, it will be thrown away by the rasterizator
}

void triangle(Vec3f *pts, Vec3f* tcs, float *zbuffer, TGAImage &image, TGAColor color, TGAImage &texture) {
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image.get_width()-1, image.get_height()-1);
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clamp[j], std::max(bboxmax[j], pts[i][j]));
        }
    }
    Vec3f P;
    int texheight = texture.get_height();
    int texwidth = texture.get_width();
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            Vec3f bc_screen  = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;

            // texture stuff
            Vec3f one = tcs[0] * bc_screen[0];
            Vec3f two = tcs[1] * bc_screen[1];
            Vec3f three = tcs[2] * bc_screen[2];
            Vec3f total = one + two + three;
            int tex_x = (int) (texwidth * total[0]);
            int tex_y = (int) (texheight * total[1]);
            TGAColor sample_color = texture.get(tex_x, tex_y);

            P.z = 0;
            for (int i=0; i<3; i++) P.z += pts[i][2]*bc_screen[i];
            if (zbuffer[int(P.x+P.y*width)]<P.z) {
                zbuffer[int(P.x+P.y*width)] = P.z;
                image.set(P.x, P.y, sample_color);
            }
        }
    }
}

Vec3f world2screen(Vec3f v) {
    return Vec3f(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
}

int main(int argc, char** argv) {
    if (2==argc) {
        model = new Model(argv[1]);
    } else {
        model = new Model("models/african_head.obj");
    }

    TGAImage texture;
    texture.read_tga_file("african_head_diffuse.tga");
    texture.flip_vertically();
    int texwidth  = texture.get_width();
    int texheight = texture.get_height();

    float *zbuffer = new float[width*height];
    for (int i=width*height; i--; zbuffer[i] = -std::numeric_limits<float>::max());

    TGAImage image(width, height, TGAImage::RGB);
    Vec3f light_dir(0,0,-1);
    for (int i=0; i<model->nfaces(); i++) {
        Face f = model->face(i);
        Vec3f screen_coords[3];
        Vec3f world_coords[3];
        Vec3f face_normal;
        Vec3f face_texcoord;
        Vec3f face_tcs[3];
        for (int j=0; j<3; j++) {
            Vec3f v = model->vert(f.vertIndices[j]);
            screen_coords[j] = world2screen(v);
            world_coords[j]  = v;
            face_normal = face_normal + model->normal(f.normIndices[j]);
            face_tcs[j] = model->texcoord(f.texIndices[j]);
            face_texcoord = face_texcoord + model->texcoord(f.texIndices[j]);
        }

        face_texcoord = face_texcoord / 3;
        int sample_x = (int)(face_texcoord[0] * texwidth);
        int sample_y = (int)(face_texcoord[1] * texheight);
        TGAColor sample_color = texture.get(sample_x, sample_y);

        Vec3f n = cross((world_coords[2]-world_coords[0]),(world_coords[1]-world_coords[0]));
        n.normalize();
        float intensity = n*light_dir;
        if (intensity>0) {
            triangle(screen_coords, face_tcs, zbuffer, image, sample_color, texture);
        }
    }

    image.flip_vertically(); // i want to have the origin at the left bottom corner of the image
    image.write_tga_file("output.tga");
    delete model;
    return 0;
}
