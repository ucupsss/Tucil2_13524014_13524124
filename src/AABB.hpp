#pragma once

#include "Vec3.hpp"
#include <array>
#include <cmath>
#include <algorithm>

struct AABB {
    Vec3 min{};
    Vec3 max{};

    constexpr AABB() = default;
    constexpr AABB(const Vec3& minPoint, const Vec3& maxPoint) : min{minPoint}, max{maxPoint} {}

    [[nodiscard]] constexpr Vec3 center() const {
        return {(min.x + max.x) / 2.0, (min.y + max.y) / 2.0, (min.z + max.z) / 2.0};
    }

    [[nodiscard]] constexpr Vec3 halfExtents() const {
        return {(max.x - min.x) / 2.0, (max.y - min.y) / 2.0, (max.z - min.z) / 2.0};
    }

    // Implementasi Separating Axis Theorem (Möller 2001) yang sudah dioptimisasi
    [[nodiscard]] bool intersectsTriangle(const Vec3& v0, const Vec3& v1, const Vec3& v2) const {
        const Vec3 c = center();
        const Vec3 e = halfExtents();

        // Pindahkan titik segitiga agar relatif terhadap titik tengah AABB
        const Vec3 v0_rel = v0 - c;
        const Vec3 v1_rel = v1 - c;
        const Vec3 v2_rel = v2 - c;

        // Vektor sisi-sisi segitiga
        const Vec3 f0 = v1_rel - v0_rel;
        const Vec3 f1 = v2_rel - v1_rel;
        const Vec3 f2 = v0_rel - v2_rel;

        // 9 Sumbu Uji SAT (Cross products dari AABB face normals & triangle edges)
        const std::array<Vec3, 9> axes = {
            Vec3{0, -f0.z, f0.y}, Vec3{0, -f1.z, f1.y}, Vec3{0, -f2.z, f2.y},
            Vec3{f0.z, 0, -f0.x}, Vec3{f1.z, 0, -f1.x}, Vec3{f2.z, 0, -f2.x},
            Vec3{-f0.y, f0.x, 0}, Vec3{-f1.y, f1.x, 0}, Vec3{-f2.y, f2.x, 0}
        };

        // Uji 9 sumbu persilangan
        for (const auto& ax : axes) {
            double p0 = v0_rel.dot(ax);
            double p1 = v1_rel.dot(ax);
            double p2 = v2_rel.dot(ax);
            
            double r = e.x * std::abs(ax.x) + e.y * std::abs(ax.y) + e.z * std::abs(ax.z);
            
            // Menggunakan nested std::max/min murni untuk menghindari alokasi initializer_list berulang
            double minP = std::min(p0, std::min(p1, p2));
            double maxP = std::max(p0, std::max(p1, p2));
            
            if (maxP < -r || minP > r) return false;
        }

        // Uji 3 sumbu normal dari wajah AABB (Sumbu X, Y, Z)
        if (std::max(v0_rel.x, std::max(v1_rel.x, v2_rel.x)) < -e.x || std::min(v0_rel.x, std::min(v1_rel.x, v2_rel.x)) > e.x) return false;
        if (std::max(v0_rel.y, std::max(v1_rel.y, v2_rel.y)) < -e.y || std::min(v0_rel.y, std::min(v1_rel.y, v2_rel.y)) > e.y) return false;
        if (std::max(v0_rel.z, std::max(v1_rel.z, v2_rel.z)) < -e.z || std::min(v0_rel.z, std::min(v1_rel.z, v2_rel.z)) > e.z) return false;

        // Uji sumbu normal dari bidang segitiga itu sendiri
        const Vec3 normal = f0.cross(f1);
        double d = normal.dot(v0_rel);
        double r2 = e.x * std::abs(normal.x) + e.y * std::abs(normal.y) + e.z * std::abs(normal.z);
        
        return std::abs(d) <= r2;
    }

    // Memecah AABB menjadi 8 oktant
    [[nodiscard]] std::array<AABB, 8> subdivide() const {
        const Vec3 c = center();
        return {
            AABB{min, c},
            AABB{{c.x, min.y, min.z}, {max.x, c.y, c.z}},
            AABB{{min.x, c.y, min.z}, {c.x, max.y, c.z}},
            AABB{{c.x, c.y, min.z}, {max.x, max.y, c.z}},
            AABB{{min.x, min.y, c.z}, {c.x, c.y, max.z}},
            AABB{{c.x, min.y, c.z}, {max.x, c.y, max.z}},
            AABB{{min.x, c.y, c.z}, {c.x, max.y, max.z}},
            AABB{c, max}
        };
    }
};