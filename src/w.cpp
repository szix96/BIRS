#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include "Audio.h"

// =================================================================================
// WIFI & AUDIO STREAM SETUP
// =================================================================================
// Replace with your WiFi credentials
const char* ssid = "your_wifi";
const char* password = "your_wifi";

struct RadioStation {
    const char* name;
    const char* url;
};

RadioStation stations[] = {
    {"Classic Rock", "http://icast.connectmedia.hu/4748/mr7.mp3"},
    {"80s Hits", "http://icast.connectmedia.hu/4770/mr7.ogg"},
    {"Jazz24", "http://mr-stream.connectmedia.hu/4748/mr7.mp3"},
    {"Chillout", "http://icast.connectmedia.hu/4747/mr7.aac"},
};

int currentStation = 0;
const int numStations = sizeof(stations) / sizeof(stations[0]);

// =================================================================================
// PIN DEFINITIONS (from platformio.ini)
// =================================================================================
// We define them here as well to be explicit, though build flags are used.
// TFT & Touch
// #define TFT_MISO 19 (Used by library)
// #define TFT_MOSI 23 (Used by library)
// #define TFT_SCLK 18 (Used by library)
// #define TFT_CS   15 (Configured in User_Setup.h or build flags)
// #define TFT_DC   2  (Configured in User_Setup.h or build flags)
// #define TFT_RST  4  (Configured in User_Setup.h or build flags)
#define TOUCH_CS 21
#define BUTTON_PIN 0 // Button to scroll stations (Connect to GND)

// I2S Audio
#define I2S_BCLK 26
#define I2S_LRC  25
#define I2S_DOUT 22

// =================================================================================
// GLOBAL OBJECTS
// =================================================================================
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(TOUCH_CS);
Audio audio;

// =================================================================================
// DRAWING & UI FUNCTIONS
// =================================================================================
void update_ui() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(TC_DATUM);
    tft.drawString("Internet Radio", tft.width() / 2, 20);
    
    tft.setTextColor(TFT_CYAN, TFT_BLACK);
    tft.drawString("Playing:", tft.width() / 2, 60);
    
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(2);
    tft.drawString(stations[currentStation].name, tft.width() / 2, tft.height() / 2);
    tft.setTextSize(1);
    
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("Press Button to Scroll", tft.width() / 2, tft.height() - 40);
}

// =================================================================================
// SETUP
// =================================================================================
void setup() {
    Serial.begin(115200);
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // --- Initialize Display ---
    tft.init();
    tft.setRotation(1); // Adjust rotation as needed (0-3)
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE);
    tft.setTextDatum(MC_DATUM);
    tft.drawString("Connecting to WiFi...", tft.width() / 2, tft.height() / 2);

    // --- Initialize Touchscreen ---
    ts.begin();
    ts.setRotation(1); // Match TFT rotation

    // --- Connect to WiFi ---
    WiFi.begin(ssid, password);
    int wifi_try = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if(wifi_try++ > 20) { // Timeout after 10 seconds
            tft.fillScreen(TFT_RED);
            tft.drawString("WiFi Connection Failed!", tft.width() / 2, tft.height() / 2);
            while(1) delay(100);
        }
    }
    Serial.println("\nWiFi connected");

    // --- Initialize Audio ---
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(10); // 0-21 (Max volume to ensure output)
    
    // --- Start First Station ---
    audio.connecttohost(stations[currentStation].url);

    // --- Draw Initial UI ---
    update_ui();
}

// =================================================================================
// MAIN LOOP
// =================================================================================
void loop() {
    // Handle audio streaming
    audio.loop();

    // Handle button input
    if (digitalRead(BUTTON_PIN) == LOW) {
        delay(50); // Debounce
        if (digitalRead(BUTTON_PIN) == LOW) {
            currentStation++;
            if (currentStation >= numStations) {
                currentStation = 0;
            }
            audio.connecttohost(stations[currentStation].url);
            update_ui();
            
            // Wait for button release
            while(digitalRead(BUTTON_PIN) == LOW) { delay(10); }
        }
    }
}

// =================================================================================
// AUDIO LIBRARY CALLBACKS (optional)
// =================================================================================
void audio_info(const char *info){
    Serial.print("audio_info: "); Serial.println(info);
}
void audio_id3data(const char *info){
    Serial.print("id3data: "); Serial.println(info);
}
void audio_eof_stream(const char *info){
    Serial.print("eof_stream: "); Serial.println(info);
}
void audio_showstation(const char *info){
    Serial.print("station: "); Serial.println(info);
}
void audio_showstreamtitle(const char *info){
    Serial.print("streamtitle: "); Serial.println(info);
}
// Add other callbacks as needed, see library examples