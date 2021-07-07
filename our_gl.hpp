#pragma once

#include "geometry.hpp"
#include <limits>
#include "tgaimage.hpp"
#include "image.hpp"

struct IShader {
    virtual Vec4f vertex(int iface, int nthvert) = 0;
    virtual bool fragment(Vec3f bar, TGAColor &color) = 0;
};

static void clamp(float &input, const float min, const float max) {
    if (min > max) {
        return;
    }
    if (input < min) {
        input = min;
    }
    else if (input > max) {
        input = max;
    }
}

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

void triangle(Vec3f *pts, Vec3f* tcs, Image &image, TGAImage &texture) {
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image._width - 1, image._height - 1);
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

            Vec3f one = tcs[0] * bc_screen[0];
            Vec3f two = tcs[1] * bc_screen[1];
            Vec3f three = tcs[2] * bc_screen[2];
            Vec3f total = one + two + three;
            int tex_x = (int) (texwidth * total[0]);
            int tex_y = (int) (texheight * total[1]);
            TGAColor sample_color = texture.get(tex_x, tex_y);

            P.z = 0;
            for (int i=0; i<3; i++) 
                P.z += pts[i][2]*bc_screen[i];

            Vec3i fill_color(sample_color.r, sample_color.g, sample_color.b);

            image.setPixel(P.x, image._height - P.y - 1, fill_color, P.z);
        }
    }
}

void triangle(Vec3f *pts, Vec3f* tcs, Image &image, TGAImage &texture, IShader& shader, Vec3f intensities) {
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clampVec(image._width - 1, image._height - 1);
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            bboxmin[j] = std::max(0.f,      std::min(bboxmin[j], pts[i][j]));
            bboxmax[j] = std::min(clampVec[j], std::max(bboxmax[j], pts[i][j]));
        }
    }

    Vec3f P;
    int texheight = texture.get_height();
    int texwidth = texture.get_width();
    for (P.x=bboxmin.x; P.x<=bboxmax.x; P.x++) {
        for (P.y=bboxmin.y; P.y<=bboxmax.y; P.y++) {
            Vec3f bc_screen  = barycentric(pts[0], pts[1], pts[2], P);
            if (bc_screen.x<0 || bc_screen.y<0 || bc_screen.z<0) continue;

            Vec3f one = tcs[0] * bc_screen[0];
            Vec3f two = tcs[1] * bc_screen[1];
            Vec3f three = tcs[2] * bc_screen[2];
            Vec3f total = one + two + three;
            int tex_x = (int) (texwidth * total[0]);
            int tex_y = (int) (texheight * total[1]);
            TGAColor sample_color = texture.get(tex_x, tex_y);

            float intensity = intensities * bc_screen;
            clamp(intensity, 0.0f, 1.0f);

            P.z = 0;
            for (int i=0; i<3; i++) 
                P.z += pts[i][2]*bc_screen[i];

            Vec3i fill_color(sample_color.r, sample_color.g, sample_color.b);

            image.setPixel(P.x, image._height - P.y - 1, fill_color * intensity, P.z);
        }
    }
}

void triangle(Vec3f *pts, Vec3f* tcs, IShader &shader, Image &image, TGAImage &texture) {
    // compute bounding box of triangle
    Vec2f bboxmin( std::numeric_limits<float>::max(),  std::numeric_limits<float>::max());
    Vec2f bboxmax(-std::numeric_limits<float>::max(), -std::numeric_limits<float>::max());
    Vec2f clamp(image._width - 1, image._height - 1);
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

            Vec3f one = tcs[0] * bc_screen[0];
            Vec3f two = tcs[1] * bc_screen[1];
            Vec3f three = tcs[2] * bc_screen[2];
            Vec3f total = one + two + three;
            int tex_x = (int) (texwidth * total[0]);
            int tex_y = (int) (texheight * total[1]);
            TGAColor sample_color = texture.get(tex_x, tex_y);

            P.z = 0;
            for (int i=0; i<3; i++) 
                P.z += pts[i][2]*bc_screen[i];

            Vec3i fill_color(sample_color.r, sample_color.g, sample_color.b);

            image.setPixel(P.x, image._height - P.y - 1, fill_color, P.z);
        }
    }
}

Vec3f world2screen(Vec3f v, const int width, const int height) {
    return Vec3f(int((v.x+1.)*width/2.+.5), int((v.y+1.)*height/2.+.5), v.z);
}

Vec3f m2v(Matrix m) {
    return Vec3f(m[0][0]/m[3][0], m[1][0]/m[3][0], m[2][0]/m[3][0]);
}

Matrix v2m(Vec3f v) {
    Matrix m(4, 1);
    m[0][0] = v.x;
    m[1][0] = v.y;
    m[2][0] = v.z;
    m[3][0] = 1.f;
    return m;
}

Matrix viewport(int x, int y, int w, int h, const int depth) {
    Matrix m = Matrix::identity(4);
    m[0][3] = x+w/2.f;
    m[1][3] = y+h/2.f;
    m[2][3] = depth/2.f;

    m[0][0] = w/2.f;
    m[1][1] = h/2.f;
    m[2][2] = depth/2.f;
    return m;
}

Matrix lookat(Vec3f eye, Vec3f center, Vec3f up) {
    Vec3f z = (eye-center).normalize();
    Vec3f x = cross(up,z).normalize();
    Vec3f y = cross(z,x).normalize();
    Matrix Minv = Matrix::identity(4);
    Matrix Tr   = Matrix::identity(4);
    for (int i=0; i<3; i++) {
        Minv[0][i] = x[i];
        Minv[1][i] = y[i];
        Minv[2][i] = z[i];
        Tr[i][3] = -center[i];
    }
    return Minv*Tr;
}

Matrix projection(float coeff) {
    Matrix m = Matrix::identity(4);
    m[3][2] = coeff;
    return m;
}