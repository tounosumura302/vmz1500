//
//  z80.cpp
//  vmz1500
//
//  Created by murasuke on 2017/04/01.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#include <iostream>
#include "z80.hpp"

//#include "z80.h"


/*
    I/O port
 */
Z80IOPort::Z80IOPort()
{
}
/*
bool Z80IOPort::setPeripheral(Word adr, Z80Peripheral_basic *per)
{
    if (_iomap.find(adr)!=_iomap.end())  return false;

    _iomap.insert(std::make_pair(adr, per));
    return true;
}
 */
bool Z80IOPort::setPeripheral(Word adr, Z80PeripheralInFunc infunc, Z80PeripheralOutFunc outfunc)
{
    if (_infunc.find(adr)!=_infunc.end())       return false;
    if (_outfunc.find(adr)!=_outfunc.end())    return false;
    
    _infunc.insert(std::make_pair(adr, infunc));
    _outfunc.insert(std::make_pair(adr, outfunc));
    return true;
}

void Z80IOPort::init()
{
//    _iomap.clear();
    _infunc.clear();
    _outfunc.clear();
}

Byte Z80IOPort::in(Word adr)
{
//    std::cout<<"z80ioport in "<<adr<<std::endl;
    
    //## i/o port addressは下位８ビットのみ有効（X1の場合は１６ビット全部を有効にしなければならない）
    adr&=0xff;
    
    try{
        return _infunc[adr].operator()();
//        return _iomap[adr]->inRequested();
    }
    catch(std::bad_function_call&){
        std::cerr<<std::hex<<adr<<" incorrect i/o port address"<<std::endl;
        return 0;
        //throw;
    }
}

void Z80IOPort::out(Word adr,Byte value)
{
//    std::cout<<"z80ioport out "<<adr<<" value "<<value<<std::endl;

    //## i/o port addressは下位８ビットのみ有効（X1の場合は１６ビット全部を有効にしなければならない）
    adr&=0xff;

    try{
        _outfunc[adr].operator()(value);
//        _iomap[adr]->outRequested(value);
    }
    catch(std::bad_function_call&){
        std::cerr<<std::hex<<adr<<" incorrect i/o port address"<<std::endl;
    }
}





/*
    Z80
 */

/*
 * constructor
 */
Z80::Z80(Memory *memory,Z80IOPort *ioport,Z80Pio *pio)
{
    _mem=memory;
    _ioport=ioport;
    _pio=pio;
    _status=STOP;
    
    std::random_device seedgen;
    _random=new std::mt19937(seedgen());
}

/*
 * destructor
 */
Z80::~Z80()
{
    delete _random;
}

/*
 * reset
 */
void Z80::reset()
{
    _pc=0;
    _f=0;
    _i=0;
    _iff=0; //disable interrupt
    _im=0;  //interrupt mode
    
    _irqflag=false; //no interruption
    _irqdata=0;
    
    _status=OK; //
}

/*
    R register
 */
void Z80::changeR()
{
    _r=(Byte)(_random->operator()()&0x0f);
}

/*
 * cheat (test)
 */
void Z80::jump(Word address)
{
    _pc=address;
    
    //まるでIPL ROMから起動されたかのような状態にする
    _sp=0x10f0; //SP
    _im=1;      //IM1
    _iff=0;     //DI
}

/*
 * execute
 */
int Z80::exec()
{
    Word old=_pc;   //@@test!!
//    Word oldv=_getWordAtAdr(0xd800);    //@@test!!
    
    //R register
    changeR();
    

    //##
//    if (_sp==0xd100){
    
    if (_pc==0x3950){
        int b=0;
        //std::cout<<"hogehoge!!!!"<<std::endl;
    }
    
    
    switch(_status){
        case OK :
            /*
             Byte op=_getByteAtPC();
             _curclock=_clock[op];
             (this->*(_opfunc[op]))();
             if (_curclock==0){
             emit messageSent(QString("error! %1 %2 %3").arg(old,4,16).arg(_pc,4,16).arg(op,2,16));
             emit hunged();
             }
             */
            
            //interrupt from Z80 PIO
            if (_pio && _pio->interrupt()){
                this->irq(_pio->intVector());
            }
            
            //not IRQ
            if (!_irqflag || !_iff){
                Byte op=_getByteAtPC();
                _execOp(op);
            }
            //IRQ occured
            else{
                _handleIrq();
            }

            /*
            //####
            if (_pc==0x0c02 || _pc==0x94e){
                std::cout<<"hogehoge!!!!"<<std::hex<<(int)old<<std::endl;
            }
*/
            
            //@@test!!
            /*
             Word newv=_getWordAtAdr(0xd800);
             if (oldv!=newv){
             messageSent(QString("d800 changed PC=%1 old=%2 new=%3").arg(old,4,16).arg(oldv,4,16).arg(newv,4,16));
             //emit stopped();
             }
             */
            /*
             //@@test!!
             //        if (_pc==0x2c5d){
             //        if (_pc==0x3de8){
             
             if (_pc==0x3fb7){
             messageSent(QString("3fb7! PC=%1 BC=%2 DE=%3 HL=%4").arg(old,4,16).arg(_bc_p.word,4,16).arg(_de_p.word,4,16).arg(_hl_p.word,4,16));
             //            _status=STOP;
             //emit stopped();
             }
             
             //@@test!!
             if (_pc==0x3cb4){
             messageSent(QString("3cb4! PC=%1 BC=%2 DE=%3 HL=%4").arg(old,4,16).arg(_bc_p.word,4,16).arg(_de_p.word,4,16).arg(_hl_p.word,4,16));
             //          _status=STOP;
             //          emit stopped();
             }
             */
            
            return(_curclock);
    }
    return(-1);
}

int Z80::_execOp(Byte op)
{
    _curclock=_clock[op];
    (this->*(_opfunc[op]))();
    if (_curclock==0){
//        emit messageSent(QString("clock error! %1").arg(_pc,4,16));
//        emit hunged();
        _status=HUNG;
    }
    return(_curclock);
}


/*
 * IRQ
 */
void Z80::irq(Byte data)
{
    _irqflag=true;
    _irqdata=data;
}

void Z80::_handleIrq()
{
    if (_iff==0) return;
    
    _irqflag=false;
    
    switch(_im){
        case 0:
//            qDebug()<<QString("IRQ mode 0 %1").arg(_irqdata,1,16);
            _execOp(_irqdata);
            break;
            
        case 1:
            //        qDebug()<<"IRQ!";
            _execOp(0xff);  //RST 38H
            //        qDebug()<<"IRQ!";
            
            break;
            
        case 2:
            //## mode2 not implemented!!
//            emit messageSent(QString("mode2 interrupt not implemented! %1").arg(_pc,4,16));
//            emit hunged();
//            _status=HUNG;
            _op_push(_pc);
            _pc=_getWordAtAdr((_i<<8)|_irqdata);
            _execOp(_getByteAtPC());
            break;
    }
}


/*
 * change cpu status
 */

void Z80::stop()
{
    //@@test!!
//    messageSent(QString("stop  PC=%1 BC=%2 DE=%3 HL=%4").arg(_pc,4,16).arg(_bc_p.word,4,16).arg(_de_p.word,4,16).arg(_hl_p.word,4,16));
    
    _status=STOP;
}

void Z80::restart()
{
    _status=OK;
}


/*
 * getter
 */

Byte Z80::regA() const {return(_a);}
Byte Z80::regF() const {return(_f);}
Byte Z80::regB() const {return(_rb);}
Byte Z80::regC() const {return(_rc);}
Byte Z80::regD() const {return(_rd);}
Byte Z80::regE() const {return(_re);}
Byte Z80::regH() const {return(_rh);}
Byte Z80::regL() const {return(_rl);}

Byte Z80::regAA() const {return(__a);}
Byte Z80::regFF() const {return(__f);}
Byte Z80::regBB() const {return(__rb);}
Byte Z80::regCC() const {return(__rc);}
Byte Z80::regDD() const {return(__rd);}
Byte Z80::regEE() const {return(__re);}
Byte Z80::regHH() const {return(__rh);}
Byte Z80::regLL() const {return(__rl);}

Byte Z80::regI() const {return(_i);}
Byte Z80::regR() const {return(_r);}
Word Z80::regBC() const {return(_rbc);}
Word Z80::regDE() const {return(_rde);}
Word Z80::regHL() const {return(_rhl);}
Word Z80::regIX() const {return(_ix);}
Word Z80::regIY() const {return(_iy);}
Word Z80::regSP() const {return(_sp);}
Word Z80::regPC() const {return(_pc);}

/*
 * get value from memory
 */
Byte Z80::_getByteAtPC()
{
    return(_mem->at(_pc++));
}

Word Z80::_getWordAtPC()
{
    Word w=_mem->at(_pc)|(_mem->at(_pc+1)<<8);
    _pc+=2;
    return(w);
}

Byte Z80::_getByteAtBC()
{
    return(_mem->at(_rbc));
}

Byte Z80::_getByteAtDE()
{
    return(_mem->at(_rde));
}

Byte Z80::_getByteAtHL()
{
    return(_mem->at(_rhl));
}

Word Z80::_getWordAtHL()
{
    Word p=_rhl;
    return(_mem->at(p)|(_mem->at(p+1)<<8));
}

Byte Z80::_getByteAtIX(Byte d)
{
    return(_mem->at(_ix+d));
}

Word Z80::_getWordAtIX()
{
    return(_mem->at(_ix)|(_mem->at(_ix+1)<<8));
}

Byte Z80::_getByteAtIY(Byte d)
{
    return(_mem->at(_iy+d));
}

Word Z80::_getWordAtIY()
{
    return(_mem->at(_iy)|(_mem->at(_iy+1)<<8));
}

Byte Z80::_getByteAtAdr(Word a)
{
    return(_mem->at(a));
}

Word Z80::_getWordAtAdr(Word a)
{
    return(_mem->at(a)|(_mem->at(a+1)<<8));
}

Byte Z80::_getByteAtSP(Word d)
{
    return(_mem->at(_sp+d));
}

Word Z80::_getWordAtSP()
{
    return(_mem->at(_sp)|(_mem->at(_sp+1)<<8));
}





void Z80::_setByteAtBC(Byte b)
{
    _mem->replace(_rbc,b);
}

void Z80::_setByteAtDE(Byte b)
{
    _mem->replace(_rde,b);
}

void Z80::_setByteAtHL(Byte b)
{
    _mem->replace(_rhl,b);
}

void Z80::_setByteAtIX(Byte d, Byte b)
{
    _mem->replace(_ix+d,b);
}

void Z80::_setByteAtIY(Byte d, Byte b)
{
    _mem->replace(_iy+d,b);
}

void Z80::_setByteAtAdr(Word a, Byte b)
{
    _mem->replace(a,b);
}

void Z80::_set2ByteAtAdr(Word a, Byte b1, Byte b2)
{
    _mem->replace(a,b1);
    _mem->replace(a+1,b2);
}

void Z80::_setWordAtAdr(Word a, Word w)
{
    _mem->replace(a,w&0xff);
    _mem->replace(a+1,(w&0xff00)>>8);
}

void Z80::_setByteAtSP(Word d, Byte b)
{
    _mem->replace(_sp+d,b);
}

void Z80::_setWordAtSP(Word w)
{
    _mem->replace(_sp,w&0xff);
    _mem->replace(_sp+1,(w&0xff00)>>8);
}

/*
 * flag
 */

Byte Z80::signFlag(const Byte b)
{
    return(b&0x80 ? FLAG_MASK_S : 0);
}

Byte Z80::signFlag(const Word b)
{
    return(b&0x8000 ? FLAG_MASK_S : 0);
}

Byte Z80::zeroFlag(const Byte b)
{
    return(b==0 ? 0b01000000 : 0);
}

Byte Z80::zeroFlag(const Word b)
{
    return(b==0 ? 0b01000000 : 0);
}

Byte Z80::halfcarryFlag(const Byte b)
{
    return(b&FLAG_MASK_H);
}

Byte Z80::overflowFlagAfterAdd(const Byte before,const Byte after)
{
    return((before&0x80)==0 && (after&0x80)!=0 ? FLAG_MASK_V : 0);
}

Byte Z80::overflowFlagAfterSub(const Byte before,const Byte after)
{
    return(((before&0x80)!=0 && (after&0x80)==0) ? FLAG_MASK_V : 0);
}

Byte Z80::overflowFlagAfterAdd(const Word before,const Word after)
{
    return((before&0x8000)==0 && (after&0x8000)!=0 ? FLAG_MASK_V : 0);
}

Byte Z80::overflowFlagAfterSub(const Word before,const Word after)
{
    return(((before&0x8000)!=0 && (after&0x8000)==0) ? FLAG_MASK_V : 0);
}

Byte Z80::parityFlag(const Byte b)
{
    return(((((b&1)+((b>>1)&1)+((b>>2)&1)+((b>>3)&1)+((b>>4)&1)+((b>>5)&1)+((b>>6)&1)+((b>>7)&1))&1)==0) ? FLAG_MASK_P : 0);
}

Byte Z80::iffFlag()
{
    return((!_iff) ? 0 : FLAG_MASK_P);
}

Byte Z80::carryFlagAfterAdd(const Byte before,const Byte after)
{
    return((after<before) ? FLAG_MASK_C : 0);
}

Byte Z80::carryFlagAfterSub(const Byte before,const Byte after)
{
    return((after>before) ? FLAG_MASK_C : 0);
}

Byte Z80::carryFlagAfterAdd(const Word before, const Word after)
{
    return((after<before) ? FLAG_MASK_C : 0);
}

Byte Z80::carryFlagAfterSub(const Word before, const Word after)
{
    return((after>before) ? FLAG_MASK_C : 0);
}

bool Z80::_isZ(){return(_f&FLAG_MASK_Z);}
bool Z80::_isNZ(){return(!(_f&FLAG_MASK_Z));}
bool Z80::_isC(){return(_f&FLAG_MASK_C);}
bool Z80::_isNC(){return(!(_f&FLAG_MASK_C));}
bool Z80::_isM(){return(_f&FLAG_MASK_S);}
bool Z80::_isP(){return(!(_f&FLAG_MASK_S));}
bool Z80::_isPE(){return(_f&FLAG_MASK_P);}
bool Z80::_isPO(){return(!(_f&FLAG_MASK_P));}

bool Z80::isZ() const{return(_f&FLAG_MASK_Z);}
bool Z80::isC() const{return(_f&FLAG_MASK_C);}
bool Z80::isM() const{return(_f&FLAG_MASK_S);}
bool Z80::isPE() const{return(_f&FLAG_MASK_P);}


/*
 * execute opecode
 */

void Z80::_op_undef(){
    //##undefined opecode##
    _status=HUNG;
//    emit messageSent(QString("error: undefined opecode\n"));
//    emit hunged();
    _status=HUNG;
}

void Z80::_op_add(Byte b){
    Byte tmp=_a;
    _a+=b;
    _f=signFlag(_a)|zeroFlag(_a)|halfcarryFlag(_a)|overflowFlagAfterAdd(tmp,_a)|carryFlagAfterAdd(tmp,_a);
}

void Z80::_op_adc(Byte b){
    _op_add(b+(_f&FLAG_MASK_C ? 1 :0));
}

void Z80::_op_sub(Byte b){
    Byte tmp=_a;
    _a-=b;
    _f=signFlag(_a)|zeroFlag(_a)|halfcarryFlag(_a)|overflowFlagAfterSub(tmp,_a)|FLAG_MASK_N|carryFlagAfterSub(tmp,_a);
}

void Z80::_op_sbc(Byte b){
    _op_sub(b+(_f&FLAG_MASK_C ? 1 :0));
}

void Z80::_op_cp(Byte b){
    Byte d=_a,tmp=_a;
    d-=b;
    _f=signFlag(d)|zeroFlag(d)|halfcarryFlag(d)|overflowFlagAfterSub(tmp,d)|FLAG_MASK_N|carryFlagAfterSub(tmp,d);
}

void Z80::_op_and(Byte b){
    _a&=b;
    _f=signFlag(_a)|zeroFlag(_a)|FLAG_MASK_H|parityFlag(_a);
}

