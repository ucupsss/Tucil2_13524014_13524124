#include "ObjParser.hpp"
#include "Octree.hpp"
#include "ObjWriter.hpp"

#ifdef ENABLE_VIEWER
#include "Viewer.hpp"
#endif

#include <iostream>
#include <chrono>
#include <string>
#include <filesystem>
#include <stdexcept>

// Gunakan alias agar tidak perlu menulis std::filesystem berulang kali
namespace fs = std::filesystem;

// Struktur untuk menyimpan konfigurasi dari terminal
struct AppConfig {
    std::string inputPath;
    int maxDepth{0};
    bool useViewer{false};
};

void printHeader() {
    std::cout << "============================================\n"
              << "  Voxelization 3D menggunakan Octree\n"
              << "  Tugas Kecil 2 IF2211 Strategi Algoritma\n"
              << "============================================\n";
}

// Memisahkan logika parsing argumen dari fungsi main
AppConfig parseArguments(int argc, char* argv[]) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <path_file.obj> <max_depth> [--view]\n"
                  << "Contoh: " << argv[0] << " test/cube.obj 4\n"
                  << "        " << argv[0] << " test/pumpkin.obj 6 --view\n";
        throw std::invalid_argument("Argumen terminal tidak lengkap.");
    }

    AppConfig config;
    config.inputPath = argv[1];

    try {
        config.maxDepth = std::stoi(argv[2]);
    } catch (...) {
        throw std::invalid_argument("max_depth harus berupa bilangan bulat.");
    }

    if (config.maxDepth < 1 || config.maxDepth > 10) {
        throw std::invalid_argument("max_depth harus berada di antara 1 dan 10.");
    }

    for (int i = 3; i < argc; ++i) {
        if (std::string(argv[i]) == "--view") {
            config.useViewer = true;
        }
    }

    return config;
}

// Main function sekarang terlihat sangat bersih seperti membaca daftar isi
int main(int argc, char* argv[]) {
    printHeader();

    AppConfig config;
    try {
        config = parseArguments(argc, argv);
    } catch (const std::exception& e) {
        std::cerr << "[Error] " << e.what() << '\n';
        return EXIT_FAILURE; 
    }

    std::cout << "Input file  : " << config.inputPath << '\n'
              << "Max depth   : " << config.maxDepth << "\n\n";

    try {
        // 1. Proses Parsing
        std::cout << "Membaca file .obj... ";
        ObjModel model = ObjParser::parse(config.inputPath);
        std::cout << "OK\n"
                  << "  Vertex asal : " << model.vertices.size() << '\n'
                  << "  Face asal   : " << model.faces.size() << "\n\n";

        // 2. Proses Voxelization (Divide and Conquer)
        std::cout << "Membangun Octree (dengan concurrency)... ";
        auto start_time = std::chrono::high_resolution_clock::now();

        Octree octree(model, config.maxDepth);
        octree.build();

        auto end_time = std::chrono::high_resolution_clock::now();
        double duration_ms = std::chrono::duration<double, std::milli>(end_time - start_time).count();
        std::cout << "OK\n\n";

        // 3. Proses Output menggunakan C++17 Filesystem
        fs::path inPath(config.inputPath);
        fs::path outPath = inPath.parent_path() / (inPath.stem().string() + "_voxel.obj");

        std::cout << "Menulis output ke " << outPath.string() << "... ";
        ObjWriter::write(outPath.string(), octree.getVoxels());
        std::cout << "OK\n\n";

        // 4. Laporan Statistik
        const auto& stats = octree.getStats();
        const int voxelCount  = stats.voxelCount;
        const int vertexCount = voxelCount * 8;
        const int faceCount   = voxelCount * 12;

        std::cout << "=== Hasil Konversi ===\n"
                  << "Banyak voxel terbentuk   : " << voxelCount  << '\n'
                  << "Banyak vertex terbentuk  : " << vertexCount << '\n'
                  << "Banyak faces terbentuk   : " << faceCount   << "\n\n";

        std::cout << "Statistik node octree yang terbentuk:\n";
        for (int d = 1; d <= config.maxDepth; ++d) {
            std::cout << "  " << d << " : " << stats.nodesFormed[d] << '\n';
        }

        std::cout << "\nStatistik node yang tidak perlu ditelusuri:\n";
        for (int d = 1; d <= config.maxDepth; ++d) {
            std::cout << "  " << d << " : " << stats.nodesSkipped[d] << '\n';
        }

        std::cout << "\nKedalaman octree  : " << config.maxDepth << '\n'
                  << "Waktu berjalan    : " << duration_ms << " ms\n"
                  << "Output disimpan di: " << outPath.string() << "\n"
                  << "============================================\n";

        // 5. Menjalankan Viewer (Opsional)
        if (config.useViewer) {
#ifdef ENABLE_VIEWER
            std::cout << "\nMembuka viewer...\n";
            Viewer::run(octree.getVoxels(), model);
#else
            std::cout << "\n[Info] Viewer tidak tersedia (dikompilasi tanpa -DENABLE_VIEWER).\n";
#endif
        }

    } catch (const std::exception& e) {
        std::cerr << "\n[Fatal Error] Program berhenti: " << e.what() << '\n';
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}