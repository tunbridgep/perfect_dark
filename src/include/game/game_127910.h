#ifndef _IN_GAME_GAME_127910_H
#define _IN_GAME_GAME_127910_H
#include <ultra64.h>
#include "types.h"

void func0f127910(void);
void playersUnrefAll(void);
void playersAllocate(s32 count);
void playerAllocate(u32 index);
u32 func0f128834(void);
u32 propGetPlayerNum(struct prop *prop);
void func0f128a9c(u32 arg0, u32 arg2);
u32 func0f128ab8(void);
u32 func0f128ad4(void);
u32 func0f128ae4(void);
s32 func0f128af4(s32 arg0);
u32 func0f128cf0(void);
u32 func0f128d20(void);
u32 func0f128dbc(void);
u32 func0f128ec8(void);
void setCurrentPlayerNum(u32 playernum);
u32 calculatePlayerIndex(u32 playernum);

#endif
