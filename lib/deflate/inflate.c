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

// 获取一字节的数据，处理最后一个literal或distance时用
#define PULLBYTE() \
    do { \
        if (have == 0) {goto inf_leave;} \
        have--; \
        hold += (uLong)*next++ << bits; \
        bits += 8; \
    } while(0) \

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

// t 的空间必须在外部申请好
LOCAL int buildTree(h_treePtr t, uInt *lengths, uInt num)
{
    if(t == H_NULL || lengths == H_NULL || num == 0 || num > 288){
        return H_ERRNO;
    }
	unsigned short offs[16] = {0};
	unsigned int i, num_codes, available;

    // tree 初始化
    memset((void *)t->counts, 0, 16*sizeof(uInt));
	t->max_symbol = -1;

	/* 记录每一种Huffman码长度的个数 */
	for (i = 0; i < num; ++i) {
        if(lengths[i] > 15){            // huffman编码长度不应超过 15
            return H_DATA_ERROR;
        }
		if (lengths[i]) {
			t->max_symbol = i;          // 设置Huffman编码长度非 0 的最大 symbol
			t->counts[lengths[i]]++;
		}
	}

	/* 计算每一种Huffman码长度的起始下标 霍式Huffman树 */
	for (available = 1, num_codes = 0, i = 0; i < 16; ++i) {
		unsigned int used = t->counts[i];

		offs[i] = num_codes;
		num_codes += used;
	}

	/* 记录每一个symbol对应的数组下标 */
	for (i = 0; i < num; ++i) {
		if (lengths[i]) {
			t->symbols[offs[lengths[i]]++] = i;
		}
	}

	return H_OK;
}

LOCAL uInt updatewindow(h_streamptr strm, const uInt8 *start, uInt len)
{
    struct inflate_state *state = strm->state;
    if (state->window == H_NULL) {
        state->window = (uInt8*)strm->halloc(strm->opaque, 1 << state->wbits, sizeof(uInt8));
        if (state->window == H_NULL) {
            return 1;
        }
        state->wsize = 1 << state->wbits;
        state->whave = 0;
        state->wnext = 0;
    }

    /* len超过窗口大小，copy后state->wsize大小的数据 */
    if (len >= state->wsize) {
        hmemcpy(state->window, start + len -  state->wsize, state->wsize);
        state->wnext = 0;
        state->whave = state->wsize;
    } else {
        uInt dist = state->wsize - state->wnext;
        if (dist > len) {          //没有超出尾部
            hmemcpy(state->window + state->wnext, start, len);
            state->wnext += len;
            state->whave += len; 
            if (state->whave > state->wsize) {
                state->whave = state->wsize;
            }
        } else {                   //超出尾部 或 刚好到达尾部
            hmemcpy(state->window + state->wnext, start, dist);
            hmemcpy(state->window, start + dist, len - dist);
            state->wnext = len - dist;
            state->whave = state->wsize;
        }
    }

    return 0;
}

LOCAL int docodeOneBit(struct inflate_state state){
    
    return 19; // sym 解码未完成
}

