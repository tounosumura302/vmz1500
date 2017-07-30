//
//  z80.hpp
//  vmz1500
//
//  Created by murasuke on 2017/04/01.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#ifndef z80_hpp
#define z80_hpp

#include <stdio.h>
#include <map>
#include <random>

#include "type.h"
#include "memory.hpp"
#include "z80pio.hpp"
//#include "ioport.h"


/*
    I/O port
 */

/*
class Z80Peripheral_basic
{
public:
    Z80Peripheral_basic();
    virtual ~Z80Peripheral_basic()=default;
    
    virtual Byte inRequested(void)=0;
    virtual void outRequested(Byte value)=0;
};
*/

//IN function
using Z80PeripheralInFunc=std::function<Byte(void)>;
//OUT function
using Z80PeripheralOutFunc=std::function<void(Byte)>;


class Z80IOPort
{
public:
    Z80IOPort();
    virtual ~Z80IOPort()=default;
    
    void init();
    
//    bool setPeripheral(Word adr,Z80Peripheral_basic *per);
    bool setPeripheral(Word adr,Z80PeripheralInFunc infunc,Z80PeripheralOutFunc outfunc);
    
    virtual Byte in(Word adr);
    virtual void out(Word adr,Byte value);
    
protected:
//    std::map<Word, Z80Peripheral_basic*> _iomap;
    
    std::map<Word,Z80PeripheralInFunc> _infunc;
    std::map<Word,Z80PeripheralOutFunc> _outfunc;
};



/*
    Z80
 */


class Z80
{
    
public:
    Z80(Memory *memory,Z80IOPort *ioport,Z80Pio *pio);
    virtual ~Z80();
    
    //void init(Memory *memory,Z80IOPort *ioport,Z80Pio *pio);
    void reset();
    
    //cheat
    void jump(Word address);
    
    
    typedef enum{
        OK,
        HUNG,
        HALT,
        STOP,
    }
    Status;
    
    int exec();
private:
    int _execOp(Byte op);
    
/*
signals:
    void messageSent(const QString &message);
    void stopped();
    void hunged();
    
    public slots:
    void stop();
    void restart();
    void irq(Byte data=0);
  */
public:
    void stop();
    void restart();
    void irq(Byte data=0);
    
protected:
    void _handleIrq();
    bool _irqflag;
    Byte _irqdata;
    
protected:
    Byte _getByteAtPC();
    Word _getWordAtPC();
    Byte _getByteAtBC();
    Byte _getByteAtDE();
    Byte _getByteAtHL();
    Word _getWordAtHL();
    Byte _getByteAtIX(Byte d);
    Word _getWordAtIX();
    Byte _getByteAtIY(Byte d);
    Word _getWordAtIY();
    Byte _getByteAtAdr(Word a);
    Word _getWordAtAdr(Word a);
    Byte _getByteAtSP(Word d);
    Word _getWordAtSP();
    
    void _setByteAtBC(Byte b);
    void _setByteAtDE(Byte b);
    void _setByteAtHL(Byte b);
    void _setByteAtIX(Byte d,Byte b);
    void _setByteAtIY(Byte d,Byte b);
    void _setByteAtAdr(Word a,Byte b);
    void _set2ByteAtAdr(Word a,Byte b1,Byte b2);
    void _setWordAtAdr(Word a,Word w);
    void _setByteAtSP(Word d,Byte b);
    void _setWordAtSP(Word w);
    
    
    //memory
    Memory *_mem;
    
    //IO port
    Z80IOPort *_ioport;
    
    //Z80 PIO
protected:
    Z80Pio *_pio;
    
    //status
protected:
    Status _status;
    
    //regiters
    //    Byte _a,_f,_b,_c,_d,_e,_h,_l;
    Byte _a,_f;
    Byte __a,__f;
    Byte _i,_r;
    Word _ix,_iy;
    Word _pc;
    Word _sp;
    Byte _iff,_iff2,_im;
    
    //register pair
    typedef union{
        Word word;
        Byte byte[2];
    }
    rpair;
    rpair _bc_p,_de_p,_hl_p;
    rpair __bc_p,__de_p,__hl_p;
#define _rb _bc_p.byte[1]
#define _rc _bc_p.byte[0]
#define _rd _de_p.byte[1]
#define _re _de_p.byte[0]
#define _rh _hl_p.byte[1]
#define _rl _hl_p.byte[0]
    
#define __rb __bc_p.byte[1]
#define __rc __bc_p.byte[0]
#define __rd __de_p.byte[1]
#define __re __de_p.byte[0]
#define __rh __hl_p.byte[1]
#define __rl __hl_p.byte[0]
    
#define _rbc _bc_p.word
#define _rde _de_p.word
#define _rhl _hl_p.word
#define _xrbc __bc_p.word
#define _xrde __de_p.word
#define _xrhl __hl_p.word
    
    
    //flag
    enum{
        FLAG_MASK_C=0b00000001,
        FLAG_MASK_N=0b00000010,
        FLAG_MASK_P=0b00000100,
        FLAG_MASK_V=0b00000100,
        FLAG_MASK_H=0b00010000,
        FLAG_MASK_Z=0b01000000,
        FLAG_MASK_S=0b10000000
    };
    
    Byte signFlag(const Byte b);
    Byte signFlag(const Word b);
    Byte zeroFlag(const Byte b);
    Byte zeroFlag(const Word b);
    Byte halfcarryFlag(const Byte b);
    Byte overflowFlagAfterAdd(const Byte before,const Byte after);
    Byte overflowFlagAfterSub(const Byte before,const Byte after);
    Byte overflowFlagAfterAdd(const Word before,const Word after);
    Byte overflowFlagAfterSub(const Word before,const Word after);
    Byte parityFlag(const Byte b);
    Byte iffFlag();  //iffの値をparity/overflowにセットするケースのみ使用
    Byte carryFlagAfterAdd(const Byte before,const Byte after);
    Byte carryFlagAfterSub(const Byte before,const Byte after);
    Byte carryFlagAfterAdd(const Word before,const Word after);
    Byte carryFlagAfterSub(const Word before,const Word after);
    
