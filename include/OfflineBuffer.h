#pragma once
#include <Arduino.h>

struct OfflineEvent {
    uint32_t timestampMs;
    char     level[8];
    char     tag[24];
    char     message[64];
};

// Ring buffer borné en RAM (taille fixe = CAPACITY) : conserve les événements
// critiques survenus hors connexion MQTT et les republie au retour du réseau.
// Volontairement borné pour ne jamais faire grossir le heap sans limite (risque de fragmentation).
class OfflineBuffer {
public:
    static constexpr size_t CAPACITY = 20;

    static void begin();
    static void push(const char* level, const char* tag, const char* message);
    static size_t count();
    static bool pop(OfflineEvent& outEvent); // FIFO
};