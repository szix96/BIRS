# ESP32 Internet Radio

A headless Internet Radio player based on the ESP32-WROOM microcontroller and PCM5102A I2S DAC. This project features a **web interface to change radio station URLs**, physical buttons for control, and an **automatic WiFi Access Point (AP) configuration page** if the device cannot connect to the hardcoded or saved WiFi credentials.

## Features

*   **High-Quality Audio:** Uses I2S digital audio output via PCM5102A.
*   **Web Interface:** 
    *   Edit the radio station list dynamically.
    *   Configure WiFi credentials without recompiling.
*   **WiFi Fallback:** Automatically creates an Access Point (`ESP32-Radio-Config`) if the configured WiFi is unavailable.
*   **Physical Controls:** Buttons to cycle through Next and Previous stations.
*   **Stream Support:** Supports MP3, AAC, and other formats supported by the ESP32-audioI2S library.

## Hardware Required

*   **ESP32 Development Board** (e.g., ESP32-WROOM-32)
*   **PCM5102A DAC Module** (I2S)
*   **PAM8610 Amplifier** (or similar) + Speakers
*   **Push Buttons** (x2)
*   Breadboard and Jumper Wires

## Wiring

### PCM5102A DAC
| PCM5102A Pin | ESP32 Pin | Description |
| :--- | :--- | :--- |
| VCC | 5V / 3.3V | Power |
| GND | GND | Ground |
| BCK | GPIO 26 | Bit Clock |
| DIN | GPIO 22 | Data In |
| LCK | GPIO 25 | Word Select / LR Clock |
| SCK | GND | System Clock (Force internal PLL) |
| FMT | GND | I2S Format (Low) |
| XMT | 3.3V | Mute (High to unmute) |

### Controls
| Component | ESP32 Pin | Connection |
| :--- | :--- | :--- |
| **Next Button** | GPIO 4 | Connect between Pin and GND |
| **Prev Button** | GPIO 13 | Connect between Pin and GND |

> **Note:** Buttons use internal pull-up resistors, so they should pull the pin LOW when pressed.

## Installation

1.  **Clone the repository:**
    ```bash
    git clone https://github.com/yourusername/esp32-internet-radio.git
    ```
2.  **Open in PlatformIO:**
    Open the project folder in VS Code with the PlatformIO extension installed.
3.  **Configure:**
    Check `src/main.cpp` or `platformio.ini` if you need to change pin definitions.
4.  **Upload:**
    Connect your ESP32 and click the **Upload** button in PlatformIO.

## Usage

### Initial Setup (WiFi)
1.  Power on the ESP32.
2.  If it cannot connect to the hardcoded WiFi, it will start an Access Point named **`ESP32-Radio-Config`**.
3.  Connect to this AP with your phone or laptop.
4.  A captive portal or web page should open (navigate to `http://192.168.4.1` if it doesn't).
5.  Enter your home WiFi SSID and Password and click **Save & Reboot**.

### Web Interface
Once connected to your home WiFi:
1.  Find the IP address of the radio (check your router or the Serial Monitor).
2.  Open that IP in a web browser.
3.  You will see the list of current stations.
4.  You can update the **Name** and **URL** of stations and click **Save Changes**.
    *   *Note: Station changes are currently stored in RAM and will reset on reboot.*

### Physical Controls
*   **Next Button (GPIO 4):** Switches to the next station in the list.
*   **Prev Button (GPIO 13):** Switches to the previous station.

## Troubleshooting

*   **No Sound:** Ensure the PCM5102A `SCK` pin is connected to Ground and `XMT` is connected to 3.3V.
*   **Stuttering:** Check your WiFi signal strength.
*   **"VORBIS works only with PSRAM":** Standard ESP32-WROOM boards cannot play OGG/Vorbis streams due to memory limitations. Use MP3 or AAC streams instead.