    bool _isZ();
    bool _isNZ();
    bool _isC();
    bool _isNC();
    bool _isM();
    bool _isP();
    bool _isPE();
    bool _isPO();
    
public:
    bool isZ() const;
    bool isC() const;
    bool isM() const;
    bool isPE() const;
    
public:
    Byte regA() const;
    Byte regF() const;
    Byte regB() const;
    Byte regC() const;
    Byte regD() const;
    Byte regE() const;
    Byte regH() const;
    Byte regL() const;
    
    Byte regAA() const;
    Byte regFF() const;
    Byte regBB() const;
    Byte regCC() const;
    Byte regDD() const;
    Byte regEE() const;
    Byte regHH() const;
    Byte regLL() const;
    
    Byte regI() const;
    Byte regR() const;
    Word regBC() const;
    Word regDE() const;
    Word regHL() const;
    Word regIX() const;
    Word regIY() const;
    Word regSP() const;
    Word regPC() const;
    
    /*
        R register
     */
protected:
    void changeR();
    std::mt19937 *_random;
    
    
protected:
    //opcode->execute function table
    /*
     *関数ポインタについて、詳しくは以下を参考。
     *メンバー関数を関数ポインタにすることが可能。ポイントは、関数ポインタにクラス名を指定すること。
     * http://qiita.com/shiro_naga/items/5967f6cd1710e7b78677
     * http://www7b.biglobe.ne.jp/~robe/cpphtml/html03/cpp03057.html
     */
    typedef void (Z80::*OpFuncPtr)();
    static OpFuncPtr _opfunc[256];
    static OpFuncPtr _opfunc_cb[256];
    static OpFuncPtr _opfunc_dd[256];
    static OpFuncPtr _opfunc_ed[256];
    static OpFuncPtr _opfunc_fd[256];
    
    //opecode execution functions(undefined opecode)
    void _op_undef();
    
    //opecode execution helper function
    void _op_add(Byte b);
    void _op_adc(Byte b);
    void _op_sub(Byte b);
    void _op_sbc(Byte b);
    void _op_cp(Byte b);
    void _op_and(Byte b);
    void _op_xor(Byte b);
    void _op_or(Byte b);
    Byte _op_inc(Byte b);
    Byte _op_dec(Byte b);
    Byte _op_rlc(Byte b);
    Byte _op_rrc(Byte b);
    Byte _op_rl(Byte b);
    Byte _op_rr(Byte b);
    Byte _op_sla(Byte b);
    Byte _op_sra(Byte b);
    Byte _op_srl(Byte b);
    
    Byte _op_set(Byte b,Byte bit);
    Byte _op_res(Byte b,Byte bit);
    void _op_bit(Byte b,Byte bit);
    static Byte _bitTable[8];
    
    void _op_push(const Byte b);
    void _op_push(const Word w);
    Byte _op_pop();
    Word _op_pop_word();
    
    Byte _op_in();
    void _op_out(const Byte b);
    
    /*
     Word _pairBC();
     void _setBC(Word bc);
     Word _pairDE();
     void _setDE(Word de);
     Word _pairHL();
     void _setHL(Word hl);
     */
    
    //opecode execution functions(no prefix)
    void _op_00(),_op_01(),_op_02(),_op_03(),_op_04(),_op_05(),_op_06(),_op_07();
    void _op_08(),_op_09(),_op_0a(),_op_0b(),_op_0c(),_op_0d(),_op_0e(),_op_0f();
    void _op_10(),_op_11(),_op_12(),_op_13(),_op_14(),_op_15(),_op_16(),_op_17();
    void _op_18(),_op_19(),_op_1a(),_op_1b(),_op_1c(),_op_1d(),_op_1e(),_op_1f();
    void _op_20(),_op_21(),_op_22(),_op_23(),_op_24(),_op_25(),_op_26(),_op_27();
    void _op_28(),_op_29(),_op_2a(),_op_2b(),_op_2c(),_op_2d(),_op_2e(),_op_2f();
    void _op_30(),_op_31(),_op_32(),_op_33(),_op_34(),_op_35(),_op_36(),_op_37();
    void _op_38(),_op_39(),_op_3a(),_op_3b(),_op_3c(),_op_3d(),_op_3e(),_op_3f();
    void _op_40(),_op_41(),_op_42(),_op_43(),_op_44(),_op_45(),_op_46(),_op_47();
    void _op_48(),_op_49(),_op_4a(),_op_4b(),_op_4c(),_op_4d(),_op_4e(),_op_4f();
    void _op_50(),_op_51(),_op_52(),_op_53(),_op_54(),_op_55(),_op_56(),_op_57();
    void _op_58(),_op_59(),_op_5a(),_op_5b(),_op_5c(),_op_5d(),_op_5e(),_op_5f();
    void _op_60(),_op_61(),_op_62(),_op_63(),_op_64(),_op_65(),_op_66(),_op_67();
    void _op_68(),_op_69(),_op_6a(),_op_6b(),_op_6c(),_op_6d(),_op_6e(),_op_6f();
    void _op_70(),_op_71(),_op_72(),_op_73(),_op_74(),_op_75(),_op_76(),_op_77();
    void _op_78(),_op_79(),_op_7a(),_op_7b(),_op_7c(),_op_7d(),_op_7e(),_op_7f();
    void _op_80(),_op_81(),_op_82(),_op_83(),_op_84(),_op_85(),_op_86(),_op_87();
    void _op_88(),_op_89(),_op_8a(),_op_8b(),_op_8c(),_op_8d(),_op_8e(),_op_8f();
    void _op_90(),_op_91(),_op_92(),_op_93(),_op_94(),_op_95(),_op_96(),_op_97();
    void _op_98(),_op_99(),_op_9a(),_op_9b(),_op_9c(),_op_9d(),_op_9e(),_op_9f();
    void _op_a0(),_op_a1(),_op_a2(),_op_a3(),_op_a4(),_op_a5(),_op_a6(),_op_a7();
    void _op_a8(),_op_a9(),_op_aa(),_op_ab(),_op_ac(),_op_ad(),_op_ae(),_op_af();
    void _op_b0(),_op_b1(),_op_b2(),_op_b3(),_op_b4(),_op_b5(),_op_b6(),_op_b7();
    void _op_b8(),_op_b9(),_op_ba(),_op_bb(),_op_bc(),_op_bd(),_op_be(),_op_bf();
    void _op_c0(),_op_c1(),_op_c2(),_op_c3(),_op_c4(),_op_c5(),_op_c6(),_op_c7();
    void _op_c8(),_op_c9(),_op_ca(),_op_cb(),_op_cc(),_op_cd(),_op_ce(),_op_cf();
    void _op_d0(),_op_d1(),_op_d2(),_op_d3(),_op_d4(),_op_d5(),_op_d6(),_op_d7();
    void _op_d8(),_op_d9(),_op_da(),_op_db(),_op_dc(),_op_dd(),_op_de(),_op_df();
    void _op_e0(),_op_e1(),_op_e2(),_op_e3(),_op_e4(),_op_e5(),_op_e6(),_op_e7();
    void _op_e8(),_op_e9(),_op_ea(),_op_eb(),_op_ec(),_op_ed(),_op_ee(),_op_ef();
    void _op_f0(),_op_f1(),_op_f2(),_op_f3(),_op_f4(),_op_f5(),_op_f6(),_op_f7();
    void _op_f8(),_op_f9(),_op_fa(),_op_fb(),_op_fc(),_op_fd(),_op_fe(),_op_ff();
    
