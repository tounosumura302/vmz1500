//
//  sequeue.hpp
//  vmz1500
//
//  Created by murasuke on 2017/03/30.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#ifndef sequeue_hpp
#define sequeue_hpp

#include <stdio.h>


/*
 スレッドセーフ版簡易メッセージキュー
 キューの長さは init() で指定し、変更不可
 キューに入れるデータはキューにコピーされるため、あまり大きなものにしないこと
 （コード上はテンプレートなのでなんでも大丈夫だが）
 
 キューの大きさは固定。
 
 */
#include <vector>
#include <mutex>

template <typename T> class SEQueue
{
public:
    SEQueue();
    virtual ~SEQueue() = default;
    
    void init(int max);
    void clear();
    
    bool push(const T &id);
    bool pop(T &id);
    
protected:
    using MsgQueue=std::vector<T>;
    MsgQueue _queue;
    
    int _max;
    int _cur;
    int _newest;
    
    std::mutex _mutex;
};


#endif /* sequeue_hpp */
