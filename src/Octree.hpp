#pragma once

#include "AABB.hpp"
#include "ObjModel.hpp"
#include <vector>
#include <future>
#include <mutex>
#include <numeric> // Ditambahkan untuk std::iota
#include <array>
#include <algorithm>

// Batas kedalaman untuk concurrency agar tidak terjadi thread-explosion
constexpr int PARALLEL_DEPTH_LIMIT = 2;

class Octree {
public:
    struct Stats {
        std::vector<int> nodesFormed;
        std::vector<int> nodesSkipped;
        int voxelCount{0};
    };

    Octree(const ObjModel& modelRef, int depthLimit)
        : model(modelRef), 
          maxDepth(depthLimit),
          stats{std::vector<int>(depthLimit + 1, 0),
                std::vector<int>(depthLimit + 1, 0), 0} 
    {} // Constructor ditutup dengan rapi di sini

    void build() {
        if (model.vertices.empty()) return;

        // 1. Cari Bounding Box (AABB) utama untuk keseluruhan model
        Vec3 minBound = model.vertices.front();
        Vec3 maxBound = model.vertices.front();

        for (const auto& v : model.vertices) {
            minBound.x = std::min(minBound.x, v.x);
            maxBound.x = std::max(maxBound.x, v.x);
            minBound.y = std::min(minBound.y, v.y);
            maxBound.y = std::max(maxBound.y, v.y);
            minBound.z = std::min(minBound.z, v.z);
            maxBound.z = std::max(maxBound.z, v.z);
        }

        // 2. Jadikan kubus sempurna agar dimensi voxel tidak gepeng
        const double dx = maxBound.x - minBound.x;
        const double dy = maxBound.y - minBound.y;
        const double dz = maxBound.z - minBound.z;
        const double maxSide = std::max({dx, dy, dz}) * 1.001; // Margin presisi

        const Vec3 center = {
            (minBound.x + maxBound.x) / 2.0, 
            (minBound.y + maxBound.y) / 2.0, 
            (minBound.z + maxBound.z) / 2.0
        };

        const Vec3 halfSize = Vec3{1, 1, 1} * (maxSide / 2.0);
        const AABB rootBox{center - halfSize, center + halfSize};

        // 3. Siapkan daftar indeks seluruh wajah (faces) model
        std::vector<int> allFaces(model.faces.size());
        // std::iota mengisi array berurutan: 0, 1, 2, 3... (Cara modern pengganti for-loop)
        std::iota(allFaces.begin(), allFaces.end(), 0);

        // 4. Mulai eksekusi algoritma Divide and Conquer
        buildRecursive(rootBox, allFaces, 0);
    }

    [[nodiscard]] const std::vector<AABB>& getVoxels() const { return voxels; }
    [[nodiscard]] const Stats& getStats() const { return stats; }

private:
    const ObjModel& model;
    const int maxDepth;
    Stats stats;

    std::vector<AABB> voxels;
    std::mutex voxelMutex;
    std::mutex statsMutex;

    void buildRecursive(const AABB& box, const std::vector<int>& activeFaces, int currentDepth) {
        // Catat statistik: Node terbentuk
        if (currentDepth > 0) {
            std::lock_guard<std::mutex> lock(statsMutex);
            stats.nodesFormed[currentDepth]++;
        }

        // Basis 1 (Pruning): Hentikan pencarian jika tidak ada permukaan 3D di area ini
        if (activeFaces.empty()) {
            if (currentDepth > 0) {
                std::lock_guard<std::mutex> lock(statsMutex);
                stats.nodesSkipped[currentDepth]++;
            }
            return;
        }

        // Basis 2 (Leaf): Jika kedalaman maksimal tercapai, catat sebagai 1 Voxel
        if (currentDepth == maxDepth) {
            std::lock_guard<std::mutex> lock(voxelMutex);
            voxels.push_back(box);
            stats.voxelCount++;
            return;
        }

        // DIVIDE: Pecah area menjadi 8 ruangan (Oktant)
        auto subBoxes = box.subdivide();
        std::array<std::vector<int>, 8> facesPerOktant;

        // Saring face mana saja yang bersilangan dengan oktant anak
        for (int faceIdx : activeFaces) {
            const auto& face = model.faces[faceIdx];
            const Vec3& v0 = model.vertices[face[0]];
            const Vec3& v1 = model.vertices[face[1]];
            const Vec3& v2 = model.vertices[face[2]];

            for (int i = 0; i < 8; ++i) {
                if (subBoxes[i].intersectsTriangle(v0, v1, v2)) {
                    facesPerOktant[i].push_back(faceIdx);
                }
            }
        }

        // CONQUER: Eksekusi sub-masalah
        if (currentDepth < PARALLEL_DEPTH_LIMIT) {
            // Gunakan Multithreading (Asynchronous) untuk level atas agar CPU bekerja maksimal
            std::array<std::future<void>, 8> threads;
            for (int i = 0; i < 8; ++i) {
                threads[i] = std::async(std::launch::async, 
                    [this, &subBoxes, &facesPerOktant, currentDepth, i]() {
                        buildRecursive(subBoxes[i], facesPerOktant[i], currentDepth + 1);
                    }
                );
            }
            
            // Barrier: Tunggu semua thread selesai sebelum kembali
            for (auto& t : threads) {
                t.get(); 
            }
        } else {
            // Sequential untuk kedalaman bawah agar memori tidak meledak
            for (int i = 0; i < 8; ++i) {
                buildRecursive(subBoxes[i], facesPerOktant[i], currentDepth + 1);
            }
        }
    }
};