    //opcode execution functions(prefix cbh)
    void _op_cb_00(),_op_cb_01(),_op_cb_02(),_op_cb_03(),_op_cb_04(),_op_cb_05(),_op_cb_06(),_op_cb_07();
    void _op_cb_08(),_op_cb_09(),_op_cb_0a(),_op_cb_0b(),_op_cb_0c(),_op_cb_0d(),_op_cb_0e(),_op_cb_0f();
    void _op_cb_10(),_op_cb_11(),_op_cb_12(),_op_cb_13(),_op_cb_14(),_op_cb_15(),_op_cb_16(),_op_cb_17();
    void _op_cb_18(),_op_cb_19(),_op_cb_1a(),_op_cb_1b(),_op_cb_1c(),_op_cb_1d(),_op_cb_1e(),_op_cb_1f();
    void _op_cb_20(),_op_cb_21(),_op_cb_22(),_op_cb_23(),_op_cb_24(),_op_cb_25(),_op_cb_26(),_op_cb_27();
    void _op_cb_28(),_op_cb_29(),_op_cb_2a(),_op_cb_2b(),_op_cb_2c(),_op_cb_2d(),_op_cb_2e(),_op_cb_2f();
    //cb 30-cb 37 undefined opecode
    void _op_cb_38(),_op_cb_39(),_op_cb_3a(),_op_cb_3b(),_op_cb_3c(),_op_cb_3d(),_op_cb_3e(),_op_cb_3f();
    void _op_cb_40(),_op_cb_41(),_op_cb_42(),_op_cb_43(),_op_cb_44(),_op_cb_45(),_op_cb_46(),_op_cb_47();
    void _op_cb_48(),_op_cb_49(),_op_cb_4a(),_op_cb_4b(),_op_cb_4c(),_op_cb_4d(),_op_cb_4e(),_op_cb_4f();
    void _op_cb_50(),_op_cb_51(),_op_cb_52(),_op_cb_53(),_op_cb_54(),_op_cb_55(),_op_cb_56(),_op_cb_57();
    void _op_cb_58(),_op_cb_59(),_op_cb_5a(),_op_cb_5b(),_op_cb_5c(),_op_cb_5d(),_op_cb_5e(),_op_cb_5f();
    void _op_cb_60(),_op_cb_61(),_op_cb_62(),_op_cb_63(),_op_cb_64(),_op_cb_65(),_op_cb_66(),_op_cb_67();
    void _op_cb_68(),_op_cb_69(),_op_cb_6a(),_op_cb_6b(),_op_cb_6c(),_op_cb_6d(),_op_cb_6e(),_op_cb_6f();
    void _op_cb_70(),_op_cb_71(),_op_cb_72(),_op_cb_73(),_op_cb_74(),_op_cb_75(),_op_cb_76(),_op_cb_77();
    void _op_cb_78(),_op_cb_79(),_op_cb_7a(),_op_cb_7b(),_op_cb_7c(),_op_cb_7d(),_op_cb_7e(),_op_cb_7f();
    void _op_cb_80(),_op_cb_81(),_op_cb_82(),_op_cb_83(),_op_cb_84(),_op_cb_85(),_op_cb_86(),_op_cb_87();
    void _op_cb_88(),_op_cb_89(),_op_cb_8a(),_op_cb_8b(),_op_cb_8c(),_op_cb_8d(),_op_cb_8e(),_op_cb_8f();
    void _op_cb_90(),_op_cb_91(),_op_cb_92(),_op_cb_93(),_op_cb_94(),_op_cb_95(),_op_cb_96(),_op_cb_97();
    void _op_cb_98(),_op_cb_99(),_op_cb_9a(),_op_cb_9b(),_op_cb_9c(),_op_cb_9d(),_op_cb_9e(),_op_cb_9f();
    void _op_cb_a0(),_op_cb_a1(),_op_cb_a2(),_op_cb_a3(),_op_cb_a4(),_op_cb_a5(),_op_cb_a6(),_op_cb_a7();
    void _op_cb_a8(),_op_cb_a9(),_op_cb_aa(),_op_cb_ab(),_op_cb_ac(),_op_cb_ad(),_op_cb_ae(),_op_cb_af();
    void _op_cb_b0(),_op_cb_b1(),_op_cb_b2(),_op_cb_b3(),_op_cb_b4(),_op_cb_b5(),_op_cb_b6(),_op_cb_b7();
    void _op_cb_b8(),_op_cb_b9(),_op_cb_ba(),_op_cb_bb(),_op_cb_bc(),_op_cb_bd(),_op_cb_be(),_op_cb_bf();
    void _op_cb_c0(),_op_cb_c1(),_op_cb_c2(),_op_cb_c3(),_op_cb_c4(),_op_cb_c5(),_op_cb_c6(),_op_cb_c7();
    void _op_cb_c8(),_op_cb_c9(),_op_cb_ca(),_op_cb_cb(),_op_cb_cc(),_op_cb_cd(),_op_cb_ce(),_op_cb_cf();
    void _op_cb_d0(),_op_cb_d1(),_op_cb_d2(),_op_cb_d3(),_op_cb_d4(),_op_cb_d5(),_op_cb_d6(),_op_cb_d7();
    void _op_cb_d8(),_op_cb_d9(),_op_cb_da(),_op_cb_db(),_op_cb_dc(),_op_cb_dd(),_op_cb_de(),_op_cb_df();
    void _op_cb_e0(),_op_cb_e1(),_op_cb_e2(),_op_cb_e3(),_op_cb_e4(),_op_cb_e5(),_op_cb_e6(),_op_cb_e7();
    void _op_cb_e8(),_op_cb_e9(),_op_cb_ea(),_op_cb_eb(),_op_cb_ec(),_op_cb_ed(),_op_cb_ee(),_op_cb_ef();
    void _op_cb_f0(),_op_cb_f1(),_op_cb_f2(),_op_cb_f3(),_op_cb_f4(),_op_cb_f5(),_op_cb_f6(),_op_cb_f7();
    void _op_cb_f8(),_op_cb_f9(),_op_cb_fa(),_op_cb_fb(),_op_cb_fc(),_op_cb_fd(),_op_cb_fe(),_op_cb_ff();
    
