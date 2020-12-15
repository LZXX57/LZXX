/*********************************************************
 * 
 * inflate.h -- internal inflate state definition
 * 
**********************************************************/

#ifndef _INFLATE_H__
#define _INFLATE_H__

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

struct inflate_state {
    // 保证文件在解压不满一个字节时，或者跨字节边界解压又刚好到达输入文件末尾时能记录信息
    h_streamptr strm;   // 指向输入输出流及内存申请函数
    h_mode hmode;       // 记录当前解压类型 copy 游程编码，lltree，dtree，data_stream 不同解压内容策略不同
    uInt8 block_type;   // block的类型
    
    struct stored_mode{             // stored模式对象属性
        uInt block_size;            // 记录block大小，不会超过65535
        uInt block_decompress_size; // 记录已解压的block大小
    };
    struct stored_mode stored_state;

    struct compress_mode{           // 压缩模式对象属性
        uInt8 lltree_syms;          // 字符和匹配长度sym数 need 5 bits
        uInt8 lltree_decode_len;    // 字符和匹配长度sym解码数
        h_treePtr lltree;           // 字符和匹配长度huffman树
        uInt8 dtree_syms;           // 偏移距离sym数 need 5 bits
        uInt8 dtree_decode_len;     // 偏移距离sym解码数
        h_treePtr dtree;            // 偏于距离huffman树

        uInt code_lens[320];        // 游程编码的编码长度 256+1+30+30
        uInt8 runltree_decode_len;  // 游程树字符sym解码数 need 4 bits
        h_treePtr runltree;         // 游程编码树

        h_codePtr llcode;           // 字符和匹配长度解码信息   解码table换为code数组？
        uInt lenbits;
        h_codePtr dcode;            // 偏于距离解码信息
        uInt distbits;

        uLong hold;                 // 待解压的数据流的(当前input解码完但是并没有解码到最后，输入为部分压缩文件，因此需要记录未解压的最后剩余的数据，方便和新的输入一起解压)
        uInt bits;                  // hold 中的有效bit
    };
    struct compress_mode compress_state;

    struct window_buf{
        uInt8 *window;       // 输出缓存区 不小于32K，满足偏移量的最大搜寻距离 （每次扩大为上一次的两倍？？）
        uLong capacity;     // window 实际空间
        uLong size;         // window 已使用空间
    };
    struct window_buf window;

    // 记录各种解压类型的 解码信息
    // 记录输出数据的缓存及输出数据的当前位置，方便 lz77 跨输入的解码
    // 解压文件当前大小 交给上一级 h_streamptr strm; 记录
    
};

/********************************************/
/*                API ZONE                  */
/********************************************/

/* inflate init z_stream struct construct */
HEXTERN int inflateInit(h_streamptr strm);

/* inflate decompress */
HEXTERN int inflate(h_streamptr strm, int flush);

/* inflate end z_stream struct deconstruct */
HEXTERN int inflateEnd(h_streamptr strm);

#ifdef __cplusplus
}
#endif

#endif /* _INFLATE_H__ */