void Z80::_op_xor(Byte b){
    _a^=b;
    _f=signFlag(_a)|zeroFlag(_a)|parityFlag(_a);
}

void Z80::_op_or(Byte b){
    _a|=b;
    _f=signFlag(_a)|zeroFlag(_a)|parityFlag(_a);
}

Byte Z80::_op_inc(Byte b){
    Byte tmp=b;
    b++;
    _f&=FLAG_MASK_C;
    _f|=signFlag(b)|zeroFlag(b)|halfcarryFlag(b)|overflowFlagAfterAdd(tmp,b);
    return(b);
}

Byte Z80::_op_dec(Byte b){
    Byte tmp=b;
    b--;
    _f&=FLAG_MASK_C;
    _f|=signFlag(b)|zeroFlag(b)|halfcarryFlag(b)|overflowFlagAfterSub(tmp,b)|FLAG_MASK_N;
    return(b);
}

Byte Z80::_op_rlc(Byte b){
    Byte c=b&0x80 ? 1 : 0;
    Byte tmp=(b<<1)|c;
    _f=signFlag(tmp)|zeroFlag(tmp)|parityFlag(tmp)|(c ? FLAG_MASK_C : 0);
    return(tmp);
}

Byte Z80::_op_rrc(Byte b){
    Byte c=b&0x01 ? 0x80 : 0;
    Byte tmp=(b>>1)|c;
    _f=signFlag(tmp)|zeroFlag(tmp)|parityFlag(tmp)|(c ? FLAG_MASK_C : 0);
    return(tmp);
}

Byte Z80::_op_rl(Byte b){
    Byte c=b&0x80 ? 1 : 0;
    Byte tmp=(b<<1)|(_f&FLAG_MASK_C ? 1 : 0);
    _f=signFlag(tmp)|zeroFlag(tmp)|parityFlag(tmp)|(c ? FLAG_MASK_C : 0);
    return(tmp);
}

Byte Z80::_op_rr(Byte b){
    Byte c=b&0x01 ? 0x80 : 0;
    Byte tmp=(b>>1)|(_f&FLAG_MASK_C ? 0x80 : 0);
    _f=signFlag(tmp)|zeroFlag(tmp)|parityFlag(tmp)|(c ? FLAG_MASK_C : 0);
    return(tmp);
}

Byte Z80::_op_sla(Byte b){
    Byte c=b&0x80 ? 1 : 0;
    Byte tmp=(b<<1);
    _f=signFlag(tmp)|zeroFlag(tmp)|parityFlag(tmp)|(c ? FLAG_MASK_C : 0);
    return(tmp);
}

Byte Z80::_op_sra(Byte b){
    Byte c=b&0x01 ? 1 : 0;
    Byte tmp=(b&0x80)|(b>>1);
    _f=signFlag(tmp)|zeroFlag(tmp)|parityFlag(tmp)|(c ? FLAG_MASK_C : 0);
    return(tmp);
}

Byte Z80::_op_srl(Byte b){
    Byte c=b&0x01 ? 1 : 0;
    Byte tmp=(b>>1);
    _f=signFlag(tmp)|zeroFlag(tmp)|parityFlag(tmp)|(c ? FLAG_MASK_C : 0);
    return(tmp);
}

Byte Z80::_bitTable[8]={0b00000001,0b00000010,0b00000100,0b00001000,0b00010000,0b00100000,0b01000000,0b10000000};
Byte Z80::_op_set(Byte b, Byte bit){return(b|_bitTable[bit]);}
Byte Z80::_op_res(Byte b, Byte bit){return(b&(_bitTable[bit]^0xff));}
void Z80::_op_bit(Byte b, Byte bit){
    b&=_bitTable[bit];
    _f&=FLAG_MASK_C;
    _f|=signFlag(b)|zeroFlag(b)|FLAG_MASK_H|parityFlag(b);
}

void Z80::_op_push(const Byte b){_mem->replace(--_sp,b);}
void Z80::_op_push(const Word w){_mem->replace(--_sp,(Byte)((w&0xff00)>>8)); _mem->replace(--_sp,(Byte)(w&0x00ff)); }
//void Z80::_op_push(const Word w){_mem->replace(--_sp,(Byte)(w&0x00ff)); _mem->replace(--_sp,(Byte)((w&0xff00)>>8));}
Byte Z80::_op_pop(){return(_mem->at(_sp++));}
Word Z80::_op_pop_word(){
    Word tmp=_mem->at(_sp);  _sp++;
    tmp|=(Word)(_mem->at(_sp))<<8;  _sp++;
    return tmp;
}
//    return (Word)(_mem->at(_sp++)) | (Word)((_mem->at(_sp++))<<8) ;}
//Word Z80::_op_pop_word(){return(((Word)(_mem->at(_sp++))<<8)|(_mem->at(_sp++)));}

Byte Z80::_op_in(){
    Byte b=_ioport->in(_rbc);
    _f&=FLAG_MASK_C;
    _f|=signFlag(b)|zeroFlag(b)|halfcarryFlag(b)|parityFlag(b);
    return(b);
}

void Z80::_op_out(const Byte b){_ioport->out(_rbc,b);}



/*
 * prefix opecode
 */

void Z80::_op_cb(){Byte op=_getByteAtPC(); _curclock=_clock_cb[op]; (this->*(_opfunc_cb[op]))();}
void Z80::_op_dd(){Byte op=_getByteAtPC(); _curclock=_clock_dd[op]; (this->*(_opfunc_dd[op]))();}
void Z80::_op_ed(){Byte op=_getByteAtPC(); _curclock=_clock_ed[op]; (this->*(_opfunc_ed[op]))();}
void Z80::_op_fd(){Byte op=_getByteAtPC(); _curclock=_clock_dd[op]; (this->*(_opfunc_fd[op]))();}

void Z80::_op_dd_cb(){
    Byte d=_getByteAtPC(),s=_getByteAtPC();
    _curclock=_clock_dd_cb[s];
    switch(s){
        case 0x06:  _op_dd_cb_d_06(d); break;
        case 0x0e:  _op_dd_cb_d_0e(d); break;
        case 0x16:  _op_dd_cb_d_16(d); break;
        case 0x1e:  _op_dd_cb_d_1e(d); break;
        case 0x26:  _op_dd_cb_d_26(d); break;
        case 0x2e:  _op_dd_cb_d_2e(d); break;
        case 0x3e:  _op_dd_cb_d_3e(d); break;
            
        case 0x46:  _op_dd_cb_d_46(d); break;
        case 0x4e:  _op_dd_cb_d_4e(d); break;
        case 0x56:  _op_dd_cb_d_56(d); break;
        case 0x5e:  _op_dd_cb_d_5e(d); break;
        case 0x66:  _op_dd_cb_d_66(d); break;
        case 0x6e:  _op_dd_cb_d_6e(d); break;
        case 0x76:  _op_dd_cb_d_76(d); break;
        case 0x7e:  _op_dd_cb_d_7e(d); break;
            
        case 0x86:  _op_dd_cb_d_86(d); break;
        case 0x8e:  _op_dd_cb_d_8e(d); break;
        case 0x96:  _op_dd_cb_d_96(d); break;
        case 0x9e:  _op_dd_cb_d_9e(d); break;
        case 0xa6:  _op_dd_cb_d_a6(d); break;
        case 0xae:  _op_dd_cb_d_ae(d); break;
        case 0xb6:  _op_dd_cb_d_b6(d); break;
        case 0xbe:  _op_dd_cb_d_be(d); break;
            
        case 0xc6:  _op_dd_cb_d_c6(d); break;
        case 0xce:  _op_dd_cb_d_ce(d); break;
        case 0xd6:  _op_dd_cb_d_d6(d); break;
        case 0xde:  _op_dd_cb_d_de(d); break;
        case 0xe6:  _op_dd_cb_d_e6(d); break;
        case 0xee:  _op_dd_cb_d_ee(d); break;
        case 0xf6:  _op_dd_cb_d_f6(d); break;
        case 0xfe:  _op_dd_cb_d_fe(d); break;
            
        default: _op_undef();
    }
}

void Z80::_op_fd_cb(){
    Byte d=_getByteAtPC(),s=_getByteAtPC();
    _curclock=_clock_dd_cb[s];
    switch(s){
        case 0x06:  _op_fd_cb_d_06(d); break;
        case 0x0e:  _op_fd_cb_d_0e(d); break;
        case 0x16:  _op_fd_cb_d_16(d); break;
        case 0x1e:  _op_fd_cb_d_1e(d); break;
        case 0x26:  _op_fd_cb_d_26(d); break;
        case 0x2e:  _op_fd_cb_d_2e(d); break;
        case 0x3e:  _op_fd_cb_d_3e(d); break;
            
        case 0x46:  _op_fd_cb_d_46(d); break;
        case 0x4e:  _op_fd_cb_d_4e(d); break;
        case 0x56:  _op_fd_cb_d_56(d); break;
        case 0x5e:  _op_fd_cb_d_5e(d); break;
        case 0x66:  _op_fd_cb_d_66(d); break;
        case 0x6e:  _op_fd_cb_d_6e(d); break;
        case 0x76:  _op_fd_cb_d_76(d); break;
        case 0x7e:  _op_fd_cb_d_7e(d); break;
            
        case 0x86:  _op_fd_cb_d_86(d); break;
        case 0x8e:  _op_fd_cb_d_8e(d); break;
        case 0x96:  _op_fd_cb_d_96(d); break;
        case 0x9e:  _op_fd_cb_d_9e(d); break;
        case 0xa6:  _op_fd_cb_d_a6(d); break;
        case 0xae:  _op_fd_cb_d_ae(d); break;
        case 0xb6:  _op_fd_cb_d_b6(d); break;
        case 0xbe:  _op_fd_cb_d_be(d); break;
            
        case 0xc6:  _op_fd_cb_d_c6(d); break;
        case 0xce:  _op_fd_cb_d_ce(d); break;
        case 0xd6:  _op_fd_cb_d_d6(d); break;
        case 0xde:  _op_fd_cb_d_de(d); break;
        case 0xe6:  _op_fd_cb_d_e6(d); break;
        case 0xee:  _op_fd_cb_d_ee(d); break;
        case 0xf6:  _op_fd_cb_d_f6(d); break;
        case 0xfe:  _op_fd_cb_d_fe(d); break;
            
        default: _op_undef();
    }
}


/*
 * CPU control
 */
//NOP
void Z80::_op_00(){/*nop*/}
//HALT
void Z80::_op_76(){_pc--;}
//DI
void Z80::_op_f3(){_iff=0;}
//EI
void Z80::_op_fb(){_iff=1;}
//IM0
void Z80::_op_ed_46(){_im=0;}
//IM1
void Z80::_op_ed_56(){_im=1;}
//IM2
void Z80::_op_ed_5e(){_im=2;}


/*
 * accumulator control
 */
//DAA
void Z80::_op_27(){
    Byte c=0,al=_a&0x0f,ah=_a&0xf0;
    if (al>9){
        ah+=0x10;
        al-=10;
    }
    if (ah>0x90){
        c=FLAG_MASK_C;
        ah-=0xa0;
    }
    _a=ah|al;
    _f=signFlag(_a)|zeroFlag(_a)|halfcarryFlag(_a)|parityFlag(_a)|c;
}

//CPL
void Z80::_op_2f(){_a^=0xff;}
//NEG
void Z80::_op_ed_44(){_a=(_a^0xff)+1;}
//CCF
void Z80::_op_37(){_f|=FLAG_MASK_C;}
//SCF
void Z80::_op_3f(){_f&=(FLAG_MASK_C^0xff);}

/*
 * exchange
 */
//EX AF,AF'
void Z80::_op_08(){Byte tmp; tmp=_a; _a=__a; __a=tmp; tmp=_f; _f=__f; __f=tmp;}
//EX DE,HL
void Z80::_op_eb(){Byte tmp; tmp=_rd; _rd=_rh; _rh=tmp; tmp=_re; _re=_rl; _rl=tmp;}
//EX (SP),HL
void Z80::_op_e3(){Byte tmp;
    tmp=_getByteAtSP(0); _setByteAtSP(0,_rl); _rl=tmp;
    tmp=_getByteAtSP(1); _setByteAtSP(1,_rh); _rh=tmp;}
//EX (SP),IX
void Z80::_op_dd_e3(){Word tmp; tmp=_getWordAtSP(); _setWordAtSP(_ix); _ix=tmp;}
//EX (SP),IY
void Z80::_op_fd_e3(){Word tmp; tmp=_getWordAtSP(); _setWordAtSP(_iy); _iy=tmp;}
//EXX
void Z80::_op_d9(){Word tmp;
    tmp=_rbc; _rbc=_xrbc; _xrbc=tmp;
    tmp=_rde; _rde=_xrde; _xrde=tmp;
    tmp=_rhl; _rhl=_xrhl; _xrhl=tmp;}

/*
 * 16bit arithmetic calculation
 */
//ADD HL,BC
void Z80::_op_09(){Word tmp=_rhl;
    _rhl+=_rbc;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_P;
    _f|=carryFlagAfterAdd(tmp,_rhl);}
//ADD HL,DE
void Z80::_op_19(){Word tmp=_rhl;
    _rhl+=_rde;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_P;
    _f|=carryFlagAfterAdd(tmp,_rhl);}
//ADD HL,HL
void Z80::_op_29(){Word tmp=_rhl;
    _rhl+=_rhl;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_P;
    _f|=carryFlagAfterAdd(tmp,_rhl);}
//ADD HL,SP
void Z80::_op_39(){Word tmp=_rhl;
    _rhl+=_sp;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_P;
    _f|=carryFlagAfterAdd(tmp,_rhl);}
//ADD IX,BC
void Z80::_op_dd_09(){Word tmp=_ix;
    _ix+=_rbc;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_P;
    _f|=carryFlagAfterAdd(tmp,_ix);}
//ADD IX,DE
void Z80::_op_dd_19(){Word tmp=_ix;
    _ix+=_rde;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_P;
    _f|=carryFlagAfterAdd(tmp,_ix);}
//ADD IX,IX
void Z80::_op_dd_29(){Word tmp=_ix;
    _ix+=_ix;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_P;
    _f|=carryFlagAfterAdd(tmp,_ix);}
//ADD IX,SP
void Z80::_op_dd_39(){Word tmp=_ix;
    _ix+=_sp;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_P;
    _f|=carryFlagAfterAdd(tmp,_ix);}
//ADD IY,BC
void Z80::_op_fd_09(){Word tmp=_iy;
    _iy+=_rbc;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_P;
    _f|=carryFlagAfterAdd(tmp,_iy);}
//ADD IY,DE
void Z80::_op_fd_19(){Word tmp=_iy;
    _iy+=_rde;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_P;
    _f|=carryFlagAfterAdd(tmp,_iy);}
//ADD IY,IY
void Z80::_op_fd_29(){Word tmp=_iy;
    _iy+=_iy;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_P;
    _f|=carryFlagAfterAdd(tmp,_iy);}
//ADD IY,SP
void Z80::_op_fd_39(){Word tmp=_iy;
    _iy+=_sp;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_P;
    _f|=carryFlagAfterAdd(tmp,_iy);}
//ADC HL,BC
void Z80::_op_ed_4a(){Word tmp=_rhl;
    _rhl+=_rbc+(_f&FLAG_MASK_C ? 1 : 0);
    _f=overflowFlagAfterAdd(tmp,_rhl)|carryFlagAfterAdd(tmp,_rhl)|zeroFlag(_rhl)|signFlag(_rhl);}
//                      _f&=FLAG_MASK_S|FLAG_MASK_P;
//                      _f|=overflowFlagAfterAdd(tmp,_rhl)|carryFlagAfterAdd(tmp,_rhl)|zeroFlag(_rhl);}
//ADC HL,DE
void Z80::_op_ed_5a(){Word tmp=_rhl;
    _rhl+=_rde+(_f&FLAG_MASK_C ? 1 : 0);
    _f=overflowFlagAfterAdd(tmp,_rhl)|carryFlagAfterAdd(tmp,_rhl)|zeroFlag(_rhl)|signFlag(_rhl);}
