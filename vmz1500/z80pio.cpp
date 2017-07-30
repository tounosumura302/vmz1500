//
//  z80pio.cpp
//  vmz1500
//
//  Created by murasuke on 2017/05/05.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#include "z80pio.hpp"

/*
    Z80 PIO port
 */

Z80PioPort::Z80PioPort()
{
    reset();
}

void Z80PioPort::reset()
{
    _data=0;
    
    _intmask=0;
    _mode=MODE1;
    _intctrl=0;
    _dataio=0;
    
    _mode3next=false;
    _intmasknext=false;
}

void Z80PioPort::setCtrl(Byte b)
{
    if (_mode3next){
        _mode3next=false;
        _dataio=b;
        return;
    }
    if (_intmasknext){
        _intmasknext=false;
        _intmask=b^0xff;    //割り込みマスクは、ポートの設定値を反転させておく（1=割り込みがかかるビット）
        return;
    }
    
    if ((b&0x01)==0){
        _intvector=b;
        return;
    }
    if ((b&0x0f)==0x0f){
        _mode=(Mode)(b&0xc0);
        _mode3next=(bool)(_mode==MODE3);
        return;
    }
    if ((b&0x0f)==0x07){
        _intctrl=b&0xe0;
        _intmasknext=(bool)(b&MASKFOLLOW);
        return;
    }
    if ((b&0x0f)==0x03){
        _intctrl=(b&0x80)|(_intctrl&0x7f);
        return;
    }
    return;
}

void Z80PioPort::setData(Byte value,Byte mask)
{
    _data=(_data&(!mask))|(value&mask); //変更したいビットのみ変更
}

Byte Z80PioPort::data() const
{
    return _data;
}

bool Z80PioPort::interrupt() const
{
    if (!(_intctrl&IntCtrlMask::INT))   return false;
    
    Byte dt=_data;
    if (!(_intctrl&IntCtrlMask::HIGHLOW)){  //low
        dt=dt^0xff;
    }
    dt&=_intmask;
    if (_intctrl&IntCtrlMask::ANDOR){   //and
        return (dt&_intmask)==_intmask;
    }
    return dt&_intmask; //or
}

Byte Z80PioPort::intVector() const
{
    return _intvector;
}

void Z80PioPort::reti()
{
    if (_intctrl&IntCtrlMask::HIGHLOW){  //high
        _data&=(_intmask^0xff);    //highの場合は、割り込みビットを0にする
    }
    else{
        _data|=_intmask;            //lowの場合は、割り込みビットを1にする
    }

}

/*
    Z80 PIO
 */

Z80Pio::Z80Pio()
{
}

void Z80Pio::reset()
{
    _port[0].reset();
    _port[1].reset();
    
    _interrupt=-1;
}

void Z80Pio::setCtrlA(Byte b)
{
    _port[0].setCtrl(b);
}

void Z80Pio::setCtrlB(Byte b)
{
    _port[1].setCtrl(b);
}

void Z80Pio::setDataA(Byte value,Byte mask)
{
    _port[0].setData(value,mask);
}

void Z80Pio::setDataB(Byte value,Byte mask)
{
    _port[1].setData(value,mask);
}

Byte Z80Pio::dataA() const
{
    return _port[0].data();
}

Byte Z80Pio::dataB() const
{
    return _port[1].data();
}

bool Z80Pio::interrupt()
{
    if (_interrupt>=0)  return false;
    if (_port[1].interrupt()){
        _interrupt=1;
        return true;
    }
    if (_port[0].interrupt()){
        _interrupt=0;
        return true;
    }
    return false;
}

Byte Z80Pio::intVector() const
{
    if (_interrupt<0)   return 0;
    return _port[_interrupt].intVector();
}

void Z80Pio::reti()
{
    if (_interrupt<0)   return;
    _port[_interrupt].reti();
    _interrupt=-1;
}