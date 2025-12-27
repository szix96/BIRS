#include <Arduino.h>
#include <WiFi.h>
#include <WebServer.h>
#include <Preferences.h>
#include "Audio.h"

// =================================================================================
// WIFI & AUDIO STREAM SETUP
// =================================================================================
// Replace with your WiFi credentials
String ssid = "your_wifi";
String password = "your_wifi";

struct RadioStation {
    String name;
    String url;
};

RadioStation stations[] = {
    {"Danko", "http://icast.connectmedia.hu/4748/mr7.mp3"},
    {"kossuth", "http://icast.connectmedia.hu/4736/mr1.mp3"},
    {"petofi", "http://icast.connectmedia.hu/4738/mr2.mp3"},
    {"danubius", "http://stream.danubiusradio.hu/danubius_320k"},
    {"juventus", "https://s2.audiostream.hu/juventus_320k"},
    {"radio1", "http://icast.connectmedia.hu/5202/live.mp3"},
    {"slager", "https://slagerfm.netregator.hu:7813/slagerfm256.mp3"},
    {"retro", "http://icast.connectmedia.hu/5002/live.mp3"},
    {"mulatos", "http://stream.lazaradio.com/mulatos.mp3"},
};

int currentStation = 0;
const int numStations = sizeof(stations) / sizeof(stations[0]);

// =================================================================================
// PIN DEFINITIONS (from platformio.ini)
// =================================================================================
// We define them here as well to be explicit, though build flags are used.
#define BUTTON_NEXT_PIN 4 // Button to next station (Connect to GND)
#define BUTTON_PREV_PIN 13 // Button to previous station (Connect to GND)

// I2S Audio
#define I2S_BCLK 26
#define I2S_LRC  25
#define I2S_DOUT 22

// =================================================================================
// GLOBAL OBJECTS
// =================================================================================
Audio audio;
WebServer server(80);
Preferences preferences;
bool inAPMode = false;

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

void handleWifiConfig() {
    String html = "<!DOCTYPE html><html><head><title>WiFi Config</title></head><body>";
    html += "<h1>Configure WiFi</h1>";
    html += "<form action='/savewifi' method='POST'>";
    html += "SSID: <input type='text' name='ssid' placeholder='SSID'><br><br>";
    html += "Password: <input type='password' name='pass' placeholder='Password'><br><br>";
    html += "<input type='submit' value='Save & Reboot'>";
    html += "</form></body></html>";
    server.send(200, "text/html", html);
}

void handleWifiSave() {
    if (server.hasArg("ssid") && server.hasArg("pass")) {
        preferences.putString("ssid", server.arg("ssid"));
        preferences.putString("password", server.arg("pass"));
        server.send(200, "text/html", "<h1>Saved! Rebooting...</h1>");
        delay(500);
        ESP.restart();
    } else {
        server.send(400, "text/plain", "Missing SSID or Password");
    }
}

// =================================================================================
// SETUP
// =================================================================================
void setup() {
    Serial.begin(115200);
    
    pinMode(BUTTON_NEXT_PIN, INPUT_PULLUP);
    pinMode(BUTTON_PREV_PIN, INPUT_PULLUP);

    preferences.begin("radio-config", false);
    String saved_ssid = preferences.getString("ssid", "");
    String saved_pass = preferences.getString("password", "");
    if(saved_ssid != "") {
        ssid = saved_ssid;
        password = saved_pass;
    }

    // --- Connect to WiFi ---
    Serial.println("Connecting to WiFi: " + ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid.c_str(), password.c_str());
    int wifi_try = 0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if(wifi_try++ > 20) { // Timeout after 10 seconds
            Serial.println("\nWiFi Connection Failed! Starting AP...");
            inAPMode = true;
            WiFi.disconnect();
            WiFi.mode(WIFI_AP);
            WiFi.softAP("ESP32-Radio-Config");
            Serial.print("AP IP Address: ");
            Serial.println(WiFi.softAPIP());
            
            server.on("/", handleWifiConfig);
            server.on("/savewifi", handleWifiSave);
            server.begin();
            return;
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
    audio.setVolume(8); // 0-21 (Max volume to ensure output)
    
    // --- Start First Station ---
    Serial.printf("Playing: %s\n", stations[currentStation].name.c_str());
    audio.connecttohost(stations[currentStation].url.c_str());
}

// =================================================================================
// MAIN LOOP
// =================================================================================
void loop() {
    if (inAPMode) {
        server.handleClient();
        return;
    }

    // Handle audio streaming
    audio.loop();
    server.handleClient();

    // Handle button input
    if (digitalRead(BUTTON_NEXT_PIN) == LOW) {
        delay(50); // Debounce
        if (digitalRead(BUTTON_NEXT_PIN) == LOW) {
            currentStation++;
            if (currentStation >= numStations) {
                currentStation = 0;
            }
            Serial.printf("Switching to: %s\n", stations[currentStation].name.c_str());
            audio.connecttohost(stations[currentStation].url.c_str());
            
            // Wait for button release
            while(digitalRead(BUTTON_NEXT_PIN) == LOW) { delay(10); }
        }
    }

    if (digitalRead(BUTTON_PREV_PIN) == LOW) {
        delay(50); // Debounce
        if (digitalRead(BUTTON_PREV_PIN) == LOW) {
            currentStation--;
            if (currentStation < 0) {
                currentStation = numStations - 1;
            }
            Serial.printf("Switching to: %s\n", stations[currentStation].name.c_str());
            audio.connecttohost(stations[currentStation].url.c_str());
            
            // Wait for button release
            while(digitalRead(BUTTON_PREV_PIN) == LOW) { delay(10); }
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