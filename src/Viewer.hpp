#pragma once
#ifdef ENABLE_VIEWER

#include "AABB.hpp"
#include "ObjModel.hpp"
#include <vector>
#include <string>
#include <cmath>
#include <iostream>
#include <array>

// OpenGL / GLFW headers
#include <GL/gl.h>
#include <GLFW/glfw3.h>

// Gunakan konstanta M_PI bawaan jika ada, atau definisikan sendiri
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

// ============================================================
// State Kamera & Interaksi
// ============================================================
struct ViewerState {
    double azimuth{45.0};   // rotasi horizontal (derajat)
    double elevation{30.0}; // rotasi vertikal (derajat)
    double distance{5.0};   // jarak kamera ke pusat
    double lastX{0.0}, lastY{0.0};
    bool dragging{false};
    bool showOriginal{false};
};

// Karena GLFW callback pakai function pointer gaya C, 
// kita butuh satu global pointer untuk menyimpan state.
static ViewerState g_state;

// ============================================================
// Fungsi Matematika Pengganti gluLookAt
// (Menghindari masalah missing GLU library di Windows/MinGW)
// ============================================================
inline void applyLookAtMatrix(const Vec3& eye, const Vec3& center, const Vec3& up) {
    Vec3 f = (center - eye).normalize();
    Vec3 s = f.cross(up).normalize();
    Vec3 u = s.cross(f);

    std::array<double, 16> matrix = {
         s.x,  u.x, -f.x,  0.0,
         s.y,  u.y, -f.y,  0.0,
         s.z,  u.z, -f.z,  0.0,
         0.0,  0.0,  0.0,  1.0
    };

    glMultMatrixd(matrix.data());
    glTranslated(-eye.x, -eye.y, -eye.z);
}

// ============================================================
// GLFW Callbacks
// ============================================================
static void mouseButtonCB(GLFWwindow* window, int btn, int action, int /*mods*/) {
    if (btn == GLFW_MOUSE_BUTTON_LEFT) {
        g_state.dragging = (action == GLFW_PRESS);
        if (g_state.dragging) {
            glfwGetCursorPos(window, &g_state.lastX, &g_state.lastY);
        }
    }
}

static void cursorPosCB(GLFWwindow* /*window*/, double x, double y) {
    if (!g_state.dragging) return;
    
    double dx = x - g_state.lastX;
    double dy = y - g_state.lastY;
    
    g_state.azimuth   += dx * 0.4;
    g_state.elevation -= dy * 0.4;
    
    // Batasi elevasi agar kamera tidak terbalik
    g_state.elevation = std::clamp(g_state.elevation, -89.0, 89.0);
    
    g_state.lastX = x;
    g_state.lastY = y;
}

static void scrollCB(GLFWwindow* /*window*/, double /*dx*/, double dy) {
    g_state.distance *= (dy > 0 ? 0.9 : 1.1);
    if (g_state.distance < 0.5) g_state.distance = 0.5;
}

static void keyCB(GLFWwindow* window, int key, int /*scancode*/, int action, int /*mods*/) {
    if (action != GLFW_PRESS) return;
    
    if (key == GLFW_KEY_ESCAPE || key == GLFW_KEY_Q) {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    } else if (key == GLFW_KEY_T) {
        g_state.showOriginal = !g_state.showOriginal;
    }
}

// ============================================================
// Fungsi Render OpenGL Klasik
// ============================================================
namespace GLRenderer {

