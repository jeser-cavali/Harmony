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

        static void dataCallback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount){

            HarmonyEngine* pHarmony = (HarmonyEngine*)pDevice->pUserData;
            if(pHarmony == NULL) return;

            ma_uint64 framesRead = 0;
            ma_data_source_read_pcm_frames(&pHarmony->decoder, pOutput, frameCount, &framesRead);

            if(framesRead < frameCount){
                pHarmony->isMusicFinished = true;
            }

            (void)pInput;
        };



        // Unique properties
        std::queue<std::string> musicQueue;

        std::atomic<bool> isMusicFinished;
        std::atomic<bool> isMusicPaused;
    public:
        HarmonyEngine(){
            _decoderConfig = ma_decoder_config_init(ma_format_f32, 2, 44100);

            _deviceConfig = ma_device_config_init(ma_device_type_playback);
            _deviceConfig.playback.format = ma_format_f32;
            _deviceConfig.playback.channels = 2;
            _deviceConfig.sampleRate = 44100;
            _deviceConfig.dataCallback = this->dataCallback;
            _deviceConfig.pUserData = this;

            isDecoderInitiated = false;
            isDeviceInitiated = false;

            isMusicFinished = false;
            isMusicPaused = false;
        };
        ~HarmonyEngine(){
            if(!isDeviceInitiated){
                ma_device_uninit(&device);
                isDeviceInitiated = false;
            }

            if(!isDecoderInitiated){
                ma_decoder_uninit(&decoder);
                isDecoderInitiated = false;
            }
        };
        
};