cmake_minimum_required(VERSION 3.14)
project(Tucil2_Voxelizer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Tambahkan optimisasi
add_compile_options(-O2 -Wall)

set(SOURCES src/main.cpp)

# ============================================================
# Target 1: Tanpa viewer (Program Utama)
# ============================================================
find_package(Threads REQUIRED)

add_executable(voxelizer ${SOURCES})
target_include_directories(voxelizer PRIVATE src)
target_link_libraries(voxelizer Threads::Threads)

# ============================================================
# Target 2: Dengan viewer (Versi WSL / Linux Anti-Bentrok)
# ============================================================
# Kita TIDAK LAGI menggunakan find_package(glfw3) dan find_package(OpenGL)
# karena rawan nyasar ke MinGW Windows.

add_executable(voxelizer_viewer ${SOURCES})
target_include_directories(voxelizer_viewer PRIVATE src)
target_compile_definitions(voxelizer_viewer PRIVATE ENABLE_VIEWER)

# Langsung "tembak" nama library asli bawaan Linux (glfw dan GL)
target_link_libraries(voxelizer_viewer
    glfw
    GL
    Threads::Threads
)

message(STATUS "Konfigurasi Anti-MinGW aktif. Siap membangun voxelizer_viewer!")