    void _op_dd_cb_d_06(Byte d);
    void _op_dd_cb_d_0e(Byte d);
    void _op_dd_cb_d_16(Byte d);
    void _op_dd_cb_d_1e(Byte d);
    void _op_dd_cb_d_26(Byte d);
    void _op_dd_cb_d_2e(Byte d);
    void _op_dd_cb_d_3e(Byte d);
    void _op_dd_cb_d_46(Byte d);
    void _op_dd_cb_d_4e(Byte d);
    void _op_dd_cb_d_56(Byte d);
    void _op_dd_cb_d_5e(Byte d);
    void _op_dd_cb_d_66(Byte d);
    void _op_dd_cb_d_6e(Byte d);
    void _op_dd_cb_d_76(Byte d);
    void _op_dd_cb_d_7e(Byte d);
    void _op_dd_cb_d_86(Byte d);
    void _op_dd_cb_d_8e(Byte d);
    void _op_dd_cb_d_96(Byte d);
    void _op_dd_cb_d_9e(Byte d);
    void _op_dd_cb_d_a6(Byte d);
    void _op_dd_cb_d_ae(Byte d);
    void _op_dd_cb_d_b6(Byte d);
    void _op_dd_cb_d_be(Byte d);
    void _op_dd_cb_d_c6(Byte d);
    void _op_dd_cb_d_ce(Byte d);
    void _op_dd_cb_d_d6(Byte d);
    void _op_dd_cb_d_de(Byte d);
    void _op_dd_cb_d_e6(Byte d);
    void _op_dd_cb_d_ee(Byte d);
    void _op_dd_cb_d_f6(Byte d);
    void _op_dd_cb_d_fe(Byte d);
    
    void _op_fd_cb_d_06(Byte d);
    void _op_fd_cb_d_0e(Byte d);
    void _op_fd_cb_d_16(Byte d);
    void _op_fd_cb_d_1e(Byte d);
    void _op_fd_cb_d_26(Byte d);
    void _op_fd_cb_d_2e(Byte d);
    void _op_fd_cb_d_3e(Byte d);
    void _op_fd_cb_d_46(Byte d);
    void _op_fd_cb_d_4e(Byte d);
    void _op_fd_cb_d_56(Byte d);
    void _op_fd_cb_d_5e(Byte d);
    void _op_fd_cb_d_66(Byte d);
    void _op_fd_cb_d_6e(Byte d);
    void _op_fd_cb_d_76(Byte d);
    void _op_fd_cb_d_7e(Byte d);
    void _op_fd_cb_d_86(Byte d);
    void _op_fd_cb_d_8e(Byte d);
    void _op_fd_cb_d_96(Byte d);
    void _op_fd_cb_d_9e(Byte d);
    void _op_fd_cb_d_a6(Byte d);
    void _op_fd_cb_d_ae(Byte d);
    void _op_fd_cb_d_b6(Byte d);
    void _op_fd_cb_d_be(Byte d);
    void _op_fd_cb_d_c6(Byte d);
    void _op_fd_cb_d_ce(Byte d);
    void _op_fd_cb_d_d6(Byte d);
    void _op_fd_cb_d_de(Byte d);
    void _op_fd_cb_d_e6(Byte d);
    void _op_fd_cb_d_ee(Byte d);
    void _op_fd_cb_d_f6(Byte d);
    void _op_fd_cb_d_fe(Byte d);
    
