//
//  vmz1500ui.cpp
//  vmz1500
//
//  Created by murasuke on 2017/03/30.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#include "vmz1500ui.hpp"

//Vmz1500Crt::Vmz1500Crt(std::shared_ptr<SDL_Renderer> &render)
Vmz1500Crt::Vmz1500Crt(SDL_Renderer *render)
{
    _render=render;
    
    _pcgpp=new SDL_Point*[8];
    _pcgpp[0]=new SDL_Point[8*8];
    _pcgpp[1]=new SDL_Point[8*8];
    _pcgpp[2]=new SDL_Point[8*8];
    _pcgpp[3]=new SDL_Point[8*8];
    _pcgpp[4]=new SDL_Point[8*8];
    _pcgpp[5]=new SDL_Point[8*8];
    _pcgpp[6]=new SDL_Point[8*8];
    _pcgpp[7]=new SDL_Point[8*8];
    
    _pcgpps=new int[8];
    
    
}

Vmz1500Crt::~Vmz1500Crt()
{
    delete [] _pcgpp[0];
    delete [] _pcgpp[1];
    delete [] _pcgpp[2];
    delete [] _pcgpp[3];
    delete [] _pcgpp[4];
    delete [] _pcgpp[5];
    delete [] _pcgpp[6];
    delete [] _pcgpp[7];
    delete [] _pcgpp;
    delete [] _pcgpps;
}

void Vmz1500Crt::refresh(const Vmz1500 &vm)
{
    int offset=0;
    Word v,attr,pcgn1,pcgn2;
    SDL_Rect rect;
    rect.w=8;
    rect.h=8;
    Vmz1500Palette fc,bc;
    for(int y=0;y<25*8;y+=8){
        for(int x=0;x<40*8;x+=8){
            attr=vm.vramValue(offset+0x0800);
            //@@test
            //@@color=0 -> 0x7f (prevent from invisible character)
            //@@i don't know who initialize attribute vram...
            /*
            if ((attr&0x77)==0){
                attr=(attr&0x80)|0x71;
            }
             */
            //@@end test

            //background
            bc=vm.textPalette(attr&0x07);
            rect.x=x;
            rect.y=y;
            SDL_SetRenderDrawColor(_render, bc.r, bc.g, bc.b, 255);
            SDL_RenderFillRect(_render, &rect);
            
            //text
            fc=vm.textPalette((attr>>4)&0x07);
            v=vm.vramValue(offset)|((attr&0x80)<<1);
            //プライオリティにより描画順序を変更（低い方から描画）
            if (vm.pcgPriority()){
                if (v>0){
                    this->drawTextChar(vm,x,y,v,fc);
                }
            }
            if (vm.pcgVisible()){
                //pcg
                pcgn2=vm.vramValue(offset+0x0c00);
                if (pcgn2&0b00001000){
                    pcgn1=(Word)vm.vramValue(offset+0x0400)|((pcgn2&0b11000000)<<2);
                    this->drawPcgChar(vm, x, y, pcgn1);
                }
            }
            if (!vm.pcgPriority()){
                if (v>0){
                    this->drawTextChar(vm,x,y,v,fc);
                }
            }
            //
            offset++;
        }
    }
}



void Vmz1500Crt::drawTextChar(const Vmz1500 &vm, int x, int y, Word c, Vmz1500Palette fc)
{
    int adr=c*8;
    
    SDL_Point p[8*8];
    int len=0,pat,j,k;
    for(j=0;j<8;j++){
        pat=vm.cgromValue(adr);//    cgrom.at(adr);
        for(k=0;k<8;k++){
            if (pat&0x80){
                p[len].x=x+k;   //setX(x+k);
                p[len].y=y+j;   //setY(y+j);
                len++;
            }
            pat=pat<<1;
        }
        adr++;
    }
    
    if (len>0){
        SDL_SetRenderDrawColor(_render, fc.r, fc.g, fc.b, 255);
        SDL_RenderDrawPoints(_render, p, len);
    }
}

void Vmz1500Crt::drawPcgChar(const Vmz1500 &vm, int x, int y, Word c)
{
    int adr=c*8;
    int i,j;
    for(i=0;i<8;i++){
        _pcgpps[i]=0;
    }
    
    Word pr,pb,pg,ix;
    for(j=0;j<8;j++){
        pb=(Word)vm.pcgBValue(adr);
        pr=((Word)vm.pcgRValue(adr))<<1;
        pg=((Word)vm.pcgGValue(adr))<<2;
        if (pb|pr|pg!=0){
            for(i=7;i>=0;i--){
                ix=(pg&0b00000100)|(pr&0b00000010)|(pb&0b00000001);
                _pcgpp[ix][_pcgpps[ix]].x=x+i;
                _pcgpp[ix][_pcgpps[ix]].y=y+j;
                _pcgpps[ix]++;
                pg>>=1;
                pr>>=1;
                pb>>=1;
            }
        }
        adr++;
    }
    
    Vmz1500Palette pal;
    for(i=1;i<8;i++){
        if (_pcgpps[i]>0){
            pal=vm.textPalette(i);
            SDL_SetRenderDrawColor(_render, pal.r, pal.g, pal.b, 255);
            SDL_RenderDrawPoints(_render, _pcgpp[i], _pcgpps[i]);
        }
    }
}

