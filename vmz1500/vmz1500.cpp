//
//  vmz1500.cpp
//  vmz1500
//
//  Created by murasuke on 2017/03/28.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#include <iostream>
#include <fstream>

#include "utility.hpp"
#include "vmz1500.hpp"


/*
 * memory block (memory mapped I/O + ROM)
 */


MemoryBlockIORom::MemoryBlockIORom(int size) : MemoryBlock(size)
{
    _ioport=new Z80IOPort();
}
 
MemoryBlockIORom::~MemoryBlockIORom()
{
    delete _ioport;
}

void MemoryBlockIORom::init(Vmz1500 &vm)
{
    //memory mapped i/o
    _ioport->init();
    
    //0xe000
    //##
    _ioport->setPeripheral(0,
                           []{return 0;},
                           [&](Byte v){vm._kbd->setStrobe(v&0x0f);}
                           );
    //0xe001
    //##
    _ioport->setPeripheral(1,
                           [&]{return vm._kbd->key();},
                           [](Byte v){}
                           );
    //0xe002
    //##
    _ioport->setPeripheral(2,
                           [&]{return (vm.isVBlank() ? 0x80 : 0)|(vm.cursorVisible() ? 0x40 : 0);},
                           [&](Byte v){
                               vm.setIntmsk((bool)(v&0x04));
                               vm.set8253Cnt0Mask((bool)(v&0x01));
                           }
                           );
    //0xe003
    //##
    _ioport->setPeripheral(3,
                           []{return 0;},
                           [&](Byte v){
                               if (v&0x80){/*mode set*/
                               }
                               else{    /* port c bit set/reset */
                                   switch (v&0x0e) {
                                       case 4:
                                           vm.setIntmsk((bool)(v&0x01));
                                           break;
                                       case 0:
                                           vm.set8253Cnt0Mask((bool)(v&0x01));
                                           break;
                                   }
                               }
                           }
                           );
    //0xe004
    //##
    _ioport->setPeripheral(4,
                           [&]{return vm.read8253Ch(0);},
                           [&](Byte v){vm.write8253Ch(0, v);}
                           );
    //0xe005
    //##
    _ioport->setPeripheral(5,
                           [&]{return vm.read8253Ch(1);},
                           [&](Byte v){vm.write8253Ch(1, v);}
                           );
    //0xe006
    //##
    _ioport->setPeripheral(6,
                           [&]{return vm.read8253Ch(2);},
                           [&](Byte v){vm.write8253Ch(2, v);}
                           );
    //0xe007
    //##
    _ioport->setPeripheral(7,
                           []{return 0;},
                           [&](Byte v){vm.write8253Ctrl(v);}
                           );
    //0xe008
    //##
    _ioport->setPeripheral(8,
                           [&]{
                               return (vm.isHBlank() ? 0x80 : 0)|(vm.tempoBit() ? 1 : 0);
                           },
                           [&](Byte v){
                               vm.set8253Gate(0,v&0x01);
                           }
                           );
    /*
     if (i<0x800){
     switch(i){
     case 0:
     case 1:
     //            emit messageSent(QString("mem I/O IN : %1\n").arg(i,2,16));
     return(_mz1500->requestKeyboard(i));
     
     case 2:
     //e002h
     //            emit messageSent(QString("mem I/O IN : %1\n").arg(i,2,16));
     return((!_mz1500->isVBlank() ? 0x80 : 0));
     
     case 4:
     return(_mz1500->read8253Ch(0));
     
     case 5:
     return(_mz1500->read8253Ch(1));
     
     case 6:
     return(_mz1500->read8253Ch(2));
     
     case 8:
     //e008h
     return((!_mz1500->isHBlank() ? 0x80 : 0));
     
     default:
     emit messageSent(QString("mem I/O IN : %1\n").arg(i,2,16));
     emit violationOccured();
     }
     }
     return(MemoryBlock::at(i));
     */
}

 const Byte MemoryBlockIORom::at(int i) const
 {
     //0xe000-0xe7ff はmemory mapped i/o
     if (i<0x800){
         return _ioport->in(i);
     }
     //0xe800- は普通のメモリ
     return(MemoryBlock::at(i));

 }
 
 
void MemoryBlockIORom::replace(int i, const Byte &value)
{
    //0xe000-0xe7ff はmemory mapped i/o
    if (i<0x800){
        //######
        if (i==0 && (value&0x0f)==6){
            int b=000;
        }
        
        return _ioport->out(i, value);
    }
    //0xe800- は普通のメモリ
    //ROMなので何もしない
    return;
    /*
 if (i<0x800){
 switch(i){
 case 0:
 case 1:
 _mz1500->receiveKeyboard(i,value);
 break;
 case 2:
 qDebug()<<QString("8255 port c %1").arg(value,1,16);
 break;
 case 3:
 //            qDebug()<<QString("8255 control %1").arg(value,1,16);
 if (value&0x80){
 //mode set
 }
 else{
 //                qDebug()<<QString("8255 control %1").arg(value,1,16);
 //INTMSK
 if (value&0x04){
 _mz1500->setIntmsk(value&0x01);
 }
 else{
 //data recorder
 }
 }
 break;
 case 4:
 _mz1500->write8253Ch(0,value);
 //            qDebug()<<QString("8253 ch0 counter %1").arg(value,1,16);
 break;
 case 5:
 _mz1500->write8253Ch(1,value);
 //            qDebug()<<QString("8253 ch1 counter %1").arg(value,1,16);
 break;
 case 6:
 _mz1500->write8253Ch(2,value);
 //            qDebug()<<QString("8253 ch2 counter %1").arg(value,1,16);
 break;
 case 7:
 _mz1500->write8253Ctrl(value);
 //            qDebug()<<QString("8253 control %1").arg(value,1,16);
 break;
 
 case 8:
 //            qDebug()<<QString("8253 gate %1").arg(value,1,16);
 //emit messageSent(QString("mem I/O OUT : %1 %2\n").arg(i,4,16).arg(value,2,16));
 break;
 
 default:
 emit messageSent(QString("mem I/O OUT : %1 %2\n").arg(i,4,16).arg(value,2,16));
 //emit violationOccured();
 
 }
 }
     */
}
 


