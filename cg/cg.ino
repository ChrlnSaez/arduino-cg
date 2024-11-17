#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);  // Adjust I2C address if needed

// Define pins for each position button
const int pilot1ButtonPin = 22;
const int pilot2ButtonPin = 23;
const int passenger1ButtonPin = 24;
const int passenger2ButtonPin = 25;
const int baggage1ButtonPin = 26;
const int baggage2ButtonPin = 27;
const int fuelButtonPin = 28;

const int frontWheelWeightSwitch = 2;
const int leftWheelWeightSwitch = 3;
const int rightWheelWeightSwitch = 4;
const int frontWheelDistanceSwitch = 8;
const int leftWheelDistanceSwitch = 9;
const int rightWheelDistanceSwitch = 10;

const int distancePointSwitch = 11;
const int forwardPointSwitch = 12;
const int leverPointSwitch = 13;
const int weightPointSwitch = A0;

// Define pins for Variable/Fixed switch, datum switches, and Save button
const int fixedSwitchPin = 35;
const int datum1Pin = 32;
const int datum2Pin = 33;
const int datum3Pin = 34;
const int saveButtonPin = 48;       // Save button for weight adjustments
const int calculateButtonPin = 49;  // Button for calculating center of gravity
const int resetButtonPin = 50;      // Button for resetting to defaults
const int buzzerPin = 6;

// Define pins for weight adjustment buttons
const int addButtons[] = { 42, 43, 44, 45, 46, 47, 7, A1 };       // Add 1, 5, 10, 20, 50, 100, 0.1, 0.01
const int subtractButtons[] = { 36, 37, 38, 39, 40, 41, 5, A2 };  // Subtract 1, 5, 10, 20, 50, 100, 0.1, 0.01

// Variables for switch states, weight storage, and error handling
bool isFixed;
bool datum1State, datum2State, datum3State;
bool isError = false;
bool unsavedChanges = false;  // Track if there are unsaved weight changes

bool frontWheelWeightState, leftWheelWeightState, rightWheelWeightState;
bool frontWheelDistanceState, leftWheelDistanceState, rightWheelDistanceState;

bool distancePointState, forwardPointState, leverPointState, weightPointState;

// Timing variables
unsigned long previousMillis = 0;
const long interval = 500;  // Update interval in milliseconds

// Weight variables for each section with initial values
float pilot2Weight = 170.0, savedPilot2Weight = 0.0;
float pilot1Weight = 170.0, savedPilot1Weight = 0.0;
float passenger1Weight = 170.0, savedPassenger1Weight = 0.0;
float passenger2Weight = 170.0, savedPassenger2Weight = 0.0;
float baggage1Weight = 30.0, savedBaggage1Weight = 0.0;
float baggage2Weight = 30.0, savedBaggage2Weight = 0.0;
float fuelWeight = 6.0, savedFuelWeight = 0.0;

// DITO PALIT VALUES
// #####################################################################
float frontWheelWeight = 20.0, savedFrontWheelWeight = 20.0;
float leftWheelWeight = 32.0, savedLeftWheelWeight = 32.0;
float rightWheelWeight = 30.0, savedRightWheelWeight = 30.0;

float frontWheelDistance = -42.0, savedFrontWheelDistance = -42.0;
float leftWheelDistance = 32.0, savedLeftWheelDistance = 32.0;
float rightWheelDistance = 32.0, savedRightWheelDistance = 32.0;

float distancePoint = 32.0, savedDistancePoint = 32.0;
float forwardPoint = 24.0, savedForwardPoint = 24.0;
float leverPoint = 12.0, savedLeverPoint = 12.0;
float weightPoint = 8.0, savedWeightPoint = 8.0;
// #####################################################################

// Timing variables for jingle
unsigned long jingleMillis = 0;
int jingleIndex = 0;

const int jingleNotes[] = { 330, 392, 330, 262 };
const int jingleDurations[] = { 150, 300, 150, 500 };
const int jingleLength = sizeof(jingleNotes) / sizeof(jingleNotes[0]);

