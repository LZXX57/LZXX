/******************************************************/
/*                       inflate.c                    */
/******************************************************/

#include "inflate.h"
#include "inffixed.h"
#include "hutil.h" 

// 准备 n bits数据，等待后续使用 不超过32bits 
// next 自动自加
#define NEEDBITS(n) \
    do{ \
        while(bits < (uInt)(n)){ \
            if(have == 0) { goto inf_leave;} \
            have--; \
            hold += (uLong)*next++ << bits; \
            bits += 8; \
        } \
    }while(0)

// 将解压临时值存储到结构体中等待数据输入或者...
#define RESTORE() \
    do{ \
        state->hold = hold; \
        state->bits = bits; \
        strm->next_out = put; \
        strm->avail_out = left; \
        strm->next_in = next; \
        strm->avail_in = have; \
    }while(0)

// 提取 hold 低 n bit值
#define BITS(n) ((uInt)(hold & ((1U << (uInt)(n) - 1))))  

// 移除 hold 中 n bits
#define DROPBITS(n) \
    do{ \
        hold >>= (uInt)(n); \
        bits -= (uInt)(n); \
    }while(0)

// 移除不满 1 Byte 的零散bit
#define BYTEBITS() \
    do{ \
        hold >>= bits & 7; \
        bits -= bits & 7; \
    }while(0)

#define RESETBITS() \
    do{ \
        hold = 0; \
        bits = 0; \
    }while(0)

LOCAL void setFixedTables(struct inflate_state *state){
    if(state){
        state->llcode = lenfix;
        state->lenbits = 9;
        state->dcode = distfix;
        state->distbits = 5;
        return H_True;
    }
    return H_False;
}

HEXTERN int inflateInit(h_streamptr strm){
    if (strm == H_NULL) return H_STREAM_ERROR;
    strm->msg = H_NULL;  // 错误信息置空
    if (strm->halloc == (alloc_func)0) {
        strm->halloc = hmalloc;
        strm->opaque = (voidptr)0;
    }
    if (strm->hfree == (free_func)0){
        strm->hfree = hmfree;
    }
    struct inflate_state *state = (struct inflate_state *)
            strm->halloc((strm)->opaque, 1, sizeof(struct inflate_state));
    if (state == H_NULL) return H_MEM_ERROR;
    memset((voidptr)state, 0, sizeof(struct inflate_state));
    strm->state = (struct internal_state *)state;
    state->strm = strm;
    state->window = H_NULL;
    state->hmode = HEAD;
    return H_OK;
}

HEXTERN int inflate(h_streamptr strm, int flush){
    /* 
        首先提需求，即需要多少个 bit，对 hold 中的有效 bit 进行补充。
        解析有效 bit 。
    */
    uInt ret = H_OK;
    uLong have = 0;
    uLong left = 0;
    uLong hold = 0;
    uInt bits = 0;
    uInt stored_len = 0;
    uInt8 *next = H_NULL;
    uInt8 *put = H_NULL;
    struct inflate_state *state = NULL;

    if(strm == H_NULL || strm->next_out == H_NULL || strm->state == H_NULL ||
      (strm->next_in == H_NULL && strm->avail_in != 0)){
          return H_STREAM_ERROR;
      }
    state = (struct inflate_state *)strm->state;
    if(state->hmode != HEAD){
        hold = state->hold;
        bits = state->bits;
    }
    next = strm->next_in;
    have = strm->avail_in;
    put = strm->next_out;
    left = strm->avail_out;
    while(1){
        switch(state->hmode){
            case HEAD:
                NEEDBITS(3);
                state->block_type = BITS(1);
                DROPBITS(1);
                switch(BITS(2)){
                    case 0:
                        state->hmode = STORED;
                        break;
                    case 1:
                        setFixedTables(state);
                        state->hmode = DATA_STREAM;
                        break;
                    case 2:
                        state->hmode = HLDCNUM;
                        break;
                    default :
                        strm->msg = (char *)"invalid block type!";
                        state->hmode = BAD;
                        break;
                }
                DROPBITS(2);
                break;
            case STORED:
                BYTEBITS(); // 移除无效 5 bits
                NEEDBITS(32);
                if((hold & 0xffff) != ((hold >> 16) ^ 0xffff)){
                    strm->msg = (char *)"invalid STORED block lengths!";
                    state->hmode = BAD;
                    break;
                }
                state->stored_state.block_size = (hold & 0xffff);
                RESETBITS();
                state->hmode = COPY;
            case COPY:
                stored_len = state->stored_state.block_size;
                if(stored_len > 0){
                    stored_len = (stored_len < have ? stored_len : have);
                    stored_len = (stored_len < left ? stored_len : left);
                    if(stored_len == 0) goto inf_leave;  // have 或者 left 为 0
                    hmemcpy(put, next, stored_len);
                    have -= stored_len;
                    next += stored_len;
                    left -= stored_len;
                    put += stored_len;
                    state->stored_state.block_size -= stored_len;
                }
                state->hmode = DONE;
                break;
            case HLDCNUM:   // 5 + 5 + 4 bits
                NEEDBITS(14);

            case RUNLTREE: // 游程码数解码 3bits * HCLEN（4bits）

            case RUNL:
            
            case LLTREE:

            case DTREE:

            case DATA_STREAM:

            case DONE:
                if(state->block_type){
                    ret = H_OK;
                }else{
                    ret = H_BLOCK_DONE;
                }

            case BAD:
                ret = H_ERRNO;

        }
    }
inf_leave:
    // 记录holded bits 输出返回状态
   RESTORE();


   return ret;
}

HEXTERN int inflateEnd(h_streamptr strm){
    if(strm == H_NULL) return H_STREAM_ERROR;
    struct inflate_state *state = (struct inflate_state *)strm->state;
    if(state == H_NULL) return H_STREAM_ERROR;
    if (state->window != H_NULL) strm->hfree((strm)->opaque, (state->window).window);
    strm->hfree((strm)->opaque, strm->state);
    strm->state = H_NULL;
    return H_OK;
}