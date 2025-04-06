#ifndef WEBGPU_COMMON_DATATYPES_HPP_
#define WEBGPU_COMMON_DATATYPES_HPP_

#include "webgpu/webgpu.hpp"
#ifdef WEBGPU_BACKEND_WGPU
#include <webgpu/wgpu.h>
#endif // WEBGPU_BACKEND_WGPU

#include <GLFW/glfw3.h>
#include <glfw3webgpu.h>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#endif // __EMSCRIPTEN__

#include <array>
#include <cassert>
// #include <cmath>
#include <cstdint>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include "../codingUtilities/MagicEnum.hpp"

#endif