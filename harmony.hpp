#pragma once

#include "miniaudio.h"

#include <iostream>
#include <atomic>
#include <queue>
#include <string>
#include <filesystem>
#include <thread>

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

        bool isValidFiletype(std::string filepath){
            return (
                filepath.contains(".mp3") ||
                filepath.contains(".wav") ||
                filepath.contains(".flac") ||
                filepath.contains(".ogg")
            );
        };

        bool initDecoder(std::string filepath){
            if(ma_decoder_init_file(filepath.c_str(), &_decoderConfig, &decoder) != MA_SUCCESS){
                return false;
            }
            isDecoderInitiated = true;
            isMusicFinished = false;
            return true;
        };
        void uninitDecoder(){
            if(isDecoderInitiated){
                ma_decoder_uninit(&decoder);
                isDecoderInitiated = false;
            }
        };

        bool initDevice(){
            if(isDecoderInitiated){
                if(ma_device_init(NULL, &_deviceConfig, &device) != MA_SUCCESS){
                    uninitDecoder();
                    return false;
                }
                isDeviceInitiated = true;
                return true;
            }
            return false;
        };

        void uninitDevice(){
            if(isDeviceInitiated){
                ma_device_uninit(&device);
                isDeviceInitiated = false;
            }
        };

        void pauseDevice(){
            isMusicPlaying = false;
            ma_device_stop(&device);
        }

        void startDevice(){
            if(isDeviceInitiated && isDecoderInitiated){
                if(ma_device_start(&device) != MA_SUCCESS){
                    uninitDevice();
                    uninitDecoder();
                    return;
                }

                isMusicPlaying = true;

                while(!isMusicFinished){
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                }

                musicQueue.pop();
                
                if(!musicQueue.empty()){
                    pauseDevice();
                    uninitDecoder();
                } else{
                    uninitDevice();
                    uninitDecoder();
                }
            }
        }

        // Unique properties
        std::queue<std::string> musicQueue;

        std::atomic<bool> loop;
        std::atomic<bool> random;

        std::atomic<bool> isMusicPlaying;
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

            loop = false;
            random = false;

            isMusicPlaying = false;
            isMusicFinished = false;
            isMusicPaused = false;
        };
        ~HarmonyEngine(){
            if(isMusicPlaying){
                ma_device_stop(&device);
                isMusicPlaying = false;

                ma_device_uninit(&device);
                isDeviceInitiated = false;

                ma_decoder_uninit(&decoder);
                isDecoderInitiated = false;
            } else{
                if(isDeviceInitiated){
                    ma_device_uninit(&device);
                    isDeviceInitiated = false;
                }

                if(isDecoderInitiated && !isDeviceInitiated){
                    ma_decoder_uninit(&decoder);
                    isDecoderInitiated = false;
                }
            }
        };
        void play(){
            if(!isMusicPlaying && !musicQueue.empty()){
                while(!musicQueue.empty()){
                    if(!isDecoderInitiated){
                        initDecoder(musicQueue.front());
                    }

                    if(!isDeviceInitiated){
                        initDevice();
                    }

                    if(isDecoderInitiated && isDeviceInitiated){
                        startDevice();
                    }
                }
            }
        };
        void play(std::string url){
            // TODO: add custom error behaviour
            // Adding music to queue
            if(std::filesystem::exists(url)){
                if(std::filesystem::is_regular_file(url) && isValidFiletype(url)){
                    musicQueue.emplace(url);
                } else if(std::filesystem::is_directory(url)){
                    for(const auto& file : std::filesystem::directory_iterator(url)){
                        if(isValidFiletype(file.path().string())){
                            musicQueue.emplace(file.path().string());
                        }
                    }
                }
            }

            play();
        };
        void pause(){
            if(isMusicPlaying){
                pauseDevice();
            }
        };
        void skip(){
            isMusicFinished = true;
        }
        void stop(){

            pause();

            isMusicFinished = true;

            uninitDevice();
            uninitDecoder();

            while(!musicQueue.empty()){
                musicQueue.pop();
            }
        }
        
};