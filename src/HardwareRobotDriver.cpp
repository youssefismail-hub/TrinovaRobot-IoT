#include "HardwareRobotDriver.h"
#include "Logger.h"

HardwareRobotDriver::HardwareRobotDriver()
    : _motors(HardwareConfig::DefaultMotorPins),
      _pca(HardwareConfig::DEFAULT_PCA9685_ADDRESS, HardwareConfig::I2C_SDA_PIN, HardwareConfig::I2C_SCL_PIN),
      _servo(_pca) {}

bool HardwareRobotDriver::begin() {
    _motorsReady = _motors.begin();
    if (!_motorsReady) {
        Logger::log(LogLevel::Error, "HW", "DcMotorDriver init failed");
    }

    bool pcaReady = _pca.begin();
    _servoReady = pcaReady && _servo.begin();
    if (!_servoReady) {
        // Non bloquant : le robot peut rouler sans servo (exigence §11 "PCA9685 absent -> pas de blocage")
        Logger::log(LogLevel::Warn, "HW", "PCA9685/Servo indisponible, mode roues seules");
    }

    return _motorsReady; // les moteurs sont obligatoires, le servo est optionnel
}

void HardwareRobotDriver::setWheelCommand(WheelCommand cmd, uint8_t speed) {
    if (!_motorsReady) return;
    
    // turnLeft  -> moteur gauche ARRÊTÉ, moteur droit AVANT (pivot sur place, moins de contrainte mécanique)
    // turnRight -> moteur droit ARRÊTÉ, moteur gauche AVANT
    switch (cmd) {
        case WheelCommand::Forward:
            _motors.setLeftMotor(DcMotorDriver::Direction::Forward, speed);
            _motors.setRightMotor(DcMotorDriver::Direction::Forward, speed);
            break;
        case WheelCommand::Backward:
            _motors.setLeftMotor(DcMotorDriver::Direction::Backward, speed);
            _motors.setRightMotor(DcMotorDriver::Direction::Backward, speed);
            break;
        case WheelCommand::TurnLeft:
            _motors.setLeftMotor(DcMotorDriver::Direction::Stop, 0);
            _motors.setRightMotor(DcMotorDriver::Direction::Forward, speed);
            break;
        case WheelCommand::TurnRight:
            _motors.setLeftMotor(DcMotorDriver::Direction::Forward, speed);
            _motors.setRightMotor(DcMotorDriver::Direction::Stop, 0);
            break;
        case WheelCommand::Stop:
            _motors.stopAll();
            break;
    }
}

void HardwareRobotDriver::stopWheels() {
    _motors.stopAll();
}

bool HardwareRobotDriver::setServoAngle(uint8_t channel, uint8_t angle) {
    if (!_servoReady) return false;
    return _servo.setAngle(channel, angle);
}

void HardwareRobotDriver::emergencyStop() {
    _motors.stopAll();
}