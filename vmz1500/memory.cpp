//
//  memory.cpp
//  vmz1500
//
//  Created by murasuke on 2017/03/27.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#include <iostream>
#include "memory.hpp"
#include "utility.hpp"

//#include "mz1500.h"
//#include <QDebug>

/*
 * memory block
 */

MemoryBlock::MemoryBlock(int size)
{
    _mem=new ByteArray(size,(char)0);
}

MemoryBlock::~MemoryBlock()
{
    delete _mem;
}

const Byte MemoryBlock::at(int i) const
{
    //qDebug()<<QString("size=%1").arg(_mem->size());
    return(_mem->at(i));
}

void MemoryBlock::replace(int i, const Byte &value)
{
    //    _mem->replace(i,value);
    _mem->operator [](i)=value;
}

void MemoryBlock::setImage(const ByteArray &data)
{
//    _mem->replace(0,data.length(),data);
    memcpy(_mem->data(), data.data(), data.size());
}

void MemoryBlock::load(Word address, const ByteArray &data, int offset,int size)
{
    if (offset+size>data.size()){
        size=data.size()-offset;
    }
    memcpy(_mem->data()+address, data.data()+offset, size);

    /*
    //_memのreplaceされる部分のサイズと、replaceするデータのサイズが一致していないと、replaceされる_mem全体のサイズが変化してしまう。
    //replace()の２つ目の値（replaceする部分のサイズ）!=data.mid().size() （replaceするデータのサイズ）
    //だと、replaceする部分のサイズがreplaceするデータのサイズに変化してしまう仕様のため。
    if (offset+size>data.size()){
        size=data.size()-offset;
    }
    _mem->replace(address,size,data.mid(offset,size));
     */
}

/*
 * memory block (ROM)
 */

MemoryBlockRom::MemoryBlockRom(int size) : MemoryBlock(size){}
MemoryBlockRom::~MemoryBlockRom(){}

void MemoryBlockRom::replace(int /*i*/, const Byte & /*value*/){}    //do not replace because of read only memory


/*
 * MMU
 */

Memory::Memory()
{}

Memory::~Memory()
{}

/*
 *
 */