/*
    memory of mz1500 architecture
 */
Memz1500::Memz1500() : Memory()
{
    _dram0=new MemoryBlock(0x1000);
    _monitor=new MemoryBlockRom(0x1000);
    
    _dram1=new MemoryBlock(0xc000);
    
    _dram2=new MemoryBlock(0x3000);
    
    _vram=new MemoryBlock(0x1000);
    _iorom=new MemoryBlockIORom(0x2000);

    //    _iorom=nullptr;
    
    _cgrom=new MemoryBlockRom(0x1000);
    //    _cgrom.reset(new MemoryBlockRom(0x1000));
    //    std::cout<<_cgrom.use_count()<<std::endl;
    
    _pcgb=new MemoryBlock(0x2000);
    _pcgr=new MemoryBlock(0x2000);
    _pcgg=new MemoryBlock(0x2000);
    
    _undef=new MemoryBlockRom(0x1000);
    
}

/****** to be implemented in the future
 Memz1500::Memz1500(Mz1500 *mz1500) : Memory()
 {
 _dram0=new MemoryBlock(0x1000);
 _monitor=new MemoryBlock(0x1000);
 
 _dram1=new MemoryBlock(0xc000);
 
 _dram2=new MemoryBlock(0x3000);
 
 _vram=new MemoryBlock(0x1000);
 _iorom=new MemoryBlockIORom(0x2000,mz1500);
 
 _cgrom=new MemoryBlockRom(0x1000);
 _pcgb=new MemoryBlock(0x2000);
 _pcgr=new MemoryBlock(0x2000);
 _pcgg=new MemoryBlock(0x2000);
 
 _undef=new MemoryBlockRom(0x1000);
 
 }
 */

Memz1500::~Memz1500()
{
    delete _dram0;
    delete _monitor;
    delete _dram1;
    delete _dram2;
    delete _vram;
    if (_iorom) delete _iorom;
    delete _cgrom;
    //    std::cout<<_cgrom.use_count()<<std::endl;
    delete _pcgb;
    delete _pcgr;
    delete _pcgg;
    delete _undef;
}

void Memz1500::initRom(const ByteArray &cgrom)
{
    _cgrom->setImage(cgrom);
}

void Memz1500::load(Word address, const ByteArray &image,int offset,int size)
{
    //##offsetなどの値のチェックをぜんぜんしてない
    int fsize=(size==0 ? image.size() : size);
    int es;
    for(int index=(address&0xf000)>>12;fsize>0&&index<16;index++){
        es=0x1000-(address&0x0fff);
        _memtbl[index].memblk->load(address-_memtbl[index].base,image,offset,es);
        fsize-=es;
        offset+=es;
        address+=es;
    }
}



void Memz1500::reset(Vmz1500 &vm)
{
    //assign block
    _memtbl[ 0].memblk=_monitor; _memtbl[ 0].base=0x0000;
    _memtbl[ 1].memblk=_dram1;   _memtbl[ 1].base=0x1000;
    _memtbl[ 2].memblk=_dram1;   _memtbl[ 2].base=0x1000;
    _memtbl[ 3].memblk=_dram1;   _memtbl[ 3].base=0x1000;
    _memtbl[ 4].memblk=_dram1;   _memtbl[ 4].base=0x1000;
    _memtbl[ 5].memblk=_dram1;   _memtbl[ 5].base=0x1000;
    _memtbl[ 6].memblk=_dram1;   _memtbl[ 6].base=0x1000;
    _memtbl[ 7].memblk=_dram1;   _memtbl[ 7].base=0x1000;
    _memtbl[ 8].memblk=_dram1;   _memtbl[ 8].base=0x1000;
    _memtbl[ 9].memblk=_dram1;   _memtbl[ 9].base=0x1000;
    _memtbl[10].memblk=_dram1;   _memtbl[10].base=0x1000;
    _memtbl[11].memblk=_dram1;   _memtbl[11].base=0x1000;
    _memtbl[12].memblk=_dram1;   _memtbl[12].base=0x1000;
    _memtbl[13].memblk=_vram;    _memtbl[13].base=0xd000;
    _memtbl[14].memblk=_iorom;   _memtbl[14].base=0xe000;
    _memtbl[15].memblk=_iorom;   _memtbl[15].base=0xe000;
    
    _ioaddress=0xe3;
    _mode_0000=ROM;
    _mode_d000=ROM;
    
    /*
     //message
     connect(_iorom,SIGNAL(messageSent(QString)),this,SIGNAL(messageSent(QString)));
     //memory violation
     connect(_iorom,SIGNAL(violationOccured()),this,SIGNAL(violationOccured()));
     */
    
    _iorom->init(vm);
    
    //##test
    //vram初期化　なんでもいいから文字を表示させたいため
    /*
    Byte v=0;
    for(int i=0;i<40*25;i++){
        _vram->replace(i, v);
        v++;
    }
     */
    
    
}

/*
 * memory access
 */
const Byte Memz1500::at(int i) const
{
    //##
    if (i==0x4c45){
        int b=000;
    }
    
    
    int s=(i&0xf000)>>12;
    return(_memtbl[s].memblk->at(i-_memtbl[s].base));
}

