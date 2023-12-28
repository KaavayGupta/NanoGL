#include "geometry.h"

template <> template <> Vec<3, int>  ::Vec(const Vec<3, float>& v) : x(int(v.x + .5f)), y(int(v.y + .5f)), z(int(v.z + .5f)) {}
template <> template <> Vec<3, float>::Vec(const Vec<3, int>& v) : x(v.x), y(v.y), z(v.z) {}
template <> template <> Vec<2, int>  ::Vec(const Vec<2, float>& v) : x(int(v.x + .5f)), y(int(v.y + .5f)) {}
template <> template <> Vec<2, float>::Vec(const Vec<2, int>& v) : x(v.x), y(v.y) {}