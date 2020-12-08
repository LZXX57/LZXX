/**************************************************************
 * 
 * DEFLATE common.h
 * 
***************************************************************/
#ifndef _DEFLATE_COMMON_H__
#define _DEFLATE_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif

#ifndef ZEXTERN
#  define ZEXTERN extern
#endif

typedef void* voidpf;         /* void * void_ptr */
typedef unsigned char  Byte;  /* 8 bits */
typedef unsigned int   uInt;  /* 16 bits or more */
typedef unsigned long  uLong; /* 32 bits or more */

typedef voidpf (*alloc_func) (voidpf opaque, uInt items, uInt size);
typedef void   (*free_func)  (voidpf opaque, voidpf address);
struct internal_state;

typedef struct z_stream_s {
    const Byte *next_in;     /* next input byte */
    uInt     avail_in;       /* number of bytes available at next_in */
    uLong    total_in;       /* total number of input bytes read so far */

    Byte    *next_out;       /* next output byte will go here */
    uInt     avail_out;      /* remaining free space at next_out */
    uLong    total_out;      /* total number of bytes output so far */

    const char *msg;         /* last error message, NULL if no error */
    struct internal_state *state; /* not visible by applications */

    alloc_func zalloc;  /* used to allocate the internal state */
    free_func  zfree;   /* used to free the internal state */
    voidpf     opaque;  /* private data object passed to zalloc and zfree */

    int     data_type;  /* best guess about the data type: binary or text
                           for deflate, or the decoding state for inflate */
    uLong   adler;      /* Adler-32 or CRC-32 value of the uncompressed data */
    uLong   reserved;   /* reserved for future use */
} z_stream;

typedef z_stream* z_streamp;

#ifdef __cplusplus
}
#endif

#endif /* _DEFLATE_COMMON_H__ */