#include <ultra64.h>
#include "game/data/data_000000.h"

void osViSetMode(OSViMode *modep)
{
	register u32 saveMask;

	saveMask = __osDisableInt();

	__osViNext->modep = modep;
	__osViNext->state = VI_STATE_01;
	__osViNext->control = __osViNext->modep->comRegs.ctrl;

	__osRestoreInt(saveMask);
}
