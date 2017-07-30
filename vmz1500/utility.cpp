//
//  utility.cpp
//  vmz1500
//
//  Created by murasuke on 2017/04/19.
//  Copyright © 2017年 murasuke. All rights reserved.
//

#include <fstream>
#include "utility.hpp"

/*
 load binary file to bytearray
 */
bool loadBinFile(const std::string &filename, ByteArray &buf)
{
    std::ifstream ifs;
    ifs.open(filename);
    if (!ifs.is_open()) return false;
    
    //file size
    ifs.seekg(0, std::fstream::end);
    auto endp=ifs.tellg();
    ifs.clear();
    ifs.seekg(0, std::fstream::beg);
    auto begp=ifs.tellg();
    auto fsize=endp-begp;
    
    //##ファイルサイズの制限がない
    buf.reserve(fsize);
    buf.resize(fsize);
    ifs.read((char*)buf.data(), fsize);
    ifs.close();
    return true;
}