    //opecode execution functions(prefix ddh)
    /*
     void _op_dd_00(),_op_dd_01(),_op_dd_02(),_op_dd_03(),_op_dd_04(),_op_dd_05(),_op_dd_06(),_op_dd_07();
     void _op_dd_08(),_op_dd_09(),_op_dd_0a(),_op_dd_0b(),_op_dd_0c(),_op_dd_0d(),_op_dd_0e(),_op_dd_0f();
     void _op_dd_10(),_op_dd_11(),_op_dd_12(),_op_dd_13(),_op_dd_14(),_op_dd_15(),_op_dd_16(),_op_dd_17();
     void _op_dd_18(),_op_dd_19(),_op_dd_1a(),_op_dd_1b(),_op_dd_1c(),_op_dd_1d(),_op_dd_1e(),_op_dd_1f();
     void _op_dd_20(),_op_dd_21(),_op_dd_22(),_op_dd_23(),_op_dd_24(),_op_dd_25(),_op_dd_26(),_op_dd_27();
     void _op_dd_28(),_op_dd_29(),_op_dd_2a(),_op_dd_2b(),_op_dd_2c(),_op_dd_2d(),_op_dd_2e(),_op_dd_2f();
     void _op_dd_30(),_op_dd_31(),_op_dd_32(),_op_dd_33(),_op_dd_34(),_op_dd_35(),_op_dd_36(),_op_dd_37();
     void _op_dd_38(),_op_dd_39(),_op_dd_3a(),_op_dd_3b(),_op_dd_3c(),_op_dd_3d(),_op_dd_3e(),_op_dd_3f();
     void _op_dd_40(),_op_dd_41(),_op_dd_42(),_op_dd_43(),_op_dd_44(),_op_dd_45(),_op_dd_46(),_op_dd_47();
     void _op_dd_48(),_op_dd_49(),_op_dd_4a(),_op_dd_4b(),_op_dd_4c(),_op_dd_4d(),_op_dd_4e(),_op_dd_4f();
     void _op_dd_50(),_op_dd_51(),_op_dd_52(),_op_dd_53(),_op_dd_54(),_op_dd_55(),_op_dd_56(),_op_dd_57();
     void _op_dd_58(),_op_dd_59(),_op_dd_5a(),_op_dd_5b(),_op_dd_5c(),_op_dd_5d(),_op_dd_5e(),_op_dd_5f();
     void _op_dd_60(),_op_dd_61(),_op_dd_62(),_op_dd_63(),_op_dd_64(),_op_dd_65(),_op_dd_66(),_op_dd_67();
     void _op_dd_68(),_op_dd_69(),_op_dd_6a(),_op_dd_6b(),_op_dd_6c(),_op_dd_6d(),_op_dd_6e(),_op_dd_6f();
     void _op_dd_70(),_op_dd_71(),_op_dd_72(),_op_dd_73(),_op_dd_74(),_op_dd_75(),_op_dd_76(),_op_dd_77();
     void _op_dd_78(),_op_dd_79(),_op_dd_7a(),_op_dd_7b(),_op_dd_7c(),_op_dd_7d(),_op_dd_7e(),_op_dd_7f();
     void _op_dd_80(),_op_dd_81(),_op_dd_82(),_op_dd_83(),_op_dd_84(),_op_dd_85(),_op_dd_86(),_op_dd_87();
     void _op_dd_88(),_op_dd_89(),_op_dd_8a(),_op_dd_8b(),_op_dd_8c(),_op_dd_8d(),_op_dd_8e(),_op_dd_8f();
     void _op_dd_90(),_op_dd_91(),_op_dd_92(),_op_dd_93(),_op_dd_94(),_op_dd_95(),_op_dd_96(),_op_dd_97();
     void _op_dd_98(),_op_dd_99(),_op_dd_9a(),_op_dd_9b(),_op_dd_9c(),_op_dd_9d(),_op_dd_9e(),_op_dd_9f();
     void _op_dd_a0(),_op_dd_a1(),_op_dd_a2(),_op_dd_a3(),_op_dd_a4(),_op_dd_a5(),_op_dd_a6(),_op_dd_a7();
     void _op_dd_a8(),_op_dd_a9(),_op_dd_aa(),_op_dd_ab(),_op_dd_ac(),_op_dd_ad(),_op_dd_ae(),_op_dd_af();
     void _op_dd_b0(),_op_dd_b1(),_op_dd_b2(),_op_dd_b3(),_op_dd_b4(),_op_dd_b5(),_op_dd_b6(),_op_dd_b7();
     void _op_dd_b8(),_op_dd_b9(),_op_dd_ba(),_op_dd_bb(),_op_dd_bc(),_op_dd_bd(),_op_dd_be(),_op_dd_bf();
     void _op_dd_c0(),_op_dd_c1(),_op_dd_c2(),_op_dd_c3(),_op_dd_c4(),_op_dd_c5(),_op_dd_c6(),_op_dd_c7();
     void _op_dd_c8(),_op_dd_c9(),_op_dd_ca(),_op_dd_cb(),_op_dd_cc(),_op_dd_cd(),_op_dd_ce(),_op_dd_cf();
     void _op_dd_d0(),_op_dd_d1(),_op_dd_d2(),_op_dd_d3(),_op_dd_d4(),_op_dd_d5(),_op_dd_d6(),_op_dd_d7();
     void _op_dd_d8(),_op_dd_d9(),_op_dd_da(),_op_dd_db(),_op_dd_dc(),_op_dd_dd(),_op_dd_de(),_op_dd_df();
     void _op_dd_e0(),_op_dd_e1(),_op_dd_e2(),_op_dd_e3(),_op_dd_e4(),_op_dd_e5(),_op_dd_e6(),_op_dd_e7();
     void _op_dd_e8(),_op_dd_e9(),_op_dd_ea(),_op_dd_eb(),_op_dd_ec(),_op_dd_ed(),_op_dd_ee(),_op_dd_ef();
     void _op_dd_f0(),_op_dd_f1(),_op_dd_f2(),_op_dd_f3(),_op_dd_f4(),_op_dd_f5(),_op_dd_f6(),_op_dd_f7();
     void _op_dd_f8(),_op_dd_f9(),_op_dd_fa(),_op_dd_fb(),_op_dd_fc(),_op_dd_fd(),_op_dd_fe(),_op_dd_ff();
     */
    void _op_dd_09();
    void _op_dd_19();
    void _op_dd_21(),_op_dd_22(),_op_dd_23(),_op_dd_29(),_op_dd_2a(),_op_dd_2b();
    void _op_dd_34(),_op_dd_35(),_op_dd_36(),_op_dd_39();
    void _op_dd_46(),_op_dd_4e();
    void _op_dd_56(),_op_dd_5e();
    void _op_dd_66(),_op_dd_6e();
    void _op_dd_70(),_op_dd_71(),_op_dd_72(),_op_dd_73(),_op_dd_74(),_op_dd_75(),_op_dd_77(),_op_dd_7e();
    void _op_dd_86(),_op_dd_8e();
    void _op_dd_96(),_op_dd_9e();
    void _op_dd_a6(),_op_dd_ae();
    void _op_dd_b6(),_op_dd_be();
    void _op_dd_cb();
    void _op_dd_e1(),_op_dd_e3(),_op_dd_e5(),_op_dd_e9();
    void _op_dd_f9();
    
