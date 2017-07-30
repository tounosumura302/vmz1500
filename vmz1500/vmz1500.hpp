//
//  vmz1500.hpp
//  vmz1500
//
//  Created by murasuke on 2017/03/28.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#ifndef vmz1500_hpp
#define vmz1500_hpp

#include <map>
#include <memory>
#include <mutex>
#include "memory.hpp"
#include "sequeue.hpp"
#include "z80.hpp"
//#include "c8253.hpp"
#include "i8253.hpp"
#include "vmz1500qd.hpp"
#include "z80pio.hpp"
#include "sn76489.hpp"

#include <SDL2/SDL_keycode.h>

/*
    memory (memory mapped i/o & rom)
 */
class Vmz1500;
class MemoryBlockIORom : public MemoryBlock
{
public:
    MemoryBlockIORom(int size);
    ~MemoryBlockIORom();

    void init(Vmz1500 &vm);
    
    const Byte at(int i) const;
    void replace(int i, const Byte &value);
    
private:
    //memory mapped i/o
    Z80IOPort *_ioport;
};

/*
 * memory of mz1500 architecture
 */

using MemoryBlockShrPtr=std::shared_ptr<MemoryBlock>;
using MemoryBlockConstShrPtr=std::shared_ptr<const MemoryBlock>;

class Memz1500 : public Memory
{
public:
    //    MZ1500Mem(Mz1500 *mz1500);
    Memz1500();
    ~Memz1500();

    //reset
    void reset(Vmz1500 &vm);

    //initialize cg rom (##experimetal)
    void initRom(const ByteArray &cgrom);
    
    //load image
//    bool load(Word address,const std::string &filename);
    void load(Word address,const ByteArray &image,int offset=0,int size=0);
    
public:
    //memory access
    const Byte at(int i) const;
    void replace(int i,const Byte & value);
    
    //bank switching
    void switchE0(void);
    void switchE1(void);
    void switchE2(void);
    void switchE3(void);
    void switchE4(void);
    void switchE5(Byte data);
    void switchE6(void);
//    void switchMemory(Word ioport,Byte data);
    
    //vram direct access
    const MemoryBlock* vram() const;
    const MemoryBlock* cgrom() const;
    //    MemoryBlockConstShrPtr cgrom() const;
    const MemoryBlock* pcgBlue() const;
    const MemoryBlock* pcgRed() const;
    const MemoryBlock* pcgGreen() const;
    
    /*
     signals:
     void messageSent(const QString &) const;
     void violationOccured();
     */
    
    
private:
    //memory table
    typedef struct{
        MemoryBlock *memblk;
        int base;
    }
    Memtable;
    Memtable _memtbl[16];
    
    //memory blocks
    //0000-0fff
    MemoryBlock *_dram0;    //0000-0fff
    MemoryBlockRom *_monitor;  //0000-0fff
    
    //1000-cfff
    MemoryBlock *_dram1;    //1000-cfff
    
    //d000-ffff
    MemoryBlock *_dram2;    //d000-ffff
    
    MemoryBlock *_vram;     //d000-dfff
    MemoryBlockIORom *_iorom;    //e000-ffff
    
    MemoryBlockRom *_cgrom;    //d000-dfff(e000-efff)
    //    MemoryBlockShrPtr _cgrom;
    
    MemoryBlock *_pcgb;     //d000-efff
    MemoryBlock *_pcgr;     //d000-efff
    MemoryBlock *_pcgg;     //d000-efff
    
    MemoryBlockRom *_undef; //undefined block
    
    Word _ioaddress;    //io port address most currently specified;
    typedef enum{RAM,ROM,CGROM,PCG_B,PCG_R,PCG_G} BankMode;
    BankMode _mode_0000;
    BankMode _mode_d000;
};



/*
    keyboard
 */

//キーコードはSDLのものととりあえず共通
using Vmz1500KeyCode=SDL_Keycode;

class Vmz1500Kbd
{
public:
    Vmz1500Kbd();
    virtual ~Vmz1500Kbd()=default;
    
    void init();
    
    void press(const Vmz1500KeyCode &key);      //thread safe
    void release(const Vmz1500KeyCode &key);    //thread safe
    
    
    //get keycode
    Byte key() const;
    
    //set strobe
    void setStrobe(int s);
    
protected:
    typedef struct{
        short strobe;
        Byte code;
    }   KeyCode;
    std::map<int,KeyCode> _keytable;
    
    Byte _matrix[10];
    
    //キーボードマトリクスのストローブ
    int _strobe;
    
    //念のためにmutableにしておく（constメンバ変数でロックをかけることもあるかもしれないので）
    mutable std::mutex _mutex;
};


/*
    8253
 */

/*
class Vmz1500I8253 : public I8253
{
public:
    Vmz1500I8253();
    
    void reset(long clock);
    
    void sync(long clock);
};
*/

