#pragma once
#include <cstdint>

// Centralise l'état de santé du système : raison du dernier reboot, heap libre,
// compteur d'erreurs. 
class DiagnosticsService {
public:
    static void begin();
    static void logBootReason();
    static uint32_t freeHeap();
    static void incrementErrorCount();
    static uint32_t errorCount();

private:
    static uint32_t _errorCount;
};