//                      _f&=FLAG_MASK_S|FLAG_MASK_P;
//                      _f|=overflowFlagAfterAdd(tmp,_rhl)|carryFlagAfterAdd(tmp,_rhl)|zeroFlag(_rhl);}
//ADC HL,HL
void Z80::_op_ed_6a(){Word tmp=_rhl;
    _rhl+=_rhl+(_f&FLAG_MASK_C ? 1 : 0);
    _f=overflowFlagAfterAdd(tmp,_rhl)|carryFlagAfterAdd(tmp,_rhl)|zeroFlag(_rhl)|signFlag(_rhl);}
//                      _f&=FLAG_MASK_S|FLAG_MASK_P;
//                      _f|=overflowFlagAfterAdd(tmp,_rhl)|carryFlagAfterAdd(tmp,_rhl)|zeroFlag(_rhl);}
//ADC HL,SP
void Z80::_op_ed_7a(){Word tmp=_rhl;
    _rhl+=_sp+(_f&FLAG_MASK_C ? 1 : 0);
    _f=overflowFlagAfterAdd(tmp,_rhl)|carryFlagAfterAdd(tmp,_rhl)|zeroFlag(_rhl)|signFlag(_rhl);}
//                      _f&=FLAG_MASK_S|FLAG_MASK_P;
//                      _f|=overflowFlagAfterAdd(tmp,_rhl)|carryFlagAfterAdd(tmp,_rhl)|zeroFlag(_rhl);}
//SBC HL,BC
void Z80::_op_ed_42(){Word tmp=_rhl;
    _rhl-=_rbc-(_f&FLAG_MASK_C ? 1 : 0);
    _f=overflowFlagAfterSub(tmp,_rhl)|carryFlagAfterSub(tmp,_rhl)|zeroFlag(_rhl)|signFlag(_rhl)|FLAG_MASK_N;}
//                      _f&=FLAG_MASK_S|FLAG_MASK_P;
//                      _f|=overflowFlagAfterSub(tmp,_rhl)|carryFlagAfterSub(tmp,_rhl)|zeroFlag(_rhl);}
//SBC HL,DE
void Z80::_op_ed_52(){Word tmp=_rhl;
    _rhl-=_rde-(_f&FLAG_MASK_C ? 1 : 0);
    _f=overflowFlagAfterSub(tmp,_rhl)|carryFlagAfterSub(tmp,_rhl)|zeroFlag(_rhl)|signFlag(_rhl)|FLAG_MASK_N;}
//                      _f&=FLAG_MASK_S|FLAG_MASK_P;
//                      _f|=overflowFlagAfterSub(tmp,_rhl)|carryFlagAfterSub(tmp,_rhl)|zeroFlag(_rhl);}
///SBC HL,HL
void Z80::_op_ed_62(){Word tmp=_rhl;
    _rhl-=_rhl-(_f&FLAG_MASK_C ? 1 : 0);
    _f=overflowFlagAfterSub(tmp,_rhl)|carryFlagAfterSub(tmp,_rhl)|zeroFlag(_rhl)|signFlag(_rhl)|FLAG_MASK_N;}
//                      _f&=FLAG_MASK_S|FLAG_MASK_P;
//                      _f|=overflowFlagAfterSub(tmp,_rhl)|carryFlagAfterSub(tmp,_rhl)|zeroFlag(_rhl);}
//SBC HL,SP
void Z80::_op_ed_72(){Word tmp=_rhl;
    _rhl-=_sp-(_f&FLAG_MASK_C ? 1 : 0);
    _f=overflowFlagAfterSub(tmp,_rhl)|carryFlagAfterSub(tmp,_rhl)|zeroFlag(_rhl)|signFlag(_rhl)|FLAG_MASK_N;}
//                      _f&=FLAG_MASK_S|FLAG_MASK_P;
//                      _f|=overflowFlagAfterSub(tmp,_rhl)|carryFlagAfterSub(tmp,_rhl)|zeroFlag(_rhl);}
//INC BC
void Z80::_op_03(){_rbc++;}
//INC DE
void Z80::_op_13(){_rde++;}
//INC HL
void Z80::_op_23(){_rhl++;}
//INC SP
void Z80::_op_33(){_sp++;}
//INC IX
void Z80::_op_dd_23(){_ix++;}
//INC IY
void Z80::_op_fd_23(){_iy++;}
//DEC BC
void Z80::_op_0b(){_rbc--;}
//DEC DE
void Z80::_op_1b(){_rde--;}
//DEC HL
void Z80::_op_2b(){_rhl--;}
//DEC SP
void Z80::_op_3b(){_sp--;}
//DEC IX
void Z80::_op_dd_2b(){_ix--;}
//DEC IY
void Z80::_op_fd_2b(){_iy--;}

/*
 * 8bit calculation
 */
//ADD A,B
void Z80::_op_80(){_op_add(_rb);}
//ADD A,C
void Z80::_op_81(){_op_add(_rc);}
//ADD A,D
void Z80::_op_82(){_op_add(_rd);}
//ADD A,E
void Z80::_op_83(){_op_add(_re);}
//ADD A,H
void Z80::_op_84(){_op_add(_rh);}
//ADD A,L
void Z80::_op_85(){_op_add(_rl);}
//ADD A,(HL)
void Z80::_op_86(){_op_add(_getByteAtHL());}
//ADD A,A
void Z80::_op_87(){_op_add(_a);}
//ADD A,(IX+d)
void Z80::_op_dd_86(){_op_add(_getByteAtIX(_getByteAtPC()));}
//ADD A,(IY+d)
void Z80::_op_fd_86(){_op_add(_getByteAtIY(_getByteAtPC()));}
//ADD A,n
void Z80::_op_c6(){_op_add(_getByteAtPC());}

//ADC A,B
void Z80::_op_88(){_op_adc(_rb);}
//ADC A,C
void Z80::_op_89(){_op_adc(_rc);}
//ADC A,D
void Z80::_op_8a(){_op_adc(_rd);}
//ADC A,E
void Z80::_op_8b(){_op_adc(_re);}
//ADC A,H
void Z80::_op_8c(){_op_adc(_rh);}
//ADC A,L
void Z80::_op_8d(){_op_adc(_rl);}
//ADC A,(HL)
void Z80::_op_8e(){_op_adc(_getByteAtHL());}
//ADC A,A
void Z80::_op_8f(){_op_adc(_a);}
//ADC A,(IX+d)
void Z80::_op_dd_8e(){_op_adc(_getByteAtIX(_getByteAtPC()));}
//ADC A,(IY+d)
void Z80::_op_fd_8e(){_op_adc(_getByteAtIY(_getByteAtPC()));}
//ADC A,n
void Z80::_op_ce(){_op_adc(_getByteAtPC());}

//SUB A,B
void Z80::_op_90(){_op_sub(_rb);}
//SUB A,C
void Z80::_op_91(){_op_sub(_rc);}
//SUB A,D
void Z80::_op_92(){_op_sub(_rd);}
//SUB A,E
void Z80::_op_93(){_op_sub(_re);}
//SUB A,H
void Z80::_op_94(){_op_sub(_rh);}
//SUB A,L
void Z80::_op_95(){_op_sub(_rl);}
//SUB A,(HL)
void Z80::_op_96(){_op_sub(_getByteAtHL());}
//SUB A,A
void Z80::_op_97(){_op_sub(_a);}
//SUB A,(IX+d)
void Z80::_op_dd_96(){_op_sub(_getByteAtIX(_getByteAtPC()));}
//SUB A,(IY+d)
void Z80::_op_fd_96(){_op_sub(_getByteAtIY(_getByteAtPC()));}
//SUB A,n
void Z80::_op_d6(){_op_sub(_getByteAtPC());}

//SBC A,B
void Z80::_op_98(){_op_sbc(_rb);}
//SBC A,C
void Z80::_op_99(){_op_sbc(_rc);}
//SBC A,D
void Z80::_op_9a(){_op_sbc(_rd);}
//SBC A,E
void Z80::_op_9b(){_op_sbc(_re);}
//SBC A,H
void Z80::_op_9c(){_op_sbc(_rh);}
//SBC A,L
void Z80::_op_9d(){_op_sbc(_rl);}
//SBC A,(HL)
void Z80::_op_9e(){_op_sbc(_getByteAtHL());}
//SBC A,A
void Z80::_op_9f(){_op_sbc(_a);}
//SBC A,(IX+d)
void Z80::_op_dd_9e(){_op_sbc(_getByteAtIX(_getByteAtPC()));}
//SBC A,(IY+d)
void Z80::_op_fd_9e(){_op_sbc(_getByteAtIY(_getByteAtPC()));}
//SBC A,n
void Z80::_op_de(){_op_sbc(_getByteAtPC());}

//AND B
void Z80::_op_a0(){_op_and(_rb);}
//AND C
void Z80::_op_a1(){_op_and(_rc);}
//AND D
void Z80::_op_a2(){_op_and(_rd);}
//AND E
void Z80::_op_a3(){_op_and(_re);}
//AND H
void Z80::_op_a4(){_op_and(_rh);}
//AND L
void Z80::_op_a5(){_op_and(_rl);}
//AND (HL)
void Z80::_op_a6(){_op_and(_getByteAtHL());}
//AND A
void Z80::_op_a7(){_op_and(_a);}
//AND (IX+d)
void Z80::_op_dd_a6(){_op_and(_getByteAtIX(_getByteAtPC()));}
//AND (IY+d)
void Z80::_op_fd_a6(){_op_and(_getByteAtIY(_getByteAtPC()));}
//AND n
void Z80::_op_e6(){_op_and(_getByteAtPC());}

//XOR B
void Z80::_op_a8(){_op_xor(_rb);}
//XOR C
void Z80::_op_a9(){_op_xor(_rc);}
//XOR D
void Z80::_op_aa(){_op_xor(_rd);}
//XOR E
void Z80::_op_ab(){_op_xor(_re);}
//XOR H
void Z80::_op_ac(){_op_xor(_rh);}
//XOR L
void Z80::_op_ad(){_op_xor(_rl);}
//XOR (HL)
void Z80::_op_ae(){_op_xor(_getByteAtHL());}
//XOR A
void Z80::_op_af(){_op_xor(_a);}
//XOR (IX+d)
void Z80::_op_dd_ae(){_op_xor(_getByteAtIX(_getByteAtPC()));}
//XOR (IY+d)
void Z80::_op_fd_ae(){_op_xor(_getByteAtIY(_getByteAtPC()));}
//XOR n
void Z80::_op_ee(){_op_xor(_getByteAtPC());}

//OR B
void Z80::_op_b0(){_op_or(_rb);}
//OR C
void Z80::_op_b1(){_op_or(_rc);}
//OR D
void Z80::_op_b2(){_op_or(_rd);}
//OR E
void Z80::_op_b3(){_op_or(_re);}
//OR H
void Z80::_op_b4(){_op_or(_rh);}
//OR L
void Z80::_op_b5(){_op_or(_rl);}
//OR (HL)
void Z80::_op_b6(){_op_or(_getByteAtHL());}
//OR A
void Z80::_op_b7(){_op_or(_a);}
//OR (IX+d)
void Z80::_op_dd_b6(){_op_or(_getByteAtIX(_getByteAtPC()));}
//OR (IY+d)
void Z80::_op_fd_b6(){_op_or(_getByteAtIY(_getByteAtPC()));}
//OR n
void Z80::_op_f6(){_op_or(_getByteAtPC());}

//CP B
void Z80::_op_b8(){_op_cp(_rb);}
//CP C
void Z80::_op_b9(){_op_cp(_rc);}
//CP D
void Z80::_op_ba(){_op_cp(_rd);}
//CP E
void Z80::_op_bb(){_op_cp(_re);}
//CP H
void Z80::_op_bc(){_op_cp(_rh);}
//CP L
void Z80::_op_bd(){_op_cp(_rl);}
//CP (HL)
void Z80::_op_be(){_op_cp(_getByteAtHL());}
//CP A
void Z80::_op_bf(){_op_cp(_a);}
//CP (IX+d)
void Z80::_op_dd_be(){_op_cp(_getByteAtIX(_getByteAtPC()));}
//CP (IY+d)
void Z80::_op_fd_be(){_op_cp(_getByteAtIY(_getByteAtPC()));}
//CP n
void Z80::_op_fe(){_op_cp(_getByteAtPC());}

//INC A
void Z80::_op_3c(){_a=_op_inc(_a);}
//INC B
void Z80::_op_04(){_rb=_op_inc(_rb);}
//INC C
void Z80::_op_0c(){_rc=_op_inc(_rc);}
//INC D
void Z80::_op_14(){_rd=_op_inc(_rd);}
//INC E
void Z80::_op_1c(){_re=_op_inc(_re);}
//INC H
void Z80::_op_24(){_rh=_op_inc(_rh);}
//INC L
void Z80::_op_2c(){_rl=_op_inc(_rl);}
//INC (HL)
void Z80::_op_34(){_setByteAtHL(_op_inc(_getByteAtHL()));}
//INC (IX+d)
void Z80::_op_dd_34(){Byte d=_getByteAtPC(); _setByteAtIX(d,_op_inc(_getByteAtIX(d)));}
//INC (IY+d)
void Z80::_op_fd_34(){Byte d=_getByteAtPC(); _setByteAtIY(d,_op_inc(_getByteAtIY(d)));}

//DEC A
void Z80::_op_3d(){_a=_op_dec(_a);}
//DEC B
void Z80::_op_05(){_rb=_op_dec(_rb);}
//DEC C
void Z80::_op_0d(){_rc=_op_dec(_rc);}
//DEC D
void Z80::_op_15(){_rd=_op_dec(_rd);}
//DEC E
void Z80::_op_1d(){_re=_op_dec(_re);}
//DEC H
void Z80::_op_25(){_rh=_op_dec(_rh);}
//DEC L
void Z80::_op_2d(){_rl=_op_dec(_rl);}
//DEC (HL)
void Z80::_op_35(){_setByteAtHL(_op_dec(_getByteAtHL()));}
//DEC (IX+d)
void Z80::_op_dd_35(){Byte d=_getByteAtPC(); _setByteAtIX(d,_op_dec(_getByteAtIX(d)));}
//DEC (IY+d)
void Z80::_op_fd_35(){Byte d=_getByteAtPC(); _setByteAtIY(d,_op_dec(_getByteAtIY(d)));}

/*
 * 8bit load
 */
//LD A,I
void Z80::_op_ed_57(){_a=_i; _f&=FLAG_MASK_C; _f|=zeroFlag(_a)|signFlag(_a)|iffFlag();}
//LD A,R
void Z80::_op_ed_5f(){_a=_r; _f&=FLAG_MASK_C; _f|=zeroFlag(_a)|signFlag(_a)|iffFlag();}

//LD A,B
void Z80::_op_78(){_a=_rb;}
//LD A,C
void Z80::_op_79(){_a=_rc;}
//LD A,D
void Z80::_op_7a(){_a=_rd;}
//LD A,E
void Z80::_op_7b(){_a=_re;}
//LD A,H
void Z80::_op_7c(){_a=_rh;}
//LD A,L
void Z80::_op_7d(){_a=_rl;}
//LD A,(HL)
void Z80::_op_7e(){_a=_getByteAtHL();}
//LD A,A
void Z80::_op_7f(){/*_a=_a;*/}
//LD A,(BC)
void Z80::_op_0a(){_a=_getByteAtBC();}
//LD A,(DE)
void Z80::_op_1a(){_a=_getByteAtDE();}
//LD A,(IX+d)
void Z80::_op_dd_7e(){_a=_getByteAtIX(_getByteAtPC());}
//LD A,(IY+d)
void Z80::_op_fd_7e(){_a=_getByteAtIY(_getByteAtPC());}
//LD A,(nn)
void Z80::_op_3a(){_a=_getByteAtAdr(_getWordAtPC());}
//LD A,n
void Z80::_op_3e(){_a=_getByteAtPC();}

