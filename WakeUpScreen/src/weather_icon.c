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

    if (strcmp(icon_name, "clear_day") == 0)
        return &clear_day;
    else if (strcmp(icon_name, "clear_night") == 0)
        return &clear_night;
    else if (strcmp(icon_name, "cloudy_1_day") == 0)
        return &cloudy_1_day;
    else if (strcmp(icon_name, "cloudy_1_night") == 0)
        return &cloudy_1_night;
    else if (strcmp(icon_name, "cloudy_2_day") == 0)
        return &cloudy_2_day;
    else if (strcmp(icon_name, "cloudy_2_night") == 0)
        return &cloudy_2_night;
    else if (strcmp(icon_name, "cloudy_3_day") == 0)
        return &cloudy_3_day;
    else if (strcmp(icon_name, "cloudy_3_night") == 0)
        return &cloudy_3_night;
    else if (strcmp(icon_name, "fog_day") == 0)
        return &fog_day;
    else if (strcmp(icon_name, "fog_night") == 0)
        return &fog_night;
    else if (strcmp(icon_name, "frost_day") == 0)
        return &frost_day;
    else if (strcmp(icon_name, "frost_night") == 0)
        return &frost_night;
    else if (strcmp(icon_name, "rainy_1_day") == 0)
        return &rainy_1_day;
    else if (strcmp(icon_name, "rainy_1_night") == 0)
        return &rainy_1_night;
    else if (strcmp(icon_name, "rainy_2_day") == 0)
        return &rainy_2_day;
    else if (strcmp(icon_name, "rainy_2_night") == 0)
        return &rainy_2_night;
    else if (strcmp(icon_name, "rainy_3_day") == 0)
        return &rainy_3_day;
    else if (strcmp(icon_name, "rainy_3_night") == 0)
        return &rainy_3_night;
    else if (strcmp(icon_name, "snowy_1_day") == 0)
        return &snowy_1_day;
    else if (strcmp(icon_name, "snowy_1_night") == 0)
        return &snowy_1_night;
    else if (strcmp(icon_name, "snowy_2_day") == 0)
        return &snowy_2_day;
    else if (strcmp(icon_name, "snowy_2_night") == 0)
        return &snowy_2_night;
    else if (strcmp(icon_name, "snowy_3_day") == 0)
        return &snowy_3_day;
    else if (strcmp(icon_name, "snowy_3_night") == 0)
        return &snowy_3_night;
    else if (strcmp(icon_name, "isolated_thunderstorms_day") == 0)
        return &isolated_thunderstorms_day;
    else if (strcmp(icon_name, "isolated_thunderstorms_night") == 0)
        return &isolated_thunderstorms_night;
    else if (strcmp(icon_name, "scattered_thunderstorms_day") == 0)
        return &scattered_thunderstorms_day;
    else if (strcmp(icon_name, "scattered_thunderstorms_night") == 0)
        return &scattered_thunderstorms_night;
    else if (strcmp(icon_name, "severe_thunderstorm") == 0)
        return &severe_thunderstorm;

    return &cloudy_3_day;
}
