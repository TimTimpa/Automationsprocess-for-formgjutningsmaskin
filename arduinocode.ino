#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Encoder.h>
#include <DHT.h>
#include <AccelStepper.h>

// Define screen dimensions and reset pin for the OLED display
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1

// Initialize the OLED display
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

// Initialize the encoder with pins 2 and 3
Encoder myEnc(2, 3);
int lastPos = -1;  // Last position of the encoder
int currentPos = 0;  // Current position of the encoder
int buttonPin = 4;  // Pin for the button
int selectedOption = -1;  // Selected menu option

// Define menu options
const char* options[] = {"Starta uppvärmning", "Starta laddning", "Status", "Felmeddelande"};
const int numOptions = sizeof(options) / sizeof(options[0]);  // Number of menu options

// Define screen states
enum ScreenState { MAIN_MENU, TEMP_MENU, LOAD_MENU };
ScreenState screenState = MAIN_MENU;  // Initial screen state

// Initialize the DHT sensor with pin 5 and type DHT11
DHT dht(5, DHT11);
float currentTemp = 0;  // Current temperature
int selectedTemp = 20;  // Selected temperature

// Initialize the stepper motor with driver pins 8 and 9
AccelStepper stepper(AccelStepper::DRIVER, 8, 9);
int microSwitchPin = 10;  // Pin for the micro switch
int currentHolder = 0;  // Current holder position

void setup() {
  pinMode(buttonPin, INPUT_PULLUP);  // Set button pin as input with pull-up resistor
  pinMode(microSwitchPin, INPUT_PULLUP);  // Set micro switch pin as input with pull-up resistor
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // Initialize the OLED display
  display.clearDisplay();  // Clear the display
  display.display();  // Update the display
  myEnc.write(0);  // Reset the encoder position
  dht.begin();  // Initialize the DHT sensor
  stepper.setMaxSpeed(1000);  // Set the maximum speed for the stepper motor
  stepper.setAcceleration(500);  // Set the acceleration for the stepper motor
}

void loop() {
  currentPos = myEnc.read() / 4;  // Read the encoder position and adjust for resolution if needed

  if (screenState == MAIN_MENU) {
    if (currentPos != lastPos) {  // If the encoder position has changed
      lastPos = currentPos;  // Update the last position
      displayMenu(currentPos % numOptions);  // Display the menu with the new highlighted option
    }

    if (digitalRead(buttonPin) == LOW) {  // If the button is pressed
      selectedOption = currentPos % numOptions;  // Update the selected option
      if (selectedOption == 0) {
        screenState = TEMP_MENU;  // Switch to temperature menu
        myEnc.write(selectedTemp * 4);  // Set the encoder position to the selected temperature
      } else if (selectedOption == 1) {
        screenState = LOAD_MENU;  // Switch to load menu
        myEnc.write(currentHolder * 4);  // Set the encoder position to the current holder
      } else {
        displayMenu(selectedOption);  // Display the menu with the selected option highlighted
      }
      delay(500);  // Debounce delay to prevent multiple selections
    }
  } else if (screenState == TEMP_MENU) {
    currentTemp = dht.readTemperature();  // Read the current temperature from the DHT sensor
    selectedTemp = myEnc.read() / 4;  // Read the selected temperature from the encoder

    displayTempMenu(currentTemp, selectedTemp);  // Display the temperature menu

    if (digitalRead(buttonPin) == LOW) {  // If the button is pressed
      // Save the selected temperature
      screenState = MAIN_MENU;  // Switch back to the main menu
      delay(500);  // Debounce delay to prevent multiple selections
    }
  } else if (screenState == LOAD_MENU) {
    currentHolder = myEnc.read() / 4;  // Read the current holder position from the encoder

    displayLoadMenu(currentHolder);  // Display the load menu

    if (digitalRead(buttonPin) == LOW) {  // If the button is pressed
      // Rotate to the selected holder
      if (currentHolder % 2 == 0) {
        rotateLeft();  // Rotate the stepper motor to the left
      } else {
        rotateRight();  // Rotate the stepper motor to the right
      }
      screenState = MAIN_MENU;  // Switch back to the main menu
      delay(500);  // Debounce delay to prevent multiple selections
    }
  }
}

void displayMenu(int highlight) {
  display.clearDisplay();  // Clear the display
  display.setTextSize(1);  // Set the text size
  display.setTextColor(SSD1306_WHITE);  // Set the text color to white

  for (int i = 0; i < numOptions; i++) {  // Loop through each menu option
    if (i == highlight) {  // If the current option is the highlighted one
      display.fillRect(0, i * 16, SCREEN_WIDTH, 16, SSD1306_WHITE);  // Draw a filled rectangle to highlight the option
      display.setTextColor(SSD1306_BLACK);  // Set the text color to black for the highlighted option
    } else {
      display.setTextColor(SSD1306_WHITE);  // Set the text color to white for non-highlighted options
    }
    display.setCursor(0, i * 16);  // Set the cursor position for each option
    display.println(options[i]);  // Print the option
  }

  display.display();  // Update the display with the new content
}

void displayTempMenu(float currentTemp, int selectedTemp) {
  display.clearDisplay();  // Clear the display
  display.setTextSize(1);  // Set the text size
  display.setTextColor(SSD1306_WHITE);  // Set the text color to white

  display.setCursor(0, 0);  // Set the cursor position
  display.print("Current Temp: ");  // Print the current temperature label
  display.println(currentTemp);  // Print the current temperature

  display.setCursor(0, 16);  // Set the cursor position
  display.print("Set Temp: ");  // Print the set temperature label
  display.println(selectedTemp);  // Print the selected temperature

  display.display();  // Update the display with the new content
}

void displayLoadMenu(int currentHolder) {
  display.clearDisplay();  // Clear the display
  display.setTextSize(1);  // Set the text size
  display.setTextColor(SSD1306_WHITE);  // Set the text color to white

  display.setCursor(0, 0);  // Set the cursor position
  display.print("Select Holder: ");  // Print the select holder label
  display.println(currentHolder);  // Print the current holder

  display.setCursor(0, 16);  // Set the cursor position
  display.print("Rotate Left");  // Print the rotate left option

  display.setCursor(0, 32);  // Set the cursor position
  display.print("Rotate Right");  // Print the rotate right option

  display.display();  // Update the display with the new content
}

void rotateLeft() {
  stepper.moveTo(stepper.currentPosition() - 60);  // Move the stepper motor 60 degrees to the left
  while (stepper.distanceToGo() != 0) {  // While the stepper motor has not reached the target position
    stepper.run();  // Run the stepper motor
    if (digitalRead(microSwitchPin) == LOW) {  // If the micro switch is activated
      stepper.stop();  // Stop the stepper motor
      break;  // Exit the loop
    }
  }
}

void rotateRight() {
  stepper.moveTo(stepper.currentPosition() + 60);  // Move the stepper motor 60 degrees to the right
  while (stepper.distanceToGo() != 0) {  // While the stepper motor has not reached the target position
    stepper.run();  // Run the stepper motor
    if (digitalRead(microSwitchPin) == LOW) {  // If the micro switch is activated
      stepper.stop();  // Stop the stepper motor
      break;  // Exit the loop
    }
  }
}