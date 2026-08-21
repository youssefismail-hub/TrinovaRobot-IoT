#pragma once
#include <cstdint>

// Enveloppe autour du watchdog matériel ESP-IDF : si une tâche enregistrée
// ne "feed" pas dans le délai imparti, l'ESP32 redémarre (protection contre blocage).
class WatchdogService {
public:
    static bool begin(uint32_t timeoutSeconds);
    static void registerCurrentTask();
    static void feed();
};