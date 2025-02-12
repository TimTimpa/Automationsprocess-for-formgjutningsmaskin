# Automationsprocess-for-formgjutningsmaskin

This project is an automation process for a molding machine using an Arduino. It includes an OLED display, a rotary encoder, a DHT11 temperature sensor, a stepper motor, and an Ethernet server for web-based status monitoring.

## Features

- **OLED Display**: Displays menu options and status information.
- **Rotary Encoder**: Used for navigating the menu and selecting options.
- **DHT11 Temperature Sensor**: Measures the current temperature.
- **Stepper Motor**: Controls the position of a holder.
- **PID Controller**: Manages the heating process.
- **Ethernet Server**: Provides a web interface to monitor the status.

## Components

- Arduino
- OLED Display (128x64)
- Rotary Encoder
- DHT11 Temperature Sensor
- Stepper Motor
- Relay
- Ethernet Shield

## Menu Options

1. **Starta uppvärmning**: Start the heating process.
2. **Starta laddning**: Start the loading process.
3. **Status**: Display the current status.
4. **Felmeddelande**: Display error messages.

## Setup

1. Connect the components as per the pin configuration in the code.
2. Upload the `arduinocode.ino` file to your Arduino.
3. Open the serial monitor to view debug information.

## Usage

- Use the rotary encoder to navigate through the menu options.
- Press the button to select an option.
- The OLED display will show the current status and menu options.
- The web interface can be accessed via the IP address `192.168.1.177` to view the current temperature, set temperature, heating status, and IP address.

## License

This project uses code from various sources. Please refer to the [Code Citations](Code%20Citations.md) file for more information.