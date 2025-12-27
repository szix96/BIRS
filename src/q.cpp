#include <Arduino.h>
#include <WiFi.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>
#include <Audio.h>

// -----------------------------------------------------------------------------
// USER CONFIGURATION
// -----------------------------------------------------------------------------
// Replace with your WiFi credentials
const char* ssid = "your_wifi";
const char* password = "your_wifi";

// -----------------------------------------------------------------------------
// HARDWARE & DISPLAY SETUP
// -----------------------------------------------------------------------------
// Pin definitions are set in platformio.ini, but we define them here to prevent
// compilation errors if the build flags aren't passed correctly.
// Make sure these match the values in your platformio.ini
#define I2S_BCLK 26
#define I2S_LRC  25
#define I2S_DOUT 22

// Screen dimensions
#define SCREEN_WIDTH  240
#define SCREEN_HEIGHT 320

// -----------------------------------------------------------------------------
// OBJECTS
// -----------------------------------------------------------------------------
TFT_eSPI tft = TFT_eSPI();
XPT2046_Touchscreen ts(TOUCH_CS);
Audio audio;

// -----------------------------------------------------------------------------
// RADIO STATIONS
// -----------------------------------------------------------------------------
struct RadioStation {
    const char* name;
    const char* url;
};

RadioStation stations[] = {
    {"Classic Rock", "http://173.236.103.98:8000/stream"},
    {"80s Hits", "http://80s.vdradio.net:8000/stream"},
    {"Jazz24", "http://live.wostreaming.net/direct/kplu-jazz24-128mp3"},
    {"Chillout", "http://icecast.vrt.be/stubru-tijdloze-high.mp3"},
};
const int numStations = sizeof(stations) / sizeof(stations[0]);
int currentStation = 0;

// -----------------------------------------------------------------------------
// UI & TOUCH HANDLING
// -----------------------------------------------------------------------------
#define BUTTON_HEIGHT 40
#define BUTTON_MARGIN 10
#define STOP_BUTTON_Y (SCREEN_HEIGHT - BUTTON_HEIGHT - BUTTON_MARGIN)

void drawUI() {
    tft.fillScreen(TFT_BLACK);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextDatum(MC_DATUM); // Middle-center alignment

    // Draw station buttons
    for (int i = 0; i < numStations; i++) {
        int y = BUTTON_MARGIN + i * (BUTTON_HEIGHT + BUTTON_MARGIN);
        tft.drawRect(BUTTON_MARGIN, y, SCREEN_WIDTH - 2 * BUTTON_MARGIN, BUTTON_HEIGHT, TFT_CYAN);
        tft.drawString(stations[i].name, SCREEN_WIDTH / 2, y + BUTTON_HEIGHT / 2);
    }

    // Draw Stop button
    tft.drawRect(BUTTON_MARGIN, STOP_BUTTON_Y, SCREEN_WIDTH - 2 * BUTTON_MARGIN, BUTTON_HEIGHT, TFT_RED);
    tft.drawString("STOP", SCREEN_WIDTH / 2, STOP_BUTTON_Y + BUTTON_HEIGHT / 2);
}

void showStatus(const char* message) {
    tft.fillRect(0, SCREEN_HEIGHT / 2 - 20, SCREEN_WIDTH, 40, TFT_BLACK);
    tft.drawString(message, SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2);
    Serial.println(message);
}

void playStation(int index) {
    if (index >= 0 && index < numStations) {
        currentStation = index;
        showStatus("Connecting...");
        audio.connecttohost(stations[currentStation].url);
        tft.fillRect(BUTTON_MARGIN,  BUTTON_MARGIN + currentStation * (BUTTON_HEIGHT + BUTTON_MARGIN), SCREEN_WIDTH - 2 * BUTTON_MARGIN, BUTTON_HEIGHT, TFT_GREEN);
        tft.drawString(stations[currentStation].name, SCREEN_WIDTH / 2, BUTTON_MARGIN + currentStation * (BUTTON_HEIGHT + BUTTON_MARGIN) + BUTTON_HEIGHT / 2);
    }
}

// -----------------------------------------------------------------------------
// SETUP
// -----------------------------------------------------------------------------
void setup() {
    Serial.begin(115200);

    // --- Display & Touch Init ---
    tft.init();
    tft.setRotation(0); // Adjust rotation if needed (0-3)
    tft.fillScreen(TFT_BLACK);

    ts.begin();
    ts.setRotation(0);

    drawUI();
    showStatus("Connecting to WiFi...");

    // --- WiFi Connection ---
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nWiFi Connected!");
    showStatus("WiFi Connected!");
    delay(1000);

    // --- Audio Init ---
    // The I2S pins are defined in platformio.ini and passed via build flags
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(10); // 0...21

    // --- Start Radio ---
    playStation(0);
}

// -----------------------------------------------------------------------------
// MAIN LOOP
// -----------------------------------------------------------------------------
void loop() {
    audio.loop();

    if (ts.touched()) {
        TS_Point p = ts.getPoint();
        // Map touch coordinates to screen coordinates
        p.x = map(p.x, 200, 3800, 0, SCREEN_WIDTH);
        p.y = map(p.y, 200, 3800, 0, SCREEN_HEIGHT);
        
        // Invert Y axis if necessary, depending on your screen's orientation
        // p.y = SCREEN_HEIGHT - p.y;
        
        Serial.printf("Touch at: x=%d, y=%d\n", p.x, p.y);

        // Check station buttons
        for (int i = 0; i < numStations; i++) {
            int y = BUTTON_MARGIN + i * (BUTTON_HEIGHT + BUTTON_MARGIN);
            if (p.x > BUTTON_MARGIN && p.x < (SCREEN_WIDTH - BUTTON_MARGIN) && p.y > y && p.y < (y + BUTTON_HEIGHT)) {
                drawUI(); // Redraw to clear previous selection
                playStation(i);
                delay(200); // Debounce
                return;
            }
        }

        // Check Stop button
        if (p.x > BUTTON_MARGIN && p.x < (SCREEN_WIDTH - BUTTON_MARGIN) && p.y > STOP_BUTTON_Y && p.y < (STOP_BUTTON_Y + BUTTON_HEIGHT)) {
            showStatus("Stopping...");
            audio.stopSong();
            delay(500);
            drawUI();
        }
    }
}

// -----------------------------------------------------------------------------
// AUDIO LIBRARY CALLBACKS
// -----------------------------------------------------------------------------
void audio_info(const char *info){
    Serial.print("audio_info: ");
    Serial.println(info);
    // You could display this on the TFT
}
void audio_id3data(const char *info){
    Serial.print("id3data: ");
    Serial.println(info);
}
// Add other callbacks as needed, e.g., for metadata
// void audio_showstation(const char *info){}
// void audio_showstreamtitle(const char *info){}
// void audio_bitrate(const char *info){}