/*
    tempo control
 */

class Vmz1500Tempo
{
public:
    Vmz1500Tempo(long _baseclock);
    
    void reset(long clock);
    void sync(long clock);
    bool tempo() const;

private:
    long _baseclock;
    long _tempocount;
    long _tempoclock;
    bool _tempobit;
};


/*
    vmz1500
 */

//vmz1500

/*
typedef enum{
    EV_NULL=0,
    EV_KEYPRESS,
    EV_KEYRELEASE,
}
    Vmz1500EvType;

typedef struct{
    Vmz1500EvType _type;
    long _parm1;
}
    Vmz1500Event;

using Vmz1500EventQ=SEQueue<Vmz1500Event>;
*/

typedef struct{
    Uint8 r;
    Uint8 g;
    Uint8 b;
}
    Vmz1500Palette;

using I8253Cnt0SoundFunc=std::function<void(int)>;

class Vmz1500
{
public:
    Vmz1500(long clock);
    virtual ~Vmz1500();
    
    //initialize
    bool init();
    bool loadCgrom(const std::string& filename);
    bool load(const std::string &filename,Word address,Word offset=0,Word size=0);
    //bool load(Word address,const std::string &filename);
    //##experimental
    bool loadTapeImage(const std::string &filename,Word &execadr);
    void jump(Word address);
    
    //##QD
    bool setQD(const std::string &filename);
    
public:
    typedef enum{
        VSYNC=      0b00000001,
        PSG1=       0b00000010,
        PSG2=       0b00000100,
        I8253SOUND= 0b00001000,
    }
    Result;
    //
    void runInThread(void);
    int run(void);
    void stop(void);
    bool isOk(void) const;
    
protected:
    bool sync(int clock);
    long _baseclock;
    long _clockcounter;
    long _clock_raster;
    int _raster;


    /*
     * 8255
     */
private:
    bool _hblank,_vblank;   //blanking status
    bool _intmsk;           //8253 ch2 interrupt mask
public:
    void setIntmsk(bool m); //set intmsk
    bool isVBlank() const;
    bool isHBlank() const;

    /*
     * 8253
     */
private:
    //C8253 *_8253;   //8253
    I8253 *_i8253;  //8253
    
    long _i8253cnt0clock;

    
public:
    Byte read8253Ch(Byte ch);
    void write8253Ch(Byte ch,Byte value);
    void write8253Ctrl(Byte value);
    
    void set8253Gate(Byte cnt,Byte value);

private:
    bool _i8253cnt0mask;
    bool _i8253sndupdate;
public:
    void set8253Cnt0Mask(bool mask);
    bool is8253SoundUpdated() const;
    void procI8253Sound(I8253Cnt0SoundFunc func);
    
    /*
     * PSG(SN76489)
     */
    Sn76489 *_psg1,*_psg2;
    void procPsg1(Sn76489DivRatioFunc funcd,Sn76489VolumeFunc funcv);
    void procPsg2(Sn76489DivRatioFunc funcd,Sn76489VolumeFunc funcv);

    
    /*
     * tempo
     */
public:
    Vmz1500Tempo *_tempo3;
    bool tempoBit() const;
    
    /*
        cursor blink
     */
public:
    Vmz1500Tempo *_cursorblink;
    bool cursorVisible() const;
    
    
    /*
        Z80 PIO
    */
private:
    Z80Pio *_pio;
    
    
public:
    //interface for user interface
    //CRT
    const Byte cgromValue(Word offset) const;

    const Byte pcgBValue(Word offset) const;
    const Byte pcgRValue(Word offset) const;
    const Byte pcgGValue(Word offset) const;
    
    const Byte vramValue(Word offset) const;
    const Vmz1500Palette textPalette(Byte cc) const;
    void setPalette(Byte ix,Byte cc);
    
    //keyboard
    void pressKey(const Vmz1500KeyCode &key);
    void releaseKey(const Vmz1500KeyCode &key);


    //memory mapped i/o のためにfriendにしておく
    friend class MemoryBlockIORom;

    //priority
private:
    bool _pcgpriority; //false=text>pcg>background
    bool _pcgvisible;  //false=don't display pcg
    
public:
    void setPcgPriority(bool prty);
    void setPcgVisible(bool visible);
    bool pcgPriority() const;
    bool pcgVisible() const;

    
protected:
    Memz1500 *_mem;
//    Vmz1500EventQ _evq;
    
    Vmz1500Kbd *_kbd;

    //memory mapped i/o
    Z80IOPort *_ioport;
    
    Z80 *_cpu;

    Vmz1500QD *_qd;
    
    
    typedef enum{
        OK,
        STOP,
        HUNG
    }
    Status;

    Status _status;


    Vmz1500Palette _textpalette[8];
};



#endif /* vmz1500_hpp */