//LD B,B
void Z80::_op_40(){/*_rb=_rb;*/}
//LD B,C
void Z80::_op_41(){_rb=_rc;}
//LD B,D
void Z80::_op_42(){_rb=_rd;}
//LD B,E
void Z80::_op_43(){_rb=_re;}
//LD B,H
void Z80::_op_44(){_rb=_rh;}
//LD B,L
void Z80::_op_45(){_rb=_rl;}
//LD B,(HL)
void Z80::_op_46(){_rb=_getByteAtHL();}
//LD B,A
void Z80::_op_47(){_rb=_a;}
//LD B,(IX+d)
void Z80::_op_dd_46(){_rb=_getByteAtIX(_getByteAtPC());}
//LD B,(IY+d)
void Z80::_op_fd_46(){_rb=_getByteAtIY(_getByteAtPC());}
//LD B,n
void Z80::_op_06(){_rb=_getByteAtPC();}

//LD C,B
void Z80::_op_48(){_rc=_rb;}
//LD C,C
void Z80::_op_49(){/*_c=_c;*/}
//LD C,D
void Z80::_op_4a(){_rc=_rd;}
//LD C,E
void Z80::_op_4b(){_rc=_re;}
//LD C,H
void Z80::_op_4c(){_rc=_rh;}
//LD C,L
void Z80::_op_4d(){_rc=_rl;}
//LD C,(HL)
void Z80::_op_4e(){_rc=_getByteAtHL();}
//LD C,A
void Z80::_op_4f(){_rc=_a;}
//LD C,(IX+d)
void Z80::_op_dd_4e(){_rc=_getByteAtIX(_getByteAtPC());}
//LD C,(IY+d)
void Z80::_op_fd_4e(){_rc=_getByteAtIY(_getByteAtPC());}
//LD C,n
void Z80::_op_0e(){_rc=_getByteAtPC();}

//LD D,B
void Z80::_op_50(){_rd=_rb;}
//LD D,C
void Z80::_op_51(){_rd=_rc;}
//LD D,D
void Z80::_op_52(){/*_d=_d;*/}
//LD D,E
void Z80::_op_53(){_rd=_re;}
//LD D,H
void Z80::_op_54(){_rd=_rh;}
//LD D,L
void Z80::_op_55(){_rd=_rl;}
//LD D,(HL)
void Z80::_op_56(){_rd=_getByteAtHL();}
//LD D,A
void Z80::_op_57(){_rd=_a;}
//LD D,(IX+d)
void Z80::_op_dd_56(){_rd=_getByteAtIX(_getByteAtPC());}
//LD D,(IY+d)
void Z80::_op_fd_56(){_rd=_getByteAtIY(_getByteAtPC());}
//LD D,n
void Z80::_op_16(){_rd=_getByteAtPC();}

//LD E,B
void Z80::_op_58(){_re=_rb;}
//LD E,C
void Z80::_op_59(){_re=_rc;}
//LD E,D
void Z80::_op_5a(){_re=_rd;}
//LD E,E
void Z80::_op_5b(){/*_e=_e;*/}
//LD E,H
void Z80::_op_5c(){_re=_rh;}
//LD E,L
void Z80::_op_5d(){_re=_rl;}
//LD E,(HL)
void Z80::_op_5e(){_re=_getByteAtHL();}
//LD E,A
void Z80::_op_5f(){_re=_a;}
//LD E,(IX+d)
void Z80::_op_dd_5e(){_re=_getByteAtIX(_getByteAtPC());}
//LD E,(IY+d)
void Z80::_op_fd_5e(){_re=_getByteAtIY(_getByteAtPC());}
//LD E,n
void Z80::_op_1e(){_re=_getByteAtPC();}

//LD H,B
void Z80::_op_60(){_rh=_rb;}
//LD H,C
void Z80::_op_61(){_rh=_rc;}
//LD H,D
void Z80::_op_62(){_rh=_rd;}
//LD H,E
void Z80::_op_63(){_rh=_re;}
//LD H,H
void Z80::_op_64(){/*_h=_h;*/}
//LD H,L
void Z80::_op_65(){_rh=_rl;}
//LD H,(HL)
void Z80::_op_66(){_rh=_getByteAtHL();}
//LD H,A
void Z80::_op_67(){_rh=_a;}
//LD H,(IX+d)
void Z80::_op_dd_66(){_rh=_getByteAtIX(_getByteAtPC());}
//LD H,(IY+d)
void Z80::_op_fd_66(){_rh=_getByteAtIY(_getByteAtPC());}
//LD H,n
void Z80::_op_26(){_rh=_getByteAtPC();}

//LD L,B
void Z80::_op_68(){_rl=_rb;}
//LD L,C
void Z80::_op_69(){_rl=_rc;}
//LD L,D
void Z80::_op_6a(){_rl=_rd;}
//LD L,E
void Z80::_op_6b(){_rl=_re;}
//LD L,H
void Z80::_op_6c(){_rl=_rh;}
//LD L,L
void Z80::_op_6d(){/*_l=_l;*/}
//LD L,(HL)
void Z80::_op_6e(){_rl=_getByteAtHL();}
//LD L,A
void Z80::_op_6f(){_rl=_a;}
//LD L,(IX+d)
void Z80::_op_dd_6e(){_rl=_getByteAtIX(_getByteAtPC());}
//LD L,(IY+d)
void Z80::_op_fd_6e(){_rl=_getByteAtIY(_getByteAtPC());}
//LD L,n
void Z80::_op_2e(){_rl=_getByteAtPC();}

//LD (HL),B
void Z80::_op_70(){_setByteAtHL(_rb);}
//LD (HL),C
void Z80::_op_71(){_setByteAtHL(_rc);}
//LD (HL),D
void Z80::_op_72(){_setByteAtHL(_rd);}
//LD (HL),E
void Z80::_op_73(){_setByteAtHL(_re);}
//LD (HL),H
void Z80::_op_74(){_setByteAtHL(_rh);}
//LD (HL),L
void Z80::_op_75(){_setByteAtHL(_rl);}
//LD (HL),A
void Z80::_op_77(){_setByteAtHL(_a);}
//LD (HL),n
void Z80::_op_36(){_setByteAtHL(_getByteAtPC());}

//LD (BC),A
void Z80::_op_02(){_setByteAtBC(_a);}
//LD (DE),A
void Z80::_op_12(){_setByteAtDE(_a);}

//LD (IX+d),B
void Z80::_op_dd_70(){_setByteAtIX(_getByteAtPC(),_rb);}
//LD (IX+d),C
void Z80::_op_dd_71(){_setByteAtIX(_getByteAtPC(),_rc);}
//LD (IX+d),D
void Z80::_op_dd_72(){_setByteAtIX(_getByteAtPC(),_rd);}
//LD (IX+d),E
void Z80::_op_dd_73(){_setByteAtIX(_getByteAtPC(),_re);}
//LD (IX+d),H
void Z80::_op_dd_74(){_setByteAtIX(_getByteAtPC(),_rh);}
//LD (IX+d),L
void Z80::_op_dd_75(){_setByteAtIX(_getByteAtPC(),_rl);}
//LD (IX+d),A
void Z80::_op_dd_77(){_setByteAtIX(_getByteAtPC(),_a);}
//LD (IX+d),n
void Z80::_op_dd_36(){_setByteAtIX(_getByteAtPC(),_getByteAtPC());}

//LD (IY+d),B
void Z80::_op_fd_70(){_setByteAtIY(_getByteAtPC(),_rb);}
//LD (IY+d),C
void Z80::_op_fd_71(){_setByteAtIY(_getByteAtPC(),_rc);}
//LD (IY+d),D
void Z80::_op_fd_72(){_setByteAtIY(_getByteAtPC(),_rd);}
//LD (IY+d),E
void Z80::_op_fd_73(){_setByteAtIY(_getByteAtPC(),_re);}
//LD (IY+d),H
void Z80::_op_fd_74(){_setByteAtIY(_getByteAtPC(),_rh);}
//LD (IY+d),L
void Z80::_op_fd_75(){_setByteAtIY(_getByteAtPC(),_rl);}
//LD (IY+d),A
void Z80::_op_fd_77(){_setByteAtIY(_getByteAtPC(),_a);}
//LD (IY+d),n
void Z80::_op_fd_36(){_setByteAtIY(_getByteAtPC(),_getByteAtPC());}

//LD (nn),A
void Z80::_op_32(){_setByteAtAdr(_getWordAtPC(),_a);}

//LD I,A
void Z80::_op_ed_47(){_i=_a;}
//LD R,A
void Z80::_op_ed_4f(){_r=_a;}

/*
 * 16bit load
 */
//LD BC,nn
void Z80::_op_01(){_rc=_getByteAtPC(); _rb=_getByteAtPC();}
//LD BC,(nn)
void Z80::_op_ed_4b(){Word a=_getWordAtPC(); _rc=_getByteAtAdr(a); _rb=_getByteAtAdr(a+1);}

//LD DE,nn
void Z80::_op_11(){_re=_getByteAtPC(); _rd=_getByteAtPC();}
//LD DE,(nn)
void Z80::_op_ed_5b(){Word a=_getWordAtPC(); _re=_getByteAtAdr(a); _rd=_getByteAtAdr(a+1);}

//LD HL,nn
void Z80::_op_21(){_rl=_getByteAtPC(); _rh=_getByteAtPC();}
//LD HL,(nn)
void Z80::_op_2a(){Word a=_getWordAtPC(); _rl=_getByteAtAdr(a); _rh=_getByteAtAdr(a+1);}

//LD SP,HL
void Z80::_op_f9(){_sp=(_rh<<8)|_rl;}
//LD SP,IX
void Z80::_op_dd_f9(){_sp=_ix;}
//LD SP,IY
void Z80::_op_fd_f9(){_sp=_iy;}
//LD SP,nn
void Z80::_op_31(){_sp=_getWordAtPC();}
//LD SP,(nn)
void Z80::_op_ed_7b(){_sp=_getWordAtAdr(_getWordAtPC());}

//LD IX,nn
void Z80::_op_dd_21(){_ix=_getWordAtPC();}
//LD IX,(nn)
void Z80::_op_dd_2a(){_ix=_getWordAtAdr(_getWordAtPC());}

//LD IY,nn
void Z80::_op_fd_21(){_iy=_getWordAtPC();}
//LD IY,(nn)
void Z80::_op_fd_2a(){_iy=_getWordAtAdr(_getWordAtPC());}

//LD (nn),BC
void Z80::_op_ed_43(){_set2ByteAtAdr(_getWordAtPC(),_rc,_rb);}
//LD (nn),DE
void Z80::_op_ed_53(){_set2ByteAtAdr(_getWordAtPC(),_re,_rd);}
//LD (nn),HL
void Z80::_op_22(){_set2ByteAtAdr(_getWordAtPC(),_rl,_rh);}
//LD (nn),SP
void Z80::_op_ed_73(){_setWordAtAdr(_getWordAtPC(),_sp);}
//LD (nn),IX
void Z80::_op_dd_22(){_setWordAtAdr(_getWordAtPC(),_ix);}
//LD (nn),IY
void Z80::_op_fd_22(){_setWordAtAdr(_getWordAtPC(),_iy);}

//PUSH BC
//void Z80::_op_c5(){_op_push(_rc); _op_push(_rb);}
void Z80::_op_c5(){_op_push(_rb); _op_push(_rc);}
//PUSH DE
void Z80::_op_d5(){_op_push(_rd); _op_push(_re);}
//void Z80::_op_d5(){_op_push(_re); _op_push(_rd);}
//PUSH HL
void Z80::_op_e5(){_op_push(_rh); _op_push(_rl);}
//void Z80::_op_e5(){_op_push(_rl); _op_push(_rh);}
//PUSH AF
void Z80::_op_f5(){_op_push(_a); _op_push(_f);}
//void Z80::_op_f5(){_op_push(_f); _op_push(_a);}
//PUSH IX
void Z80::_op_dd_e5(){_op_push(_ix);}
//PUSH IY
void Z80::_op_fd_e5(){_op_push(_iy);}


//POP BC
void Z80::_op_c1(){_rc=_op_pop(); _rb=_op_pop();}
//void Z80::_op_c1(){_rb=_op_pop(); _rc=_op_pop();}
//POP DE
void Z80::_op_d1(){_re=_op_pop(); _rd=_op_pop();}
//void Z80::_op_d1(){_rd=_op_pop(); _re=_op_pop();}
//POP HL
void Z80::_op_e1(){_rl=_op_pop(); _rh=_op_pop();}
//void Z80::_op_e1(){_rh=_op_pop(); _rl=_op_pop();}
//POP AF
void Z80::_op_f1(){_f=_op_pop(); _a=_op_pop();}
//void Z80::_op_f1(){_a=_op_pop(); _f=_op_pop();}
//POP IX
void Z80::_op_dd_e1(){_ix=_op_pop_word();}
//POP IY
void Z80::_op_fd_e1(){_iy=_op_pop_word();}

/*
 * block transfer
 */
//LDI
void Z80::_op_ed_a0(){
    _setByteAtDE(_getByteAtHL());
    _rhl++; _rde++; _rbc--;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_C;
    _f|=(_rbc ? FLAG_MASK_V : 0);
}
//LDIR
void Z80::_op_ed_b0(){
    _curclock=0;
    do{
        _setByteAtDE(_getByteAtHL());
        _rhl++; _rde++; _rbc--;
        _curclock+=21;
    }
    while(_rbc!=0);
    _curclock-=5;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_C;
}
//LDD
void Z80::_op_ed_a8(){
    _setByteAtDE(_getByteAtHL());
    _rhl--; _rde--; _rbc--;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_C;
    _f|=(_rbc ? FLAG_MASK_V : 0);
}
//LDDR
void Z80::_op_ed_b8(){
    _curclock=0;
    do{
        _setByteAtDE(_getByteAtHL());
        _rhl--; _rde--; _rbc--;
        _curclock+=21;
    }
    while(_rbc!=0);
    _curclock-=5;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_C;
}

/*
 * block search
 */
//CPI
void Z80::_op_ed_a1(){
    Byte d=_a;
    d-=_getByteAtHL();
    _rhl++; _rbc--;
    _f&=FLAG_MASK_C;
    _f|=signFlag(d)|zeroFlag(d)|halfcarryFlag(d)|FLAG_MASK_N|(_rbc!=0 ? FLAG_MASK_V : 0);
}
//CPIR
void Z80::_op_ed_b1(){
    Byte d,tmp;
    _f&=FLAG_MASK_C;
    _curclock=0;
    do{
        d=_a; tmp=_a;
        d-=_getByteAtHL();
        _rhl++; _rbc--;
        _curclock+=21;
        if (d==0){
            _f|=signFlag(d)|zeroFlag(d)|halfcarryFlag(d)|FLAG_MASK_N|FLAG_MASK_V;
            return;
        }
    }
    while(_rbc!=0);
    _curclock-=5;
    _f|=signFlag(d)|zeroFlag(d)|halfcarryFlag(d)|FLAG_MASK_N;
}
//CPD
void Z80::_op_ed_a9(){
    Byte d=_a;
    d-=_getByteAtHL();
    _rhl--; _rbc--;
    _f&=FLAG_MASK_C;
    _f|=signFlag(d)|zeroFlag(d)|halfcarryFlag(d)|FLAG_MASK_N|(_rbc!=0 ? FLAG_MASK_V : 0);
}
//CPDR
void Z80::_op_ed_b9(){
    Byte d,tmp;
    _f&=FLAG_MASK_C;
    _curclock=0;
    do{
        d=_a; tmp=_a;
        d-=_getByteAtHL();
        _rhl--; _rbc--;
        _curclock+=21;
        if (d==0){
            _f|=signFlag(d)|zeroFlag(d)|halfcarryFlag(d)|FLAG_MASK_N|FLAG_MASK_V;
            return;
        }
    }
    while(_rbc!=0);
    _curclock-=5;
    _f|=signFlag(d)|zeroFlag(d)|halfcarryFlag(d)|FLAG_MASK_N;
}


/*
 * rotate / shift
 */
