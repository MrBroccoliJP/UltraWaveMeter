
/**
 * UltraWaveMeter - Ultrasonic measurement system designed for precise distance and wave detection
 * Copyright (c) 2022-2025 João Fernandes
 * 
 * This work is licensed under the Creative Commons Attribution-NonCommercial 
 * 4.0 International License. To view a copy of this license, visit:
 * http://creativecommons.org/licenses/by-nc/4.0/
 * 
 * @author João Fernandes (Original author)
 * @version 6.0
 * @date 2022 (Original)
 * @date 2025 (Translation)
 * 
 * This program uses an ultrasonic sensor to measure distances over time,
 * which can be used for wave measurement applications. It provides different
 * measurement modes and outputs data via serial communication.
 */

#include <NewPing.h>

// Hardware configuration
#define TRIGGER_PIN 13     // Arduino pin connected to the ultrasonic sensor's trigger pin
#define ECHO_PIN 15        // Arduino pin connected to the ultrasonic sensor's echo pin
#define MAX_DISTANCE 170   // Maximum measurable distance in centimeters
#define LED_PIN 16         // Status LED pin
#define BUTTON_1_PIN 0     // First control button (INPUT_PULLUP) BTN1
#define BUTTON_2_PIN 10     // Second control button (INPUT_PULLUP) BTN2

// Initialize ultrasonic sensor with NewPing library
NewPing sonar(TRIGGER_PIN, ECHO_PIN, MAX_DISTANCE);

// Environment and calibration variables
float temp = 25;                        // Default ambient temperature in Celsius
float soundSpeedFactor;                 // Factor to calculate speed of sound based on temperature
float baselineAverage = 0;              // Average baseline measurement

// Program control variables
short int currentMode = 0;              // Operating mode (0: standby, 1: standard measurement, 2: custom interval)
bool buttonState;                       // Current state of buttons
unsigned long lastButtonPress = 0;      // Timestamp of last button press for debouncing
short int buttonDebounceDelay = 250;    // Delay for button debounce in milliseconds
short int readingCount = 0;             // Counter for measurement sessions
unsigned long measurementStartTime = 0; // Timestamp when measurement session started
short int sampleInterval = 40;          // Default time between samples in milliseconds (minimum 40ms)

// Serial communication variables
String measurementData;                 // Holds formatted measurement data for output
String serialInput;                     // Holds input from serial port

// User instructions and program information
String instructions = "\nUltrasonic Wave Measurement System\n"
                      "To start measurements, press Button 1, Button 2, or enter 'S' in serial console.\n"
                      "To stop, press Button 1, Button 2, or enter 'S' in serial console.\n"
                      "To use a custom sample interval (default = 40ms), enter 'N'.\n"
                      "To RESET the system, enter 'R'.";
String codeVersion = "V6.0";

/**
 * Initial setup function - runs once at program start
 */
void setup() {
  // Initialize serial communication
  Serial.begin(9600);
  
  // Configure pin modes
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_1_PIN, INPUT_PULLUP);
  pinMode(BUTTON_2_PIN, INPUT_PULLUP);

  // LED startup sequence for visual confirmation
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  delay(500);
  digitalWrite(LED_PIN, HIGH);
  delay(100);
  digitalWrite(LED_PIN, LOW);
  delay(1000);
  
  // Print startup message
  Serial.print("\nPROGRAM STARTING\n");
  Serial.print("\nEnter ambient temperature: ");

  // Wait for temperature input while blinking the LED
  while (!Serial || !Serial.available()) {
    // Blink LED while waiting
    if (millis() % 250 == 0) {
      digitalWrite(LED_PIN, HIGH);
    }
    else if (millis() % 100 == 0) {
      digitalWrite(LED_PIN, LOW);
    }
    
    // Periodically repeat the prompt
    if (millis() % 3000 == 0) {
      Serial.print("\nEnter ambient temperature: ");
      Serial.flush();
    }
    yield();  // Allow ESP8266 to handle system tasks
  }

  // Process temperature input
  serialInput = "";
  while (serialInput.toInt() == 0) {
    serialInput = Serial.readStringUntil('\n');
    Serial.println(serialInput);
    Serial.println("Temperature saved\n===== Program Initialized =====\n");
    Serial.flush();
    yield();
  }
  
  // Set temperature and calculate sound speed factor
  temp = serialInput.toInt();
  soundSpeedFactor = sqrt(1 + temp / 273.15) / 60.368;  // Speed of sound calculation based on temperature

  // Take initial baseline measurements
  for (int i = 2; i > 0; i--) {
    float baselineReading = ((float)sonar.ping_median(5) * soundSpeedFactor);
    baselineAverage += baselineReading;
    measurementData = "Baseline distance: " + (String)baselineReading + " cm ";
    measurementData.replace(".", ",");  // Format for CSV compatibility
    Serial.println(measurementData);
    delay(300);
  }
  
  // Calculate and display average baseline
  baselineAverage = baselineAverage / 2;
  measurementData = "\nAverage: " + (String)baselineAverage + " cm ";
  measurementData.replace(".", ",");
  Serial.println(measurementData);
  
  // Output program information and instructions
  Serial.print("\nVersion: ");
  Serial.println(codeVersion);
  Serial.println(instructions);
  Serial.println("****************************\n");
  Serial.println("Waiting to start -> Press Button 1 or Button 2 or enter 'S' OR 'N'");
  
  // Set initial mode to standby
  currentMode = 0;
}