// Define distances from datum (in inches) for each section
const float aircraftDistance = 41.0;
const float pilotDistance = 37.0;
const float passengerDistance = 73.0;
const float baggageDistance = 123.0;
const float fuelDistance = 48.2;

// Base empty weight of the aircraft
const float baseEmptyWeight = 1680.0;

// Define acceptable Center of Gravity range
const float minCG = 35.0;
const float maxCG = 47.3;

bool resetTriggered = false;              // Flag for reset button
bool calculateTriggered = false;          // Flag for calculate button
bool displayUpdated = false;              // Flag to track if LCD has been updated
unsigned long resetDebounceTime = 0;      // Debounce timer for reset button
unsigned long calculateDebounceTime = 0;  // Debounce timer for calculate button
const unsigned long debounceDelay = 200;  // Debounce delay for button presses

void setup() {
  Serial.begin(9600);

  // Initialize buttons as input with pullup resistors
  pinMode(pilot1ButtonPin, INPUT);
  pinMode(pilot2ButtonPin, INPUT);
  pinMode(passenger1ButtonPin, INPUT);
  pinMode(passenger2ButtonPin, INPUT);
  pinMode(baggage1ButtonPin, INPUT);
  pinMode(baggage2ButtonPin, INPUT);
  pinMode(fuelButtonPin, INPUT);

  pinMode(fixedSwitchPin, INPUT_PULLUP);
  pinMode(datum1Pin, INPUT_PULLUP);
  pinMode(datum2Pin, INPUT_PULLUP);
  pinMode(datum3Pin, INPUT_PULLUP);
  pinMode(saveButtonPin, INPUT_PULLUP);
  pinMode(calculateButtonPin, INPUT_PULLUP);
  pinMode(resetButtonPin, INPUT_PULLUP);
  pinMode(buzzerPin, OUTPUT);

  pinMode(frontWheelWeightSwitch, INPUT_PULLUP);
  pinMode(leftWheelWeightSwitch, INPUT_PULLUP);
  pinMode(rightWheelWeightSwitch, INPUT_PULLUP);
  pinMode(frontWheelDistanceSwitch, INPUT_PULLUP);
  pinMode(leftWheelDistanceSwitch, INPUT_PULLUP);
  pinMode(rightWheelDistanceSwitch, INPUT_PULLUP);

  pinMode(distancePointSwitch, INPUT_PULLUP);
  pinMode(forwardPointSwitch, INPUT_PULLUP);
  pinMode(leverPointSwitch, INPUT_PULLUP);
  pinMode(weightPointSwitch, INPUT_PULLUP);

  lcd.init();
  lcd.backlight();
  lcd.clear();
  noTone(buzzerPin);

  // Initialize add and subtract buttons
  for (int i = 0; i < 8; i++) {
    pinMode(addButtons[i], INPUT_PULLUP);
    pinMode(subtractButtons[i], INPUT_PULLUP);
  }
}

void controlBuzzer() {
  if (isError) {
    playErrorJingleLoop();
  } else {
    noTone(buzzerPin);
  }
}

// Function to play the jingle for error conditions
void playErrorJingleLoop() {
  static unsigned long jingleStartTime = 0;
  unsigned long currentMillis = millis();
  ;
  if (currentMillis - jingleStartTime >= jingleDurations[jingleIndex]) {
    jingleStartTime = currentMillis;  // Reset the jingle start time
    tone(buzzerPin, jingleNotes[jingleIndex], jingleDurations[jingleIndex]);
    jingleIndex = (jingleIndex + 1) % jingleLength;  // Move to the next note
  }
}

const char* currentSection = nullptr;  // Track the active section being adjusted
const char* latestButton = nullptr;

// Modification flags to track if a weight has been changed since the last save
bool pilot1Modified = false;
bool pilot2Modified = false;
bool passenger1Modified = false;
bool passenger2Modified = false;
bool baggage1Modified = false;
bool baggage2Modified = false;
bool fuelModified = false;

