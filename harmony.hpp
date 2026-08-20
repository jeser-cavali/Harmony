#pragma once
#define MINIAUDIO_IMPLEMENTATION

#include "miniaudio.h"

#include <atomic>
#include <queue>
#include <string>

class HarmonyEngine{
    private:
        // Miniaudio related properties
        ma_decoder_config _decoderConfig;
        ma_device_config _deviceConfig;

        ma_decoder decoder;
        ma_device device;

        std::atomic<bool> isDecoderInitiated;
        std::atomic<bool> isDeviceInitiated;

        // Unique properties
        std::queue<std::string> musicQueue;

        std::atomic<bool> isMusicFinished;
        std::atomic<bool> isMusicPaused;
    public:
};