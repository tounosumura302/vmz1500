//
//  vmz1500qd.cpp
//  vmz1500
//
//  Created by murasuke on 2017/04/17.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#include <fstream>
#include "utility.hpp"
#include "vmz1500qd.hpp"

Vmz1500QD::Vmz1500QD()
{
    init();
}

bool Vmz1500QD::init()
{
    _motor=false;
    _imgfilename.empty();
    _image.empty();
    _imageptr=0;
    
    _rega=0;
    _regb=0;
    
    return true;
}

bool Vmz1500QD::isQDSet() const
{
    return _imgfilename.length()!=0;
}

void Vmz1500QD::eject()
{
    init();
}

bool Vmz1500QD::setQD(const std::string &filename)
{
    eject();

    ByteArray buf;
    if (!loadBinFile(filename, buf))    return false;
    
    auto filesz=buf.size();
    _image.resize(filesz);
    _image.reserve(filesz);
    
    Byte blocknum=0;
    
    //## blocks
    _image[0]=0xa5; //data mark
    _image[1]=2;    //number of blocks
    _image[2]=0;    //crc
    _image[3]=0;    //crc
    _image[4]=0;    //sync
    
    Byte attribute;
    Word datasize;
    size_t ix,bufix;
    for (bufix=0,ix=5; filesz>=128; ) {
        //##information
        _image[ix++]=0xa5; //data mark
        _image[ix++]=0;   //block flag
        _image[ix++]=64;   //block size
        _image[ix++]=0;    //block size
        //_image[ix++]=1;    //attribute
        memcpy(&_image[ix], buf.data()+bufix, 18); //attribute + file name
        attribute=_image[ix];   //cache attribute
        ix+=18;
        //_image[ix++]=0x0d;    //file name terminator
        _image[ix++]=0;   //lock
        _image[ix++]=0;   //secret
        memcpy(&_image[ix], buf.data()+bufix+0x12, 44);   //size+load addr+exec addr+comment
        datasize=_image[ix]|(_image[ix+1]<<8);  //cache datasize
        ix+=44;
        _image[ix++]=0;    //crc
        _image[ix++]=0;    //crc
        _image[ix++]=0;    //sync
        
        filesz-=128;
        bufix+=128;
        blocknum++;
        
        //##data
        if (filesz<datasize)    break;
        _image[ix++]=0xa5; //data mark
        _image[ix++]=0x01;   //block flag
        _image[ix++]=datasize&0xff;   //block size
        _image[ix++]=(datasize&0xff00)>>8;
        memcpy(&_image[ix], buf.data()+bufix, datasize);
        ix+=datasize;
        _image[ix++]=0;    //crc
        _image[ix++]=0;    //crc
        _image[ix++]=0;    //sync
        
        filesz-=datasize;
        bufix+=datasize;
        blocknum++;
    }
    _image[1]=blocknum;    //number of blocks
    _image.resize(ix);
    
    /*
    //## mzt形式専用
    // blocks(5)+information(4+64+3)+data(4+imagesize+3)
    int imagesize=5+(4+64+3)+(4+((Byte)buf.at(0x12)+((Byte)buf.at(0x13))*256)+3);
    _image.reserve(imagesize);
    _image.resize(imagesize);
    
    //##blocks
    _image[0]=0xa5; //data mark
    _image[1]=2;    //number of blocks
    _image[2]=0;    //crc
    _image[3]=0;    //crc
    _image[4]=0;    //sync
    
    //##information
    _image[5]=0xa5; //data mark
    _image[6]=0;   //block flag
    _image[7]=64;   //block size
    _image[8]=0;    //block size
    _image[9]=1;    //attribute
    memcpy(&_image[10], buf.data()+1, 16); //attribute + file name
    _image[26]=0x0d;    //file name terminator
    _image[27]=0;   //lock
    _image[28]=0;   //secret
    memcpy(&_image[29], buf.data()+0x12, 44);   //size+load addr+exec addr+comment
    _image[73]=0;    //crc
    _image[74]=0;    //crc
    _image[75]=0;    //sync
    
    //##data
    Word datasize=_image[29]|(_image[30]<<8);
    _image[76]=0xa5; //data mark
    _image[77]=0x01;   //##block flag  (OBJ)
    _image[78]=datasize&0xff;   //block size
    _image[79]=(datasize&0xff00)>>8;
    memcpy(&_image[80], buf.data()+0x80, datasize);
    _image[80+datasize]=0;    //crc
    _image[81+datasize]=0;    //crc
    _image[82+datasize]=0;    //sync
*/
    
    return true;
}


void Vmz1500QD::setSioACtrl(Byte value)
{
    //## 9z-502mでは何もしない
    
    //次回のレジスタ番号を記録
    _rega=(_rega==0) ? value&0x07 : 0;
}

void Vmz1500QD::setSioBCtrl(Byte value)
{
    //## 9z-502m
    //## WR5 に書き込まれる値によって
    //## モーターがoffのときに0以外    モーターon&イメージファイルを先頭へ
    //## 0  モーターoff
    if (_regb==5){
        if (value==0){
            motorOff();
        }
        else{
//            if ((value==0x82)||(!isMotorOn())){
            if ((!isMotorOn())){
                motorOn();
            }
        }
    }
    
    //次回のレジスタ番号を記録
    _regb=(_regb==0) ? value&0x07 : 0;
}

void Vmz1500QD::setSioAData(Byte value)
{
    //## QDの書き込みが必要になるまで実装する必要なし
}

void Vmz1500QD::setSioBData(Byte value)
{
    //## QDでは未使用
}

Byte Vmz1500QD::sioACtrl()
{
    //## 9z-502mに対しては 0x2d を返しておけばエラーにはならない
    //## SIO RR0,RR1,RR2共通
    return 0x2d;
}

Byte Vmz1500QD::sioBCtrl()
{
    //## 9z-502mに対しては、指定されたレジスタごとに返しておけば良い値が異なる
    //## RR0 = 0x08
    //## RR2 = 0x81
    //## それ以外　9z-502mではINするケースがないので適当
    switch (_regb) {
        case 0:
            return 0x08;
        case 2:
            return 0x81;
    }
    return 0;
}

Byte Vmz1500QD::sioAData()
{
    //## 9z-502m
    //## モーターがonのときは、イメージを１バイトずつ順に返す
    if (!isMotorOn())   return 0;
    
    if (_imageptr>=_image.size())   return 0;
    
    //##debug
    if (_imageptr>=0xa269){
        int bb=10;
        
    }

    return _image[_imageptr++];
}

Byte Vmz1500QD::sioBData()
{
    //## QDでは未使用
    return 0;
}

void Vmz1500QD::motorOn()
{
    
    _motor=true;
    _imageptr=0;
}

void Vmz1500QD::motorOff()
{
    _motor=false;
    _imageptr=0;
}

bool Vmz1500QD::isMotorOn() const
{
    return _motor;
}
