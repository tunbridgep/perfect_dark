//
// G5 Building (MP)
//

#include "stagesetup.h"

s32 intro[];
u8 props[];
struct path paths[];
struct ailist ailists[];

struct stagesetup setup = {
	NULL,
	NULL,
	NULL,
	intro,
	props,
	paths,
	ailists,
	NULL,
};

u8 props[] = {
	endprops
};

s32 intro[] = {
	intro_weapon(WEAPON_FALCON2, -1)
	endintro
};

struct path paths[] = {
	{ NULL, 0, 0 },
};

struct ailist ailists[] = {
	{ NULL, 0 },
};

