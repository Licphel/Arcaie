#pragma once

#define LIB_NAME "arcaie"
#define LIB_VERSION "v1.0.0"
#define DEBUG_C true

// I prefer the y-axis to point upwards, but modern graphics apis usually have
// the y-axis pointing downwards. you can undefine this to make the y-axis point upwards.
// however, it is not guaranteed that all parts of the engine will respect this setting.
#define ARC_Y_IS_DOWN