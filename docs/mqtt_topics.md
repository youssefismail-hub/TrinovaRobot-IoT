# Topics MQTT — TrinovaRobot IoT

Racine : `trinova/robot/{deviceId}/`
`{deviceId}` est généré automatiquement au premier boot (`robot-ESP32-XXXXXX`), stocké en NVS.

| Topic | Sens | QoS/Retained | Payload |
|---|---|---|---|
| `.../status` | Robot -> Cloud | Retained | `"online"` / `"offline"` (LWT automatique) |
| `.../telemetry` | Robot -> Cloud | QoS 0, 1x/s | JSON état + batterie + heap + rssi + uptime |
| `.../command` | Cloud -> Robot | QoS 0 | JSON commande (voir schéma ci-dessous) |
| `.../error` | Robot -> Cloud | QoS 0 | Événements bufferisés hors-ligne, republiés à la reconnexion |
| `.../ota` | Cloud -> Robot | QoS 0 | `{"url": "http://..."}` déclenche une mise à jour firmware |

## Schéma des commandes (`.../command`)

```json
{"type": "move", "direction": "forward", "speed": 60}
{"type": "move", "direction": "backward", "speed": 40}
{"type": "move", "direction": "left", "speed": 30}
{"type": "move", "direction": "right", "speed": 30}
{"type": "stop"}
{"type": "servo", "channel": 0, "angle": 90}
{"type": "emergency_stop"}
{"type": "clear_emergency_stop"}
```

**Sécurité** : `speed` et `angle` sont toujours reclampés côté firmware par `MotionSafety`
(0-100 pour la vitesse, bornes par canal pour l'angle), quelle que soit la valeur envoyée
sur MQTT. Aucune commande réseau ne peut contourner ces limites.

## Exemple de payload télémétrie

```json
{"state":"MOVING","battery":7.4,"heap":178420,"rssi":-58,"uptime":342,"errors":0}
```