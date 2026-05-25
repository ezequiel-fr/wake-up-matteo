#include <lvgl.h>
#include <string.h>

#include "../assets/weather/clear_day.c"
#include "../assets/weather/clear_night.c"
#include "../assets/weather/cloudy_1_day.c"
#include "../assets/weather/cloudy_1_night.c"
#include "../assets/weather/cloudy_2_day.c"
#include "../assets/weather/cloudy_2_night.c"
#include "../assets/weather/cloudy_3_day.c"
#include "../assets/weather/cloudy_3_night.c"
#include "../assets/weather/fog_day.c"
#include "../assets/weather/fog_night.c"
#include "../assets/weather/frost_day.c"
#include "../assets/weather/frost_night.c"
#include "../assets/weather/rainy_1_day.c"
#include "../assets/weather/rainy_1_night.c"
#include "../assets/weather/rainy_2_day.c"
#include "../assets/weather/rainy_2_night.c"
#include "../assets/weather/rainy_3_day.c"
#include "../assets/weather/rainy_3_night.c"
#include "../assets/weather/snowy_1_day.c"
#include "../assets/weather/snowy_1_night.c"
#include "../assets/weather/snowy_2_day.c"
#include "../assets/weather/snowy_2_night.c"
#include "../assets/weather/snowy_3_day.c"
#include "../assets/weather/snowy_3_night.c"
#include "../assets/weather/isolated_thunderstorms_day.c"
#include "../assets/weather/isolated_thunderstorms_night.c"
#include "../assets/weather/scattered_thunderstorms_day.c"
#include "../assets/weather/scattered_thunderstorms_night.c"
#include "../assets/weather/severe_thunderstorm.c"

const lv_image_dsc_t *get_weather_icon(const char *icon_name)
{
    if (icon_name == NULL)
        return &cloudy_3_day;

    char normalized[64];
    size_t index = 0;

    for (; icon_name[index] != '\0' && index < sizeof(normalized) - 1; ++index)
        normalized[index] = icon_name[index] == '-' ? '_' : icon_name[index];

    normalized[index] = '\0';

    if (strcmp(normalized, "clear-day") == 0)
        return &clear_day;
    else if (strcmp(normalized, "clear-night") == 0)
        return &clear_night;
    else if (strcmp(normalized, "cloudy-1-day") == 0)
        return &cloudy_1_day;
    else if (strcmp(normalized, "cloudy-1-night") == 0)
        return &cloudy_1_night;
    else if (strcmp(normalized, "cloudy-2-day") == 0)
        return &cloudy_2_day;
    else if (strcmp(normalized, "cloudy-2-night") == 0)
        return &cloudy_2_night;
    else if (strcmp(normalized, "cloudy-3-day") == 0)
        return &cloudy_3_day;
    else if (strcmp(normalized, "cloudy-3-night") == 0)
        return &cloudy_3_night;
    else if (strcmp(normalized, "fog-day") == 0)
        return &fog_day;
    else if (strcmp(normalized, "fog-night") == 0)
        return &fog_night;
    else if (strcmp(normalized, "frost-day") == 0)
        return &frost_day;
    else if (strcmp(normalized, "frost-night") == 0)
        return &frost_night;
    else if (strcmp(normalized, "rainy-1-day") == 0)
        return &rainy_1_day;
    else if (strcmp(normalized, "rainy-1-night") == 0)
        return &rainy_1_night;
    else if (strcmp(normalized, "rainy-2-day") == 0)
        return &rainy_2_day;
    else if (strcmp(normalized, "rainy-2-night") == 0)
        return &rainy_2_night;
    else if (strcmp(normalized, "rainy-3-day") == 0)
        return &rainy_3_day;
    else if (strcmp(normalized, "rainy-3-night") == 0)
        return &rainy_3_night;
    else if (strcmp(normalized, "snowy-1-day") == 0)
        return &snowy_1_day;
    else if (strcmp(normalized, "snowy-1-night") == 0)
        return &snowy_1_night;
    else if (strcmp(normalized, "snowy-2-day") == 0)
        return &snowy_2_day;
    else if (strcmp(normalized, "snowy-2-night") == 0)
        return &snowy_2_night;
    else if (strcmp(normalized, "snowy-3-day") == 0)
        return &snowy_3_day;
    else if (strcmp(normalized, "snowy-3-night") == 0)
        return &snowy_3_night;
    else if (strcmp(normalized, "isolated-thunderstorms-day") == 0)
        return &isolated_thunderstorms_day;
    else if (strcmp(normalized, "isolated-thunderstorms-night") == 0)
        return &isolated_thunderstorms_night;
    else if (strcmp(normalized, "scattered-thunderstorms-day") == 0)
        return &scattered_thunderstorms_day;
    else if (strcmp(normalized, "scattered-thunderstorms-night") == 0)
        return &scattered_thunderstorms_night;
    else if (strcmp(normalized, "severe-thunderstorm") == 0)
        return &severe_thunderstorm;

    return &cloudy_3_day;
}
