//
//  i8253.hpp
//  vmz1500
//
//  Created by murasuke on 2017/04/29.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#ifndef i8253_hpp
#define i8253_hpp


#include "type.h"

/*
 * 8253 channel
 */
class I8253Cnt
{
public:
    I8253Cnt();
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
    
    void setGate(bool gate);
    
    void setControl(Byte v);
    
    void write(Byte v);
    Byte read();
    
    bool count();
    
    bool out() const;
    bool edge();
    
    bool gate() const;
    Mode mode() const;
    Word rate() const;
    bool isUpdated() const;
    
private:
    Word _count;
    Word _rate;
    RLMode _rlmode;
    bool _latch;
    Mode _mode;
    CountMode _countmode;
    bool _high;
    bool _need2load;
    
    bool _gate;
    Word _latchedcnt;
    bool _out;
    bool _edge;
    
    bool _updatecnt;
};

/*
 * 8253
 */

class I8253
{
public:
    I8253();
    ~I8253();
    
    void reset();
    
    void setGate(Byte cnt,bool gate);
    
    void setControl(Byte v);
    
    void write(Byte cnt,Byte v);
    Byte read(Byte cnt);
    
    bool count(Byte cnt);
    
//    virtual void sync(long clock)=0;
    
    bool out(Byte cnt) const;
    bool edge(Byte cnt);
    
    
    bool gate(Byte cnt) const;
    I8253Cnt::Mode mode(Byte cnt) const;
    Word rate(Byte cnt) const;
    bool isUpdated(Byte cnt) const;
    
    //bool interrupt() const;
    
protected:
    I8253Cnt _ch[3];
};



#endif /* i8253_hpp */
