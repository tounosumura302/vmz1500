//
//  c8253.hpp
//  vmz1500
//
//  Created by murasuke on 2017/04/06.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#ifndef c8253_hpp
#define c8253_hpp

#include "type.h"

/*
 * 8253 channel
 */
class C8253Ch
{
public:
    C8253Ch();
    void reset();
    
    typedef enum{
        LATCH   =0b00000000,
        LOWBYTE =0b00010000,
        HIGHBYTE=0b00100000,
        LOWHIGH =0b00110000
    }
    RLMode;
    
    typedef enum{
        MODE0   =0b00000000,
        MODE1   =0b00000010,
        MODE2   =0b00000100,
        MODE3   =0b00000110,
        MODE4   =0b00001000,
        MODE5   =0b00001010,
    }
    Mode;
    
    typedef enum{
        BIN=0,
        BCD=1
    }
    CountMode;
    
    void set(Byte v);
    
    void write(Byte value);
    Byte read();
    
    bool count();
    
private:
    Word _count;
    Word _rate;
    RLMode _rlmode;
    bool _latch;
    Mode _mode;
    CountMode _countmode;
    bool _high;
    bool _need2load;
};

/*
 * 8253
 */

class C8253
{
public:
    C8253(long clock);
    ~C8253();
    
    void reset(long clock);
    
    void set(Byte v);
    
    void write(Byte ch,Byte v);
    Byte read(Byte ch);
    
    void sync(long clock);
    
    bool interrupt() const;
    
private:
    C8253Ch _ch[3];
    
    long _ch1clockcount;
    long _ch1clockrate;
    
    bool _timerinterrupt;

    //tempo timer
public:
    void initTempo(long clock);
    void syncTempo(long clock);
    bool tempo() const;
private:
    long _tempocount;
    long _tempoclock;
    bool _tempobit;
};

#endif /* c8253_hpp */
