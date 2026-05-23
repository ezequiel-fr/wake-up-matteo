#include <Adafruit_NeoPixel.h>
#include <DHT.h>
#include <lvgl.h>

#include "display.h"

// Hardware Pins
#define DHTPIN 17 // Change to your DHT11 Data Pin
#define DHTTYPE DHT11
#define RGB_PIN 18   // Change to your WS2812B Data Pin
#define NUM_PIXELS 8 // Number of LEDs in your strip

// Objects
DHT dht(DHTPIN, DHTTYPE);
Adafruit_NeoPixel strip(NUM_PIXELS, RGB_PIN, NEO_GRB + NEO_KHZ800);

// UI Pointers
lv_obj_t *ui_temp_label;
lv_obj_t *ui_hum_label;
// UI Element Pointers
lv_obj_t *ui_screen;
lv_obj_t *ui_card_container;
lv_obj_t *ui_clock_label;

// Function to create a styled "Control Card"
void create_control_card(lv_obj_t *parent, const char *title, const char *icon_text, bool has_slider, bool has_switch)
{
  // Card Base
  lv_obj_t *card = lv_obj_create(parent);
  lv_obj_set_size(card, 230, 180); // Sized for 3 cards per row on 800px width
  lv_obj_set_style_bg_color(card, lv_color_hex(0x1E293B), 0);
  lv_obj_set_style_border_width(card, 0, 0);
  lv_obj_set_style_radius(card, 12, 0);
  lv_obj_set_flex_flow(card, LV_FLEX_FLOW_COLUMN);
  lv_obj_set_flex_align(card, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);

  // Title & Icon Row
  lv_obj_t *header_row = lv_obj_create(card);
  lv_obj_set_size(header_row, LV_PCT(100), LV_SIZE_CONTENT);
  lv_obj_set_style_bg_opa(header_row, 0, 0);
  lv_obj_set_style_border_width(header_row, 0, 0);
  lv_obj_set_style_pad_all(header_row, 0, 0);
  lv_obj_set_flex_flow(header_row, LV_FLEX_FLOW_ROW);
  lv_obj_set_flex_align(header_row, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

  lv_obj_t *lbl_title = lv_label_create(header_row);
  lv_label_set_text(lbl_title, title);
  lv_obj_set_style_text_color(lbl_title, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_14, 0);

  // Control Elements
  if (has_switch)
  {
    lv_obj_t *sw = lv_switch_create(card);
    lv_obj_add_state(sw, LV_STATE_CHECKED);
  }

  if (has_slider)
  {
    lv_obj_t *slider = lv_slider_create(card);
    lv_obj_set_width(slider, LV_PCT(100));
    lv_slider_set_value(slider, 70, LV_ANIM_OFF);
  }
}

void build_smart_home_ui()
{
  ui_screen = lv_screen_active();
  lv_obj_set_style_bg_color(ui_screen, lv_color_hex(0x0F172A), 0); // Modern Dark Theme

  // 1. TOP HEADER BAR
  lv_obj_t *header = lv_obj_create(ui_screen);
  lv_obj_set_size(header, 800, 70);
  lv_obj_set_style_bg_color(header, lv_color_hex(0x111827), 0);
  lv_obj_set_style_border_side(header, LV_BORDER_SIDE_BOTTOM, 0);
  lv_obj_set_style_border_color(header, lv_color_hex(0x374151), 0);
  lv_obj_set_style_radius(header, 0, 0);
  lv_obj_align(header, LV_ALIGN_TOP_MID, 0, 0);

  lv_obj_t *main_title = lv_label_create(header);
  lv_label_set_text(main_title, "RESIDENCE DASHBOARD");
  lv_obj_set_style_text_color(main_title, lv_color_hex(0xF9FAFB), 0);
  lv_obj_set_style_text_font(main_title, &lv_font_montserrat_14, 0);
  lv_obj_align(main_title, LV_ALIGN_LEFT_MID, 15, 0);

  ui_clock_label = lv_label_create(header);
  lv_label_set_text(ui_clock_label, "13:45");
  lv_obj_set_style_text_color(ui_clock_label, lv_color_hex(0x9CA3AF), 0);
  lv_obj_set_style_text_font(ui_clock_label, &lv_font_montserrat_14, 0);
  lv_obj_align(ui_clock_label, LV_ALIGN_RIGHT_MID, -15, 0);

  // 2. MAIN CONTENT AREA (Flex Container)
  ui_card_container = lv_obj_create(ui_screen);
  lv_obj_set_size(ui_card_container, 800, 410);
  lv_obj_align(ui_card_container, LV_ALIGN_BOTTOM_MID, 0, 0);
  lv_obj_set_style_bg_opa(ui_card_container, 0, 0);
  lv_obj_set_style_border_width(ui_card_container, 0, 0);

  // Configure Flexbox for 800x480 grid
  lv_obj_set_flex_flow(ui_card_container, LV_FLEX_FLOW_ROW_WRAP);
  lv_obj_set_flex_align(ui_card_container, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
  lv_obj_set_style_pad_gap(ui_card_container, 20, 0);
  lv_obj_set_style_pad_top(ui_card_container, 20, 0);

  // 3. CREATE INDIVIDUAL CARDS
  create_control_card(ui_card_container, "Living Room Lights", "💡", true, true);
  create_control_card(ui_card_container, "Kitchen Fan", "🌪️", false, true);
  create_control_card(ui_card_container, "Bedroom Dimmer", "🌙", true, false);

  // Thermostat Arc Card
  lv_obj_t *thermo_card = lv_obj_create(ui_card_container);
  lv_obj_set_size(thermo_card, 230, 180);
  lv_obj_set_style_bg_color(thermo_card, lv_color_hex(0x1E293B), 0);
  lv_obj_set_style_border_width(thermo_card, 0, 0);
  lv_obj_set_style_radius(thermo_card, 12, 0);

  lv_obj_t *arc = lv_arc_create(thermo_card);
  lv_obj_set_size(arc, 120, 120);
  lv_arc_set_rotation(arc, 135);
  lv_arc_set_bg_angles(arc, 0, 270);
  lv_arc_set_value(arc, 22);
  lv_obj_center(arc);

  lv_obj_t *arc_lbl = lv_label_create(arc);
  lv_label_set_text(arc_lbl, "22°C");
  lv_obj_center(arc_lbl);

  lv_obj_t *thermo_title = lv_label_create(thermo_card);
  lv_label_set_text(thermo_title, "AC Temp");
  lv_obj_set_style_text_color(thermo_title, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(thermo_title, LV_ALIGN_TOP_LEFT, 0, 0);
}
// RGB Control Callback
static void rgb_slider_event_cb(lv_event_t *e)
{
  lv_obj_t *slider = (lv_obj_t *)lv_event_get_target(e);
  int value = lv_slider_get_value(slider);

  // Simple Hue to RGB conversion (simplified for example)
  // 0-255 mapped to Red -> Green -> Blue
  for (int i = 0; i < NUM_PIXELS; i++)
  {
    strip.setPixelColor(i, strip.Color(value, 255 - value, 128));
  }
  strip.show();
}

void add_sensor_and_rgb_sections(lv_obj_t *parent)
{
  // --- DHT11 CLIMATE CARD ---
  lv_obj_t *climate_card = lv_obj_create(parent);
  lv_obj_set_size(climate_card, 230, 180);
  lv_obj_set_style_bg_color(climate_card, lv_color_hex(0x1E293B), 0);
  lv_obj_set_style_border_width(climate_card, 0, 0);
  lv_obj_set_style_radius(climate_card, 12, 0);
  lv_obj_set_flex_flow(climate_card, LV_FLEX_FLOW_COLUMN);

  lv_obj_t *climate_title = lv_label_create(climate_card);
  lv_label_set_text(climate_title, "Climate (DHT11)");
  lv_obj_set_style_text_color(climate_title, lv_color_hex(0xFFFFFF), 0);

  ui_temp_label = lv_label_create(climate_card);
  lv_label_set_text(ui_temp_label, "Temp: --°C");
  lv_obj_set_style_text_color(ui_temp_label, lv_color_hex(0x60A5FA), 0);

  ui_hum_label = lv_label_create(climate_card);
  lv_label_set_text(ui_hum_label, "Humidity: --%");
  lv_obj_set_style_text_color(ui_hum_label, lv_color_hex(0x34D399), 0);

  // --- RGB NEOPIXEL CARD ---
  lv_obj_t *rgb_card = lv_obj_create(parent);
  lv_obj_set_size(rgb_card, 230, 180);
  lv_obj_set_style_bg_color(rgb_card, lv_color_hex(0x1E293B), 0);
  lv_obj_set_style_border_width(rgb_card, 0, 0);
  lv_obj_set_style_radius(rgb_card, 12, 0);

  lv_obj_t *rgb_title = lv_label_create(rgb_card);
  lv_label_set_text(rgb_title, "RGB Strip Control");
  lv_obj_set_style_text_color(rgb_title, lv_color_hex(0xFFFFFF), 0);
  lv_obj_align(rgb_title, LV_ALIGN_TOP_LEFT, 0, 0);

  lv_obj_t *rgb_slider = lv_slider_create(rgb_card);
  lv_obj_set_width(rgb_slider, LV_PCT(90));
  lv_obj_align(rgb_slider, LV_ALIGN_CENTER, 0, 10);
  lv_slider_set_range(rgb_slider, 0, 255);
  lv_obj_add_event_cb(rgb_slider, rgb_slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
}

// In your existing loop(), add a timer to update sensors
unsigned long last_sensor_read = 0;
void update_sensors()
{
  if (millis() - last_sensor_read > 2000)
  { // Update every 2 seconds
    float h = dht.readHumidity();
    float t = dht.readTemperature();

    if (!isnan(h) && !isnan(t))
    {
      lv_label_set_text_fmt(ui_temp_label, "Temp: %.1f°C", t);
      lv_label_set_text_fmt(ui_hum_label, "Humidity: %.1f%%", h);
    }
    last_sensor_read = millis();
  }
}

void setup()
{
  setup_display();
  dht.begin();
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  build_smart_home_ui();                          // Call your previous dashboard function
  add_sensor_and_rgb_sections(ui_card_container); // Add the new cards
}

void loop()
{
  // Display loop
  loop_display();
  // Sensor update loop
  update_sensors();
}
