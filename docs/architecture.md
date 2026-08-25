
# Architecture — TrinovaRobot-IoT

5 couches, isolation stricte, exécution multi-tâches FreeRTOS sur ESP32-S3 (2 cœurs).

```text
Core 1 (temps réel)                    Core 0 (réseau)

┌─────────────────────┐               ┌───────────────────────┐
│ SafetyTask (prio 3) │               │ IoTTask (prio 1)      │
│ MotionTask (prio 2) │               │ TelemetryTask (prio 1)│
└─────────┬───────────┘               └──────────┬────────────┘
          │                                      │
          ▼                                      ▼
┌───────────────┐  mutex g_robotMutex   ┌────────────────────┐
│ TrinovaRobot  │<───────────────────── │ CommandProcessor   │
│ (Core+Safety) │                       │ TelemetryPublisher │
└───────┬───────┘                       └────────────────────┘
        │
        ▼
┌───────┴───────┐
│ IRobotDriver  │
│ Mock/Hardware │
└───────────────┘
````

## Principe non négociable

Le Core (`TrinovaRobot`, `MotionSafety`, `RobotStateMachine`) ne dépend **jamais** de l'IoT Layer.

Le robot reste pilotable et sûr en local (mode Mock ou Hardware) même si le Wi-Fi/MQTT est coupé.

Toute commande, qu'elle vienne du firmware local ou de MQTT, repasse obligatoirement par `MotionSafety` avant d'atteindre les moteurs — voir `docs/mqtt_topics.md` pour le détail de cette garantie côté `CommandProcessor`.

## Couches

1. **Drivers** — `Mock`, `DcMotorDriver`, `Pca9685Driver`, `ServoDriver` (aucune dépendance réseau)
2. **Core** — `TrinovaRobot`, `MotionSafety`, `RobotStateMachine` (déterministe, testé en natif)
3. **Services** — `Logger`, `WatchdogService`, `BatteryMonitor`, `DiagnosticsService`
4. **IoT** — `DeviceIdentity`, `WiFiManager`, `MqttClient`, `CommandProcessor`, `TelemetryPublisher`, `OtaManager`, `OfflineBuffer`
5. **Application** — `main.cpp`, orchestration des 4 tâches FreeRTOS
