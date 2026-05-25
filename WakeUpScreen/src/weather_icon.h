#ifndef WEATHER_ICON_H
#define WEATHER_ICON_H

#include <lvgl.h>

#ifdef __cplusplus
extern "C" {
#endif

// Return the image descriptor for a normalized icon name (hyphens allowed).
const lv_image_dsc_t *get_weather_icon(const char *icon_name);

#ifdef __cplusplus
}
#endif

#endif // WEATHER_ICON_H
