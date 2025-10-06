#pragma once
#include <memory>

#define LIB_NAME "arcaie"
#define LIB_VERSION "v1.0.0"

// I prefer the y-axis to point upwards, but modern graphics apis usually have
// the y-axis pointing downwards. you can undefine this to make the y-axis point upwards.
// however, it is not guaranteed that all parts of the engine will respect this setting.
#define ARC_Y_IS_DOWN

namespace arcaie
{

typedef unsigned char byte;
// represent a UTF-32 code point.
typedef char32_t u32_char;

// smart pointers
template <class T> using unique = std::unique_ptr<T>;
template <class T> using shared = std::shared_ptr<T>;
template <class T> using weak = std::weak_ptr<T>;

} // namespace arcaie