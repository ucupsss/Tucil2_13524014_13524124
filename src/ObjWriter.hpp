#pragma once

#include "AABB.hpp"
#include <vector>
#include <string>
#include <fstream>
#include <stdexcept>

namespace ObjWriter {

    inline void write(const std::string& path, const std::vector<AABB>& voxels) {
        std::ofstream file(path);
        if (!file.is_open()) {
            throw std::runtime_error("Gagal membuat file output: " + path);
        }

        file << "# Voxelized OBJ - Tucil2 IF2211 Strategi Algoritma\n"
             << "# Voxel count: " << voxels.size() << "\n\n";

        // 1. Menulis semua koordinat titik sudut (vertices) untuk setiap voxel
        for (const auto& box : voxels) {
            const double x0 = box.min.x, y0 = box.min.y, z0 = box.min.z;
            const double x1 = box.max.x, y1 = box.max.y, z1 = box.max.z;

            // Rantai operator (Chaining) untuk performa penulisan disk yang lebih cepat
            file << "v " << x0 << " " << y0 << " " << z0 << '\n'
                 << "v " << x1 << " " << y0 << " " << z0 << '\n'
                 << "v " << x1 << " " << y1 << " " << z0 << '\n'
                 << "v " << x0 << " " << y1 << " " << z0 << '\n'
                 << "v " << x0 << " " << y0 << " " << z1 << '\n'
                 << "v " << x1 << " " << y0 << " " << z1 << '\n'
                 << "v " << x1 << " " << y1 << " " << z1 << '\n'
                 << "v " << x0 << " " << y1 << " " << z1 << '\n';
        }

        file << '\n';

        // 2. Merajut titik-titik tersebut menjadi bidang kotak (12 segitiga per voxel)
        int vIdx = 1;
        for (size_t i = 0; i < voxels.size(); ++i) {
            file << "f " << vIdx     << " " << vIdx + 1 << " " << vIdx + 2 << '\n'
                 << "f " << vIdx     << " " << vIdx + 2 << " " << vIdx + 3 << '\n'
                 << "f " << vIdx + 4 << " " << vIdx + 6 << " " << vIdx + 5 << '\n'
                 << "f " << vIdx + 4 << " " << vIdx + 7 << " " << vIdx + 6 << '\n'
                 << "f " << vIdx     << " " << vIdx + 1 << " " << vIdx + 5 << '\n'
                 << "f " << vIdx     << " " << vIdx + 5 << " " << vIdx + 4 << '\n'
                 << "f " << vIdx + 2 << " " << vIdx + 3 << " " << vIdx + 7 << '\n'
                 << "f " << vIdx + 2 << " " << vIdx + 7 << " " << vIdx + 6 << '\n'
                 << "f " << vIdx     << " " << vIdx + 3 << " " << vIdx + 7 << '\n'
                 << "f " << vIdx     << " " << vIdx + 7 << " " << vIdx + 4 << '\n'
                 << "f " << vIdx + 1 << " " << vIdx + 2 << " " << vIdx + 6 << '\n'
                 << "f " << vIdx + 1 << " " << vIdx + 6 << " " << vIdx + 5 << '\n';
                 
            vIdx += 8;
        }
    }

} // namespace ObjWriter