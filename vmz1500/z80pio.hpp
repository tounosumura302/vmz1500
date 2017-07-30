//
//  z80pio.hpp
//  vmz1500
//
//  Created by murasuke on 2017/05/05.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#ifndef z80pio_hpp
#define z80pio_hpp

#include "type.h"
#include <stdio.h>

/*
    Z80 PIO port
 */

class Z80PioPort
{
public:
    Z80PioPort();
    ~Z80PioPort()=default;
    
    void reset();
    
    void setCtrl(Byte b);
    
    void setData(Byte value,Byte mask=0xff);   //maskは変更したいビットを 1 にする
    
    Byte data() const;
    
    bool interrupt() const;
    Byte intVector() const;
    void reti();
    
    typedef enum{
        MODE0=0b00000000,
        MODE1=0b01000000,
        MODE2=0b10000000,
        MODE3=0b11000000
    }
    Mode;
    
    typedef enum{
        INT         =0b10000000,
        ANDOR       =0b01000000,
        HIGHLOW     =0b00100000,
        MASKFOLLOW  =0b00010000
    }
    IntCtrlMask;
    
protected:
    Byte _data;
    
    Byte _intvector;    //interrupt vector
    Mode _mode;         //operationg mode
    Byte _intctrl; //interrupt control word
    Byte _dataio;       //data bus input/output(1=input)
    Byte _intmask;      //interrupt mask(0=interrupt monitored)
    
    bool _mode3next;    //
    bool _intmasknext;  //
};

/*
    Z80 PIO
 */

class Z80Pio
{
public:
    Z80Pio();
    ~Z80Pio()=default;
    
    void reset();
    
    void setCtrlA(Byte b);
    void setCtrlB(Byte b);
    
    void setDataA(Byte value,Byte mask=0xff);
    void setDataB(Byte value,Byte mask=0xff);
    
    Byte dataA() const;
    Byte dataB() const;
    
    bool interrupt();
    Byte intVector() const;
    void reti();
    
protected:
    Z80PioPort _port[2];
    
    int _interrupt; //割り込み源となっているポート（0=A , 1=B）
};

#endif /* z80pio_hpp */