    inline void drawCube(const AABB& box, bool wireframe) {
        double x0 = box.min.x, y0 = box.min.y, z0 = box.min.z;
        double x1 = box.max.x, y1 = box.max.y, z1 = box.max.z;

        if (wireframe) {
            glBegin(GL_LINES);
            auto e = [&](double ax, double ay, double az, double bx, double by, double bz) {
                glVertex3d(ax, ay, az); glVertex3d(bx, by, bz);
            };
            e(x0,y0,z0, x1,y0,z0); e(x1,y0,z0, x1,y1,z0); e(x1,y1,z0, x0,y1,z0); e(x0,y1,z0, x0,y0,z0);
            e(x0,y0,z1, x1,y0,z1); e(x1,y0,z1, x1,y1,z1); e(x1,y1,z1, x0,y1,z1); e(x0,y1,z1, x0,y0,z1);
            e(x0,y0,z0, x0,y0,z1); e(x1,y0,z0, x1,y0,z1); e(x1,y1,z0, x1,y1,z1); e(x0,y1,z0, x0,y1,z1);
            glEnd();
        } else {
            glBegin(GL_QUADS);
            glNormal3d( 0,  0, -1); glVertex3d(x0,y0,z0); glVertex3d(x0,y1,z0); glVertex3d(x1,y1,z0); glVertex3d(x1,y0,z0);
            glNormal3d( 0,  0,  1); glVertex3d(x0,y0,z1); glVertex3d(x1,y0,z1); glVertex3d(x1,y1,z1); glVertex3d(x0,y1,z1);
            glNormal3d( 0, -1,  0); glVertex3d(x0,y0,z0); glVertex3d(x1,y0,z0); glVertex3d(x1,y0,z1); glVertex3d(x0,y0,z1);
            glNormal3d( 0,  1,  0); glVertex3d(x0,y1,z0); glVertex3d(x0,y1,z1); glVertex3d(x1,y1,z1); glVertex3d(x1,y1,z0);
            glNormal3d(-1,  0,  0); glVertex3d(x0,y0,z0); glVertex3d(x0,y0,z1); glVertex3d(x0,y1,z1); glVertex3d(x0,y1,z0);
            glNormal3d( 1,  0,  0); glVertex3d(x1,y0,z0); glVertex3d(x1,y1,z0); glVertex3d(x1,y1,z1); glVertex3d(x1,y0,z1);
            glEnd();
        }
    }

    inline void drawOriginalModel(const ObjModel& model) {
        glColor4f(0.9f, 0.6f, 0.2f, 0.35f);
        glBegin(GL_TRIANGLES);
        for (const auto& face : model.faces) {
            const Vec3& v0 = model.vertices[face[0]];
            const Vec3& v1 = model.vertices[face[1]];
            const Vec3& v2 = model.vertices[face[2]];
            
            Vec3 n = (v1 - v0).cross(v2 - v0).normalize();
            glNormal3d(n.x, n.y, n.z);
            glVertex3d(v0.x, v0.y, v0.z);
            glVertex3d(v1.x, v1.y, v1.z);
            glVertex3d(v2.x, v2.y, v2.z);
        }
        glEnd();
    }
}

// ============================================================
// Entry Point Viewer
// ============================================================
class Viewer {
private:
    static void computeModelBounds(const ObjModel& model, Vec3& center, double& scale) {
        if (model.vertices.empty()) return;
        
        Vec3 minB = model.vertices[0];
        Vec3 maxB = model.vertices[0];
        
        for (const auto& v : model.vertices) {
            minB.x = std::min(minB.x, v.x); maxB.x = std::max(maxB.x, v.x);
            minB.y = std::min(minB.y, v.y); maxB.y = std::max(maxB.y, v.y);
            minB.z = std::min(minB.z, v.z); maxB.z = std::max(maxB.z, v.z);
        }
        
        center = {(minB.x + maxB.x) / 2.0, (minB.y + maxB.y) / 2.0, (minB.z + maxB.z) / 2.0};
        scale  = std::max({maxB.x - minB.x, maxB.y - minB.y, maxB.z - minB.z});
        if (scale < 1e-9) scale = 1.0;
    }

public:
    static void run(const std::vector<AABB>& voxels, const ObjModel& model,
                    const std::string& title = "Voxel Viewer - IF2211") {
        if (!glfwInit()) {
            std::cerr << "[Error] Gagal inisialisasi GLFW\n";
            return;
        }

        GLFWwindow* window = glfwCreateWindow(1000, 700, title.c_str(), nullptr, nullptr);
        if (!window) {
            std::cerr << "[Error] Gagal membuat window GLFW\n";
            glfwTerminate();
            return;
        }

        glfwMakeContextCurrent(window);
        glfwSetMouseButtonCallback(window, mouseButtonCB);
        glfwSetCursorPosCallback(window, cursorPosCB);
        glfwSetScrollCallback(window, scrollCB);
        glfwSetKeyCallback(window, keyCB);

        // Setup OpenGL State
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_LIGHT0);
        glEnable(GL_COLOR_MATERIAL);
        glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
        glClearColor(0.12f, 0.12f, 0.15f, 1.0f);