void adjustWheelWeight(float& weight, const char* wheelName, bool isDistance) {
  bool weightChanged = false;

  lcd.clear();
  lcd.print(wheelName);
  lcd.setCursor(0, 1);
  if (isDistance) {
    lcd.print("Distance: ");
  } else {
    lcd.print("Weight: ");
  }
  lcd.print(weight);
  if (isDistance) {
    lcd.print("in");
  } else {
    lcd.print(" lbs");
  }

  for (int i = 0; i < 8; i++) {
    if (!digitalRead(addButtons[i])) {
      weight += (i == 0) ? 1 : (i == 1) ? 5
                             : (i == 2) ? 10
                             : (i == 3) ? 20
                             : (i == 4) ? 50
                             : (i == 5) ? 100
                             : (i == 6) ? 0.1
                                        : 0.01;
      weightChanged = true;
      delay(200);  // Short delay to prevent rapid increment
    }
    if (!digitalRead(subtractButtons[i])) {
      weight -= (i == 0) ? 1 : (i == 1) ? 5
                             : (i == 2) ? 10
                             : (i == 3) ? 20
                             : (i == 4) ? 50
                             : (i == 5) ? 100
                             : (i == 6) ? 0.1
                                        : 0.01;
      if (weight < 0 && !isDistance) weight = 0;
      weightChanged = true;
      delay(200);  // Short delay to prevent rapid decrement
    }

    if (weightChanged) {
      lcd.clear();
      lcd.print(wheelName);
      lcd.setCursor(0, 1);
      if (isDistance) {
        lcd.print("Distance: ");
      } else {
        lcd.print("Weight: ");
      }
      lcd.print(weight);
      if (isDistance) {
        lcd.print("in");
      } else {
        lcd.print(" lbs");
      }


      weightChanged = false;  // Reset the flag for the next iteration
    }
  }
}

void adjustWeight(float& weight, const char* sectionName) {
  bool weightChanged = false;

  // Display the initial weight immediately when the function is called
  lcd.clear();
  lcd.print(sectionName);
  lcd.setCursor(0, 1);
  lcd.print("Weight: ");
  lcd.print(weight);
  lcd.print(" lbs");

  // Loop through add and subtract buttons to detect weight changes
  for (int i = 0; i < 8; i++) {
    if (!digitalRead(addButtons[i])) {
      weight += (i == 0) ? 1 : (i == 1) ? 5
                             : (i == 2) ? 10
                             : (i == 3) ? 20
                             : (i == 4) ? 50
                             : (i == 5) ? 100
                             : (i == 6) ? 0.1
                                        : 0.01;
      weightChanged = true;
      delay(200);  // Short delay to prevent rapid increment
    }
    if (!digitalRead(subtractButtons[i])) {
      weight -= (i == 0) ? 1 : (i == 1) ? 5
                             : (i == 2) ? 10
                             : (i == 3) ? 20
                             : (i == 4) ? 50
                             : (i == 5) ? 100
                             : (i == 6) ? 0.1
                                        : 0.01;
      if (weight < 0) weight = 0;
      weightChanged = true;
      delay(200);  // Short delay to prevent rapid decrement
    }

    // If weight changed, update the display and set the corresponding modification flag
    if (weightChanged) {
      lcd.clear();
      lcd.print(sectionName);
      lcd.setCursor(0, 1);
      lcd.print("Weight: ");
      lcd.print(weight);
      lcd.print(" lbs");

      // Set the appropriate modification flag based on the section name
      if (sectionName == "Pilot1") pilot1Modified = true;
      else if (sectionName == "Pilot2") pilot2Modified = true;
      else if (sectionName == "Passenger1") passenger1Modified = true;
      else if (sectionName == "Passenger2") passenger2Modified = true;
      else if (sectionName == "Baggage1") baggage1Modified = true;
      else if (sectionName == "Baggage2") baggage2Modified = true;
      else if (sectionName == "Fuel") fuelModified = true;

      // Check for overload warning if adjusting Baggage weights
      if (baggage1Weight + baggage2Weight > 120) {
        lcd.setCursor(0, 2);
        lcd.print("Warning: Overloaded");
      }

      weightChanged = false;  // Reset the flag for the next iteration
    }
  }
}