void Memz1500::replace(int i, const Byte &value)
{
    /*
    //##
//    if (i==0xd100 && value==0x01){
    if (i==0x0000){
        std::cout<<"!! 0000="<<std::hex<<(int)value<<std::endl;
    }
    if (i==0x0038 && value==0xfd){
        std::cout<<"!!!!!!!!!"<<std::endl;
    }
    if (i==0xbc0f && value<0x80){
        std::cout<<"!! bc0f="<<std::hex<<(int)value<<std::endl;
    }
    */
    
    int s=(i&0xf000)>>12;
    _memtbl[s].memblk->replace(i-_memtbl[s].base,value);
}

/*
 * bank switching
 */

void Memz1500::switchE0()
{
    //0x0000-0x0fff = dram
    _memtbl[ 0].memblk=_dram0; _memtbl[ 0].base=0x0000;
    _mode_0000=RAM;
}

void Memz1500::switchE1()
{
    //PCGバンクになっている場合は、この時点ではバンク切り替えをしない
    //PCGバンクを閉じたときにDRAMになるように、記録だけしておく
    switch (_mode_d000) {
        case CGROM:
        case PCG_B:
        case PCG_G:
        case PCG_R:
            _ioaddress=0xe1;
            return;
    }
    //0xd000-0xffff = dram
    _memtbl[13].memblk=_dram2;  _memtbl[13].base=0xd000;
    _memtbl[14].memblk=_dram2;  _memtbl[14].base=0xd000;
    _memtbl[15].memblk=_dram2;  _memtbl[15].base=0xd000;
    _ioaddress=0xe1;
    _mode_d000=RAM;
}

void Memz1500::switchE2()
{
    //0x0000-0x0fff = monitor rom
    _memtbl[ 0].memblk=_monitor; _memtbl[ 0].base=0x0000;
    _mode_0000=ROM;
}

void Memz1500::switchE3()
{
    //PCGバンクになっている場合は、この時点ではバンク切り替えをしない
    //PCGバンクを閉じたときにVRAMになるように、記録だけしておく
    switch (_mode_d000) {
        case CGROM:
        case PCG_B:
        case PCG_G:
        case PCG_R:
            _ioaddress=0xe3;
            return;
    }

    //0xd000-0xffff = vram/io/rom
    _memtbl[13].memblk=_vram;    _memtbl[13].base=0xd000;
    _memtbl[14].memblk=_iorom;   _memtbl[14].base=0xe000;
    _memtbl[15].memblk=_iorom;   _memtbl[15].base=0xe000;
    _ioaddress=0xe3;
    _mode_d000=ROM;
}

void Memz1500::switchE4()
{
    //0x0000-0x0fff = monitor rom  0xd000-0xffff = vram/io/rom
    _memtbl[ 0].memblk=_monitor; _memtbl[ 0].base=0x0000;
    _memtbl[13].memblk=_vram;    _memtbl[13].base=0xd000;
    _memtbl[14].memblk=_iorom;   _memtbl[14].base=0xe000;
    _memtbl[15].memblk=_iorom;   _memtbl[15].base=0xe000;
    _ioaddress=0xe4;
    _mode_0000=ROM;
    _mode_d000=ROM;
}

void Memz1500::switchE5(Byte data)
{
    //pcg
    switch(data&3){
        case 0 :
            _memtbl[13].memblk=_cgrom;  _memtbl[13].base=0xd000;
            _memtbl[14].memblk=_cgrom;  _memtbl[14].base=0xe000;
            _memtbl[15].memblk=_undef;  _memtbl[15].base=0xf000;
            _mode_d000=CGROM;
            break;
        case 1 :
            _memtbl[13].memblk=_pcgb;   _memtbl[13].base=0xd000;
            _memtbl[14].memblk=_pcgb;   _memtbl[14].base=0xd000;
            _memtbl[15].memblk=_undef;  _memtbl[15].base=0xf000;
            _mode_d000=PCG_B;
            break;
        case 2 :
            _memtbl[13].memblk=_pcgr;   _memtbl[13].base=0xd000;
            _memtbl[14].memblk=_pcgr;   _memtbl[14].base=0xd000;
            _memtbl[15].memblk=_undef;  _memtbl[15].base=0xf000;
            _mode_d000=PCG_R;
            break;
        case 3 :
            _memtbl[13].memblk=_pcgg;   _memtbl[13].base=0xd000;
            _memtbl[14].memblk=_pcgg;   _memtbl[14].base=0xd000;
            _memtbl[15].memblk=_undef;  _memtbl[15].base=0xf000;
            _mode_d000=PCG_G;
            break;
    }
}

void Memz1500::switchE6()
{
    switch (_ioaddress) {
        case 0xe1:
            //0xd000-0xffff = dram
            _memtbl[13].memblk=_dram2;  _memtbl[13].base=0xd000;
            _memtbl[14].memblk=_dram2;  _memtbl[14].base=0xd000;
            _memtbl[15].memblk=_dram2;  _memtbl[15].base=0xd000;
            _ioaddress=0xe1;
            _mode_d000=RAM;
            break;

        case 0xe3:
        case 0xe4:
            //0xd000-0xffff = vram/io/rom
            _memtbl[13].memblk=_vram;    _memtbl[13].base=0xd000;
            _memtbl[14].memblk=_iorom;   _memtbl[14].base=0xe000;
            _memtbl[15].memblk=_iorom;   _memtbl[15].base=0xe000;
            _ioaddress=0xe3;
            _mode_d000=ROM;
            break;
    }
}


/*
 * vram direct access
 */
const MemoryBlock* Memz1500::vram() const{return(_vram);}
const MemoryBlock* Memz1500::cgrom() const{return(_cgrom);}
//MemoryBlockConstShrPtr Memz1500::cgrom() const{return _cgrom;}
const MemoryBlock* Memz1500::pcgBlue() const{return(_pcgb);}
const MemoryBlock* Memz1500::pcgRed() const{return(_pcgr);}
const MemoryBlock* Memz1500::pcgGreen() const{return(_pcgg);}



