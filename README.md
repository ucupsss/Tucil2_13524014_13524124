# Tucil2 IF2211 - Voxelization Objek 3D menggunakan Octree

Program konversi model 3D format `.obj` menjadi representasi voxel menggunakan **Octree** dan algoritma **Divide and Conquer**. Program ini ditulis dalam bahasa **C++** dengan dukungan **concurrency** (multithreading) dan bonus **3D viewer interaktif**.

---

## Deskripsi Singkat

Program ini membaca file `.obj`, membangun struktur data *Octree* secara rekursif dari *bounding box* model (menggunakan pendekatan *Divide and Conquer*), lalu menghasilkan voxel berupa kubus-kubus seragam pada setiap *leaf node* yang bersentuhan dengan permukaan objek. 

- **Algoritma**: Divide and Conquer via Octree.
- **Interseksi**: Triangle-AABB menggunakan Separating Axis Theorem (Möller 2001).
- **Concurrency**: Paralelisme menggunakan `std::async` pada level kedalaman awal octree untuk mempercepat komputasi.
- **Viewer (Bonus)**: OpenGL murni + GLFW (orbit camera, toggle efek X-Ray ke model asli, wireframe mode).

---

## Requirement & Instalasi

**Minimum (Spesifikasi Wajib):**
- Compiler C++17 (g++ ≥ 7, clang++ ≥ 5, MSVC 2019+)
- CMake ≥ 3.14 **atau** GNU Make

**Untuk Viewer (Spesifikasi Bonus):**
- GLFW3
- OpenGL (GL)

### Instalasi Dependensi Viewer

**Ubuntu / Debian / WSL:**
```bash
sudo apt update
sudo apt install build-essential cmake libglfw3-dev libgl1-mesa-dev
```

**Windows (MSYS2/MinGW):**
```bash
pacman -S mingw-w64-x86_64-gcc mingw-w64-x86_64-cmake mingw-w64-x86_64-glfw
```

---

## Cara Mengkompilasi

Sangat disarankan menggunakan **CMake** untuk menghindari bentrokan *environment* antar sistem operasi.

```bash
# 1. Masuk ke direktori proyek
cd Tucil2_13524014_13524124

# 2. Buat folder bin dan konfigurasi CMake
mkdir bin
cd bin
cmake ..

# 3. Lakukan kompilasi
make
```

Jika dependensi GLFW tersedia di sistem, CMake otomatis akan membangun dua file *executable*:
1. `voxelizer` — Program utama CLI (tanpa viewer).
2. `voxelizer_viewer` — Program utama dengan viewer 3D interaktif.


---

## Cara Menjalankan

Program dijalankan melalui Command Line Interface (CLI). Asumsi terminal berada di dalam folder tempat *executable* berada (misal: `bin/`

```bash
# Skenario 1: Tanpa viewer
./voxelizer <path_file.obj> <max_depth>

# Skenario 2: Dengan viewer interaktif (tambahkan flag --view)
./voxelizer_viewer <path_file.obj> <max_depth> --view
```

**Contoh Penggunaan:**
```bash
./voxelizer ../test/input/pumpkin.obj 3
./voxelizer_viewer ../test/input/pumpkin.obj 5 --view
```

### Parameter
| Parameter | Keterangan |
|-----------|-----------|
| `path_file.obj` | Path relatif atau absolut menuju file `.obj` input. |
| `max_depth` | Kedalaman maksimum octree (integer, 1–10). Semakin besar, voxel semakin kecil dan detail. |
| `--view` | (Opsional) Flag untuk membuka jendela 3D Viewer setelah proses voxelization selesai. |

### Kontrol 3D Viewer
| Input / Tombol | Aksi |
|--------|------|
| **Klik Kiri + Drag** | Rotasi kamera (Orbit) |
| **Scroll Mouse** | Zoom in / Zoom out |
| **T** | Toggle (menampilkan/menyembunyikan) model asli secara transparan di dalam voxel |
| **Q / ESC** | Menutup viewer dan mengakhiri program |

---

## Struktur Repository

```text
Tucil2_13524014_13524124/
│
├── src/                  # Source code utama
│   ├── main.cpp          # Entry point program
│   ├── Vec3.hpp          # Operasi vektor 3D
│   ├── AABB.hpp          # Bounding box & SAT intersection
│   ├── ObjModel.hpp      # Container struktur data .obj
│   ├── ObjParser.hpp     # Parser file input .obj
│   ├── ObjWriter.hpp     # Penulis file output voxel .obj
│   ├── Octree.hpp        # Algoritma Divide & Conquer + Concurrency
│   └── Viewer.hpp        # Implementasi OpenGL/GLFW untuk Viewer (Bonus)
│
├── bin/                # Folder hasil generate CMake (di-ignore di .gitignore)
├── test/                 # Folder berisi data uji (.obj) dan hasil konversinya
├── doc/                  # Folder berisi Laporan Tugas Kecil (PDF)
│
├── CMakeLists.txt        # Skrip konfigurasi build otomatis
└── README.md             # Dokumentasi proyek
```

---

## Catatan Teknis

- **Format face yang didukung**: `v`, `v/vt`, `v/vt/vn`, `v//vn`. Poligon dengan >3 vertex akan otomatis dipecah menggunakan algoritma *fan triangulation*.
- **Uniformitas voxel**: Root *bounding box* selalu dipaksa menjadi kubus sempurna berdasarkan sisi terpanjang, sehingga ukuran semua voxel dijamin sama (panjang = lebar = tinggi).
- **Concurrency**: Paralelisme dijalankan menggunakan `std::async` hanya hingga batas kedalaman tertentu (PARALLEL_DEPTH_LIMIT = 2) untuk mencegah *thread explosion* dan *overhead* memori berlebih. Sisa kedalaman dieksekusi secara sekuensial.

---

## Author

| Nama | NIM |
|------|-----|
| Yusuf Faishal L | 13524014 |
| Zahran Alvan P W | 13524124 |

*Program Studi Teknik Informatika — Sekolah Teknik Elektro dan Informatika*<br>
*Institut Teknologi Bandung — Semester II 2025/2026*