void calculateDatum1CG() {
  float totalMoment = (savedFrontWheelWeight * savedFrontWheelDistance) + (savedLeftWheelWeight * savedLeftWheelDistance) + (savedRightWheelWeight * savedRightWheelDistance);
  float totalWeight = savedFrontWheelWeight + savedLeftWheelWeight + savedRightWheelWeight;

  if (!displayUpdated) {
    lcd.clear();

    float centerOfGravity = totalMoment / totalWeight;

    lcd.print("Center of Gravity: ");
    lcd.setCursor(0, 2);
    lcd.print(centerOfGravity, 1);

    displayUpdated = true;
  }

  controlBuzzer();
}

void calculateDatum2CG() {
  if (!displayUpdated) {
    lcd.clear();
    float centerOfGravity = savedDistancePoint - ((savedForwardPoint * savedLeverPoint) / savedWeightPoint);

    centerOfGravity = abs(centerOfGravity);

    lcd.print("Center of Gravity: ");
    lcd.setCursor(0, 2);
    lcd.print(centerOfGravity, 1);

    displayUpdated = true;
  }

  controlBuzzer();
}

void calculateDatum3CG() {
  if (!displayUpdated) {
    lcd.clear();

    float centerOfGravity = savedDistancePoint + ((savedForwardPoint * savedLeverPoint) / savedWeightPoint);

    centerOfGravity = -abs(centerOfGravity);

    lcd.print("Center of Gravity: ");
    lcd.setCursor(0, 2);
    lcd.print(centerOfGravity, 1);

    displayUpdated = true;
  }

  controlBuzzer();
}

// Function to calculate Center of Gravity
void calculateCenterOfGravity() {
  Serial.print("Pilot1: ");
  Serial.println(savedPilot1Weight);
  Serial.print("Pilot2: ");
  Serial.println(savedPilot2Weight);
  Serial.print("Passenger1: ");
  Serial.println(savedPassenger1Weight);
  Serial.print("Passenger2: ");
  Serial.println(savedPassenger2Weight);
  Serial.print("Baggage1: ");
  Serial.println(savedBaggage1Weight);
  Serial.print("Baggage2: ");
  Serial.println(savedBaggage2Weight);
  Serial.print("Fuel: ");
  Serial.println(savedFuelWeight);
  // Calculate Total Moment
  float totalMoment = baseEmptyWeight * aircraftDistance + (savedPilot1Weight + savedPilot2Weight) * pilotDistance + (savedPassenger1Weight + savedPassenger2Weight) * passengerDistance + (savedBaggage1Weight + savedBaggage2Weight) * baggageDistance + savedFuelWeight * fuelDistance;

  // Calculate Total Weight
  float totalWeight = baseEmptyWeight + savedPilot1Weight + savedPilot2Weight + savedPassenger1Weight + savedPassenger2Weight + savedBaggage1Weight + savedBaggage2Weight + savedFuelWeight;

  // Only update LCD once if message hasn't been displayed
  if (!displayUpdated) {
    lcd.clear();

    // Check if total weight exceeds the maximum allowable weight
    if (totalWeight > 2550.0) {
      lcd.print("Overweight!");
      lcd.setCursor(0, 1);
      lcd.print("Total Weight: ");
      lcd.print(totalWeight, 1);  // Display total weight with 1 decimal place
      isError = true;             // Set error flag to play jingle
      playErrorJingleLoop();      // Play the error jingle
      displayUpdated = true;      // Set flag to avoid rapid updates
      return;                     // Exit function to avoid further calculations
    }

    // Calculate Center of Gravity if weight is within limits
    float centerOfGravity = totalMoment / totalWeight;

    lcd.print("CG: ");
    lcd.print(centerOfGravity, 1);  // Display CG value with 1 decimal place

    if (centerOfGravity >= minCG && centerOfGravity <= maxCG) {
      lcd.setCursor(0, 1);
      lcd.print("Acceptable");
      isError = false;
    } else {
      lcd.setCursor(0, 1);
      lcd.print("Unacceptable");
      isError = true;
    }

    displayUpdated = true;  // Set flag to indicate LCD is updated
  }

  latestButton = nullptr;

  controlBuzzer();  // Ensure buzzer is controlled based on `isError`
}

