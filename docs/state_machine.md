# Machine à états — TrinovaRobot

```text
              begin() OK
                  │
                  ▼
               [Idle]
                  │
                  │
        ┌─────────┴─────────┐
        │                   │
        │                   │
        ▼                   ▼
     [Moving]          [EmergencyStop]
        │                   ▲
        │                   │
        └───────► ◄─────────┘
        emergencyStop()

                  │
                  │ clearEmergencyStop()
                  ▼
               [Idle]

begin() FAIL ──────────────► [Error]


## Règles

- `EmergencyStop` a priorité absolue : atteignable depuis n'importe quel état, à tout moment.
- Seul `clearEmergencyStop()` permet de sortir de `EmergencyStop` — aucune commande de mouvement (locale ou MQTT) n'est acceptée tant que ce n'est pas fait explicitement.
- `Error` est un état terminal pour la session courante (échec `begin()`, ex: driver moteur absent). 