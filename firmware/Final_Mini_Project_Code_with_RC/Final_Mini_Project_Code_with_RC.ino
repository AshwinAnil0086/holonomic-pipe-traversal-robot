#include <Dynamixel2Arduino.h>

#define DXL_SERIAL   Serial3
#define DEBUG_SERIAL Serial

const int DXL_DIR_PIN = 22;

Dynamixel2Arduino dxl(DXL_SERIAL, DXL_DIR_PIN);

const float DXL_PROTOCOL_VERSION = 2.0;

// RC Receiver pins
#define MOVE_CH 10    // CH2 -> Up / Down
#define TURN_CH 11    // CH4 -> Rotate CW / CCW

int movePWM = 1500;
int turnPWM = 1500;

void setup() {

  DEBUG_SERIAL.begin(115200);

  pinMode(MOVE_CH, INPUT);
  pinMode(TURN_CH, INPUT);

  // Dynamixel setup
  dxl.begin(1000000);
  dxl.setPortProtocolVersion(DXL_PROTOCOL_VERSION);

  // Configure motors
  for (int DXL_ID = 1; DXL_ID <= 4; DXL_ID++) {
    dxl.torqueOff(DXL_ID);
    dxl.setOperatingMode(DXL_ID, OP_VELOCITY);
    dxl.torqueOn(DXL_ID);
  }
  DEBUG_SERIAL.println("Dual Channel Dynamixel Control Started");
}

void loop() {
  // Read PWM
  movePWM = pulseIn(MOVE_CH, HIGH, 25000);
  turnPWM = pulseIn(TURN_CH, HIGH, 25000);
  
  // Fail-safe
  if (movePWM < 1000 || movePWM > 2000)
    movePWM = 1500;
  if (turnPWM < 1000 || turnPWM > 2000)
    turnPWM = 1500;

  // Convert PWM to RPM
  float moveVelocity = -pwmToRPM(movePWM);
  float turnVelocity = pwmToRPM(turnPWM);

  // Mixing
  float leftVelocity  = -moveVelocity + turnVelocity;
  float rightVelocity = moveVelocity + turnVelocity;

  // Limit RPM
  leftVelocity  = constrain(leftVelocity,  -40, 40);
  rightVelocity = constrain(rightVelocity, -40, 40);

  // Debug
  // Debug
  DEBUG_SERIAL.print("MOVE PWM: ");
  DEBUG_SERIAL.print(movePWM);
  
  DEBUG_SERIAL.print(" | TURN PWM: ");
  DEBUG_SERIAL.print(turnPWM);
  
  DEBUG_SERIAL.print(" | MOVE RPM: ");
  DEBUG_SERIAL.print(moveVelocity);
  
  DEBUG_SERIAL.print(" | TURN RPM: ");
  DEBUG_SERIAL.print(turnVelocity);
  
  DEBUG_SERIAL.print(" | LEFT RPM: ");
  DEBUG_SERIAL.print(leftVelocity);
  
  DEBUG_SERIAL.print(" | RIGHT RPM: ");
  DEBUG_SERIAL.println(rightVelocity);

  // Left side motors
  dxl.setGoalVelocity(1, leftVelocity, UNIT_RPM);
  dxl.setGoalVelocity(2, leftVelocity, UNIT_RPM);

  // Right side motors
  dxl.setGoalVelocity(3, rightVelocity, UNIT_RPM);
  dxl.setGoalVelocity(4, rightVelocity, UNIT_RPM);

  delay(20);
}


// ======================================================
// Function: PWM -> RPM with deadband
// ======================================================

float pwmToRPM(int pwm)
{
  // Deadband
  if (pwm >= 1460 && pwm <= 1540) {
    return 0;
  }
  
  // Reverse
  else if (pwm < 1460) {
    return map(pwm, 1000, 1460, -40, 0);
  }
  
  // Forward
  else {
    return map(pwm, 1540, 2000, 0, 40);
  }
}
