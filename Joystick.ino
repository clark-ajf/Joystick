#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>


//Joystick Pins
#define JOYSTICK_X_PIN A0;                                               
#define JOYSTICK_Y_PIN A1;

// Define Joystick Values - Start at 512 (middle position)
#define IDLE_POSITION 512;
#define DEAD_ZONE 52;
#define MIN_SPEED 8;

int X_POS = IDLE_POSITION;
int Y_POS = IDLE_POSITION;

// Declare unsigned 8-bit motorcontrol array
// 2 Bytes for motor speeds plus 1 byte for direction control
uint8_t motorcontrol[3];

//nRF24L01 Pins
#define CE_PIN = 7;
#define SCK_PIN = 13;
#define MISO_PIN = 12;
#define CSN_PIN  = 8;
#define MOSI_PIN = 11; 

RF24 radio(CE_PIN, CSN_PIN); // CE, CSN
const byte address[6] = "00001";


void setup() {
    radio.begin();
    radio.openWritingPipe(address);
    radio.setPALevel(RF24_PA_MIN);
    radio.stopListening();
}

void loop() {
    // Read Joystick values
    X_POS = analogRead(JOYSTICK_X_PIN);
    Y_POS = analogRead(JOYSTICK_Y_PIN);

    // Determine if this is a forward or backward motion
    if (Y_POS < IDLE_POSITION - DEAD_ZONE) { // Backwards
        // Set Motors backward
        motorcontrol[2] = 1;    
        //Determine Motor Speeds
        // As we are going backwards we need to reverse readings
        motorcontrol[0] = map(JOYSTICK_Y_PIN, IDLE_POSITION - DEAD_ZONE, 0, 0, 255);
        motorcontrol[1] = map(JOYSTICK_Y_PIN, IDLE_POSITION - DEAD_ZONE, 0, 0, 255);
    } else if (Y_POS > IDLE_POSITION + DEAD_ZONE) { // Forward
        // Set Motors forward
        motorcontrol[2] = 0;    
        //Determine Motor Speeds
        motorcontrol[0] = map(JOYSTICK_Y_PIN, IDLE_POSITION + DEAD_ZONE, 1023, 0, 255);
        motorcontrol[1] = map(JOYSTICK_Y_PIN, IDLE_POSITION + DEAD_ZONE, 1023, 0, 255);

    } else { // Stop
        motorcontrol[0] = 0;
        motorcontrol[1] = 0;
        motorcontrol[3] = 0;
    }

    // Now the steering. The Horizontal position will "weigh" the motor speed values for each motor
    if (X_POS < IDLE_POSITION - DEAD_ZONE) { // Left
        X_POS = map(JOYSTICK_X_PIN, IDLE_POSITION - DEAD_ZONE, 0, 0, 255);
        motorcontrol[0] = motorcontrol[0] - X_POS;
        motorcontrol[1] = motorcontrol[1] + X_POS;

        // Don't exceed range of 0-255 for motor speeds
        motorcontrol[0] = motorcontrol[0] < 0? 0: motorcontrol[0];
        motorcontrol[1] = motorcontrol[1] > 255? 255: motorcontrol[1];
    } else if (X_POS > IDLE_POSITION + DEAD_ZONE) { // Right
        X_POS = map(JOYSTICK_X_PIN, IDLE_POSITION + DEAD_ZONE, 1023, 0, 255);
        motorcontrol[0] = motorcontrol[0] + X_POS;
        motorcontrol[1] = motorcontrol[1] - X_POS;

        // Don't exceed range of 0-255 for motor speeds
        motorcontrol[1] = motorcontrol[1] < 0? 0: motorcontrol[1];
        motorcontrol[0] = motorcontrol[0] > 255? 255: motorcontrol[0];
    }

    // Adjust to prevent "buzzing" at very low speed
    motorcontrol[0] = motorcontrol[0] < MIN_SPEED ? 0: motorcontrol[0];
    motorcontrol[1] = motorcontrol[1] < MIN_SPEED ? 0: motorcontrol[1];

    radio.write(motorcontrol, sizeof(motorcontrol));
    delay(100);
}
