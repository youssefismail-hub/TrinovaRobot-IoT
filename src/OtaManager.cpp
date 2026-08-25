#include "OtaManager.h"
#include <HTTPUpdate.h>
#include <WiFiClient.h>
#include "Logger.h"
#include "DiagnosticsService.h"

static const char* FW_VERSION = "1.0.0-iot";

const char* OtaManager::currentVersion() {
    return FW_VERSION;
}

void OtaManager::performUpdate(const String& url) {
    Logger::log(LogLevel::Warn, "OTA", "Demarrage mise a jour OTA");

    WiFiClient client; 
    httpUpdate.rebootOnUpdate(true); // redémarre automatiquement en cas de succès

    t_httpUpdate_return result = httpUpdate.update(client, url);

    switch (result) {
        case HTTP_UPDATE_FAILED:
            Logger::log(LogLevel::Error, "OTA", httpUpdate.getLastErrorString().c_str());
            DiagnosticsService::incrementErrorCount();
            break;
        case HTTP_UPDATE_NO_UPDATES:
            Logger::log(LogLevel::Info, "OTA", "Aucune mise a jour disponible");
            break;
        case HTTP_UPDATE_OK:
            Logger::log(LogLevel::Info, "OTA", "Mise a jour OK, redemarrage...");
            break;
    }
}