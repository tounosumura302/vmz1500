//
//  vmz1500sound.cpp
//  vmz1500
//
//  Created by murasuke on 2017/05/10.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#include "vmz1500sound.hpp"
#include <algorithm>
#include <iostream>

/*
    wave generator
*/

WaveGenerator::WaveGenerator()
{
}

/*
    square wave generator
*/

SqWavGen::SqWavGen() : WaveGenerator()
{}

SqWavGen::~SqWavGen()
{
    //chunkは Mix_FreeChunk() で解放する
    std::for_each(_chunks.begin(), _chunks.end(), [](std::pair<int, Mix_Chunk*> i){
        Mix_FreeChunk(i.second);
    });
}

bool SqWavGen::init()
{
    _chunks.clear();
    return true;
}

Mix_Chunk* SqWavGen::waveChunk(int n)
{
    //##
    if (n<2)    return nullptr;
    
    
    try{
        return _chunks.at(n);   //at() は、要素がない場合例外を発生
    }
    //要素がない場合
    catch(std::out_of_range&){
        //duty比50の矩形波
        //振幅を大きくしすぎると、音が刺々しくなるのでほどほどに
        Uint8 *buf=new Uint8[n];
        int i;
        for(i=0;i<n/2;i++){
            buf[i]=160;
        }
        for(;i<n;i++){
            buf[i]=96;
        }
        _chunks[n]=Mix_QuickLoad_RAW(buf, n);   //operator[] は、要素がない場合は作成
        //delete [] buf;
        return _chunks[n];
    }
}

/*
    PSG
*/

int Vmz1500Psg::_volvalue[]={128,102,81,64,51,40,32,26,20,16,13,10,8,6,5,0};

Vmz1500Psg::Vmz1500Psg(int basechl)
{
    _sqwav=new SqWavGen();
    _openaudio=false;
    _basechl=basechl;
    
}

Vmz1500Psg::~Vmz1500Psg()
{
    if (_openaudio){
        Mix_CloseAudio();
    }
    delete _sqwav;
}

bool Vmz1500Psg::init()
{
    for(int i=0;i<CHANNEL_NUM;i++){
        _prevfreq[i]=0;
    }

    if (_openaudio){
        Mix_CloseAudio();
    }
    _openaudio=(Mix_OpenAudio(OUTPUT_FREQ, AUDIO_U8, 1, 1024)==0);
    if (!_openaudio)    return false;
    return _openaudio;
}

void Vmz1500Psg::setTone(int chl, int freq)
{
    if (freq==_prevfreq[chl])   return;
    _prevfreq[chl]=freq;
    Mix_Chunk* chunk=_sqwav->waveChunk((freq*32*OUTPUT_FREQ)/INPUT_FREQ);
    if (chunk){
        Mix_PlayChannel(_basechl+chl, chunk, -1);
    }
}

void Vmz1500Psg::setVolume(int chl, int vol)
{
    //## volが範囲内かチェックしてない
    Mix_Volume(_basechl+chl, _volvalue[vol]);
}

void Vmz1500Psg::setTone8253(int freq)
{
    if (freq==_prevfreq[8]){
        Mix_Volume(_basechl+8, MIX_MAX_VOLUME);
        return;        
    }

    

    Mix_Chunk* chunk=_sqwav->waveChunk((freq*OUTPUT_FREQ)/INPUT_FREQ_8253);
    if (chunk){
        Mix_PlayChannel(_basechl+8, chunk, -1);
        //if (_prevfreq[8]==0){
            Mix_Volume(_basechl+8, MIX_MAX_VOLUME);
        //}
        _prevfreq[8]=freq;
    }
}

void Vmz1500Psg::setVolume8253(int vol)
{
    Mix_Volume(_basechl+8, _volvalue[vol]);
}

/*
*/

Vmz1500SndDrv::Vmz1500SndDrv()
{
    _psg=nullptr;
}

Vmz1500SndDrv::~Vmz1500SndDrv()
{
    if (_psg)   delete _psg;
    Mix_Quit();
}

bool Vmz1500SndDrv::init()
{
    if (_psg)   delete _psg;
    _psg=new Vmz1500Psg(0);
    Mix_Init(0);
    if (!_psg->init())  return false;

    Mix_AllocateChannels(Vmz1500Psg::CHANNEL_NUM);
    Mix_ReserveChannels(Vmz1500Psg::CHANNEL_NUM);

    return true;
}

Vmz1500Psg& Vmz1500SndDrv::psg()
{
    return *_psg;
}
