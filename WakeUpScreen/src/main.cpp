#include <lvgl.h>

#include <cstdio>
#include <cstring>

#include "display.h"

#define LINK_BAUD_RATE 19200

HardwareSerial Link(1);

namespace
{
  lv_obj_t *ui_screen = nullptr;
  lv_obj_t *ui_clock_label = nullptr;
  lv_obj_t *ui_date_label = nullptr;

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

  ClockState clock_state;

  void refresh_clock_labels()
  {
    if (ui_clock_label != nullptr)
    {
      lv_label_set_text_fmt(ui_clock_label, "%02d:%02d:%02d", clock_state.hour, clock_state.minute, clock_state.second);
    }

    if (ui_date_label != nullptr)
    {
      lv_label_set_text_fmt(ui_date_label, "%02d/%02d/%04d", clock_state.day, clock_state.month, clock_state.year);
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
      second > 59
    ) return false;

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

  void process_serial_line(const char *line)
  {
    if (line == nullptr || line[0] == '\0') return;

    if (parse_set_time_command(line, clock_state))
      return refresh_clock_labels();

    if (std::strcmp(line, "TIME?") == 0)
      refresh_clock_labels();
  }

  void poll_serial_commands()
  {
    static char line_buffer[32];
    static size_t line_length = 0;

    while (Link.available() > 0)
    {
      const char incoming = static_cast<char>(Link.read());
      if (incoming == '\r') continue;

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

  void build_status_screen()
  {
    ui_screen = lv_screen_active();
    lv_obj_set_style_bg_color(ui_screen, lv_color_hex(0x0B1020), 0);
    lv_obj_set_style_bg_opa(ui_screen, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(ui_screen, 0, 0);
    lv_obj_clear_flag(ui_screen, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *container = lv_obj_create(ui_screen);
    lv_obj_set_size(container, LV_PCT(100), LV_PCT(100));
    lv_obj_center(container);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_set_flex_flow(container, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_gap(container, 18, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title_label = lv_label_create(container);
    lv_label_set_text(title_label, "REVEIL");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0x93C5FD), 0);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_14, 0);

    ui_clock_label = lv_label_create(container);
    lv_label_set_text(ui_clock_label, "00:00:00");
    lv_obj_set_style_text_color(ui_clock_label, lv_color_hex(0xF8FAFC), 0);
    lv_obj_set_style_text_font(ui_clock_label, &lv_font_montserrat_48, 0);

    ui_date_label = lv_label_create(container);
    lv_label_set_text(ui_date_label, "01/01/2026");
    lv_obj_set_style_text_color(ui_date_label, lv_color_hex(0xCBD5E1), 0);
    lv_obj_set_style_text_font(ui_date_label, &lv_font_montserrat_14, 0);

    refresh_clock_labels();
  }
} // namespace

void setup()
{
  setup_display();
  build_status_screen();

  Serial.println("Clock ready");
  Link.begin(LINK_BAUD_RATE, SERIAL_8N1, 18, 17);

  clock_state.valid = false;
  clock_state.last_sync_ms = millis();

  refresh_clock_labels();
}

void loop()
{
  loop_display();

  poll_serial_commands();
}