/*
    vmz1500
 */

Vmz1500::Vmz1500(long clock)
{
    _mem=new Memz1500();
//    _evq.init(100);
    
    _kbd=new Vmz1500Kbd();
    
    _ioport=new Z80IOPort();
    
    _pio=new Z80Pio();
    
    _cpu=new Z80(_mem,_ioport,_pio);
    
    //_8253=new C8253(clock);
    _i8253=new I8253(); //##
    
    _psg1=new Sn76489();
    _psg2=new Sn76489();
    
    _tempo3=new Vmz1500Tempo(clock);
    _cursorblink=new Vmz1500Tempo(clock*2); //1.5Hzだと整数にならないので、２倍にする。
    
    _qd=new Vmz1500QD();
    
    _baseclock=clock;
    _status=STOP;
}

Vmz1500::~Vmz1500()
{
    delete _qd;
    delete _cursorblink;
    delete _tempo3;
    delete _psg1;
    delete _psg2;
    delete _i8253;  //##
    //delete _8253;
    delete _cpu;
    delete _pio;
    delete _ioport;
    delete _mem;
    delete _kbd;
}

bool Vmz1500::init()
{
    
    _cpu->reset();
    
    _mem->reset(*this);
//    _evq.clear();
    _kbd->init();
    
    _ioport->init();
    
    //_8253->reset(_baseclock);
    
    //8253
    _i8253->reset();
    _i8253->setGate(0, false);
    _i8253->setGate(1, true);
    _i8253->setGate(2, true);
    _i8253cnt0clock=0;
    
    //8253 counter0 -> sound
    _i8253cnt0mask=false;
    _i8253sndupdate=false;
    
    //psg
    _psg1->init();
    _psg2->init();
    
    //timer
    _tempo3->reset(32); //32Hz
    _cursorblink->reset(1.5*2); //1.5Hz
    
    _qd->init();
    
    _pio->reset();

    //I/Oポートの初期化
    _ioport->setPeripheral(0xe0,
                           []{return 0;},
                           [&](Byte v){_mem->switchE0();}
//                           [&](Byte v){std::cout<<"bank switch e0\n";_mem->switchE0();}
                           );
    _ioport->setPeripheral(0xe1,
                           []{return 0;},
                           [&](Byte v){_mem->switchE1();}
//                           [&](Byte v){std::cout<<"bank switch e1\n";_mem->switchE1();}
                           );
    _ioport->setPeripheral(0xe2,
                           []{return 0;},
                           [&](Byte v){_mem->switchE2();}
//                           [&](Byte v){std::cout<<"bank switch e2\n";_mem->switchE2();}
                           );
    _ioport->setPeripheral(0xe3,
                           []{return 0;},
                           [&](Byte v){_mem->switchE3();}
//                           [&](Byte v){std::cout<<"bank switch e3\n";_mem->switchE3();}
                           );
    _ioport->setPeripheral(0xe4,
                           []{return 0;},
                           [&](Byte v){_mem->switchE4();}
//                           [&](Byte v){std::cout<<"bank switch e4\n";_mem->switchE4();}
                           );
    _ioport->setPeripheral(0xe5,
                           []{return 0;},
                           [&](Byte v){_mem->switchE5(v);}
//                           [&](Byte v){std::cout<<"bank switch e5\n";_mem->switchE5(v);}
                           );
    _ioport->setPeripheral(0xe6,
                           []{return 0;},
                           [&](Byte v){_mem->switchE6();}
//                           [&](Byte v){std::cout<<"bank switch e6\n";_mem->switchE6();}
                           );
    //priority
    _ioport->setPeripheral(0xf0,
                           []{return 0;},
                           [&](Byte v){this->setPcgPriority(v&0x2);this->setPcgVisible(v&1);}
                           );
    //palette
    _ioport->setPeripheral(0xf1,
                           []{return 0;},
                           [&](Byte v){this->setPalette((v&0x70)>>4, v&0x07);}
                           );
    //PSG
    //##
    _ioport->setPeripheral(0xf2,
                           []{return 0;},
                           [&](Byte v){_psg1->set(v);}
                           );
    _ioport->setPeripheral(0xf3,
                           []{return 0;},
                           [&](Byte v){_psg2->set(v);}
                           );
    _ioport->setPeripheral(0xe9,
                           []{return 0;},
                           [&](Byte v){_psg1->set(v);_psg2->set(v);}
                           );
    //QD
    _ioport->setPeripheral(0xf6,
                           [&]{return _qd->sioACtrl();},
                           [&](Byte v){_qd->setSioACtrl(v);}
                           );
    _ioport->setPeripheral(0xf7,
                           [&]{return _qd->sioBCtrl();},
                           [&](Byte v){_qd->setSioBCtrl(v);}
                           );
    _ioport->setPeripheral(0xf4,
                           [&]{return _qd->sioAData();},
                           [&](Byte v){_qd->setSioAData(v);}
                           );
    _ioport->setPeripheral(0xf5,
                           [&]{return _qd->sioBData();},
                           [&](Byte v){_qd->setSioBData(v);}
                           );

    //FDD(MZ-1E05)
    //##
    _ioport->setPeripheral(0xd8,
                           []{std::cout<<"i/o port d8h not supported";return 0;},
                           [](Byte v){std::cout<<"i/o port d8h not supported";}
                           );
    _ioport->setPeripheral(0xd9,
                           []{std::cout<<"i/o port d9h not supported";return 0;},
                           [](Byte v){std::cout<<"i/o port d9h not supported";}
                           );
    //MZ-1R12
    //##
    _ioport->setPeripheral(0xf8,
                           []{std::cout<<"i/o port f8h not supported"<<std::endl;return 0;},
                           [](Byte v){std::cout<<"i/o port f8h not supported"<<std::endl;}
                           );
    _ioport->setPeripheral(0xf9,
                           []{std::cout<<"i/o port f9h not supported"<<std::endl;return 0;},
                           [](Byte v){std::cout<<"i/o port f9h not supported"<<std::endl;}
                           );

    _ioport->setPeripheral(0xa8,
                           []{std::cout<<"i/o port a8h not supported"<<std::endl;return 0;},
                           [](Byte v){std::cout<<"i/o port a8h not supported"<<std::endl;}
                           );
    _ioport->setPeripheral(0xa9,
                           []{std::cout<<"i/o port a9h not supported"<<std::endl;return 0;},
                           [](Byte v){std::cout<<"i/o port a9h not supported"<<std::endl;}
                           );

    //Z80 PIO
    //##
    _ioport->setPeripheral(0xfc,
                           []{std::cout<<"i/o port fch IN"<<std::endl;return 0;},
                           [&](Byte v){
                               _pio->setCtrlA(v);
                               std::cout<<"i/o port fch OUT="<<std::hex<<(int)v<<std::endl;}
                           );
    _ioport->setPeripheral(0xfd,
                           []{std::cout<<"i/o port fdh IN"<<std::endl;return 0;},
                           [&](Byte v){
                               _pio->setCtrlB(v);
                               std::cout<<"i/o port fdh OUT="<<std::hex<<(int)v<<std::endl;}
                           );
    _ioport->setPeripheral(0xfe,
                           [&]{
                               std::cout<<"i/o port feh IN"<<std::endl;
                               return _pio->dataA();
                           },
                           [&](Byte v){std::cout<<"i/o port feh OUT="<<std::hex<<(int)v<<std::endl;
                               _pio->setDataA(v);
                           }
                           );
    _ioport->setPeripheral(0xff,
                           [&]{std::cout<<"i/o port ffh IN"<<std::endl;
                               return _pio->dataB();},
                           [&](Byte v){std::cout<<"i/o port ffh OUT="<<std::hex<<(int)v<<std::endl;
                               _pio->setDataB(v);
                           }
                           );

    
    
    //palette
    //                  R,G,B
    _textpalette[0]={000,000,000};
    _textpalette[1]={000,000,255};
    _textpalette[2]={255,000,000};
    _textpalette[3]={255,000,255};
    _textpalette[4]={000,255,000};
    _textpalette[5]={000,255,255};
    _textpalette[6]={255,255,000};
    _textpalette[7]={255,255,255};
    
    
    _clockcounter=0;

    //crt
    _clock_raster=0;
    _raster=0;
    _hblank=false;
    _vblank=false;
    
    //8255
    _intmsk=false;
    
    //status
    _status=OK;

    return true;
}


