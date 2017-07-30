//
//  sequeue.cpp
//  vmz1500
//
//  Created by murasuke on 2017/03/30.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#include "sequeue.hpp"


template <typename T> SEQueue<T>::SEQueue()
{
    _max=0;
    _cur=0;
    _newest=1;
}

template <typename T> void SEQueue<T>::init(int max)
{
    {   //lock
        std::lock_guard<std::mutex> lock(_mutex);
        _max=max+1;
        _queue.reserve(_max);
        
        _cur=0;
        _newest=1;
    }   //unlock
}

template <typename T> void SEQueue<T>::clear()
{
    {   //lock
        std::lock_guard<std::mutex> lock(_mutex);

        _cur=0;
        _newest=1;
    }   //unlock
}

template <typename T> bool SEQueue<T>::push(const T &id)
{
    if (_newest==_cur)  return false;
    
    {   //lock
        std::lock_guard<std::mutex> lock(_mutex);
        _queue[_newest++]=id;
        if (_newest>=_max){
            _newest=0;
        }
        return true;
    }   //unlock
}

template <typename T> bool SEQueue<T>::pop(T &id)
{
    int tmp=_cur+1;
    if (tmp>=_max){
        tmp=0;
    }
    if (tmp==_newest)   return false;
    
    {   //lock
        std::lock_guard<std::mutex> lock(_mutex);
        _cur=tmp;
        id=_queue[_cur];
        return true;
    }   //unlock
}
