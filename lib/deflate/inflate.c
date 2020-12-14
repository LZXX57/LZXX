/******************************************************/
/*                       inflate.c                    */
/******************************************************/

#include "inflate.h"
#include "hutil.h"

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
    state->hstate = HEAD;
    return H_OK;
}

HEXTERN int inflate(h_streamptr strm, int flush){
    /* 
        首先提需求，即需要多少个 bit，对 hold 中的有效 bit 进行补充。
        解析有效 bit 。
     */
}

HEXTERN int inflateEnd(h_streamptr strm){
    if(strm == H_NULL) return H_STREAM_ERROR;
    struct inflate_state *state = (struct inflate_state *)strm->state;
    if(state == H_NULL) return H_STREAM_ERROR;
    if (state->window != H_NULL) strm->hfree((strm)->opaque, state->window.window);
    strm->hfree((strm)->opaque, strm->state);
    strm->state = H_NULL;
    return H_OK;
}