//## かなり適当
bool Vmz1500::loadCgrom(const std::string& filename)
{
    std::ifstream ifs;
    ifs.open(filename);
    if (!ifs.is_open()) return false;
    
    ByteArray buf(0x1000);
    ifs.read((char*)buf.data(), 0x1000);
    
    _mem->initRom(buf);
    
    return true;
}

bool Vmz1500::load(const std::string &filename, Word address,Word offset,Word size)
//bool Vmz1500::load(Word address, const std::string &filename)
{
    ByteArray buf;
    if (!loadBinFile(filename, buf))   return false;
    _mem->load(address, buf,offset,size);
    return true;
}

bool Vmz1500::loadTapeImage(const std::string &filename, Word &execadr)
{
    ByteArray mzt;
    if (!loadBinFile(filename, mzt))   return false;

    Word datasize=(Byte)mzt.at(0x12)+((Byte)mzt.at(0x13))*256;
    Word loadaddress=(Byte)mzt.at(0x14)+((Byte)mzt.at(0x15))*256;
    execadr=(Byte)mzt.at(0x16)|((Byte)mzt.at(0x17))*256;
    _mem->load(loadaddress, mzt,128,mzt.size()-128);
    
    return true;
}

//@@@@@@@@@@@ loaffile() でファイルをbytearrayにloadし、それをloadtapeimageとかloadで使うようにする（ファイルとバッファを分離）

/*
bool Vmz1500::loadFile(const std::string &filename, ByteArray &buf)
{
    std::ifstream ifs;
    ifs.open(filename);
    if (!ifs.is_open()) return false;
    
    //file size
    ifs.seekg(0, std::fstream::end);
    auto endp=ifs.tellg();
    ifs.clear();
    ifs.seekg(0, std::fstream::beg);
    auto begp=ifs.tellg();
    auto fsize=endp-begp;
    
    //##ファイルサイズの制限がない
    buf.reserve(fsize);
    buf.resize(fsize);
    ifs.read((char*)buf.data(), fsize);
    ifs.close();
    return true;
}
*/

void Vmz1500::jump(Word address)
{
    _cpu->jump(address);
}


/*
    QD
 */

bool Vmz1500::setQD(const std::string &filename)
{
    return _qd->setQD(filename);
}



/*
    CRT interface
 */



const Byte Vmz1500::cgromValue(Word offset) const
{
    return _mem->cgrom()->at(offset);
}

const Byte Vmz1500::pcgBValue(Word offset) const
{
    return _mem->pcgBlue()->at(offset);
}

const Byte Vmz1500::pcgRValue(Word offset) const
{
    return _mem->pcgRed()->at(offset);
}

const Byte Vmz1500::pcgGValue(Word offset) const
{
    return _mem->pcgGreen()->at(offset);
}

const Byte Vmz1500::vramValue(Word offset) const
{
    return _mem->vram()->at(offset);
}

const Vmz1500Palette Vmz1500::textPalette(Byte cc) const
{
    //## ccの範囲チェックしてない
    return _textpalette[cc];
}

