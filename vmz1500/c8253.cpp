//
//  c8253.cpp
//  vmz1500
//
//  Created by murasuke on 2017/04/06.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#include <iostream>
#include "c8253.hpp"

C8253Ch::C8253Ch()
{}

void C8253Ch::reset()
{
    _count=0;
    _rate=0;
    _rlmode=LOWBYTE;    //##初期値不明
    _latch=false;       //##初期値不明
    _mode=MODE0;
    _countmode=BIN;
    _high=false;
    _need2load=false;
}

void C8253Ch::set(Byte v)
{
    //RL1,RL0(Read/Load)の指定で、LATCH(=0)の場合、前回までの設定は保存されるらしい？？
    RLMode rlm=(RLMode)(v&0x30);
    if (rlm==LATCH){
        _latch=true;
        return;
    }
    else{
        _latch=false;
        _rlmode=rlm;
        if (_rlmode==LOWHIGH){
            _high=false;
        }
    }
    
    _mode=(Mode)(v&0x0e);
    if (_mode>MODE5){
        _mode=(Mode)(_mode&0x06);    //110 -> 010 / 111 -> 011
    }
    _countmode=(CountMode)(v&0x01);
    //## _high=false;
}

void C8253Ch::write(Byte value)
{
    RLMode tmp=_rlmode;
    if (tmp==LOWHIGH){
        if (!_high){
            tmp=LOWBYTE;
            _high=true;
            _need2load=false;
        }
        else{
            tmp=HIGHBYTE;
            _high=false;
            _need2load=true;
        }
        /*
        tmp=_high ? HIGHBYTE : LOWBYTE;
        _high=_high ? false : true; //switch flag
         */
    }
    else{
        _need2load=true;
    }
    
    switch(tmp){
        case LATCH: //##what's latch operation??
            break;
            
        case LOWBYTE:
            _rate=(_rate&0xff00)|((Word)value); //下位８ビットのみ更新
//            _count=_rate;
            break;
            
        case HIGHBYTE:
            _rate=(_rate&0x00ff)|((Word)value<<8); //上位８ビットのみ更新
//            _count=_rate;
            break;
    }
}

Byte C8253Ch::read()
{
    RLMode tmp=_rlmode;
    if (tmp==LOWHIGH){
        tmp=_high ? HIGHBYTE : LOWBYTE;
        _high=_high ? false : true; //switch flag
    }
    
    switch(tmp){
        case LOWBYTE:
            return(_count&0x00ff);
            
        case HIGHBYTE:
            return((_count&0xff00)>>8);
    }
    return(0);
}

//カウントダウン
bool C8253Ch::count()
{
    if (_need2load){
        _count=_rate;
        _need2load=false;
        return false;
    }
    
//    if (_rate==0)   return(false);
    if (_count==0)   return(false);
    
    _count--;
    if (_count>0)   return false;
    //## _mode によって処理を変える
    //## MODE0=割り込み発生
    //## MODE1=終了
    //## MODE2=繰り返し
    //## それ以外はよくわからん
    _count=_rate;
//    _count++;
//    if (_count<=_rate)  return(false);
    
//    _count=0;
    return(true);
}



/*
 * 8253
 */



C8253::C8253(long clock)
{
    reset(clock);
}

C8253::~C8253()
{}

void C8253::reset(long clock)
{
    _ch[0].reset();
    _ch[1].reset();
    _ch[2].reset();
    
//    _ch1clockrate=clock/31250;  //31.25kHz
    _ch1clockrate=clock/15700;  //15.7kHz    mz-700以降はこのクロックらしい（hblankと同じ？）
    _ch1clockcount=0;
    
    _timerinterrupt=false;
    
    initTempo(clock);
}

void C8253::set(Byte v)
{
    Byte ch=(v>>6)&0x03;
    _ch[ch].set(v);
    
    //##
    std::cout<<"8253 ctrl out "<<(int)ch<<" "<<std::hex<<(int)v<<std::endl;
    //    qDebug()<<"8253 set";
}

void C8253::write(Byte ch, Byte v)
{
    //##
    std::cout<<"8253 ch out "<<(int)ch<<" "<<std::hex<<(int)v<<std::endl;

    _ch[ch&0x03].write(v);
    if (ch==2){
        _timerinterrupt=false;
    }

    //    qDebug()<<"8253 write";
}

Byte C8253::read(Byte ch)
{
    //##
    std::cout<<"8253 in "<<std::hex<<(int)ch<<std::endl;

    return(_ch[ch&0x03].read());
}


void C8253::sync(long clock)
{
    _timerinterrupt=false;

    _ch1clockcount+=clock;
    if (_ch1clockcount<_ch1clockrate)   return;
    _ch1clockcount-=_ch1clockrate;

    if (_ch[1].count()){
        if (_ch[2].count()){
            _timerinterrupt=true;
            //qDebug()<<"interrupt!";
        }
    }
    
    
    /*
     _timerinterrupt=false;
    if (_ch[1].count()){
        if (_ch[2].count()){
            _timerinterrupt=true;
            //qDebug()<<"interrupt!";
        }
    }
     */
}

//interruption occured?
bool C8253::interrupt() const{return(_timerinterrupt);}


//tempo
//## これであっているのかどうか不明

void C8253::initTempo(long clock)
{
    _tempoclock=(clock/32)/2;       //## 32Hzらしいので
    _tempocount=_tempoclock;
    _tempobit=false;
}

void C8253::syncTempo(long clock)
{
    _tempocount-=clock;
    if (_tempocount<=0){
        _tempocount=_tempoclock;
        _tempobit=!_tempobit;
    }
}

bool C8253::tempo() const
{
    return _tempobit;
}