//RLCA
void Z80::_op_07(){
    Byte c=_a&0x80 ? 1 : 0;
    _a=(_a<<1)|c;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_P;
    _f|=(c ? FLAG_MASK_C : 0);
}
//RRCA
void Z80::_op_0f(){
    Byte c=_a&0x01 ? 0x80 : 0;
    _a=(_a>>1)|c;
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_P;
    _f|=(c ? FLAG_MASK_C : 0);
}
//RLA
void Z80::_op_17(){
    Byte c=_a&0x80 ? 1 : 0;
    _a=(_a<<1)|(_f&FLAG_MASK_C ? 1 : 0);
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_P;
    _f|=(c ? FLAG_MASK_C : 0);
}
//RRA
void Z80::_op_1f(){
    Byte c=_a&0x01 ? 0x80 : 0;
    _a=(_a>>1)|(_f&FLAG_MASK_C ? 0x80 : 0);
    _f&=FLAG_MASK_S|FLAG_MASK_Z|FLAG_MASK_P;
    _f|=(c ? FLAG_MASK_C : 0);
}

//RLC B
void Z80::_op_cb_00(){_rb=_op_rlc(_rb);}
//RLC C
void Z80::_op_cb_01(){_rc=_op_rlc(_rc);}
//RLC D
void Z80::_op_cb_02(){_rd=_op_rlc(_rd);}
//RLC E
void Z80::_op_cb_03(){_re=_op_rlc(_re);}
//RLC H
void Z80::_op_cb_04(){_rh=_op_rlc(_rh);}
//RLC L
void Z80::_op_cb_05(){_rl=_op_rlc(_rl);}
//RLC (HL)
void Z80::_op_cb_06(){_setByteAtHL(_op_rlc(_getByteAtHL()));}
//RLC A
void Z80::_op_cb_07(){_a=_op_rlc(_a);}
//RLC (IX+d)
void Z80::_op_dd_cb_d_06(Byte d){_setByteAtIX(d,_op_rlc(_getByteAtIX(d)));}
//RLC (IY+d)
void Z80::_op_fd_cb_d_06(Byte d){_setByteAtIY(d,_op_rlc(_getByteAtIY(d)));}

//RRC B
void Z80::_op_cb_08(){_rb=_op_rrc(_rb);}
//RRC C
void Z80::_op_cb_09(){_rc=_op_rrc(_rc);}
//RRC D
void Z80::_op_cb_0a(){_rd=_op_rrc(_rd);}
//RRC E
void Z80::_op_cb_0b(){_re=_op_rrc(_re);}
//RRC H
void Z80::_op_cb_0c(){_rh=_op_rrc(_rh);}
//RRC L
void Z80::_op_cb_0d(){_rl=_op_rrc(_rl);}
//RRC (HL)
void Z80::_op_cb_0e(){_setByteAtHL(_op_rrc(_getByteAtHL()));}
//RRC A
void Z80::_op_cb_0f(){_a=_op_rrc(_a);}
//RRC (IX+d)
void Z80::_op_dd_cb_d_0e(Byte d){_setByteAtIX(d,_op_rrc(_getByteAtIX(d)));}
//RRC (IY+d)
void Z80::_op_fd_cb_d_0e(Byte d){_setByteAtIY(d,_op_rrc(_getByteAtIY(d)));}

//RL B
void Z80::_op_cb_10(){_rb=_op_rl(_rb);}
//RL C
void Z80::_op_cb_11(){_rc=_op_rl(_rc);}
//RL D
void Z80::_op_cb_12(){_rd=_op_rl(_rd);}
//RL E
void Z80::_op_cb_13(){_re=_op_rl(_re);}
//RL H
void Z80::_op_cb_14(){_rh=_op_rl(_rh);}
//RL L
void Z80::_op_cb_15(){_rl=_op_rl(_rl);}
//RL (HL)
void Z80::_op_cb_16(){_setByteAtHL(_op_rl(_getByteAtHL()));}
//RL A
void Z80::_op_cb_17(){_a=_op_rl(_a);}
//RL (IX+d)
void Z80::_op_dd_cb_d_16(Byte d){_setByteAtIX(d,_op_rl(_getByteAtIX(d)));}
//RL (IY+d)
void Z80::_op_fd_cb_d_16(Byte d){_setByteAtIY(d,_op_rl(_getByteAtIY(d)));}

//RR B
void Z80::_op_cb_18(){_rb=_op_rr(_rb);}
//RR C
void Z80::_op_cb_19(){_rc=_op_rr(_rc);}
//RR D
void Z80::_op_cb_1a(){_rd=_op_rr(_rd);}
//RR E
void Z80::_op_cb_1b(){_re=_op_rr(_re);}
//RR H
void Z80::_op_cb_1c(){_rh=_op_rr(_rh);}
//RR L
void Z80::_op_cb_1d(){_rl=_op_rr(_rl);}
//RR (HL)
void Z80::_op_cb_1e(){_setByteAtHL(_op_rr(_getByteAtHL()));}
//RR A
void Z80::_op_cb_1f(){_a=_op_rr(_a);}
//RR (IX+d)
void Z80::_op_dd_cb_d_1e(Byte d){_setByteAtIX(d,_op_rr(_getByteAtIX(d)));}
//RR (IY+d)
void Z80::_op_fd_cb_d_1e(Byte d){_setByteAtIY(d,_op_rr(_getByteAtIY(d)));}

//SLA B
void Z80::_op_cb_20(){_rb=_op_sla(_rb);}
//SLA C
void Z80::_op_cb_21(){_rc=_op_sla(_rc);}
//SLA D
void Z80::_op_cb_22(){_rd=_op_sla(_rd);}
//SLA E
void Z80::_op_cb_23(){_re=_op_sla(_re);}
//SLA H
void Z80::_op_cb_24(){_rh=_op_sla(_rh);}
//SLA L
void Z80::_op_cb_25(){_rl=_op_sla(_rl);}
//SLA (HL)
void Z80::_op_cb_26(){_setByteAtHL(_op_sla(_getByteAtHL()));}
//SLA A
void Z80::_op_cb_27(){_a=_op_sla(_a);}
//SLA (IX+d)
void Z80::_op_dd_cb_d_26(Byte d){_setByteAtIX(d,_op_sla(_getByteAtIX(d)));}
//SLA (IY+d)
void Z80::_op_fd_cb_d_26(Byte d){_setByteAtIY(d,_op_sla(_getByteAtIY(d)));}

//SRA B
void Z80::_op_cb_28(){_rb=_op_sra(_rb);}
//SRA C
void Z80::_op_cb_29(){_rc=_op_sra(_rc);}
//SRA D
void Z80::_op_cb_2a(){_rd=_op_sra(_rd);}
//SRA E
void Z80::_op_cb_2b(){_re=_op_sra(_re);}
//SRA H
void Z80::_op_cb_2c(){_rh=_op_sra(_rh);}
//SRA L
void Z80::_op_cb_2d(){_rl=_op_sra(_rl);}
//SRA (HL)
void Z80::_op_cb_2e(){_setByteAtHL(_op_sra(_getByteAtHL()));}
//SRA A
void Z80::_op_cb_2f(){_a=_op_sra(_a);}
//SRA (IX+d)
void Z80::_op_dd_cb_d_2e(Byte d){_setByteAtIX(d,_op_sra(_getByteAtIX(d)));}
//SRA (IY+d)
void Z80::_op_fd_cb_d_2e(Byte d){_setByteAtIY(d,_op_sra(_getByteAtIY(d)));}

//SRL B
void Z80::_op_cb_38(){_rb=_op_srl(_rb);}
//SRL C
void Z80::_op_cb_39(){_rc=_op_srl(_rc);}
//SRL D
void Z80::_op_cb_3a(){_rd=_op_srl(_rd);}
//SRL E
void Z80::_op_cb_3b(){_re=_op_srl(_re);}
//SRL H
void Z80::_op_cb_3c(){_rh=_op_srl(_rh);}
//SRL L
void Z80::_op_cb_3d(){_rl=_op_srl(_rl);}
//SRL (HL)
void Z80::_op_cb_3e(){_setByteAtHL(_op_srl(_getByteAtHL()));}
//SRL A
void Z80::_op_cb_3f(){_a=_op_srl(_a);}
//SRL (IX+d)
void Z80::_op_dd_cb_d_3e(Byte d){_setByteAtIX(d,_op_srl(_getByteAtIX(d)));}
//SRL (IY+d)
void Z80::_op_fd_cb_d_3e(Byte d){_setByteAtIY(d,_op_srl(_getByteAtIY(d)));}

//RLD
void Z80::_op_ed_6f(){
    Byte hl=_getByteAtHL();
    _setByteAtHL(((_a&0x0f)>>4)|((hl&0x0f)<<4));
    _a=(_a&0xf0)|((hl&0xf0)>>4);
    _f&=FLAG_MASK_C;
    _f|=signFlag(_a)|zeroFlag(_a)|parityFlag(_a);
}
//RRD
void Z80::_op_ed_67(){
    Byte hl=_getByteAtHL();
    _setByteAtHL(((_a&0x0f)<<4)|((hl&0xf0)>>4));
    _a=(_a&0xf0)|(hl&0x0f);
    _f&=FLAG_MASK_C;
    _f|=signFlag(_a)|zeroFlag(_a)|parityFlag(_a);
}

/*
 * jump instructions
 */
//JP nn
void Z80::_op_c3(){_pc=_getWordAtPC();}
//JP C,nn
void Z80::_op_da(){Word tmp=_getWordAtPC(); if (_isC()) _pc=tmp;}
//JP NC,nn
void Z80::_op_d2(){Word tmp=_getWordAtPC(); if (_isNC()) _pc=tmp;}
//JP Z,nn
void Z80::_op_ca(){Word tmp=_getWordAtPC(); if (_isZ()) _pc=tmp;}
//JP NZ,nn
void Z80::_op_c2(){Word tmp=_getWordAtPC(); if (_isNZ()) _pc=tmp;}
//JP PE,nn
void Z80::_op_ea(){Word tmp=_getWordAtPC(); if (_isPE()) _pc=tmp;}
//JP PO,nn
void Z80::_op_e2(){Word tmp=_getWordAtPC(); if (_isPO()) _pc=tmp;}
//JP M,nn
void Z80::_op_fa(){Word tmp=_getWordAtPC(); if (_isM()) _pc=tmp;}
//JP P,nn
void Z80::_op_f2(){Word tmp=_getWordAtPC(); if (_isP()) _pc=tmp;}

//JR e
void Z80::_op_18(){_pc+=(SByte)_getByteAtPC();}
//JR C,e
void Z80::_op_38(){SByte e=_getByteAtPC(); if (_isC()){_pc+=e; _curclock=12;}}
//JR NC,e
void Z80::_op_30(){SByte e=_getByteAtPC(); if (_isNC()){_pc+=e; _curclock=12;}}
//JR Z,e
void Z80::_op_28(){SByte e=_getByteAtPC(); if (_isZ()){_pc+=e; _curclock=12;}}
//JR NZ,e
void Z80::_op_20(){SByte e=_getByteAtPC(); if (_isNZ()){_pc+=e; _curclock=12;}}

//JP (HL)
void Z80::_op_e9(){_pc=_rhl;}
//JP (IX)
void Z80::_op_dd_e9(){_pc=_ix;}
//JP (IY)
void Z80::_op_fd_e9(){_pc=_iy;}

//CALL nn
void Z80::_op_cd(){
    Word p=_getWordAtPC();
    _op_push(_pc);
    _pc=p;
}
//CALL C,nn
void Z80::_op_dc(){
    Word p=_getWordAtPC();
    if (_isC()){
        _op_push(_pc);
        _pc=p;
        _curclock=17;
    }
}
//CALL NC,nn
void Z80::_op_d4(){
    Word p=_getWordAtPC();
    if (_isNC()){
        _op_push(_pc);
        _pc=p;
        _curclock=17;
    }
}
//CALL Z,nn
void Z80::_op_cc(){
    Word p=_getWordAtPC();
    if (_isZ()){
        _op_push(_pc);
        _pc=p;
        _curclock=17;
    }
}
//CALL NZ,nn
void Z80::_op_c4(){
    Word p=_getWordAtPC();
    if (_isNZ()){
        _op_push(_pc);
        _pc=p;
        _curclock=17;
    }
}
//CALL PE,nn
void Z80::_op_ec(){
    Word p=_getWordAtPC();
    if (_isPE()){
        _op_push(_pc);
        _pc=p;
        _curclock=17;
    }
}
//CALL PO,nn
void Z80::_op_e4(){
    Word p=_getWordAtPC();
    if (_isPO()){
        _op_push(_pc);
        _pc=p;
        _curclock=17;
    }
}
//CALL M,nn
void Z80::_op_fc(){
    Word p=_getWordAtPC();
    if (_isM()){
        _op_push(_pc);
        _pc=p;
        _curclock=17;
    }
}
//CALL P,nn
void Z80::_op_f4(){
    Word p=_getWordAtPC();
    if (_isP()){
        _op_push(_pc);
        _pc=p;
        _curclock=17;
    }
}

//DJNZ e
void Z80::_op_10(){SByte e=_getByteAtPC(); if (--_rb!=0){_pc+=e; _curclock=13;}}

//RET
void Z80::_op_c9(){_pc=_op_pop_word();}
//RET C
void Z80::_op_d8(){if (_isC()){_pc=_op_pop_word(); _curclock=11;}}
//RET NC
void Z80::_op_d0(){if (_isNC()){_pc=_op_pop_word(); _curclock=11;}}
//RET Z
void Z80::_op_c8(){if (_isZ()){_pc=_op_pop_word(); _curclock=11;}}
//RET NZ
void Z80::_op_c0(){if (_isNZ()){_pc=_op_pop_word(); _curclock=11;}}
//RET PE
void Z80::_op_e8(){if (_isPE()){_pc=_op_pop_word(); _curclock=11;}}
//RET PO
void Z80::_op_e0(){if (_isPO()){_pc=_op_pop_word(); _curclock=11;}}
//RET M
void Z80::_op_f8(){if (_isM()){_pc=_op_pop_word(); _curclock=11;}}
//RET P
void Z80::_op_f0(){if (_isP()){_pc=_op_pop_word(); _curclock=11;}}
//RETI
void Z80::_op_ed_4d(){
    if (_pio){
        _pio->reti();
    }
    _pc=_op_pop_word();} // RETI with interrupt handling!
//RETN
void Z80::_op_ed_45(){_iff=_iff2;_pc=_op_pop_word();}

//RST 00H
void Z80::_op_c7(){_op_push(_pc);_pc=0x0000;}
//RST 08H
void Z80::_op_cf(){_op_push(_pc);_pc=0x0008;}
//RST 10H
void Z80::_op_d7(){_op_push(_pc);_pc=0x0010;}
//RST 18H
void Z80::_op_df(){_op_push(_pc);_pc=0x0018;}
//RST 20H
void Z80::_op_e7(){_op_push(_pc);_pc=0x0020;}
//RST 28H
void Z80::_op_ef(){_op_push(_pc);_pc=0x0028;}
//RST 30H
void Z80::_op_f7(){_op_push(_pc);_pc=0x0030;}
//RST 38H
void Z80::_op_ff(){_op_push(_pc);_pc=0x0038;}

/*
 * i/o
 */
void Z80::_op_d3(){_ioport->out(_getByteAtPC(),_a);}    //OUT (n),a ##I/Oアドレスの上位８ビットは、X1の場合はAレジスタにする
void Z80::_op_db(){_a=_ioport->in(_getByteAtPC());}     //IN A,(n)  ##I/Oアドレスの上位８ビットは、X1の場合はAレジスタにする

void Z80::_op_ed_40(){_rb=_op_in();}    //IN B,(C)
void Z80::_op_ed_41(){_op_out(_rb);}    //OUT (C),B
void Z80::_op_ed_48(){_rc=_op_in();}    //IN C,(C)
void Z80::_op_ed_49(){_op_out(_rc);}    //OUT (C),C
void Z80::_op_ed_50(){_rd=_op_in();}    //IN D,(C)
void Z80::_op_ed_51(){_op_out(_rd);}    //OUT (C),D
void Z80::_op_ed_58(){_re=_op_in();}    //IN E,(C)
void Z80::_op_ed_59(){_op_out(_re);}    //OUT (C),E
void Z80::_op_ed_60(){_rh=_op_in();}    //IN H,(C)
void Z80::_op_ed_61(){_op_out(_rh);}    //OUT (C),H
void Z80::_op_ed_68(){_rl=_op_in();}    //IN L,(C)
void Z80::_op_ed_69(){_op_out(_rl);}    //OUT (C),L
void Z80::_op_ed_78(){_a=_op_in();}     //IN A,(C)
void Z80::_op_ed_79(){_op_out(_a);}     //OUT (C),A

