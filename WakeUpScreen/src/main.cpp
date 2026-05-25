#include <lvgl.h>

#include <cstdio>
#include <cstring>

#include "display.h"
#include "ui.h"
#include "weather_icon.h"

#define LINK_BAUD_RATE 19200

HardwareSerial Link(1);

namespace
{
  struct ClockState
  {
    int year = 2026;
    int month = 1;
    int day = 1;
    int hour = 0;
    int minute = 0;
    int second = 0;
    uint32_t last_sync_ms = 0;
    bool valid = false;
  };

  struct WeatherState
  {
    int temperature_c = 0;
    int wind_kmh = 0;
    char sunrise[32] = "";
    char icon[64] = "cloudy";
    bool valid = false;
  };

  ClockState clock_state;
  WeatherState weather_state;

  void sync_ui_clock()
  {
    ui_set_clock(
        clock_state.year,
        clock_state.month,
        clock_state.day,
        clock_state.hour,
        clock_state.minute,
        clock_state.second);
  }

  void sync_ui_weather()
  {
    // Debug: fetch descriptor and print some fields to serial to verify
    const lv_image_dsc_t *d = get_weather_icon(weather_state.icon);
    if (d != nullptr)
    {
      Serial0.printf("get_weather_icon('%s') -> addr=%p w=%u h=%u data_size=%u\n", weather_state.icon, (void *)d, (unsigned)d->header.w, (unsigned)d->header.h, (unsigned)d->data_size);
    }
    else
    {
      Serial0.printf("get_weather_icon('%s') -> NULL\n", weather_state.icon);
    }

    ui_set_weather(
        weather_state.temperature_c,
        weather_state.wind_kmh,
        weather_state.sunrise,
        weather_state.icon);

    const lv_image_dsc_t *set_d = ui_get_current_icon();
    int img_w = 0, img_h = 0;
    ui_get_icon_obj_size(&img_w, &img_h);
    if (set_d != nullptr)
    {
      Serial0.printf("ui_set_weather set descriptor addr=%p w=%u h=%u data_size=%u obj_size=%d x %d\n",
                    (void *)set_d,
                    (unsigned)set_d->header.w,
                    (unsigned)set_d->header.h,
                    (unsigned)set_d->data_size,
                    img_w,
                    img_h);
    }
    else
    {
      Serial0.printf("ui_set_weather set descriptor = NULL obj_size=%d x %d\n", img_w, img_h);
    }
  }

  bool parse_set_time_command(const char *line, ClockState &state)
  {
    if (line == nullptr || std::strncmp(line, "SET_TIME:", 9) != 0)
      return false;

    int day = 0;
    int month = 0;
    int year = 0;
    int hour = 0;
    int minute = 0;
    int second = 0;

    if (std::sscanf(line, "SET_TIME:%2d/%2d/%4d,%2d:%2d:%2d", &day, &month, &year, &hour, &minute, &second) != 6)
      return false;

    if (
        day < 1 ||
        day > 31 ||
        month < 1 ||
        month > 12 ||
        hour < 0 ||
        hour > 23 ||
        minute < 0 ||
        minute > 59 ||
        second < 0 ||
        second > 59)
      return false;

    state.day = day;
    state.month = month;
    state.year = year;
    state.hour = hour;
    state.minute = minute;
    state.second = second;
    state.last_sync_ms = millis();
    state.valid = true;

    return true;
  }

  bool parse_set_weather_command(const char *line, WeatherState &state)
  {
    if (line == nullptr || std::strncmp(line, "SET_WEATHER:", 12) != 0)
      return false;

    int temperature_c = 0;
    int wind_kmh = 0;
    char sunrise[32] = {0};
    char icon[64] = {0};

    if (std::sscanf(line, "SET_WEATHER:%d,%d,%31[^,],%63s", &temperature_c, &wind_kmh, sunrise, icon) != 4)
      return false;

    state.temperature_c = temperature_c;
    state.wind_kmh = wind_kmh;
    std::strncpy(state.sunrise, sunrise, sizeof(state.sunrise) - 1);
    std::strncpy(state.icon, icon, sizeof(state.icon) - 1);
    state.valid = true;

    return true;
  }

  void process_serial_line(const char *line)
  {
    if (line == nullptr || line[0] == '\0')
      return;

    if (parse_set_time_command(line, clock_state))
      return sync_ui_clock();

    if (parse_set_weather_command(line, weather_state))
      return sync_ui_weather();

    if (std::strcmp(line, "TIME?") == 0)
      sync_ui_clock();
  }

  void poll_serial_commands()
  {
    static char line_buffer[128];
    static size_t line_length = 0;

    while (Link.available() > 0)
    {
      const char incoming = static_cast<char>(Link.read());
      if (incoming == '\r')
        continue;

      if (incoming == '\n')
      {
        line_buffer[line_length] = '\0';
        process_serial_line(line_buffer);
        line_length = 0;

        continue;
      }

      if (line_length < sizeof(line_buffer) - 1)
        line_buffer[line_length++] = incoming;
      else
        line_length = 0;
    }
  }
} // namespace

void setup()
{
  setup_display();
  setup_ui();

  Serial0.begin(115200);
  Link.begin(LINK_BAUD_RATE, SERIAL_8N1, 18, 17);

  Serial0.println("Clock ready");

  clock_state.valid = false;
  clock_state.last_sync_ms = millis();
  weather_state.valid = false;

  sync_ui_clock();
  sync_ui_weather();
}

void loop()
{
  loop_display();
  poll_serial_commands();
}
