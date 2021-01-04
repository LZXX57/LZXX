

#include "../lib/deflate/inflate.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CHECK_ERR(err, msg) { \
    if (err != H_OK) { \
        fprintf(stderr, "%s error: %d\n", msg, err); \
        exit(1); \
    } \
}

static alloc_func zalloc = (alloc_func)0;
static free_func zfree = (free_func)0;

/* ===========================================================================
 * Test inflate() with small buffers
 */
void test_inflate(compr, comprLen, uncompr, uncomprLen)
    Byte *compr, *uncompr;
    uLong comprLen, uncomprLen;
{
    int err;
    h_stream d_stream; /* decompression stream */

    //strcpy((char*)uncompr, "garbage");

    d_stream.halloc = zalloc;
    d_stream.hfree = zfree;
    d_stream.opaque = (void *)0;

    d_stream.next_in  = compr;
    d_stream.avail_in = 0;
    d_stream.next_out = uncompr;

    d_stream.total_out = 0;
    d_stream.total_in = 0;

    err = inflateInit(&d_stream);
    CHECK_ERR(err, "inflateInit");

    while (d_stream.total_out < uncomprLen && d_stream.total_in < 1179) {
        d_stream.avail_in = comprLen; /* force small buffers */
        d_stream.avail_out = 2000; /* force small buffers */
        err = inflate(&d_stream, 0);
        if (err == H_STREAM_END) break;
        CHECK_ERR(err, "inflate");
    }

    err = inflateEnd(&d_stream);
    CHECK_ERR(err, "inflateEnd");

    // if (strcmp((char*)uncompr, hello)) {
    //     fprintf(stderr, "bad inflate\n");
    //     exit(1);
    // } else {
    //     printf("inflate(): %s\n", (char *)uncompr);
    // }
}

/* ===========================================================================
 * Test inflate() with large buffers
//  */
// void test_large_inflate(compr, comprLen, uncompr, uncomprLen)
//     Byte *compr, *uncompr;
//     uLong comprLen, uncomprLen;
// {
//     int err;
//     h_stream d_stream; /* decompression stream */

//     strcpy((char*)uncompr, "garbage");

//     d_stream.halloc = zalloc;
//     d_stream.hfree = zfree;
//     d_stream.opaque = (void *)0;

//     d_stream.next_in  = compr;
//     d_stream.avail_in = (uInt)comprLen;

//     err = inflateInit(&d_stream);
//     CHECK_ERR(err, "inflateInit");

//     for (;;) {
//         d_stream.next_out = uncompr;            /* discard the output */
//         d_stream.avail_out = (uInt)uncomprLen;
//         err = inflate(&d_stream, 0);
//         if (err == H_STREAM_END) break;
//         CHECK_ERR(err, "large inflate");
//     }

//     err = inflateEnd(&d_stream);
//     CHECK_ERR(err, "inflateEnd");

//     if (d_stream.total_out != 2*uncomprLen + comprLen/2) {
//         fprintf(stderr, "bad large inflate: %ld\n", d_stream.total_out);
//         exit(1);
//     } else {
//         printf("large_inflate(): OK\n");
//     }
// }


int main(argc, argv)
    int argc;
    char *argv[];
{
    Byte *compr = NULL, *uncompr = NULL;
    uLong comprLen = 2000*sizeof(int); /* don't overflow on MSDOS */
    uLong uncomprLen = comprLen;
    system("pwd");
    compr = malloc(comprLen);
    if(compr==NULL) return -1;
    uncompr = malloc(uncomprLen);
    if(uncompr==NULL) return -2;
    FILE * fd = fopen("f", "r");
    if(fd==NULL) return -3;
    int n = fread(compr,1,comprLen,fd);

    test_inflate(compr, n, uncompr, uncomprLen);

    //test_large_inflate(compr, comprLen, uncompr, uncomprLen);
    FILE * fdw = fopen("df", "w");
    fwrite(uncompr, 1174, 1, fdw);
    fclose(fdw);
    fclose(fd);

    if(compr)
        free(compr);
    
    if(uncompr)
        free(uncompr);

    return 0;
}
