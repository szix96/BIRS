# Next Steps

**UPDATE:** The file `src/main.cpp` was just modified to fix a compilation error (`identifier "I2S_DOUT" is undefined`). The I2S audio pins are now defined directly in the C++ code to ensure the project compiles correctly. You can now proceed with the steps below.

---

Your Internet Radio project has been set up. Here is what you need to do to get it running:

1.  **Update WiFi Credentials:**
    *   Open the `src/main.cpp` file.
    *   On lines 13 and 14, replace `"YOUR_WIFI_SSID"` and `"YOUR_WIFI_PASSWORD"` with your actual WiFi network name and password.

2.  **Verify Hardware Pins:**
    *   Open the `platformio.ini` file.
    *   Review the pin definitions under the `build_flags` section. These pins must match how you have wired your components to the ESP32.
        ```ini
        ; --- TFT_eSPI Display ---
        -D TFT_MISO=19
        -D TFT_MOSI=23
        -D TFT_SCLK=18
        -D TFT_CS=15
        -D TFT_DC=2
        -D TFT_RST=4

        ; --- XPT2046 Touchscreen ---
        -D TOUCH_CS=21

        ; --- I2S Audio (PCM5102A) ---
        -D I2S_BCLK=26
        -D I2S_LRC=25
        -D I2S_DOUT=22
        ```
    *   The boot issue you mentioned (`rst:0x1 (POWERON_RESET),boot:0x13 (SPI_FAST_FLASH_BOOT)`) can often be caused by incorrect pin assignments, especially if certain pins required for booting are used for peripherals. Please double-check your wiring against the ESP32's datasheet and the pin definitions in this file.

3.  **Build and Upload:**
    *   Connect your ESP32 board to your computer.
    *   Use the PlatformIO CLI to build and upload the project. You can do this with the following command in your terminal:
        ```bash
        platformio run --target upload
        ```
    *   Alternatively, you can use the "Upload" button within the Visual Studio Code PlatformIO extension.

4.  **Troubleshooting:**
    *   If the screen is white or the touch is not working, you may need to adjust the display driver (`ILI9341_DRIVER`) or the touch calibration values in `src/main.cpp`.
    *   If there is no sound, double-check the I2S pin connections to your PCM5102A board.
    *   Use the Serial Monitor (`platformio device monitor`) to see debug messages from the ESP32, which can help diagnose issues with WiFi connection or audio streaming.
