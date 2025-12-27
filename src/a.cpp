#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include "Audio.h"

// =================================================================================
// WIFI & AUDIO STREAM SETUP
// =================================================================================
// Replace with your WiFi credentials
const char* ssid = "your_wifi";
const char* password = "your_wifi";

struct RadioStation {
    String name;
    String url;
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
#define BUTTON_PIN 0 // Button to scroll stations (Connect to GND)

// I2S Audio
#define I2S_BCLK 26
#define I2S_LRC  25
#define I2S_DOUT 22

// =================================================================================
// GLOBAL OBJECTS
// =================================================================================
Audio audio;
WebServer server(80);

void handleRoot() {
    String html = "<!DOCTYPE html><html><head><title>ESP32 Radio</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:sans-serif;margin:20px} input{width:100%;margin-bottom:10px;padding:5px}</style>";
    html += "</head><body><h1>Radio Stations</h1>";
    html += "<form action='/save' method='POST'>";
    for (int i = 0; i < numStations; i++) {
        html += "<h3>Station " + String(i + 1) + "</h3>";
        html += "Name: <input type='text' name='n" + String(i) + "' value='" + stations[i].name + "'>";
        html += "URL: <input type='text' name='u" + String(i) + "' value='" + stations[i].url + "'>";
    }
    html += "<br><br><input type='submit' value='Save Changes' style='width:auto;padding:10px 20px'>";
    html += "</form></body></html>";
    server.send(200, "text/html", html);
}

void handleSave() {
    for (int i = 0; i < numStations; i++) {
        if (server.hasArg("n" + String(i))) stations[i].name = server.arg("n" + String(i));
        if (server.hasArg("u" + String(i))) stations[i].url = server.arg("u" + String(i));
    }
    server.send(200, "text/html", "<html><body><h1>Saved!</h1><a href='/'>Back</a></body></html>");
}

// =================================================================================
// SETUP
// =================================================================================
void setup() {
    Serial.begin(115200);
    
    pinMode(BUTTON_PIN, INPUT_PULLUP);

    // --- Connect to WiFi ---
    Serial.println("Connecting to WiFi...");
    WiFi.begin(ssid, password);
    int wifi_try = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if(wifi_try++ > 20) { // Timeout after 10 seconds
            Serial.println("\nWiFi Connection Failed!");
            while(1) delay(100);
        }
    }
    Serial.println("\nWiFi connected");
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    server.on("/", handleRoot);
    server.on("/save", handleSave);
    server.begin();

    // --- Initialize Audio ---
    audio.setPinout(I2S_BCLK, I2S_LRC, I2S_DOUT);
    audio.setVolume(10); // 0-21 (Max volume to ensure output)
    
    // --- Start First Station ---
    Serial.printf("Playing: %s\n", stations[currentStation].name.c_str());
    audio.connecttohost(stations[currentStation].url.c_str());
}

// =================================================================================
// MAIN LOOP
// =================================================================================
void loop() {
    // Handle audio streaming
    audio.loop();
    server.handleClient();

    // Handle button input
    if (digitalRead(BUTTON_PIN) == LOW) {
        delay(50); // Debounce
        if (digitalRead(BUTTON_PIN) == LOW) {
            currentStation++;
            if (currentStation >= numStations) {
                currentStation = 0;
            }
            Serial.printf("Switching to: %s\n", stations[currentStation].name.c_str());
            audio.connecttohost(stations[currentStation].url.c_str());
            
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