void Z80::_op_ed_a2(){  //INI
    _setByteAtHL(_ioport->in((Word)_rc));
    _rhl++; _rb--;
    _f&=FLAG_MASK_C;
    _f|=FLAG_MASK_N|(_rb==0 ? FLAG_MASK_Z : 0);
}
void Z80::_op_ed_a3(){  //OUTI
    _ioport->out((Word)_rc,_getByteAtHL());
    _rhl++; _rb--;
    _f&=FLAG_MASK_C;
    _f|=FLAG_MASK_N|(_rb==0 ? FLAG_MASK_Z : 0);
}
void Z80::_op_ed_aa(){  //IND
    _setByteAtHL(_ioport->in((Word)_rc));
    _rhl--; _rb--;
    _f&=FLAG_MASK_C;
    _f|=FLAG_MASK_N|(_rb==0 ? FLAG_MASK_Z : 0);
}
void Z80::_op_ed_ab(){  //OUTD
    _ioport->out((Word)_rc,_getByteAtHL());
    _rhl--; _rb--;
    _f&=FLAG_MASK_C;
    _f|=FLAG_MASK_N|(_rb==0 ? FLAG_MASK_Z : 0);
}
void Z80::_op_ed_b2(){  //INIR
    _curclock=0;
    do{
        _setByteAtHL(_ioport->in((Word)_rc));
        _rhl++; _rb--;
        _curclock+=21;
    }
    while(_rb!=0);
    _curclock-=5;
    _f&=FLAG_MASK_C;
    _f|=FLAG_MASK_N|FLAG_MASK_Z;
}
void Z80::_op_ed_b3(){  //OTIR
    _curclock=0;
    do{
        _ioport->out((Word)_rc,_getByteAtHL());
        _rhl++; _rb--;   //_rbc--;
        _curclock+=21;
    }
    while(_rb!=0);
//    while(_rbc!=0);
    _curclock-=5;
    _f&=FLAG_MASK_C;
    _f|=FLAG_MASK_N|FLAG_MASK_Z;
}
void Z80::_op_ed_ba(){  //INDR
    _curclock=0;
    do{
        _setByteAtHL(_ioport->in((Word)_rc));
        _rhl--; _rb--;
        _curclock+=21;
    }
    while(_rb!=0);
    _curclock-=5;
    _f&=FLAG_MASK_C;
    _f|=FLAG_MASK_N|FLAG_MASK_Z;
}
void Z80::_op_ed_bb(){  //OTDR
    _curclock=0;
    do{
        _ioport->out((Word)_rc,_getByteAtHL());
        _rhl--; _rb--;
        _curclock+=21;
    }
    while(_rb!=0);
    _curclock-=5;
    _f&=FLAG_MASK_C;
    _f|=FLAG_MASK_N|FLAG_MASK_Z;
}



/*
 * bit
 */
//BIT
void Z80::_op_cb_40(){_op_bit(_rb,0);}
void Z80::_op_cb_41(){_op_bit(_rc,0);}
void Z80::_op_cb_42(){_op_bit(_rd,0);}
void Z80::_op_cb_43(){_op_bit(_re,0);}
void Z80::_op_cb_44(){_op_bit(_rh,0);}
void Z80::_op_cb_45(){_op_bit(_rl,0);}
void Z80::_op_cb_46(){_op_bit(_getByteAtHL(),0);}
void Z80::_op_cb_47(){_op_bit(_a,0);}

void Z80::_op_cb_48(){_op_bit(_rb,1);}
void Z80::_op_cb_49(){_op_bit(_rc,1);}
void Z80::_op_cb_4a(){_op_bit(_rd,1);}
void Z80::_op_cb_4b(){_op_bit(_re,1);}
void Z80::_op_cb_4c(){_op_bit(_rh,1);}
void Z80::_op_cb_4d(){_op_bit(_rl,1);}
void Z80::_op_cb_4e(){_op_bit(_getByteAtHL(),1);}
void Z80::_op_cb_4f(){_op_bit(_a,1);}

void Z80::_op_cb_50(){_op_bit(_rb,2);}
void Z80::_op_cb_51(){_op_bit(_rc,2);}
void Z80::_op_cb_52(){_op_bit(_rd,2);}
void Z80::_op_cb_53(){_op_bit(_re,2);}
void Z80::_op_cb_54(){_op_bit(_rh,2);}
void Z80::_op_cb_55(){_op_bit(_rl,2);}
void Z80::_op_cb_56(){_op_bit(_getByteAtHL(),2);}
void Z80::_op_cb_57(){_op_bit(_a,2);}

void Z80::_op_cb_58(){_op_bit(_rb,3);}
void Z80::_op_cb_59(){_op_bit(_rc,3);}
void Z80::_op_cb_5a(){_op_bit(_rd,3);}
void Z80::_op_cb_5b(){_op_bit(_re,3);}
void Z80::_op_cb_5c(){_op_bit(_rh,3);}
void Z80::_op_cb_5d(){_op_bit(_rl,3);}
void Z80::_op_cb_5e(){_op_bit(_getByteAtHL(),3);}
void Z80::_op_cb_5f(){_op_bit(_a,3);}

void Z80::_op_cb_60(){_op_bit(_rb,4);}
void Z80::_op_cb_61(){_op_bit(_rc,4);}
void Z80::_op_cb_62(){_op_bit(_rd,4);}
void Z80::_op_cb_63(){_op_bit(_re,4);}
void Z80::_op_cb_64(){_op_bit(_rh,4);}
void Z80::_op_cb_65(){_op_bit(_rl,4);}
void Z80::_op_cb_66(){_op_bit(_getByteAtHL(),4);}
void Z80::_op_cb_67(){_op_bit(_a,4);}

void Z80::_op_cb_68(){_op_bit(_rb,5);}
void Z80::_op_cb_69(){_op_bit(_rc,5);}
void Z80::_op_cb_6a(){_op_bit(_rd,5);}
void Z80::_op_cb_6b(){_op_bit(_re,5);}
void Z80::_op_cb_6c(){_op_bit(_rh,5);}
void Z80::_op_cb_6d(){_op_bit(_rl,5);}
void Z80::_op_cb_6e(){_op_bit(_getByteAtHL(),5);}
void Z80::_op_cb_6f(){_op_bit(_a,5);}

void Z80::_op_cb_70(){_op_bit(_rb,6);}
void Z80::_op_cb_71(){_op_bit(_rc,6);}
void Z80::_op_cb_72(){_op_bit(_rd,6);}
void Z80::_op_cb_73(){_op_bit(_re,6);}
void Z80::_op_cb_74(){_op_bit(_rh,6);}
void Z80::_op_cb_75(){_op_bit(_rl,6);}
void Z80::_op_cb_76(){_op_bit(_getByteAtHL(),6);}
void Z80::_op_cb_77(){_op_bit(_a,6);}

void Z80::_op_cb_78(){_op_bit(_rb,7);}
void Z80::_op_cb_79(){_op_bit(_rc,7);}
void Z80::_op_cb_7a(){_op_bit(_rd,7);}
void Z80::_op_cb_7b(){_op_bit(_re,7);}
void Z80::_op_cb_7c(){_op_bit(_rh,7);}
void Z80::_op_cb_7d(){_op_bit(_rl,7);}
void Z80::_op_cb_7e(){_op_bit(_getByteAtHL(),7);}
void Z80::_op_cb_7f(){_op_bit(_a,7);}

void Z80::_op_dd_cb_d_46(Byte d){_op_bit(_getByteAtIX(d),0);}
void Z80::_op_dd_cb_d_4e(Byte d){_op_bit(_getByteAtIX(d),1);}
void Z80::_op_dd_cb_d_56(Byte d){_op_bit(_getByteAtIX(d),2);}
void Z80::_op_dd_cb_d_5e(Byte d){_op_bit(_getByteAtIX(d),3);}
void Z80::_op_dd_cb_d_66(Byte d){_op_bit(_getByteAtIX(d),4);}
void Z80::_op_dd_cb_d_6e(Byte d){_op_bit(_getByteAtIX(d),5);}
void Z80::_op_dd_cb_d_76(Byte d){_op_bit(_getByteAtIX(d),6);}
void Z80::_op_dd_cb_d_7e(Byte d){_op_bit(_getByteAtIX(d),7);}

void Z80::_op_fd_cb_d_46(Byte d){_op_bit(_getByteAtIY(d),0);}
void Z80::_op_fd_cb_d_4e(Byte d){_op_bit(_getByteAtIY(d),1);}
void Z80::_op_fd_cb_d_56(Byte d){_op_bit(_getByteAtIY(d),2);}
void Z80::_op_fd_cb_d_5e(Byte d){_op_bit(_getByteAtIY(d),3);}
void Z80::_op_fd_cb_d_66(Byte d){_op_bit(_getByteAtIY(d),4);}
void Z80::_op_fd_cb_d_6e(Byte d){_op_bit(_getByteAtIY(d),5);}
void Z80::_op_fd_cb_d_76(Byte d){_op_bit(_getByteAtIY(d),6);}
void Z80::_op_fd_cb_d_7e(Byte d){_op_bit(_getByteAtIY(d),7);}

//RES
void Z80::_op_cb_80(){_rb=_op_res(_rb,0);}
void Z80::_op_cb_81(){_rc=_op_res(_rc,0);}
void Z80::_op_cb_82(){_rd=_op_res(_rd,0);}
void Z80::_op_cb_83(){_re=_op_res(_re,0);}
void Z80::_op_cb_84(){_rh=_op_res(_rh,0);}
void Z80::_op_cb_85(){_rl=_op_res(_rl,0);}
void Z80::_op_cb_86(){_setByteAtHL(_op_res(_getByteAtHL(),0));}
void Z80::_op_cb_87(){_a=_op_res(_a,0);}

void Z80::_op_cb_88(){_rb=_op_res(_rb,1);}
void Z80::_op_cb_89(){_rc=_op_res(_rc,1);}
void Z80::_op_cb_8a(){_rd=_op_res(_rd,1);}
void Z80::_op_cb_8b(){_re=_op_res(_re,1);}
void Z80::_op_cb_8c(){_rh=_op_res(_rh,1);}
void Z80::_op_cb_8d(){_rl=_op_res(_rl,1);}
void Z80::_op_cb_8e(){_setByteAtHL(_op_res(_getByteAtHL(),1));}
void Z80::_op_cb_8f(){_a=_op_res(_a,1);}

void Z80::_op_cb_90(){_rb=_op_res(_rb,2);}
void Z80::_op_cb_91(){_rc=_op_res(_rc,2);}
void Z80::_op_cb_92(){_rd=_op_res(_rd,2);}
void Z80::_op_cb_93(){_re=_op_res(_re,2);}
void Z80::_op_cb_94(){_rh=_op_res(_rh,2);}
void Z80::_op_cb_95(){_rl=_op_res(_rl,2);}
void Z80::_op_cb_96(){_setByteAtHL(_op_res(_getByteAtHL(),2));}
void Z80::_op_cb_97(){_a=_op_res(_a,2);}

void Z80::_op_cb_98(){_rb=_op_res(_rb,3);}
void Z80::_op_cb_99(){_rc=_op_res(_rc,3);}
void Z80::_op_cb_9a(){_rd=_op_res(_rd,3);}
void Z80::_op_cb_9b(){_re=_op_res(_re,3);}
void Z80::_op_cb_9c(){_rh=_op_res(_rh,3);}
void Z80::_op_cb_9d(){_rl=_op_res(_rl,3);}
void Z80::_op_cb_9e(){_setByteAtHL(_op_res(_getByteAtHL(),3));}
void Z80::_op_cb_9f(){_a=_op_res(_a,3);}

void Z80::_op_cb_a0(){_rb=_op_res(_rb,4);}
void Z80::_op_cb_a1(){_rc=_op_res(_rc,4);}
void Z80::_op_cb_a2(){_rd=_op_res(_rd,4);}
void Z80::_op_cb_a3(){_re=_op_res(_re,4);}
void Z80::_op_cb_a4(){_rh=_op_res(_rh,4);}
void Z80::_op_cb_a5(){_rl=_op_res(_rl,4);}
void Z80::_op_cb_a6(){_setByteAtHL(_op_res(_getByteAtHL(),4));}
void Z80::_op_cb_a7(){_a=_op_res(_a,4);}

void Z80::_op_cb_a8(){_rb=_op_res(_rb,5);}
void Z80::_op_cb_a9(){_rc=_op_res(_rc,5);}
void Z80::_op_cb_aa(){_rd=_op_res(_rd,5);}
void Z80::_op_cb_ab(){_re=_op_res(_re,5);}
void Z80::_op_cb_ac(){_rh=_op_res(_rh,5);}
void Z80::_op_cb_ad(){_rl=_op_res(_rl,5);}
void Z80::_op_cb_ae(){_setByteAtHL(_op_res(_getByteAtHL(),5));}
void Z80::_op_cb_af(){_a=_op_res(_a,5);}

void Z80::_op_cb_b0(){_rb=_op_res(_rb,6);}
void Z80::_op_cb_b1(){_rc=_op_res(_rc,6);}
void Z80::_op_cb_b2(){_rd=_op_res(_rd,6);}
void Z80::_op_cb_b3(){_re=_op_res(_re,6);}
void Z80::_op_cb_b4(){_rh=_op_res(_rh,6);}
void Z80::_op_cb_b5(){_rl=_op_res(_rl,6);}
void Z80::_op_cb_b6(){_setByteAtHL(_op_res(_getByteAtHL(),6));}
void Z80::_op_cb_b7(){_a=_op_res(_a,6);}

void Z80::_op_cb_b8(){_rb=_op_res(_rb,7);}
void Z80::_op_cb_b9(){_rc=_op_res(_rc,7);}
void Z80::_op_cb_ba(){_rd=_op_res(_rd,7);}
void Z80::_op_cb_bb(){_re=_op_res(_re,7);}
void Z80::_op_cb_bc(){_rh=_op_res(_rh,7);}
void Z80::_op_cb_bd(){_rl=_op_res(_rl,7);}
void Z80::_op_cb_be(){_setByteAtHL(_op_res(_getByteAtHL(),7));}
void Z80::_op_cb_bf(){_a=_op_res(_a,7);}

void Z80::_op_dd_cb_d_86(Byte d){_setByteAtIX(d,_op_res(_getByteAtIX(d),0));}
void Z80::_op_dd_cb_d_8e(Byte d){_setByteAtIX(d,_op_res(_getByteAtIX(d),1));}
void Z80::_op_dd_cb_d_96(Byte d){_setByteAtIX(d,_op_res(_getByteAtIX(d),2));}
void Z80::_op_dd_cb_d_9e(Byte d){_setByteAtIX(d,_op_res(_getByteAtIX(d),3));}
void Z80::_op_dd_cb_d_a6(Byte d){_setByteAtIX(d,_op_res(_getByteAtIX(d),4));}
void Z80::_op_dd_cb_d_ae(Byte d){_setByteAtIX(d,_op_res(_getByteAtIX(d),5));}
void Z80::_op_dd_cb_d_b6(Byte d){_setByteAtIX(d,_op_res(_getByteAtIX(d),6));}
void Z80::_op_dd_cb_d_be(Byte d){_setByteAtIX(d,_op_res(_getByteAtIX(d),7));}