// Function to reset all weights and states
void resetToDefault() {
  // Reset unsaved (current) weights
  pilot1Weight = 170.0;
  pilot2Weight = 170.0;
  passenger1Weight = 170.0;
  passenger2Weight = 170.0;
  baggage1Weight = 30.0;
  baggage2Weight = 30.0;
  fuelWeight = 6.0;

  // DITO PALIT VALUES
  // ###################################################
  frontWheelWeight = 20.0;
  leftWheelWeight = 32.0;
  rightWheelWeight = 30.0;
  frontWheelDistance = -42.0;
  leftWheelDistance = 32.0;
  rightWheelDistance = 32.0;

  savedFrontWheelWeight = 20.0;
  savedLeftWheelWeight = 32.0;
  savedRightWheelWeight = 30.0;
  savedFrontWheelDistance = -42.0;
  savedLeftWheelDistance = 32.0;
  savedRightWheelDistance = 32.0;

  distancePoint = 32.0;
  forwardPoint = 24.0;
  leverPoint = 12.0;
  weightPoint = 8.0;

  savedDistancePoint = 32.0;
  savedForwardPoint = 24.0;
  savedLeverPoint = 12.0;
  savedWeightPoint = 8.0;
  // #####################################################

  // Reset saved weights
  savedPilot1Weight = 0.0;
  savedPilot2Weight = 0.0;
  savedPassenger1Weight = 0.0;
  savedPassenger2Weight = 0.0;
  savedBaggage1Weight = 0.0;
  savedBaggage2Weight = 0.0;
  savedFuelWeight = 0.0;

  // Clear the active section after resetting
  currentSection = nullptr;
  latestButton = nullptr;

  noTone(buzzerPin);  // Stop the buzzer if it's sounding

  lcd.clear();
  lcd.print("Reset to defaults");
  delay(1000);             // Brief delay to show the reset message
  resetTriggered = false;  // Reset the flag after completing reset
  isError = false;
}

// Variables to track the press state of each button
bool pilot1Pressed = false;
bool pilot2Pressed = false;
bool passenger1Pressed = false;
bool passenger2Pressed = false;
bool baggage1Pressed = false;
bool baggage2Pressed = false;
bool fuelPressed = false;

void checkObjectPresenceAndAdjustWeight() {

  // Check each button and update latestButton based on press states
  if (!digitalRead(pilot1ButtonPin)) {
    if (!pilot1Pressed) {  // Button was not pressed before
      latestButton = "Pilot1";
      pilot1Pressed = true;
    }
  } else {
    pilot1Pressed = false;
  }

  if (!digitalRead(pilot2ButtonPin)) {
    if (!pilot2Pressed) {
      latestButton = "Pilot2";
      pilot2Pressed = true;
    }
  } else {
    pilot2Pressed = false;
  }

  if (!digitalRead(passenger1ButtonPin)) {
    if (!passenger1Pressed) {
      latestButton = "Passenger1";
      passenger1Pressed = true;
    }
  } else {
    passenger1Pressed = false;
  }

  if (!digitalRead(passenger2ButtonPin)) {
    if (!passenger2Pressed) {
      latestButton = "Passenger2";
      passenger2Pressed = true;
    }
  } else {
    passenger2Pressed = false;
  }

  if (!digitalRead(baggage1ButtonPin)) {
    if (!baggage1Pressed) {
      latestButton = "Baggage1";
      baggage1Pressed = true;
    }
  } else {
    baggage1Pressed = false;
  }

  if (!digitalRead(baggage2ButtonPin)) {
    if (!baggage2Pressed) {
      latestButton = "Baggage2";
      baggage2Pressed = true;
    }
  } else {
    baggage2Pressed = false;
  }

  if (!digitalRead(fuelButtonPin)) {
    if (!fuelPressed) {
      latestButton = "Fuel";
      fuelPressed = true;
    }
  } else {
    fuelPressed = false;
  }

  // Update currentSection if a new button is pressed or the same button is held
  if (latestButton != nullptr) {
    currentSection = latestButton;

    // Call adjustWeight continuously for the active section
    if (currentSection == "Pilot1") adjustWeight(pilot1Weight, "Pilot1");
    else if (currentSection == "Pilot2") adjustWeight(pilot2Weight, "Pilot2");
    else if (currentSection == "Passenger1") adjustWeight(passenger1Weight, "Passenger1");
    else if (currentSection == "Passenger2") adjustWeight(passenger2Weight, "Passenger2");
    else if (currentSection == "Baggage1") adjustWeight(baggage1Weight, "Baggage1");
    else if (currentSection == "Baggage2") adjustWeight(baggage2Weight, "Baggage2");
    else if (currentSection == "Fuel") adjustWeight(fuelWeight, "Fuel");
  } else {
    // Reset currentSection only if no button is pressed
    currentSection = nullptr;
  }
}


