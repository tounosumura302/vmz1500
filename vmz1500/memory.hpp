//
//  memory.hpp
//  vmz1500
//
//  Created by murasuke on 2017/03/27.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#ifndef memory_hpp
#define memory_hpp

#include <stdio.h>

//#include <vector>

#include "type.h"

//using ByteArray=std::vector<Byte>;


/*
 * memory block
 */
class MemoryBlock
{
public:
    MemoryBlock(int size);
    virtual ~MemoryBlock();
    
    virtual const Byte at(int i) const;
    virtual void replace(int i,const Byte & value);
    
    void setImage(const ByteArray &data);
    void load(Word address,const ByteArray &data,int offset,int size);

    /*
signals:
    void messageSent(const QString &) const;
    void violationOccured() const;
    */
    
protected:
    ByteArray *_mem;
};

/*
 ROM
 */

class MemoryBlockRom : public MemoryBlock
{
public:
    MemoryBlockRom(int size);
    ~MemoryBlockRom();
    
    void replace(int i,const Byte & value);
};



/*
 * MMU(Memory Management Unit)
 */

class Memory
{
public:
    Memory();
    virtual ~Memory();
    
    virtual const Byte at(int i) const=0;
    virtual void replace(int i,const Byte & value)=0;
};


#endif /* memory_hpp */
