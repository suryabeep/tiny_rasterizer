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
public:
    std::vector<int> vertIndices;
    std::vector<int> normIndices;
    std::vector<int> texIndices;
};

class Model {
private:
	std::vector<Vec3f> _verts;
    std::vector<Vec3f> _normals;
    std::vector<Vec3f> _texcoords;
    std::vector<Face> _faces;

public:
	Model(const char *filename);
	~Model();
	int nverts();
	int nfaces();
	Vec3f vert(int i);
    Vec3f normal(int i);
    Vec3f texcoord(int i);
	Face face(int idx);
};

Model::Model(const char *filename) : _verts(), _faces() {
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
            _verts.push_back(v);
        } else if (!line.compare(0, 2, "f ")) {
            Face face;
            face.vertIndices.resize(3);
            face.normIndices.resize(3);
            face.texIndices.resize(3);

            sscanf(line.c_str(), "f %d/%d/%d %d/%d/%d %d/%d/%d", &face.vertIndices[0], &face.texIndices[0], &face.normIndices[0],
                                                                 &face.vertIndices[1], &face.texIndices[1], &face.normIndices[1],
                                                                 &face.vertIndices[2], &face.texIndices[2], &face.normIndices[2]);
            for (size_t i = 0; i < 3; ++i) {
                face.vertIndices[i]--;
                face.texIndices[i]--;
                face.normIndices[i]--;
            }
            _faces.push_back(face);
        } else if (!line.compare(0, 4, "vn  ")) {
            Vec3f v;
            sscanf(line.c_str(), "vn  %f %f %f", &v[0], &v[1], &v[2]);
            // iss >> trash;
            // Vec3f v;
            // for (int i=0;i<3;i++) iss >> v[i];
            _normals.push_back(v);
        } else if (!line.compare(0, 4, "vt  ")) {
            Vec3f v;
            sscanf(line.c_str(), "vt  %f %f %f", &v[0], &v[1], &v[2]);
            _texcoords.push_back(v);
        } 
    }
    std::cerr << "# v# " << _verts.size() << " f# "  << _faces.size() << std::endl;
}

Model::~Model() {
}

int Model::nverts() {
    return (int)_verts.size();
}

int Model::nfaces() {
    return (int)_faces.size();
}

Face Model::face(int idx) {
    return _faces[idx];
}

Vec3f Model::vert(int i) {
    return _verts[i];
}

Vec3f Model::normal(int i) {
    return _normals[i];
}

Vec3f Model::texcoord(int i) {
    return _texcoords[i];
}

#endif //__MODEL_H__