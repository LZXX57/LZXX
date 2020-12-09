/*********************************************************
 * inflate.h -- internal inflate state definition
 * 
**********************************************************/

#ifndef _INFLATE_H__
#define _INFLATE_H__

#include "common.h"

#ifdef __cplusplus
extern "C" {
#endif

/* inflate init z_stream struct construct */
ZEXTERN int inflateInit(z_streamp strm);

/* inflate decompress */
ZEXTERN int inflate(z_streamp strm, int flush);

/* inflate end z_stream struct deconstruct */
ZEXTERN int inflateEnd(z_streamp strm);

struct inflate_state {
    // 保证文件在解压不满一个字节时，或者跨字节边界解压又刚好到达输入文件末尾时能记录信息
    // copy 存储时的信息记录
    // 记录当前解压类型 游程编码，lltree，dtree，data_stream 不同解压内容策略不同
    // 记录各种解压类型的 解码信息
    // 记录输出数据的缓存及输出数据的当前位置，方便 lz77 跨输入的解码
    // 记录范式huffman树解码结构 三棵huffman树，动静态lltree，动静态dtree，游程编码tree
    // 解压文件当前大小
    // 上级结构体指针用于获取和输出inbuf和outbuf
    // z_streamp strm;             /* pointer back to this zlib stream */
    // inflate_mode mode;          /* current inflate mode */
    // int last;                   /* true if processing last block */
    // int wrap;                   /* bit 0 true for zlib, bit 1 true for gzip,
    //                                bit 2 true to validate check value */
    // int havedict;               /* true if dictionary provided */
    // int flags;                  /* gzip header method and flags (0 if zlib) */
    // unsigned dmax;              /* zlib header max distance (INFLATE_STRICT) */
    // unsigned long check;        /* protected copy of check value */
    // unsigned long total;        /* protected copy of output count */
    // gz_headerp head;            /* where to save gzip header information */
    //     /* sliding window */
    // unsigned wbits;             /* log base 2 of requested window size */
    // unsigned wsize;             /* window size or zero if not using window */
    // unsigned whave;             /* valid bytes in the window */
    // unsigned wnext;             /* window write index */
    // unsigned char FAR *window;  /* allocated sliding window, if needed */
    //     /* bit accumulator */
    // unsigned long hold;         /* input bit accumulator */
    // unsigned bits;              /* number of bits in "in" */
    //     /* for string and stored block copying */
    // unsigned length;            /* literal or length of data to copy */
    // unsigned offset;            /* distance back to copy string from */
    //     /* for table and code decoding */
    // unsigned extra;             /* extra bits needed */
    //     /* fixed and dynamic code tables */
    // code const FAR *lencode;    /* starting table for length/literal codes */
    // code const FAR *distcode;   /* starting table for distance codes */
    // unsigned lenbits;           /* index bits for lencode */
    // unsigned distbits;          /* index bits for distcode */
    //     /* dynamic table building */
    // unsigned ncode;             /* number of code length code lengths */
    // unsigned nlen;              /* number of length code lengths */
    // unsigned ndist;             /* number of distance code lengths */
    // unsigned have;              /* number of code lengths in lens[] */
    // code FAR *next;             /* next available space in codes[] */
    // unsigned short lens[320];   /* temporary storage for code lengths */
    // unsigned short work[288];   /* work area for code table building */
    // code codes[ENOUGH];         /* space for code tables */
    // int sane;                   /* if false, allow invalid distance too far */
    // int back;                   /* bits back of last unprocessed length/lit */
    // unsigned was;               /* initial length of match */
};

#ifdef __cplusplus
}
#endif

#endif /* _INFLATE_H__ */