void Z80::_op_fd_cb_d_86(Byte d){_setByteAtIY(d,_op_res(_getByteAtIY(d),0));}
void Z80::_op_fd_cb_d_8e(Byte d){_setByteAtIY(d,_op_res(_getByteAtIY(d),1));}
void Z80::_op_fd_cb_d_96(Byte d){_setByteAtIY(d,_op_res(_getByteAtIY(d),2));}
void Z80::_op_fd_cb_d_9e(Byte d){_setByteAtIY(d,_op_res(_getByteAtIY(d),3));}
void Z80::_op_fd_cb_d_a6(Byte d){_setByteAtIY(d,_op_res(_getByteAtIY(d),4));}
void Z80::_op_fd_cb_d_ae(Byte d){_setByteAtIY(d,_op_res(_getByteAtIY(d),5));}
void Z80::_op_fd_cb_d_b6(Byte d){_setByteAtIY(d,_op_res(_getByteAtIY(d),6));}
void Z80::_op_fd_cb_d_be(Byte d){_setByteAtIY(d,_op_res(_getByteAtIY(d),7));}

//SET
void Z80::_op_cb_c0(){_rb=_op_set(_rb,0);}
void Z80::_op_cb_c1(){_rc=_op_set(_rc,0);}
void Z80::_op_cb_c2(){_rd=_op_set(_rd,0);}
void Z80::_op_cb_c3(){_re=_op_set(_re,0);}
void Z80::_op_cb_c4(){_rh=_op_set(_rh,0);}
void Z80::_op_cb_c5(){_rl=_op_set(_rl,0);}
void Z80::_op_cb_c6(){_setByteAtHL(_op_set(_getByteAtHL(),0));}
void Z80::_op_cb_c7(){_a=_op_set(_a,0);}

void Z80::_op_cb_c8(){_rb=_op_set(_rb,1);}
void Z80::_op_cb_c9(){_rc=_op_set(_rc,1);}
void Z80::_op_cb_ca(){_rd=_op_set(_rd,1);}
void Z80::_op_cb_cb(){_re=_op_set(_re,1);}
void Z80::_op_cb_cc(){_rh=_op_set(_rh,1);}
void Z80::_op_cb_cd(){_rl=_op_set(_rl,1);}
void Z80::_op_cb_ce(){_setByteAtHL(_op_set(_getByteAtHL(),1));}
void Z80::_op_cb_cf(){_a=_op_set(_a,1);}

void Z80::_op_cb_d0(){_rb=_op_set(_rb,2);}
void Z80::_op_cb_d1(){_rc=_op_set(_rc,2);}
void Z80::_op_cb_d2(){_rd=_op_set(_rd,2);}
void Z80::_op_cb_d3(){_re=_op_set(_re,2);}
void Z80::_op_cb_d4(){_rh=_op_set(_rh,2);}
void Z80::_op_cb_d5(){_rl=_op_set(_rl,2);}
void Z80::_op_cb_d6(){_setByteAtHL(_op_set(_getByteAtHL(),2));}
void Z80::_op_cb_d7(){_a=_op_set(_a,2);}

void Z80::_op_cb_d8(){_rb=_op_set(_rb,3);}
void Z80::_op_cb_d9(){_rc=_op_set(_rc,3);}
void Z80::_op_cb_da(){_rd=_op_set(_rd,3);}
void Z80::_op_cb_db(){_re=_op_set(_re,3);}
void Z80::_op_cb_dc(){_rh=_op_set(_rh,3);}
void Z80::_op_cb_dd(){_rl=_op_set(_rl,3);}
void Z80::_op_cb_de(){_setByteAtHL(_op_set(_getByteAtHL(),3));}
void Z80::_op_cb_df(){_a=_op_set(_a,3);}

void Z80::_op_cb_e0(){_rb=_op_set(_rb,4);}
void Z80::_op_cb_e1(){_rc=_op_set(_rc,4);}
void Z80::_op_cb_e2(){_rd=_op_set(_rd,4);}
void Z80::_op_cb_e3(){_re=_op_set(_re,4);}
void Z80::_op_cb_e4(){_rh=_op_set(_rh,4);}
void Z80::_op_cb_e5(){_rl=_op_set(_rl,4);}
void Z80::_op_cb_e6(){_setByteAtHL(_op_set(_getByteAtHL(),4));}
void Z80::_op_cb_e7(){_a=_op_set(_a,4);}

void Z80::_op_cb_e8(){_rb=_op_set(_rb,5);}
void Z80::_op_cb_e9(){_rc=_op_set(_rc,5);}
void Z80::_op_cb_ea(){_rd=_op_set(_rd,5);}
void Z80::_op_cb_eb(){_re=_op_set(_re,5);}
void Z80::_op_cb_ec(){_rh=_op_set(_rh,5);}
void Z80::_op_cb_ed(){_rl=_op_set(_rl,5);}
void Z80::_op_cb_ee(){_setByteAtHL(_op_set(_getByteAtHL(),5));}
void Z80::_op_cb_ef(){_a=_op_set(_a,5);}

void Z80::_op_cb_f0(){_rb=_op_set(_rb,6);}
void Z80::_op_cb_f1(){_rc=_op_set(_rc,6);}
void Z80::_op_cb_f2(){_rd=_op_set(_rd,6);}
void Z80::_op_cb_f3(){_re=_op_set(_re,6);}
void Z80::_op_cb_f4(){_rh=_op_set(_rh,6);}
void Z80::_op_cb_f5(){_rl=_op_set(_rl,6);}
void Z80::_op_cb_f6(){_setByteAtHL(_op_set(_getByteAtHL(),6));}
void Z80::_op_cb_f7(){_a=_op_set(_a,6);}

void Z80::_op_cb_f8(){_rb=_op_set(_rb,7);}
void Z80::_op_cb_f9(){_rc=_op_set(_rc,7);}
void Z80::_op_cb_fa(){_rd=_op_set(_rd,7);}
void Z80::_op_cb_fb(){_re=_op_set(_re,7);}
void Z80::_op_cb_fc(){_rh=_op_set(_rh,7);}
void Z80::_op_cb_fd(){_rl=_op_set(_rl,7);}
void Z80::_op_cb_fe(){_setByteAtHL(_op_set(_getByteAtHL(),7));}
void Z80::_op_cb_ff(){_a=_op_set(_a,7);}

void Z80::_op_dd_cb_d_c6(Byte d){_setByteAtIX(d,_op_set(_getByteAtIX(d),0));}
void Z80::_op_dd_cb_d_ce(Byte d){_setByteAtIX(d,_op_set(_getByteAtIX(d),1));}
void Z80::_op_dd_cb_d_d6(Byte d){_setByteAtIX(d,_op_set(_getByteAtIX(d),2));}
void Z80::_op_dd_cb_d_de(Byte d){_setByteAtIX(d,_op_set(_getByteAtIX(d),3));}
void Z80::_op_dd_cb_d_e6(Byte d){_setByteAtIX(d,_op_set(_getByteAtIX(d),4));}
void Z80::_op_dd_cb_d_ee(Byte d){_setByteAtIX(d,_op_set(_getByteAtIX(d),5));}
void Z80::_op_dd_cb_d_f6(Byte d){_setByteAtIX(d,_op_set(_getByteAtIX(d),6));}
void Z80::_op_dd_cb_d_fe(Byte d){_setByteAtIX(d,_op_set(_getByteAtIX(d),7));}

void Z80::_op_fd_cb_d_c6(Byte d){_setByteAtIY(d,_op_set(_getByteAtIY(d),0));}
void Z80::_op_fd_cb_d_ce(Byte d){_setByteAtIY(d,_op_set(_getByteAtIY(d),1));}
void Z80::_op_fd_cb_d_d6(Byte d){_setByteAtIY(d,_op_set(_getByteAtIY(d),2));}
void Z80::_op_fd_cb_d_de(Byte d){_setByteAtIY(d,_op_set(_getByteAtIY(d),3));}
void Z80::_op_fd_cb_d_e6(Byte d){_setByteAtIY(d,_op_set(_getByteAtIY(d),4));}
void Z80::_op_fd_cb_d_ee(Byte d){_setByteAtIY(d,_op_set(_getByteAtIY(d),5));}
void Z80::_op_fd_cb_d_f6(Byte d){_setByteAtIY(d,_op_set(_getByteAtIY(d),6));}
void Z80::_op_fd_cb_d_fe(Byte d){_setByteAtIY(d,_op_set(_getByteAtIY(d),7));}




/*
 * opecode table
 */
Z80::OpFuncPtr Z80::_opfunc[]={
    &Z80::_op_00,&Z80::_op_01,&Z80::_op_02,&Z80::_op_03,&Z80::_op_04,&Z80::_op_05,&Z80::_op_06,&Z80::_op_07,
    &Z80::_op_08,&Z80::_op_09,&Z80::_op_0a,&Z80::_op_0b,&Z80::_op_0c,&Z80::_op_0d,&Z80::_op_0e,&Z80::_op_0f,
    &Z80::_op_10,&Z80::_op_11,&Z80::_op_12,&Z80::_op_13,&Z80::_op_14,&Z80::_op_15,&Z80::_op_16,&Z80::_op_17,
    &Z80::_op_18,&Z80::_op_19,&Z80::_op_1a,&Z80::_op_1b,&Z80::_op_1c,&Z80::_op_1d,&Z80::_op_1e,&Z80::_op_1f,
    &Z80::_op_20,&Z80::_op_21,&Z80::_op_22,&Z80::_op_23,&Z80::_op_24,&Z80::_op_25,&Z80::_op_26,&Z80::_op_27,
    &Z80::_op_28,&Z80::_op_29,&Z80::_op_2a,&Z80::_op_2b,&Z80::_op_2c,&Z80::_op_2d,&Z80::_op_2e,&Z80::_op_2f,
    &Z80::_op_30,&Z80::_op_31,&Z80::_op_32,&Z80::_op_33,&Z80::_op_34,&Z80::_op_35,&Z80::_op_36,&Z80::_op_37,
    &Z80::_op_38,&Z80::_op_39,&Z80::_op_3a,&Z80::_op_3b,&Z80::_op_3c,&Z80::_op_3d,&Z80::_op_3e,&Z80::_op_3f,
    &Z80::_op_40,&Z80::_op_41,&Z80::_op_42,&Z80::_op_43,&Z80::_op_44,&Z80::_op_45,&Z80::_op_46,&Z80::_op_47,
    &Z80::_op_48,&Z80::_op_49,&Z80::_op_4a,&Z80::_op_4b,&Z80::_op_4c,&Z80::_op_4d,&Z80::_op_4e,&Z80::_op_4f,
    &Z80::_op_50,&Z80::_op_51,&Z80::_op_52,&Z80::_op_53,&Z80::_op_54,&Z80::_op_55,&Z80::_op_56,&Z80::_op_57,
    &Z80::_op_58,&Z80::_op_59,&Z80::_op_5a,&Z80::_op_5b,&Z80::_op_5c,&Z80::_op_5d,&Z80::_op_5e,&Z80::_op_5f,
    &Z80::_op_60,&Z80::_op_61,&Z80::_op_62,&Z80::_op_63,&Z80::_op_64,&Z80::_op_65,&Z80::_op_66,&Z80::_op_67,
    &Z80::_op_68,&Z80::_op_69,&Z80::_op_6a,&Z80::_op_6b,&Z80::_op_6c,&Z80::_op_6d,&Z80::_op_6e,&Z80::_op_6f,
    &Z80::_op_70,&Z80::_op_71,&Z80::_op_72,&Z80::_op_73,&Z80::_op_74,&Z80::_op_75,&Z80::_op_76,&Z80::_op_77,
    &Z80::_op_78,&Z80::_op_79,&Z80::_op_7a,&Z80::_op_7b,&Z80::_op_7c,&Z80::_op_7d,&Z80::_op_7e,&Z80::_op_7f,
    &Z80::_op_80,&Z80::_op_81,&Z80::_op_82,&Z80::_op_83,&Z80::_op_84,&Z80::_op_85,&Z80::_op_86,&Z80::_op_87,
    &Z80::_op_88,&Z80::_op_89,&Z80::_op_8a,&Z80::_op_8b,&Z80::_op_8c,&Z80::_op_8d,&Z80::_op_8e,&Z80::_op_8f,
    &Z80::_op_90,&Z80::_op_91,&Z80::_op_92,&Z80::_op_93,&Z80::_op_94,&Z80::_op_95,&Z80::_op_96,&Z80::_op_97,
    &Z80::_op_98,&Z80::_op_99,&Z80::_op_9a,&Z80::_op_9b,&Z80::_op_9c,&Z80::_op_9d,&Z80::_op_9e,&Z80::_op_9f,
    &Z80::_op_a0,&Z80::_op_a1,&Z80::_op_a2,&Z80::_op_a3,&Z80::_op_a4,&Z80::_op_a5,&Z80::_op_a6,&Z80::_op_a7,
    &Z80::_op_a8,&Z80::_op_a9,&Z80::_op_aa,&Z80::_op_ab,&Z80::_op_ac,&Z80::_op_ad,&Z80::_op_ae,&Z80::_op_af,
    &Z80::_op_b0,&Z80::_op_b1,&Z80::_op_b2,&Z80::_op_b3,&Z80::_op_b4,&Z80::_op_b5,&Z80::_op_b6,&Z80::_op_b7,
    &Z80::_op_b8,&Z80::_op_b9,&Z80::_op_ba,&Z80::_op_bb,&Z80::_op_bc,&Z80::_op_bd,&Z80::_op_be,&Z80::_op_bf,
    &Z80::_op_c0,&Z80::_op_c1,&Z80::_op_c2,&Z80::_op_c3,&Z80::_op_c4,&Z80::_op_c5,&Z80::_op_c6,&Z80::_op_c7,
    &Z80::_op_c8,&Z80::_op_c9,&Z80::_op_ca,&Z80::_op_cb,&Z80::_op_cc,&Z80::_op_cd,&Z80::_op_ce,&Z80::_op_cf,
    &Z80::_op_d0,&Z80::_op_d1,&Z80::_op_d2,&Z80::_op_d3,&Z80::_op_d4,&Z80::_op_d5,&Z80::_op_d6,&Z80::_op_d7,
    &Z80::_op_d8,&Z80::_op_d9,&Z80::_op_da,&Z80::_op_db,&Z80::_op_dc,&Z80::_op_dd,&Z80::_op_de,&Z80::_op_df,
    &Z80::_op_e0,&Z80::_op_e1,&Z80::_op_e2,&Z80::_op_e3,&Z80::_op_e4,&Z80::_op_e5,&Z80::_op_e6,&Z80::_op_e7,
    &Z80::_op_e8,&Z80::_op_e9,&Z80::_op_ea,&Z80::_op_eb,&Z80::_op_ec,&Z80::_op_ed,&Z80::_op_ee,&Z80::_op_ef,
    &Z80::_op_f0,&Z80::_op_f1,&Z80::_op_f2,&Z80::_op_f3,&Z80::_op_f4,&Z80::_op_f5,&Z80::_op_f6,&Z80::_op_f7,
    &Z80::_op_f8,&Z80::_op_f9,&Z80::_op_fa,&Z80::_op_fb,&Z80::_op_fc,&Z80::_op_fd,&Z80::_op_fe,&Z80::_op_ff,
};

