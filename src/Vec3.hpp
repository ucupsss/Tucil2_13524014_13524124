#pragma once

#include <cmath>
#include <ostream>
#include <iomanip>

struct Vec3 {
    double x{0.0};
    double y{0.0};
    double z{0.0};

    constexpr Vec3() = default;
    constexpr Vec3(double x, double y, double z) : x{x}, y{y}, z{z} {}

    [[nodiscard]] constexpr Vec3 operator+(const Vec3& o) const { return {x + o.x, y + o.y, z + o.z}; }
    [[nodiscard]] constexpr Vec3 operator-(const Vec3& o) const { return {x - o.x, y - o.y, z - o.z}; }
    [[nodiscard]] constexpr Vec3 operator*(double t) const { return {x * t, y * t, z * t}; }

    [[nodiscard]] constexpr double dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    [[nodiscard]] constexpr Vec3 cross(const Vec3& o) const {
        return {y * o.z - z * o.y, z * o.x - x * o.z, x * o.y - y * o.x};
    }

    [[nodiscard]] double length() const { return std::sqrt(x * x + y * y + z * z); }
    
    [[nodiscard]] Vec3 normalize() const { 
        double l = length(); 
        return (l > 0.0) ? Vec3{x / l, y / l, z / l} : Vec3{}; 
    }

    // Overload operator << sebagai pengganti fungsi str()
    friend std::ostream& operator<<(std::ostream& os, const Vec3& v) {
        return os << std::fixed << std::setprecision(6) << v.x << " " << v.y << " " << v.z;
    }
};