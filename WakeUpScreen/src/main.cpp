#include <cstdio>
#include <cstring>

#include <lvgl.h>
#include <SPI.h>
#include <SD.h>

#include "AudioFileSourceSD.h"
#include "AudioGeneratorMP3.h"
#include "AudioOutputI2S.h"

#include "display.h"
#include "ui.h"
#include "weather_icon.h"

// Serial communication baud rate for the link to the main controller
#define LINK_BAUD_RATE 19200

// SD card chip select pin
#define SD_CS 10

// IS2 pin definitions for the audio output
#define I2S_DOUT 17
#define I2S_BCLK  0
#define I2S_LRC  18

HardwareSerial Link(1);

AudioGeneratorMP3 *mp3;
AudioFileSourceSD *file;
AudioOutputI2S    *out;

bool alarm_music_started = false;

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
    ui_set_weather(
        weather_state.temperature_c,
        weather_state.wind_kmh,
        weather_state.sunrise,
        weather_state.icon);

  }

  void release_alarm_music_source()
  {
    if (file != nullptr)
    {
      delete file;
      file = nullptr;
    }
  }

  bool start_alarm_music()
  {
    if (mp3 == nullptr || out == nullptr)
      return false;

    if (mp3->isRunning())
      mp3->stop();

    release_alarm_music_source();

    file = new AudioFileSourceSD("/music/bap.mp3");
    if (file == nullptr)
    {
      alarm_music_started = false;
      Link.println("Failed to open music file");
      return false;
    }

    if (!mp3->begin(file, out))
    {
      alarm_music_started = false;
      release_alarm_music_source();
      Link.println("Failed to start music");
      return false;
    }

    alarm_music_started = true;
    Link.println("Playing...");

    return true;
  }

  bool stop_alarm_music()
  {
    if (mp3 != nullptr && mp3->isRunning())
      mp3->stop();

    release_alarm_music_source();
    alarm_music_started = false;
    Link.println("Stopped");

    return true;
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

    if (std::strcmp(line, "RING:1") == 0)
      return start_alarm_music();

    if (std::strcmp(line, "RING:STOP") == 0)
      return stop_alarm_music();

    if (std::strcmp(line, "TIME?") == 0)
      return sync_ui_clock();

    Serial.printf("Unknown command received: '%s'\n", line);
  }

  void poll_serial_commands()
  {
    static char line_buffer[128];
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
} // namespace

void setup()
{
  setup_display();
  setup_ui();

  // Serial0.begin(115200);
  // Serial0.begin(LINK_BAUD_RATE, SERIAL_8N1, 18, 17);
  // Serial0.begin(LINK_BAUD_RATE, SERIAL_8N1, 44, 43);
  Link.begin(LINK_BAUD_RATE, SERIAL_8N1, 44, 43);

  // Serial0.println("Clock ready");

  clock_state.valid = false;
  clock_state.last_sync_ms = millis();
  weather_state.valid = false;

  sync_ui_clock();
  sync_ui_weather();

  // audio
  // SD card
  SPI.begin(12, 13, 11, SD_CS);

  if (!SD.begin(SD_CS)) {
    Link.println("SD Card failed!");
    while (1);
  }

  Link.println("SD OK");

  // Audio output
  out = new AudioOutputI2S();

  out->SetPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
  out->SetGain(0.05);

  // MP3 file
  file = new AudioFileSourceSD("/music/bap.mp3");
  mp3 = new AudioGeneratorMP3();
}

void loop()
{
  if (mp3->isRunning()) {
    if (!mp3->loop()) {
      mp3->stop();

      alarm_music_started = false;
      release_alarm_music_source();

      Link.println("Finished");
    }
  }

  loop_display();
  poll_serial_commands();
}