void Vmz1500::setPalette(Byte ix, Byte cc)
{
    _textpalette[ix&7].b=cc&1 ? 255 : 0;
    _textpalette[ix&7].r=cc&2 ? 255 : 0;
    _textpalette[ix&7].g=cc&4 ? 255 : 0;
}



void Vmz1500::setPcgPriority(bool prty)
{
    _pcgpriority=prty;
}

void Vmz1500::setPcgVisible(bool visible)
{
    _pcgvisible=visible;
}

bool Vmz1500::pcgPriority() const
{
    return _pcgpriority;
}

bool Vmz1500::pcgVisible() const
{
    return _pcgvisible;
}

void Vmz1500::pressKey(const Vmz1500KeyCode &key)
{
    _kbd->press(key);
}

void Vmz1500::releaseKey(const Vmz1500KeyCode &key)
{
    _kbd->release(key);
}

//

void Vmz1500::stop()
{
    _status=STOP;
}

bool Vmz1500::isOk() const
{
    return _status==OK;
}

//全体の同期が取れるまでループ（垂直帰線期間を基準とする）
int Vmz1500::run()
{
    int clock;
    int r=0;
    while(_status==OK){
        clock=_cpu->exec();
        if (clock>=0){
            if (sync(clock)){
                r|=Vmz1500::VSYNC;
            }
        }
        if (_psg1->isUpdated()){
            r|=Vmz1500::PSG1;
        }
        if (_psg2->isUpdated()){
            r|=Vmz1500::PSG2;
        }
        if (_i8253sndupdate){
            r|=Vmz1500::I8253SOUND;
        }
        
        if (r>0)  return r;
    }
    return 0;
}

bool Vmz1500::sync(int clock)
{
    bool sync=false;
    
    //clock timing
    _clockcounter+=clock;
    if (_clockcounter>=_baseclock){
        _clockcounter-=_baseclock;
    }
    
    //8253 counter0  895kHz   (not 1.1088MHz)
    _i8253cnt0clock+=clock;
    if (_i8253cnt0clock>=_baseclock/895000){
        _i8253cnt0clock-=_baseclock/895000;
        _i8253->count(0);
        //## 8253 OUT0 は Z80PIOポートAに直結
        _pio->setDataA(_i8253->out(0) ? 0x10 : 0,0x10);
    }
    
    //tempo ##
    _tempo3->sync(clock);
    //cursor blink ##
    _cursorblink->sync(clock*2);
    
    //hblank/vblank
    _clock_raster+=clock;
    if (_hblank){
        //hblank
        if (_clock_raster>=63){
            //hblank is over
            _clock_raster-=63;
            _hblank=false;
            _raster++;
            if (_vblank && _raster>=262){
                //vblank is over too
                _raster=0;
                _vblank=false;
                sync=true;
            }
        }
    }
    //not hblank
    else{
        if (_clock_raster>=165){
            //enter hblank
            _clock_raster-=165;
            _hblank=true;
            if (!_vblank && _raster>=200){
                //enter vblank too
                _vblank=true;
            }
            //#####
            //_8253->sync(60*262);
            if (_i8253->count(1)){
                _i8253->count(2);
                //## 8253 OUT2 は Z80PIOポートAに直結
                _pio->setDataA(_i8253->out(0) ? 0x20 : 0,0x20);

            }

//            if (_i8253->out(2) && _intmsk){
            if (_i8253->edge(2) && _intmsk){
                _cpu->irq(0);
            }

            /*
            //8253 is synchronized with hblank
            _8253->sync();
            
            //8253 ch2 interrupt occured?
            if (_8253->interrupt() && _intmsk){
                _cpu->irq(0);
            }
             */
        }
    }   //if (_hblank)

    //8253
    /*
    _8253->sync(clock);    
    //8253 ch2 interrupt occured?
    if (_8253->interrupt() && _intmsk){
        _cpu->irq(0);
    }
     */

    return(sync);
}

/*
    8255
 */

bool Vmz1500::isVBlank() const
{
    return _vblank;
}

bool Vmz1500::isHBlank() const
{
    return _hblank;
}

void Vmz1500::setIntmsk(bool m)
{
    _intmsk=m;
}

/*
 * 8253
 */

Byte Vmz1500::read8253Ch(Byte ch)
{
    return _i8253->read(ch);
//    return(_8253->read(ch));
}

void Vmz1500::write8253Ch(Byte ch, Byte value)
{
    _i8253->write(ch, value);
    if (_i8253->isUpdated(0))   _i8253sndupdate=true;
//    _8253->write(ch,value);
}

void Vmz1500::write8253Ctrl(Byte value)
{
    _i8253->setControl(value);
    
    //##
    if (value==0x3a){
        int b;
        b=0;
    }
        
    if (_i8253->isUpdated(0))   _i8253sndupdate=true;
//    _8253->set(value);
}

void Vmz1500::set8253Gate(Byte cnt, Byte value)
{
    _i8253->setGate(cnt,value);
    //std::cout<<"## gate "<<(int)value<<std::endl;
    if (cnt==0) _i8253sndupdate=true;
}


void Vmz1500::set8253Cnt0Mask(bool mask)
{
    _i8253cnt0mask=mask;
    //std::cout<<"## mask "<<(int)(mask ? 1 : 0)<<std::endl;

    if (mask==false){
        int b;
        b=0;
    }
    _i8253sndupdate=true;
}

