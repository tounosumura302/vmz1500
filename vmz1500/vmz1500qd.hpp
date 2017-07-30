//
//  vmz1500qd.hpp
//  vmz1500
//
//  Created by murasuke on 2017/04/17.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#ifndef vmz1500qd_hpp
#define vmz1500qd_hpp


#include <string>
#include "utility.hpp"
#include "type.h"

class Vmz1500QD
{
public:
    Vmz1500QD();
    virtual ~Vmz1500QD()=default;
    
    bool init();
    
    //QD
public:
    bool isQDSet() const;
    void eject();
    bool setQD(const std::string &filename);
    
    //image file
private:
    std::string _imgfilename;
    ByteArray _image;
    int _imageptr;
    
public:
    //write
    void setSioACtrl(Byte value);
    void setSioBCtrl(Byte value);
    void setSioAData(Byte value);
    void setSioBData(Byte value);
    
    //read
    Byte sioACtrl();
    Byte sioBCtrl();
    Byte sioAData();
    Byte sioBData();

private:
    Byte _rega; //register no(channel a)
    Byte _regb; //register no(channel b)
    
    //motor
private:
    void motorOn();
    void motorOff();
    bool isMotorOn() const;
    
    bool _motor;    //motor off/on
};

#endif /* vmz1500qd_hpp */
