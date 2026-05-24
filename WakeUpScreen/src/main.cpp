#include <lvgl.h>

#include <cstring>

#include "display.h"

HardwareSerial Link(1);

namespace
{
  lv_obj_t *ui_screen = nullptr;
  lv_obj_t *ui_status_circle = nullptr;
  lv_obj_t *ui_status_label = nullptr;
  lv_obj_t *ui_touch_button = nullptr;
  lv_obj_t *ui_touch_feedback_label = nullptr;
  uint32_t touch_count = 0;

  void touch_button_event_cb(lv_event_t *e)
  {
    if (lv_event_get_code(e) != LV_EVENT_CLICKED || ui_touch_feedback_label == nullptr || ui_touch_button == nullptr)
    {
      return;
    }

    touch_count++;
    lv_label_set_text_fmt(ui_touch_feedback_label, "Touch OK (%lu)", static_cast<unsigned long>(touch_count));
    lv_obj_set_style_bg_color(ui_touch_button, lv_color_hex(0x22C55E), 0);
    lv_obj_set_style_border_color(ui_touch_button, lv_color_hex(0x22C55E), 0);
  }

  void apply_led_state(bool enabled)
  {
    if (ui_status_circle == nullptr) return;

    const lv_color_t color = enabled ? lv_color_hex(0x22C55E) : lv_color_hex(0xEF4444);

    lv_obj_set_style_bg_color(ui_status_circle, color, 0);
    lv_obj_set_style_border_color(ui_status_circle, color, 0);
  }

  void process_serial_line(const char *line)
  {
    if (line == nullptr || line[0] == '\0')
    {
      return;
    }

    if (std::strcmp(line, "LED_OFF") == 0)
    {
      apply_led_state(false);
      return;
    }

    apply_led_state(true);
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
      else line_length = 0;
    }
  }

  void build_status_screen()
  {
    ui_screen = lv_screen_active();
    lv_obj_set_style_bg_color(ui_screen, lv_color_hex(0x0F172A), 0);
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
    lv_obj_set_style_pad_gap(container, 24, 0);
    lv_obj_clear_flag(container, LV_OBJ_FLAG_SCROLLABLE);

    ui_status_circle = lv_obj_create(container);
    lv_obj_set_size(ui_status_circle, 180, 180);
    lv_obj_set_style_radius(ui_status_circle, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(ui_status_circle, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(ui_status_circle, 0, 0);
    lv_obj_set_style_shadow_width(ui_status_circle, 0, 0);
    lv_obj_clear_flag(ui_status_circle, LV_OBJ_FLAG_SCROLLABLE);

    ui_status_label = lv_label_create(container);
    lv_label_set_text(ui_status_label, "Etat de la LED");
    lv_obj_set_style_text_color(ui_status_label, lv_color_hex(0xF8FAFC), 0);
    lv_obj_set_style_text_font(ui_status_label, &lv_font_montserrat_14, 0);

    ui_touch_button = lv_button_create(container);
    lv_obj_set_size(ui_touch_button, 220, 52);
    lv_obj_set_style_radius(ui_touch_button, 14, 0);
    lv_obj_set_style_bg_color(ui_touch_button, lv_color_hex(0x2563EB), 0);
    lv_obj_set_style_border_width(ui_touch_button, 0, 0);
    lv_obj_add_event_cb(ui_touch_button, touch_button_event_cb, LV_EVENT_CLICKED, nullptr);

    lv_obj_t *button_label = lv_label_create(ui_touch_button);
    lv_label_set_text(button_label, "Tester le tactile");
    lv_obj_set_style_text_color(button_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_center(button_label);

    ui_touch_feedback_label = lv_label_create(container);
    lv_label_set_text(ui_touch_feedback_label, "Appuie sur le bouton");
    lv_obj_set_style_text_color(ui_touch_feedback_label, lv_color_hex(0xCBD5E1), 0);
    lv_obj_set_style_text_font(ui_touch_feedback_label, &lv_font_montserrat_14, 0);

    apply_led_state(false);
  }
} // namespace

void setup()
{
  setup_display();
  build_status_screen();

  Serial0.begin(115200);
  Link.begin(9600, SERIAL_8N1, 18, 17);

  delay(2000);
}

void loop()
{
  loop_display();

  // if (Link.available()) {
  //   String msg = Link.readStringUntil('\n');
  //   msg.trim();
  //   Serial0.println("From screen: " + msg);
  // }

  poll_serial_commands();

  // if (Serial0.available() > 0)
  // {
  //   String packet = Serial0.readStringUntil('\n');
  //   packet.trim();

  //   apply_led_state(true);
  //   delay(1000);
  //   apply_led_state(false);
    // apply_led_state(packet == "LED_ON");
  // }
}