void saveDatum1() {
  lcd.clear();
  lcd.print("Saving Datum 1...");

  savedFrontWheelWeight = frontWheelWeight;
  savedLeftWheelWeight = leftWheelWeight;
  savedRightWheelWeight = rightWheelWeight;

  savedFrontWheelDistance = frontWheelDistance;
  savedLeftWheelDistance = leftWheelDistance;
  savedRightWheelDistance = rightWheelDistance;

  delay(500);

  lcd.clear();
  lcd.print("Values Saved");
  delay(1000);
}

void saveOtherDatums(bool isDatum3) {
  lcd.clear();
  lcd.print("Saving ");
  if (isDatum3) {
    lcd.print("3...");
  } else {
    lcd.print("2...");
  }

  savedDistancePoint = distancePoint;
  savedForwardPoint = forwardPoint;
  savedLeverPoint = leverPoint;
  savedWeightPoint = weightPoint;

  delay(500);

  lcd.clear();
  lcd.print("Values Saved");
  delay(1000);
}

void saveWeight() {
  bool anyWeightSaved = false;  // Track if any weight was actually saved

  lcd.clear();
  lcd.print("Saving...");

  // Check each button and save the weight if the button is currently pressed
  if (!digitalRead(pilot1ButtonPin)) {
    savedPilot1Weight = pilot1Weight;
    anyWeightSaved = true;
    lcd.setCursor(0, 1);
    lcd.print("Pilot1 saved");
    delay(500);
  }
  if (!digitalRead(pilot2ButtonPin)) {
    savedPilot2Weight = pilot2Weight;
    anyWeightSaved = true;
    lcd.setCursor(0, 1);
    lcd.print("Pilot2 saved");
    delay(500);
  }
  if (!digitalRead(passenger1ButtonPin)) {
    savedPassenger1Weight = passenger1Weight;
    anyWeightSaved = true;
    lcd.setCursor(0, 1);
    lcd.print("Passenger1 saved");
    delay(500);
  }
  if (!digitalRead(passenger2ButtonPin)) {
    savedPassenger2Weight = passenger2Weight;
    anyWeightSaved = true;
    lcd.setCursor(0, 1);
    lcd.print("Passenger2 saved");
    delay(500);
  }
  if (!digitalRead(baggage1ButtonPin)) {
    savedBaggage1Weight = baggage1Weight;
    anyWeightSaved = true;
    lcd.setCursor(0, 1);
    lcd.print("Baggage1 saved");
    delay(500);
  }
  if (!digitalRead(baggage2ButtonPin)) {
    savedBaggage2Weight = baggage2Weight;
    anyWeightSaved = true;
    lcd.setCursor(0, 1);
    lcd.print("Baggage2 saved");
    delay(500);
  }
  if (!digitalRead(fuelButtonPin)) {
    savedFuelWeight = fuelWeight;
    anyWeightSaved = true;
    lcd.setCursor(0, 1);
    lcd.print("Fuel saved");
    delay(500);
  }

  // Clear the active section after saving
  currentSection = nullptr;
  latestButton = nullptr;

  // Provide final feedback
  lcd.clear();
  if (anyWeightSaved) {
    lcd.print("Weights Saved!");
  } else {
    lcd.print("No changes to save");
  }
  delay(1000);  // Show the message briefly
}