HEXTERN int inflate(h_streamptr strm, int flush){
    /* 
        首先提需求，即需要多少个 bit，对 hold 中的有效 bit 进行补充。
        解析有效 bit 。
    */
    uInt rlength = 0;
    uInt ret = H_OK;
    uLong have = 0;
    uLong left = 0;
    uLong hold = 0;
    uInt bits = 0;
    uInt stored_len = 0;
    uInt8 *next = H_NULL;
    uInt8 *put = H_NULL;
    struct inflate_state *state = H_NULL;
    static const uInt run_order[19] = {
        16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15 };
    h_code here;
    uInt copylen = 0;
    uInt8* copyfrom = H_NULL;
    uLong out = 0;
	
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
    out = strm->avail_out;

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
                    break;
                }
                state->hmode = DONE;
                break;
            case HLDCNUM:   // 5 + 5 + 4 bits
                NEEDBITS(14);
                state->compress_state.lltree_decode_len = BITS(5) + 257;
                DROPBITS(5);
                state->compress_state.dtree_decode_len = BITS(5) + 1;
                DROPBITS(5);
                state->compress_state.runltree_decode_len = BITS(4) + 4;
                DROPBITS(4);
                if(state->compress_state.lltree_decode_len > 286 || 
                   state->compress_state.dtree_decode_len > 30 ||
                   state->compress_state.runltree_decode_len > 19){
                       strm->msg = (char *)"HLDCNUM too many symbols!";
                       state->hmode = BAD;
                }
                state->compress_state.code_count = 0;
                state->hmode = RUNLTREE;
            case RUNLTREE: // 游程码数解码 3bits * HCLEN（4bits）
                while(state->compress_state.code_count < state->compress_state.runltree_decode_len){
                    NEEDBITS(3);
                    state->compress_state.code_lens[run_order[state->compress_state.code_count++]] = (uInt)BITS(3);
                    DROPBITS(3);
                }
                while(state->compress_state.code_count < 19){
                    state->compress_state.code_lens[run_order[state->compress_state.code_count++]] = 0;
                }
                ret = buildTree(&(state->compress_state.runltree), state->compress_state.code_lens, 19);
                if(-1 == state->compress_state.runltree.max_symbol || ret != H_OK){
                    state->hmode = BAD;
                    break;
                }
                state->compress_state.code_count = 0;
                state->compress_state.sym = 19;
                state->compress_state.dlen = 0;
                state->compress_state.dbase = 0;
                state->compress_state.doffs = 0;
                state->hmode = RUNL;
            case RUNL:
                while(state->compress_state.code_count < (state->compress_state.lltree_decode_len + state->compress_state.dtree_decode_len)){
                    switch(state->compress_state.sym){
                        case 16:
                            if(state->compress_state.code_count == 0){ } // error
                            state->compress_state.sym = state->compress_state.code_lens[state->compress_state.code_count - 1];
                            NEEDBITS(2);
                            rlength = BITS(2) + 3;
                            DROPBITS(2);
                            break;
                        case 17:
                            state->compress_state.sym = 0;
                            NEEDBITS(3);
                            rlength = BITS(3) + 3;
                            DROPBITS(3);
                            break;
                        case 18:
                            state->compress_state.sym = 0;
                            NEEDBITS(7);
                            rlength = BITS(7) + 11;
                            DROPBITS(7);
                            break;
                        case 19:   // 未解压出sym
                            rlength = 0;
                            break;
                        default :
                            rlength = 1;
                            break;
                    }
                    if(rlength > (state->compress_state.lltree_decode_len + state->compress_state.dtree_decode_len - state->compress_state.code_count)){
                        state->hmode = BAD;
                        break;
                    }
                    while(rlength){
                        state->compress_state.code_lens[state->compress_state.code_count++] = state->compress_state.sym;
                        rlength--;
                    }
                    if(state->compress_state.code_count == (state->compress_state.lltree_decode_len + state->compress_state.dtree_decode_len)) break;
                    // if(!rlength) 
                        state->compress_state.sym = 19;
                    NEEDBITS(15); // 解压出至少一个字符
                    for(;state->compress_state.dlen < bits; state->compress_state.dlen++){
                        state->compress_state.doffs = state->compress_state.doffs << 1 + BITS(1);
                        if(state->compress_state.doffs < state->compress_state.runltree.counts[state->compress_state.dlen]){
                            state->compress_state.sym = state->compress_state.runltree.symbols[state->compress_state.dbase + state->compress_state.doffs];
                            state->compress_state.dbase = 0;
                            state->compress_state.doffs = 0;
                            break;
                        }
                        state->compress_state.dbase += state->compress_state.runltree.counts[state->compress_state.dlen];
                        state->compress_state.doffs -= state->compress_state.runltree.counts[state->compress_state.dlen];
                    }
                    DROPBITS(state->compress_state.dlen);
                    state->compress_state.dlen = 0;
                }
                state->hmode = LLTREE;
            
            case LLTREE:
                ret = buildTree(&(state->compress_state.lltree), state->compress_state.code_lens, state->compress_state.lltree_decode_len);
                if(ret != H_OK){
                    state->hmode = BAD;
                    break;
                }
                state->hmode = DTREE;

            case DTREE:
                ret = buildTree(&(state->compress_state.dtree), state->compress_state.code_lens + state->compress_state.lltree_decode_len, state->compress_state.dtree_decode_len);
                if(ret != H_OK){
                    state->hmode = BAD;
                    break;
                }
                state->hmode = D_DATA_STREAM;
            case D_DATA_STREAM:  // 动态树的解码流与静态树不一致，分开处理
                if(have >= 2) NEEDBITS(15);  // 如果数据多则多读取一些
                else NEEDBITS(1);    // 至少提供 1 bit
                for(;state->compress_state.dlen < bits; state->compress_state.dlen++){
                    state->compress_state.doffs = state->compress_state.doffs << 1 + BITS(1);
                    if(state->compress_state.doffs < state->compress_state.runltree.counts[state->compress_state.dlen]){
                        state->compress_state.sym = state->compress_state.runltree.symbols[state->compress_state.dbase + state->compress_state.doffs];
                        state->compress_state.dbase = 0;
                        state->compress_state.doffs = 0;
                        break;
                    }
                    state->compress_state.dbase += state->compress_state.runltree.counts[state->compress_state.dlen];
                    state->compress_state.doffs -= state->compress_state.runltree.counts[state->compress_state.dlen];
                }
                DROPBITS(state->compress_state.dlen);
                state->compress_state.dlen = 0;
                if(state->compress_state.sym < 256){
                    if (left == 0) {
                    goto inf_leave;
                    }
                    *put++ = (uInt8)state->compress_state.sym;
                    --left;
                    break;
                }else{
                    if(state->compress_state.sym == 256){
                        state->hmode = DONE;
                        break;
                    }else{
                        state->hmode = D_LENEXT;
                    }
                    
                }
            case D_LENEXT:
                state->compress_state.sym -= 257;
                NEEDBITS(length_bits[state->compress_state.sym]); //跳出循环。。。。。
                state->compress_state.length = BITS(length_bits[state->compress_state.sym]) + length_base[state->compress_state.sym];
                DROPBITS(length_bits[state->compress_state.sym]);
                state->hmode = D_DIST;
            case D_DIST:
                if(have >= 2) NEEDBITS(15);  // 如果数据多则多读取一些
                else NEEDBITS(1);    // 至少提供 1 bit
                for(;state->compress_state.dlen < bits; state->compress_state.dlen++){
                    state->compress_state.doffs = state->compress_state.doffs << 1 + BITS(1);
                    if(state->compress_state.doffs < state->compress_state.runltree.counts[state->compress_state.dlen]){
                        state->compress_state.sym = state->compress_state.runltree.symbols[state->compress_state.dbase + state->compress_state.doffs];
                        state->compress_state.dbase = 0;
                        state->compress_state.doffs = 0;
                        state->hmode = D_DEXT;
                        break;
                    }
                    state->compress_state.dbase += state->compress_state.runltree.counts[state->compress_state.dlen];
                    state->compress_state.doffs -= state->compress_state.runltree.counts[state->compress_state.dlen];
                }
                DROPBITS(state->compress_state.dlen);
                state->compress_state.dlen = 0;
            case D_DEXT:
                state->compress_state.dist = BITS(dist_bits[state->compress_state.sym]) + dist_base[state->compress_state.sym];
                state->hmode = MATCH;
                break;
            case DATA_STREAM:
                while (1) {
                    here = state->llcode[BITS(state->lenbits)];
                    if ((uInt)here.bits <= bits) {
                        break;
                    }
                    PULLBYTE();
                }
                DROPBITS(here.bits);
                state->length = here.val;
                if (here.op & 0x10) {
                    state->extrabits = here.op & 0x0F;
                    if (state->extrabits) {
                        state->hmode = LENEXT;
                    } else {
                        state->hmode = DIST;
                    }
                    break;              
                } else if (here.op == 0x60) {
                    state->hmode = HEAD;
                    break;
                } else if (here.op != 0) {
                    strm->msg = "invalid literal/length code";
                    state->hmode = BAD;
                    break;
                }
                state->hmode = LIT;

            case LIT:
                if (left == 0) {
                    goto inf_leave;
                }
                *put++ = (uInt8)state->length;
                --left;
                state->hmode = DATA_STREAM;
                break;

            case LENEXT:
                NEEDBITS(state->extrabits);
                state->length += BITS(state->extrabits);
                DROPBITS(state->extrabits);
                state->hmode = DIST;

            case DIST:
                while (1) {
                    here = state->dcode[BITS(state->distbits)];
                    if ((uInt)here.bits <= bits) {
                        break;
                    }
                    PULLBYTE();
                }
                DROPBITS(here.bits);
                state->dist = here.val;
                state->extrabits = here.op & 0x0F;
                if (!state->extrabits) {
                    state->hmode = MATCH;
                    break;
                }
                state->hmode = DEXT;

            case DEXT:
                NEEDBITS(state->extrabits);
                state->dist += BITS(state->extrabits);
                DROPBITS(state->extrabits);
                state->hmode = MATCH;

            case MATCH:
                if (left == 0) {
                    goto inf_leave;
                }
                copylen = strm->avail_out - left;
                if (state->dist > copylen) {           // copy起始点超出outbuffer起始点,需要从滑动窗口中取数据
                    copylen = state->dist - copylen;
                    if (copylen > state->whave) {      // copy起始点超出滑动窗口，直接报错
                        strm->msg = (char*)"invalid distance too far back";
                        state->hmode = BAD;
                        break;
                    }
                    if (copylen > state->wnext) {      // copy起始点超过当前循环的起始位置,需要从上一次循环的窗口中获取数据
                        copylen -= state->wnext;
                        copyfrom = state->window + (state->wsize - copylen);
                    } else {                           // copy起始点在当前循环内
                        copyfrom = state->window + (state->wnext - copylen);
                    }
                    if (copylen > state->length) {     // 没有横跨两次滑动窗口 或 没有横跨滑动窗口和outbuffer
                        copylen = state->length;
                    }
                } else {                               // copy起始点在outbuffer上
                    copyfrom = put - state->dist;
                    copylen = state->length;
                }
                if (copylen > left) {                  // 超出outbuffer剩余大小，先只copy剩余部分大小的数据
                    copylen = left;
                }
                do {
                    *put++ = *copyfrom++;
                } while (--copylen);

                left -= copylen;                      // 减去已经copy的长度，dist不用变，因为更新滑动窗口时，窗口前移copylen，刚好抵消刚刚消耗的距离。
                state->length -= copylen;

                if (state->length == 0) {             // 全部copy完成，才去解析下一个三元组
                    if((state->llcode == lenfix) || (state->dcode == distfix)){
                        state->hmode = DATA_STREAM;
                    }else{
                        state->hmode = D_DATA_STREAM;
                    }
                    
                }
                break;

            case DONE:
                if(state->block_type){
                    ret = H_STREAM_END;
                    goto inf_leave;
                }else{
                    ret = H_OK;
                    goto inf_leave;
                }

            case BAD:
                ret = H_ERRNO;
                goto inf_leave;

        }
    }
inf_leave:
    // 记录holded bits 输出返回状态
   RESTORE();
   if (out != strm->avail_out) {
       if (updatewindow(strm, out, out - strm->avail_out)) {
           return H_MEM_ERROR;
       }
   }

   return ret;
}

HEXTERN int inflateEnd(h_streamptr strm){
    if(strm == H_NULL) return H_STREAM_ERROR;
    struct inflate_state *state = (struct inflate_state *)strm->state;
    if(state == H_NULL) return H_STREAM_ERROR;
    if (state->window_buf.window != H_NULL) strm->hfree((strm)->opaque, state->window_buf.window);
    strm->hfree((strm)->opaque, strm->state);
    strm->state = H_NULL;
    return H_OK;
}