        const float lightPos[] = {3.0f, 5.0f, 4.0f, 0.0f};
        glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

        Vec3 modelCenter;
        double modelScale{1.0};
        computeModelBounds(model, modelCenter, modelScale);
        
        // Reset state untuk setiap pemanggilan Viewer
        g_state = ViewerState{}; 
        g_state.distance = 2.0;

        std::cout << "\n=== Viewer Kontrol ===\n"
                  << "  Drag kiri : Rotasi kamera\n"
                  << "  Scroll    : Zoom in/out\n"
                  << "  T         : Toggle model asli\n"
                  << "  Q / ESC   : Keluar viewer\n"
                  << "======================\n\n";

        while (!glfwWindowShouldClose(window)) {
            int W, H;
            glfwGetFramebufferSize(window, &W, &H);
            if (H == 0) H = 1; // Cegah division by zero
            glViewport(0, 0, W, H);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // 1. Matrix Proyeksi (Kamera Lensa)
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            
            double aspect = static_cast<double>(W) / H;
            double nearClip = 0.01 * modelScale;
            double farClip  = 100.0 * modelScale;
            double top = nearClip * std::tan((45.0 * M_PI / 180.0) / 2.0);
            
            glFrustum(-top * aspect, top * aspect, -top, top, nearClip, farClip);

            // 2. Matrix ModelView (Posisi Kamera)
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();
            
            double az = g_state.azimuth * M_PI / 180.0;
            double el = g_state.elevation * M_PI / 180.0;
            double dist = g_state.distance * modelScale;
            
            Vec3 eye{
                dist * std::cos(el) * std::sin(az) + modelCenter.x,
                dist * std::sin(el) + modelCenter.y,
                dist * std::cos(el) * std::cos(az) + modelCenter.z
            };
            
            applyLookAtMatrix(eye, modelCenter, Vec3{0, 1, 0});

            // 3. Render Model Asli Transparan (Opsional)
            if (g_state.showOriginal) {
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                GLRenderer::drawOriginalModel(model);
                glDisable(GL_BLEND);
            }

            // 4. Render Voxel Solid DULUAN
            glColor3f(0.45f, 0.75f, 0.95f);
            for (const auto& box : voxels) {
                GLRenderer::drawCube(box, false);
            }

            // 5. Render Garis Tepi (Wireframe) Voxel
            glDisable(GL_LIGHTING);
            glColor3f(0.1f, 0.1f, 0.15f);
            glLineWidth(0.8f);
            for (const auto& box : voxels) {
                GLRenderer::drawCube(box, true);
            }
            glEnable(GL_LIGHTING);

            // 3. Render Model Asli Transparan BELAKANGAN (Efek X-Ray)
            if (g_state.showOriginal) {
                glDisable(GL_DEPTH_TEST); // Matikan hukum fisika (Tembus pandang)
                glEnable(GL_BLEND);
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
                
                GLRenderer::drawOriginalModel(model); // Gambar labu aslinya
                
                glDisable(GL_BLEND);
                glEnable(GL_DEPTH_TEST); // Nyalakan hukum fisika kembali
            }

            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        glfwDestroyWindow(window);
        glfwTerminate();
    }
};

#endif // ENABLE_VIEWER