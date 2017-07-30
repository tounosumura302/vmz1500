//
//  i8253.cpp
//  vmz1500
//
//  Created by murasuke on 2017/04/29.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#include <iostream>
#include "i8253.hpp"

/*
    8253 channel
 */

I8253Cnt::I8253Cnt()
{}

void I8253Cnt::reset()
{
    _count=0;
    _rate=0;
    _rlmode=LOWBYTE;    //##初期値不明
    _latch=false;       //##初期値不明
    _mode=MODE0;
    _countmode=BIN;
    _high=false;
    _need2load=false;
    
    _gate=false;
    _latchedcnt=0;
    _out=false;
    _edge=false;
    
    _updatecnt=false;
}

void I8253Cnt::setGate(bool gate)
{
    //rising(low->high)
    if (!_gate && gate){
        switch (_mode) {
                //## MODE1 未実装
            case MODE2:
                _count=_rate;
                break;
        }
    }
    else{
        //low
        if (!gate){
            switch (_mode) {
                case MODE2:
                case MODE3:
                    _edge=!_out;
                    _out=true;
            }
        }
    }
    _gate=gate;
}



void I8253Cnt::setControl(Byte v)
{
    //RL1,RL0(Read/Load)の指定で、LATCH(=0)の場合、前回までの設定は保存されるらしい？？
    RLMode rlm=(RLMode)(v&0x30);
    if (rlm==LATCH){
        _latch=true;
        _latchedcnt=_count;
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
    
    //モード変更後の初期値設定
    _edge=(!_out && _mode!=MODE0);
    _out=(_mode==MODE0) ? false : true;
    
    //##
    //0x36 をOUTした場合、音が止まるようなので、rateをクリアの上更新フラグをセット
    if (_mode==MODE3){
        _rate=0;
        _count=0;
        _updatecnt=true;
    }
    
    _countmode=(CountMode)(v&0x01);
}

void I8253Cnt::write(Byte value)
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
        case LOWBYTE:
            _rate=(_rate&0xff00)|((Word)value); //下位８ビットのみ更新
            //_count=_rate;
            break;
            
        case HIGHBYTE:
            _rate=(_rate&0x00ff)|((Word)value<<8); //上位８ビットのみ更新
            //_count=_rate;
            break;
    }
    
    if (_mode==MODE0){
        _out=false;
        _edge=false;
    }
}

Byte I8253Cnt::read()
{
    RLMode tmp=_rlmode;
    if (tmp==LOWHIGH){
        tmp=_high ? HIGHBYTE : LOWBYTE;
        _high=_high ? false : true; //switch flag
    }
    
    switch(tmp){
        case LOWBYTE:
            return _count&0x00ff;
//            return _latch ? _latchedcnt&0x00ff : _count&0x00ff;
            
        case HIGHBYTE:
            return (_count&0xff00)>>8;
//            return _latch ? (_latchedcnt&0xff00)>>8 : (_count&0xff00)>>8;
    }
    return(0);
}

//カウントダウン
bool I8253Cnt::count()
{
    if (_need2load){
        _count=_rate;
        _need2load=false;
        _updatecnt=true;
        return false;
    }
    _updatecnt=false;

    if (_count==0){
        _edge=false;
        return _out;
    }

    bool _prev=_out;
    
    //## mode1,4,5 は未実装
    switch (_mode) {
        case MODE0:
            if (!_gate){
                _out=false;
            }
            else{
                _count--;
                _out=(_count==0);   //0なら常に out=1
            }
            break;
            
        case MODE2:
            if (!_gate){
                //_out=false;
            }
            else{
                _count--;
                if (_count==0){
                    _out=true;
                    _need2load=true;
                    //_count=_rate;
                }
                else{
                    _out=false;
                }
            }
            break;
            
        case MODE3:
            if (!_gate){
                //_out=false;
            }
            else{
                _count--;
                if (_count>=_rate/2){
                    _out=true;
                }
                else{
                    _out=false;
                    if (_count==0){
                        _need2load=true;
                        //_count=_rate;
                    }
                }
            }
            break;
            
        case MODE4: //##
            if (!_gate){
                _out=false;
            }
            else{
                _count--;
            }
            break;
            
        default:
            _count--;
            break;
    }
    
    //立ち上がりの検知
    _edge=(_out && !_prev) ? true : false;
    
    return _out;
}

bool I8253Cnt::out() const
{
    return _out;
}

bool  I8253Cnt::edge()
{
    bool r=_edge;
    _edge=false;
    return r;
}

bool I8253Cnt::gate() const
{
    return _gate;
}

I8253Cnt::Mode I8253Cnt::mode() const
{
    return _mode;
}

Word I8253Cnt::rate() const
{
    return _rate;
}

bool I8253Cnt::isUpdated() const
{
    return _updatecnt;
}

/*
 * 8253
 */



I8253::I8253()
{}

I8253::~I8253()
{}

void I8253::reset()
{
    _ch[0].reset();
    _ch[1].reset();
    _ch[2].reset();
}

void I8253::setGate(Byte cnt, bool gate)
{
    _ch[cnt].setGate(gate);
}

void I8253::setControl(Byte v)
{
    Byte ch=(v>>6)&0x03;
    _ch[ch].setControl(v);
    
    //##
    std::cout<<"8253 ctrl out "<<(int)ch<<" "<<std::hex<<(int)v<<std::endl;
    //    qDebug()<<"8253 set";
}

void I8253::write(Byte cnt, Byte v)
{
    //##
    std::cout<<"8253 ch out "<<(int)cnt<<" "<<std::hex<<(int)v<<std::endl;
    
    _ch[cnt&0x03].write(v);
}

Byte I8253::read(Byte cnt)
{
    //##
    std::cout<<"8253 in "<<std::hex<<(int)cnt<<std::endl;
    
    return(_ch[cnt&0x03].read());
}

bool I8253::count(Byte cnt)
{
    return _ch[cnt].count();
}

bool I8253::out(Byte cnt) const
{
    return _ch[cnt].out();
}

bool I8253::edge(Byte cnt)
{
    return _ch[cnt].edge();
}

bool I8253::gate(Byte cnt) const
{
    return _ch[cnt].gate();
}

I8253Cnt::Mode I8253::mode(Byte cnt) const
{
    return _ch[cnt].mode();
}

Word I8253::rate(Byte cnt) const
{
    return _ch[cnt].rate();
}

bool I8253::isUpdated(Byte cnt) const
{
    return _ch[cnt].isUpdated();
}

