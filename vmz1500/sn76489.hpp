//
//  sn76489.hpp
//  vmz1500
//
//  Created by murasuke on 2017/05/11.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#ifndef sn76489_hpp
#define sn76489_hpp

#include <functional>
#include "type.h"

using Sn76489DivRatioFunc=std::function<void(int,int)>;
using Sn76489VolumeFunc=std::function<void(int,int)>;

class Sn76489
{
public:
    Sn76489();
    ~Sn76489()=default;
    
    void init();
    
    void set(Byte f);
    
    bool isUpdated() const;
    void resetUpdate();
    
    //Word divRatio(int chl) const;
    //Byte volume(int chl) const;

    void procUpdatedDivRatio(Sn76489DivRatioFunc func);
    void procUpdatedVolume(Sn76489VolumeFunc func);

private:
    static const int NUM_CHLS=4;
    Word _divratio[NUM_CHLS];
    Byte _volume[NUM_CHLS];
    bool _updatediv[NUM_CHLS];
    bool _updatevol[NUM_CHLS];
    bool _updateany;
    Byte _register;
};

#endif /* sn76489_hpp */