Z80::OpFuncPtr Z80::_opfunc_cb[]={
    &Z80::_op_cb_00,&Z80::_op_cb_01,&Z80::_op_cb_02,&Z80::_op_cb_03,&Z80::_op_cb_04,&Z80::_op_cb_05,&Z80::_op_cb_06,&Z80::_op_cb_07,
    &Z80::_op_cb_08,&Z80::_op_cb_09,&Z80::_op_cb_0a,&Z80::_op_cb_0b,&Z80::_op_cb_0c,&Z80::_op_cb_0d,&Z80::_op_cb_0e,&Z80::_op_cb_0f,
    &Z80::_op_cb_10,&Z80::_op_cb_11,&Z80::_op_cb_12,&Z80::_op_cb_13,&Z80::_op_cb_14,&Z80::_op_cb_15,&Z80::_op_cb_16,&Z80::_op_cb_17,
    &Z80::_op_cb_18,&Z80::_op_cb_19,&Z80::_op_cb_1a,&Z80::_op_cb_1b,&Z80::_op_cb_1c,&Z80::_op_cb_1d,&Z80::_op_cb_1e,&Z80::_op_cb_1f,
    &Z80::_op_cb_20,&Z80::_op_cb_21,&Z80::_op_cb_22,&Z80::_op_cb_23,&Z80::_op_cb_24,&Z80::_op_cb_25,&Z80::_op_cb_26,&Z80::_op_cb_27,
    &Z80::_op_cb_28,&Z80::_op_cb_29,&Z80::_op_cb_2a,&Z80::_op_cb_2b,&Z80::_op_cb_2c,&Z80::_op_cb_2d,&Z80::_op_cb_2e,&Z80::_op_cb_2f,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_cb_38,&Z80::_op_cb_39,&Z80::_op_cb_3a,&Z80::_op_cb_3b,&Z80::_op_cb_3c,&Z80::_op_cb_3d,&Z80::_op_cb_3e,&Z80::_op_cb_3f,
    &Z80::_op_cb_40,&Z80::_op_cb_41,&Z80::_op_cb_42,&Z80::_op_cb_43,&Z80::_op_cb_44,&Z80::_op_cb_45,&Z80::_op_cb_46,&Z80::_op_cb_47,
    &Z80::_op_cb_48,&Z80::_op_cb_49,&Z80::_op_cb_4a,&Z80::_op_cb_4b,&Z80::_op_cb_4c,&Z80::_op_cb_4d,&Z80::_op_cb_4e,&Z80::_op_cb_4f,
    &Z80::_op_cb_50,&Z80::_op_cb_51,&Z80::_op_cb_52,&Z80::_op_cb_53,&Z80::_op_cb_54,&Z80::_op_cb_55,&Z80::_op_cb_56,&Z80::_op_cb_57,
    &Z80::_op_cb_58,&Z80::_op_cb_59,&Z80::_op_cb_5a,&Z80::_op_cb_5b,&Z80::_op_cb_5c,&Z80::_op_cb_5d,&Z80::_op_cb_5e,&Z80::_op_cb_5f,
    &Z80::_op_cb_60,&Z80::_op_cb_61,&Z80::_op_cb_62,&Z80::_op_cb_63,&Z80::_op_cb_64,&Z80::_op_cb_65,&Z80::_op_cb_66,&Z80::_op_cb_67,
    &Z80::_op_cb_68,&Z80::_op_cb_69,&Z80::_op_cb_6a,&Z80::_op_cb_6b,&Z80::_op_cb_6c,&Z80::_op_cb_6d,&Z80::_op_cb_6e,&Z80::_op_cb_6f,
    &Z80::_op_cb_70,&Z80::_op_cb_71,&Z80::_op_cb_72,&Z80::_op_cb_73,&Z80::_op_cb_74,&Z80::_op_cb_75,&Z80::_op_cb_76,&Z80::_op_cb_77,
    &Z80::_op_cb_78,&Z80::_op_cb_79,&Z80::_op_cb_7a,&Z80::_op_cb_7b,&Z80::_op_cb_7c,&Z80::_op_cb_7d,&Z80::_op_cb_7e,&Z80::_op_cb_7f,
    &Z80::_op_cb_80,&Z80::_op_cb_81,&Z80::_op_cb_82,&Z80::_op_cb_83,&Z80::_op_cb_84,&Z80::_op_cb_85,&Z80::_op_cb_86,&Z80::_op_cb_87,
    &Z80::_op_cb_88,&Z80::_op_cb_89,&Z80::_op_cb_8a,&Z80::_op_cb_8b,&Z80::_op_cb_8c,&Z80::_op_cb_8d,&Z80::_op_cb_8e,&Z80::_op_cb_8f,
    &Z80::_op_cb_90,&Z80::_op_cb_91,&Z80::_op_cb_92,&Z80::_op_cb_93,&Z80::_op_cb_94,&Z80::_op_cb_95,&Z80::_op_cb_96,&Z80::_op_cb_97,
    &Z80::_op_cb_98,&Z80::_op_cb_99,&Z80::_op_cb_9a,&Z80::_op_cb_9b,&Z80::_op_cb_9c,&Z80::_op_cb_9d,&Z80::_op_cb_9e,&Z80::_op_cb_9f,
    &Z80::_op_cb_a0,&Z80::_op_cb_a1,&Z80::_op_cb_a2,&Z80::_op_cb_a3,&Z80::_op_cb_a4,&Z80::_op_cb_a5,&Z80::_op_cb_a6,&Z80::_op_cb_a7,
    &Z80::_op_cb_a8,&Z80::_op_cb_a9,&Z80::_op_cb_aa,&Z80::_op_cb_ab,&Z80::_op_cb_ac,&Z80::_op_cb_ad,&Z80::_op_cb_ae,&Z80::_op_cb_af,
    &Z80::_op_cb_b0,&Z80::_op_cb_b1,&Z80::_op_cb_b2,&Z80::_op_cb_b3,&Z80::_op_cb_b4,&Z80::_op_cb_b5,&Z80::_op_cb_b6,&Z80::_op_cb_b7,
    &Z80::_op_cb_b8,&Z80::_op_cb_b9,&Z80::_op_cb_ba,&Z80::_op_cb_bb,&Z80::_op_cb_bc,&Z80::_op_cb_bd,&Z80::_op_cb_be,&Z80::_op_cb_bf,
    &Z80::_op_cb_c0,&Z80::_op_cb_c1,&Z80::_op_cb_c2,&Z80::_op_cb_c3,&Z80::_op_cb_c4,&Z80::_op_cb_c5,&Z80::_op_cb_c6,&Z80::_op_cb_c7,
    &Z80::_op_cb_c8,&Z80::_op_cb_c9,&Z80::_op_cb_ca,&Z80::_op_cb_cb,&Z80::_op_cb_cc,&Z80::_op_cb_cd,&Z80::_op_cb_ce,&Z80::_op_cb_cf,
    &Z80::_op_cb_d0,&Z80::_op_cb_d1,&Z80::_op_cb_d2,&Z80::_op_cb_d3,&Z80::_op_cb_d4,&Z80::_op_cb_d5,&Z80::_op_cb_d6,&Z80::_op_cb_d7,
    &Z80::_op_cb_d8,&Z80::_op_cb_d9,&Z80::_op_cb_da,&Z80::_op_cb_db,&Z80::_op_cb_dc,&Z80::_op_cb_cb,&Z80::_op_cb_de,&Z80::_op_cb_df,
    &Z80::_op_cb_e0,&Z80::_op_cb_e1,&Z80::_op_cb_e2,&Z80::_op_cb_e3,&Z80::_op_cb_e4,&Z80::_op_cb_e5,&Z80::_op_cb_e6,&Z80::_op_cb_e7,
    &Z80::_op_cb_e8,&Z80::_op_cb_e9,&Z80::_op_cb_ea,&Z80::_op_cb_eb,&Z80::_op_cb_ec,&Z80::_op_cb_ed,&Z80::_op_cb_ee,&Z80::_op_cb_ef,
    &Z80::_op_cb_f0,&Z80::_op_cb_f1,&Z80::_op_cb_f2,&Z80::_op_cb_f3,&Z80::_op_cb_f4,&Z80::_op_cb_f5,&Z80::_op_cb_f6,&Z80::_op_cb_f7,
    &Z80::_op_cb_f8,&Z80::_op_cb_f9,&Z80::_op_cb_fa,&Z80::_op_cb_fb,&Z80::_op_cb_fc,&Z80::_op_cb_fd,&Z80::_op_cb_fe,&Z80::_op_cb_ff,
};

Z80::OpFuncPtr Z80::_opfunc_dd[]={
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_dd_09,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_dd_19,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_dd_21,&Z80::_op_dd_22,&Z80::_op_dd_23,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_dd_29,&Z80::_op_dd_2a,&Z80::_op_dd_2b,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_34,&Z80::_op_dd_35,&Z80::_op_dd_36,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_dd_39,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_46,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_4e,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_56,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_5e,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_66,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_6e,&Z80::_op_undef,
    &Z80::_op_dd_70,&Z80::_op_dd_71,&Z80::_op_dd_72,&Z80::_op_dd_73,&Z80::_op_dd_74,&Z80::_op_dd_75,&Z80::_op_undef,&Z80::_op_dd_77,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_7e,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_86,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_8e,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_96,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_9e,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_a6,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_ae,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_b6,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_be,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_dd_cb,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_dd_e1,&Z80::_op_undef,&Z80::_op_dd_e3,&Z80::_op_undef,&Z80::_op_dd_e5,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_dd_e9,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_dd_f9,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
};

Z80::OpFuncPtr Z80::_opfunc_ed[]={
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_ed_40,&Z80::_op_ed_41,&Z80::_op_ed_42,&Z80::_op_ed_43,&Z80::_op_ed_44,&Z80::_op_ed_45,&Z80::_op_ed_46,&Z80::_op_ed_47,
    &Z80::_op_ed_48,&Z80::_op_ed_49,&Z80::_op_ed_4a,&Z80::_op_ed_4b,&Z80::_op_undef,&Z80::_op_ed_4d,&Z80::_op_undef,&Z80::_op_ed_4f,
    &Z80::_op_ed_50,&Z80::_op_ed_51,&Z80::_op_ed_52,&Z80::_op_ed_53,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_ed_56,&Z80::_op_ed_57,
    &Z80::_op_ed_58,&Z80::_op_ed_59,&Z80::_op_ed_5a,&Z80::_op_ed_5b,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_ed_5e,&Z80::_op_ed_5f,
    &Z80::_op_ed_60,&Z80::_op_ed_61,&Z80::_op_ed_62,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_ed_67,
    &Z80::_op_ed_68,&Z80::_op_ed_69,&Z80::_op_ed_6a,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_ed_6f,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_ed_72,&Z80::_op_ed_73,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_ed_78,&Z80::_op_ed_79,&Z80::_op_ed_7a,&Z80::_op_ed_7b,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_ed_a0,&Z80::_op_ed_a1,&Z80::_op_ed_a2,&Z80::_op_ed_a3,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_ed_a8,&Z80::_op_ed_a9,&Z80::_op_ed_aa,&Z80::_op_ed_ab,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_ed_b0,&Z80::_op_ed_b1,&Z80::_op_ed_b2,&Z80::_op_ed_b3,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_ed_b8,&Z80::_op_ed_b9,&Z80::_op_ed_ba,&Z80::_op_ed_bb,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
};

Z80::OpFuncPtr Z80::_opfunc_fd[]={
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_fd_09,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_fd_19,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_fd_21,&Z80::_op_fd_22,&Z80::_op_fd_23,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_fd_29,&Z80::_op_fd_2a,&Z80::_op_fd_2b,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_34,&Z80::_op_fd_35,&Z80::_op_fd_36,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_fd_39,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_46,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_4e,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_56,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_5e,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_66,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_6e,&Z80::_op_undef,
    &Z80::_op_fd_70,&Z80::_op_fd_71,&Z80::_op_fd_72,&Z80::_op_fd_73,&Z80::_op_fd_74,&Z80::_op_fd_75,&Z80::_op_undef,&Z80::_op_fd_77,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_7e,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_86,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_8e,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_96,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_9e,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_a6,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_ae,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_b6,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_be,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_fd_cb,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_fd_e1,&Z80::_op_undef,&Z80::_op_fd_e3,&Z80::_op_undef,&Z80::_op_fd_e5,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_fd_e9,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
    &Z80::_op_undef,&Z80::_op_fd_f9,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,&Z80::_op_undef,
};


/*
 * clock per instruction
 */

Byte Z80::_clock[]={04,10,07,06,04,04,07,04,04,11,07,06,04,04,07,04,
    8,10,07,06,04,04,07,04,12,11,07,06,04,04,07,04,
    07,10,16,06,04,04,07,04,07,11,16,06,04,04,07,04,
    07,10,13,06,11,11,10,04,07,11,13,06,04,04,07,04,
    04,04,04,04,04,04,07,04,04,04,04,04,04,04,07,04,
    04,04,04,04,04,04,07,04,04,04,04,04,04,04,07,04,
    04,04,04,04,04,04,07,04,04,04,04,04,04,04,07,04,
    07,07,07,07,07,07,04,07,04,04,04,04,04,04,07,04,
    04,04,04,04,04,04,07,04,04,04,04,04,04,04,07,04,
    04,04,04,04,04,04,07,04,04,04,04,04,04,04,07,04,
    04,04,04,04,04,04,07,04,04,04,04,04,04,04,07,04,
    04,04,04,04,04,04,07,04,04,04,04,04,04,04,07,04,
    05,10,10,10,10,11,07,11,05,10,10,00,10,17,07,11,
    05,10,10,11,10,11,07,11,05,04,10,11,10,00,07,11,
    05,10,10,19,10,11,07,11,05,04,10,04,10,12,07,11,
    05,10,10,04,10,11,07,11,05,06,10,04,10,00,07,11};

//prefix CB
Byte Z80::_clock_cb[]={ 8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    00,00,00,00,00,00,00,00, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
    8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
    8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
    8, 8, 8, 8, 8, 8,12, 8, 8, 8, 8, 8, 8, 8,12, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8,
    8, 8, 8, 8, 8, 8,15, 8, 8, 8, 8, 8, 8, 8,15, 8};

//prefix DD or FD
Byte Z80::_clock_dd[]={00,00,00,00,00,00,00,00,00,15,00,00,00,00,00,00,
    00,00,00,00,00,00,00,00,00,15,00,00,00,00,00,00,
    00,14,20,10,00,00,00,00,00,15,20,10,00,00,00,00,
    00,00,00,00,23,23,19,00,00,15,00,00,00,00,00,00,
    00,00,00,00,00,00,19,00,00,00,00,00,00,00,19,00,
    00,00,00,00,00,00,19,00,00,00,00,00,00,00,19,00,
    00,00,00,00,00,00,19,00,00,00,00,00,00,00,19,00,
    19,19,19,19,19,19,00,19,00,00,00,00,00,00,19,00,
    00,00,00,00,00,00,19,00,00,00,00,00,00,00,19,00,
    00,00,00,00,00,00,19,00,00,00,00,00,00,00,19,00,
    00,00,00,00,00,00,19,00,00,00,00,00,00,00,19,00,
    00,00,00,00,00,00,19,00,00,00,00,00,00,00,19,00,
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
    00,14,00,23,00,15,00,00,00, 8,00,00,00,00,00,00,
    00,00,00,00,00,00,00,00,00,10,00,00,00,00,00,00};

//prefix DD CB or FD CB
Byte Z80::_clock_dd_cb[]={00,00,00,00,00,00,23,00,00,00,00,00,00,00,23,00,
    00,00,00,00,00,00,23,00,00,00,00,00,00,00,23,00,
    00,00,00,00,00,00,23,00,00,00,00,00,00,00,23,00,
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,23,00,
    00,00,00,00,00,00,20,00,00,00,00,00,00,00,20,00,
    00,00,00,00,00,00,20,00,00,00,00,00,00,00,20,00,
    00,00,00,00,00,00,20,00,00,00,00,00,00,00,20,00,
    00,00,00,00,00,00,20,00,00,00,00,00,00,00,20,00,
    00,00,00,00,00,00,23,00,00,00,00,00,00,00,23,00,
    00,00,00,00,00,00,23,00,00,00,00,00,00,00,23,00,
    00,00,00,00,00,00,23,00,00,00,00,00,00,00,23,00,
    00,00,00,00,00,00,23,00,00,00,00,00,00,00,23,00,
    00,00,00,00,00,00,23,00,00,00,00,00,00,00,23,00,
    00,00,00,00,00,00,23,00,00,00,00,00,00,00,23,00,
    00,00,00,00,00,00,23,00,00,00,00,00,00,00,23,00,
    00,00,00,00,00,00,23,00,00,00,00,00,00,00,23,00};

//prefix ED
Byte Z80::_clock_ed[]={00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
    12,12,15,20, 8,14, 8, 9,12,12,15,20,00,14,00, 9,
    12,12,15,20,00,00, 8, 9,12,12,15,20,00,00, 8, 9,
    12,12,15,00,00,00,00,18,12,12,15,00,00,00,00,18,
    00,00,15,20,00,00,00,00,12,12,15,20,00,00,00,00,
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
    16,16,16,16,00,00,00,00,16,16,16,16,00,00,00,00,
    21,21,16,16,00,00,00,00,21,21,16,16,00,00,00,00,
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,
    00,00,00,00,00,00,00,00,00,00,00,00,00,00,00,00};
