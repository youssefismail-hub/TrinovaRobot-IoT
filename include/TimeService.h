#pragma once

// Synchronisation NTP obligatoire avant toute connexion TLS (le certificat
// serveur est validé par date -> une horloge ESP32 à zéro (1970) fait
// échouer systématiquement la poignée de main TLS.

class TimeService {
public:
    static bool syncNow(uint32_t timeoutMs = 10000);
};