/**
 * Main program loop
 */
void loop() {
  // MODE 0: Standby - waiting for user input to start measurement
  if (currentMode == 0) {
    serialInput = "";
    while (currentMode == 0) {
      // Status LED blink pattern for standby mode
      if (millis() % 500 == 0) {
        digitalWrite(LED_PIN, HIGH);
      }
      else if (millis() % 250 == 0) {
        digitalWrite(LED_PIN, LOW);
      }

      // Check for serial commands
      if (Serial.available() > 0) {
        serialInput = Serial.readStringUntil('\n');
        Serial.println(serialInput);

        // Process commands
        if (serialInput == "S") {
          currentMode = 1;  // Start standard measurement
          Serial.flush();
        }
        else if (serialInput == "R") {
          ESP.reset();      // Reset the system
        }
        else if (serialInput == "N") {
          currentMode = 2;  // Start custom interval measurement
          Serial.flush();
        }
        else {
          Serial.println("Invalid mode -> Use 'S' OR 'N'");
          serialInput = "";
          Serial.flush();
        }
      }

      // Check Button 1 state
      buttonState = digitalRead(BUTTON_1_PIN);
      if (buttonState == LOW) {
        if (millis() - lastButtonPress > buttonDebounceDelay) {
          currentMode = 1;  // Start standard measurement
          buttonState = HIGH;
        }
        lastButtonPress = millis();
      }
      
      // Check Button 2 state
      buttonState = digitalRead(BUTTON_2_PIN);
      if (buttonState == LOW) {
        if (millis() - lastButtonPress > buttonDebounceDelay) {
          currentMode = 1;  // Start standard measurement
          buttonState = HIGH;
        }
        lastButtonPress = millis();
      }
      
      yield();  // Allow ESP8266 to handle system tasks
    }
  }
  
  // MODE 2: Custom interval measurement
  else if (currentMode == 2) {
    sampleInterval = 0;
    baselineAverage = 0;

    Serial.print("\nMODE 2\n");
    Serial.print("\nEnter desired sample interval in ms (minimum 40ms): ");

    // Get valid interval input
    while (sampleInterval < 40) {
      if (Serial.available() > 0) {
        serialInput = Serial.readStringUntil('\n');
        
        // Process input
        if (serialInput.toInt() > 40 && serialInput.toInt() < 5000) {
          sampleInterval = serialInput.toInt();
          Serial.flush();
        }
        else if (serialInput == "R") {
          ESP.reset();
        }
        else if (serialInput == "S") {
          currentMode = 0;
          Serial.flush();
          break;
        }
        else {
          Serial.println("\nInvalid value ('R' for RESET & 'S' to return to main menu)\nEnter a value >40ms");
          Serial.flush();
        }
      }
      yield();
    }

    // Start new measurement session
    readingCount++;
    if (currentMode != 0) {
      Serial.print("\n********** READING NUMBER: ");
      Serial.print(readingCount);
      Serial.println(" **********");

      // Take baseline measurements
      for (int i = 5; i > 0; i--) {
        float baselineReading = ((float)sonar.ping_median(5) * soundSpeedFactor);
        baselineAverage += baselineReading;
        measurementData = "Baseline distance: " + (String)baselineReading + " cm ";
        measurementData.replace(".", ",");
        Serial.println(measurementData);
        delay(300);
      }
      
      // Calculate and display average baseline
      baselineAverage = baselineAverage / 5;
      measurementData = "\nBaseline distance: Average = " + (String)baselineAverage + " cm ";
      measurementData.replace(".", ",");
      Serial.println(measurementData);
      Serial.print("\nDISTANCE (cm) || TIME (ms)\n\n");

      // Record start time for relative time measurement
      measurementStartTime = millis();
    }

    // Continuous measurement loop
    while (currentMode != 0) {
      digitalWrite(LED_PIN, HIGH);  // LED on during measurement

      // Check for serial commands
      if (Serial.available() > 0) {
        serialInput = Serial.readStringUntil('\n');
        if (serialInput == "S") {
          currentMode = 0;
          Serial.flush();
        }
        if (serialInput == "R") {
          ESP.reset();
        }
      }

      // Check Button 1 state
      buttonState = digitalRead(BUTTON_1_PIN);
      if (buttonState == LOW) {
        if (millis() - lastButtonPress > buttonDebounceDelay) {
          currentMode = 0;
          buttonState = HIGH;
        }
        lastButtonPress = millis();
      }
      
      // Check Button 2 state
      buttonState = digitalRead(BUTTON_2_PIN);
      if (buttonState == LOW) {
        if (millis() - lastButtonPress > buttonDebounceDelay) {
          currentMode = 0;
          buttonState = HIGH;
        }
        lastButtonPress = millis();
      }

      // Take measurement at specified interval
      if (millis() % sampleInterval == 0) {
        measurementData = String(((float)sonar.ping_median(1) * soundSpeedFactor), 2) + " " + 
                          (String)(millis() - measurementStartTime) + "\n";
        measurementData.replace(".", ",");  // Format for CSV compatibility
        Serial.print(measurementData);
        Serial.flush();
      }

      yield();  // Allow ESP8266 to handle system tasks
    }
  }
  
  // MODE 1: Standard measurement with default 40ms interval
  else {
    baselineAverage = 0;
    readingCount++;
    
    Serial.print("\n********** READING NUMBER: ");
    Serial.print(readingCount);
    Serial.println(" **********");

    // Take baseline measurements
    for (int i = 5; i > 0; i--) {
      float baselineReading = ((float)sonar.ping_median(5) * soundSpeedFactor);
      baselineAverage += baselineReading;
      measurementData = "Baseline distance: " + (String)baselineReading + " cm ";
      measurementData.replace(".", ",");
      Serial.println(measurementData);
      delay(300);
    }
    
    // Calculate and display average baseline
    baselineAverage = baselineAverage / 5;
    measurementData = "\nBaseline distance: Average = " + (String)baselineAverage + " cm ";
    measurementData.replace(".", ",");
    Serial.println(measurementData);
    Serial.print("\nSample interval: 40ms \n\n DISTANCE (cm) || TIME (ms)\n\n");

    // Record start time for relative time measurement
    measurementStartTime = millis();

    // Continuous measurement loop
    while (currentMode != 0) {
      digitalWrite(LED_PIN, HIGH);  // LED on during measurement

      // Check for serial commands
      if (Serial.available() > 0) {
        serialInput = Serial.readStringUntil('\n');

        if (serialInput == "S") {
          currentMode = 0;
          Serial.flush();
        }
        else if (serialInput == "R") {
          ESP.reset();
        }
        else {
          Serial.flush();
        }
      }

      // Check Button 1 state
      buttonState = digitalRead(BUTTON_1_PIN);
      if (buttonState == LOW) {
        if (millis() - lastButtonPress > buttonDebounceDelay) {
          currentMode = 0;
          buttonState = HIGH;
        }
        lastButtonPress = millis();
      }
      
      // Check Button 2 state
      buttonState = digitalRead(BUTTON_2_PIN);
      if (buttonState == LOW) {
        if (millis() - lastButtonPress > buttonDebounceDelay) {
          currentMode = 0;
          buttonState = HIGH;
        }
        lastButtonPress = millis();
      }

      // Take measurement at default interval (40ms)
      if (millis() % 40 == 0) {
        measurementData = String(((float)sonar.ping_median(1) * soundSpeedFactor), 2) + " " + 
                          (String)(millis() - measurementStartTime) + "\n";
        measurementData.replace(".", ",");  // Format for CSV compatibility
        Serial.print(measurementData);
        Serial.flush();
      }

      yield();  // Allow ESP8266 to handle system tasks
    }
  }
}