void loop() {
  unsigned long currentMillis = millis();

  // Check for reset button press using millis for debounce
  if (!digitalRead(resetButtonPin)) {
    if (!resetTriggered && (currentMillis - resetDebounceTime >= debounceDelay)) {
      resetTriggered = true;              // Set the reset flag
      resetDebounceTime = currentMillis;  // Update debounce timer
    }
  }

  // If reset is triggered, perform reset
  if (resetTriggered) {
    resetToDefault();
    return;  // Exit loop early to complete reset and avoid further processing
  }

  // Check if calculate button is held down
  if (!digitalRead(calculateButtonPin)) {
    // Show center of gravity while calculate button is pressed
    if (isFixed) {
      if (datum1State) {
        calculateDatum1CG();
      } else if (datum2State) {
        calculateDatum2CG();
      } else if (datum3State) {
        calculateDatum3CG();
      }
    } else {
      calculateCenterOfGravity();
    }
    if (isError) {
      playErrorJingleLoop();
    }
    return;
  }

  // Check if save button is pressed
  if (!digitalRead(saveButtonPin)) {
    if (isFixed) {
      if (datum1State) {
        saveDatum1();
      } else if (datum2State) {
        saveOtherDatums(false);
      } else if (datum3State) {
        saveOtherDatums(true);
      }
    } else {
      saveWeight();  // Save weights when save button is pressed
    }
    return;
  }

  displayUpdated = false;

  controlBuzzer();

  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    isFixed = !digitalRead(fixedSwitchPin);

    if (!isFixed) {  // Variable mode
      lcd.clear();
      lcd.print("Variable Mode");
      noTone(buzzerPin);
      isError = false;

      checkObjectPresenceAndAdjustWeight();

    } else {
      lcd.clear();
      lcd.print("Fixed Mode Active");

      // Read datum switch states
      datum1State = !digitalRead(datum1Pin);
      datum2State = !digitalRead(datum2Pin);
      datum3State = !digitalRead(datum3Pin);

      int datumCount = datum1State + datum2State + datum3State;

      if (datumCount > 1) {
        lcd.clear();
        // More than one datum switch is ON, trigger error
        lcd.print("Error: Multiple");
        lcd.setCursor(0, 1);
        lcd.print("datum switches ON");

        // Set error flag to true to play the jingle
        isError = true;
        controlBuzzer();
      } else {
        // Clear the error flag and stop buzzer if no error
        isError = false;
        noTone(buzzerPin);

        // Display the center of gravity based on active datum switch
        if (datum1State) {
          frontWheelWeightState = !digitalRead(frontWheelWeightSwitch);
          leftWheelWeightState = !digitalRead(leftWheelWeightSwitch);
          rightWheelWeightState = !digitalRead(rightWheelWeightSwitch);

          frontWheelDistanceState = !digitalRead(frontWheelDistanceSwitch);
          leftWheelDistanceState = !digitalRead(leftWheelDistanceSwitch);
          rightWheelDistanceState = !digitalRead(rightWheelDistanceSwitch);

          int weightSwitchCount = frontWheelWeightState + leftWheelWeightState + rightWheelWeightState;
          int distanceSwitchCount = frontWheelDistanceState + leftWheelDistanceState + rightWheelDistanceState;

          if (weightSwitchCount > 0 && distanceSwitchCount > 0) {
            lcd.clear();
            // More than one datum switch is ON, trigger error
            lcd.print("Error: Multiple");
            lcd.setCursor(0, 1);
            lcd.print("weight and distance");

            // Set error flag to true to play the jingle
            isError = true;
            controlBuzzer();

            return;
          }

          if (weightSwitchCount > 1) {
            lcd.clear();
            // More than one datum switch is ON, trigger error
            lcd.print("Error: Multiple");
            lcd.setCursor(0, 1);
            lcd.print("weight switches ON");

            // Set error flag to true to play the jingle
            isError = true;
            controlBuzzer();

            return;
          }

          if (distanceSwitchCount > 1) {
            lcd.clear();
            // More than one datum switch is ON, trigger error
            lcd.print("Error: Multiple");
            lcd.setCursor(0, 1);
            lcd.print("distance switches ON");

            // Set error flag to true to play the jingle
            isError = true;
            controlBuzzer();

            return;
          }

          isError = false;
          noTone(buzzerPin);

          if (frontWheelWeightState) adjustWheelWeight(frontWheelWeight, "Nose Wheel", false);
          else if (leftWheelWeightState) adjustWheelWeight(leftWheelWeight, "Left Wheel", false);
          else if (rightWheelWeightState) adjustWheelWeight(rightWheelWeight, "Right Wheel", false);
          else if (frontWheelDistanceState) adjustWheelWeight(frontWheelDistance, "Nose Wheel", true);
          else if (leftWheelDistanceState) adjustWheelWeight(leftWheelDistance, "Left Wheel", true);
          else if (rightWheelDistanceState) adjustWheelWeight(rightWheelDistance, "Right Wheel", true);
        } else if (datum2State) {
          distancePointState = !digitalRead(distancePointSwitch);
          forwardPointState = !digitalRead(forwardPointSwitch);
          leverPointState = !digitalRead(leverPointSwitch);
          weightPointState = !digitalRead(weightPointSwitch);

          int pointCount = distancePointState + forwardPointState + leverPointState + weightPointState;

          if (pointCount > 1) {
            lcd.clear();
            // More than one datum switch is ON, trigger error
            lcd.print("Error: Multiple");
            lcd.setCursor(0, 1);
            lcd.print("point switches ON");

            // Set error flag to true to play the jingle
            isError = true;
            controlBuzzer();

            return;
          }

          distancePoint = abs(distancePoint);
          forwardPoint = abs(forwardPoint);
          leverPoint = abs(leverPoint);
          weightPoint = abs(weightPoint);

          if (distancePointState) adjustWheelWeight(distancePoint, "Distance Point", true);
          else if (forwardPointState) adjustWheelWeight(forwardPoint, "Forward Point", false);
          else if (leverPointState) adjustWheelWeight(leverPoint, "Lever Point", true);
          else if (weightPointState) adjustWheelWeight(weightPoint, "Total Weight", true);
        } else if (datum3State) {
          distancePointState = !digitalRead(distancePointSwitch);
          forwardPointState = !digitalRead(forwardPointSwitch);
          leverPointState = !digitalRead(leverPointSwitch);
          weightPointState = !digitalRead(weightPointSwitch);

          int pointCount = distancePointState + forwardPointState + leverPointState + weightPointState;

          if (pointCount > 1) {
            lcd.clear();
            // More than one datum switch is ON, trigger error
            lcd.print("Error: Multiple");
            lcd.setCursor(0, 1);
            lcd.print("point switches ON");

            // Set error flag to true to play the jingle
            isError = true;
            controlBuzzer();

            return;
          }

          distancePoint = abs(distancePoint);
          forwardPoint = abs(forwardPoint);
          leverPoint = abs(leverPoint);
          weightPoint = abs(weightPoint);

          if (distancePointState) adjustWheelWeight(distancePoint, "Distance Point", true);
          else if (forwardPointState) adjustWheelWeight(forwardPoint, "Forward Point", false);
          else if (leverPointState) adjustWheelWeight(leverPoint, "Lever Point", true);
          else if (weightPointState) adjustWheelWeight(weightPoint, "Total Weight", false);
        } else {
          lcd.setCursor(0, 1);
          lcd.print("No Datum Active");
        }
      }
    }
  }
}