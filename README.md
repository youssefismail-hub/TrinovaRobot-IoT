# TrinovaRobot-IoT

Firmware robotique edge IoT pour ESP32-S3 — moteurs DC, servomoteurs PCA9685,
sécurité temps réel, connectivité MQTT, OTA.

## Câblage

⚠️ **L'alimentation des moteurs/servos doit être séparée de l'alimentation logique
de l'ESP32.** Un pic de courant moteur peut faire chuter la tension logique et
provoquer un reboot inattendu si les deux sont sur la même source.

| Fonction | Pin ESP32-S3 | Notes |
|---|---|---|
| Moteur gauche IN1/IN2/PWM | 4 / 5 / 6 | Voir `HardwarePins.h` |
| Moteur droit IN1/IN2/PWM | 7 / 15 / 16 | Voir `HardwarePins.h` |
| I2C SDA / SCL (PCA9685) | 8 / 9 | Adresse par défaut `0x40` |
| ADC batterie | 1 | Pont diviseur à calibrer (voir `BatteryMonitor`) |

## Installation

```bash
git clone <url-du-depot>
cd TrinovaRobot-IoT
cp include/IoTConfig.h.example include/IoTConfig.h   # renseigne SSID/mot de passe/broker
pio run -e esp32dev -t upload
pio device monitor
```

## Tests

```bash
pio test -e native        # tests unitaires MotionSafety + RobotStateMachine (nécessite g++)
pio run -e esp32dev        # compilation cible réelle
```

## API publique

```cpp
#include <TrinovaRobot.h>
TrinovaRobot robot;
robot.begin(RobotMode::Mock);   // ou RobotMode::Hardware
robot.forward(40);
robot.backward(40);
robot.turnLeft(40);
robot.turnRight(40);
robot.stop();
robot.moveServo(0, 90);
robot.emergencyStop();
robot.clearEmergencyStop();
```

## Sécurité

- Toute vitesse est bornée 0-100, tout angle borné par canal, quelle que soit la source
  (série locale ou commande MQTT) — voir `MotionSafety`.
- Timeout de commande : arrêt automatique si aucune commande reçue dans `commandTimeoutMs`.
- `emergencyStop()` a priorité absolue ; seul `clearEmergencyStop()` débloque.
- Voir `docs/state_machine.md` et `docs/mqtt_topics.md` pour le détail.

## Limites connues

- OTA en HTTP simple (non chiffré) — à durcir en TLS avant tout déploiement hors labo.
- `BatteryMonitor` nécessite une calibration manuelle du pont diviseur avant d'être fiable.
- Pas de PID moteur (boucle ouverte PWM) — pas d'asservissement de vitesse réelle.

## Documentation complémentaire

- `docs/architecture.md` — vue d'ensemble des 5 couches et des tâches FreeRTOS
- `docs/mqtt_topics.md` — schéma complet des topics et payloads
- `docs/state_machine.md` — diagramme des transitions d'état