void Vmz1500::procI8253Sound(I8253Cnt0SoundFunc func)
{
    // gate0=0 || soundmask=0 なら0
    //カウンター０のモードが3以外なら 0
    //それ以外ならカウンター０の値を渡す
//    int f=(_i8253->gate(0) && _i8253cnt0mask && _i8253->mode(0)==I8253Cnt::MODE3) ? _i8253->rate(0) : 0;
    int f=(_i8253cnt0mask && ((_i8253->mode(0)==I8253Cnt::MODE3 && _i8253->gate(0)) || (_i8253->mode(0)==I8253Cnt::MODE5))) ? _i8253->rate(0) : 0;
//    int f=(_i8253->gate(0) && _i8253cnt0mask) ? _i8253->rate(0) : 0;
    func.operator()(f);
    _i8253sndupdate=false;
}

bool Vmz1500::tempoBit() const
{
//    return _8253->tempo();
    return _tempo3->tempo();
}

bool Vmz1500::cursorVisible() const
{
    return _cursorblink->tempo();
}




/*
    PSG
 */
void Vmz1500::procPsg1(Sn76489DivRatioFunc funcd, Sn76489VolumeFunc funcv)
{
    _psg1->procUpdatedDivRatio(funcd);
    _psg1->procUpdatedVolume(funcv);
    _psg1->resetUpdate();
}

void Vmz1500::procPsg2(Sn76489DivRatioFunc funcd, Sn76489VolumeFunc funcv)
{
    _psg2->procUpdatedDivRatio(funcd);
    _psg2->procUpdatedVolume(funcv);
    _psg2->resetUpdate();
}


/*
    tempo
 */

Vmz1500Tempo::Vmz1500Tempo(long baseclock)
{
    _baseclock=baseclock;
    reset(0);
}

void Vmz1500Tempo::reset(long clock)
{
    if (clock!=0){
        _tempoclock=_baseclock/clock;
    }
    else{
        _tempoclock=0;
    }
//    _tempoclock=(clock/32)/2;       //## 32Hzらしいので
    _tempocount=_tempoclock;
    _tempobit=false;

}

void Vmz1500Tempo::sync(long clock)
{
    _tempocount-=clock;
    if (_tempocount>_tempoclock/2){
        _tempobit=true;
    }
    else{
        if (_tempocount>0){
            _tempobit=false;
        }
        else{
            _tempocount+=_tempoclock;
        }
    }
    /*
    if (_tempocount<=0){
        _tempocount+=_tempoclock;
        _tempobit=!_tempobit;
    }
     */
}

bool Vmz1500Tempo::tempo() const
{
    return _tempobit;
}


/*
    keyboard
*/
Vmz1500Kbd::Vmz1500Kbd()
{
    
}

