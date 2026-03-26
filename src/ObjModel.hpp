#pragma once

#include <vector>
#include "Vec3.hpp"

using Face = std::vector<int>;

struct ObjModel {
    std::vector<Vec3> vertices;
    std::vector<Face> faces;
};