    //opecode execution functions(prefix edh)
    /*
     void _op_ed_00(),_op_ed_01(),_op_ed_02(),_op_ed_03(),_op_ed_04(),_op_ed_05(),_op_ed_06(),_op_ed_07();
     void _op_ed_08(),_op_ed_09(),_op_ed_0a(),_op_ed_0b(),_op_ed_0c(),_op_ed_0d(),_op_ed_0e(),_op_ed_0f();
     void _op_ed_10(),_op_ed_11(),_op_ed_12(),_op_ed_13(),_op_ed_14(),_op_ed_15(),_op_ed_16(),_op_ed_17();
     void _op_ed_18(),_op_ed_19(),_op_ed_1a(),_op_ed_1b(),_op_ed_1c(),_op_ed_1d(),_op_ed_1e(),_op_ed_1f();
     void _op_ed_20(),_op_ed_21(),_op_ed_22(),_op_ed_23(),_op_ed_24(),_op_ed_25(),_op_ed_26(),_op_ed_27();
     void _op_ed_28(),_op_ed_29(),_op_ed_2a(),_op_ed_2b(),_op_ed_2c(),_op_ed_2d(),_op_ed_2e(),_op_ed_2f();
     void _op_ed_30(),_op_ed_31(),_op_ed_32(),_op_ed_33(),_op_ed_34(),_op_ed_35(),_op_ed_36(),_op_ed_37();
     void _op_ed_38(),_op_ed_39(),_op_ed_3a(),_op_ed_3b(),_op_ed_3c(),_op_ed_3d(),_op_ed_3e(),_op_ed_3f();
     void _op_ed_40(),_op_ed_41(),_op_ed_42(),_op_ed_43(),_op_ed_44(),_op_ed_45(),_op_ed_46(),_op_ed_47();
     void _op_ed_48(),_op_ed_49(),_op_ed_4a(),_op_ed_4b(),_op_ed_4c(),_op_ed_4d(),_op_ed_4e(),_op_ed_4f();
     void _op_ed_50(),_op_ed_51(),_op_ed_52(),_op_ed_53(),_op_ed_54(),_op_ed_55(),_op_ed_56(),_op_ed_57();
     void _op_ed_58(),_op_ed_59(),_op_ed_5a(),_op_ed_5b(),_op_ed_5c(),_op_ed_5d(),_op_ed_5e(),_op_ed_5f();
     void _op_ed_60(),_op_ed_61(),_op_ed_62(),_op_ed_63(),_op_ed_64(),_op_ed_65(),_op_ed_66(),_op_ed_67();
     void _op_ed_68(),_op_ed_69(),_op_ed_6a(),_op_ed_6b(),_op_ed_6c(),_op_ed_6d(),_op_ed_6e(),_op_ed_6f();
     void _op_ed_70(),_op_ed_71(),_op_ed_72(),_op_ed_73(),_op_ed_74(),_op_ed_75(),_op_ed_76(),_op_ed_77();
     void _op_ed_78(),_op_ed_79(),_op_ed_7a(),_op_ed_7b(),_op_ed_7c(),_op_ed_7d(),_op_ed_7e(),_op_ed_7f();
     void _op_ed_80(),_op_ed_81(),_op_ed_82(),_op_ed_83(),_op_ed_84(),_op_ed_85(),_op_ed_86(),_op_ed_87();
     void _op_ed_88(),_op_ed_89(),_op_ed_8a(),_op_ed_8b(),_op_ed_8c(),_op_ed_8d(),_op_ed_8e(),_op_ed_8f();
     void _op_ed_90(),_op_ed_91(),_op_ed_92(),_op_ed_93(),_op_ed_94(),_op_ed_95(),_op_ed_96(),_op_ed_97();
     void _op_ed_98(),_op_ed_99(),_op_ed_9a(),_op_ed_9b(),_op_ed_9c(),_op_ed_9d(),_op_ed_9e(),_op_ed_9f();
     void _op_ed_a0(),_op_ed_a1(),_op_ed_a2(),_op_ed_a3(),_op_ed_a4(),_op_ed_a5(),_op_ed_a6(),_op_ed_a7();
     void _op_ed_a8(),_op_ed_a9(),_op_ed_aa(),_op_ed_ab(),_op_ed_ac(),_op_ed_ad(),_op_ed_ae(),_op_ed_af();
     void _op_ed_b0(),_op_ed_b1(),_op_ed_b2(),_op_ed_b3(),_op_ed_b4(),_op_ed_b5(),_op_ed_b6(),_op_ed_b7();
     void _op_ed_b8(),_op_ed_b9(),_op_ed_ba(),_op_ed_bb(),_op_ed_bc(),_op_ed_bd(),_op_ed_be(),_op_ed_bf();
     void _op_ed_c0(),_op_ed_c1(),_op_ed_c2(),_op_ed_c3(),_op_ed_c4(),_op_ed_c5(),_op_ed_c6(),_op_ed_c7();
     void _op_ed_c8(),_op_ed_c9(),_op_ed_ca(),_op_ed_cb(),_op_ed_cc(),_op_ed_cd(),_op_ed_ce(),_op_ed_cf();
     void _op_ed_d0(),_op_ed_d1(),_op_ed_d2(),_op_ed_d3(),_op_ed_d4(),_op_ed_d5(),_op_ed_d6(),_op_ed_d7();
     void _op_ed_d8(),_op_ed_d9(),_op_ed_da(),_op_ed_db(),_op_ed_dc(),_op_ed_dd(),_op_ed_de(),_op_ed_df();
     void _op_ed_e0(),_op_ed_e1(),_op_ed_e2(),_op_ed_e3(),_op_ed_e4(),_op_ed_e5(),_op_ed_e6(),_op_ed_e7();
     void _op_ed_e8(),_op_ed_e9(),_op_ed_ea(),_op_ed_eb(),_op_ed_ec(),_op_ed_ed(),_op_ed_ee(),_op_ed_ef();
     void _op_ed_f0(),_op_ed_f1(),_op_ed_f2(),_op_ed_f3(),_op_ed_f4(),_op_ed_f5(),_op_ed_f6(),_op_ed_f7();
     void _op_ed_f8(),_op_ed_f9(),_op_ed_fa(),_op_ed_fb(),_op_ed_fc(),_op_ed_fd(),_op_ed_fe(),_op_ed_ff();
     */
    void _op_ed_40(),_op_ed_41(),_op_ed_42(),_op_ed_43(),_op_ed_44(),_op_ed_45(),_op_ed_46(),_op_ed_47();
    void _op_ed_48(),_op_ed_49(),_op_ed_4a(),_op_ed_4b(),_op_ed_4d(),_op_ed_4f();
    void _op_ed_50(),_op_ed_51(),_op_ed_52(),_op_ed_53(),_op_ed_56(),_op_ed_57();
    void _op_ed_58(),_op_ed_59(),_op_ed_5a(),_op_ed_5b(),_op_ed_5e(),_op_ed_5f();
    void _op_ed_60(),_op_ed_61(),_op_ed_62(),_op_ed_67();
    void _op_ed_68(),_op_ed_69(),_op_ed_6a(),_op_ed_6f();
    void _op_ed_72(),_op_ed_73(),_op_ed_78(),_op_ed_79(),_op_ed_7a(),_op_ed_7b();
    void _op_ed_a0(),_op_ed_a1(),_op_ed_a2(),_op_ed_a3(),_op_ed_a8(),_op_ed_a9(),_op_ed_aa(),_op_ed_ab();
    void _op_ed_b0(),_op_ed_b1(),_op_ed_b2(),_op_ed_b3(),_op_ed_b8(),_op_ed_b9(),_op_ed_ba(),_op_ed_bb();
    
