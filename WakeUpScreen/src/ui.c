#include <stdbool.h>

#include <lvgl.h>

#include "ui.h"

#include "weather_icon.h"

static const char DEFAULT_ICON[] = "cloudy";

static lv_obj_t *ui_screen;
static lv_obj_t *clock_label;
static lv_obj_t *date_label;
static lv_obj_t *weather_card;
static lv_obj_t *weather_icon;
static lv_obj_t *weather_title;
static lv_obj_t *weather_caption;
static lv_obj_t *weather_hint;
static lv_obj_t *temperature_label;
static lv_obj_t *wind_label;
static lv_obj_t *sunrise_label;
static lv_obj_t *icon_label;
static const lv_image_dsc_t *current_icon = NULL;

static void apply_weather_mode(bool is_daytime)
{
    const lv_color_t card_color = is_daytime ? lv_color_hex(0xE7F2FF) : lv_color_hex(0x101A32);
    const lv_color_t accent_color = is_daytime ? lv_color_hex(0x2563EB) : lv_color_hex(0x93C5FD);
    const lv_color_t title_color = is_daytime ? lv_color_hex(0x0F172A) : lv_color_hex(0xF8FAFC);
    const lv_color_t body_color = is_daytime ? lv_color_hex(0x334155) : lv_color_hex(0xCBD5E1);
    const lv_color_t value_color = is_daytime ? lv_color_hex(0x0F172A) : lv_color_hex(0xF8FAFC);

    if (weather_card != NULL)
    {
        lv_obj_set_style_bg_color(weather_card, card_color, 0);
        lv_obj_set_style_border_color(weather_card, accent_color, 0);
    }

    if (weather_title != NULL)
    {
        lv_label_set_text(weather_title, is_daytime ? "DAY MODE" : "NIGHT MODE");
        lv_obj_set_style_text_color(weather_title, title_color, 0);
    }

    if (weather_caption != NULL)
    {
        lv_obj_set_style_text_color(weather_caption, body_color, 0);
    }

    if (weather_hint != NULL)
    {
        lv_label_set_text(weather_hint, is_daytime ? "Sunlight layout active" : "Night layout active");
        lv_obj_set_style_text_color(weather_hint, body_color, 0);
    }

    if (temperature_label != NULL)
    {
        lv_obj_set_style_text_color(temperature_label, value_color, 0);
    }

    if (wind_label != NULL)
    {
        lv_obj_set_style_text_color(wind_label, body_color, 0);
    }

    if (sunrise_label != NULL)
    {
        lv_obj_set_style_text_color(sunrise_label, body_color, 0);
    }

    if (icon_label != NULL)
    {
        lv_obj_set_style_text_color(icon_label, body_color, 0);
    }

    (void)accent_color;

    if (clock_label != NULL)
    {
        lv_obj_set_style_text_color(clock_label, lv_color_hex(0xF8FAFC), 0);
    }

    if (date_label != NULL)
    {
        lv_obj_set_style_text_color(date_label, lv_color_hex(0xCBD5E1), 0);
    }

    if (weather_icon != NULL)
    {
        // Keep original icon colors; full recolor can turn RGB565 icons into a flat block.
        lv_obj_set_style_img_recolor_opa(weather_icon, LV_OPA_TRANSP, 0);
    }
}

