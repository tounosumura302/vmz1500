//
//  sn76489.cpp
//  vmz1500
//
//  Created by murasuke on 2017/05/11.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#include "sn76489.hpp"

Sn76489::Sn76489()
{
    init();
}

void Sn76489::init()
{
    _register=0;
    _updateany=false;
    for(int i=0;i<NUM_CHLS;i++){
        _divratio[i]=0;
        _volume[i]=0;
        _updatediv[i]=false;
        _updatevol[i]=false;
    }
}

void Sn76489::set(Byte f)
{
    //
    if (f&0x80){
        //トーンの第１バイト
        if ((f&0x10)==0){
            _register=(f&0x70)>>5;
            _divratio[_register]&=0x3f0;
            _divratio[_register]|=(Word)(f&0x0f);
            _updatediv[_register]=false;
        }
        //ボリューム
        else{
            _volume[(f&0x60)>>5]=f&0x0f;
            _updatevol[(f&0x60)>>5]=true;
            _updateany=true;
        }
    }
    //トーンの第２バイト
    else{
        _divratio[_register]&=0x0f;
        _divratio[_register]|=(((Word)f)&0x3f)<<4;
        _updatediv[_register]=true;
        _updateany=true;
    }
}

bool Sn76489::isUpdated() const
{
    return _updateany;
}

void Sn76489::resetUpdate()
{
    _updateany=false;
}

/*
Word Sn76489::divRatio(int chl) const
{
    //##chlの範囲チェックしておらず
    return _divratio[chl];
}

Byte Sn76489::volume(int chl) const
{
    //##chlの範囲チェックしておらず
    return _volume[chl];
}
*/
 
void Sn76489::procUpdatedDivRatio(Sn76489DivRatioFunc func)
{
    for(int i=0;i<NUM_CHLS;i++){
        if (_updatediv[i]){
            func.operator()(i, _divratio[i]);
            _updatediv[i]=false;
        }
    }
}

void Sn76489::procUpdatedVolume(Sn76489VolumeFunc func)
{
    for(int i=0;i<NUM_CHLS;i++){
        if (_updatevol[i]){
            func.operator()(i, _volume[i]);
            _updatevol[i]=false;
        }
    }
}