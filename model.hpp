#ifndef __MODEL_H__
#define __MODEL_H__

#include <vector>
#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include "geometry.hpp"

class Face {
private:
	std::vector<int> vertices;
	std::vector<int> texcoords;
	std::vector<int> normals;

public:
	std::vector<int> getVertices()  { return vertices; }
	std::vector<int> getTexCoords() { return texcoords; }
	std::vector<int> getNormals()   { return normals; }
	void pushVertex(int vertex)     { vertices.push_back(vertex); }
	void pushTexCoord(int coord)    { texcoords.push_back(coord); }
	void pushNormal(int norm)       { normals.push_back(norm); }
};

class Model {
private:
	std::vector<Vec3f> verts_;
	std::vector<Vec3f> texcoords_;
	std::vector<Vec3f> normals_;
	std::vector<Face> faces_;
public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
	Vec3f texcoord(int i);
	Vec3f normal(int i);
	Face face(int idx);
};

Model::Model(const char *filename) : verts_(), faces_() {
    std::ifstream in;
    in.open (filename, std::ifstream::in);
    if (in.fail()) return;
    std::string line;
    while (!in.eof()) {
        std::getline(in, line);
        std::istringstream iss(line.c_str());
        char trash;
        if (!line.compare(0, 2, "v ")) {
            iss >> trash;
            Vec3f v;
            for (int i=0;i<3;i++) iss >> v[i];
            verts_.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            int v1 = 0, v2 = 0, v3 = 0, t1 = 0, t2 = 0, t3 = 0, n1 = 0, n2 = 0, n3 = 0;
            sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &v1, &t1, &n1, &v2, &t2, &n2, &v3, &t3, &n3);
            Face face;
            face.pushVertex(v1);
            face.pushVertex(v2);
            face.pushVertex(v3);
            face.pushTexCoord(t1);
            face.pushTexCoord(t2);
            face.pushTexCoord(t3);
            face.pushNormal(n1);
            face.pushNormal(n2);
            face.pushNormal(n3);
            faces_.push_back(face);
        } 
        // else if (!line.compare(0, 3, "vt ")) {
        //     iss >> trash;
        //     Vec3f v;
        //     for (int i = 0; i < 3; i++) iss >> v[i];
        //     texcoords_.push_back(v);
        // } else if (!line.compare(0, 3, "vn ")) {
        //     iss >> trash;
        //     Vec3f v;
        //     for (int i = 0; i < 3; i++) iss >> v[i];
        //     normals_.push_back(v);
        // }
    }
    std::cerr << "# v# " << verts_.size() << " f# "  << faces_.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)verts_.size();
}

int Model::nfaces() {
    return (int)faces_.size();
}

Face Model::face(int idx) {
    return faces_[idx];
}

Vec3f Model::vert(int i) {
    return verts_[i];
}

Vec3f Model::texcoord(int i) {
    return texcoords_[i];
}

Vec3f Model::normal(int i) {
    return normals_[i];
}

#endif //__MODEL_H__