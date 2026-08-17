#pragma once
#include <cstdint>
#include "RobotTypes.h"

// Règles de sécurité pures : clamp vitesse/angle, timeout de commande, emergency stop.
// Aucune dépendance Arduino -> testable en environnement "native".
class MotionSafety {
public:
    explicit MotionSafety(const RobotConfig& config);

    uint8_t clampSpeed(uint8_t speed) const;
    uint8_t clampAngle(uint8_t angle) const;

    void onCommandIssued(uint32_t nowMs);
    bool isTimedOut(uint32_t nowMs) const;

    void triggerEmergencyStop();
    bool clearEmergencyStop();
    bool isEmergencyStopActive() const { return _emergencyActive; }

private:
    RobotConfig _config;
    uint32_t    _lastCommandMs = 0;
    bool        _hasCommand    = false;
    bool        _emergencyActive = false;
};