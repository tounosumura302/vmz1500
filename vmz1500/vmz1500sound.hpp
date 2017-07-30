//
//  vmz1500sound.hpp
//  vmz1500
//
//  Created by murasuke on 2017/05/10.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#ifndef vmz1500sound_hpp
#define vmz1500sound_hpp

//#include <stdio.h>
#include <SDL2_mixer/SDL_mixer.h>
#include <map>

/*
    wave generator
 */
class WaveGenerator
{
public:
    WaveGenerator();
    virtual ~WaveGenerator()=default;

    virtual bool init()=0;
    virtual Mix_Chunk* waveChunk(int n)=0;
    
};

/*
    square wave generator
 */
class SqWavGen : public WaveGenerator
{
public:
    SqWavGen();
    ~SqWavGen();
    
    bool init();
    
    Mix_Chunk* waveChunk(int n);
    
private:
    std::map<int, Mix_Chunk*> _chunks;
};

/*
    PSG
 */
class Vmz1500Psg
{
public:
    static const long INPUT_FREQ=3579545;   //入力周波数(3.579545MHz)
    static const long OUTPUT_FREQ=22050;    //出力周波数（44100HzだとCD音質）
    static const int CHANNEL_NUM=9;            //必要なチャネル数（２つのPSGのチャネル数とi8253のサウンドの合計）

    static const long INPUT_FREQ_8253=895000;   //入力周波数(895KHz)

public:
    Vmz1500Psg(int basechl);
    Vmz1500Psg(const Vmz1500Psg&)=delete;   //コピーコンストラクタ禁止
    ~Vmz1500Psg();
    
    bool init();
    
    void setTone(int chl,int freq);
    void setVolume(int chl,int vol);
    
    void setTone8253(int freq);
    void setVolume8253(int vol);
    
private:
    SqWavGen* _sqwav;
    bool _openaudio;
    int _basechl;
    
    int _prevfreq[CHANNEL_NUM];
    
    static const int MAX_VOLUME=16;
    static int _volvalue[MAX_VOLUME];
};



/*
 */

class Vmz1500SndDrv
{
public:
    Vmz1500SndDrv();
    ~Vmz1500SndDrv();
    
    bool init();
    
    Vmz1500Psg& psg();
    
private:
    Vmz1500Psg* _psg;
};

#endif /* vmz1500sound_hpp */
