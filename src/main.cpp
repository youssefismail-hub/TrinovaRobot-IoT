#include <Arduino.h>
#include "TrinovaRobot.h"

TrinovaRobot robot;

void setup() {
    Serial.begin(115200);
    delay(1000);
    bool ok = robot.begin(RobotMode::Mock);
    Serial.printf("[Setup] begin() = %s\n", ok ? "OK" : "FAIL");
}

void loop() {
    robot.update(); // vérifie le timeout de commande à chaque cycle

    robot.forward(40);
    delay(800);

    robot.turnLeft(30);
    delay(400);

    robot.stop();
    delay(1500);
}