static void build_ui(void)
{
    ui_screen = lv_screen_active();
    lv_obj_set_style_bg_color(ui_screen, lv_color_hex(0x08111F), 0);
    lv_obj_set_style_bg_grad_color(ui_screen, lv_color_hex(0x14213D), 0);
    lv_obj_set_style_bg_grad_dir(ui_screen, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_pad_all(ui_screen, 0, 0);
    lv_obj_clear_flag(ui_screen, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *shell = lv_obj_create(ui_screen);
    lv_obj_set_size(shell, LV_PCT(100), LV_PCT(100));
    lv_obj_center(shell);
    lv_obj_set_style_bg_opa(shell, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(shell, 0, 0);
    lv_obj_set_style_pad_all(shell, 24, 0);
    lv_obj_set_style_pad_gap(shell, 18, 0);
    lv_obj_set_flex_flow(shell, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(shell, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(shell, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *hero_panel = lv_obj_create(shell);
    lv_obj_set_width(hero_panel, LV_PCT(100));
    lv_obj_set_flex_grow(hero_panel, 1);
    lv_obj_set_style_bg_color(hero_panel, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_bg_opa(hero_panel, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(hero_panel, 0, 0);
    lv_obj_set_style_radius(hero_panel, 28, 0);
    lv_obj_set_style_pad_all(hero_panel, 24, 0);
    lv_obj_set_style_pad_gap(hero_panel, 8, 0);
    lv_obj_set_flex_flow(hero_panel, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(hero_panel, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(hero_panel, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title_label = lv_label_create(hero_panel);
    lv_label_set_text(title_label, "REVEIL");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x93C5FD), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, 0);

    clock_label = lv_label_create(hero_panel);
    lv_label_set_text(clock_label, "00:00:00");
    lv_obj_set_style_text_color(clock_label, lv_color_hex(0xF8FAFC), 0);
    lv_obj_set_style_text_font(clock_label, &lv_font_montserrat_48, 0);

    date_label = lv_label_create(hero_panel);
    lv_label_set_text(date_label, "01/01/2026");
    lv_obj_set_style_text_color(date_label, lv_color_hex(0xCBD5E1), 0);
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_14, 0);

    weather_card = lv_obj_create(shell);
    lv_obj_set_width(weather_card, LV_PCT(100));
    lv_obj_set_style_bg_color(weather_card, lv_color_hex(0xE7F2FF), 0);
    lv_obj_set_style_bg_opa(weather_card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(weather_card, 1, 0);
    lv_obj_set_style_border_color(weather_card, lv_color_hex(0x2563EB), 0);
    lv_obj_set_style_radius(weather_card, 24, 0);
    lv_obj_set_style_pad_all(weather_card, 18, 0);
    lv_obj_set_style_pad_column(weather_card, 16, 0);
    lv_obj_set_flex_flow(weather_card, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(weather_card, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_clear_flag(weather_card, LV_OBJ_FLAG_SCROLLABLE);

    weather_icon = lv_image_create(weather_card);
    current_icon = get_weather_icon(DEFAULT_ICON);
    lv_image_set_src(weather_icon, current_icon);
    if (current_icon != NULL)
    {
        lv_obj_set_size(weather_icon, current_icon->header.w, current_icon->header.h);
    }

    lv_obj_t *weather_text = lv_obj_create(weather_card);
    lv_obj_set_width(weather_text, LV_PCT(100));
    lv_obj_set_style_bg_opa(weather_text, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(weather_text, 0, 0);
    lv_obj_set_style_pad_all(weather_text, 0, 0);
    lv_obj_set_style_pad_gap(weather_text, 6, 0);
    lv_obj_set_flex_flow(weather_text, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(weather_text, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    lv_obj_set_flex_grow(weather_text, 1);
    lv_obj_clear_flag(weather_text, LV_OBJ_FLAG_SCROLLABLE);

    weather_title = lv_label_create(weather_text);
    lv_label_set_text(weather_title, "WEATHER");
    lv_obj_set_style_text_color(weather_title, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_text_font(weather_title, &lv_font_montserrat_14, 0);

    weather_caption = lv_label_create(weather_text);
    lv_label_set_text(weather_caption, "Waiting for weather payload");
    lv_obj_set_style_text_color(weather_caption, lv_color_hex(0x334155), 0);
    lv_obj_set_style_text_font(weather_caption, &lv_font_montserrat_14, 0);

    temperature_label = lv_label_create(weather_text);
    lv_label_set_text(temperature_label, "0°C");
    lv_obj_set_style_text_color(temperature_label, lv_color_hex(0x0F172A), 0);
    lv_obj_set_style_text_font(temperature_label, &lv_font_montserrat_14, 0);

    wind_label = lv_label_create(weather_text);
    lv_label_set_text(wind_label, "Wind: 0 km/h");
    lv_obj_set_style_text_color(wind_label, lv_color_hex(0x334155), 0);
    lv_obj_set_style_text_font(wind_label, &lv_font_montserrat_14, 0);

    sunrise_label = lv_label_create(weather_text);
    lv_label_set_text(sunrise_label, "Sunrise: --");
    lv_obj_set_style_text_color(sunrise_label, lv_color_hex(0x334155), 0);
    lv_obj_set_style_text_font(sunrise_label, &lv_font_montserrat_14, 0);

    icon_label = lv_label_create(weather_text);
    lv_label_set_text(icon_label, "Icon: cloudy");
    lv_obj_set_style_text_color(icon_label, lv_color_hex(0x334155), 0);
    lv_obj_set_style_text_font(icon_label, &lv_font_montserrat_14, 0);

    weather_hint = lv_label_create(weather_text);
    lv_label_set_text(weather_hint, "Sunlight layout active");
    lv_obj_set_style_text_color(weather_hint, lv_color_hex(0x334155), 0);
    lv_obj_set_style_text_font(weather_hint, &lv_font_montserrat_14, 0);
}

void setup_ui(void)
{
    build_ui();
    apply_weather_mode(true);
}

void ui_set_clock(int year, int month, int day, int hour, int minute, int second)
{
    if (clock_label != NULL)
    {
        lv_label_set_text_fmt(clock_label, "%02d:%02d:%02d", hour, minute, second);
    }

    if (date_label != NULL)
    {
        lv_label_set_text_fmt(date_label, "%02d/%02d/%04d", day, month, year);
    }

    apply_weather_mode(hour >= 6 && hour < 18);
}

void ui_set_weather(int temperature_c, int wind_kmh, const char *sunrise_iso8601, const char *icon_name)
{
    if (weather_icon != NULL)
    {
        const lv_image_dsc_t *icon = get_weather_icon(icon_name);
        const lv_image_dsc_t *to_set = icon != NULL ? icon : get_weather_icon(DEFAULT_ICON);

        lv_image_set_src(weather_icon, to_set);

        /* Ensure the image object matches the descriptor size so LVGL can render it */
        if (to_set != NULL)
        {
            lv_obj_set_width(weather_icon, to_set->header.w);
            lv_obj_set_height(weather_icon, to_set->header.h);
        }
        /* store last-set descriptor for debugging */

        current_icon = to_set;
    }

    if (temperature_label != NULL)
    {
        lv_label_set_text_fmt(temperature_label, "%d°C", temperature_c);
    }

    if (wind_label != NULL)
    {
        lv_label_set_text_fmt(wind_label, "Wind: %d km/h", wind_kmh);
    }

    if (sunrise_label != NULL)
    {
        lv_label_set_text_fmt(sunrise_label, "Sunrise: %s", sunrise_iso8601 != NULL ? sunrise_iso8601 : "--");
    }

    if (icon_label != NULL)
    {
        lv_label_set_text_fmt(icon_label, "Icon: %s", icon_name != NULL ? icon_name : "cloudy");
    }

    if (weather_caption != NULL)
    {
        lv_label_set_text(weather_caption, "Weather payload received");
    }
}

const lv_image_dsc_t *ui_get_current_icon(void)
{
    return current_icon;
}

void ui_get_icon_obj_size(int *w, int *h)
{
    if (w) *w = weather_icon ? lv_obj_get_width(weather_icon) : 0;
    if (h) *h = weather_icon ? lv_obj_get_height(weather_icon) : 0;
}