    //opecode execution functions(prefix fdh)
    /*
     void _op_fd_00(),_op_fd_01(),_op_fd_02(),_op_fd_03(),_op_fd_04(),_op_fd_05(),_op_fd_06(),_op_fd_07();
     void _op_fd_08(),_op_fd_09(),_op_fd_0a(),_op_fd_0b(),_op_fd_0c(),_op_fd_0d(),_op_fd_0e(),_op_fd_0f();
     void _op_fd_10(),_op_fd_11(),_op_fd_12(),_op_fd_13(),_op_fd_14(),_op_fd_15(),_op_fd_16(),_op_fd_17();
     void _op_fd_18(),_op_fd_19(),_op_fd_1a(),_op_fd_1b(),_op_fd_1c(),_op_fd_1d(),_op_fd_1e(),_op_fd_1f();
     void _op_fd_20(),_op_fd_21(),_op_fd_22(),_op_fd_23(),_op_fd_24(),_op_fd_25(),_op_fd_26(),_op_fd_27();
     void _op_fd_28(),_op_fd_29(),_op_fd_2a(),_op_fd_2b(),_op_fd_2c(),_op_fd_2d(),_op_fd_2e(),_op_fd_2f();
     void _op_fd_30(),_op_fd_31(),_op_fd_32(),_op_fd_33(),_op_fd_34(),_op_fd_35(),_op_fd_36(),_op_fd_37();
     void _op_fd_38(),_op_fd_39(),_op_fd_3a(),_op_fd_3b(),_op_fd_3c(),_op_fd_3d(),_op_fd_3e(),_op_fd_3f();
     void _op_fd_40(),_op_fd_41(),_op_fd_42(),_op_fd_43(),_op_fd_44(),_op_fd_45(),_op_fd_46(),_op_fd_47();
     void _op_fd_48(),_op_fd_49(),_op_fd_4a(),_op_fd_4b(),_op_fd_4c(),_op_fd_4d(),_op_fd_4e(),_op_fd_4f();
     void _op_fd_50(),_op_fd_51(),_op_fd_52(),_op_fd_53(),_op_fd_54(),_op_fd_55(),_op_fd_56(),_op_fd_57();
     void _op_fd_58(),_op_fd_59(),_op_fd_5a(),_op_fd_5b(),_op_fd_5c(),_op_fd_5d(),_op_fd_5e(),_op_fd_5f();
     void _op_fd_60(),_op_fd_61(),_op_fd_62(),_op_fd_63(),_op_fd_64(),_op_fd_65(),_op_fd_66(),_op_fd_67();
     void _op_fd_68(),_op_fd_69(),_op_fd_6a(),_op_fd_6b(),_op_fd_6c(),_op_fd_6d(),_op_fd_6e(),_op_fd_6f();
     void _op_fd_70(),_op_fd_71(),_op_fd_72(),_op_fd_73(),_op_fd_74(),_op_fd_75(),_op_fd_76(),_op_fd_77();
     void _op_fd_78(),_op_fd_79(),_op_fd_7a(),_op_fd_7b(),_op_fd_7c(),_op_fd_7d(),_op_fd_7e(),_op_fd_7f();
     void _op_fd_80(),_op_fd_81(),_op_fd_82(),_op_fd_83(),_op_fd_84(),_op_fd_85(),_op_fd_86(),_op_fd_87();
     void _op_fd_88(),_op_fd_89(),_op_fd_8a(),_op_fd_8b(),_op_fd_8c(),_op_fd_8d(),_op_fd_8e(),_op_fd_8f();
     void _op_fd_90(),_op_fd_91(),_op_fd_92(),_op_fd_93(),_op_fd_94(),_op_fd_95(),_op_fd_96(),_op_fd_97();
     void _op_fd_98(),_op_fd_99(),_op_fd_9a(),_op_fd_9b(),_op_fd_9c(),_op_fd_9d(),_op_fd_9e(),_op_fd_9f();
     void _op_fd_a0(),_op_fd_a1(),_op_fd_a2(),_op_fd_a3(),_op_fd_a4(),_op_fd_a5(),_op_fd_a6(),_op_fd_a7();
     void _op_fd_a8(),_op_fd_a9(),_op_fd_aa(),_op_fd_ab(),_op_fd_ac(),_op_fd_ad(),_op_fd_ae(),_op_fd_af();
     void _op_fd_b0(),_op_fd_b1(),_op_fd_b2(),_op_fd_b3(),_op_fd_b4(),_op_fd_b5(),_op_fd_b6(),_op_fd_b7();
     void _op_fd_b8(),_op_fd_b9(),_op_fd_ba(),_op_fd_bb(),_op_fd_bc(),_op_fd_bd(),_op_fd_be(),_op_fd_bf();
     void _op_fd_c0(),_op_fd_c1(),_op_fd_c2(),_op_fd_c3(),_op_fd_c4(),_op_fd_c5(),_op_fd_c6(),_op_fd_c7();
     void _op_fd_c8(),_op_fd_c9(),_op_fd_ca(),_op_fd_cb(),_op_fd_cc(),_op_fd_cd(),_op_fd_ce(),_op_fd_cf();
     void _op_fd_d0(),_op_fd_d1(),_op_fd_d2(),_op_fd_d3(),_op_fd_d4(),_op_fd_d5(),_op_fd_d6(),_op_fd_d7();
     void _op_fd_d8(),_op_fd_d9(),_op_fd_da(),_op_fd_db(),_op_fd_dc(),_op_fd_dd(),_op_fd_de(),_op_fd_df();
     void _op_fd_e0(),_op_fd_e1(),_op_fd_e2(),_op_fd_e3(),_op_fd_e4(),_op_fd_e5(),_op_fd_e6(),_op_fd_e7();
     void _op_fd_e8(),_op_fd_e9(),_op_fd_ea(),_op_fd_eb(),_op_fd_ec(),_op_fd_ed(),_op_fd_ee(),_op_fd_ef();
     void _op_fd_f0(),_op_fd_f1(),_op_fd_f2(),_op_fd_f3(),_op_fd_f4(),_op_fd_f5(),_op_fd_f6(),_op_fd_f7();
     void _op_fd_f8(),_op_fd_f9(),_op_fd_fa(),_op_fd_fb(),_op_fd_fc(),_op_fd_fd(),_op_fd_fe(),_op_fd_ff();
     */
    void _op_fd_09();
    void _op_fd_19();
    void _op_fd_21(),_op_fd_22(),_op_fd_23(),_op_fd_29(),_op_fd_2a(),_op_fd_2b();
    void _op_fd_34(),_op_fd_35(),_op_fd_36(),_op_fd_39();
    void _op_fd_46(),_op_fd_4e();
    void _op_fd_56(),_op_fd_5e();
    void _op_fd_66(),_op_fd_6e();
    void _op_fd_70(),_op_fd_71(),_op_fd_72(),_op_fd_73(),_op_fd_74(),_op_fd_75(),_op_fd_77(),_op_fd_7e();
    void _op_fd_86(),_op_fd_8e();
    void _op_fd_96(),_op_fd_9e();
    void _op_fd_a6(),_op_fd_ae();
    void _op_fd_b6(),_op_fd_be();
    void _op_fd_cb();
    void _op_fd_e1(),_op_fd_e3(),_op_fd_e5(),_op_fd_e9();
    void _op_fd_f9();
    
    
    /*
     * clock
     */
    Byte _curclock;
    static Byte _clock[256],_clock_cb[256],_clock_dd[256],_clock_ed[256],_clock_dd_cb[256];
    
};



#endif /* z80_hpp */
