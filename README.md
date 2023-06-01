# Automated Trashcan with Weight Sensing, Proximity Detection, and Notification System

![Project Image](project_image.jpg) <!-- Add an image of your project -->

> Short project description and motivation.

## Table of Contents

- [Automated Trashcan with Weight Sensing, Proximity Detection, and Notification System](#automated-trashcan-with-weight-sensing-proximity-detection-and-notification-system)
  - [Table of Contents](#table-of-contents)
  - [Features](#features)
  - [Demo](#demo)
  - [Hardware Components](#hardware-components)
  - [Software Libraries](#software-libraries)
  - [Installation](#installation)
  - [Usage](#usage)
  - [Configuration](#configuration)
  - [Contributing](#contributing)
  - [License](#license)

## Features

- Weight sensing to determine trashcan's fill level.
- Proximity detection to automatically open the trashcan when a person is nearby.
- Real-time notifications for trashcan status and maintenance alerts.
- Configuration portal for custom parameter setup.
- Data storage using JSON file on SPIFFS filesystem.

## Demo

![Demo GIF](demo.gif) <!-- Add a demo GIF or video of your project in action -->

## Hardware Components

- ESP32 development board
- Load cell or weight sensor
- Ultrasonic Proximity sensor
- Servo Motor
- Trashcan
- RGB LED's 

## Software Libraries

- ESP32Servo
- NewPing
- HX711_ADC
- EEPROM
- WiFi
- FS
- SPIFFS
- WiFiManager
- ArduinoJson
- WiFiClientSecure
- UniversalTelegramBot

## Installation

1. Clone this repository.
2. Set up the Arduino IDE or platform of your choice.
3. Install the required libraries listed in the "Software Libraries" section.
4. Connect the hardware components according to the provided circuit diagram.
5. Upload the code to your ESP32 board.

## Usage

1. Power on the automated trashcan.
2. Connect your device to the same WiFi network as the trashcan.
3. Access the trashcan's web portal to configure Wifi, custom parameters and thresholds.
4. The trashcan will automatically connect to the WiFi network using WiFi Manager after first setup.
5. The trashcan will continuously monitor the weight and proximity to detect when to open.
6. Real-time notifications will be sent to the designated user for maintenance or alerts.
7. Empty the trashcan when necessary.
   
## Configuration

The WiFi Configuration Device allows users to configure WiFi settings and custom parameters through a web-based configuration portal. Follow the steps below to configure the device:

1. Power on the device.
2. The device will attempt to connect to the saved WiFi network automatically. If no saved configuration is found or the connection fails, the device will enter configuration mode.
3. If the device enters configuration mode, it will create a WiFi network named "WiFiConfigDevice". Connect to this network using a computer or mobile device.
4. Open a web browser and navigate to http://192.168.4.1 (or the IP address assigned to the device if changed).
5. The web-based configuration portal will appear. Here, you can perform the following configurations:

   - Select the desired WiFi network from the available networks and enter the password (if required).
   - Enter any custom parameters such as device location or identification number.

6. Click the "Save" button to save the configuration.
7. The device will reboot and attempt to connect to the configured WiFi network using the provided credentials.
8. If the connection is successful, the device will operate normally.
9. If the connection fails, the device will re-enter configuration mode, allowing you to repeat the configuration process.

Please note that the provided code serves as a basic example and can be customized to fit your specific requirements. You can add additional custom parameters to collect specific information or settings from the user. The appearance of the configuration portal can be customized by modifying the HTML and CSS files provided in the code. Additionally, you may want to enhance the error handling and recovery mechanisms to provide better user feedback in case of connection failures or other issues.

## Contributing

1. Fork the repository.
2. Create a new branch for your feature: `git checkout -b feature-name`.
3. Commit your changes: `git commit -m 'Add some feature'`.
4. Push to the branch: `git push origin feature-name`.
5. Submit a pull request.

## License

MIT License © 2023 [BSCPE 3B - Group 1] - BISU - Main Campus - Batch 2020-2024
