#include <Wire.h>
#include <Adafruit_PWMServoDriver.h>

#define MIN_PULSE_WIDTH       650
#define MAX_PULSE_WIDTH       2350
#define FREQUENCY             50
#define DEBOUNCE_DELAY        50

Adafruit_PWMServoDriver pwm = Adafruit_PWMServoDriver();

// Potentiometer pins
int potWrist = A3;
int potElbow = A2;
int potShoulder = A1;
int potBase = A0;

// Button pin (changed from pin 13 to avoid conflict)
int buttonPin = 2;  // Use digital pin 2 for button instead

// Servo channel assignments on PCA9685
int hand = 11;
int wrist = 12;
int elbow = 13;
int shoulder = 14;
int base = 15;

// Button debounce variables
int lastButtonState = HIGH;
int buttonState = HIGH;
unsigned long lastDebounceTime = 0;

void setup() 
{
  delay(5000);  // Time to position controller
  
  pwm.begin();
  pwm.setPWMFreq(FREQUENCY);
  pwm.setPWM(hand, 0, pulseWidthToPWM(90));  // Close gripper initially
  
  pinMode(buttonPin, INPUT_PULLUP);  // Button pin with pullup
  
  Serial.begin(9600);
}

// Helper function to convert angle to PWM value
int pulseWidthToPWM(int angle) 
{
  int pulse_wide = map(angle, 0, 180, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
  int pulse_width = int(float(pulse_wide) / 1000000 * FREQUENCY * 4096);
  return pulse_width;
}

void moveMotor(int controlIn, int motorOut)
{
  int pulse_wide, pulse_width, potVal;
  
  potVal = analogRead(controlIn);  // Read potentiometer (0-1023)
  
  // FIXED: Corrected mapping - map FROM (0, 1023) TO pulse widths
  pulse_wide = map(potVal, 0, 1023, MIN_PULSE_WIDTH, MAX_PULSE_WIDTH);
  pulse_width = int(float(pulse_wide) / 1000000 * FREQUENCY * 4096);
  
  pwm.setPWM(motorOut, 0, pulse_width);
}

void loop() 
{
  // Control all servo motors with potentiometers
  moveMotor(potWrist, wrist);
  moveMotor(potElbow, elbow);
  moveMotor(potShoulder, shoulder);
  moveMotor(potBase, base);
  
  // FIXED: Added button debounce logic
  int reading = digitalRead(buttonPin);
  
  if (reading != lastButtonState) {
    lastDebounceTime = millis();  // Reset debounce timer
  }
  
  if ((millis() - lastDebounceTime) > DEBOUNCE_DELAY) {
    // If reading has been stable for debounce delay
    if (reading != buttonState) {
      buttonState = reading;
      
      // Control gripper based on stable button state
      if (buttonState == LOW) {
        pwm.setPWM(hand, 0, pulseWidthToPWM(180));  // Close gripper
        Serial.println("Grab");
      }
      else {
        pwm.setPWM(hand, 0, pulseWidthToPWM(90));   // Open gripper
        Serial.println("Release");
      }
    }
  }
  
  lastButtonState = reading;
}
