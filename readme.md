Basic internet radio based on ESP32-wroom and PCM5102A

    Update WiFi Credentials:
        Open the src/main.cpp file.
        On lines 13 and 14, replace "YOUR_WIFI_SSID" and "YOUR_WIFI_PASSWORD" with your actual WiFi network name and password.

Set radio stations to your like: RadioStation stations[] = { {"Danko", "http://icast.connectmedia.hu/4748/mr7.mp3"}, {"kossuth", "http://icast.connectmedia.hu/4736/mr1.mp3"}, {"petofi", "http://icast.connectmedia.hu/4738/mr2.mp3"}, {"danubius", "http://stream.danubiusradio.hu/danubius_320k"}, {"juventus", "https://s2.audiostream.hu/juventus_320k"}, {"radio1", "http://icast.connectmedia.hu/5202/live.mp3"}, {"slager", "https://slagerfm.netregator.hu:7813/slagerfm256.mp3"}, {"retro", "http://icast.connectmedia.hu/5002/live.mp3"}, {"mulatos", "http://stream.lazaradio.com/mulatos.mp3"}, };

    Verify Hardware Pins:
        Open the platformio.ini file.
        Review the pin definitions under the build_flags section. These pins must match how you have wired your components to the ESP32.

        #define BUTTON_NEXT_PIN 4 // Button to next station (Connect to GND)
        #define BUTTON_PREV_PIN 13 // Button to previous station (Connect to GND)

        ; --- I2S Audio (PCM5102A) ---
        #define I2S_BCLK 26
        #define I2S_LRC  25
        #define I2S_DOUT 22

    The boot issue you mentioned (rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)) can often be caused by incorrect pin assignments, especially if certain pins required for booting are used for peripherals. Please double-check your wiring against the ESP32's datasheet and the pin definitions in this file.

Build and Upload:

    Connect your ESP32 board to your computer.
    Use the PlatformIO CLI to build and upload the project. You can do this with the following command in your terminal:

    platformio run --target upload

    Alternatively, you can use the "Upload" button within the Visual Studio Code PlatformIO extension.

Troubleshooting:

    If there is no sound, double-check the I2S pin connections to your PCM5102A board.
    Use the Serial Monitor (platformio device monitor) to see debug messages from the ESP32, which can help diagnose issues with WiFi connection or audio streaming.

