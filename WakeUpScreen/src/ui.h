#ifndef WAKE_UP_SCREEN_UI_H
#define WAKE_UP_SCREEN_UI_H

#ifdef __cplusplus
extern "C" {
#endif

void setup_ui(void);
void ui_set_clock(int year, int month, int day, int hour, int minute, int second);
void ui_set_weather(int temperature_c, int wind_kmh, const char *sunrise_iso8601, const char *icon_name);

// Debug helpers
const lv_image_dsc_t *ui_get_current_icon(void);
void ui_get_icon_obj_size(int *w, int *h);

#ifdef __cplusplus
}
#endif

#endif