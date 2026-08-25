#include "OfflineBuffer.h"
#include <cstring>
#include <freertos/FreeRTOS.h>
#include <freertos/semphr.h>

static OfflineEvent      g_buffer[OfflineBuffer::CAPACITY];
static size_t            g_head = 0;
static size_t            g_count = 0;
static SemaphoreHandle_t g_mutex;

void OfflineBuffer::begin() {
    g_mutex = xSemaphoreCreateMutex();
}

void OfflineBuffer::push(const char* level, const char* tag, const char* message) {
    if (xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) != pdTRUE) return;

    size_t writeIndex = (g_head + g_count) % CAPACITY;
    if (g_count == CAPACITY) {
        // Buffer plein : on écrase le plus ancien (FIFO avec perte contrôlée, jamais de dépassement)
        g_head = (g_head + 1) % CAPACITY;
    } else {
        g_count++;
    }

    OfflineEvent& ev = g_buffer[writeIndex];
    ev.timestampMs = millis();
    strncpy(ev.level, level, sizeof(ev.level) - 1);     ev.level[sizeof(ev.level) - 1] = '\0';
    strncpy(ev.tag, tag, sizeof(ev.tag) - 1);           ev.tag[sizeof(ev.tag) - 1] = '\0';
    strncpy(ev.message, message, sizeof(ev.message) - 1); ev.message[sizeof(ev.message) - 1] = '\0';

    xSemaphoreGive(g_mutex);
}

size_t OfflineBuffer::count() {
    return g_count;
}

bool OfflineBuffer::pop(OfflineEvent& outEvent) {
    if (xSemaphoreTake(g_mutex, pdMS_TO_TICKS(50)) != pdTRUE) return false;
    if (g_count == 0) {
        xSemaphoreGive(g_mutex);
        return false;
    }
    outEvent = g_buffer[g_head];
    g_head = (g_head + 1) % CAPACITY;
    g_count--;
    xSemaphoreGive(g_mutex);
    return true;
}