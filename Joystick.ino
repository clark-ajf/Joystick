#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


//Joystick Pins
#define JOYSTICK_X_PIN A0
#define JOYSTICK_Y_PIN A1

// Define Joystick Values - Start at 512 (middle position)
#define IDLE_POSITION 512
#define DEAD_ZONE 52
#define MIN_SPEED 8

#define DIRECTION 2
#define MOTOR_RIGHT 1
#define MOTOR_LEFT 0

int X_POS = IDLE_POSITION;
int Y_POS = IDLE_POSITION;

// Declare unsigned 8-bit motorcontrol array
// 2 Bytes for motor speeds plus 1 byte for direction control
uint8_t motorcontrol[3];

//nRF24L01 Pins
#define CE_PIN 7
#define SCK_PIN 13
#define MISO_PIN 12
#define CSN_PIN  8
#define MOSI_PIN 11

RF24 radio(CE_PIN, CSN_PIN); // CE, CSN
const byte address[6] = "00001";


void setup() {
    //Starting the serial communication at 9600 baud rate
    Serial.begin (9600);

    // Initializing Radio Trasmitter
    radio.begin();
    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_MIN);
    radio.stopListening();

    //Initializing Joystick PINs as INPUT
    pinMode(JOYSTICK_X_PIN, INPUT) ;                     
    pinMode(JOYSTICK_Y_PIN, INPUT) ;
}

void loop() {
    // Read Joystick values
    X_POS = analogRead(JOYSTICK_X_PIN);
    Y_POS = analogRead(JOYSTICK_Y_PIN);

    // Determine if this is a forward or backward motion
    if (Y_POS < IDLE_POSITION - DEAD_ZONE) { // Backwards ### REPLACED AS FORDWARD IN CURRENT JOYSTICK
        // Set Motors backward
        motorcontrol[DIRECTION] = 0; // ### REPLACED AS FORDWARD IN CURRENT JOYSTICK (SHOULD BE 1)
        //Determine Motor Speeds
        // As we are going backwards we need to reverse readings
        motorcontrol[MOTOR_LEFT] = map(JOYSTICK_Y_PIN, IDLE_POSITION - DEAD_ZONE, 0, 0, 255);
        motorcontrol[MOTOR_RIGHT] = map(JOYSTICK_Y_PIN, IDLE_POSITION - DEAD_ZONE, 0, 0, 255);
    } else if (Y_POS > IDLE_POSITION + DEAD_ZONE) { // Forward ### REPLACED AS BACKWARD IN CURRENT JOYSTICK
        // Set Motors forward
        motorcontrol[DIRECTION] = 1; // ### REPLACED AS BACKWARD IN CURRENT JOYSTICK (SHOULD BE 1)
        //Determine Motor Speeds
        motorcontrol[MOTOR_LEFT] = map(JOYSTICK_Y_PIN, IDLE_POSITION + DEAD_ZONE, 1023, 0, 255);
        motorcontrol[MOTOR_RIGHT] = map(JOYSTICK_Y_PIN, IDLE_POSITION + DEAD_ZONE, 1023, 0, 255);

    } else { // Stop
        motorcontrol[MOTOR_LEFT] = 0;
        motorcontrol[MOTOR_RIGHT] = 0;
        motorcontrol[DIRECTION] = 0;
    }

    // Now the steering. The Horizontal position will "weigh" the motor speed values for each motor
    if (X_POS < IDLE_POSITION - DEAD_ZONE) { // Left
        X_POS = map(JOYSTICK_X_PIN, IDLE_POSITION - DEAD_ZONE, 0, 0, 255);
        motorcontrol[MOTOR_LEFT] = motorcontrol[MOTOR_LEFT] - X_POS;
        motorcontrol[MOTOR_RIGHT] = motorcontrol[MOTOR_RIGHT] + X_POS;

        // Don't exceed range of 0-255 for motor speeds
        motorcontrol[MOTOR_LEFT] = motorcontrol[MOTOR_LEFT] < 0? 0: motorcontrol[MOTOR_LEFT];
        motorcontrol[MOTOR_RIGHT] = motorcontrol[MOTOR_RIGHT] > 255? 255: motorcontrol[1];
    } else if (X_POS > IDLE_POSITION + DEAD_ZONE) { // Right
        X_POS = map(JOYSTICK_X_PIN, IDLE_POSITION + DEAD_ZONE, 1023, 0, 255);
        motorcontrol[MOTOR_LEFT] = motorcontrol[MOTOR_LEFT] + X_POS;
        motorcontrol[MOTOR_RIGHT] = motorcontrol[MOTOR_RIGHT] - X_POS;

        // Don't exceed range of 0-255 for motor speeds
        motorcontrol[MOTOR_RIGHT] = motorcontrol[MOTOR_RIGHT] < 0? 0: motorcontrol[1];
        motorcontrol[MOTOR_LEFT] = motorcontrol[MOTOR_LEFT] > 255? 255: motorcontrol[MOTOR_LEFT];
    }

    // Adjust to prevent "buzzing" at very low speed
    motorcontrol[MOTOR_LEFT] = motorcontrol[MOTOR_LEFT] < MIN_SPEED ? 0: motorcontrol[MOTOR_LEFT];
    motorcontrol[MOTOR_RIGHT] = motorcontrol[MOTOR_RIGHT] < MIN_SPEED ? 0: motorcontrol[MOTOR_RIGHT];

    radio.write(motorcontrol, sizeof(motorcontrol));

    Serial.print("Motor LEFT: ");
    Serial.print(motorcontrol[MOTOR_LEFT]);
    Serial.print(" - Motor RIGHT: ");
    Serial.print(motorcontrol[MOTOR_RIGHT]);
    Serial.print(" - Direction: ");
    Serial.println(motorcontrol[DIRECTION]);
}
