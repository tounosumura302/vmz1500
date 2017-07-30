//
//  vmz1500ui.hpp
//  vmz1500
//
//  Created by murasuke on 2017/03/30.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#ifndef vmz1500ui_hpp
#define vmz1500ui_hpp

#include <stdio.h>

#include <memory>
#include <SDL2/SDL.h>

#include "type.h"
#include "vmz1500.hpp"

class Vmz1500Crt
{
public:
    Vmz1500Crt(SDL_Renderer *render);
    virtual ~Vmz1500Crt();
    
    void refresh(const Vmz1500 &vm);
    
private:
    void drawTextChar(const Vmz1500 &vm, int x, int y, Word c,Vmz1500Palette fc);
    
    //pcg drawing
private:
    void drawPcgChar(const Vmz1500 &vm,int x,int y,Word c);
    SDL_Point **_pcgpp;
    int *_pcgpps;
    
    
private:
    SDL_Renderer *_render;
};

#endif /* vmz1500ui_hpp */