void Vmz1500Kbd::init()
{
    //key event code -> key matrix  convert table
    //## 一部不明なコードあり
//    _keytable[SDLK_Katakana]=(KeyCode){0,0b10000000};
//    _keytable[SDLK_Game]=    (KeyCode){0,0b01000000};
    _keytable[SDLK_EQUALS]=   (KeyCode){0,0b00100000};
//    _keytable[SDLK_Eisu_toggle]=(KeyCode){0,0b00010000};
    _keytable[SDLK_SEMICOLON]=(KeyCode){0,0b00000100};
    _keytable[SDLK_COLON]=   (KeyCode){0,0b00000010};
    _keytable[SDLK_RETURN]=   (KeyCode){0,0b00000001};
    
    _keytable[SDLK_y]=       (KeyCode){1,0b10000000};
    _keytable[SDLK_z]=       (KeyCode){1,0b01000000};
    _keytable[SDLK_AT]=      (KeyCode){1,0b00100000};
    _keytable[SDLK_LEFTPAREN]=(KeyCode){1,0b00010000};
    _keytable[SDLK_RIGHTPAREN]=(KeyCode){1,0b00001000};
    
    _keytable[SDLK_q]=       (KeyCode){2,0b10000000};
    _keytable[SDLK_r]=       (KeyCode){2,0b01000000};
    _keytable[SDLK_s]=       (KeyCode){2,0b00100000};
    _keytable[SDLK_t]=       (KeyCode){2,0b00010000};
    _keytable[SDLK_u]=       (KeyCode){2,0b00001000};
    _keytable[SDLK_v]=       (KeyCode){2,0b00000100};
    _keytable[SDLK_w]=       (KeyCode){2,0b00000010};
    _keytable[SDLK_x]=       (KeyCode){2,0b00000001};
    
    _keytable[SDLK_i]=       (KeyCode){3,0b10000000};
    _keytable[SDLK_j]=       (KeyCode){3,0b01000000};
    _keytable[SDLK_k]=       (KeyCode){3,0b00100000};
    _keytable[SDLK_l]=       (KeyCode){3,0b00010000};
    _keytable[SDLK_m]=       (KeyCode){3,0b00001000};
    _keytable[SDLK_n]=       (KeyCode){3,0b00000100};
    _keytable[SDLK_o]=       (KeyCode){3,0b00000010};
    _keytable[SDLK_p]=       (KeyCode){3,0b00000001};
    
    _keytable[SDLK_a]=       (KeyCode){4,0b10000000};
    _keytable[SDLK_b]=       (KeyCode){4,0b01000000};
    _keytable[SDLK_c]=       (KeyCode){4,0b00100000};
    _keytable[SDLK_d]=       (KeyCode){4,0b00010000};
    _keytable[SDLK_e]=       (KeyCode){4,0b00001000};
    _keytable[SDLK_f]=       (KeyCode){4,0b00000100};
    _keytable[SDLK_g]=       (KeyCode){4,0b00000010};
    _keytable[SDLK_h]=       (KeyCode){4,0b00000001};
    
    _keytable[SDLK_1]=       (KeyCode){5,0b10000000};
    _keytable[SDLK_2]=       (KeyCode){5,0b01000000};
    _keytable[SDLK_3]=       (KeyCode){5,0b00100000};
    _keytable[SDLK_4]=       (KeyCode){5,0b00010000};
    _keytable[SDLK_5]=       (KeyCode){5,0b00001000};
    _keytable[SDLK_6]=       (KeyCode){5,0b00000100};
    _keytable[SDLK_7]=       (KeyCode){5,0b00000010};
    _keytable[SDLK_8]=       (KeyCode){5,0b00000001};
/*
    _keytable[SDLK_1]=       (KeyCode){5,0b10000000};
    _keytable[SDLK_2]=       (KeyCode){5,0b01000000};
    _keytable[SDLK_3]=       (KeyCode){5,0b00100000};
    _keytable[SDLK_4]=       (KeyCode){5,0b00010000};
    _keytable[SDLK_5]=       (KeyCode){5,0b00001000};
    _keytable[SDLK_6]=       (KeyCode){5,0b00000100};
    _keytable[SDLK_7]=       (KeyCode){5,0b00000010};
    _keytable[SDLK_8]=       (KeyCode){5,0b00000001};
*/    
    _keytable[SDLK_ASTERISK]=(KeyCode){6,0b10000000};
    _keytable[SDLK_PLUS]=    (KeyCode){6,0b01000000};
    _keytable[SDLK_MINUS]=   (KeyCode){6,0b00100000};
    _keytable[SDLK_SPACE]=   (KeyCode){6,0b00010000};
    _keytable[SDLK_0]=       (KeyCode){6,0b00001000};
    _keytable[SDLK_9]=       (KeyCode){6,0b00000100};
    _keytable[SDLK_COMMA]=   (KeyCode){6,0b00000010};
    _keytable[SDLK_PERIOD]=  (KeyCode){6,0b00000001};
    
    _keytable[SDLK_INSERT]=  (KeyCode){7,0b10000000};
    _keytable[SDLK_DELETE]=  (KeyCode){7,0b01000000};
    _keytable[SDLK_BACKSPACE]=  (KeyCode){7,0b01000000};
    _keytable[SDLK_UP]=      (KeyCode){7,0b00100000};
    _keytable[SDLK_DOWN]=    (KeyCode){7,0b00010000};
    _keytable[SDLK_RIGHT]=   (KeyCode){7,0b00001000};
    _keytable[SDLK_LEFT]=    (KeyCode){7,0b00000100};
    _keytable[SDLK_QUESTION]=(KeyCode){7,0b00000010};
    _keytable[SDLK_SLASH]=   (KeyCode){7,0b00000001};

    _keytable[SDLK_RGUI]=    (KeyCode){8,0b10000000};
    _keytable[SDLK_LCTRL]=   (KeyCode){8,0b01000000};
    _keytable[SDLK_RCTRL]=   (KeyCode){8,0b01000000};
    _keytable[SDLK_LSHIFT]=  (KeyCode){8,0b00000001};
    _keytable[SDLK_RSHIFT]=  (KeyCode){8,0b00000001};

    _keytable[SDLK_F1]=      (KeyCode){9,0b10000000};
    _keytable[SDLK_F2]=      (KeyCode){9,0b01000000};
    _keytable[SDLK_F3]=      (KeyCode){9,0b00100000};
    _keytable[SDLK_F4]=      (KeyCode){9,0b00010000};
    _keytable[SDLK_F5]=      (KeyCode){9,0b00001000};

    //strobe
    _strobe=0;
    
    //initialize matrix
    //ビットがセットされていると押されてない状態
    {   //lock
        std::lock_guard<std::mutex> lock(_mutex);
        for(int i=0;i<=10;i++)  _matrix[i]=0xff;
    }   //unlock
}


void Vmz1500Kbd::release(const Vmz1500KeyCode &key)
{
    //std::cout<<"release "<<key<<std::endl;
    {
        std::lock_guard<std::mutex> lock(_mutex);   //lock
    
        try{
            KeyCode c=_keytable.at(key);
            _matrix[c.strobe]|=c.code;
        }
        catch(std::out_of_range&){
        }
    
        switch(key){
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                _matrix[8]|=0b00000001; break;
            
            case SDLK_LCTRL:
            case SDLK_RCTRL:
                _matrix[8]|=0b01000000; break;
        }
    }   //unlock
}

void Vmz1500Kbd::press(const Vmz1500KeyCode &key)
{
    //std::cout<<"press "<<key<<std::endl;
    {
        std::lock_guard<std::mutex> lock(_mutex);   //lock

        try{
            KeyCode c=_keytable.at(key);
            _matrix[c.strobe]&=c.code^0xff;
        }
        catch(std::out_of_range&){
        }

        switch(key){
            case SDLK_LSHIFT:
            case SDLK_RSHIFT:
                _matrix[8]&=0b11111110; break;
            
            case SDLK_LCTRL:
            case SDLK_RCTRL:
                _matrix[8]&=0b10111111; break;
        }
    }   //unlock
}

void Vmz1500Kbd::setStrobe(int s){
    {   //lock
        std::lock_guard<std::mutex> lock(_mutex);
        _strobe=s;
    }   //unlock
}

//_matrix[] は更新中の可能性があるので厳密にはロックをかけるべきかもしれないが
//そこまでするほどのもんでもなかろう
Byte Vmz1500Kbd::key() const{return(_matrix[_strobe]);}


/*
    8253
 */

/*
Vmz1500I8253::Vmz1500I8253() : I8253()
{}

void Vmz1500I8253::reset(long clock)
{
    I8253::reset(clock);
    
    setGate(0, false);
//    setClock(0, 894886);    //894.88625kHz
    setGate(1, true);
//    setClock(1, 60*262); //15.72kHz(60frames*262rasters)
    setGate(2, true);
//    setClock(2, 1);     //
}

void Vmz1500I8253::sync(long clock)
{
    if (_ch[1].count(clock)){
        _ch[2].count(1);
    }
}

*/
