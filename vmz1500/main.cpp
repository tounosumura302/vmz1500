//
//  main.cpp
//  vmz1500
//
//  Created by murasuke on 2017/03/27.
//  Copyright © 2017年 murasuke. All rights reserved.
//


#include <iostream>
#include <thread>

#include <SDL2/SDL.h>

#include "vmz1500.hpp"
#include "vmz1500ui.hpp"
#include "vmz1500sound.hpp"


using namespace std;



//ユーザーイベントコード
typedef enum{
    VSYNC,  //vsync
}
    VmEventCode;



int main(int argc, const char * argv[]) {
    std::cout << "begin vmz1500\n";

    
    //vm用意
    Vmz1500 vm(3.579545*1000000);   //CPU周波数 3.57Mhz
    vm.init();

    //##ファイルからROMなどのイメージを読み込む
    //##これはテストのために適当にやってるだけ
    if (vm.loadCgrom("/Users/bonze/dev/vmz1500/resource/MZ7CGjp.rom")){
        std::cout<<"cgrom read successfully\n";
    }
//    if (vm.load(0, "/Users/bonze/dev/vmz1500/resource/NEWMON7.ROM")){
    /*
    if (vm.load("/Users/bonze/dev/vmz1500/resource/1Z-009B.ROM",0)){
        std::cout<<"monitor rom read successfully\n";
    }
     */
    if (vm.load("/Users/bonze/dev/vmz1500/resource/9z-502m.rom",0x0000,0x0000,0x1000)){
        std::cout<<"monitor rom read successfully\n";
    }
    if (vm.load("/Users/bonze/dev/vmz1500/resource/9z-502m.rom",0xe800,0x1000,0x1800)){
        std::cout<<"monitor rom read successfully\n";
    }
    
    //##
/*
    Word execadr;
//    if (vm.loadTapeImage("/Users/bonze/dev/vmz1500/resource/F1200.mzf", execadr)){//bomber man  ->ok
//    if (vm.loadTapeImage("/Users/bonze/dev/vmz1500/resource/BdHopper.mzt", execadr)){//building hopper ->??
//    if (vm.loadTapeImage("/Users/bonze/dev/vmz1500/resource/CannonBall.mzf", execadr)){//cannon ball ->ok
//    if (vm.loadTapeImage("/Users/bonze/dev/vmz1500/resource/PCGRALLY.mzf", execadr)){//rally x??
//    if (vm.loadTapeImage("/Users/bonze/dev/vmz1500/resource/druaga/Druaga_B.mzt", execadr)){//druaga ??
//    if (vm.loadTapeImage("/Users/bonze/dev/vmz1500/resource/ZELBUS.MZT", execadr)){//zelbus  ->ok
    if (vm.loadTapeImage("/Users/bonze/dev/vmz1500/resource/ZELDIS.MZT", execadr)){//zeldis ->almost ok?
//    if (vm.loadTapeImage("/Users/bonze/dev/vmz1500/resource/ZEPLIS.MZT", execadr)){//zeplis ->almost ok
//    if (vm.loadTapeImage("/Users/bonze/dev/vmz1500/resource/spaceharrier.m12", execadr)){//
//    if (vm.loadTapeImage("/Users/bonze/dev/vmz1500/resource/t_tunnel/TIMET2A.MZT", execadr)){//
//    if (vm.loadTapeImage("/Users/bonze/dev/vmz1500/resource/GALAGA.MZT", execadr)){//
//    if (vm.loadTapeImage("/Users/bonze/dev/vmz1500/resource/ThunderF.mzt", execadr)){//
        //vm.jump(execadr);
        std::cout<<"tape image loaded\n";
    }
*/
    
    //## QD test
//    if (vm.setQD("/Users/bonze/dev/vmz1500/resource/qd/game_m_a.mzt")){
    if (vm.setQD("/Users/bonze/dev/vmz1500/resource/y2k/Y2K.MZT")){
//        if (vm.setQD("/Users/bonze/dev/vmz1500/resource/gameroman_1500/holy-a.mzt")){
//        if (vm.setQD("/Users/bonze/dev/vmz1500/resource/gameroman_1500/feizer21-a.mzt")){
//        if (vm.setQD("/Users/bonze/dev/vmz1500/resource/gameroman_1500/devil-a.mzt")){
 //       if (vm.setQD("/Users/bonze/dev/vmz1500/resource/eugea/eugea.m12")){
//            if (vm.setQD("/Users/bonze/dev/vmz1500/resource/furu1/pendant.m12")){
//            if (vm.setQD("/Users/bonze/dev/vmz1500/resource/furu1/eyelarth.m12")){
//            if (vm.setQD("/Users/bonze/dev/vmz1500/resource/furu1/sideroll-f-.m12")){
//            if (vm.setQD("/Users/bonze/dev/vmz1500/resource/1500game/S-WORLD.M12")){
//            if (vm.setQD("/Users/bonze/dev/vmz1500/resource/1500game/CRUISER3.M12")){
//            if (vm.setQD("/Users/bonze/dev/vmz1500/resource/qd/basic_5z001_a.mzt")){
//            if (vm.setQD("/Users/bonze/dev/vmz1500/resource/qd/gateoflabyrinth_a.mzt")){
//        if (vm.setQD("/Users/bonze/dev/vmz1500/resource/qd/yakyukyo_a.mzt")){
//        if (vm.setQD("/Users/bonze/dev/vmz1500/resource/qd/edasm_a.mzt")){
//        if (vm.setQD("/Users/bonze/dev/vmz1500/resource/qd/flappy.mzt")){
//        if (vm.setQD("/Users/bonze/dev/vmz1500/resource/qd/knither_a.mzt")){
//    if (vm.setQD("/Users/bonze/dev/vmz1500/resource/qd/demoncrystal_a.mzt")){
//        if (vm.setQD("/Users/bonze/dev/vmz1500/resource/qd/cosmoblaster_a.mzt")){
//    if (vm.setQD("/Users/bonze/dev/vmz1500/resource/druaga/Druaga_A.mzt")){
//    if (vm.setQD("/Users/bonze/dev/vmz1500/resource/GENOSS_700.MZT")){
//    if (vm.setQD("/Users/bonze/dev/vmz1500/resource/ZELBUS.MZT")){
//    if (vm.setQD("/Users/bonze/dev/vmz1500/resource/ZELDIS.MZT")){
//    if (vm.setQD("/Users/bonze/dev/vmz1500/resource/ZEPLIS2.MZT")){
//    if (vm.setQD("/Users/bonze/dev/vmz1500/resource/ZEPLIS3.MZT")){
//    if (vm.setQD("/Users/bonze/dev/vmz1500/resource/BdHopper.mzt")){
//    if (vm.setQD("/Users/bonze/dev/vmz1500/resource/ThunderF.mzt")){
//    if (vm.setQD("/Users/bonze/dev/vmz1500/resource/spaceharrier.m12")){
//    if (vm.setQD("/Users/bonze/dev/vmz1500/resource/GALAGA.MZT")){
//    if (vm.setQD("/Users/bonze/dev/vmz1500/resource/PCGRALLY.MZF")){
        std::cout<<"QD image loaded\n";
    }
    
    

    //vmからメインスレッドに渡すイベント
    Uint32 vmeventtype=SDL_RegisterEvents(1);
    if (vmeventtype==-1){
        std::cerr<<"error : SDL_RegisterEvent() returns -1"<<std::endl;
        return 0;
    }
    SDL_Event vmevent;
    SDL_zero(vmevent);
    vmevent.type=vmeventtype;
    
    //SDLの初期化
    SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO);
    SDL_Window* window = SDL_CreateWindow("VMz1500 beta",SDL_WINDOWPOS_CENTERED,SDL_WINDOWPOS_CENTERED,640,400,SDL_WINDOW_RESIZABLE);
    SDL_Renderer* render = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED|SDL_RENDERER_TARGETTEXTURE);
    SDL_Texture* vmtex=SDL_CreateTexture(render, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_TARGET, 320, 200);

    //sound
    Vmz1500SndDrv snddrv;
    snddrv.init();
    Vmz1500Psg& psg=snddrv.psg();


    //描画オブジェクトの用意
    Vmz1500Crt crt(render);

    //vmのスレッドを起動
    std::thread vmthread([&]{
        Uint32 timer=SDL_GetTicks();    //current tick
        Uint32 msec;
        int result;
        while(vm.isOk()){
            result=vm.run();
            if (result&Vmz1500::VSYNC){
                //垂直帰線期間が経過した状態
                //ここでSDLの描画はできないため、イベントでメインスレッドに通知
                vmevent.user.code=VSYNC;
                SDL_PushEvent(&vmevent);

                //時間が余っていたら待つ
                msec=SDL_GetTicks()-timer;
                if (msec<1000/60){
                    SDL_Delay(1000/60-msec);
                }
                timer=SDL_GetTicks();
            }
            if (result&Vmz1500::PSG1){
                //##
                vm.procPsg1([&](int chl,int div){psg.setTone(chl, div);}, [&](int chl,int vol){psg.setVolume(chl, vol);});
            }
            if (result&Vmz1500::PSG2){
                //##
                vm.procPsg2([&](int chl,int div){psg.setTone(chl+4, div);}, [&](int chl,int vol){psg.setVolume(chl+4, vol);});
            }
            if (result&Vmz1500::I8253SOUND){
                vm.procI8253Sound([&](int f){
                    if (f>0){
                        psg.setTone8253(f);
                    }
                    else{
                        psg.setVolume8253(15);
                    }
                });
            }
        }
    });
    
    //イベントループ
    SDL_Event ev;
    bool loop=true;
    while(loop){
        while(SDL_PollEvent(&ev))
        {
            //user event
            if (ev.type==vmeventtype){
                switch(ev.user.code){
                        //子スレッドでSDLの描画はできないので、描画タイミングは子スレッドからイベントとして通知してもらう
                    case VSYNC:
                        SDL_SetRenderTarget(render, vmtex);
                        SDL_SetRenderDrawColor(render, 0, 0, 0, 255);
                        SDL_RenderClear(render);
                        crt.refresh(vm);
                        SDL_SetRenderTarget(render, NULL);
                        SDL_RenderCopy(render, vmtex, NULL, NULL);
                        SDL_RenderPresent(render);
                        break;
                }
            }
            //system event
            else{
                switch (ev.type) {
                    case SDL_QUIT:
                        loop=false;
                        break;
                    case SDL_KEYDOWN:
                        vm.pressKey(ev.key.keysym.sym);
                        
                        //##test  switch qd media
                        if (ev.key.keysym.sym==SDLK_TAB){
//                            if (vm.setQD("/Users/bonze/dev/vmz1500/resource/qd/game_m_a.mzt")){
                           if (vm.setQD("/Users/bonze/dev/vmz1500/resource/druaga/Druaga_B.mzt")){
//                            if (vm.setQD("/Users/bonze/dev/vmz1500/resource/gameroman_1500/holy-b.mzt")){
//                                if (vm.setQD("/Users/bonze/dev/vmz1500/resource/gameroman_1500/feizer21-b.mzt")){
//                                if (vm.setQD("/Users/bonze/dev/vmz1500/resource/gameroman_1500/devil-b.mzt")){
//                                if (vm.setQD("/Users/bonze/dev/vmz1500/resource/qd/gateoflabyrinth_b.mzt")){
//                            if (vm.setQD("/Users/bonze/dev/vmz1500/resource/qd/demoncrystal_b.mzt")){
//                            if (vm.setQD("/Users/bonze/dev/vmz1500/resource/qd/knither_b.mzt")){
//                            if (vm.setQD("/Users/bonze/dev/vmz1500/resource/qd/flappyb.mzt")){
                                std::cout<<"QD image loaded\n";
                            }
                        }

                        break;
                    case SDL_KEYUP:
                        vm.releaseKey(ev.key.keysym.sym);
                        break;
                        
                    default:
                        break;
                }
            }
        }
    }

    //終了
    std::cout<<"stopping mz1500..."<<std::endl;
    vm.stop();
    vmthread.join();
    std::cout<<"mz1500 stopped."<<std::endl;
    
    return 0;
}
