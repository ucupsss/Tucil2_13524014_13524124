#pragma once

#include "ObjModel.hpp"
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <stdexcept>

namespace ObjParser {

    // Gunakan 'inline' agar aman di-include di berbagai file
    inline ObjModel parse(const std::string& path) {
        std::ifstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Gagal membuka file: " + path);
        }

        ObjModel model;
        std::string line;
        int lineNum = 0;

        while (std::getline(file, line)) {
            lineNum++;
            // Lewati baris kosong atau komentar
            if (line.empty() || line[0] == '#') continue;

            std::istringstream lineStream(line);
            std::string token;
            lineStream >> token;

            if (token == "v") {
                double x, y, z;
                if (lineStream >> x >> y >> z) {
                    model.vertices.push_back({x, y, z});
                } else {
                    std::cerr << "[Warning] Format vertex tidak valid di baris " << lineNum << '\n';
                }
            } 
            else if (token == "f") {
                std::vector<int> faceIndices;
                std::string vertexDef;

                while (lineStream >> vertexDef) {
                    // Ambil indeks vertex pertama (sebelum tanda '/')
                    auto slashPos = vertexDef.find('/');
                    std::string indexStr = vertexDef.substr(0, slashPos);
                    
                    try {
                        int idx = std::stoi(indexStr) - 1; // Indeks OBJ dimulai dari 1
                        
                        if (idx < 0 || idx >= static_cast<int>(model.vertices.size())) {
                            std::cerr << "[Warning] Indeks vertex di luar batas di baris " << lineNum << '\n';
                            faceIndices.clear();
                            break;
                        }
                        faceIndices.push_back(idx);
                    } catch (const std::exception&) {
                        std::cerr << "[Warning] Data face korup di baris " << lineNum << '\n';
                        faceIndices.clear();
                        break;
                    }
                }

                // Fan Triangulation: Memecah poligon besar (segibanyak) menjadi segitiga-segitiga
                for (size_t i = 1; i + 1 < faceIndices.size(); ++i) {
                    model.faces.push_back({faceIndices[0], faceIndices[i], faceIndices[i + 1]});
                }
            }
        }

        if (model.vertices.empty() || model.faces.empty()) {
            throw std::runtime_error("File .obj tidak memiliki data vertex atau face yang valid.");
        }

        return model;
    }

} // namespace ObjParser