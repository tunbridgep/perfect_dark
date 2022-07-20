#include <ultra64.h>
#include "constants.h"
#include "game/quaternion.h"
#include "game/game_0b2150.h"
#include "game/camera.h"
#include "game/sky.h"
#include "game/game_152fa0.h"
#include "game/env.h"
#include "game/pad.h"
#include "bss.h"
#include "lib/vi.h"
#include "lib/mtx.h"
#include "lib/sched.h"
#include "data.h"
#include "types.h"

u32 g_SkyStageNum;
s32 var800a33a4;
Mtxf var800a33a8;
struct coord g_SunPositions[3]; // relative to centre screen, with a huge scale
u32 var800a340c;
f32 g_SunScreenXPositions[4];
f32 g_SunScreenYPositions[4];

f32 g_SkyCloudOffset = 0;
f32 g_SkyWindSpeed = 1;
f32 g_SunAlphaFracs[3] = {0};
s32 g_SunFlareTimers240[3] = {0};
u32 var8007dba0 = 0x00000010;
u32 var8007dba4 = 0x00000020;
u32 var8007dba8 = 0x0000000c;
u32 var8007dbac = 0x00000020;
u32 var8007dbb0 = 0x00000018;
u32 var8007dbb4 = 0x00000040;
u32 var8007dbb8 = 0x0000003c;
u32 var8007dbbc = 0x00000050;
u32 var8007dbc0 = 0x000000e1;
u32 var8007dbc4 = 0x00000113;
u32 var8007dbc8 = 0x000001d6;
u32 var8007dbcc = 0x0000023a;
u32 var8007dbd0 = 0xff99ffff;
u32 var8007dbd4 = 0x9999ffff;
u32 var8007dbd8 = 0x99ffffff;
u32 var8007dbdc = 0x99ff99ff;
u32 var8007dbe0 = 0xffff99ff;
u32 var8007dbe4 = 0xff9999ff;
struct coord g_TeleportToPos = {0, 0, 0};
struct coord g_TeleportToUp = {0, 0, 1};
struct coord g_TeleportToLook = {0, 1, 0};

void sky0f11f000(f32 left, f32 top, struct coord *arg2)
{
	Mtxf *mtx = camGetProjectionMtxF();
	f32 pos[2];

	pos[0] = left + camGetScreenLeft();
	pos[1] = top + camGetScreenTop() + envGetCurrent()->unk40;

	cam0f0b4c3c(pos, arg2, 100);
	mtx4RotateVecInPlace(mtx, arg2);
}

bool sky0f11f07c(struct coord *arg0, struct coord *arg1, f32 *arg2)
{
	struct coord *campos = &g_Vars.currentplayer->cam_pos;
	f32 f12 = 2.0f * arg0->y / sqrtf(arg0->f[0] * arg0->f[0] + arg0->f[2] * arg0->f[2] + 0.0001f);
	f32 sp2c;
	f32 f12_2;
	f32 sp24;
	u32 stack[2];

	if (f12 > 1.0f) {
		f12 = 1.0f;
	}

	*arg2 = 1.0f - f12;

	if (arg0->y == 0.0f) {
		sp24 = 0.01f;
	} else {
		sp24 = arg0->y;
	}

	if (sp24 > 0.0f) {
		sp2c = (envGetCurrent()->clouds_scale - campos->y) / sp24;
		f12_2 = sqrtf(arg0->f[0] * arg0->f[0] + arg0->f[2] * arg0->f[2]) * sp2c;

		if (f12_2 > 300000) {
			sp2c *= 300000 / f12_2;
		}

		arg1->x = campos->x + sp2c * arg0->f[0];
		arg1->y = campos->y + sp2c * sp24;
		arg1->z = campos->z + sp2c * arg0->f[2];

		return true;
	}

	return false;
}

bool sky0f11f1fc(struct coord *arg0, struct coord *arg1, f32 *arg2)
{
	struct coord *campos = &g_Vars.currentplayer->cam_pos;
	f32 f12 = -2.0f * arg0->y / sqrtf(arg0->f[0] * arg0->f[0] + arg0->f[2] * arg0->f[2] + 0.0001f);
	f32 sp2c;
	f32 f12_2;
	f32 sp24;
	u32 stack[2];

	if (f12 > 1.0f) {
		f12 = 1.0f;
	}

	*arg2 = 1.0f - f12;

	if (arg0->y == 0.0f) {
		sp24 = -0.01f;
	} else {
		sp24 = arg0->y;
	}

	if (sp24 < 0.0f) {
		sp2c = (envGetCurrent()->water_scale - campos->y) / sp24;
		f12_2 = sqrtf(arg0->f[0] * arg0->f[0] + arg0->f[2] * arg0->f[2]) * sp2c;

		if (f12_2 > 300000) {
			sp2c *= 300000 / f12_2;
		}

		arg1->x = campos->x + sp2c * arg0->f[0];
		arg1->y = campos->y + sp2c * sp24;
		arg1->z = campos->z + sp2c * arg0->f[2];

		return true;
	}

	return false;
}

/**
 * Scale base based on the height percentage between base and ref...
 * except the new y is zero.
 */
void sky0f11f384(struct coord *base, struct coord *ref, struct coord *out)
{
	f32 mult = base->y / (base->y - ref->y);

	out->x = (ref->x - base->x) * mult + base->x;
	out->y = 0;
	out->z = (ref->z - base->z) * mult + base->z;
}

f32 skyClamp(f32 value, f32 min, f32 max)
{
	if (value < min) {
		return min;
	}

	if (value > max) {
		return max;
	}

	return value;
}

f32 skyRound(f32 value)
{
	return (s32)(value + 0.5f);
}

GLOBAL_ASM(
glabel sky0f11f438
.late_rodata
glabel var7f1b4ff8
.word 0x3b808081
.text
/*  f11f438:	27bdffe8 */ 	addiu	$sp,$sp,-24
/*  f11f43c:	afbf0014 */ 	sw	$ra,0x14($sp)
/*  f11f440:	afa5001c */ 	sw	$a1,0x1c($sp)
/*  f11f444:	0fc595f3 */ 	jal	envGetCurrent
/*  f11f448:	afa40018 */ 	sw	$a0,0x18($sp)
/*  f11f44c:	904e0008 */ 	lbu	$t6,0x8($v0)
/*  f11f450:	3c013f80 */ 	lui	$at,0x3f80
/*  f11f454:	44818000 */ 	mtc1	$at,$f16
/*  f11f458:	448e2000 */ 	mtc1	$t6,$f4
/*  f11f45c:	3c017f1b */ 	lui	$at,%hi(var7f1b4ff8)
/*  f11f460:	c4324ff8 */ 	lwc1	$f18,%lo(var7f1b4ff8)($at)
/*  f11f464:	8fa40018 */ 	lw	$a0,0x18($sp)
/*  f11f468:	05c10005 */ 	bgez	$t6,.L0f11f480
/*  f11f46c:	468020a0 */ 	cvt.s.w	$f2,$f4
/*  f11f470:	3c014f80 */ 	lui	$at,0x4f80
/*  f11f474:	44813000 */ 	mtc1	$at,$f6
/*  f11f478:	00000000 */ 	nop
/*  f11f47c:	46061080 */ 	add.s	$f2,$f2,$f6
.L0f11f480:
/*  f11f480:	904f0009 */ 	lbu	$t7,0x9($v0)
/*  f11f484:	3c014f80 */ 	lui	$at,0x4f80
/*  f11f488:	3c0d800a */ 	lui	$t5,%hi(var800a33a4)
/*  f11f48c:	448f4000 */ 	mtc1	$t7,$f8
/*  f11f490:	05e10004 */ 	bgez	$t7,.L0f11f4a4
/*  f11f494:	46804320 */ 	cvt.s.w	$f12,$f8
/*  f11f498:	44815000 */ 	mtc1	$at,$f10
/*  f11f49c:	00000000 */ 	nop
/*  f11f4a0:	460a6300 */ 	add.s	$f12,$f12,$f10
.L0f11f4a4:
/*  f11f4a4:	9058000a */ 	lbu	$t8,0xa($v0)
/*  f11f4a8:	3c014f80 */ 	lui	$at,0x4f80
/*  f11f4ac:	c7a8001c */ 	lwc1	$f8,0x1c($sp)
/*  f11f4b0:	44982000 */ 	mtc1	$t8,$f4
/*  f11f4b4:	240f00ff */ 	addiu	$t7,$zero,0xff
/*  f11f4b8:	468023a0 */ 	cvt.s.w	$f14,$f4
/*  f11f4bc:	46121102 */ 	mul.s	$f4,$f2,$f18
/*  f11f4c0:	07030005 */ 	bgezl	$t8,.L0f11f4d8
/*  f11f4c4:	46048181 */ 	sub.s	$f6,$f16,$f4
/*  f11f4c8:	44813000 */ 	mtc1	$at,$f6
/*  f11f4cc:	00000000 */ 	nop
/*  f11f4d0:	46067380 */ 	add.s	$f14,$f14,$f6
/*  f11f4d4:	46048181 */ 	sub.s	$f6,$f16,$f4
.L0f11f4d8:
/*  f11f4d8:	c44a001c */ 	lwc1	$f10,0x1c($v0)
/*  f11f4dc:	24080001 */ 	addiu	$t0,$zero,0x1
/*  f11f4e0:	46088001 */ 	sub.s	$f0,$f16,$f8
/*  f11f4e4:	46065202 */ 	mul.s	$f8,$f10,$f6
/*  f11f4e8:	3c014f00 */ 	lui	$at,0x4f00
/*  f11f4ec:	46004102 */ 	mul.s	$f4,$f8,$f0
/*  f11f4f0:	46022280 */ 	add.s	$f10,$f4,$f2
/*  f11f4f4:	4459f800 */ 	cfc1	$t9,$31
/*  f11f4f8:	44c8f800 */ 	ctc1	$t0,$31
/*  f11f4fc:	00000000 */ 	nop
/*  f11f500:	460051a4 */ 	cvt.w.s	$f6,$f10
/*  f11f504:	4448f800 */ 	cfc1	$t0,$31
/*  f11f508:	00000000 */ 	nop
/*  f11f50c:	31080078 */ 	andi	$t0,$t0,0x78
/*  f11f510:	51000013 */ 	beqzl	$t0,.L0f11f560
/*  f11f514:	44083000 */ 	mfc1	$t0,$f6
/*  f11f518:	44813000 */ 	mtc1	$at,$f6
/*  f11f51c:	24080001 */ 	addiu	$t0,$zero,0x1
/*  f11f520:	46065181 */ 	sub.s	$f6,$f10,$f6
/*  f11f524:	44c8f800 */ 	ctc1	$t0,$31
/*  f11f528:	00000000 */ 	nop
/*  f11f52c:	460031a4 */ 	cvt.w.s	$f6,$f6
/*  f11f530:	4448f800 */ 	cfc1	$t0,$31
/*  f11f534:	00000000 */ 	nop
/*  f11f538:	31080078 */ 	andi	$t0,$t0,0x78
/*  f11f53c:	15000005 */ 	bnez	$t0,.L0f11f554
/*  f11f540:	00000000 */ 	nop
/*  f11f544:	44083000 */ 	mfc1	$t0,$f6
/*  f11f548:	3c018000 */ 	lui	$at,0x8000
/*  f11f54c:	10000007 */ 	b	.L0f11f56c
/*  f11f550:	01014025 */ 	or	$t0,$t0,$at
.L0f11f554:
/*  f11f554:	10000005 */ 	b	.L0f11f56c
/*  f11f558:	2408ffff */ 	addiu	$t0,$zero,-1
/*  f11f55c:	44083000 */ 	mfc1	$t0,$f6
.L0f11f560:
/*  f11f560:	00000000 */ 	nop
/*  f11f564:	0500fffb */ 	bltz	$t0,.L0f11f554
/*  f11f568:	00000000 */ 	nop
.L0f11f56c:
/*  f11f56c:	44d9f800 */ 	ctc1	$t9,$31
/*  f11f570:	a0880014 */ 	sb	$t0,0x14($a0)
/*  f11f574:	c4480020 */ 	lwc1	$f8,0x20($v0)
/*  f11f578:	46126102 */ 	mul.s	$f4,$f12,$f18
/*  f11f57c:	240a0001 */ 	addiu	$t2,$zero,0x1
/*  f11f580:	3c014f00 */ 	lui	$at,0x4f00
/*  f11f584:	46048281 */ 	sub.s	$f10,$f16,$f4
/*  f11f588:	460a4182 */ 	mul.s	$f6,$f8,$f10
/*  f11f58c:	00000000 */ 	nop
/*  f11f590:	46003102 */ 	mul.s	$f4,$f6,$f0
/*  f11f594:	460c2200 */ 	add.s	$f8,$f4,$f12
/*  f11f598:	4449f800 */ 	cfc1	$t1,$31
/*  f11f59c:	44caf800 */ 	ctc1	$t2,$31
/*  f11f5a0:	00000000 */ 	nop
/*  f11f5a4:	460042a4 */ 	cvt.w.s	$f10,$f8
/*  f11f5a8:	444af800 */ 	cfc1	$t2,$31
/*  f11f5ac:	00000000 */ 	nop
/*  f11f5b0:	314a0078 */ 	andi	$t2,$t2,0x78
/*  f11f5b4:	11400012 */ 	beqz	$t2,.L0f11f600
/*  f11f5b8:	00000000 */ 	nop
/*  f11f5bc:	44815000 */ 	mtc1	$at,$f10
/*  f11f5c0:	240a0001 */ 	addiu	$t2,$zero,0x1
/*  f11f5c4:	460a4281 */ 	sub.s	$f10,$f8,$f10
/*  f11f5c8:	44caf800 */ 	ctc1	$t2,$31
/*  f11f5cc:	00000000 */ 	nop
/*  f11f5d0:	460052a4 */ 	cvt.w.s	$f10,$f10
/*  f11f5d4:	444af800 */ 	cfc1	$t2,$31
/*  f11f5d8:	00000000 */ 	nop
/*  f11f5dc:	314a0078 */ 	andi	$t2,$t2,0x78
/*  f11f5e0:	15400005 */ 	bnez	$t2,.L0f11f5f8
/*  f11f5e4:	00000000 */ 	nop
/*  f11f5e8:	440a5000 */ 	mfc1	$t2,$f10
/*  f11f5ec:	3c018000 */ 	lui	$at,0x8000
/*  f11f5f0:	10000007 */ 	b	.L0f11f610
/*  f11f5f4:	01415025 */ 	or	$t2,$t2,$at
.L0f11f5f8:
/*  f11f5f8:	10000005 */ 	b	.L0f11f610
/*  f11f5fc:	240affff */ 	addiu	$t2,$zero,-1
.L0f11f600:
/*  f11f600:	440a5000 */ 	mfc1	$t2,$f10
/*  f11f604:	00000000 */ 	nop
/*  f11f608:	0540fffb */ 	bltz	$t2,.L0f11f5f8
/*  f11f60c:	00000000 */ 	nop
.L0f11f610:
/*  f11f610:	44c9f800 */ 	ctc1	$t1,$31
/*  f11f614:	a08a0015 */ 	sb	$t2,0x15($a0)
/*  f11f618:	c4460024 */ 	lwc1	$f6,0x24($v0)
/*  f11f61c:	46127102 */ 	mul.s	$f4,$f14,$f18
/*  f11f620:	240c0001 */ 	addiu	$t4,$zero,0x1
/*  f11f624:	3c014f00 */ 	lui	$at,0x4f00
/*  f11f628:	240200ff */ 	addiu	$v0,$zero,0xff
/*  f11f62c:	46048201 */ 	sub.s	$f8,$f16,$f4
/*  f11f630:	46083282 */ 	mul.s	$f10,$f6,$f8
/*  f11f634:	00000000 */ 	nop
/*  f11f638:	46005102 */ 	mul.s	$f4,$f10,$f0
/*  f11f63c:	460e2180 */ 	add.s	$f6,$f4,$f14
/*  f11f640:	444bf800 */ 	cfc1	$t3,$31
/*  f11f644:	44ccf800 */ 	ctc1	$t4,$31
/*  f11f648:	00000000 */ 	nop
/*  f11f64c:	46003224 */ 	cvt.w.s	$f8,$f6
/*  f11f650:	444cf800 */ 	cfc1	$t4,$31
/*  f11f654:	00000000 */ 	nop
/*  f11f658:	318c0078 */ 	andi	$t4,$t4,0x78
/*  f11f65c:	11800012 */ 	beqz	$t4,.L0f11f6a8
/*  f11f660:	00000000 */ 	nop
/*  f11f664:	44814000 */ 	mtc1	$at,$f8
/*  f11f668:	240c0001 */ 	addiu	$t4,$zero,0x1
/*  f11f66c:	46083201 */ 	sub.s	$f8,$f6,$f8
/*  f11f670:	44ccf800 */ 	ctc1	$t4,$31
/*  f11f674:	00000000 */ 	nop
/*  f11f678:	46004224 */ 	cvt.w.s	$f8,$f8
/*  f11f67c:	444cf800 */ 	cfc1	$t4,$31
/*  f11f680:	00000000 */ 	nop
/*  f11f684:	318c0078 */ 	andi	$t4,$t4,0x78
/*  f11f688:	15800005 */ 	bnez	$t4,.L0f11f6a0
/*  f11f68c:	00000000 */ 	nop
/*  f11f690:	440c4000 */ 	mfc1	$t4,$f8
/*  f11f694:	3c018000 */ 	lui	$at,0x8000
/*  f11f698:	10000007 */ 	b	.L0f11f6b8
/*  f11f69c:	01816025 */ 	or	$t4,$t4,$at
.L0f11f6a0:
/*  f11f6a0:	10000005 */ 	b	.L0f11f6b8
/*  f11f6a4:	240cffff */ 	addiu	$t4,$zero,-1
.L0f11f6a8:
/*  f11f6a8:	440c4000 */ 	mfc1	$t4,$f8
/*  f11f6ac:	00000000 */ 	nop
/*  f11f6b0:	0580fffb */ 	bltz	$t4,.L0f11f6a0
/*  f11f6b4:	00000000 */ 	nop
.L0f11f6b8:
/*  f11f6b8:	a08c0016 */ 	sb	$t4,0x16($a0)
/*  f11f6bc:	8dad33a4 */ 	lw	$t5,%lo(var800a33a4)($t5)
/*  f11f6c0:	44cbf800 */ 	ctc1	$t3,$31
/*  f11f6c4:	51a00005 */ 	beqzl	$t5,.L0f11f6dc
/*  f11f6c8:	a08f0017 */ 	sb	$t7,0x17($a0)
/*  f11f6cc:	a0820016 */ 	sb	$v0,0x16($a0)
/*  f11f6d0:	a0820015 */ 	sb	$v0,0x15($a0)
/*  f11f6d4:	a0820014 */ 	sb	$v0,0x14($a0)
/*  f11f6d8:	a08f0017 */ 	sb	$t7,0x17($a0)
.L0f11f6dc:
/*  f11f6dc:	8fbf0014 */ 	lw	$ra,0x14($sp)
/*  f11f6e0:	27bd0018 */ 	addiu	$sp,$sp,0x18
/*  f11f6e4:	03e00008 */ 	jr	$ra
/*  f11f6e8:	00000000 */ 	nop
);

GLOBAL_ASM(
glabel sky0f11f6ec
.late_rodata
glabel var7f1b4ffc
.word 0x3b808081
.text
/*  f11f6ec:	27bdffe8 */ 	addiu	$sp,$sp,-24
/*  f11f6f0:	afbf0014 */ 	sw	$ra,0x14($sp)
/*  f11f6f4:	afa5001c */ 	sw	$a1,0x1c($sp)
/*  f11f6f8:	0fc595f3 */ 	jal	envGetCurrent
/*  f11f6fc:	afa40018 */ 	sw	$a0,0x18($sp)
/*  f11f700:	904e0008 */ 	lbu	$t6,0x8($v0)
/*  f11f704:	3c013f80 */ 	lui	$at,0x3f80
/*  f11f708:	44818000 */ 	mtc1	$at,$f16
/*  f11f70c:	448e2000 */ 	mtc1	$t6,$f4
/*  f11f710:	3c017f1b */ 	lui	$at,%hi(var7f1b4ffc)
/*  f11f714:	c4324ffc */ 	lwc1	$f18,%lo(var7f1b4ffc)($at)
/*  f11f718:	8fa40018 */ 	lw	$a0,0x18($sp)
/*  f11f71c:	05c10005 */ 	bgez	$t6,.L0f11f734
/*  f11f720:	468020a0 */ 	cvt.s.w	$f2,$f4
/*  f11f724:	3c014f80 */ 	lui	$at,0x4f80
/*  f11f728:	44813000 */ 	mtc1	$at,$f6
/*  f11f72c:	00000000 */ 	nop
/*  f11f730:	46061080 */ 	add.s	$f2,$f2,$f6
.L0f11f734:
/*  f11f734:	904f0009 */ 	lbu	$t7,0x9($v0)
/*  f11f738:	3c014f80 */ 	lui	$at,0x4f80
/*  f11f73c:	240d00ff */ 	addiu	$t5,$zero,0xff
/*  f11f740:	448f4000 */ 	mtc1	$t7,$f8
/*  f11f744:	05e10004 */ 	bgez	$t7,.L0f11f758
/*  f11f748:	46804320 */ 	cvt.s.w	$f12,$f8
/*  f11f74c:	44815000 */ 	mtc1	$at,$f10
/*  f11f750:	00000000 */ 	nop
/*  f11f754:	460a6300 */ 	add.s	$f12,$f12,$f10
.L0f11f758:
/*  f11f758:	9058000a */ 	lbu	$t8,0xa($v0)
/*  f11f75c:	3c014f80 */ 	lui	$at,0x4f80
/*  f11f760:	c7a8001c */ 	lwc1	$f8,0x1c($sp)
/*  f11f764:	44982000 */ 	mtc1	$t8,$f4
/*  f11f768:	00000000 */ 	nop
/*  f11f76c:	468023a0 */ 	cvt.s.w	$f14,$f4
/*  f11f770:	46121102 */ 	mul.s	$f4,$f2,$f18
/*  f11f774:	07030005 */ 	bgezl	$t8,.L0f11f78c
/*  f11f778:	46048181 */ 	sub.s	$f6,$f16,$f4
/*  f11f77c:	44813000 */ 	mtc1	$at,$f6
/*  f11f780:	00000000 */ 	nop
/*  f11f784:	46067380 */ 	add.s	$f14,$f14,$f6
/*  f11f788:	46048181 */ 	sub.s	$f6,$f16,$f4
.L0f11f78c:
/*  f11f78c:	c44a0034 */ 	lwc1	$f10,0x34($v0)
/*  f11f790:	24080001 */ 	addiu	$t0,$zero,0x1
/*  f11f794:	46088001 */ 	sub.s	$f0,$f16,$f8
/*  f11f798:	46065202 */ 	mul.s	$f8,$f10,$f6
/*  f11f79c:	3c014f00 */ 	lui	$at,0x4f00
/*  f11f7a0:	46004102 */ 	mul.s	$f4,$f8,$f0
/*  f11f7a4:	46022280 */ 	add.s	$f10,$f4,$f2
/*  f11f7a8:	4459f800 */ 	cfc1	$t9,$31
/*  f11f7ac:	44c8f800 */ 	ctc1	$t0,$31
/*  f11f7b0:	00000000 */ 	nop
/*  f11f7b4:	460051a4 */ 	cvt.w.s	$f6,$f10
/*  f11f7b8:	4448f800 */ 	cfc1	$t0,$31
/*  f11f7bc:	00000000 */ 	nop
/*  f11f7c0:	31080078 */ 	andi	$t0,$t0,0x78
/*  f11f7c4:	51000013 */ 	beqzl	$t0,.L0f11f814
/*  f11f7c8:	44083000 */ 	mfc1	$t0,$f6
/*  f11f7cc:	44813000 */ 	mtc1	$at,$f6
/*  f11f7d0:	24080001 */ 	addiu	$t0,$zero,0x1
/*  f11f7d4:	46065181 */ 	sub.s	$f6,$f10,$f6
/*  f11f7d8:	44c8f800 */ 	ctc1	$t0,$31
/*  f11f7dc:	00000000 */ 	nop
/*  f11f7e0:	460031a4 */ 	cvt.w.s	$f6,$f6
/*  f11f7e4:	4448f800 */ 	cfc1	$t0,$31
/*  f11f7e8:	00000000 */ 	nop
/*  f11f7ec:	31080078 */ 	andi	$t0,$t0,0x78
/*  f11f7f0:	15000005 */ 	bnez	$t0,.L0f11f808
/*  f11f7f4:	00000000 */ 	nop
/*  f11f7f8:	44083000 */ 	mfc1	$t0,$f6
/*  f11f7fc:	3c018000 */ 	lui	$at,0x8000
/*  f11f800:	10000007 */ 	b	.L0f11f820
/*  f11f804:	01014025 */ 	or	$t0,$t0,$at
.L0f11f808:
/*  f11f808:	10000005 */ 	b	.L0f11f820
/*  f11f80c:	2408ffff */ 	addiu	$t0,$zero,-1
/*  f11f810:	44083000 */ 	mfc1	$t0,$f6
.L0f11f814:
/*  f11f814:	00000000 */ 	nop
/*  f11f818:	0500fffb */ 	bltz	$t0,.L0f11f808
/*  f11f81c:	00000000 */ 	nop
.L0f11f820:
/*  f11f820:	44d9f800 */ 	ctc1	$t9,$31
/*  f11f824:	a0880014 */ 	sb	$t0,0x14($a0)
/*  f11f828:	c4480038 */ 	lwc1	$f8,0x38($v0)
/*  f11f82c:	46126102 */ 	mul.s	$f4,$f12,$f18
/*  f11f830:	240a0001 */ 	addiu	$t2,$zero,0x1
/*  f11f834:	3c014f00 */ 	lui	$at,0x4f00
/*  f11f838:	46048281 */ 	sub.s	$f10,$f16,$f4
/*  f11f83c:	460a4182 */ 	mul.s	$f6,$f8,$f10
/*  f11f840:	00000000 */ 	nop
/*  f11f844:	46003102 */ 	mul.s	$f4,$f6,$f0
/*  f11f848:	460c2200 */ 	add.s	$f8,$f4,$f12
/*  f11f84c:	4449f800 */ 	cfc1	$t1,$31
/*  f11f850:	44caf800 */ 	ctc1	$t2,$31
/*  f11f854:	00000000 */ 	nop
/*  f11f858:	460042a4 */ 	cvt.w.s	$f10,$f8
/*  f11f85c:	444af800 */ 	cfc1	$t2,$31
/*  f11f860:	00000000 */ 	nop
/*  f11f864:	314a0078 */ 	andi	$t2,$t2,0x78
/*  f11f868:	11400012 */ 	beqz	$t2,.L0f11f8b4
/*  f11f86c:	00000000 */ 	nop
/*  f11f870:	44815000 */ 	mtc1	$at,$f10
/*  f11f874:	240a0001 */ 	addiu	$t2,$zero,0x1
/*  f11f878:	460a4281 */ 	sub.s	$f10,$f8,$f10
/*  f11f87c:	44caf800 */ 	ctc1	$t2,$31
/*  f11f880:	00000000 */ 	nop
/*  f11f884:	460052a4 */ 	cvt.w.s	$f10,$f10
/*  f11f888:	444af800 */ 	cfc1	$t2,$31
/*  f11f88c:	00000000 */ 	nop
/*  f11f890:	314a0078 */ 	andi	$t2,$t2,0x78
/*  f11f894:	15400005 */ 	bnez	$t2,.L0f11f8ac
/*  f11f898:	00000000 */ 	nop
/*  f11f89c:	440a5000 */ 	mfc1	$t2,$f10
/*  f11f8a0:	3c018000 */ 	lui	$at,0x8000
/*  f11f8a4:	10000007 */ 	b	.L0f11f8c4
/*  f11f8a8:	01415025 */ 	or	$t2,$t2,$at
.L0f11f8ac:
/*  f11f8ac:	10000005 */ 	b	.L0f11f8c4
/*  f11f8b0:	240affff */ 	addiu	$t2,$zero,-1
.L0f11f8b4:
/*  f11f8b4:	440a5000 */ 	mfc1	$t2,$f10
/*  f11f8b8:	00000000 */ 	nop
/*  f11f8bc:	0540fffb */ 	bltz	$t2,.L0f11f8ac
/*  f11f8c0:	00000000 */ 	nop
.L0f11f8c4:
/*  f11f8c4:	44c9f800 */ 	ctc1	$t1,$31
/*  f11f8c8:	a08a0015 */ 	sb	$t2,0x15($a0)
/*  f11f8cc:	c446003c */ 	lwc1	$f6,0x3c($v0)
/*  f11f8d0:	46127102 */ 	mul.s	$f4,$f14,$f18
/*  f11f8d4:	240c0001 */ 	addiu	$t4,$zero,0x1
/*  f11f8d8:	3c014f00 */ 	lui	$at,0x4f00
/*  f11f8dc:	46048201 */ 	sub.s	$f8,$f16,$f4
/*  f11f8e0:	46083282 */ 	mul.s	$f10,$f6,$f8
/*  f11f8e4:	00000000 */ 	nop
/*  f11f8e8:	46005102 */ 	mul.s	$f4,$f10,$f0
/*  f11f8ec:	460e2180 */ 	add.s	$f6,$f4,$f14
/*  f11f8f0:	444bf800 */ 	cfc1	$t3,$31
/*  f11f8f4:	44ccf800 */ 	ctc1	$t4,$31
/*  f11f8f8:	00000000 */ 	nop
/*  f11f8fc:	46003224 */ 	cvt.w.s	$f8,$f6
/*  f11f900:	444cf800 */ 	cfc1	$t4,$31
/*  f11f904:	00000000 */ 	nop
/*  f11f908:	318c0078 */ 	andi	$t4,$t4,0x78
/*  f11f90c:	11800012 */ 	beqz	$t4,.L0f11f958
/*  f11f910:	00000000 */ 	nop
/*  f11f914:	44814000 */ 	mtc1	$at,$f8
/*  f11f918:	240c0001 */ 	addiu	$t4,$zero,0x1
/*  f11f91c:	46083201 */ 	sub.s	$f8,$f6,$f8
/*  f11f920:	44ccf800 */ 	ctc1	$t4,$31
/*  f11f924:	00000000 */ 	nop
/*  f11f928:	46004224 */ 	cvt.w.s	$f8,$f8
/*  f11f92c:	444cf800 */ 	cfc1	$t4,$31
/*  f11f930:	00000000 */ 	nop
/*  f11f934:	318c0078 */ 	andi	$t4,$t4,0x78
/*  f11f938:	15800005 */ 	bnez	$t4,.L0f11f950
/*  f11f93c:	00000000 */ 	nop
/*  f11f940:	440c4000 */ 	mfc1	$t4,$f8
/*  f11f944:	3c018000 */ 	lui	$at,0x8000
/*  f11f948:	10000007 */ 	b	.L0f11f968
/*  f11f94c:	01816025 */ 	or	$t4,$t4,$at
.L0f11f950:
/*  f11f950:	10000005 */ 	b	.L0f11f968
/*  f11f954:	240cffff */ 	addiu	$t4,$zero,-1
.L0f11f958:
/*  f11f958:	440c4000 */ 	mfc1	$t4,$f8
/*  f11f95c:	00000000 */ 	nop
/*  f11f960:	0580fffb */ 	bltz	$t4,.L0f11f950
/*  f11f964:	00000000 */ 	nop
.L0f11f968:
/*  f11f968:	a08c0016 */ 	sb	$t4,0x16($a0)
/*  f11f96c:	a08d0017 */ 	sb	$t5,0x17($a0)
/*  f11f970:	8fbf0014 */ 	lw	$ra,0x14($sp)
/*  f11f974:	44cbf800 */ 	ctc1	$t3,$31
/*  f11f978:	27bd0018 */ 	addiu	$sp,$sp,0x18
/*  f11f97c:	03e00008 */ 	jr	$ra
/*  f11f980:	00000000 */ 	nop
);

Gfx *skyRender(Gfx *gdl)
{
	struct coord sp6a4;
	struct coord sp698;
	struct coord sp68c;
	struct coord sp680;
	struct coord sp674;
	struct coord sp668;
	struct coord sp65c;
	struct coord sp650;
	struct coord sp644;
	struct coord sp638;
	struct coord sp62c;
	struct coord sp620;
	struct coord sp614;
	struct coord sp608;
	struct coord sp5fc;
	struct coord sp5f0;
	struct coord sp5e4;
	struct coord sp5d8;
	struct coord sp5cc;
	struct coord sp5c0;
	struct coord sp5b4;
	struct coord sp5a8;
	struct coord sp59c;
	struct coord sp590;
	f32 sp58c;
	f32 sp588;
	f32 sp584;
	f32 sp580;
	f32 sp57c;
	f32 sp578;
	f32 sp574;
	f32 sp570;
	f32 sp56c;
	f32 sp568;
	f32 sp564;
	f32 sp560;
	f32 sp55c;
	f32 sp558;
	f32 sp554;
	f32 sp550;
	f32 sp54c;
	f32 sp548;
	s32 s1;
	s32 j;
	s32 k;
	s32 sp538;
	s32 sp534;
	s32 sp530;
	s32 sp52c;
	struct skything18 sp4b4[5];
	struct skything18 sp43c[5];
	f32 tmp;
	f32 scale;
	bool sp430;
	struct environment *env;

	sp430 = false;
	env = envGetCurrent();

	if (!env->clouds_enabled || g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
		if (PLAYERCOUNT() == 1) {
			gDPSetCycleType(gdl++, G_CYC_FILL);

			if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
				gdl = viSetFillColour(gdl, 0, 0, 0);
			} else {
				gdl = viSetFillColour(gdl, env->sky_r, env->sky_g, env->sky_b);
			}

			gDPFillRectangle(gdl++, viGetViewLeft(), viGetViewTop(),
					viGetViewLeft() + viGetViewWidth() - 1,
					viGetViewTop() + viGetViewHeight() - 1);

			gDPPipeSync(gdl++);
			return gdl;
		}

		gDPPipeSync(gdl++);
		gDPSetCycleType(gdl++, G_CYC_FILL);

		if (g_Vars.currentplayer->visionmode == VISIONMODE_XRAY) {
			gdl = viSetFillColour(gdl, 0, 0, 0);
		} else {
			gdl = viSetFillColour(gdl, env->sky_r, env->sky_g, env->sky_b);
		}

		gDPSetRenderMode(gdl++, G_RM_NOOP, G_RM_NOOP2);

		gDPFillRectangle(gdl++,
				g_Vars.currentplayer->viewleft, g_Vars.currentplayer->viewtop,
				g_Vars.currentplayer->viewleft + g_Vars.currentplayer->viewwidth - 1,
				g_Vars.currentplayer->viewtop + g_Vars.currentplayer->viewheight - 1);

		gDPPipeSync(gdl++);
		return gdl;
	}

	gdl = viSetFillColour(gdl, env->sky_r, env->sky_g, env->sky_b);

	if (&sp6a4);

	sky0f11f000(0.0f, 0.0f, &sp6a4);
	sky0f11f000(camGetScreenWidth() - 0.1f, 0.0f, &sp698);
	sky0f11f000(0.0f, camGetScreenHeight() - 0.1f, &sp68c);
	sky0f11f000(camGetScreenWidth() - 0.1f, camGetScreenHeight() - 0.1f, &sp680);

	sp538 = sky0f11f07c(&sp6a4, &sp644, &sp58c);
	sp534 = sky0f11f07c(&sp698, &sp638, &sp588);
	sp530 = sky0f11f07c(&sp68c, &sp62c, &sp584);
	sp52c = sky0f11f07c(&sp680, &sp620, &sp580);

	sky0f11f1fc(&sp6a4, &sp5e4, &sp56c);
	sky0f11f1fc(&sp698, &sp5d8, &sp568);
	sky0f11f1fc(&sp68c, &sp5cc, &sp564);
	sky0f11f1fc(&sp680, &sp5c0, &sp560);

	if (sp538 != sp530) {
		sp54c = camGetScreenTop() + camGetScreenHeight() * (sp6a4.f[1] / (sp6a4.f[1] - sp68c.f[1]));

		sky0f11f000(0.0f, sp54c, &sp65c);
		sky0f11f384(&sp6a4, &sp68c, &sp65c);
		sky0f11f07c(&sp65c, &sp5fc, &sp574);
		sky0f11f1fc(&sp65c, &sp59c, &sp554);
	} else {
		sp54c = 0.0f;
	}

	if (sp534 != sp52c) {
		sp548 = camGetScreenTop() + camGetScreenHeight() * (sp698.f[1] / (sp698.f[1] - sp680.f[1]));

		sky0f11f000(camGetScreenWidth() - 0.1f, sp548, &sp650);
		sky0f11f384(&sp698, &sp680, &sp650);
		sky0f11f07c(&sp650, &sp5f0, &sp570);
		sky0f11f1fc(&sp650, &sp590, &sp550);
	} else {
		sp548 = 0.0f;
	}

	if (sp538 != sp534) {
		sky0f11f000(camGetScreenLeft() + camGetScreenWidth() * (sp6a4.f[1] / (sp6a4.f[1] - sp698.f[1])), 0.0f, &sp674);
		sky0f11f384(&sp6a4, &sp698, &sp674);
		sky0f11f07c(&sp674, &sp614, &sp57c);
		sky0f11f1fc(&sp674, &sp5b4, &sp55c);
	}

	if (sp530 != sp52c) {
		tmp = camGetScreenLeft() + camGetScreenWidth() * (sp68c.f[1] / (sp68c.f[1] - sp680.f[1]));

		sky0f11f000(tmp, camGetScreenHeight() - 0.1f, &sp668);
		sky0f11f384(&sp68c, &sp680, &sp668);
		sky0f11f07c(&sp668, &sp608, &sp578);
		sky0f11f1fc(&sp668, &sp5a8, &sp558);
	}

	switch ((sp538 << 3) | (sp534 << 2) | (sp530 << 1) | sp52c) {
	case 15:
		s1 = 0;
		scale = 0.033333335f;
		break;
	case 0:
		s1 = 4;
		scale = 0.033333335f;
		sp43c[0].unk00 = sp5e4.f[0] * scale;
		sp43c[0].unk04 = sp5e4.f[1] * scale;
		sp43c[0].unk08 = sp5e4.f[2] * scale;
		sp43c[1].unk00 = sp5d8.f[0] * scale;
		sp43c[1].unk04 = sp5d8.f[1] * scale;
		sp43c[1].unk08 = sp5d8.f[2] * scale;
		sp43c[2].unk00 = sp5cc.f[0] * scale;
		sp43c[2].unk04 = sp5cc.f[1] * scale;
		sp43c[2].unk08 = sp5cc.f[2] * scale;
		sp43c[3].unk00 = sp5c0.f[0] * scale;
		sp43c[3].unk04 = sp5c0.f[1] * scale;
		sp43c[3].unk08 = sp5c0.f[2] * scale;
		sp43c[0].unk0c = sp5e4.f[0];
		sp43c[0].unk10 = sp5e4.f[2] + g_SkyCloudOffset;
		sp43c[1].unk0c = sp5d8.f[0];
		sp43c[1].unk10 = sp5d8.f[2] + g_SkyCloudOffset;
		sp43c[2].unk0c = sp5cc.f[0];
		sp43c[2].unk10 = sp5cc.f[2] + g_SkyCloudOffset;
		sp43c[3].unk0c = sp5c0.f[0];
		sp43c[3].unk10 = sp5c0.f[2] + g_SkyCloudOffset;

		sky0f11f6ec(&sp43c[0], sp56c);
		sky0f11f6ec(&sp43c[1], sp568);
		sky0f11f6ec(&sp43c[2], sp564);
		sky0f11f6ec(&sp43c[3], sp560);
		break;
	case 3:
		s1 = 4;
		scale = 0.033333335f;
		sp43c[0].unk00 = sp5e4.f[0] * scale;
		sp43c[0].unk04 = sp5e4.f[1] * scale;
		sp43c[0].unk08 = sp5e4.f[2] * scale;
		sp43c[1].unk00 = sp5d8.f[0] * scale;
		sp43c[1].unk04 = sp5d8.f[1] * scale;
		sp43c[1].unk08 = sp5d8.f[2] * scale;
		sp43c[2].unk00 = sp59c.f[0] * scale;
		sp43c[2].unk04 = sp59c.f[1] * scale;
		sp43c[2].unk08 = sp59c.f[2] * scale;
		sp43c[3].unk00 = sp590.f[0] * scale;
		sp43c[3].unk04 = sp590.f[1] * scale;
		sp43c[3].unk08 = sp590.f[2] * scale;
		sp43c[0].unk0c = sp5e4.f[0];
		sp43c[0].unk10 = sp5e4.f[2] + g_SkyCloudOffset;
		sp43c[1].unk0c = sp5d8.f[0];
		sp43c[1].unk10 = sp5d8.f[2] + g_SkyCloudOffset;
		sp43c[2].unk0c = sp59c.f[0];
		sp43c[2].unk10 = sp59c.f[2] + g_SkyCloudOffset;
		sp43c[3].unk0c = sp590.f[0];
		sp43c[3].unk10 = sp590.f[2] + g_SkyCloudOffset;

		sky0f11f6ec(&sp43c[0], sp56c);
		sky0f11f6ec(&sp43c[1], sp568);
		sky0f11f6ec(&sp43c[2], sp554);
		sky0f11f6ec(&sp43c[3], sp550);
		break;
	case 12:
		s1 = 4;
		sp430 = true;
		scale = 0.033333335f;
		sp43c[0].unk00 = sp5c0.f[0] * scale;
		sp43c[0].unk04 = sp5c0.f[1] * scale;
		sp43c[0].unk08 = sp5c0.f[2] * scale;
		sp43c[1].unk00 = sp5cc.f[0] * scale;
		sp43c[1].unk04 = sp5cc.f[1] * scale;
		sp43c[1].unk08 = sp5cc.f[2] * scale;
		sp43c[2].unk00 = sp590.f[0] * scale;
		sp43c[2].unk04 = sp590.f[1] * scale;
		sp43c[2].unk08 = sp590.f[2] * scale;
		sp43c[3].unk00 = sp59c.f[0] * scale;
		sp43c[3].unk04 = sp59c.f[1] * scale;
		sp43c[3].unk08 = sp59c.f[2] * scale;
		sp43c[0].unk0c = sp5c0.f[0];
		sp43c[0].unk10 = sp5c0.f[2] + g_SkyCloudOffset;
		sp43c[1].unk0c = sp5cc.f[0];
		sp43c[1].unk10 = sp5cc.f[2] + g_SkyCloudOffset;
		sp43c[2].unk0c = sp590.f[0];
		sp43c[2].unk10 = sp590.f[2] + g_SkyCloudOffset;
		sp43c[3].unk0c = sp59c.f[0];
		sp43c[3].unk10 = sp59c.f[2] + g_SkyCloudOffset;

		sky0f11f6ec(&sp43c[0], sp560);
		sky0f11f6ec(&sp43c[1], sp564);
		sky0f11f6ec(&sp43c[2], sp550);
		sky0f11f6ec(&sp43c[3], sp554);
		break;
	case 10:
		s1 = 4;
		scale = 0.033333335f;
		sp43c[0].unk00 = sp5d8.f[0] * scale;
		sp43c[0].unk04 = sp5d8.f[1] * scale;
		sp43c[0].unk08 = sp5d8.f[2] * scale;
		sp43c[1].unk00 = sp5c0.f[0] * scale;
		sp43c[1].unk04 = sp5c0.f[1] * scale;
		sp43c[1].unk08 = sp5c0.f[2] * scale;
		sp43c[2].unk00 = sp5b4.f[0] * scale;
		sp43c[2].unk04 = sp5b4.f[1] * scale;
		sp43c[2].unk08 = sp5b4.f[2] * scale;
		sp43c[3].unk00 = sp5a8.f[0] * scale;
		sp43c[3].unk04 = sp5a8.f[1] * scale;
		sp43c[3].unk08 = sp5a8.f[2] * scale;
		sp43c[0].unk0c = sp5d8.f[0];
		sp43c[0].unk10 = sp5d8.f[2] + g_SkyCloudOffset;
		sp43c[1].unk0c = sp5c0.f[0];
		sp43c[1].unk10 = sp5c0.f[2] + g_SkyCloudOffset;
		sp43c[2].unk0c = sp5b4.f[0];
		sp43c[2].unk10 = sp5b4.f[2] + g_SkyCloudOffset;
		sp43c[3].unk0c = sp5a8.f[0];
		sp43c[3].unk10 = sp5a8.f[2] + g_SkyCloudOffset;

		sky0f11f6ec(&sp43c[0], sp568);
		sky0f11f6ec(&sp43c[1], sp560);
		sky0f11f6ec(&sp43c[2], sp55c);
		sky0f11f6ec(&sp43c[3], sp558);
		break;
	case 5:
		s1 = 4;
		scale = 0.033333335f;
		sp43c[0].unk00 = sp5cc.f[0] * scale;
		sp43c[0].unk04 = sp5cc.f[1] * scale;
		sp43c[0].unk08 = sp5cc.f[2] * scale;
		sp43c[1].unk00 = sp5e4.f[0] * scale;
		sp43c[1].unk04 = sp5e4.f[1] * scale;
		sp43c[1].unk08 = sp5e4.f[2] * scale;
		sp43c[2].unk00 = sp5a8.f[0] * scale;
		sp43c[2].unk04 = sp5a8.f[1] * scale;
		sp43c[2].unk08 = sp5a8.f[2] * scale;
		sp43c[3].unk00 = sp5b4.f[0] * scale;
		sp43c[3].unk04 = sp5b4.f[1] * scale;
		sp43c[3].unk08 = sp5b4.f[2] * scale;
		sp43c[0].unk0c = sp5cc.f[0];
		sp43c[0].unk10 = sp5cc.f[2] + g_SkyCloudOffset;
		sp43c[1].unk0c = sp5e4.f[0];
		sp43c[1].unk10 = sp5e4.f[2] + g_SkyCloudOffset;
		sp43c[2].unk0c = sp5a8.f[0];
		sp43c[2].unk10 = sp5a8.f[2] + g_SkyCloudOffset;
		sp43c[3].unk0c = sp5b4.f[0];
		sp43c[3].unk10 = sp5b4.f[2] + g_SkyCloudOffset;

		sky0f11f6ec(&sp43c[0], sp564);
		sky0f11f6ec(&sp43c[1], sp56c);
		sky0f11f6ec(&sp43c[2], sp558);
		sky0f11f6ec(&sp43c[3], sp55c);
		break;
	case 14:
		s1 = 3;
		scale = 0.033333335f;
		sp43c[0].unk00 = sp5c0.f[0] * scale;
		sp43c[0].unk04 = sp5c0.f[1] * scale;
		sp43c[0].unk08 = sp5c0.f[2] * scale;
		sp43c[1].unk00 = sp5a8.f[0] * scale;
		sp43c[1].unk04 = sp5a8.f[1] * scale;
		sp43c[1].unk08 = sp5a8.f[2] * scale;
		sp43c[2].unk00 = sp590.f[0] * scale;
		sp43c[2].unk04 = sp590.f[1] * scale;
		sp43c[2].unk08 = sp590.f[2] * scale;
		sp43c[0].unk0c = sp5c0.f[0];
		sp43c[0].unk10 = sp5c0.f[2] + g_SkyCloudOffset;
		sp43c[1].unk0c = sp5a8.f[0];
		sp43c[1].unk10 = sp5a8.f[2] + g_SkyCloudOffset;
		sp43c[2].unk0c = sp590.f[0];
		sp43c[2].unk10 = sp590.f[2] + g_SkyCloudOffset;

		sky0f11f6ec(&sp43c[0], sp560);
		sky0f11f6ec(&sp43c[1], sp558);
		sky0f11f6ec(&sp43c[2], sp550);
		break;
	case 13:
		s1 = 3;
		scale = 0.033333335f;
		sp43c[0].unk00 = sp5cc.f[0] * scale;
		sp43c[0].unk04 = sp5cc.f[1] * scale;
		sp43c[0].unk08 = sp5cc.f[2] * scale;
		sp43c[1].unk00 = sp59c.f[0] * scale;
		sp43c[1].unk04 = sp59c.f[1] * scale;
		sp43c[1].unk08 = sp59c.f[2] * scale;
		sp43c[2].unk00 = sp5a8.f[0] * scale;
		sp43c[2].unk04 = sp5a8.f[1] * scale;
		sp43c[2].unk08 = sp5a8.f[2] * scale;
		sp43c[0].unk0c = sp5cc.f[0];
		sp43c[0].unk10 = sp5cc.f[2] + g_SkyCloudOffset;
		sp43c[1].unk0c = sp59c.f[0];
		sp43c[1].unk10 = sp59c.f[2] + g_SkyCloudOffset;
		sp43c[2].unk0c = sp5a8.f[0];
		sp43c[2].unk10 = sp5a8.f[2] + g_SkyCloudOffset;

		sky0f11f6ec(&sp43c[0], sp564);
		sky0f11f6ec(&sp43c[1], sp554);
		sky0f11f6ec(&sp43c[2], sp558);
		break;
	case 11:
		s1 = 3;
		scale = 0.033333335f;
		sp43c[0].unk00 = sp5d8.f[0] * scale;
		sp43c[0].unk04 = sp5d8.f[1] * scale;
		sp43c[0].unk08 = sp5d8.f[2] * scale;
		sp43c[1].unk00 = sp590.f[0] * scale;
		sp43c[1].unk04 = sp590.f[1] * scale;
		sp43c[1].unk08 = sp590.f[2] * scale;
		sp43c[2].unk00 = sp5b4.f[0] * scale;
		sp43c[2].unk04 = sp5b4.f[1] * scale;
		sp43c[2].unk08 = sp5b4.f[2] * scale;
		sp43c[0].unk0c = sp5d8.f[0];
		sp43c[0].unk10 = sp5d8.f[2] + g_SkyCloudOffset;
		sp43c[1].unk0c = sp590.f[0];
		sp43c[1].unk10 = sp590.f[2] + g_SkyCloudOffset;
		sp43c[2].unk0c = sp5b4.f[0];
		sp43c[2].unk10 = sp5b4.f[2] + g_SkyCloudOffset;

		sky0f11f6ec(&sp43c[0], sp568);
		sky0f11f6ec(&sp43c[1], sp550);
		sky0f11f6ec(&sp43c[2], sp55c);
		break;
	case 7:
		s1 = 3;
		scale = 0.033333335f;
		sp43c[0].unk00 = sp5e4.f[0] * scale;
		sp43c[0].unk04 = sp5e4.f[1] * scale;
		sp43c[0].unk08 = sp5e4.f[2] * scale;
		sp43c[1].unk00 = sp5b4.f[0] * scale;
		sp43c[1].unk04 = sp5b4.f[1] * scale;
		sp43c[1].unk08 = sp5b4.f[2] * scale;
		sp43c[2].unk00 = sp59c.f[0] * scale;
		sp43c[2].unk04 = sp59c.f[1] * scale;
		sp43c[2].unk08 = sp59c.f[2] * scale;
		sp43c[0].unk0c = sp5e4.f[0];
		sp43c[0].unk10 = sp5e4.f[2] + g_SkyCloudOffset;
		sp43c[1].unk0c = sp5b4.f[0];
		sp43c[1].unk10 = sp5b4.f[2] + g_SkyCloudOffset;
		sp43c[2].unk0c = sp59c.f[0];
		sp43c[2].unk10 = sp59c.f[2] + g_SkyCloudOffset;

		sky0f11f6ec(&sp43c[0], sp56c);
		sky0f11f6ec(&sp43c[1], sp55c);
		sky0f11f6ec(&sp43c[2], sp554);
		break;
	case 1:
		s1 = 5;
		scale = 0.033333335f;
		sp43c[0].unk00 = sp5cc.f[0] * scale;
		sp43c[0].unk04 = sp5cc.f[1] * scale;
		sp43c[0].unk08 = sp5cc.f[2] * scale;
		sp43c[1].unk00 = sp5e4.f[0] * scale;
		sp43c[1].unk04 = sp5e4.f[1] * scale;
		sp43c[1].unk08 = sp5e4.f[2] * scale;
		sp43c[2].unk00 = sp5d8.f[0] * scale;
		sp43c[2].unk04 = sp5d8.f[1] * scale;
		sp43c[2].unk08 = sp5d8.f[2] * scale;
		sp43c[3].unk00 = sp590.f[0] * scale;
		sp43c[3].unk04 = sp590.f[1] * scale;
		sp43c[3].unk08 = sp590.f[2] * scale;
		sp43c[4].unk00 = sp5a8.f[0] * scale;
		sp43c[4].unk04 = sp5a8.f[1] * scale;
		sp43c[4].unk08 = sp5a8.f[2] * scale;
		sp43c[0].unk0c = sp5cc.f[0];
		sp43c[0].unk10 = sp5cc.f[2] + g_SkyCloudOffset;
		sp43c[1].unk0c = sp5e4.f[0];
		sp43c[1].unk10 = sp5e4.f[2] + g_SkyCloudOffset;
		sp43c[2].unk0c = sp5d8.f[0];
		sp43c[2].unk10 = sp5d8.f[2] + g_SkyCloudOffset;
		sp43c[3].unk0c = sp590.f[0];
		sp43c[3].unk10 = sp590.f[2] + g_SkyCloudOffset;
		sp43c[4].unk0c = sp5a8.f[0];
		sp43c[4].unk10 = sp5a8.f[2] + g_SkyCloudOffset;

		sky0f11f6ec(&sp43c[0], sp564);
		sky0f11f6ec(&sp43c[1], sp56c);
		sky0f11f6ec(&sp43c[2], sp568);
		sky0f11f6ec(&sp43c[3], sp550);
		sky0f11f6ec(&sp43c[4], sp558);
		break;
	case 2:
		s1 = 5;
		scale = 0.033333335f;
		sp43c[0].unk00 = sp5e4.f[0] * scale;
		sp43c[0].unk04 = sp5e4.f[1] * scale;
		sp43c[0].unk08 = sp5e4.f[2] * scale;
		sp43c[1].unk00 = sp5d8.f[0] * scale;
		sp43c[1].unk04 = sp5d8.f[1] * scale;
		sp43c[1].unk08 = sp5d8.f[2] * scale;
		sp43c[2].unk00 = sp5c0.f[0] * scale;
		sp43c[2].unk04 = sp5c0.f[1] * scale;
		sp43c[2].unk08 = sp5c0.f[2] * scale;
		sp43c[3].unk00 = sp5a8.f[0] * scale;
		sp43c[3].unk04 = sp5a8.f[1] * scale;
		sp43c[3].unk08 = sp5a8.f[2] * scale;
		sp43c[4].unk00 = sp59c.f[0] * scale;
		sp43c[4].unk04 = sp59c.f[1] * scale;
		sp43c[4].unk08 = sp59c.f[2] * scale;
		sp43c[0].unk0c = sp5e4.f[0];
		sp43c[0].unk10 = sp5e4.f[2] + g_SkyCloudOffset;
		sp43c[1].unk0c = sp5d8.f[0];
		sp43c[1].unk10 = sp5d8.f[2] + g_SkyCloudOffset;
		sp43c[2].unk0c = sp5c0.f[0];
		sp43c[2].unk10 = sp5c0.f[2] + g_SkyCloudOffset;
		sp43c[3].unk0c = sp5a8.f[0];
		sp43c[3].unk10 = sp5a8.f[2] + g_SkyCloudOffset;
		sp43c[4].unk0c = sp59c.f[0];
		sp43c[4].unk10 = sp59c.f[2] + g_SkyCloudOffset;

		sky0f11f6ec(&sp43c[0], sp56c);
		sky0f11f6ec(&sp43c[1], sp568);
		sky0f11f6ec(&sp43c[2], sp560);
		sky0f11f6ec(&sp43c[3], sp558);
		sky0f11f6ec(&sp43c[4], sp554);
		break;
	case 4:
		s1 = 5;
		scale = 0.033333335f;
		sp43c[0].unk00 = sp5c0.f[0] * scale;
		sp43c[0].unk04 = sp5c0.f[1] * scale;
		sp43c[0].unk08 = sp5c0.f[2] * scale;
		sp43c[1].unk00 = sp5cc.f[0] * scale;
		sp43c[1].unk04 = sp5cc.f[1] * scale;
		sp43c[1].unk08 = sp5cc.f[2] * scale;
		sp43c[2].unk00 = sp5e4.f[0] * scale;
		sp43c[2].unk04 = sp5e4.f[1] * scale;
		sp43c[2].unk08 = sp5e4.f[2] * scale;
		sp43c[3].unk00 = sp5b4.f[0] * scale;
		sp43c[3].unk04 = sp5b4.f[1] * scale;
		sp43c[3].unk08 = sp5b4.f[2] * scale;
		sp43c[4].unk00 = sp590.f[0] * scale;
		sp43c[4].unk04 = sp590.f[1] * scale;
		sp43c[4].unk08 = sp590.f[2] * scale;
		sp43c[0].unk0c = sp5c0.f[0];
		sp43c[0].unk10 = sp5c0.f[2] + g_SkyCloudOffset;
		sp43c[1].unk0c = sp5cc.f[0];
		sp43c[1].unk10 = sp5cc.f[2] + g_SkyCloudOffset;
		sp43c[2].unk0c = sp5e4.f[0];
		sp43c[2].unk10 = sp5e4.f[2] + g_SkyCloudOffset;
		sp43c[3].unk0c = sp5b4.f[0];
		sp43c[3].unk10 = sp5b4.f[2] + g_SkyCloudOffset;
		sp43c[4].unk0c = sp590.f[0];
		sp43c[4].unk10 = sp590.f[2] + g_SkyCloudOffset;

		sky0f11f6ec(&sp43c[0], sp560);
		sky0f11f6ec(&sp43c[1], sp564);
		sky0f11f6ec(&sp43c[2], sp56c);
		sky0f11f6ec(&sp43c[3], sp55c);
		sky0f11f6ec(&sp43c[4], sp550);
		break;
	case 8:
		s1 = 5;
		scale = 0.033333335f;
		sp43c[0].unk00 = sp5d8.f[0] * scale;
		sp43c[0].unk04 = sp5d8.f[1] * scale;
		sp43c[0].unk08 = sp5d8.f[2] * scale;
		sp43c[1].unk00 = sp5c0.f[0] * scale;
		sp43c[1].unk04 = sp5c0.f[1] * scale;
		sp43c[1].unk08 = sp5c0.f[2] * scale;
		sp43c[2].unk00 = sp5cc.f[0] * scale;
		sp43c[2].unk04 = sp5cc.f[1] * scale;
		sp43c[2].unk08 = sp5cc.f[2] * scale;
		sp43c[3].unk00 = sp59c.f[0] * scale;
		sp43c[3].unk04 = sp59c.f[1] * scale;
		sp43c[3].unk08 = sp59c.f[2] * scale;
		sp43c[4].unk00 = sp5b4.f[0] * scale;
		sp43c[4].unk04 = sp5b4.f[1] * scale;
		sp43c[4].unk08 = sp5b4.f[2] * scale;
		sp43c[0].unk0c = sp5d8.f[0];
		sp43c[0].unk10 = sp5d8.f[2] + g_SkyCloudOffset;
		sp43c[1].unk0c = sp5c0.f[0];
		sp43c[1].unk10 = sp5c0.f[2] + g_SkyCloudOffset;
		sp43c[2].unk0c = sp5cc.f[0];
		sp43c[2].unk10 = sp5cc.f[2] + g_SkyCloudOffset;
		sp43c[3].unk0c = sp59c.f[0];
		sp43c[3].unk10 = sp59c.f[2] + g_SkyCloudOffset;
		sp43c[4].unk0c = sp5b4.f[0];
		sp43c[4].unk10 = sp5b4.f[2] + g_SkyCloudOffset;

		sky0f11f6ec(&sp43c[0], sp568);
		sky0f11f6ec(&sp43c[1], sp560);
		sky0f11f6ec(&sp43c[2], sp564);
		sky0f11f6ec(&sp43c[3], sp554);
		sky0f11f6ec(&sp43c[4], sp55c);
		break;
	default:
		return gdl;
	}

	if (s1 > 0) {
		Mtxf sp3cc;
		Mtxf sp38c;
		struct skything38 sp274[5];
		s32 i;

		mtx4MultMtx4(camGetMtxF1754(), camGetWorldToScreenMtxf(), &sp3cc);
		guScaleF(var800a33a8.m, 1.0f / scale, 1.0f / scale, 1.0f / scale);
		mtx4MultMtx4(&sp3cc, &var800a33a8, &sp38c);

		for (i = 0; i < s1; i++) {
			sky0f1228d0(&sp43c[i], &sp38c, 130, 65535.0f, 65535.0f, &sp274[i]);

			sp274[i].unk28 = skyClamp(sp274[i].unk28, camGetScreenLeft() * 4.0f, (camGetScreenLeft() + camGetScreenWidth()) * 4.0f - 1.0f);
			sp274[i].unk2c = skyClamp(sp274[i].unk2c, camGetScreenTop() * 4.0f, (camGetScreenTop() + camGetScreenHeight()) * 4.0f - 1.0f);

			if (sp274[i].unk2c > camGetScreenTop() * 4.0f + 4.0f
					&& sp274[i].unk2c < (camGetScreenTop() + camGetScreenHeight()) * 4.0f - 4.0f) {
				sp274[i].unk2c -= 4.0f;
			}
		}

		if (!env->water_enabled) {
			f32 f14 = 1279.0f;
			f32 f16 = 959.0f;
			f32 f2 = 0.0f;
			f32 f12 = 0.0f;

			for (j = 0; j < s1; j++) {
				if (sp274[j].unk28 < f14) {
					f14 = sp274[j].unk28;
				}

				if (sp274[j].unk28 > f2) {
					f2 = sp274[j].unk28;
				}

				if (sp274[j].unk2c < f16) {
					f16 = sp274[j].unk2c;
				}

				if (sp274[j].unk2c > f12) {
					f12 = sp274[j].unk2c;
				}
			}

			gDPPipeSync(gdl++);
			gDPSetCycleType(gdl++, G_CYC_FILL);
			gDPSetRenderMode(gdl++, G_RM_NOOP, G_RM_NOOP2);
			gDPSetTexturePersp(gdl++, G_TP_NONE);
			gDPFillRectangle(gdl++, (s32)(f14 * 0.25f), (s32)(f16 * 0.25f), (s32)(f2 * 0.25f), (s32)(f12 * 0.25f));
			gDPPipeSync(gdl++);
			gDPSetTexturePersp(gdl++, G_TP_PERSP);
		} else {
			gDPPipeSync(gdl++);

			texSelect(&gdl, &g_TexWaterConfigs[env->water_type], 1, 0, 2, 1, NULL);

			gDPSetRenderMode(gdl++, G_RM_OPA_SURF, G_RM_OPA_SURF2);

			if (s1 == 4) {
				gdl = sky0f122d4c(gdl, &sp274[0], &sp274[1], &sp274[3], 130.0f, true);

				if (sp430) {
					sp274[0].unk2c++;
					sp274[1].unk2c++;
					sp274[2].unk2c++;
					sp274[3].unk2c++;
				}

				gdl = sky0f122d4c(gdl, &sp274[3], &sp274[2], &sp274[0], 130.0f, true);
			} else if (s1 == 5) {
				gdl = sky0f122d4c(gdl, &sp274[0], &sp274[1], &sp274[2], 130.0f, true);
				gdl = sky0f122d4c(gdl, &sp274[0], &sp274[2], &sp274[3], 130.0f, true);
				gdl = sky0f122d4c(gdl, &sp274[0], &sp274[3], &sp274[4], 130.0f, true);
			} else if (s1 == 3) {
				gdl = sky0f122d4c(gdl, &sp274[0], &sp274[1], &sp274[2], 130.0f, true);
			}
		}
	}

	switch ((sp538 << 3) | (sp534 << 2) | (sp530 << 1) | sp52c) {
	case 0:
		return gdl;
	case 15:
		s1 = 4;
		sp4b4[0].unk00 = sp644.f[0] * scale;
		sp4b4[0].unk04 = sp644.f[1] * scale;
		sp4b4[0].unk08 = sp644.f[2] * scale;
		sp4b4[1].unk00 = sp638.f[0] * scale;
		sp4b4[1].unk04 = sp638.f[1] * scale;
		sp4b4[1].unk08 = sp638.f[2] * scale;
		sp4b4[2].unk00 = sp62c.f[0] * scale;
		sp4b4[2].unk04 = sp62c.f[1] * scale;
		sp4b4[2].unk08 = sp62c.f[2] * scale;
		sp4b4[3].unk00 = sp620.f[0] * scale;
		sp4b4[3].unk04 = sp620.f[1] * scale;
		sp4b4[3].unk08 = sp620.f[2] * scale;
		sp4b4[0].unk0c = sp644.f[0] * 0.1f;
		sp4b4[0].unk10 = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[1].unk0c = sp638.f[0] * 0.1f;
		sp4b4[1].unk10 = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[2].unk0c = sp62c.f[0] * 0.1f;
		sp4b4[2].unk10 = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[3].unk0c = sp620.f[0] * 0.1f;
		sp4b4[3].unk10 = sp620.f[2] * 0.1f + g_SkyCloudOffset;

		sky0f11f438(&sp4b4[0], sp58c);
		sky0f11f438(&sp4b4[1], sp588);
		sky0f11f438(&sp4b4[2], sp584);
		sky0f11f438(&sp4b4[3], sp580);
		break;
	case 12:
		s1 = 4;
		sp4b4[0].unk00 = sp644.f[0] * scale;
		sp4b4[0].unk04 = sp644.f[1] * scale;
		sp4b4[0].unk08 = sp644.f[2] * scale;
		sp4b4[1].unk00 = sp638.f[0] * scale;
		sp4b4[1].unk04 = sp638.f[1] * scale;
		sp4b4[1].unk08 = sp638.f[2] * scale;
		sp4b4[2].unk00 = sp5fc.f[0] * scale;
		sp4b4[2].unk04 = sp5fc.f[1] * scale;
		sp4b4[2].unk08 = sp5fc.f[2] * scale;
		sp4b4[3].unk00 = sp5f0.f[0] * scale;
		sp4b4[3].unk04 = sp5f0.f[1] * scale;
		sp4b4[3].unk08 = sp5f0.f[2] * scale;
		sp4b4[0].unk0c = sp644.f[0] * 0.1f;
		sp4b4[0].unk10 = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[1].unk0c = sp638.f[0] * 0.1f;
		sp4b4[1].unk10 = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[2].unk0c = sp5fc.f[0] * 0.1f;
		sp4b4[2].unk10 = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[3].unk0c = sp5f0.f[0] * 0.1f;
		sp4b4[3].unk10 = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;

		sky0f11f438(&sp4b4[0], sp58c);
		sky0f11f438(&sp4b4[1], sp588);
		sky0f11f438(&sp4b4[2], sp574);
		sky0f11f438(&sp4b4[3], sp570);
		break;
	case 3:
		s1 = 4;
		sp4b4[0].unk00 = sp620.f[0] * scale;
		sp4b4[0].unk04 = sp620.f[1] * scale;
		sp4b4[0].unk08 = sp620.f[2] * scale;
		sp4b4[1].unk00 = sp62c.f[0] * scale;
		sp4b4[1].unk04 = sp62c.f[1] * scale;
		sp4b4[1].unk08 = sp62c.f[2] * scale;
		sp4b4[2].unk00 = sp5f0.f[0] * scale;
		sp4b4[2].unk04 = sp5f0.f[1] * scale;
		sp4b4[2].unk08 = sp5f0.f[2] * scale;
		sp4b4[3].unk00 = sp5fc.f[0] * scale;
		sp4b4[3].unk04 = sp5fc.f[1] * scale;
		sp4b4[3].unk08 = sp5fc.f[2] * scale;
		sp4b4[0].unk0c = sp620.f[0] * 0.1f;
		sp4b4[0].unk10 = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[1].unk0c = sp62c.f[0] * 0.1f;
		sp4b4[1].unk10 = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[2].unk0c = sp5f0.f[0] * 0.1f;
		sp4b4[2].unk10 = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[3].unk0c = sp5fc.f[0] * 0.1f;
		sp4b4[3].unk10 = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;

		sky0f11f438(&sp4b4[0], sp580);
		sky0f11f438(&sp4b4[1], sp584);
		sky0f11f438(&sp4b4[2], sp570);
		sky0f11f438(&sp4b4[3], sp574);
		break;
	case 5:
		s1 = 4;
		sp4b4[0].unk00 = sp638.f[0] * scale;
		sp4b4[0].unk04 = sp638.f[1] * scale;
		sp4b4[0].unk08 = sp638.f[2] * scale;
		sp4b4[1].unk00 = sp620.f[0] * scale;
		sp4b4[1].unk04 = sp620.f[1] * scale;
		sp4b4[1].unk08 = sp620.f[2] * scale;
		sp4b4[2].unk00 = sp614.f[0] * scale;
		sp4b4[2].unk04 = sp614.f[1] * scale;
		sp4b4[2].unk08 = sp614.f[2] * scale;
		sp4b4[3].unk00 = sp608.f[0] * scale;
		sp4b4[3].unk04 = sp608.f[1] * scale;
		sp4b4[3].unk08 = sp608.f[2] * scale;
		sp4b4[0].unk0c = sp638.f[0] * 0.1f;
		sp4b4[0].unk10 = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[1].unk0c = sp620.f[0] * 0.1f;
		sp4b4[1].unk10 = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[2].unk0c = sp614.f[0] * 0.1f;
		sp4b4[2].unk10 = sp614.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[3].unk0c = sp608.f[0] * 0.1f;
		sp4b4[3].unk10 = sp608.f[2] * 0.1f + g_SkyCloudOffset;

		sky0f11f438(&sp4b4[0], sp588);
		sky0f11f438(&sp4b4[1], sp580);
		sky0f11f438(&sp4b4[2], sp57c);
		sky0f11f438(&sp4b4[3], sp578);
		break;
	case 10:
		s1 = 4;
		sp4b4[0].unk00 = sp62c.f[0] * scale;
		sp4b4[0].unk04 = sp62c.f[1] * scale;
		sp4b4[0].unk08 = sp62c.f[2] * scale;
		sp4b4[1].unk00 = sp644.f[0] * scale;
		sp4b4[1].unk04 = sp644.f[1] * scale;
		sp4b4[1].unk08 = sp644.f[2] * scale;
		sp4b4[2].unk00 = sp608.f[0] * scale;
		sp4b4[2].unk04 = sp608.f[1] * scale;
		sp4b4[2].unk08 = sp608.f[2] * scale;
		sp4b4[3].unk00 = sp614.f[0] * scale;
		sp4b4[3].unk04 = sp614.f[1] * scale;
		sp4b4[3].unk08 = sp614.f[2] * scale;
		sp4b4[0].unk0c = sp62c.f[0] * 0.1f;
		sp4b4[0].unk10 = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[1].unk0c = sp644.f[0] * 0.1f;
		sp4b4[1].unk10 = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[2].unk0c = sp608.f[0] * 0.1f;
		sp4b4[2].unk10 = sp608.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[3].unk0c = sp614.f[0] * 0.1f;
		sp4b4[3].unk10 = sp614.f[2] * 0.1f + g_SkyCloudOffset;

		sky0f11f438(&sp4b4[0], sp584);
		sky0f11f438(&sp4b4[1], sp58c);
		sky0f11f438(&sp4b4[2], sp578);
		sky0f11f438(&sp4b4[3], sp57c);
		break;
	case 1:
		s1 = 3;
		sp4b4[0].unk00 = sp620.f[0] * scale;
		sp4b4[0].unk04 = sp620.f[1] * scale;
		sp4b4[0].unk08 = sp620.f[2] * scale;
		sp4b4[1].unk00 = sp608.f[0] * scale;
		sp4b4[1].unk04 = sp608.f[1] * scale;
		sp4b4[1].unk08 = sp608.f[2] * scale;
		sp4b4[2].unk00 = sp5f0.f[0] * scale;
		sp4b4[2].unk04 = sp5f0.f[1] * scale;
		sp4b4[2].unk08 = sp5f0.f[2] * scale;
		sp4b4[0].unk0c = sp620.f[0] * 0.1f;
		sp4b4[0].unk10 = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[1].unk0c = sp608.f[0] * 0.1f;
		sp4b4[1].unk10 = sp608.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[2].unk0c = sp5f0.f[0] * 0.1f;
		sp4b4[2].unk10 = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;

		sky0f11f438(&sp4b4[0], sp580);
		sky0f11f438(&sp4b4[1], sp578);
		sky0f11f438(&sp4b4[2], sp570);
		break;
	case 2:
		s1 = 3;
		sp4b4[0].unk00 = sp62c.f[0] * scale;
		sp4b4[0].unk04 = sp62c.f[1] * scale;
		sp4b4[0].unk08 = sp62c.f[2] * scale;
		sp4b4[1].unk00 = sp5fc.f[0] * scale;
		sp4b4[1].unk04 = sp5fc.f[1] * scale;
		sp4b4[1].unk08 = sp5fc.f[2] * scale;
		sp4b4[2].unk00 = sp608.f[0] * scale;
		sp4b4[2].unk04 = sp608.f[1] * scale;
		sp4b4[2].unk08 = sp608.f[2] * scale;
		sp4b4[0].unk0c = sp62c.f[0] * 0.1f;
		sp4b4[0].unk10 = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[1].unk0c = sp5fc.f[0] * 0.1f;
		sp4b4[1].unk10 = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[2].unk0c = sp608.f[0] * 0.1f;
		sp4b4[2].unk10 = sp608.f[2] * 0.1f + g_SkyCloudOffset;

		sky0f11f438(&sp4b4[0], sp584);
		sky0f11f438(&sp4b4[1], sp574);
		sky0f11f438(&sp4b4[2], sp578);
		break;
	case 4:
		s1 = 3;
		sp4b4[0].unk00 = sp638.f[0] * scale;
		sp4b4[0].unk04 = sp638.f[1] * scale;
		sp4b4[0].unk08 = sp638.f[2] * scale;
		sp4b4[1].unk00 = sp5f0.f[0] * scale;
		sp4b4[1].unk04 = sp5f0.f[1] * scale;
		sp4b4[1].unk08 = sp5f0.f[2] * scale;
		sp4b4[2].unk00 = sp614.f[0] * scale;
		sp4b4[2].unk04 = sp614.f[1] * scale;
		sp4b4[2].unk08 = sp614.f[2] * scale;
		sp4b4[0].unk0c = sp638.f[0] * 0.1f;
		sp4b4[0].unk10 = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[1].unk0c = sp5f0.f[0] * 0.1f;
		sp4b4[1].unk10 = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[2].unk0c = sp614.f[0] * 0.1f;
		sp4b4[2].unk10 = sp614.f[2] * 0.1f + g_SkyCloudOffset;

		sky0f11f438(&sp4b4[0], sp588);
		sky0f11f438(&sp4b4[1], sp570);
		sky0f11f438(&sp4b4[2], sp57c);
		break;
	case 8:
		s1 = 3;
		sp4b4[0].unk00 = sp644.f[0] * scale;
		sp4b4[0].unk04 = sp644.f[1] * scale;
		sp4b4[0].unk08 = sp644.f[2] * scale;
		sp4b4[1].unk00 = sp614.f[0] * scale;
		sp4b4[1].unk04 = sp614.f[1] * scale;
		sp4b4[1].unk08 = sp614.f[2] * scale;
		sp4b4[2].unk00 = sp5fc.f[0] * scale;
		sp4b4[2].unk04 = sp5fc.f[1] * scale;
		sp4b4[2].unk08 = sp5fc.f[2] * scale;
		sp4b4[0].unk0c = sp644.f[0] * 0.1f;
		sp4b4[0].unk10 = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[1].unk0c = sp614.f[0] * 0.1f;
		sp4b4[1].unk10 = sp614.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[2].unk0c = sp5fc.f[0] * 0.1f;
		sp4b4[2].unk10 = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;

		sky0f11f438(&sp4b4[0], sp58c);
		sky0f11f438(&sp4b4[1], sp57c);
		sky0f11f438(&sp4b4[2], sp574);
		break;
	case 14:
		s1 = 5;
		sp4b4[0].unk00 = sp62c.f[0] * scale;
		sp4b4[0].unk04 = sp62c.f[1] * scale;
		sp4b4[0].unk08 = sp62c.f[2] * scale;
		sp4b4[1].unk00 = sp644.f[0] * scale;
		sp4b4[1].unk04 = sp644.f[1] * scale;
		sp4b4[1].unk08 = sp644.f[2] * scale;
		sp4b4[2].unk00 = sp638.f[0] * scale;
		sp4b4[2].unk04 = sp638.f[1] * scale;
		sp4b4[2].unk08 = sp638.f[2] * scale;
		sp4b4[3].unk00 = sp5f0.f[0] * scale;
		sp4b4[3].unk04 = sp5f0.f[1] * scale;
		sp4b4[3].unk08 = sp5f0.f[2] * scale;
		sp4b4[4].unk00 = sp608.f[0] * scale;
		sp4b4[4].unk04 = sp608.f[1] * scale;
		sp4b4[4].unk08 = sp608.f[2] * scale;
		sp4b4[0].unk0c = sp62c.f[0] * 0.1f;
		sp4b4[0].unk10 = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[1].unk0c = sp644.f[0] * 0.1f;
		sp4b4[1].unk10 = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[2].unk0c = sp638.f[0] * 0.1f;
		sp4b4[2].unk10 = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[3].unk0c = sp5f0.f[0] * 0.1f;
		sp4b4[3].unk10 = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[4].unk0c = sp608.f[0] * 0.1f;
		sp4b4[4].unk10 = sp608.f[2] * 0.1f + g_SkyCloudOffset;

		sky0f11f438(&sp4b4[0], sp584);
		sky0f11f438(&sp4b4[1], sp58c);
		sky0f11f438(&sp4b4[2], sp588);
		sky0f11f438(&sp4b4[3], sp570);
		sky0f11f438(&sp4b4[4], sp578);
		break;
	case 13:
		s1 = 5;
		sp4b4[0].unk00 = sp644.f[0] * scale;
		sp4b4[0].unk04 = sp644.f[1] * scale;
		sp4b4[0].unk08 = sp644.f[2] * scale;
		sp4b4[1].unk00 = sp638.f[0] * scale;
		sp4b4[1].unk04 = sp638.f[1] * scale;
		sp4b4[1].unk08 = sp638.f[2] * scale;
		sp4b4[2].unk00 = sp620.f[0] * scale;
		sp4b4[2].unk04 = sp620.f[1] * scale;
		sp4b4[2].unk08 = sp620.f[2] * scale;
		sp4b4[3].unk00 = sp608.f[0] * scale;
		sp4b4[3].unk04 = sp608.f[1] * scale;
		sp4b4[3].unk08 = sp608.f[2] * scale;
		sp4b4[4].unk00 = sp5fc.f[0] * scale;
		sp4b4[4].unk04 = sp5fc.f[1] * scale;
		sp4b4[4].unk08 = sp5fc.f[2] * scale;
		sp4b4[0].unk0c = sp644.f[0] * 0.1f;
		sp4b4[0].unk10 = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[1].unk0c = sp638.f[0] * 0.1f;
		sp4b4[1].unk10 = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[2].unk0c = sp620.f[0] * 0.1f;
		sp4b4[2].unk10 = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[3].unk0c = sp608.f[0] * 0.1f;
		sp4b4[3].unk10 = sp608.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[4].unk0c = sp5fc.f[0] * 0.1f;
		sp4b4[4].unk10 = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;

		sky0f11f438(&sp4b4[0], sp58c);
		sky0f11f438(&sp4b4[1], sp588);
		sky0f11f438(&sp4b4[2], sp580);
		sky0f11f438(&sp4b4[3], sp578);
		sky0f11f438(&sp4b4[4], sp574);
		break;
	case 11:
		s1 = 5;
		sp4b4[0].unk00 = sp620.f[0] * scale;
		sp4b4[0].unk04 = sp620.f[1] * scale;
		sp4b4[0].unk08 = sp620.f[2] * scale;
		sp4b4[1].unk00 = sp62c.f[0] * scale;
		sp4b4[1].unk04 = sp62c.f[1] * scale;
		sp4b4[1].unk08 = sp62c.f[2] * scale;
		sp4b4[2].unk00 = sp644.f[0] * scale;
		sp4b4[2].unk04 = sp644.f[1] * scale;
		sp4b4[2].unk08 = sp644.f[2] * scale;
		sp4b4[3].unk00 = sp614.f[0] * scale;
		sp4b4[3].unk04 = sp614.f[1] * scale;
		sp4b4[3].unk08 = sp614.f[2] * scale;
		sp4b4[4].unk00 = sp5f0.f[0] * scale;
		sp4b4[4].unk04 = sp5f0.f[1] * scale;
		sp4b4[4].unk08 = sp5f0.f[2] * scale;
		sp4b4[0].unk0c = sp620.f[0] * 0.1f;
		sp4b4[0].unk10 = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[1].unk0c = sp62c.f[0] * 0.1f;
		sp4b4[1].unk10 = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[2].unk0c = sp644.f[0] * 0.1f;
		sp4b4[2].unk10 = sp644.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[3].unk0c = sp614.f[0] * 0.1f;
		sp4b4[3].unk10 = sp614.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[4].unk0c = sp5f0.f[0] * 0.1f;
		sp4b4[4].unk10 = sp5f0.f[2] * 0.1f + g_SkyCloudOffset;

		sky0f11f438(&sp4b4[0], sp580);
		sky0f11f438(&sp4b4[1], sp584);
		sky0f11f438(&sp4b4[2], sp58c);
		sky0f11f438(&sp4b4[3], sp57c);
		sky0f11f438(&sp4b4[4], sp570);
		break;
	case 7:
		s1 = 5;
		sp4b4[0].unk00 = sp638.f[0] * scale;
		sp4b4[0].unk04 = sp638.f[1] * scale;
		sp4b4[0].unk08 = sp638.f[2] * scale;
		sp4b4[1].unk00 = sp620.f[0] * scale;
		sp4b4[1].unk04 = sp620.f[1] * scale;
		sp4b4[1].unk08 = sp620.f[2] * scale;
		sp4b4[2].unk00 = sp62c.f[0] * scale;
		sp4b4[2].unk04 = sp62c.f[1] * scale;
		sp4b4[2].unk08 = sp62c.f[2] * scale;
		sp4b4[3].unk00 = sp5fc.f[0] * scale;
		sp4b4[3].unk04 = sp5fc.f[1] * scale;
		sp4b4[3].unk08 = sp5fc.f[2] * scale;
		sp4b4[4].unk00 = sp614.f[0] * scale;
		sp4b4[4].unk04 = sp614.f[1] * scale;
		sp4b4[4].unk08 = sp614.f[2] * scale;
		sp4b4[0].unk0c = sp638.f[0] * 0.1f;
		sp4b4[0].unk10 = sp638.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[1].unk0c = sp620.f[0] * 0.1f;
		sp4b4[1].unk10 = sp620.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[2].unk0c = sp62c.f[0] * 0.1f;
		sp4b4[2].unk10 = sp62c.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[3].unk0c = sp5fc.f[0] * 0.1f;
		sp4b4[3].unk10 = sp5fc.f[2] * 0.1f + g_SkyCloudOffset;
		sp4b4[4].unk0c = sp614.f[0] * 0.1f;
		sp4b4[4].unk10 = sp614.f[2] * 0.1f + g_SkyCloudOffset;

		sky0f11f438(&sp4b4[0], sp588);
		sky0f11f438(&sp4b4[1], sp580);
		sky0f11f438(&sp4b4[2], sp584);
		sky0f11f438(&sp4b4[3], sp574);
		sky0f11f438(&sp4b4[4], sp57c);
		break;
	default:
		return gdl;
	}

	gDPPipeSync(gdl++);

	texSelect(&gdl, &g_TexWaterConfigs[env->unk18], 1, 0, 2, 1, NULL);

	if (1);

	gDPSetEnvColor(gdl++, env->sky_r, env->sky_g, env->sky_b, 0xff);
	gDPSetCombineLERP(gdl++,
			SHADE, ENVIRONMENT, TEXEL0, ENVIRONMENT, 0, 0, 0, SHADE,
			SHADE, ENVIRONMENT, TEXEL0, ENVIRONMENT, 0, 0, 0, SHADE);

	{
		s32 stack;
		Mtxf sp1ec;
		Mtxf sp1ac;
		struct skything38 sp94[5];
		s32 i;

		mtx4MultMtx4(camGetMtxF1754(), camGetWorldToScreenMtxf(), &sp1ec);
		guScaleF(var800a33a8.m, 1.0f / scale, 1.0f / scale, 1.0f / scale);
		mtx4MultMtx4(&sp1ec, &var800a33a8, &sp1ac);

		for (i = 0; i < s1; i++) {
			sky0f1228d0(&sp4b4[i], &sp1ac, 130, 65535.0f, 65535.0f, &sp94[i]);

			sp94[i].unk28 = skyClamp(sp94[i].unk28, camGetScreenLeft() * 4.0f, (camGetScreenLeft() + camGetScreenWidth()) * 4.0f - 1.0f);
			sp94[i].unk2c = skyClamp(sp94[i].unk2c, camGetScreenTop() * 4.0f, (camGetScreenTop() + camGetScreenHeight()) * 4.0f - 1.0f);
		}

		if (s1 == 4) {
			if (((sp538 << 3) | (sp534 << 2) | (sp530 << 1) | sp52c) == 12) {
				if (sp548 < sp54c) {
					if (sp94[3].unk2c >= sp94[1].unk2c + 4.0f) {
						sp94[0].unk28 = camGetScreenLeft() * 4.0f;
						sp94[0].unk2c = camGetScreenTop() * 4.0f;
						sp94[1].unk28 = (camGetScreenLeft() + camGetScreenWidth()) * 4.0f - 1.0f;
						sp94[1].unk2c = camGetScreenTop() * 4.0f;
						sp94[2].unk28 = camGetScreenLeft() * 4.0f;
						sp94[3].unk28 = (camGetScreenLeft() + camGetScreenWidth()) * 4.0f - 1.0f;

						gdl = sky0f123fd4(gdl, &sp94[0], &sp94[1], &sp94[2], &sp94[3], 130.0f);
					} else {
						gdl = sky0f122d4c(gdl, &sp94[0], &sp94[1], &sp94[2], 130.0f, 1);
					}
				} else if (sp94[2].unk2c >= sp94[0].unk2c + 4.0f) {
					sp94[0].unk28 = camGetScreenLeft() * 4.0f;
					sp94[0].unk2c = camGetScreenTop() * 4.0f;
					sp94[1].unk28 = (camGetScreenLeft() + camGetScreenWidth()) * 4.0f - 1.0f;
					sp94[1].unk2c = camGetScreenTop() * 4.0f;
					sp94[2].unk28 = camGetScreenLeft() * 4.0f;
					sp94[3].unk28 = (camGetScreenLeft() + camGetScreenWidth()) * 4.0f - 1.0f;

					gdl = sky0f123fd4(gdl, &sp94[1], &sp94[0], &sp94[3], &sp94[2], 130.0f);
				} else {
					gdl = sky0f122d4c(gdl, &sp94[1], &sp94[0], &sp94[3], 130.0f, 1);
				}
			} else {
				gdl = sky0f122d4c(gdl, &sp94[0], &sp94[1], &sp94[3], 130.0f, 1);
				gdl = sky0f122d4c(gdl, &sp94[3], &sp94[2], &sp94[0], 130.0f, 1);
			}
		} else if (s1 == 5) {
			gdl = sky0f122d4c(gdl, &sp94[0], &sp94[1], &sp94[2], 130.0f, 1);
			gdl = sky0f122d4c(gdl, &sp94[0], &sp94[2], &sp94[3], 130.0f, 1);
			gdl = sky0f122d4c(gdl, &sp94[0], &sp94[3], &sp94[4], 130.0f, 1);
		} else if (s1 == 3) {
			gdl = sky0f122d4c(gdl, &sp94[0], &sp94[1], &sp94[2], 130.0f, 1);
		}
	}

	return gdl;
}

GLOBAL_ASM(
glabel sky0f1228d0
.late_rodata
glabel var7f1b50cc
.word 0x46fffe00
glabel var7f1b50d0
.word 0x46fffe00
glabel var7f1b50d4
.word 0x43ff8000
glabel var7f1b50d8
.word 0xc57fa000
glabel var7f1b50dc
.word 0xc57fa000
.text
/*  f1228d0:	27bdff88 */ 	addiu	$sp,$sp,-120
/*  f1228d4:	30ceffff */ 	andi	$t6,$a2,0xffff
/*  f1228d8:	448e2000 */ 	mtc1	$t6,$f4
/*  f1228dc:	afb00028 */ 	sw	$s0,0x28($sp)
/*  f1228e0:	44877000 */ 	mtc1	$a3,$f14
/*  f1228e4:	00808025 */ 	or	$s0,$a0,$zero
/*  f1228e8:	afbf002c */ 	sw	$ra,0x2c($sp)
/*  f1228ec:	f7b60020 */ 	sdc1	$f22,0x20($sp)
/*  f1228f0:	f7b40018 */ 	sdc1	$f20,0x18($sp)
/*  f1228f4:	afa60080 */ 	sw	$a2,0x80($sp)
/*  f1228f8:	05c10005 */ 	bgez	$t6,.L0f122910
/*  f1228fc:	468021a0 */ 	cvt.s.w	$f6,$f4
/*  f122900:	3c014f80 */ 	lui	$at,0x4f80
/*  f122904:	44814000 */ 	mtc1	$at,$f8
/*  f122908:	00000000 */ 	nop
/*  f12290c:	46083180 */ 	add.s	$f6,$f6,$f8
.L0f122910:
/*  f122910:	3c014780 */ 	lui	$at,0x4780
/*  f122914:	44815000 */ 	mtc1	$at,$f10
/*  f122918:	c6120000 */ 	lwc1	$f18,0x0($s0)
/*  f12291c:	c4a40000 */ 	lwc1	$f4,0x0($a1)
/*  f122920:	460a3003 */ 	div.s	$f0,$f6,$f10
/*  f122924:	c4aa0010 */ 	lwc1	$f10,0x10($a1)
/*  f122928:	c6060004 */ 	lwc1	$f6,0x4($s0)
/*  f12292c:	3c013780 */ 	lui	$at,0x3780
/*  f122930:	46049202 */ 	mul.s	$f8,$f18,$f4
/*  f122934:	44816000 */ 	mtc1	$at,$f12
/*  f122938:	44808000 */ 	mtc1	$zero,$f16
/*  f12293c:	460a3482 */ 	mul.s	$f18,$f6,$f10
/*  f122940:	c4aa0020 */ 	lwc1	$f10,0x20($a1)
/*  f122944:	c6060008 */ 	lwc1	$f6,0x8($s0)
/*  f122948:	46124100 */ 	add.s	$f4,$f8,$f18
/*  f12294c:	460a3202 */ 	mul.s	$f8,$f6,$f10
/*  f122950:	c4a60030 */ 	lwc1	$f6,0x30($a1)
/*  f122954:	46000086 */ 	mov.s	$f2,$f0
/*  f122958:	46082480 */ 	add.s	$f18,$f4,$f8
/*  f12295c:	46123280 */ 	add.s	$f10,$f6,$f18
/*  f122960:	e7aa0068 */ 	swc1	$f10,0x68($sp)
/*  f122964:	c6040000 */ 	lwc1	$f4,0x0($s0)
/*  f122968:	c4a80004 */ 	lwc1	$f8,0x4($a1)
/*  f12296c:	c6120004 */ 	lwc1	$f18,0x4($s0)
/*  f122970:	c4aa0014 */ 	lwc1	$f10,0x14($a1)
/*  f122974:	46082182 */ 	mul.s	$f6,$f4,$f8
/*  f122978:	00000000 */ 	nop
/*  f12297c:	460a9102 */ 	mul.s	$f4,$f18,$f10
/*  f122980:	c4aa0024 */ 	lwc1	$f10,0x24($a1)
/*  f122984:	c6120008 */ 	lwc1	$f18,0x8($s0)
/*  f122988:	46043200 */ 	add.s	$f8,$f6,$f4
/*  f12298c:	460a9182 */ 	mul.s	$f6,$f18,$f10
/*  f122990:	c4b20034 */ 	lwc1	$f18,0x34($a1)
/*  f122994:	46064100 */ 	add.s	$f4,$f8,$f6
/*  f122998:	46049280 */ 	add.s	$f10,$f18,$f4
/*  f12299c:	e7aa006c */ 	swc1	$f10,0x6c($sp)
/*  f1229a0:	c6080000 */ 	lwc1	$f8,0x0($s0)
/*  f1229a4:	c4a60008 */ 	lwc1	$f6,0x8($a1)
/*  f1229a8:	c6040004 */ 	lwc1	$f4,0x4($s0)
/*  f1229ac:	c4aa0018 */ 	lwc1	$f10,0x18($a1)
/*  f1229b0:	46064482 */ 	mul.s	$f18,$f8,$f6
/*  f1229b4:	00000000 */ 	nop
/*  f1229b8:	460a2202 */ 	mul.s	$f8,$f4,$f10
/*  f1229bc:	c4aa0028 */ 	lwc1	$f10,0x28($a1)
/*  f1229c0:	c6040008 */ 	lwc1	$f4,0x8($s0)
/*  f1229c4:	46089180 */ 	add.s	$f6,$f18,$f8
/*  f1229c8:	460a2482 */ 	mul.s	$f18,$f4,$f10
/*  f1229cc:	c4a40038 */ 	lwc1	$f4,0x38($a1)
/*  f1229d0:	46123200 */ 	add.s	$f8,$f6,$f18
/*  f1229d4:	46082280 */ 	add.s	$f10,$f4,$f8
/*  f1229d8:	e7aa0070 */ 	swc1	$f10,0x70($sp)
/*  f1229dc:	c4b2000c */ 	lwc1	$f18,0xc($a1)
/*  f1229e0:	c6060000 */ 	lwc1	$f6,0x0($s0)
/*  f1229e4:	c4aa001c */ 	lwc1	$f10,0x1c($a1)
/*  f1229e8:	c6080004 */ 	lwc1	$f8,0x4($s0)
/*  f1229ec:	46123102 */ 	mul.s	$f4,$f6,$f18
/*  f1229f0:	00000000 */ 	nop
/*  f1229f4:	460a4182 */ 	mul.s	$f6,$f8,$f10
/*  f1229f8:	c4aa002c */ 	lwc1	$f10,0x2c($a1)
/*  f1229fc:	c6080008 */ 	lwc1	$f8,0x8($s0)
/*  f122a00:	46062480 */ 	add.s	$f18,$f4,$f6
/*  f122a04:	460a4102 */ 	mul.s	$f4,$f8,$f10
/*  f122a08:	c4a8003c */ 	lwc1	$f8,0x3c($a1)
/*  f122a0c:	46049180 */ 	add.s	$f6,$f18,$f4
/*  f122a10:	460c7102 */ 	mul.s	$f4,$f14,$f12
/*  f122a14:	46064280 */ 	add.s	$f10,$f8,$f6
/*  f122a18:	e7aa0074 */ 	swc1	$f10,0x74($sp)
/*  f122a1c:	c612000c */ 	lwc1	$f18,0xc($s0)
/*  f122a20:	c7aa0088 */ 	lwc1	$f10,0x88($sp)
/*  f122a24:	46049202 */ 	mul.s	$f8,$f18,$f4
/*  f122a28:	e7a80060 */ 	swc1	$f8,0x60($sp)
/*  f122a2c:	460c5482 */ 	mul.s	$f18,$f10,$f12
/*  f122a30:	c6060010 */ 	lwc1	$f6,0x10($s0)
/*  f122a34:	c7a80074 */ 	lwc1	$f8,0x74($sp)
/*  f122a38:	46088032 */ 	c.eq.s	$f16,$f8
/*  f122a3c:	46123102 */ 	mul.s	$f4,$f6,$f18
/*  f122a40:	e7a40064 */ 	swc1	$f4,0x64($sp)
/*  f122a44:	45000004 */ 	bc1f	.L0f122a58
/*  f122a48:	c7a40068 */ 	lwc1	$f4,0x68($sp)
/*  f122a4c:	3c017f1b */ 	lui	$at,%hi(var7f1b50cc)
/*  f122a50:	10000006 */ 	b	.L0f122a6c
/*  f122a54:	c43650cc */ 	lwc1	$f22,%lo(var7f1b50cc)($at)
.L0f122a58:
/*  f122a58:	c7a60074 */ 	lwc1	$f6,0x74($sp)
/*  f122a5c:	3c013f80 */ 	lui	$at,0x3f80
/*  f122a60:	44815000 */ 	mtc1	$at,$f10
/*  f122a64:	46003482 */ 	mul.s	$f18,$f6,$f0
/*  f122a68:	46125583 */ 	div.s	$f22,$f10,$f18
.L0f122a6c:
/*  f122a6c:	4610b03c */ 	c.lt.s	$f22,$f16
/*  f122a70:	4600b006 */ 	mov.s	$f0,$f22
/*  f122a74:	45000002 */ 	bc1f	.L0f122a80
/*  f122a78:	3c017f1b */ 	lui	$at,%hi(var7f1b50d0)
/*  f122a7c:	c42050d0 */ 	lwc1	$f0,%lo(var7f1b50d0)($at)
.L0f122a80:
/*  f122a80:	46002202 */ 	mul.s	$f8,$f4,$f0
/*  f122a84:	c7aa006c */ 	lwc1	$f10,0x6c($sp)
/*  f122a88:	46024182 */ 	mul.s	$f6,$f8,$f2
/*  f122a8c:	c7a80070 */ 	lwc1	$f8,0x70($sp)
/*  f122a90:	46005482 */ 	mul.s	$f18,$f10,$f0
/*  f122a94:	e7a60048 */ 	swc1	$f6,0x48($sp)
/*  f122a98:	46029102 */ 	mul.s	$f4,$f18,$f2
/*  f122a9c:	c7b20074 */ 	lwc1	$f18,0x74($sp)
/*  f122aa0:	46004182 */ 	mul.s	$f6,$f8,$f0
/*  f122aa4:	e7a4004c */ 	swc1	$f4,0x4c($sp)
/*  f122aa8:	46023282 */ 	mul.s	$f10,$f6,$f2
/*  f122aac:	00000000 */ 	nop
/*  f122ab0:	46009102 */ 	mul.s	$f4,$f18,$f0
/*  f122ab4:	e7aa0050 */ 	swc1	$f10,0x50($sp)
/*  f122ab8:	46022202 */ 	mul.s	$f8,$f4,$f2
/*  f122abc:	0fc2d5f6 */ 	jal	camGetScreenWidth
/*  f122ac0:	e7a80054 */ 	swc1	$f8,0x54($sp)
/*  f122ac4:	0fc2d5f6 */ 	jal	camGetScreenWidth
/*  f122ac8:	46000506 */ 	mov.s	$f20,$f0
/*  f122acc:	0fc2d5fe */ 	jal	camGetScreenLeft
/*  f122ad0:	e7a00030 */ 	swc1	$f0,0x30($sp)
/*  f122ad4:	3c014080 */ 	lui	$at,0x4080
/*  f122ad8:	44813000 */ 	mtc1	$at,$f6
/*  f122adc:	c7a20030 */ 	lwc1	$f2,0x30($sp)
/*  f122ae0:	c7a80048 */ 	lwc1	$f8,0x48($sp)
/*  f122ae4:	46060282 */ 	mul.s	$f10,$f0,$f6
/*  f122ae8:	46021480 */ 	add.s	$f18,$f2,$f2
/*  f122aec:	4614a180 */ 	add.s	$f6,$f20,$f20
/*  f122af0:	46125100 */ 	add.s	$f4,$f10,$f18
/*  f122af4:	46064282 */ 	mul.s	$f10,$f8,$f6
/*  f122af8:	460a2480 */ 	add.s	$f18,$f4,$f10
/*  f122afc:	0fc2d5fa */ 	jal	camGetScreenHeight
/*  f122b00:	e7b20038 */ 	swc1	$f18,0x38($sp)
/*  f122b04:	0fc2d5fa */ 	jal	camGetScreenHeight
/*  f122b08:	46000506 */ 	mov.s	$f20,$f0
/*  f122b0c:	0fc2d602 */ 	jal	camGetScreenTop
/*  f122b10:	e7a00030 */ 	swc1	$f0,0x30($sp)
/*  f122b14:	3c017f1b */ 	lui	$at,%hi(var7f1b50d4)
/*  f122b18:	c43050d4 */ 	lwc1	$f16,%lo(var7f1b50d4)($at)
/*  f122b1c:	3c014080 */ 	lui	$at,0x4080
/*  f122b20:	44814000 */ 	mtc1	$at,$f8
/*  f122b24:	c7a20030 */ 	lwc1	$f2,0x30($sp)
/*  f122b28:	c7b2004c */ 	lwc1	$f18,0x4c($sp)
/*  f122b2c:	46080182 */ 	mul.s	$f6,$f0,$f8
/*  f122b30:	46021100 */ 	add.s	$f4,$f2,$f2
/*  f122b34:	3c017f1b */ 	lui	$at,%hi(var7f1b50d8)
/*  f122b38:	3c06457f */ 	lui	$a2,0x457f
/*  f122b3c:	46009207 */ 	neg.s	$f8,$f18
/*  f122b40:	34c6a000 */ 	ori	$a2,$a2,0xa000
/*  f122b44:	c42e50d8 */ 	lwc1	$f14,%lo(var7f1b50d8)($at)
/*  f122b48:	46043280 */ 	add.s	$f10,$f6,$f4
/*  f122b4c:	c7ac0038 */ 	lwc1	$f12,0x38($sp)
/*  f122b50:	4614a180 */ 	add.s	$f6,$f20,$f20
/*  f122b54:	46064102 */ 	mul.s	$f4,$f8,$f6
/*  f122b58:	c7a80050 */ 	lwc1	$f8,0x50($sp)
/*  f122b5c:	46104182 */ 	mul.s	$f6,$f8,$f16
/*  f122b60:	46045480 */ 	add.s	$f18,$f10,$f4
/*  f122b64:	c7a40054 */ 	lwc1	$f4,0x54($sp)
/*  f122b68:	46103280 */ 	add.s	$f10,$f6,$f16
/*  f122b6c:	e7b2003c */ 	swc1	$f18,0x3c($sp)
/*  f122b70:	44809000 */ 	mtc1	$zero,$f18
/*  f122b74:	e7aa0040 */ 	swc1	$f10,0x40($sp)
/*  f122b78:	46122202 */ 	mul.s	$f8,$f4,$f18
/*  f122b7c:	0fc47cf4 */ 	jal	skyClamp
/*  f122b80:	e7a80044 */ 	swc1	$f8,0x44($sp)
/*  f122b84:	3c017f1b */ 	lui	$at,%hi(var7f1b50dc)
/*  f122b88:	3c06457f */ 	lui	$a2,0x457f
/*  f122b8c:	e7a00038 */ 	swc1	$f0,0x38($sp)
/*  f122b90:	34c6a000 */ 	ori	$a2,$a2,0xa000
/*  f122b94:	c42e50dc */ 	lwc1	$f14,%lo(var7f1b50dc)($at)
/*  f122b98:	0fc47cf4 */ 	jal	skyClamp
/*  f122b9c:	c7ac003c */ 	lwc1	$f12,0x3c($sp)
/*  f122ba0:	44807000 */ 	mtc1	$zero,$f14
/*  f122ba4:	3c0646ff */ 	lui	$a2,0x46ff
/*  f122ba8:	e7a0003c */ 	swc1	$f0,0x3c($sp)
/*  f122bac:	34c6fe00 */ 	ori	$a2,$a2,0xfe00
/*  f122bb0:	0fc47cf4 */ 	jal	skyClamp
/*  f122bb4:	c7ac0040 */ 	lwc1	$f12,0x40($sp)
/*  f122bb8:	44807000 */ 	mtc1	$zero,$f14
/*  f122bbc:	3c0646ff */ 	lui	$a2,0x46ff
/*  f122bc0:	e7a00040 */ 	swc1	$f0,0x40($sp)
/*  f122bc4:	34c6fe00 */ 	ori	$a2,$a2,0xfe00
/*  f122bc8:	0fc47cf4 */ 	jal	skyClamp
/*  f122bcc:	c7ac0044 */ 	lwc1	$f12,0x44($sp)
/*  f122bd0:	8fa3008c */ 	lw	$v1,0x8c($sp)
/*  f122bd4:	c7a60068 */ 	lwc1	$f6,0x68($sp)
/*  f122bd8:	e7a00044 */ 	swc1	$f0,0x44($sp)
/*  f122bdc:	e4660000 */ 	swc1	$f6,0x0($v1)
/*  f122be0:	c7aa006c */ 	lwc1	$f10,0x6c($sp)
/*  f122be4:	e46a0004 */ 	swc1	$f10,0x4($v1)
/*  f122be8:	c7a40070 */ 	lwc1	$f4,0x70($sp)
/*  f122bec:	e4640008 */ 	swc1	$f4,0x8($v1)
/*  f122bf0:	c7b20074 */ 	lwc1	$f18,0x74($sp)
/*  f122bf4:	e472000c */ 	swc1	$f18,0xc($v1)
/*  f122bf8:	c7a80060 */ 	lwc1	$f8,0x60($sp)
/*  f122bfc:	e4680020 */ 	swc1	$f8,0x20($v1)
/*  f122c00:	c7a60064 */ 	lwc1	$f6,0x64($sp)
/*  f122c04:	e4660024 */ 	swc1	$f6,0x24($v1)
/*  f122c08:	c7aa0038 */ 	lwc1	$f10,0x38($sp)
/*  f122c0c:	0fc595f3 */ 	jal	envGetCurrent
/*  f122c10:	e46a0028 */ 	swc1	$f10,0x28($v1)
/*  f122c14:	3c014080 */ 	lui	$at,0x4080
/*  f122c18:	44819000 */ 	mtc1	$at,$f18
/*  f122c1c:	c4440040 */ 	lwc1	$f4,0x40($v0)
/*  f122c20:	c7a6003c */ 	lwc1	$f6,0x3c($sp)
/*  f122c24:	8fa3008c */ 	lw	$v1,0x8c($sp)
/*  f122c28:	46122202 */ 	mul.s	$f8,$f4,$f18
/*  f122c2c:	3c014f80 */ 	lui	$at,0x4f80
/*  f122c30:	46083281 */ 	sub.s	$f10,$f6,$f8
/*  f122c34:	e46a002c */ 	swc1	$f10,0x2c($v1)
/*  f122c38:	c7a40040 */ 	lwc1	$f4,0x40($sp)
/*  f122c3c:	e4760034 */ 	swc1	$f22,0x34($v1)
/*  f122c40:	e4640030 */ 	swc1	$f4,0x30($v1)
/*  f122c44:	920f0014 */ 	lbu	$t7,0x14($s0)
/*  f122c48:	448f9000 */ 	mtc1	$t7,$f18
/*  f122c4c:	05e10004 */ 	bgez	$t7,.L0f122c60
/*  f122c50:	468091a0 */ 	cvt.s.w	$f6,$f18
/*  f122c54:	44814000 */ 	mtc1	$at,$f8
/*  f122c58:	00000000 */ 	nop
/*  f122c5c:	46083180 */ 	add.s	$f6,$f6,$f8
.L0f122c60:
/*  f122c60:	e4660010 */ 	swc1	$f6,0x10($v1)
/*  f122c64:	92180015 */ 	lbu	$t8,0x15($s0)
/*  f122c68:	3c014f80 */ 	lui	$at,0x4f80
/*  f122c6c:	44985000 */ 	mtc1	$t8,$f10
/*  f122c70:	07010004 */ 	bgez	$t8,.L0f122c84
/*  f122c74:	46805120 */ 	cvt.s.w	$f4,$f10
/*  f122c78:	44819000 */ 	mtc1	$at,$f18
/*  f122c7c:	00000000 */ 	nop
/*  f122c80:	46122100 */ 	add.s	$f4,$f4,$f18
.L0f122c84:
/*  f122c84:	e4640014 */ 	swc1	$f4,0x14($v1)
/*  f122c88:	92190016 */ 	lbu	$t9,0x16($s0)
/*  f122c8c:	3c014f80 */ 	lui	$at,0x4f80
/*  f122c90:	44994000 */ 	mtc1	$t9,$f8
/*  f122c94:	07210004 */ 	bgez	$t9,.L0f122ca8
/*  f122c98:	468041a0 */ 	cvt.s.w	$f6,$f8
/*  f122c9c:	44815000 */ 	mtc1	$at,$f10
/*  f122ca0:	00000000 */ 	nop
/*  f122ca4:	460a3180 */ 	add.s	$f6,$f6,$f10
.L0f122ca8:
/*  f122ca8:	e4660018 */ 	swc1	$f6,0x18($v1)
/*  f122cac:	92080017 */ 	lbu	$t0,0x17($s0)
/*  f122cb0:	3c014f80 */ 	lui	$at,0x4f80
/*  f122cb4:	44889000 */ 	mtc1	$t0,$f18
/*  f122cb8:	05010004 */ 	bgez	$t0,.L0f122ccc
/*  f122cbc:	46809120 */ 	cvt.s.w	$f4,$f18
/*  f122cc0:	44814000 */ 	mtc1	$at,$f8
/*  f122cc4:	00000000 */ 	nop
/*  f122cc8:	46082100 */ 	add.s	$f4,$f4,$f8
.L0f122ccc:
/*  f122ccc:	e464001c */ 	swc1	$f4,0x1c($v1)
/*  f122cd0:	8fbf002c */ 	lw	$ra,0x2c($sp)
/*  f122cd4:	8fb00028 */ 	lw	$s0,0x28($sp)
/*  f122cd8:	d7b60020 */ 	ldc1	$f22,0x20($sp)
/*  f122cdc:	d7b40018 */ 	ldc1	$f20,0x18($sp)
/*  f122ce0:	03e00008 */ 	jr	$ra
/*  f122ce4:	27bd0078 */ 	addiu	$sp,$sp,0x78
);

GLOBAL_ASM(
glabel sky0f122ce8
/*  f122ce8:	27bdffe8 */ 	addiu	$sp,$sp,-24
/*  f122cec:	afbf0014 */ 	sw	$ra,0x14($sp)
/*  f122cf0:	c4a60028 */ 	lwc1	$f6,0x28($a1)
/*  f122cf4:	c4840028 */ 	lwc1	$f4,0x28($a0)
/*  f122cf8:	c4aa002c */ 	lwc1	$f10,0x2c($a1)
/*  f122cfc:	c488002c */ 	lwc1	$f8,0x2c($a0)
/*  f122d00:	46062001 */ 	sub.s	$f0,$f4,$f6
/*  f122d04:	460a4081 */ 	sub.s	$f2,$f8,$f10
/*  f122d08:	46000402 */ 	mul.s	$f16,$f0,$f0
/*  f122d0c:	00000000 */ 	nop
/*  f122d10:	46021482 */ 	mul.s	$f18,$f2,$f2
/*  f122d14:	0c012974 */ 	jal	sqrtf
/*  f122d18:	46128300 */ 	add.s	$f12,$f16,$f18
/*  f122d1c:	3c013f80 */ 	lui	$at,0x3f80
/*  f122d20:	44812000 */ 	mtc1	$at,$f4
/*  f122d24:	8fbf0014 */ 	lw	$ra,0x14($sp)
/*  f122d28:	27bd0018 */ 	addiu	$sp,$sp,0x18
/*  f122d2c:	4604003c */ 	c.lt.s	$f0,$f4
/*  f122d30:	00001825 */ 	or	$v1,$zero,$zero
/*  f122d34:	45000003 */ 	bc1f	.L0f122d44
/*  f122d38:	00000000 */ 	nop
/*  f122d3c:	10000001 */ 	b	.L0f122d44
/*  f122d40:	24030001 */ 	addiu	$v1,$zero,0x1
.L0f122d44:
/*  f122d44:	03e00008 */ 	jr	$ra
/*  f122d48:	00601025 */ 	or	$v0,$v1,$zero
);

GLOBAL_ASM(
glabel sky0f122d4c
.late_rodata
glabel var7f1b50e0
.word 0xc4eac000
glabel var7f1b50e4
.word 0xc4eac000
glabel var7f1b50e8
.word 0xc4eac000
glabel var7f1b50ec
.word 0x46fffe00
.text
/*  f122d4c:	27bdfb78 */ 	addiu	$sp,$sp,-1160
/*  f122d50:	afb00020 */ 	sw	$s0,0x20($sp)
/*  f122d54:	00808025 */ 	or	$s0,$a0,$zero
/*  f122d58:	afbf0024 */ 	sw	$ra,0x24($sp)
/*  f122d5c:	afa5048c */ 	sw	$a1,0x48c($sp)
/*  f122d60:	00a02025 */ 	or	$a0,$a1,$zero
/*  f122d64:	f7b40018 */ 	sdc1	$f20,0x18($sp)
/*  f122d68:	00c02825 */ 	or	$a1,$a2,$zero
/*  f122d6c:	afa60490 */ 	sw	$a2,0x490($sp)
/*  f122d70:	0fc48b3a */ 	jal	sky0f122ce8
/*  f122d74:	afa70494 */ 	sw	$a3,0x494($sp)
/*  f122d78:	8fa60490 */ 	lw	$a2,0x490($sp)
/*  f122d7c:	14400013 */ 	bnez	$v0,.L0f122dcc
/*  f122d80:	8fa70494 */ 	lw	$a3,0x494($sp)
/*  f122d84:	00c02025 */ 	or	$a0,$a2,$zero
/*  f122d88:	00e02825 */ 	or	$a1,$a3,$zero
/*  f122d8c:	afa60490 */ 	sw	$a2,0x490($sp)
/*  f122d90:	0fc48b3a */ 	jal	sky0f122ce8
/*  f122d94:	afa70494 */ 	sw	$a3,0x494($sp)
/*  f122d98:	8faf048c */ 	lw	$t7,0x48c($sp)
/*  f122d9c:	8fa60490 */ 	lw	$a2,0x490($sp)
/*  f122da0:	1440000a */ 	bnez	$v0,.L0f122dcc
/*  f122da4:	8fa70494 */ 	lw	$a3,0x494($sp)
/*  f122da8:	00e02025 */ 	or	$a0,$a3,$zero
/*  f122dac:	01e02825 */ 	or	$a1,$t7,$zero
/*  f122db0:	afa60490 */ 	sw	$a2,0x490($sp)
/*  f122db4:	0fc48b3a */ 	jal	sky0f122ce8
/*  f122db8:	afa70494 */ 	sw	$a3,0x494($sp)
/*  f122dbc:	8fa3048c */ 	lw	$v1,0x48c($sp)
/*  f122dc0:	8fa60490 */ 	lw	$a2,0x490($sp)
/*  f122dc4:	10400003 */ 	beqz	$v0,.L0f122dd4
/*  f122dc8:	8fa70494 */ 	lw	$a3,0x494($sp)
.L0f122dcc:
/*  f122dcc:	1000047c */ 	b	.L0f123fc0
/*  f122dd0:	02001025 */ 	or	$v0,$s0,$zero
.L0f122dd4:
/*  f122dd4:	3c014780 */ 	lui	$at,0x4780
/*  f122dd8:	44813000 */ 	mtc1	$at,$f6
/*  f122ddc:	c7a40498 */ 	lwc1	$f4,0x498($sp)
/*  f122de0:	3c013780 */ 	lui	$at,0x3780
/*  f122de4:	4480a000 */ 	mtc1	$zero,$f20
/*  f122de8:	46062203 */ 	div.s	$f8,$f4,$f6
/*  f122dec:	e7a80378 */ 	swc1	$f8,0x378($sp)
/*  f122df0:	c4c4002c */ 	lwc1	$f4,0x2c($a2)
/*  f122df4:	c4ca0028 */ 	lwc1	$f10,0x28($a2)
/*  f122df8:	c4600028 */ 	lwc1	$f0,0x28($v1)
/*  f122dfc:	e7a4005c */ 	swc1	$f4,0x5c($sp)
/*  f122e00:	c4e80028 */ 	lwc1	$f8,0x28($a3)
/*  f122e04:	46005301 */ 	sub.s	$f12,$f10,$f0
/*  f122e08:	c46e002c */ 	lwc1	$f14,0x2c($v1)
/*  f122e0c:	c7a6005c */ 	lwc1	$f6,0x5c($sp)
/*  f122e10:	46004281 */ 	sub.s	$f10,$f8,$f0
/*  f122e14:	460e3401 */ 	sub.s	$f16,$f6,$f14
/*  f122e18:	e7aa046c */ 	swc1	$f10,0x46c($sp)
/*  f122e1c:	c4e4002c */ 	lwc1	$f4,0x2c($a3)
/*  f122e20:	c7a8046c */ 	lwc1	$f8,0x46c($sp)
/*  f122e24:	e7a40054 */ 	swc1	$f4,0x54($sp)
/*  f122e28:	c7a60054 */ 	lwc1	$f6,0x54($sp)
/*  f122e2c:	46104282 */ 	mul.s	$f10,$f8,$f16
/*  f122e30:	44814000 */ 	mtc1	$at,$f8
/*  f122e34:	460e3481 */ 	sub.s	$f18,$f6,$f14
/*  f122e38:	46126102 */ 	mul.s	$f4,$f12,$f18
/*  f122e3c:	46045181 */ 	sub.s	$f6,$f10,$f4
/*  f122e40:	c7a4005c */ 	lwc1	$f4,0x5c($sp)
/*  f122e44:	46083082 */ 	mul.s	$f2,$f6,$f8
/*  f122e48:	4602a032 */ 	c.eq.s	$f20,$f2
/*  f122e4c:	e7a20444 */ 	swc1	$f2,0x444($sp)
/*  f122e50:	45020004 */ 	bc1fl	.L0f122e64
/*  f122e54:	460e203c */ 	c.lt.s	$f4,$f14
/*  f122e58:	10000459 */ 	b	.L0f123fc0
/*  f122e5c:	02001025 */ 	or	$v0,$s0,$zero
/*  f122e60:	460e203c */ 	c.lt.s	$f4,$f14
.L0f122e64:
/*  f122e64:	3c013f80 */ 	lui	$at,0x3f80
/*  f122e68:	44815000 */ 	mtc1	$at,$f10
/*  f122e6c:	00604825 */ 	or	$t1,$v1,$zero
/*  f122e70:	00c04025 */ 	or	$t0,$a2,$zero
/*  f122e74:	00e05825 */ 	or	$t3,$a3,$zero
/*  f122e78:	45000009 */ 	bc1f	.L0f122ea0
/*  f122e7c:	46025403 */ 	div.s	$f16,$f10,$f2
/*  f122e80:	3c01bf80 */ 	lui	$at,0xbf80
/*  f122e84:	44816000 */ 	mtc1	$at,$f12
/*  f122e88:	00604025 */ 	or	$t0,$v1,$zero
/*  f122e8c:	00c04825 */ 	or	$t1,$a2,$zero
/*  f122e90:	460c1382 */ 	mul.s	$f14,$f2,$f12
/*  f122e94:	e7ae0444 */ 	swc1	$f14,0x444($sp)
/*  f122e98:	460c8402 */ 	mul.s	$f16,$f16,$f12
/*  f122e9c:	00000000 */ 	nop
.L0f122ea0:
/*  f122ea0:	c500002c */ 	lwc1	$f0,0x2c($t0)
/*  f122ea4:	c7a60054 */ 	lwc1	$f6,0x54($sp)
/*  f122ea8:	3c01bf80 */ 	lui	$at,0xbf80
/*  f122eac:	44816000 */ 	mtc1	$at,$f12
/*  f122eb0:	4600303c */ 	c.lt.s	$f6,$f0
/*  f122eb4:	c7ae0444 */ 	lwc1	$f14,0x444($sp)
/*  f122eb8:	3c013e80 */ 	lui	$at,0x3e80
/*  f122ebc:	45020007 */ 	bc1fl	.L0f122edc
/*  f122ec0:	e7ae0444 */ 	swc1	$f14,0x444($sp)
/*  f122ec4:	460c7382 */ 	mul.s	$f14,$f14,$f12
/*  f122ec8:	01005825 */ 	or	$t3,$t0,$zero
/*  f122ecc:	00e04025 */ 	or	$t0,$a3,$zero
/*  f122ed0:	460c8402 */ 	mul.s	$f16,$f16,$f12
/*  f122ed4:	c4e0002c */ 	lwc1	$f0,0x2c($a3)
/*  f122ed8:	e7ae0444 */ 	swc1	$f14,0x444($sp)
.L0f122edc:
/*  f122edc:	e7b00440 */ 	swc1	$f16,0x440($sp)
/*  f122ee0:	c528002c */ 	lwc1	$f8,0x2c($t1)
/*  f122ee4:	4608003c */ 	c.lt.s	$f0,$f8
/*  f122ee8:	44810000 */ 	mtc1	$at,$f0
/*  f122eec:	3c014080 */ 	lui	$at,0x4080
/*  f122ef0:	44811000 */ 	mtc1	$at,$f2
/*  f122ef4:	45000008 */ 	bc1f	.L0f122f18
/*  f122ef8:	3c013e80 */ 	lui	$at,0x3e80
/*  f122efc:	460c7382 */ 	mul.s	$f14,$f14,$f12
/*  f122f00:	01001025 */ 	or	$v0,$t0,$zero
/*  f122f04:	01204025 */ 	or	$t0,$t1,$zero
/*  f122f08:	460c8402 */ 	mul.s	$f16,$f16,$f12
/*  f122f0c:	00404825 */ 	or	$t1,$v0,$zero
/*  f122f10:	e7ae0444 */ 	swc1	$f14,0x444($sp)
/*  f122f14:	e7b00440 */ 	swc1	$f16,0x440($sp)
.L0f122f18:
/*  f122f18:	c50a0028 */ 	lwc1	$f10,0x28($t0)
/*  f122f1c:	44812000 */ 	mtc1	$at,$f4
/*  f122f20:	e7b40424 */ 	swc1	$f20,0x424($sp)
/*  f122f24:	3c0644ea */ 	lui	$a2,0x44ea
/*  f122f28:	46045182 */ 	mul.s	$f6,$f10,$f4
/*  f122f2c:	44815000 */ 	mtc1	$at,$f10
/*  f122f30:	34c6a000 */ 	ori	$a2,$a2,0xa000
/*  f122f34:	e7a60420 */ 	swc1	$f6,0x420($sp)
/*  f122f38:	c5280028 */ 	lwc1	$f8,0x28($t1)
/*  f122f3c:	e7b4042c */ 	swc1	$f20,0x42c($sp)
/*  f122f40:	460a4102 */ 	mul.s	$f4,$f8,$f10
/*  f122f44:	44814000 */ 	mtc1	$at,$f8
/*  f122f48:	3c017f1b */ 	lui	$at,%hi(var7f1b50e0)
/*  f122f4c:	e7a40428 */ 	swc1	$f4,0x428($sp)
/*  f122f50:	c5260028 */ 	lwc1	$f6,0x28($t1)
/*  f122f54:	e7b40434 */ 	swc1	$f20,0x434($sp)
/*  f122f58:	e7b4043c */ 	swc1	$f20,0x43c($sp)
/*  f122f5c:	46083282 */ 	mul.s	$f10,$f6,$f8
/*  f122f60:	e7b40438 */ 	swc1	$f20,0x438($sp)
/*  f122f64:	e7aa0430 */ 	swc1	$f10,0x430($sp)
/*  f122f68:	c5640028 */ 	lwc1	$f4,0x28($t3)
/*  f122f6c:	e7a40448 */ 	swc1	$f4,0x448($sp)
/*  f122f70:	c566002c */ 	lwc1	$f6,0x2c($t3)
/*  f122f74:	e7a6044c */ 	swc1	$f6,0x44c($sp)
/*  f122f78:	c5080028 */ 	lwc1	$f8,0x28($t0)
/*  f122f7c:	e7a80450 */ 	swc1	$f8,0x450($sp)
/*  f122f80:	c50a002c */ 	lwc1	$f10,0x2c($t0)
/*  f122f84:	c7a80450 */ 	lwc1	$f8,0x450($sp)
/*  f122f88:	e7aa0454 */ 	swc1	$f10,0x454($sp)
/*  f122f8c:	c5240028 */ 	lwc1	$f4,0x28($t1)
/*  f122f90:	e7a40458 */ 	swc1	$f4,0x458($sp)
/*  f122f94:	c7aa0458 */ 	lwc1	$f10,0x458($sp)
/*  f122f98:	c526002c */ 	lwc1	$f6,0x2c($t1)
/*  f122f9c:	e7a80028 */ 	swc1	$f8,0x28($sp)
/*  f122fa0:	460a4101 */ 	sub.s	$f4,$f8,$f10
/*  f122fa4:	e7a6045c */ 	swc1	$f6,0x45c($sp)
/*  f122fa8:	c7a60454 */ 	lwc1	$f6,0x454($sp)
/*  f122fac:	c7a8045c */ 	lwc1	$f8,0x45c($sp)
/*  f122fb0:	e7a40474 */ 	swc1	$f4,0x474($sp)
/*  f122fb4:	e7a40030 */ 	swc1	$f4,0x30($sp)
/*  f122fb8:	e7aa002c */ 	swc1	$f10,0x2c($sp)
/*  f122fbc:	c7a4002c */ 	lwc1	$f4,0x2c($sp)
/*  f122fc0:	46083401 */ 	sub.s	$f16,$f6,$f8
/*  f122fc4:	e7a6002c */ 	swc1	$f6,0x2c($sp)
/*  f122fc8:	c7a6044c */ 	lwc1	$f6,0x44c($sp)
/*  f122fcc:	c7aa0448 */ 	lwc1	$f10,0x448($sp)
/*  f122fd0:	e7b403fc */ 	swc1	$f20,0x3fc($sp)
/*  f122fd4:	46083481 */ 	sub.s	$f18,$f6,$f8
/*  f122fd8:	c7a80028 */ 	lwc1	$f8,0x28($sp)
/*  f122fdc:	e7b403f8 */ 	swc1	$f20,0x3f8($sp)
/*  f122fe0:	46045101 */ 	sub.s	$f4,$f10,$f4
/*  f122fe4:	e7b403dc */ 	swc1	$f20,0x3dc($sp)
/*  f122fe8:	e7b403d8 */ 	swc1	$f20,0x3d8($sp)
/*  f122fec:	46085301 */ 	sub.s	$f12,$f10,$f8
/*  f122ff0:	c7aa002c */ 	lwc1	$f10,0x2c($sp)
/*  f122ff4:	e7a4046c */ 	swc1	$f4,0x46c($sp)
/*  f122ff8:	e7b403a0 */ 	swc1	$f20,0x3a0($sp)
/*  f122ffc:	46006202 */ 	mul.s	$f8,$f12,$f0
/*  f123000:	460a3381 */ 	sub.s	$f14,$f6,$f10
/*  f123004:	c7aa0030 */ 	lwc1	$f10,0x30($sp)
/*  f123008:	e7ac0464 */ 	swc1	$f12,0x464($sp)
/*  f12300c:	e7b403a8 */ 	swc1	$f20,0x3a8($sp)
/*  f123010:	46007182 */ 	mul.s	$f6,$f14,$f0
/*  f123014:	e7ae0460 */ 	swc1	$f14,0x460($sp)
/*  f123018:	e7a803e0 */ 	swc1	$f8,0x3e0($sp)
/*  f12301c:	46005202 */ 	mul.s	$f8,$f10,$f0
/*  f123020:	e7b403b0 */ 	swc1	$f20,0x3b0($sp)
/*  f123024:	e7b403bc */ 	swc1	$f20,0x3bc($sp)
/*  f123028:	e7b403b8 */ 	swc1	$f20,0x3b8($sp)
/*  f12302c:	e7a603e4 */ 	swc1	$f6,0x3e4($sp)
/*  f123030:	46008182 */ 	mul.s	$f6,$f16,$f0
/*  f123034:	e7b40380 */ 	swc1	$f20,0x380($sp)
/*  f123038:	e7a803e8 */ 	swc1	$f8,0x3e8($sp)
/*  f12303c:	46002202 */ 	mul.s	$f8,$f4,$f0
/*  f123040:	e7b40388 */ 	swc1	$f20,0x388($sp)
/*  f123044:	e7b40390 */ 	swc1	$f20,0x390($sp)
/*  f123048:	e7b4039c */ 	swc1	$f20,0x39c($sp)
/*  f12304c:	e7a603ec */ 	swc1	$f6,0x3ec($sp)
/*  f123050:	46009182 */ 	mul.s	$f6,$f18,$f0
/*  f123054:	c7a00460 */ 	lwc1	$f0,0x460($sp)
/*  f123058:	e7a803f0 */ 	swc1	$f8,0x3f0($sp)
/*  f12305c:	46026202 */ 	mul.s	$f8,$f12,$f2
/*  f123060:	e7b40398 */ 	swc1	$f20,0x398($sp)
/*  f123064:	afab047c */ 	sw	$t3,0x47c($sp)
/*  f123068:	afa90484 */ 	sw	$t1,0x484($sp)
/*  f12306c:	e7a603f4 */ 	swc1	$f6,0x3f4($sp)
/*  f123070:	46027182 */ 	mul.s	$f6,$f14,$f2
/*  f123074:	c42e50e0 */ 	lwc1	$f14,%lo(var7f1b50e0)($at)
/*  f123078:	e7a803c0 */ 	swc1	$f8,0x3c0($sp)
/*  f12307c:	46025202 */ 	mul.s	$f8,$f10,$f2
/*  f123080:	afa80480 */ 	sw	$t0,0x480($sp)
/*  f123084:	e7a603c4 */ 	swc1	$f6,0x3c4($sp)
/*  f123088:	46028182 */ 	mul.s	$f6,$f16,$f2
/*  f12308c:	e7a803c8 */ 	swc1	$f8,0x3c8($sp)
/*  f123090:	46022202 */ 	mul.s	$f8,$f4,$f2
/*  f123094:	e7a603cc */ 	swc1	$f6,0x3cc($sp)
/*  f123098:	46029182 */ 	mul.s	$f6,$f18,$f2
/*  f12309c:	e7a803d0 */ 	swc1	$f8,0x3d0($sp)
/*  f1230a0:	46001203 */ 	div.s	$f8,$f2,$f0
/*  f1230a4:	e7a603d4 */ 	swc1	$f6,0x3d4($sp)
/*  f1230a8:	46101183 */ 	div.s	$f6,$f2,$f16
/*  f1230ac:	e7a803a4 */ 	swc1	$f8,0x3a4($sp)
/*  f1230b0:	46121203 */ 	div.s	$f8,$f2,$f18
/*  f1230b4:	e7a603ac */ 	swc1	$f6,0x3ac($sp)
/*  f1230b8:	c7a60464 */ 	lwc1	$f6,0x464($sp)
/*  f1230bc:	46003303 */ 	div.s	$f12,$f6,$f0
/*  f1230c0:	e7a803b4 */ 	swc1	$f8,0x3b4($sp)
/*  f1230c4:	46122183 */ 	div.s	$f6,$f4,$f18
/*  f1230c8:	e7ac0384 */ 	swc1	$f12,0x384($sp)
/*  f1230cc:	46105203 */ 	div.s	$f8,$f10,$f16
/*  f1230d0:	e7a60394 */ 	swc1	$f6,0x394($sp)
/*  f1230d4:	0fc47cf4 */ 	jal	skyClamp
/*  f1230d8:	e7a8038c */ 	swc1	$f8,0x38c($sp)
/*  f1230dc:	3c017f1b */ 	lui	$at,%hi(var7f1b50e4)
/*  f1230e0:	3c0644ea */ 	lui	$a2,0x44ea
/*  f1230e4:	e7a00384 */ 	swc1	$f0,0x384($sp)
/*  f1230e8:	34c6a000 */ 	ori	$a2,$a2,0xa000
/*  f1230ec:	c42e50e4 */ 	lwc1	$f14,%lo(var7f1b50e4)($at)
/*  f1230f0:	0fc47cf4 */ 	jal	skyClamp
/*  f1230f4:	c7ac038c */ 	lwc1	$f12,0x38c($sp)
/*  f1230f8:	3c017f1b */ 	lui	$at,%hi(var7f1b50e8)
/*  f1230fc:	3c0644ea */ 	lui	$a2,0x44ea
/*  f123100:	e7a0038c */ 	swc1	$f0,0x38c($sp)
/*  f123104:	34c6a000 */ 	ori	$a2,$a2,0xa000
/*  f123108:	c42e50e8 */ 	lwc1	$f14,%lo(var7f1b50e8)($at)
/*  f12310c:	0fc47cf4 */ 	jal	skyClamp
/*  f123110:	c7ac0394 */ 	lwc1	$f12,0x394($sp)
/*  f123114:	8fa90484 */ 	lw	$t1,0x484($sp)
/*  f123118:	e7a00394 */ 	swc1	$f0,0x394($sp)
/*  f12311c:	3c013e80 */ 	lui	$at,0x3e80
/*  f123120:	44814000 */ 	mtc1	$at,$f8
/*  f123124:	c52a002c */ 	lwc1	$f10,0x2c($t1)
/*  f123128:	3c014600 */ 	lui	$at,0x4600
/*  f12312c:	46085082 */ 	mul.s	$f2,$f10,$f8
/*  f123130:	4600110d */ 	trunc.w.s	$f4,$f2
/*  f123134:	44192000 */ 	mfc1	$t9,$f4
/*  f123138:	c7a4038c */ 	lwc1	$f4,0x38c($sp)
/*  f12313c:	44993000 */ 	mtc1	$t9,$f6
/*  f123140:	00000000 */ 	nop
/*  f123144:	468032a0 */ 	cvt.s.w	$f10,$f6
/*  f123148:	44813000 */ 	mtc1	$at,$f6
/*  f12314c:	00000000 */ 	nop
/*  f123150:	46062302 */ 	mul.s	$f12,$f4,$f6
/*  f123154:	460a1201 */ 	sub.s	$f8,$f2,$f10
/*  f123158:	0fc47d04 */ 	jal	skyRound
/*  f12315c:	e7a8037c */ 	swc1	$f8,0x37c($sp)
/*  f123160:	3c013900 */ 	lui	$at,0x3900
/*  f123164:	44815000 */ 	mtc1	$at,$f10
/*  f123168:	c7a4037c */ 	lwc1	$f4,0x37c($sp)
/*  f12316c:	3c014600 */ 	lui	$at,0x4600
/*  f123170:	460a0202 */ 	mul.s	$f8,$f0,$f10
/*  f123174:	c7aa0428 */ 	lwc1	$f10,0x428($sp)
/*  f123178:	46044182 */ 	mul.s	$f6,$f8,$f4
/*  f12317c:	c7a40394 */ 	lwc1	$f4,0x394($sp)
/*  f123180:	46065201 */ 	sub.s	$f8,$f10,$f6
/*  f123184:	44815000 */ 	mtc1	$at,$f10
/*  f123188:	00000000 */ 	nop
/*  f12318c:	460a2302 */ 	mul.s	$f12,$f4,$f10
/*  f123190:	0fc47d04 */ 	jal	skyRound
/*  f123194:	e7a80408 */ 	swc1	$f8,0x408($sp)
/*  f123198:	3c013900 */ 	lui	$at,0x3900
/*  f12319c:	44813000 */ 	mtc1	$at,$f6
/*  f1231a0:	c7a4037c */ 	lwc1	$f4,0x37c($sp)
/*  f1231a4:	3c07b400 */ 	lui	$a3,0xb400
/*  f1231a8:	46060202 */ 	mul.s	$f8,$f0,$f6
/*  f1231ac:	c7a60430 */ 	lwc1	$f6,0x430($sp)
/*  f1231b0:	8fa80480 */ 	lw	$t0,0x480($sp)
/*  f1231b4:	8fa90484 */ 	lw	$t1,0x484($sp)
/*  f1231b8:	8fab047c */ 	lw	$t3,0x47c($sp)
/*  f1231bc:	02002825 */ 	or	$a1,$s0,$zero
/*  f1231c0:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1231c4:	46044282 */ 	mul.s	$f10,$f8,$f4
/*  f1231c8:	24190001 */ 	addiu	$t9,$zero,0x1
/*  f1231cc:	3c06c800 */ 	lui	$a2,0xc800
/*  f1231d0:	02001025 */ 	or	$v0,$s0,$zero
/*  f1231d4:	460a3201 */ 	sub.s	$f8,$f6,$f10
/*  f1231d8:	e7a80410 */ 	swc1	$f8,0x410($sp)
/*  f1231dc:	aca70000 */ 	sw	$a3,0x0($a1)
/*  f1231e0:	8fb8049c */ 	lw	$t8,0x49c($sp)
/*  f1231e4:	c7a40444 */ 	lwc1	$f4,0x444($sp)
/*  f1231e8:	13000003 */ 	beqz	$t8,.L0f1231f8
/*  f1231ec:	00000000 */ 	nop
/*  f1231f0:	10000001 */ 	b	.L0f1231f8
/*  f1231f4:	3c06ce00 */ 	lui	$a2,0xce00
.L0f1231f8:
/*  f1231f8:	4614203c */ 	c.lt.s	$f4,$f20
/*  f1231fc:	00001825 */ 	or	$v1,$zero,$zero
/*  f123200:	45000003 */ 	bc1f	.L0f123210
/*  f123204:	00000000 */ 	nop
/*  f123208:	10000001 */ 	b	.L0f123210
/*  f12320c:	3c030080 */ 	lui	$v1,0x80
.L0f123210:
/*  f123210:	444ef800 */ 	cfc1	$t6,$31
/*  f123214:	44d9f800 */ 	ctc1	$t9,$31
/*  f123218:	c566002c */ 	lwc1	$f6,0x2c($t3)
/*  f12321c:	00667825 */ 	or	$t7,$v1,$a2
/*  f123220:	3c014f00 */ 	lui	$at,0x4f00
/*  f123224:	460032a4 */ 	cvt.w.s	$f10,$f6
/*  f123228:	4459f800 */ 	cfc1	$t9,$31
/*  f12322c:	00000000 */ 	nop
/*  f123230:	33390078 */ 	andi	$t9,$t9,0x78
/*  f123234:	53200013 */ 	beqzl	$t9,.L0f123284
/*  f123238:	44195000 */ 	mfc1	$t9,$f10
/*  f12323c:	44815000 */ 	mtc1	$at,$f10
/*  f123240:	24190001 */ 	addiu	$t9,$zero,0x1
/*  f123244:	460a3281 */ 	sub.s	$f10,$f6,$f10
/*  f123248:	44d9f800 */ 	ctc1	$t9,$31
/*  f12324c:	00000000 */ 	nop
/*  f123250:	460052a4 */ 	cvt.w.s	$f10,$f10
/*  f123254:	4459f800 */ 	cfc1	$t9,$31
/*  f123258:	00000000 */ 	nop
/*  f12325c:	33390078 */ 	andi	$t9,$t9,0x78
/*  f123260:	17200005 */ 	bnez	$t9,.L0f123278
/*  f123264:	00000000 */ 	nop
/*  f123268:	44195000 */ 	mfc1	$t9,$f10
/*  f12326c:	3c018000 */ 	lui	$at,0x8000
/*  f123270:	10000007 */ 	b	.L0f123290
/*  f123274:	0321c825 */ 	or	$t9,$t9,$at
.L0f123278:
/*  f123278:	10000005 */ 	b	.L0f123290
/*  f12327c:	2419ffff */ 	addiu	$t9,$zero,-1
/*  f123280:	44195000 */ 	mfc1	$t9,$f10
.L0f123284:
/*  f123284:	00000000 */ 	nop
/*  f123288:	0720fffb */ 	bltz	$t9,.L0f123278
/*  f12328c:	00000000 */ 	nop
.L0f123290:
/*  f123290:	44cef800 */ 	ctc1	$t6,$31
/*  f123294:	01f9c025 */ 	or	$t8,$t7,$t9
/*  f123298:	acb80004 */ 	sw	$t8,0x4($a1)
/*  f12329c:	3c0eb200 */ 	lui	$t6,0xb200
/*  f1232a0:	ac4e0000 */ 	sw	$t6,0x0($v0)
/*  f1232a4:	c508002c */ 	lwc1	$f8,0x2c($t0)
/*  f1232a8:	c526002c */ 	lwc1	$f6,0x2c($t1)
/*  f1232ac:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1232b0:	4600410d */ 	trunc.w.s	$f4,$f8
/*  f1232b4:	02002025 */ 	or	$a0,$s0,$zero
/*  f1232b8:	3c013e80 */ 	lui	$at,0x3e80
/*  f1232bc:	4600328d */ 	trunc.w.s	$f10,$f6
/*  f1232c0:	44192000 */ 	mfc1	$t9,$f4
/*  f1232c4:	44812000 */ 	mtc1	$at,$f4
/*  f1232c8:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1232cc:	440f5000 */ 	mfc1	$t7,$f10
/*  f1232d0:	0019c400 */ 	sll	$t8,$t9,0x10
/*  f1232d4:	030fc825 */ 	or	$t9,$t8,$t7
/*  f1232d8:	ac590004 */ 	sw	$t9,0x4($v0)
/*  f1232dc:	ac870000 */ 	sw	$a3,0x0($a0)
/*  f1232e0:	c5080028 */ 	lwc1	$f8,0x28($t0)
/*  f1232e4:	afab047c */ 	sw	$t3,0x47c($sp)
/*  f1232e8:	afa90484 */ 	sw	$t1,0x484($sp)
/*  f1232ec:	46044302 */ 	mul.s	$f12,$f8,$f4
/*  f1232f0:	afa80480 */ 	sw	$t0,0x480($sp)
/*  f1232f4:	0fc54be8 */ 	jal	func0f152fa0
/*  f1232f8:	afa40180 */ 	sw	$a0,0x180($sp)
/*  f1232fc:	8fa40180 */ 	lw	$a0,0x180($sp)
/*  f123300:	02001825 */ 	or	$v1,$s0,$zero
/*  f123304:	3c0eb200 */ 	lui	$t6,0xb200
/*  f123308:	ac820004 */ 	sw	$v0,0x4($a0)
/*  f12330c:	ac6e0000 */ 	sw	$t6,0x0($v1)
/*  f123310:	afa3017c */ 	sw	$v1,0x17c($sp)
/*  f123314:	c7ac0384 */ 	lwc1	$f12,0x384($sp)
/*  f123318:	0fc54be8 */ 	jal	func0f152fa0
/*  f12331c:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f123320:	8fa3017c */ 	lw	$v1,0x17c($sp)
/*  f123324:	02002025 */ 	or	$a0,$s0,$zero
/*  f123328:	3c18b400 */ 	lui	$t8,0xb400
/*  f12332c:	ac620004 */ 	sw	$v0,0x4($v1)
/*  f123330:	ac980000 */ 	sw	$t8,0x0($a0)
/*  f123334:	afa40178 */ 	sw	$a0,0x178($sp)
/*  f123338:	c7ac0410 */ 	lwc1	$f12,0x410($sp)
/*  f12333c:	0fc54be8 */ 	jal	func0f152fa0
/*  f123340:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f123344:	8fa40178 */ 	lw	$a0,0x178($sp)
/*  f123348:	02001825 */ 	or	$v1,$s0,$zero
/*  f12334c:	3c0fb200 */ 	lui	$t7,0xb200
/*  f123350:	ac820004 */ 	sw	$v0,0x4($a0)
/*  f123354:	ac6f0000 */ 	sw	$t7,0x0($v1)
/*  f123358:	afa30174 */ 	sw	$v1,0x174($sp)
/*  f12335c:	c7ac0394 */ 	lwc1	$f12,0x394($sp)
/*  f123360:	0fc54be8 */ 	jal	func0f152fa0
/*  f123364:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f123368:	8fa30174 */ 	lw	$v1,0x174($sp)
/*  f12336c:	02002025 */ 	or	$a0,$s0,$zero
/*  f123370:	3c19b400 */ 	lui	$t9,0xb400
/*  f123374:	ac620004 */ 	sw	$v0,0x4($v1)
/*  f123378:	ac990000 */ 	sw	$t9,0x0($a0)
/*  f12337c:	afa40170 */ 	sw	$a0,0x170($sp)
/*  f123380:	c7ac0408 */ 	lwc1	$f12,0x408($sp)
/*  f123384:	0fc54be8 */ 	jal	func0f152fa0
/*  f123388:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f12338c:	8fa40170 */ 	lw	$a0,0x170($sp)
/*  f123390:	02001825 */ 	or	$v1,$s0,$zero
/*  f123394:	3c0eb200 */ 	lui	$t6,0xb200
/*  f123398:	ac820004 */ 	sw	$v0,0x4($a0)
/*  f12339c:	ac6e0000 */ 	sw	$t6,0x0($v1)
/*  f1233a0:	afa3016c */ 	sw	$v1,0x16c($sp)
/*  f1233a4:	c7ac038c */ 	lwc1	$f12,0x38c($sp)
/*  f1233a8:	0fc54be8 */ 	jal	func0f152fa0
/*  f1233ac:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1233b0:	8fa3016c */ 	lw	$v1,0x16c($sp)
/*  f1233b4:	8fa80480 */ 	lw	$t0,0x480($sp)
/*  f1233b8:	8fa90484 */ 	lw	$t1,0x484($sp)
/*  f1233bc:	8fab047c */ 	lw	$t3,0x47c($sp)
/*  f1233c0:	ac620004 */ 	sw	$v0,0x4($v1)
/*  f1233c4:	8fb8049c */ 	lw	$t8,0x49c($sp)
/*  f1233c8:	c7a00378 */ 	lwc1	$f0,0x378($sp)
/*  f1233cc:	3c017f1b */ 	lui	$at,%hi(var7f1b50ec)
/*  f1233d0:	57000004 */ 	bnezl	$t8,.L0f1233e4
/*  f1233d4:	c526000c */ 	lwc1	$f6,0xc($t1)
/*  f1233d8:	100002f9 */ 	b	.L0f123fc0
/*  f1233dc:	02001025 */ 	or	$v0,$s0,$zero
/*  f1233e0:	c526000c */ 	lwc1	$f6,0xc($t1)
.L0f1233e4:
/*  f1233e4:	46003282 */ 	mul.s	$f10,$f6,$f0
/*  f1233e8:	e7aa036c */ 	swc1	$f10,0x36c($sp)
/*  f1233ec:	c508000c */ 	lwc1	$f8,0xc($t0)
/*  f1233f0:	c7b2036c */ 	lwc1	$f18,0x36c($sp)
/*  f1233f4:	46004102 */ 	mul.s	$f4,$f8,$f0
/*  f1233f8:	e7a40370 */ 	swc1	$f4,0x370($sp)
/*  f1233fc:	c566000c */ 	lwc1	$f6,0xc($t3)
/*  f123400:	c7a80370 */ 	lwc1	$f8,0x370($sp)
/*  f123404:	46003282 */ 	mul.s	$f10,$f6,$f0
/*  f123408:	4612403c */ 	c.lt.s	$f8,$f18
/*  f12340c:	e7aa0374 */ 	swc1	$f10,0x374($sp)
/*  f123410:	45000002 */ 	bc1f	.L0f12341c
/*  f123414:	c7a00374 */ 	lwc1	$f0,0x374($sp)
/*  f123418:	46004486 */ 	mov.s	$f18,$f8
.L0f12341c:
/*  f12341c:	4612003c */ 	c.lt.s	$f0,$f18
/*  f123420:	00000000 */ 	nop
/*  f123424:	45000002 */ 	bc1f	.L0f123430
/*  f123428:	00000000 */ 	nop
/*  f12342c:	46000486 */ 	mov.s	$f18,$f0
.L0f123430:
/*  f123430:	c42050ec */ 	lwc1	$f0,%lo(var7f1b50ec)($at)
/*  f123434:	3c013f00 */ 	lui	$at,0x3f00
/*  f123438:	44811000 */ 	mtc1	$at,$f2
/*  f12343c:	c5240034 */ 	lwc1	$f4,0x34($t1)
/*  f123440:	46029482 */ 	mul.s	$f18,$f18,$f2
/*  f123444:	00000000 */ 	nop
/*  f123448:	46122182 */ 	mul.s	$f6,$f4,$f18
/*  f12344c:	e7a6035c */ 	swc1	$f6,0x35c($sp)
/*  f123450:	c50a0034 */ 	lwc1	$f10,0x34($t0)
/*  f123454:	46125202 */ 	mul.s	$f8,$f10,$f18
/*  f123458:	c7aa035c */ 	lwc1	$f10,0x35c($sp)
/*  f12345c:	e7a80360 */ 	swc1	$f8,0x360($sp)
/*  f123460:	c5640034 */ 	lwc1	$f4,0x34($t3)
/*  f123464:	46122182 */ 	mul.s	$f6,$f4,$f18
/*  f123468:	e7a60364 */ 	swc1	$f6,0x364($sp)
/*  f12346c:	c5280020 */ 	lwc1	$f8,0x20($t1)
/*  f123470:	46085102 */ 	mul.s	$f4,$f10,$f8
/*  f123474:	e7a40338 */ 	swc1	$f4,0x338($sp)
/*  f123478:	c5260024 */ 	lwc1	$f6,0x24($t1)
/*  f12347c:	4604a03e */ 	c.le.s	$f20,$f4
/*  f123480:	46065202 */ 	mul.s	$f8,$f10,$f6
/*  f123484:	00000000 */ 	nop
/*  f123488:	46005182 */ 	mul.s	$f6,$f10,$f0
/*  f12348c:	e7a8033c */ 	swc1	$f8,0x33c($sp)
/*  f123490:	c7a80360 */ 	lwc1	$f8,0x360($sp)
/*  f123494:	e7a60340 */ 	swc1	$f6,0x340($sp)
/*  f123498:	c50a0020 */ 	lwc1	$f10,0x20($t0)
/*  f12349c:	460a4182 */ 	mul.s	$f6,$f8,$f10
/*  f1234a0:	e7a60344 */ 	swc1	$f6,0x344($sp)
/*  f1234a4:	c50a0024 */ 	lwc1	$f10,0x24($t0)
/*  f1234a8:	460a4182 */ 	mul.s	$f6,$f8,$f10
/*  f1234ac:	00000000 */ 	nop
/*  f1234b0:	46004282 */ 	mul.s	$f10,$f8,$f0
/*  f1234b4:	e7a60348 */ 	swc1	$f6,0x348($sp)
/*  f1234b8:	c7a60364 */ 	lwc1	$f6,0x364($sp)
/*  f1234bc:	e7aa034c */ 	swc1	$f10,0x34c($sp)
/*  f1234c0:	c5680020 */ 	lwc1	$f8,0x20($t3)
/*  f1234c4:	46083282 */ 	mul.s	$f10,$f6,$f8
/*  f1234c8:	e7aa0350 */ 	swc1	$f10,0x350($sp)
/*  f1234cc:	c5680024 */ 	lwc1	$f8,0x24($t3)
/*  f1234d0:	46083282 */ 	mul.s	$f10,$f6,$f8
/*  f1234d4:	00000000 */ 	nop
/*  f1234d8:	46003202 */ 	mul.s	$f8,$f6,$f0
/*  f1234dc:	c7a0033c */ 	lwc1	$f0,0x33c($sp)
/*  f1234e0:	e7aa0354 */ 	swc1	$f10,0x354($sp)
/*  f1234e4:	45000003 */ 	bc1f	.L0f1234f4
/*  f1234e8:	e7a80358 */ 	swc1	$f8,0x358($sp)
/*  f1234ec:	10000003 */ 	b	.L0f1234fc
/*  f1234f0:	46002386 */ 	mov.s	$f14,$f4
.L0f1234f4:
/*  f1234f4:	c7ae0338 */ 	lwc1	$f14,0x338($sp)
/*  f1234f8:	46007387 */ 	neg.s	$f14,$f14
.L0f1234fc:
/*  f1234fc:	4600a03e */ 	c.le.s	$f20,$f0
/*  f123500:	00000000 */ 	nop
/*  f123504:	45020004 */ 	bc1fl	.L0f123518
/*  f123508:	46000407 */ 	neg.s	$f16,$f0
/*  f12350c:	10000002 */ 	b	.L0f123518
/*  f123510:	46000406 */ 	mov.s	$f16,$f0
/*  f123514:	46000407 */ 	neg.s	$f16,$f0
.L0f123518:
/*  f123518:	c7a00344 */ 	lwc1	$f0,0x344($sp)
/*  f12351c:	4600a03e */ 	c.le.s	$f20,$f0
/*  f123520:	00000000 */ 	nop
/*  f123524:	45020004 */ 	bc1fl	.L0f123538
/*  f123528:	46000307 */ 	neg.s	$f12,$f0
/*  f12352c:	10000002 */ 	b	.L0f123538
/*  f123530:	46000306 */ 	mov.s	$f12,$f0
/*  f123534:	46000307 */ 	neg.s	$f12,$f0
.L0f123538:
/*  f123538:	460c703c */ 	c.lt.s	$f14,$f12
/*  f12353c:	00000000 */ 	nop
/*  f123540:	45020009 */ 	bc1fl	.L0f123568
/*  f123544:	c7a00348 */ 	lwc1	$f0,0x348($sp)
/*  f123548:	4600a03e */ 	c.le.s	$f20,$f0
/*  f12354c:	00000000 */ 	nop
/*  f123550:	45020004 */ 	bc1fl	.L0f123564
/*  f123554:	46000387 */ 	neg.s	$f14,$f0
/*  f123558:	10000002 */ 	b	.L0f123564
/*  f12355c:	46000386 */ 	mov.s	$f14,$f0
/*  f123560:	46000387 */ 	neg.s	$f14,$f0
.L0f123564:
/*  f123564:	c7a00348 */ 	lwc1	$f0,0x348($sp)
.L0f123568:
/*  f123568:	4600a03e */ 	c.le.s	$f20,$f0
/*  f12356c:	00000000 */ 	nop
/*  f123570:	45020004 */ 	bc1fl	.L0f123584
/*  f123574:	46000307 */ 	neg.s	$f12,$f0
/*  f123578:	10000002 */ 	b	.L0f123584
/*  f12357c:	46000306 */ 	mov.s	$f12,$f0
/*  f123580:	46000307 */ 	neg.s	$f12,$f0
.L0f123584:
/*  f123584:	460c803c */ 	c.lt.s	$f16,$f12
/*  f123588:	e7b20368 */ 	swc1	$f18,0x368($sp)
/*  f12358c:	4502000b */ 	bc1fl	.L0f1235bc
/*  f123590:	c7a00350 */ 	lwc1	$f0,0x350($sp)
/*  f123594:	4600a03e */ 	c.le.s	$f20,$f0
/*  f123598:	00000000 */ 	nop
/*  f12359c:	45020005 */ 	bc1fl	.L0f1235b4
/*  f1235a0:	46000407 */ 	neg.s	$f16,$f0
/*  f1235a4:	46000406 */ 	mov.s	$f16,$f0
/*  f1235a8:	10000003 */ 	b	.L0f1235b8
/*  f1235ac:	e7b20368 */ 	swc1	$f18,0x368($sp)
/*  f1235b0:	46000407 */ 	neg.s	$f16,$f0
.L0f1235b4:
/*  f1235b4:	e7b20368 */ 	swc1	$f18,0x368($sp)
.L0f1235b8:
/*  f1235b8:	c7a00350 */ 	lwc1	$f0,0x350($sp)
.L0f1235bc:
/*  f1235bc:	4600a03e */ 	c.le.s	$f20,$f0
/*  f1235c0:	00000000 */ 	nop
/*  f1235c4:	45020004 */ 	bc1fl	.L0f1235d8
/*  f1235c8:	46000307 */ 	neg.s	$f12,$f0
/*  f1235cc:	10000002 */ 	b	.L0f1235d8
/*  f1235d0:	46000306 */ 	mov.s	$f12,$f0
/*  f1235d4:	46000307 */ 	neg.s	$f12,$f0
.L0f1235d8:
/*  f1235d8:	460c703c */ 	c.lt.s	$f14,$f12
/*  f1235dc:	e7ae0330 */ 	swc1	$f14,0x330($sp)
/*  f1235e0:	4502000a */ 	bc1fl	.L0f12360c
/*  f1235e4:	c7a00354 */ 	lwc1	$f0,0x354($sp)
/*  f1235e8:	4600a03e */ 	c.le.s	$f20,$f0
/*  f1235ec:	00000000 */ 	nop
/*  f1235f0:	45020004 */ 	bc1fl	.L0f123604
/*  f1235f4:	46000387 */ 	neg.s	$f14,$f0
/*  f1235f8:	10000003 */ 	b	.L0f123608
/*  f1235fc:	e7a00330 */ 	swc1	$f0,0x330($sp)
/*  f123600:	46000387 */ 	neg.s	$f14,$f0
.L0f123604:
/*  f123604:	e7ae0330 */ 	swc1	$f14,0x330($sp)
.L0f123608:
/*  f123608:	c7a00354 */ 	lwc1	$f0,0x354($sp)
.L0f12360c:
/*  f12360c:	4600a03e */ 	c.le.s	$f20,$f0
/*  f123610:	00000000 */ 	nop
/*  f123614:	45020004 */ 	bc1fl	.L0f123628
/*  f123618:	46000307 */ 	neg.s	$f12,$f0
/*  f12361c:	10000002 */ 	b	.L0f123628
/*  f123620:	46000306 */ 	mov.s	$f12,$f0
/*  f123624:	46000307 */ 	neg.s	$f12,$f0
.L0f123628:
/*  f123628:	460c803c */ 	c.lt.s	$f16,$f12
/*  f12362c:	e7b00334 */ 	swc1	$f16,0x334($sp)
/*  f123630:	4502000a */ 	bc1fl	.L0f12365c
/*  f123634:	c52a0010 */ 	lwc1	$f10,0x10($t1)
/*  f123638:	4600a03e */ 	c.le.s	$f20,$f0
/*  f12363c:	00000000 */ 	nop
/*  f123640:	45020004 */ 	bc1fl	.L0f123654
/*  f123644:	46000407 */ 	neg.s	$f16,$f0
/*  f123648:	10000003 */ 	b	.L0f123658
/*  f12364c:	e7a00334 */ 	swc1	$f0,0x334($sp)
/*  f123650:	46000407 */ 	neg.s	$f16,$f0
.L0f123654:
/*  f123654:	e7b00334 */ 	swc1	$f16,0x334($sp)
.L0f123658:
/*  f123658:	c52a0010 */ 	lwc1	$f10,0x10($t1)
.L0f12365c:
/*  f12365c:	27a402b0 */ 	addiu	$a0,$sp,0x2b0
/*  f123660:	27a602f0 */ 	addiu	$a2,$sp,0x2f0
/*  f123664:	46025180 */ 	add.s	$f6,$f10,$f2
/*  f123668:	27a20310 */ 	addiu	$v0,$sp,0x310
/*  f12366c:	27a30290 */ 	addiu	$v1,$sp,0x290
/*  f123670:	27a502d0 */ 	addiu	$a1,$sp,0x2d0
/*  f123674:	e7a60310 */ 	swc1	$f6,0x310($sp)
/*  f123678:	c5280014 */ 	lwc1	$f8,0x14($t1)
/*  f12367c:	27a702f0 */ 	addiu	$a3,$sp,0x2f0
/*  f123680:	46024100 */ 	add.s	$f4,$f8,$f2
/*  f123684:	e7a40314 */ 	swc1	$f4,0x314($sp)
/*  f123688:	c52a0018 */ 	lwc1	$f10,0x18($t1)
/*  f12368c:	46025180 */ 	add.s	$f6,$f10,$f2
/*  f123690:	e7a60318 */ 	swc1	$f6,0x318($sp)
/*  f123694:	c528001c */ 	lwc1	$f8,0x1c($t1)
/*  f123698:	46024100 */ 	add.s	$f4,$f8,$f2
/*  f12369c:	e7a4031c */ 	swc1	$f4,0x31c($sp)
/*  f1236a0:	c50a0010 */ 	lwc1	$f10,0x10($t0)
/*  f1236a4:	46025180 */ 	add.s	$f6,$f10,$f2
/*  f1236a8:	e7a602f0 */ 	swc1	$f6,0x2f0($sp)
/*  f1236ac:	c5080014 */ 	lwc1	$f8,0x14($t0)
/*  f1236b0:	46024100 */ 	add.s	$f4,$f8,$f2
/*  f1236b4:	e7a402f4 */ 	swc1	$f4,0x2f4($sp)
/*  f1236b8:	c50a0018 */ 	lwc1	$f10,0x18($t0)
/*  f1236bc:	46025180 */ 	add.s	$f6,$f10,$f2
/*  f1236c0:	e7a602f8 */ 	swc1	$f6,0x2f8($sp)
/*  f1236c4:	c508001c */ 	lwc1	$f8,0x1c($t0)
/*  f1236c8:	46024100 */ 	add.s	$f4,$f8,$f2
/*  f1236cc:	e7a402fc */ 	swc1	$f4,0x2fc($sp)
/*  f1236d0:	c56a0010 */ 	lwc1	$f10,0x10($t3)
/*  f1236d4:	46025180 */ 	add.s	$f6,$f10,$f2
/*  f1236d8:	e7a602d0 */ 	swc1	$f6,0x2d0($sp)
/*  f1236dc:	c5680014 */ 	lwc1	$f8,0x14($t3)
/*  f1236e0:	46024100 */ 	add.s	$f4,$f8,$f2
/*  f1236e4:	e7a402d4 */ 	swc1	$f4,0x2d4($sp)
/*  f1236e8:	c56a0018 */ 	lwc1	$f10,0x18($t3)
/*  f1236ec:	46025180 */ 	add.s	$f6,$f10,$f2
/*  f1236f0:	c7aa0338 */ 	lwc1	$f10,0x338($sp)
/*  f1236f4:	e7a602d8 */ 	swc1	$f6,0x2d8($sp)
/*  f1236f8:	c568001c */ 	lwc1	$f8,0x1c($t3)
/*  f1236fc:	e7aa0320 */ 	swc1	$f10,0x320($sp)
/*  f123700:	c7a6033c */ 	lwc1	$f6,0x33c($sp)
/*  f123704:	46024100 */ 	add.s	$f4,$f8,$f2
/*  f123708:	c7a80340 */ 	lwc1	$f8,0x340($sp)
/*  f12370c:	c7aa0348 */ 	lwc1	$f10,0x348($sp)
/*  f123710:	e7a60324 */ 	swc1	$f6,0x324($sp)
/*  f123714:	e7a402dc */ 	swc1	$f4,0x2dc($sp)
/*  f123718:	c7a40344 */ 	lwc1	$f4,0x344($sp)
/*  f12371c:	e7a80328 */ 	swc1	$f8,0x328($sp)
/*  f123720:	e7aa0304 */ 	swc1	$f10,0x304($sp)
/*  f123724:	e7a40300 */ 	swc1	$f4,0x300($sp)
/*  f123728:	c7a6034c */ 	lwc1	$f6,0x34c($sp)
/*  f12372c:	c7a40354 */ 	lwc1	$f4,0x354($sp)
/*  f123730:	c7aa0358 */ 	lwc1	$f10,0x358($sp)
/*  f123734:	c7a80350 */ 	lwc1	$f8,0x350($sp)
/*  f123738:	e7a60308 */ 	swc1	$f6,0x308($sp)
/*  f12373c:	e7a402e4 */ 	swc1	$f4,0x2e4($sp)
/*  f123740:	e7aa02e8 */ 	swc1	$f10,0x2e8($sp)
/*  f123744:	e7a802e0 */ 	swc1	$f8,0x2e0($sp)
/*  f123748:	c5260030 */ 	lwc1	$f6,0x30($t1)
/*  f12374c:	27a90230 */ 	addiu	$t1,$sp,0x230
/*  f123750:	e7a6032c */ 	swc1	$f6,0x32c($sp)
/*  f123754:	c5080030 */ 	lwc1	$f8,0x30($t0)
/*  f123758:	27a80230 */ 	addiu	$t0,$sp,0x230
/*  f12375c:	e7a8030c */ 	swc1	$f8,0x30c($sp)
/*  f123760:	c5640030 */ 	lwc1	$f4,0x30($t3)
/*  f123764:	e7a402ec */ 	swc1	$f4,0x2ec($sp)
.L0f123768:
/*  f123768:	c4400000 */ 	lwc1	$f0,0x0($v0)
/*  f12376c:	c4ca0000 */ 	lwc1	$f10,0x0($a2)
/*  f123770:	c4a80000 */ 	lwc1	$f8,0x0($a1)
/*  f123774:	24a50004 */ 	addiu	$a1,$a1,0x4
/*  f123778:	46005181 */ 	sub.s	$f6,$f10,$f0
/*  f12377c:	00a7082b */ 	sltu	$at,$a1,$a3
/*  f123780:	24840004 */ 	addiu	$a0,$a0,0x4
/*  f123784:	46004101 */ 	sub.s	$f4,$f8,$f0
/*  f123788:	24c60004 */ 	addiu	$a2,$a2,0x4
/*  f12378c:	24420004 */ 	addiu	$v0,$v0,0x4
/*  f123790:	24630004 */ 	addiu	$v1,$v1,0x4
/*  f123794:	e486fffc */ 	swc1	$f6,-0x4($a0)
/*  f123798:	1420fff3 */ 	bnez	$at,.L0f123768
/*  f12379c:	e464fffc */ 	swc1	$f4,-0x4($v1)
/*  f1237a0:	27a402b0 */ 	addiu	$a0,$sp,0x2b0
/*  f1237a4:	27a20310 */ 	addiu	$v0,$sp,0x310
/*  f1237a8:	27a30290 */ 	addiu	$v1,$sp,0x290
/*  f1237ac:	27a70250 */ 	addiu	$a3,$sp,0x250
/*  f1237b0:	27a50270 */ 	addiu	$a1,$sp,0x270
/*  f1237b4:	27a60210 */ 	addiu	$a2,$sp,0x210
/*  f1237b8:	c7b203c8 */ 	lwc1	$f18,0x3c8($sp)
.L0f1237bc:
/*  f1237bc:	c4600000 */ 	lwc1	$f0,0x0($v1)
/*  f1237c0:	c7aa03cc */ 	lwc1	$f10,0x3cc($sp)
/*  f1237c4:	c4820000 */ 	lwc1	$f2,0x0($a0)
/*  f1237c8:	c7a803d4 */ 	lwc1	$f8,0x3d4($sp)
/*  f1237cc:	460a0182 */ 	mul.s	$f6,$f0,$f10
/*  f1237d0:	3c013780 */ 	lui	$at,0x3780
/*  f1237d4:	24c60004 */ 	addiu	$a2,$a2,0x4
/*  f1237d8:	46024102 */ 	mul.s	$f4,$f8,$f2
/*  f1237dc:	44814000 */ 	mtc1	$at,$f8
/*  f1237e0:	24840004 */ 	addiu	$a0,$a0,0x4
/*  f1237e4:	24420004 */ 	addiu	$v0,$v0,0x4
/*  f1237e8:	24630004 */ 	addiu	$v1,$v1,0x4
/*  f1237ec:	24e70004 */ 	addiu	$a3,$a3,0x4
/*  f1237f0:	24a50004 */ 	addiu	$a1,$a1,0x4
/*  f1237f4:	46043281 */ 	sub.s	$f10,$f6,$f4
/*  f1237f8:	25080004 */ 	addiu	$t0,$t0,0x4
/*  f1237fc:	46085302 */ 	mul.s	$f12,$f10,$f8
/*  f123800:	e4ecfffc */ 	swc1	$f12,-0x4($a3)
/*  f123804:	c7a603d0 */ 	lwc1	$f6,0x3d0($sp)
/*  f123808:	46061102 */ 	mul.s	$f4,$f2,$f6
/*  f12380c:	44813000 */ 	mtc1	$at,$f6
/*  f123810:	00c9082b */ 	sltu	$at,$a2,$t1
/*  f123814:	46009282 */ 	mul.s	$f10,$f18,$f0
/*  f123818:	460a2201 */ 	sub.s	$f8,$f4,$f10
/*  f12381c:	46064102 */ 	mul.s	$f4,$f8,$f6
/*  f123820:	e4a4fffc */ 	swc1	$f4,-0x4($a1)
/*  f123824:	c7aa0440 */ 	lwc1	$f10,0x440($sp)
/*  f123828:	c4a6fffc */ 	lwc1	$f6,-0x4($a1)
/*  f12382c:	460a6202 */ 	mul.s	$f8,$f12,$f10
/*  f123830:	e468fffc */ 	swc1	$f8,-0x4($v1)
/*  f123834:	c7a40440 */ 	lwc1	$f4,0x440($sp)
/*  f123838:	c468fffc */ 	lwc1	$f8,-0x4($v1)
/*  f12383c:	46043382 */ 	mul.s	$f14,$f6,$f4
/*  f123840:	c444fffc */ 	lwc1	$f4,-0x4($v0)
/*  f123844:	e48efffc */ 	swc1	$f14,-0x4($a0)
/*  f123848:	c7aa0394 */ 	lwc1	$f10,0x394($sp)
/*  f12384c:	46085182 */ 	mul.s	$f6,$f10,$f8
/*  f123850:	460e3400 */ 	add.s	$f16,$f6,$f14
/*  f123854:	e510fffc */ 	swc1	$f16,-0x4($t0)
/*  f123858:	c7aa037c */ 	lwc1	$f10,0x37c($sp)
/*  f12385c:	460a8202 */ 	mul.s	$f8,$f16,$f10
/*  f123860:	46082181 */ 	sub.s	$f6,$f4,$f8
/*  f123864:	1420ffd5 */ 	bnez	$at,.L0f1237bc
/*  f123868:	e4c6fffc */ 	swc1	$f6,-0x4($a2)
/*  f12386c:	0fc54be8 */ 	jal	func0f152fa0
/*  f123870:	c7ac0210 */ 	lwc1	$f12,0x210($sp)
/*  f123874:	afa20168 */ 	sw	$v0,0x168($sp)
/*  f123878:	0fc54be8 */ 	jal	func0f152fa0
/*  f12387c:	c7ac0214 */ 	lwc1	$f12,0x214($sp)
/*  f123880:	afa20164 */ 	sw	$v0,0x164($sp)
/*  f123884:	0fc54be8 */ 	jal	func0f152fa0
/*  f123888:	c7ac0218 */ 	lwc1	$f12,0x218($sp)
/*  f12388c:	afa20160 */ 	sw	$v0,0x160($sp)
/*  f123890:	0fc54be8 */ 	jal	func0f152fa0
/*  f123894:	c7ac021c */ 	lwc1	$f12,0x21c($sp)
/*  f123898:	afa2015c */ 	sw	$v0,0x15c($sp)
/*  f12389c:	0fc54be8 */ 	jal	func0f152fa0
/*  f1238a0:	c7ac0290 */ 	lwc1	$f12,0x290($sp)
/*  f1238a4:	afa20158 */ 	sw	$v0,0x158($sp)
/*  f1238a8:	0fc54be8 */ 	jal	func0f152fa0
/*  f1238ac:	c7ac0294 */ 	lwc1	$f12,0x294($sp)
/*  f1238b0:	afa20154 */ 	sw	$v0,0x154($sp)
/*  f1238b4:	0fc54be8 */ 	jal	func0f152fa0
/*  f1238b8:	c7ac0298 */ 	lwc1	$f12,0x298($sp)
/*  f1238bc:	afa20150 */ 	sw	$v0,0x150($sp)
/*  f1238c0:	0fc54be8 */ 	jal	func0f152fa0
/*  f1238c4:	c7ac029c */ 	lwc1	$f12,0x29c($sp)
/*  f1238c8:	afa2014c */ 	sw	$v0,0x14c($sp)
/*  f1238cc:	0fc54be8 */ 	jal	func0f152fa0
/*  f1238d0:	c7ac02b0 */ 	lwc1	$f12,0x2b0($sp)
/*  f1238d4:	afa20138 */ 	sw	$v0,0x138($sp)
/*  f1238d8:	0fc54be8 */ 	jal	func0f152fa0
/*  f1238dc:	c7ac02b4 */ 	lwc1	$f12,0x2b4($sp)
/*  f1238e0:	afa20134 */ 	sw	$v0,0x134($sp)
/*  f1238e4:	0fc54be8 */ 	jal	func0f152fa0
/*  f1238e8:	c7ac02b8 */ 	lwc1	$f12,0x2b8($sp)
/*  f1238ec:	afa20130 */ 	sw	$v0,0x130($sp)
/*  f1238f0:	0fc54be8 */ 	jal	func0f152fa0
/*  f1238f4:	c7ac02bc */ 	lwc1	$f12,0x2bc($sp)
/*  f1238f8:	afa2012c */ 	sw	$v0,0x12c($sp)
/*  f1238fc:	0fc54be8 */ 	jal	func0f152fa0
/*  f123900:	c7ac0230 */ 	lwc1	$f12,0x230($sp)
/*  f123904:	afa20148 */ 	sw	$v0,0x148($sp)
/*  f123908:	0fc54be8 */ 	jal	func0f152fa0
/*  f12390c:	c7ac0234 */ 	lwc1	$f12,0x234($sp)
/*  f123910:	afa20144 */ 	sw	$v0,0x144($sp)
/*  f123914:	0fc54be8 */ 	jal	func0f152fa0
/*  f123918:	c7ac0238 */ 	lwc1	$f12,0x238($sp)
/*  f12391c:	c7ac023c */ 	lwc1	$f12,0x23c($sp)
/*  f123920:	0fc54be8 */ 	jal	func0f152fa0
/*  f123924:	afa20140 */ 	sw	$v0,0x140($sp)
/*  f123928:	8fac0140 */ 	lw	$t4,0x140($sp)
/*  f12392c:	8fad015c */ 	lw	$t5,0x15c($sp)
/*  f123930:	8fbf0168 */ 	lw	$ra,0x168($sp)
/*  f123934:	3c08b400 */ 	lui	$t0,0xb400
/*  f123938:	02001825 */ 	or	$v1,$s0,$zero
/*  f12393c:	ac680000 */ 	sw	$t0,0x0($v1)
/*  f123940:	8fb90164 */ 	lw	$t9,0x164($sp)
/*  f123944:	3c09ffff */ 	lui	$t1,0xffff
/*  f123948:	03e97824 */ 	and	$t7,$ra,$t1
/*  f12394c:	03297024 */ 	and	$t6,$t9,$t1
/*  f123950:	000ec402 */ 	srl	$t8,$t6,0x10
/*  f123954:	01f8c825 */ 	or	$t9,$t7,$t8
/*  f123958:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f12395c:	3c0ab200 */ 	lui	$t2,0xb200
/*  f123960:	ac790004 */ 	sw	$t9,0x4($v1)
/*  f123964:	02002025 */ 	or	$a0,$s0,$zero
/*  f123968:	ac8a0000 */ 	sw	$t2,0x0($a0)
/*  f12396c:	8fae0160 */ 	lw	$t6,0x160($sp)
/*  f123970:	01a9c024 */ 	and	$t8,$t5,$t1
/*  f123974:	0018cc02 */ 	srl	$t9,$t8,0x10
/*  f123978:	01c97824 */ 	and	$t7,$t6,$t1
/*  f12397c:	01f97025 */ 	or	$t6,$t7,$t9
/*  f123980:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f123984:	ac8e0004 */ 	sw	$t6,0x4($a0)
/*  f123988:	02002825 */ 	or	$a1,$s0,$zero
/*  f12398c:	aca80000 */ 	sw	$t0,0x0($a1)
/*  f123990:	8fb90154 */ 	lw	$t9,0x154($sp)
/*  f123994:	8fb80158 */ 	lw	$t8,0x158($sp)
/*  f123998:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f12399c:	03297024 */ 	and	$t6,$t9,$t1
/*  f1239a0:	03097824 */ 	and	$t7,$t8,$t1
/*  f1239a4:	000ec402 */ 	srl	$t8,$t6,0x10
/*  f1239a8:	01f8c825 */ 	or	$t9,$t7,$t8
/*  f1239ac:	acb90004 */ 	sw	$t9,0x4($a1)
/*  f1239b0:	8fab014c */ 	lw	$t3,0x14c($sp)
/*  f1239b4:	02003025 */ 	or	$a2,$s0,$zero
/*  f1239b8:	acca0000 */ 	sw	$t2,0x0($a2)
/*  f1239bc:	8fae0150 */ 	lw	$t6,0x150($sp)
/*  f1239c0:	0169c024 */ 	and	$t8,$t3,$t1
/*  f1239c4:	0018cc02 */ 	srl	$t9,$t8,0x10
/*  f1239c8:	01c97824 */ 	and	$t7,$t6,$t1
/*  f1239cc:	01f97025 */ 	or	$t6,$t7,$t9
/*  f1239d0:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1239d4:	acce0004 */ 	sw	$t6,0x4($a2)
/*  f1239d8:	02001825 */ 	or	$v1,$s0,$zero
/*  f1239dc:	ac680000 */ 	sw	$t0,0x0($v1)
/*  f1239e0:	8fb90164 */ 	lw	$t9,0x164($sp)
/*  f1239e4:	001f7c00 */ 	sll	$t7,$ra,0x10
/*  f1239e8:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1239ec:	332effff */ 	andi	$t6,$t9,0xffff
/*  f1239f0:	01eec025 */ 	or	$t8,$t7,$t6
/*  f1239f4:	ac780004 */ 	sw	$t8,0x4($v1)
/*  f1239f8:	02002025 */ 	or	$a0,$s0,$zero
/*  f1239fc:	ac8a0000 */ 	sw	$t2,0x0($a0)
/*  f123a00:	8faf0160 */ 	lw	$t7,0x160($sp)
/*  f123a04:	31b8ffff */ 	andi	$t8,$t5,0xffff
/*  f123a08:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f123a0c:	000f7400 */ 	sll	$t6,$t7,0x10
/*  f123a10:	01d8c825 */ 	or	$t9,$t6,$t8
/*  f123a14:	ac990004 */ 	sw	$t9,0x4($a0)
/*  f123a18:	02003825 */ 	or	$a3,$s0,$zero
/*  f123a1c:	ace80000 */ 	sw	$t0,0x0($a3)
/*  f123a20:	8fb90154 */ 	lw	$t9,0x154($sp)
/*  f123a24:	8fae0158 */ 	lw	$t6,0x158($sp)
/*  f123a28:	8fa80148 */ 	lw	$t0,0x148($sp)
/*  f123a2c:	332fffff */ 	andi	$t7,$t9,0xffff
/*  f123a30:	000ec400 */ 	sll	$t8,$t6,0x10
/*  f123a34:	8fa60144 */ 	lw	$a2,0x144($sp)
/*  f123a38:	030f7025 */ 	or	$t6,$t8,$t7
/*  f123a3c:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f123a40:	acee0004 */ 	sw	$t6,0x4($a3)
/*  f123a44:	02001825 */ 	or	$v1,$s0,$zero
/*  f123a48:	ac6a0000 */ 	sw	$t2,0x0($v1)
/*  f123a4c:	8fb80150 */ 	lw	$t8,0x150($sp)
/*  f123a50:	316effff */ 	andi	$t6,$t3,0xffff
/*  f123a54:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f123a58:	00187c00 */ 	sll	$t7,$t8,0x10
/*  f123a5c:	01eec825 */ 	or	$t9,$t7,$t6
/*  f123a60:	ac790004 */ 	sw	$t9,0x4($v1)
/*  f123a64:	3c18b400 */ 	lui	$t8,0xb400
/*  f123a68:	02002025 */ 	or	$a0,$s0,$zero
/*  f123a6c:	00c97024 */ 	and	$t6,$a2,$t1
/*  f123a70:	000ecc02 */ 	srl	$t9,$t6,0x10
/*  f123a74:	ac980000 */ 	sw	$t8,0x0($a0)
/*  f123a78:	01097824 */ 	and	$t7,$t0,$t1
/*  f123a7c:	01f9c025 */ 	or	$t8,$t7,$t9
/*  f123a80:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f123a84:	ac980004 */ 	sw	$t8,0x4($a0)
/*  f123a88:	02002825 */ 	or	$a1,$s0,$zero
/*  f123a8c:	00497824 */ 	and	$t7,$v0,$t1
/*  f123a90:	000fcc02 */ 	srl	$t9,$t7,0x10
/*  f123a94:	01897024 */ 	and	$t6,$t4,$t1
/*  f123a98:	01d9c025 */ 	or	$t8,$t6,$t9
/*  f123a9c:	acb80004 */ 	sw	$t8,0x4($a1)
/*  f123aa0:	acaa0000 */ 	sw	$t2,0x0($a1)
/*  f123aa4:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f123aa8:	8fad0130 */ 	lw	$t5,0x130($sp)
/*  f123aac:	3c07b400 */ 	lui	$a3,0xb400
/*  f123ab0:	02001825 */ 	or	$v1,$s0,$zero
/*  f123ab4:	ac670000 */ 	sw	$a3,0x0($v1)
/*  f123ab8:	8fb90134 */ 	lw	$t9,0x134($sp)
/*  f123abc:	8faf0138 */ 	lw	$t7,0x138($sp)
/*  f123ac0:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f123ac4:	0329c024 */ 	and	$t8,$t9,$t1
/*  f123ac8:	01e97024 */ 	and	$t6,$t7,$t1
/*  f123acc:	00187c02 */ 	srl	$t7,$t8,0x10
/*  f123ad0:	01cfc825 */ 	or	$t9,$t6,$t7
/*  f123ad4:	ac790004 */ 	sw	$t9,0x4($v1)
/*  f123ad8:	3c18b200 */ 	lui	$t8,0xb200
/*  f123adc:	02002025 */ 	or	$a0,$s0,$zero
/*  f123ae0:	ac980000 */ 	sw	$t8,0x0($a0)
/*  f123ae4:	8faf012c */ 	lw	$t7,0x12c($sp)
/*  f123ae8:	01a97024 */ 	and	$t6,$t5,$t1
/*  f123aec:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f123af0:	01e9c824 */ 	and	$t9,$t7,$t1
/*  f123af4:	0019c402 */ 	srl	$t8,$t9,0x10
/*  f123af8:	01d87825 */ 	or	$t7,$t6,$t8
/*  f123afc:	ac8f0004 */ 	sw	$t7,0x4($a0)
/*  f123b00:	02002825 */ 	or	$a1,$s0,$zero
/*  f123b04:	00087400 */ 	sll	$t6,$t0,0x10
/*  f123b08:	30d8ffff */ 	andi	$t8,$a2,0xffff
/*  f123b0c:	01d87825 */ 	or	$t7,$t6,$t8
/*  f123b10:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f123b14:	acaf0004 */ 	sw	$t7,0x4($a1)
/*  f123b18:	aca70000 */ 	sw	$a3,0x0($a1)
/*  f123b1c:	02005825 */ 	or	$t3,$s0,$zero
/*  f123b20:	3058ffff */ 	andi	$t8,$v0,0xffff
/*  f123b24:	000c7400 */ 	sll	$t6,$t4,0x10
/*  f123b28:	01d87825 */ 	or	$t7,$t6,$t8
/*  f123b2c:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f123b30:	3c1fb200 */ 	lui	$ra,0xb200
/*  f123b34:	ad7f0000 */ 	sw	$ra,0x0($t3)
/*  f123b38:	ad6f0004 */ 	sw	$t7,0x4($t3)
/*  f123b3c:	02003825 */ 	or	$a3,$s0,$zero
/*  f123b40:	3c19b400 */ 	lui	$t9,0xb400
/*  f123b44:	acf90000 */ 	sw	$t9,0x0($a3)
/*  f123b48:	8fb90134 */ 	lw	$t9,0x134($sp)
/*  f123b4c:	8fb80138 */ 	lw	$t8,0x138($sp)
/*  f123b50:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f123b54:	332effff */ 	andi	$t6,$t9,0xffff
/*  f123b58:	00187c00 */ 	sll	$t7,$t8,0x10
/*  f123b5c:	01eec025 */ 	or	$t8,$t7,$t6
/*  f123b60:	acf80004 */ 	sw	$t8,0x4($a3)
/*  f123b64:	02004025 */ 	or	$t0,$s0,$zero
/*  f123b68:	ad1f0000 */ 	sw	$ra,0x0($t0)
/*  f123b6c:	8fae012c */ 	lw	$t6,0x12c($sp)
/*  f123b70:	000d7c00 */ 	sll	$t7,$t5,0x10
/*  f123b74:	3c013d00 */ 	lui	$at,0x3d00
/*  f123b78:	31d8ffff */ 	andi	$t8,$t6,0xffff
/*  f123b7c:	01f8c825 */ 	or	$t9,$t7,$t8
/*  f123b80:	ad190004 */ 	sw	$t9,0x4($t0)
/*  f123b84:	44817000 */ 	mtc1	$at,$f14
/*  f123b88:	c7aa0330 */ 	lwc1	$f10,0x330($sp)
/*  f123b8c:	c7a80334 */ 	lwc1	$f8,0x334($sp)
/*  f123b90:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f123b94:	460e5102 */ 	mul.s	$f4,$f10,$f14
/*  f123b98:	c7aa0368 */ 	lwc1	$f10,0x368($sp)
/*  f123b9c:	27a201d0 */ 	addiu	$v0,$sp,0x1d0
/*  f123ba0:	460e4182 */ 	mul.s	$f6,$f8,$f14
/*  f123ba4:	27a501b0 */ 	addiu	$a1,$sp,0x1b0
/*  f123ba8:	27a601d0 */ 	addiu	$a2,$sp,0x1d0
/*  f123bac:	27a402b0 */ 	addiu	$a0,$sp,0x2b0
/*  f123bb0:	e7a40200 */ 	swc1	$f4,0x200($sp)
/*  f123bb4:	460e5102 */ 	mul.s	$f4,$f10,$f14
/*  f123bb8:	27a30290 */ 	addiu	$v1,$sp,0x290
/*  f123bbc:	e7a60204 */ 	swc1	$f6,0x204($sp)
/*  f123bc0:	3c013f80 */ 	lui	$at,0x3f80
/*  f123bc4:	e7a40208 */ 	swc1	$f4,0x208($sp)
.L0f123bc8:
/*  f123bc8:	c4600000 */ 	lwc1	$f0,0x0($v1)
/*  f123bcc:	4600a03e */ 	c.le.s	$f20,$f0
/*  f123bd0:	00000000 */ 	nop
/*  f123bd4:	45020004 */ 	bc1fl	.L0f123be8
/*  f123bd8:	46000307 */ 	neg.s	$f12,$f0
/*  f123bdc:	10000002 */ 	b	.L0f123be8
/*  f123be0:	46000306 */ 	mov.s	$f12,$f0
/*  f123be4:	46000307 */ 	neg.s	$f12,$f0
.L0f123be8:
/*  f123be8:	c4820000 */ 	lwc1	$f2,0x0($a0)
/*  f123bec:	460e6202 */ 	mul.s	$f8,$f12,$f14
/*  f123bf0:	4602a03e */ 	c.le.s	$f20,$f2
/*  f123bf4:	00000000 */ 	nop
/*  f123bf8:	45000003 */ 	bc1f	.L0f123c08
/*  f123bfc:	e4c80000 */ 	swc1	$f8,0x0($a2)
/*  f123c00:	10000002 */ 	b	.L0f123c0c
/*  f123c04:	46001306 */ 	mov.s	$f12,$f2
.L0f123c08:
/*  f123c08:	46001307 */ 	neg.s	$f12,$f2
.L0f123c0c:
/*  f123c0c:	460e6182 */ 	mul.s	$f6,$f12,$f14
/*  f123c10:	24a50004 */ 	addiu	$a1,$a1,0x4
/*  f123c14:	24840004 */ 	addiu	$a0,$a0,0x4
/*  f123c18:	24630004 */ 	addiu	$v1,$v1,0x4
/*  f123c1c:	24c60004 */ 	addiu	$a2,$a2,0x4
/*  f123c20:	14a2ffe9 */ 	bne	$a1,$v0,.L0f123bc8
/*  f123c24:	e4a6fffc */ 	swc1	$f6,-0x4($a1)
/*  f123c28:	c7a201e0 */ 	lwc1	$f2,0x1e0($sp)
/*  f123c2c:	c7aa0200 */ 	lwc1	$f10,0x200($sp)
/*  f123c30:	c7ac01e4 */ 	lwc1	$f12,0x1e4($sp)
/*  f123c34:	46021100 */ 	add.s	$f4,$f2,$f2
/*  f123c38:	c7a601c0 */ 	lwc1	$f6,0x1c0($sp)
/*  f123c3c:	c7ae01e8 */ 	lwc1	$f14,0x1e8($sp)
/*  f123c40:	44811000 */ 	mtc1	$at,$f2
/*  f123c44:	46045200 */ 	add.s	$f8,$f10,$f4
/*  f123c48:	c7aa0204 */ 	lwc1	$f10,0x204($sp)
/*  f123c4c:	3c013a80 */ 	lui	$at,0x3a80
/*  f123c50:	460c6100 */ 	add.s	$f4,$f12,$f12
/*  f123c54:	46083000 */ 	add.s	$f0,$f6,$f8
/*  f123c58:	c7a801c4 */ 	lwc1	$f8,0x1c4($sp)
/*  f123c5c:	46045180 */ 	add.s	$f6,$f10,$f4
/*  f123c60:	c7aa0208 */ 	lwc1	$f10,0x208($sp)
/*  f123c64:	460e7100 */ 	add.s	$f4,$f14,$f14
/*  f123c68:	46064400 */ 	add.s	$f16,$f8,$f6
/*  f123c6c:	c7a601c8 */ 	lwc1	$f6,0x1c8($sp)
/*  f123c70:	46045200 */ 	add.s	$f8,$f10,$f4
/*  f123c74:	44815000 */ 	mtc1	$at,$f10
/*  f123c78:	e7b001a4 */ 	swc1	$f16,0x1a4($sp)
/*  f123c7c:	4610003c */ 	c.lt.s	$f0,$f16
/*  f123c80:	46083480 */ 	add.s	$f18,$f6,$f8
/*  f123c84:	45020004 */ 	bc1fl	.L0f123c98
/*  f123c88:	4612003c */ 	c.lt.s	$f0,$f18
/*  f123c8c:	46008006 */ 	mov.s	$f0,$f16
/*  f123c90:	e7b001a4 */ 	swc1	$f16,0x1a4($sp)
/*  f123c94:	4612003c */ 	c.lt.s	$f0,$f18
.L0f123c98:
/*  f123c98:	e7b201a8 */ 	swc1	$f18,0x1a8($sp)
/*  f123c9c:	45000003 */ 	bc1f	.L0f123cac
/*  f123ca0:	00000000 */ 	nop
/*  f123ca4:	46009006 */ 	mov.s	$f0,$f18
/*  f123ca8:	e7b201a8 */ 	swc1	$f18,0x1a8($sp)
.L0f123cac:
/*  f123cac:	460a0002 */ 	mul.s	$f0,$f0,$f10
/*  f123cb0:	4600103c */ 	c.lt.s	$f2,$f0
/*  f123cb4:	00000000 */ 	nop
/*  f123cb8:	45020005 */ 	bc1fl	.L0f123cd0
/*  f123cbc:	e7a001a0 */ 	swc1	$f0,0x1a0($sp)
/*  f123cc0:	46001503 */ 	div.s	$f20,$f2,$f0
/*  f123cc4:	10000003 */ 	b	.L0f123cd4
/*  f123cc8:	e7a001a0 */ 	swc1	$f0,0x1a0($sp)
/*  f123ccc:	e7a001a0 */ 	swc1	$f0,0x1a0($sp)
.L0f123cd0:
/*  f123cd0:	46001506 */ 	mov.s	$f20,$f2
.L0f123cd4:
/*  f123cd4:	c7a40220 */ 	lwc1	$f4,0x220($sp)
/*  f123cd8:	e7b40190 */ 	swc1	$f20,0x190($sp)
/*  f123cdc:	4604a302 */ 	mul.s	$f12,$f20,$f4
/*  f123ce0:	0fc54be8 */ 	jal	func0f152fa0
/*  f123ce4:	00000000 */ 	nop
/*  f123ce8:	c7b40190 */ 	lwc1	$f20,0x190($sp)
/*  f123cec:	c7a60224 */ 	lwc1	$f6,0x224($sp)
/*  f123cf0:	afa200e8 */ 	sw	$v0,0xe8($sp)
/*  f123cf4:	4606a302 */ 	mul.s	$f12,$f20,$f6
/*  f123cf8:	0fc54be8 */ 	jal	func0f152fa0
/*  f123cfc:	00000000 */ 	nop
/*  f123d00:	c7b40190 */ 	lwc1	$f20,0x190($sp)
/*  f123d04:	c7a80228 */ 	lwc1	$f8,0x228($sp)
/*  f123d08:	afa200e4 */ 	sw	$v0,0xe4($sp)
/*  f123d0c:	4608a302 */ 	mul.s	$f12,$f20,$f8
/*  f123d10:	0fc54be8 */ 	jal	func0f152fa0
/*  f123d14:	00000000 */ 	nop
/*  f123d18:	c7b40190 */ 	lwc1	$f20,0x190($sp)
/*  f123d1c:	c7aa02a0 */ 	lwc1	$f10,0x2a0($sp)
/*  f123d20:	afa200e0 */ 	sw	$v0,0xe0($sp)
/*  f123d24:	460aa302 */ 	mul.s	$f12,$f20,$f10
/*  f123d28:	0fc54be8 */ 	jal	func0f152fa0
/*  f123d2c:	00000000 */ 	nop
/*  f123d30:	c7b40190 */ 	lwc1	$f20,0x190($sp)
/*  f123d34:	c7a402a4 */ 	lwc1	$f4,0x2a4($sp)
/*  f123d38:	afa200d8 */ 	sw	$v0,0xd8($sp)
/*  f123d3c:	4604a302 */ 	mul.s	$f12,$f20,$f4
/*  f123d40:	0fc54be8 */ 	jal	func0f152fa0
/*  f123d44:	00000000 */ 	nop
/*  f123d48:	c7b40190 */ 	lwc1	$f20,0x190($sp)
/*  f123d4c:	c7a602a8 */ 	lwc1	$f6,0x2a8($sp)
/*  f123d50:	afa200d4 */ 	sw	$v0,0xd4($sp)
/*  f123d54:	4606a302 */ 	mul.s	$f12,$f20,$f6
/*  f123d58:	0fc54be8 */ 	jal	func0f152fa0
/*  f123d5c:	00000000 */ 	nop
/*  f123d60:	c7b40190 */ 	lwc1	$f20,0x190($sp)
/*  f123d64:	c7a802c0 */ 	lwc1	$f8,0x2c0($sp)
/*  f123d68:	afa200d0 */ 	sw	$v0,0xd0($sp)
/*  f123d6c:	4608a302 */ 	mul.s	$f12,$f20,$f8
/*  f123d70:	0fc54be8 */ 	jal	func0f152fa0
/*  f123d74:	00000000 */ 	nop
/*  f123d78:	c7b40190 */ 	lwc1	$f20,0x190($sp)
/*  f123d7c:	c7aa02c4 */ 	lwc1	$f10,0x2c4($sp)
/*  f123d80:	afa200b8 */ 	sw	$v0,0xb8($sp)
/*  f123d84:	460aa302 */ 	mul.s	$f12,$f20,$f10
/*  f123d88:	0fc54be8 */ 	jal	func0f152fa0
/*  f123d8c:	00000000 */ 	nop
/*  f123d90:	c7b40190 */ 	lwc1	$f20,0x190($sp)
/*  f123d94:	c7a402c8 */ 	lwc1	$f4,0x2c8($sp)
/*  f123d98:	afa200b4 */ 	sw	$v0,0xb4($sp)
/*  f123d9c:	4604a302 */ 	mul.s	$f12,$f20,$f4
/*  f123da0:	0fc54be8 */ 	jal	func0f152fa0
/*  f123da4:	00000000 */ 	nop
/*  f123da8:	c7b40190 */ 	lwc1	$f20,0x190($sp)
/*  f123dac:	c7a60240 */ 	lwc1	$f6,0x240($sp)
/*  f123db0:	afa200b0 */ 	sw	$v0,0xb0($sp)
/*  f123db4:	4606a302 */ 	mul.s	$f12,$f20,$f6
/*  f123db8:	0fc54be8 */ 	jal	func0f152fa0
/*  f123dbc:	00000000 */ 	nop
/*  f123dc0:	c7b40190 */ 	lwc1	$f20,0x190($sp)
/*  f123dc4:	c7a80244 */ 	lwc1	$f8,0x244($sp)
/*  f123dc8:	afa200c8 */ 	sw	$v0,0xc8($sp)
/*  f123dcc:	4608a302 */ 	mul.s	$f12,$f20,$f8
/*  f123dd0:	0fc54be8 */ 	jal	func0f152fa0
/*  f123dd4:	00000000 */ 	nop
/*  f123dd8:	c7b40190 */ 	lwc1	$f20,0x190($sp)
/*  f123ddc:	c7aa0248 */ 	lwc1	$f10,0x248($sp)
/*  f123de0:	afa200c4 */ 	sw	$v0,0xc4($sp)
/*  f123de4:	460aa302 */ 	mul.s	$f12,$f20,$f10
/*  f123de8:	0fc54be8 */ 	jal	func0f152fa0
/*  f123dec:	00000000 */ 	nop
/*  f123df0:	8fad00e4 */ 	lw	$t5,0xe4($sp)
/*  f123df4:	8fac00e8 */ 	lw	$t4,0xe8($sp)
/*  f123df8:	3c09ffff */ 	lui	$t1,0xffff
/*  f123dfc:	8fa600c4 */ 	lw	$a2,0xc4($sp)
/*  f123e00:	8fbf00e0 */ 	lw	$ra,0xe0($sp)
/*  f123e04:	3c0eb400 */ 	lui	$t6,0xb400
/*  f123e08:	01a9c024 */ 	and	$t8,$t5,$t1
/*  f123e0c:	0018cc02 */ 	srl	$t9,$t8,0x10
/*  f123e10:	ae0e0000 */ 	sw	$t6,0x0($s0)
/*  f123e14:	01897824 */ 	and	$t7,$t4,$t1
/*  f123e18:	01f97025 */ 	or	$t6,$t7,$t9
/*  f123e1c:	26040008 */ 	addiu	$a0,$s0,0x8
/*  f123e20:	ae0e0004 */ 	sw	$t6,0x4($s0)
/*  f123e24:	3c18b200 */ 	lui	$t8,0xb200
/*  f123e28:	03e97824 */ 	and	$t7,$ra,$t1
/*  f123e2c:	ac8f0004 */ 	sw	$t7,0x4($a0)
/*  f123e30:	ac980000 */ 	sw	$t8,0x0($a0)
/*  f123e34:	24850008 */ 	addiu	$a1,$a0,0x8
/*  f123e38:	3c19b400 */ 	lui	$t9,0xb400
/*  f123e3c:	acb90000 */ 	sw	$t9,0x0($a1)
/*  f123e40:	8faf00d4 */ 	lw	$t7,0xd4($sp)
/*  f123e44:	8fae00d8 */ 	lw	$t6,0xd8($sp)
/*  f123e48:	24a70008 */ 	addiu	$a3,$a1,0x8
/*  f123e4c:	01e9c824 */ 	and	$t9,$t7,$t1
/*  f123e50:	01c9c024 */ 	and	$t8,$t6,$t1
/*  f123e54:	00197402 */ 	srl	$t6,$t9,0x10
/*  f123e58:	030e7825 */ 	or	$t7,$t8,$t6
/*  f123e5c:	acaf0004 */ 	sw	$t7,0x4($a1)
/*  f123e60:	8fab00d0 */ 	lw	$t3,0xd0($sp)
/*  f123e64:	3c19b200 */ 	lui	$t9,0xb200
/*  f123e68:	acf90000 */ 	sw	$t9,0x0($a3)
/*  f123e6c:	0169c024 */ 	and	$t8,$t3,$t1
/*  f123e70:	acf80004 */ 	sw	$t8,0x4($a3)
/*  f123e74:	24e30008 */ 	addiu	$v1,$a3,0x8
/*  f123e78:	3c0eb400 */ 	lui	$t6,0xb400
/*  f123e7c:	ac6e0000 */ 	sw	$t6,0x0($v1)
/*  f123e80:	31b8ffff */ 	andi	$t8,$t5,0xffff
/*  f123e84:	000ccc00 */ 	sll	$t9,$t4,0x10
/*  f123e88:	03387025 */ 	or	$t6,$t9,$t8
/*  f123e8c:	ac6e0004 */ 	sw	$t6,0x4($v1)
/*  f123e90:	24700008 */ 	addiu	$s0,$v1,0x8
/*  f123e94:	3c0fb200 */ 	lui	$t7,0xb200
/*  f123e98:	001fc400 */ 	sll	$t8,$ra,0x10
/*  f123e9c:	ae180004 */ 	sw	$t8,0x4($s0)
/*  f123ea0:	ae0f0000 */ 	sw	$t7,0x0($s0)
/*  f123ea4:	26080008 */ 	addiu	$t0,$s0,0x8
/*  f123ea8:	3c0eb400 */ 	lui	$t6,0xb400
/*  f123eac:	ad0e0000 */ 	sw	$t6,0x0($t0)
/*  f123eb0:	8fae00d4 */ 	lw	$t6,0xd4($sp)
/*  f123eb4:	8fb900d8 */ 	lw	$t9,0xd8($sp)
/*  f123eb8:	8fa700c8 */ 	lw	$a3,0xc8($sp)
/*  f123ebc:	31cfffff */ 	andi	$t7,$t6,0xffff
/*  f123ec0:	0019c400 */ 	sll	$t8,$t9,0x10
/*  f123ec4:	030fc825 */ 	or	$t9,$t8,$t7
/*  f123ec8:	ad190004 */ 	sw	$t9,0x4($t0)
/*  f123ecc:	25030008 */ 	addiu	$v1,$t0,0x8
/*  f123ed0:	3c0eb200 */ 	lui	$t6,0xb200
/*  f123ed4:	000b7c00 */ 	sll	$t7,$t3,0x10
/*  f123ed8:	ac6f0004 */ 	sw	$t7,0x4($v1)
/*  f123edc:	ac6e0000 */ 	sw	$t6,0x0($v1)
/*  f123ee0:	24640008 */ 	addiu	$a0,$v1,0x8
/*  f123ee4:	3c19b400 */ 	lui	$t9,0xb400
/*  f123ee8:	00c9c024 */ 	and	$t8,$a2,$t1
/*  f123eec:	00187c02 */ 	srl	$t7,$t8,0x10
/*  f123ef0:	ac990000 */ 	sw	$t9,0x0($a0)
/*  f123ef4:	00e97024 */ 	and	$t6,$a3,$t1
/*  f123ef8:	01cfc825 */ 	or	$t9,$t6,$t7
/*  f123efc:	ac990004 */ 	sw	$t9,0x4($a0)
/*  f123f00:	24850008 */ 	addiu	$a1,$a0,0x8
/*  f123f04:	00497024 */ 	and	$t6,$v0,$t1
/*  f123f08:	3c18b200 */ 	lui	$t8,0xb200
/*  f123f0c:	acb80000 */ 	sw	$t8,0x0($a1)
/*  f123f10:	acae0004 */ 	sw	$t6,0x4($a1)
/*  f123f14:	8fa800b4 */ 	lw	$t0,0xb4($sp)
/*  f123f18:	8fac00b8 */ 	lw	$t4,0xb8($sp)
/*  f123f1c:	00405025 */ 	or	$t2,$v0,$zero
/*  f123f20:	24b00008 */ 	addiu	$s0,$a1,0x8
/*  f123f24:	8fab00b0 */ 	lw	$t3,0xb0($sp)
/*  f123f28:	02001025 */ 	or	$v0,$s0,$zero
/*  f123f2c:	3c0fb400 */ 	lui	$t7,0xb400
/*  f123f30:	0109c024 */ 	and	$t8,$t0,$t1
/*  f123f34:	00187402 */ 	srl	$t6,$t8,0x10
/*  f123f38:	ac4f0000 */ 	sw	$t7,0x0($v0)
/*  f123f3c:	0189c824 */ 	and	$t9,$t4,$t1
/*  f123f40:	032e7825 */ 	or	$t7,$t9,$t6
/*  f123f44:	ac4f0004 */ 	sw	$t7,0x4($v0)
/*  f123f48:	26030008 */ 	addiu	$v1,$s0,0x8
/*  f123f4c:	3c18b200 */ 	lui	$t8,0xb200
/*  f123f50:	0169c824 */ 	and	$t9,$t3,$t1
/*  f123f54:	ac790004 */ 	sw	$t9,0x4($v1)
/*  f123f58:	ac780000 */ 	sw	$t8,0x0($v1)
/*  f123f5c:	24700008 */ 	addiu	$s0,$v1,0x8
/*  f123f60:	3c0eb400 */ 	lui	$t6,0xb400
/*  f123f64:	ae0e0000 */ 	sw	$t6,0x0($s0)
/*  f123f68:	30d9ffff */ 	andi	$t9,$a2,0xffff
/*  f123f6c:	0007c400 */ 	sll	$t8,$a3,0x10
/*  f123f70:	03197025 */ 	or	$t6,$t8,$t9
/*  f123f74:	ae0e0004 */ 	sw	$t6,0x4($s0)
/*  f123f78:	26050008 */ 	addiu	$a1,$s0,0x8
/*  f123f7c:	000acc00 */ 	sll	$t9,$t2,0x10
/*  f123f80:	acb90004 */ 	sw	$t9,0x4($a1)
/*  f123f84:	3c0fb200 */ 	lui	$t7,0xb200
/*  f123f88:	acaf0000 */ 	sw	$t7,0x0($a1)
/*  f123f8c:	24a30008 */ 	addiu	$v1,$a1,0x8
/*  f123f90:	3c0eb400 */ 	lui	$t6,0xb400
/*  f123f94:	ac6e0000 */ 	sw	$t6,0x0($v1)
/*  f123f98:	3119ffff */ 	andi	$t9,$t0,0xffff
/*  f123f9c:	000cc400 */ 	sll	$t8,$t4,0x10
/*  f123fa0:	03197025 */ 	or	$t6,$t8,$t9
/*  f123fa4:	24640008 */ 	addiu	$a0,$v1,0x8
/*  f123fa8:	ac6e0004 */ 	sw	$t6,0x4($v1)
/*  f123fac:	3c0fb300 */ 	lui	$t7,0xb300
/*  f123fb0:	000bcc00 */ 	sll	$t9,$t3,0x10
/*  f123fb4:	ac990004 */ 	sw	$t9,0x4($a0)
/*  f123fb8:	ac8f0000 */ 	sw	$t7,0x0($a0)
/*  f123fbc:	24820008 */ 	addiu	$v0,$a0,0x8
.L0f123fc0:
/*  f123fc0:	8fbf0024 */ 	lw	$ra,0x24($sp)
/*  f123fc4:	d7b40018 */ 	ldc1	$f20,0x18($sp)
/*  f123fc8:	8fb00020 */ 	lw	$s0,0x20($sp)
/*  f123fcc:	03e00008 */ 	jr	$ra
/*  f123fd0:	27bd0488 */ 	addiu	$sp,$sp,0x488
);

GLOBAL_ASM(
glabel sky0f123fd4
.late_rodata
glabel var7f1b50f0
.word 0xc4eac000
glabel var7f1b50f4
.word 0xc4eac000
glabel var7f1b50f8
.word 0xc4eac000
glabel var7f1b50fc
.word 0xc4eac000
glabel var7f1b5100
.word 0x44eaa000
glabel var7f1b5104
.word 0x46fffe00
.text
/*  f123fd4:	27bdfb30 */ 	addiu	$sp,$sp,-1232
/*  f123fd8:	afb00020 */ 	sw	$s0,0x20($sp)
/*  f123fdc:	00808025 */ 	or	$s0,$a0,$zero
/*  f123fe0:	afbf0024 */ 	sw	$ra,0x24($sp)
/*  f123fe4:	afa504d4 */ 	sw	$a1,0x4d4($sp)
/*  f123fe8:	00a02025 */ 	or	$a0,$a1,$zero
/*  f123fec:	f7b40018 */ 	sdc1	$f20,0x18($sp)
/*  f123ff0:	afa604d8 */ 	sw	$a2,0x4d8($sp)
/*  f123ff4:	afa704dc */ 	sw	$a3,0x4dc($sp)
/*  f123ff8:	0fc48b3a */ 	jal	sky0f122ce8
/*  f123ffc:	00c02825 */ 	or	$a1,$a2,$zero
/*  f124000:	14400015 */ 	bnez	$v0,.L0f124058
/*  f124004:	8fa404d8 */ 	lw	$a0,0x4d8($sp)
/*  f124008:	0fc48b3a */ 	jal	sky0f122ce8
/*  f12400c:	8fa504dc */ 	lw	$a1,0x4dc($sp)
/*  f124010:	14400011 */ 	bnez	$v0,.L0f124058
/*  f124014:	8fa404dc */ 	lw	$a0,0x4dc($sp)
/*  f124018:	0fc48b3a */ 	jal	sky0f122ce8
/*  f12401c:	8fa504d4 */ 	lw	$a1,0x4d4($sp)
/*  f124020:	1440000d */ 	bnez	$v0,.L0f124058
/*  f124024:	8fa404e0 */ 	lw	$a0,0x4e0($sp)
/*  f124028:	0fc48b3a */ 	jal	sky0f122ce8
/*  f12402c:	8fa504d4 */ 	lw	$a1,0x4d4($sp)
/*  f124030:	14400009 */ 	bnez	$v0,.L0f124058
/*  f124034:	8fa404e0 */ 	lw	$a0,0x4e0($sp)
/*  f124038:	0fc48b3a */ 	jal	sky0f122ce8
/*  f12403c:	8fa504d8 */ 	lw	$a1,0x4d8($sp)
/*  f124040:	14400005 */ 	bnez	$v0,.L0f124058
/*  f124044:	8fa404e0 */ 	lw	$a0,0x4e0($sp)
/*  f124048:	0fc48b3a */ 	jal	sky0f122ce8
/*  f12404c:	8fa504dc */ 	lw	$a1,0x4dc($sp)
/*  f124050:	10400003 */ 	beqz	$v0,.L0f124060
/*  f124054:	c7aa04e4 */ 	lwc1	$f10,0x4e4($sp)
.L0f124058:
/*  f124058:	10000636 */ 	b	.L0f125934
/*  f12405c:	02001025 */ 	or	$v0,$s0,$zero
.L0f124060:
/*  f124060:	3c013780 */ 	lui	$at,0x3780
/*  f124064:	44812000 */ 	mtc1	$at,$f4
/*  f124068:	8fa304d4 */ 	lw	$v1,0x4d4($sp)
/*  f12406c:	8fa404dc */ 	lw	$a0,0x4dc($sp)
/*  f124070:	46045202 */ 	mul.s	$f8,$f10,$f4
/*  f124074:	8fa904d8 */ 	lw	$t1,0x4d8($sp)
/*  f124078:	00603825 */ 	or	$a3,$v1,$zero
/*  f12407c:	00804025 */ 	or	$t0,$a0,$zero
/*  f124080:	01202825 */ 	or	$a1,$t1,$zero
/*  f124084:	e7a803c0 */ 	swc1	$f8,0x3c0($sp)
/*  f124088:	c46c0028 */ 	lwc1	$f12,0x28($v1)
/*  f12408c:	c48a0028 */ 	lwc1	$f10,0x28($a0)
/*  f124090:	c460002c */ 	lwc1	$f0,0x2c($v1)
/*  f124094:	c52e002c */ 	lwc1	$f14,0x2c($t1)
/*  f124098:	460c5101 */ 	sub.s	$f4,$f10,$f12
/*  f12409c:	c5260028 */ 	lwc1	$f6,0x28($t1)
/*  f1240a0:	46007081 */ 	sub.s	$f2,$f14,$f0
/*  f1240a4:	e7a404b0 */ 	swc1	$f4,0x4b0($sp)
/*  f1240a8:	c494002c */ 	lwc1	$f20,0x2c($a0)
/*  f1240ac:	c7a804b0 */ 	lwc1	$f8,0x4b0($sp)
/*  f1240b0:	460c3481 */ 	sub.s	$f18,$f6,$f12
/*  f1240b4:	46024182 */ 	mul.s	$f6,$f8,$f2
/*  f1240b8:	4600a401 */ 	sub.s	$f16,$f20,$f0
/*  f1240bc:	44814000 */ 	mtc1	$at,$f8
/*  f1240c0:	3c013f80 */ 	lui	$at,0x3f80
/*  f1240c4:	46109282 */ 	mul.s	$f10,$f18,$f16
/*  f1240c8:	4600703c */ 	c.lt.s	$f14,$f0
/*  f1240cc:	460a3101 */ 	sub.s	$f4,$f6,$f10
/*  f1240d0:	44815000 */ 	mtc1	$at,$f10
/*  f1240d4:	46082182 */ 	mul.s	$f6,$f4,$f8
/*  f1240d8:	46065103 */ 	div.s	$f4,$f10,$f6
/*  f1240dc:	e7a60054 */ 	swc1	$f6,0x54($sp)
/*  f1240e0:	e7a60488 */ 	swc1	$f6,0x488($sp)
/*  f1240e4:	e7a40050 */ 	swc1	$f4,0x50($sp)
/*  f1240e8:	4500000a */ 	bc1f	.L0f124114
/*  f1240ec:	e7a40484 */ 	swc1	$f4,0x484($sp)
/*  f1240f0:	3c01bf80 */ 	lui	$at,0xbf80
/*  f1240f4:	44811000 */ 	mtc1	$at,$f2
/*  f1240f8:	00602825 */ 	or	$a1,$v1,$zero
/*  f1240fc:	01203825 */ 	or	$a3,$t1,$zero
/*  f124100:	46023302 */ 	mul.s	$f12,$f6,$f2
/*  f124104:	00000000 */ 	nop
/*  f124108:	46022382 */ 	mul.s	$f14,$f4,$f2
/*  f12410c:	e7ac0488 */ 	swc1	$f12,0x488($sp)
/*  f124110:	e7ae0484 */ 	swc1	$f14,0x484($sp)
.L0f124114:
/*  f124114:	c4a0002c */ 	lwc1	$f0,0x2c($a1)
/*  f124118:	3c01bf80 */ 	lui	$at,0xbf80
/*  f12411c:	44811000 */ 	mtc1	$at,$f2
/*  f124120:	4600a03c */ 	c.lt.s	$f20,$f0
/*  f124124:	c7ac0488 */ 	lwc1	$f12,0x488($sp)
/*  f124128:	c7ae0484 */ 	lwc1	$f14,0x484($sp)
/*  f12412c:	3c014080 */ 	lui	$at,0x4080
/*  f124130:	45020006 */ 	bc1fl	.L0f12414c
/*  f124134:	e7ae0484 */ 	swc1	$f14,0x484($sp)
/*  f124138:	00a04025 */ 	or	$t0,$a1,$zero
/*  f12413c:	46027382 */ 	mul.s	$f14,$f14,$f2
/*  f124140:	00802825 */ 	or	$a1,$a0,$zero
/*  f124144:	c480002c */ 	lwc1	$f0,0x2c($a0)
/*  f124148:	e7ae0484 */ 	swc1	$f14,0x484($sp)
.L0f12414c:
/*  f12414c:	c4e8002c */ 	lwc1	$f8,0x2c($a3)
/*  f124150:	4608003c */ 	c.lt.s	$f0,$f8
/*  f124154:	44810000 */ 	mtc1	$at,$f0
/*  f124158:	3c013e80 */ 	lui	$at,0x3e80
/*  f12415c:	44813000 */ 	mtc1	$at,$f6
/*  f124160:	45020007 */ 	bc1fl	.L0f124180
/*  f124164:	c4aa0028 */ 	lwc1	$f10,0x28($a1)
/*  f124168:	46027382 */ 	mul.s	$f14,$f14,$f2
/*  f12416c:	00a01025 */ 	or	$v0,$a1,$zero
/*  f124170:	00e02825 */ 	or	$a1,$a3,$zero
/*  f124174:	00403825 */ 	or	$a3,$v0,$zero
/*  f124178:	e7ae0484 */ 	swc1	$f14,0x484($sp)
/*  f12417c:	c4aa0028 */ 	lwc1	$f10,0x28($a1)
.L0f124180:
/*  f124180:	4480a000 */ 	mtc1	$zero,$f20
/*  f124184:	3c0644ea */ 	lui	$a2,0x44ea
/*  f124188:	46065102 */ 	mul.s	$f4,$f10,$f6
/*  f12418c:	e7b40468 */ 	swc1	$f20,0x468($sp)
/*  f124190:	44815000 */ 	mtc1	$at,$f10
/*  f124194:	34c6a000 */ 	ori	$a2,$a2,0xa000
/*  f124198:	e7a40464 */ 	swc1	$f4,0x464($sp)
/*  f12419c:	c4e80028 */ 	lwc1	$f8,0x28($a3)
/*  f1241a0:	e7b40470 */ 	swc1	$f20,0x470($sp)
/*  f1241a4:	460a4182 */ 	mul.s	$f6,$f8,$f10
/*  f1241a8:	44814000 */ 	mtc1	$at,$f8
/*  f1241ac:	e7a6046c */ 	swc1	$f6,0x46c($sp)
/*  f1241b0:	c4e40028 */ 	lwc1	$f4,0x28($a3)
/*  f1241b4:	e7b40478 */ 	swc1	$f20,0x478($sp)
/*  f1241b8:	e7b40480 */ 	swc1	$f20,0x480($sp)
/*  f1241bc:	46082282 */ 	mul.s	$f10,$f4,$f8
/*  f1241c0:	e7b4047c */ 	swc1	$f20,0x47c($sp)
/*  f1241c4:	e7aa0474 */ 	swc1	$f10,0x474($sp)
/*  f1241c8:	c5060028 */ 	lwc1	$f6,0x28($t0)
/*  f1241cc:	e7a6048c */ 	swc1	$f6,0x48c($sp)
/*  f1241d0:	c504002c */ 	lwc1	$f4,0x2c($t0)
/*  f1241d4:	e7a40490 */ 	swc1	$f4,0x490($sp)
/*  f1241d8:	c4a80028 */ 	lwc1	$f8,0x28($a1)
/*  f1241dc:	e7a80494 */ 	swc1	$f8,0x494($sp)
/*  f1241e0:	c4aa002c */ 	lwc1	$f10,0x2c($a1)
/*  f1241e4:	c7a80494 */ 	lwc1	$f8,0x494($sp)
/*  f1241e8:	e7aa0498 */ 	swc1	$f10,0x498($sp)
/*  f1241ec:	c4e60028 */ 	lwc1	$f6,0x28($a3)
/*  f1241f0:	e7a6049c */ 	swc1	$f6,0x49c($sp)
/*  f1241f4:	c4e4002c */ 	lwc1	$f4,0x2c($a3)
/*  f1241f8:	c7a60498 */ 	lwc1	$f6,0x498($sp)
/*  f1241fc:	c7aa049c */ 	lwc1	$f10,0x49c($sp)
/*  f124200:	e7a404a0 */ 	swc1	$f4,0x4a0($sp)
/*  f124204:	c7a404a0 */ 	lwc1	$f4,0x4a0($sp)
/*  f124208:	e7a6002c */ 	swc1	$f6,0x2c($sp)
/*  f12420c:	e7a80028 */ 	swc1	$f8,0x28($sp)
/*  f124210:	46043081 */ 	sub.s	$f2,$f6,$f4
/*  f124214:	c7a60490 */ 	lwc1	$f6,0x490($sp)
/*  f124218:	afa804c4 */ 	sw	$t0,0x4c4($sp)
/*  f12421c:	460a4481 */ 	sub.s	$f18,$f8,$f10
/*  f124220:	c7a8048c */ 	lwc1	$f8,0x48c($sp)
/*  f124224:	afa704cc */ 	sw	$a3,0x4cc($sp)
/*  f124228:	46043401 */ 	sub.s	$f16,$f6,$f4
/*  f12422c:	c7a40028 */ 	lwc1	$f4,0x28($sp)
/*  f124230:	afa504c8 */ 	sw	$a1,0x4c8($sp)
/*  f124234:	460a4281 */ 	sub.s	$f10,$f8,$f10
/*  f124238:	e7b40440 */ 	swc1	$f20,0x440($sp)
/*  f12423c:	e7b4043c */ 	swc1	$f20,0x43c($sp)
/*  f124240:	46044301 */ 	sub.s	$f12,$f8,$f4
/*  f124244:	c7a8002c */ 	lwc1	$f8,0x2c($sp)
/*  f124248:	44812000 */ 	mtc1	$at,$f4
/*  f12424c:	e7aa04b0 */ 	swc1	$f10,0x4b0($sp)
/*  f124250:	46083381 */ 	sub.s	$f14,$f6,$f8
/*  f124254:	46046182 */ 	mul.s	$f6,$f12,$f4
/*  f124258:	44814000 */ 	mtc1	$at,$f8
/*  f12425c:	e7ac04a8 */ 	swc1	$f12,0x4a8($sp)
/*  f124260:	e7ae04a4 */ 	swc1	$f14,0x4a4($sp)
/*  f124264:	46087102 */ 	mul.s	$f4,$f14,$f8
/*  f124268:	e7b40420 */ 	swc1	$f20,0x420($sp)
/*  f12426c:	e7b4041c */ 	swc1	$f20,0x41c($sp)
/*  f124270:	e7a60424 */ 	swc1	$f6,0x424($sp)
/*  f124274:	44813000 */ 	mtc1	$at,$f6
/*  f124278:	e7b403e4 */ 	swc1	$f20,0x3e4($sp)
/*  f12427c:	e7b403ec */ 	swc1	$f20,0x3ec($sp)
/*  f124280:	46069202 */ 	mul.s	$f8,$f18,$f6
/*  f124284:	e7a40428 */ 	swc1	$f4,0x428($sp)
/*  f124288:	44812000 */ 	mtc1	$at,$f4
/*  f12428c:	e7b403f4 */ 	swc1	$f20,0x3f4($sp)
/*  f124290:	e7b40400 */ 	swc1	$f20,0x400($sp)
/*  f124294:	46041182 */ 	mul.s	$f6,$f2,$f4
/*  f124298:	e7b403fc */ 	swc1	$f20,0x3fc($sp)
/*  f12429c:	e7a8042c */ 	swc1	$f8,0x42c($sp)
/*  f1242a0:	44814000 */ 	mtc1	$at,$f8
/*  f1242a4:	e7b403c4 */ 	swc1	$f20,0x3c4($sp)
/*  f1242a8:	e7b403cc */ 	swc1	$f20,0x3cc($sp)
/*  f1242ac:	46085102 */ 	mul.s	$f4,$f10,$f8
/*  f1242b0:	e7a60430 */ 	swc1	$f6,0x430($sp)
/*  f1242b4:	44813000 */ 	mtc1	$at,$f6
/*  f1242b8:	3c017f1b */ 	lui	$at,%hi(var7f1b50f0)
/*  f1242bc:	e7b403d4 */ 	swc1	$f20,0x3d4($sp)
/*  f1242c0:	46068202 */ 	mul.s	$f8,$f16,$f6
/*  f1242c4:	e7b403e0 */ 	swc1	$f20,0x3e0($sp)
/*  f1242c8:	e7a40434 */ 	swc1	$f4,0x434($sp)
/*  f1242cc:	46006102 */ 	mul.s	$f4,$f12,$f0
/*  f1242d0:	e7b403dc */ 	swc1	$f20,0x3dc($sp)
/*  f1242d4:	46007182 */ 	mul.s	$f6,$f14,$f0
/*  f1242d8:	e7a80438 */ 	swc1	$f8,0x438($sp)
/*  f1242dc:	c42e50f0 */ 	lwc1	$f14,%lo(var7f1b50f0)($at)
/*  f1242e0:	46009202 */ 	mul.s	$f8,$f18,$f0
/*  f1242e4:	e7a40404 */ 	swc1	$f4,0x404($sp)
/*  f1242e8:	46001102 */ 	mul.s	$f4,$f2,$f0
/*  f1242ec:	e7a60408 */ 	swc1	$f6,0x408($sp)
/*  f1242f0:	46005182 */ 	mul.s	$f6,$f10,$f0
/*  f1242f4:	e7a8040c */ 	swc1	$f8,0x40c($sp)
/*  f1242f8:	46008202 */ 	mul.s	$f8,$f16,$f0
/*  f1242fc:	e7a40410 */ 	swc1	$f4,0x410($sp)
/*  f124300:	c7a404a4 */ 	lwc1	$f4,0x4a4($sp)
/*  f124304:	e7a60414 */ 	swc1	$f6,0x414($sp)
/*  f124308:	46040183 */ 	div.s	$f6,$f0,$f4
/*  f12430c:	e7a80418 */ 	swc1	$f8,0x418($sp)
/*  f124310:	46020203 */ 	div.s	$f8,$f0,$f2
/*  f124314:	e7a603e8 */ 	swc1	$f6,0x3e8($sp)
/*  f124318:	46100183 */ 	div.s	$f6,$f0,$f16
/*  f12431c:	e7a803f0 */ 	swc1	$f8,0x3f0($sp)
/*  f124320:	c7a804a8 */ 	lwc1	$f8,0x4a8($sp)
/*  f124324:	46044303 */ 	div.s	$f12,$f8,$f4
/*  f124328:	e7a603f8 */ 	swc1	$f6,0x3f8($sp)
/*  f12432c:	46029183 */ 	div.s	$f6,$f18,$f2
/*  f124330:	e7ac03c8 */ 	swc1	$f12,0x3c8($sp)
/*  f124334:	46105203 */ 	div.s	$f8,$f10,$f16
/*  f124338:	e7a603d0 */ 	swc1	$f6,0x3d0($sp)
/*  f12433c:	0fc47cf4 */ 	jal	skyClamp
/*  f124340:	e7a803d8 */ 	swc1	$f8,0x3d8($sp)
/*  f124344:	3c017f1b */ 	lui	$at,%hi(var7f1b50f4)
/*  f124348:	3c0644ea */ 	lui	$a2,0x44ea
/*  f12434c:	e7a003c8 */ 	swc1	$f0,0x3c8($sp)
/*  f124350:	34c6a000 */ 	ori	$a2,$a2,0xa000
/*  f124354:	c42e50f4 */ 	lwc1	$f14,%lo(var7f1b50f4)($at)
/*  f124358:	0fc47cf4 */ 	jal	skyClamp
/*  f12435c:	c7ac03d0 */ 	lwc1	$f12,0x3d0($sp)
/*  f124360:	3c017f1b */ 	lui	$at,%hi(var7f1b50f8)
/*  f124364:	3c0644ea */ 	lui	$a2,0x44ea
/*  f124368:	e7a003d0 */ 	swc1	$f0,0x3d0($sp)
/*  f12436c:	34c6a000 */ 	ori	$a2,$a2,0xa000
/*  f124370:	c42e50f8 */ 	lwc1	$f14,%lo(var7f1b50f8)($at)
/*  f124374:	0fc47cf4 */ 	jal	skyClamp
/*  f124378:	c7ac03d8 */ 	lwc1	$f12,0x3d8($sp)
/*  f12437c:	c7a4046c */ 	lwc1	$f4,0x46c($sp)
/*  f124380:	c7a60474 */ 	lwc1	$f6,0x474($sp)
/*  f124384:	8fae04d4 */ 	lw	$t6,0x4d4($sp)
/*  f124388:	8fb804d8 */ 	lw	$t8,0x4d8($sp)
/*  f12438c:	e7a003d8 */ 	swc1	$f0,0x3d8($sp)
/*  f124390:	e7a4044c */ 	swc1	$f4,0x44c($sp)
/*  f124394:	e7a60454 */ 	swc1	$f6,0x454($sp)
/*  f124398:	c5ca0028 */ 	lwc1	$f10,0x28($t6)
/*  f12439c:	c7080028 */ 	lwc1	$f8,0x28($t8)
/*  f1243a0:	4608503c */ 	c.lt.s	$f10,$f8
/*  f1243a4:	00000000 */ 	nop
/*  f1243a8:	4502009d */ 	bc1fl	.L0f124620
/*  f1243ac:	8fae04dc */ 	lw	$t6,0x4dc($sp)
/*  f1243b0:	8fb904dc */ 	lw	$t9,0x4dc($sp)
/*  f1243b4:	8faf04e0 */ 	lw	$t7,0x4e0($sp)
/*  f1243b8:	3c013f80 */ 	lui	$at,0x3f80
/*  f1243bc:	c724002c */ 	lwc1	$f4,0x2c($t9)
/*  f1243c0:	c5e6002c */ 	lwc1	$f6,0x2c($t7)
/*  f1243c4:	44814000 */ 	mtc1	$at,$f8
/*  f1243c8:	46062281 */ 	sub.s	$f10,$f4,$f6
/*  f1243cc:	4608503c */ 	c.lt.s	$f10,$f8
/*  f1243d0:	00000000 */ 	nop
/*  f1243d4:	45000004 */ 	bc1f	.L0f1243e8
/*  f1243d8:	3c017f1b */ 	lui	$at,%hi(var7f1b50fc)
/*  f1243dc:	c42450fc */ 	lwc1	$f4,%lo(var7f1b50fc)($at)
/*  f1243e0:	10000010 */ 	b	.L0f124424
/*  f1243e4:	e7a401bc */ 	swc1	$f4,0x1bc($sp)
.L0f1243e8:
/*  f1243e8:	0fc2d5f6 */ 	jal	camGetScreenWidth
/*  f1243ec:	00000000 */ 	nop
/*  f1243f0:	8fae04dc */ 	lw	$t6,0x4dc($sp)
/*  f1243f4:	8fb804e0 */ 	lw	$t8,0x4e0($sp)
/*  f1243f8:	3c013e80 */ 	lui	$at,0x3e80
/*  f1243fc:	c5c6002c */ 	lwc1	$f6,0x2c($t6)
/*  f124400:	c70a002c */ 	lwc1	$f10,0x2c($t8)
/*  f124404:	44812000 */ 	mtc1	$at,$f4
/*  f124408:	460a3201 */ 	sub.s	$f8,$f6,$f10
/*  f12440c:	44815000 */ 	mtc1	$at,$f10
/*  f124410:	46044182 */ 	mul.s	$f6,$f8,$f4
/*  f124414:	460a0201 */ 	sub.s	$f8,$f0,$f10
/*  f124418:	46004107 */ 	neg.s	$f4,$f8
/*  f12441c:	46062283 */ 	div.s	$f10,$f4,$f6
/*  f124420:	e7aa01bc */ 	swc1	$f10,0x1bc($sp)
.L0f124424:
/*  f124424:	3c05b400 */ 	lui	$a1,0xb400
/*  f124428:	02001025 */ 	or	$v0,$s0,$zero
/*  f12442c:	ac450000 */ 	sw	$a1,0x0($v0)
/*  f124430:	8fb904dc */ 	lw	$t9,0x4dc($sp)
/*  f124434:	444ff800 */ 	cfc1	$t7,$31
/*  f124438:	240e0001 */ 	addiu	$t6,$zero,0x1
/*  f12443c:	44cef800 */ 	ctc1	$t6,$31
/*  f124440:	c728002c */ 	lwc1	$f8,0x2c($t9)
/*  f124444:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f124448:	02001825 */ 	or	$v1,$s0,$zero
/*  f12444c:	46004124 */ 	cvt.w.s	$f4,$f8
/*  f124450:	3c19b200 */ 	lui	$t9,0xb200
/*  f124454:	444ef800 */ 	cfc1	$t6,$31
/*  f124458:	00000000 */ 	nop
/*  f12445c:	31ce0078 */ 	andi	$t6,$t6,0x78
/*  f124460:	11c00012 */ 	beqz	$t6,.L0f1244ac
/*  f124464:	3c014f00 */ 	lui	$at,0x4f00
/*  f124468:	44812000 */ 	mtc1	$at,$f4
/*  f12446c:	240e0001 */ 	addiu	$t6,$zero,0x1
/*  f124470:	46044101 */ 	sub.s	$f4,$f8,$f4
/*  f124474:	44cef800 */ 	ctc1	$t6,$31
/*  f124478:	00000000 */ 	nop
/*  f12447c:	46002124 */ 	cvt.w.s	$f4,$f4
/*  f124480:	444ef800 */ 	cfc1	$t6,$31
/*  f124484:	00000000 */ 	nop
/*  f124488:	31ce0078 */ 	andi	$t6,$t6,0x78
/*  f12448c:	15c00005 */ 	bnez	$t6,.L0f1244a4
/*  f124490:	00000000 */ 	nop
/*  f124494:	440e2000 */ 	mfc1	$t6,$f4
/*  f124498:	3c018000 */ 	lui	$at,0x8000
/*  f12449c:	10000007 */ 	b	.L0f1244bc
/*  f1244a0:	01c17025 */ 	or	$t6,$t6,$at
.L0f1244a4:
/*  f1244a4:	10000005 */ 	b	.L0f1244bc
/*  f1244a8:	240effff */ 	addiu	$t6,$zero,-1
.L0f1244ac:
/*  f1244ac:	440e2000 */ 	mfc1	$t6,$f4
/*  f1244b0:	00000000 */ 	nop
/*  f1244b4:	05c0fffb */ 	bltz	$t6,.L0f1244a4
/*  f1244b8:	00000000 */ 	nop
.L0f1244bc:
/*  f1244bc:	3c01ce80 */ 	lui	$at,0xce80
/*  f1244c0:	01c1c025 */ 	or	$t8,$t6,$at
/*  f1244c4:	ac580004 */ 	sw	$t8,0x4($v0)
/*  f1244c8:	44cff800 */ 	ctc1	$t7,$31
/*  f1244cc:	ac790000 */ 	sw	$t9,0x0($v1)
/*  f1244d0:	8faf04e0 */ 	lw	$t7,0x4e0($sp)
/*  f1244d4:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1244d8:	02002025 */ 	or	$a0,$s0,$zero
/*  f1244dc:	c5e6002c */ 	lwc1	$f6,0x2c($t7)
/*  f1244e0:	8faf04d4 */ 	lw	$t7,0x4d4($sp)
/*  f1244e4:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1244e8:	4600328d */ 	trunc.w.s	$f10,$f6
/*  f1244ec:	c5e8002c */ 	lwc1	$f8,0x2c($t7)
/*  f1244f0:	4600410d */ 	trunc.w.s	$f4,$f8
/*  f1244f4:	44185000 */ 	mfc1	$t8,$f10
/*  f1244f8:	00000000 */ 	nop
/*  f1244fc:	0018cc00 */ 	sll	$t9,$t8,0x10
/*  f124500:	44182000 */ 	mfc1	$t8,$f4
/*  f124504:	00000000 */ 	nop
/*  f124508:	03387825 */ 	or	$t7,$t9,$t8
/*  f12450c:	ac6f0004 */ 	sw	$t7,0x4($v1)
/*  f124510:	ac850000 */ 	sw	$a1,0x0($a0)
/*  f124514:	0fc2d5fe */ 	jal	camGetScreenLeft
/*  f124518:	afa401b0 */ 	sw	$a0,0x1b0($sp)
/*  f12451c:	0fc2d5f6 */ 	jal	camGetScreenWidth
/*  f124520:	e7a0005c */ 	swc1	$f0,0x5c($sp)
/*  f124524:	c7a6005c */ 	lwc1	$f6,0x5c($sp)
/*  f124528:	3c013e80 */ 	lui	$at,0x3e80
/*  f12452c:	44814000 */ 	mtc1	$at,$f8
/*  f124530:	46060280 */ 	add.s	$f10,$f0,$f6
/*  f124534:	0fc54be8 */ 	jal	func0f152fa0
/*  f124538:	46085301 */ 	sub.s	$f12,$f10,$f8
/*  f12453c:	8fae01b0 */ 	lw	$t6,0x1b0($sp)
/*  f124540:	02001825 */ 	or	$v1,$s0,$zero
/*  f124544:	3c19b200 */ 	lui	$t9,0xb200
/*  f124548:	adc20004 */ 	sw	$v0,0x4($t6)
/*  f12454c:	ac790000 */ 	sw	$t9,0x0($v1)
/*  f124550:	afa301ac */ 	sw	$v1,0x1ac($sp)
/*  f124554:	c7ac01bc */ 	lwc1	$f12,0x1bc($sp)
/*  f124558:	0fc54be8 */ 	jal	func0f152fa0
/*  f12455c:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f124560:	8fa301ac */ 	lw	$v1,0x1ac($sp)
/*  f124564:	02002025 */ 	or	$a0,$s0,$zero
/*  f124568:	3c18b400 */ 	lui	$t8,0xb400
/*  f12456c:	ac620004 */ 	sw	$v0,0x4($v1)
/*  f124570:	ac980000 */ 	sw	$t8,0x0($a0)
/*  f124574:	afa401a8 */ 	sw	$a0,0x1a8($sp)
/*  f124578:	0fc2d5fe */ 	jal	camGetScreenLeft
/*  f12457c:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f124580:	0fc54be8 */ 	jal	func0f152fa0
/*  f124584:	46000306 */ 	mov.s	$f12,$f0
/*  f124588:	8faf01a8 */ 	lw	$t7,0x1a8($sp)
/*  f12458c:	02001825 */ 	or	$v1,$s0,$zero
/*  f124590:	3c0eb200 */ 	lui	$t6,0xb200
/*  f124594:	ade20004 */ 	sw	$v0,0x4($t7)
/*  f124598:	ac6e0000 */ 	sw	$t6,0x0($v1)
/*  f12459c:	afa301a4 */ 	sw	$v1,0x1a4($sp)
/*  f1245a0:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1245a4:	0fc54be8 */ 	jal	func0f152fa0
/*  f1245a8:	4600a306 */ 	mov.s	$f12,$f20
/*  f1245ac:	8fa301a4 */ 	lw	$v1,0x1a4($sp)
/*  f1245b0:	02002025 */ 	or	$a0,$s0,$zero
/*  f1245b4:	3c19b400 */ 	lui	$t9,0xb400
/*  f1245b8:	ac620004 */ 	sw	$v0,0x4($v1)
/*  f1245bc:	ac990000 */ 	sw	$t9,0x0($a0)
/*  f1245c0:	afa401a0 */ 	sw	$a0,0x1a0($sp)
/*  f1245c4:	0fc2d5fe */ 	jal	camGetScreenLeft
/*  f1245c8:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1245cc:	0fc2d5f6 */ 	jal	camGetScreenWidth
/*  f1245d0:	e7a0005c */ 	swc1	$f0,0x5c($sp)
/*  f1245d4:	c7a4005c */ 	lwc1	$f4,0x5c($sp)
/*  f1245d8:	3c013e80 */ 	lui	$at,0x3e80
/*  f1245dc:	44815000 */ 	mtc1	$at,$f10
/*  f1245e0:	46040180 */ 	add.s	$f6,$f0,$f4
/*  f1245e4:	0fc54be8 */ 	jal	func0f152fa0
/*  f1245e8:	460a3301 */ 	sub.s	$f12,$f6,$f10
/*  f1245ec:	8fb801a0 */ 	lw	$t8,0x1a0($sp)
/*  f1245f0:	02001825 */ 	or	$v1,$s0,$zero
/*  f1245f4:	3c0fb200 */ 	lui	$t7,0xb200
/*  f1245f8:	af020004 */ 	sw	$v0,0x4($t8)
/*  f1245fc:	ac6f0000 */ 	sw	$t7,0x0($v1)
/*  f124600:	afa3019c */ 	sw	$v1,0x19c($sp)
/*  f124604:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f124608:	0fc54be8 */ 	jal	func0f152fa0
/*  f12460c:	4600a306 */ 	mov.s	$f12,$f20
/*  f124610:	8fa3019c */ 	lw	$v1,0x19c($sp)
/*  f124614:	10000094 */ 	b	.L0f124868
/*  f124618:	ac620004 */ 	sw	$v0,0x4($v1)
/*  f12461c:	8fae04dc */ 	lw	$t6,0x4dc($sp)
.L0f124620:
/*  f124620:	8fb904e0 */ 	lw	$t9,0x4e0($sp)
/*  f124624:	3c013f80 */ 	lui	$at,0x3f80
/*  f124628:	c5c8002c */ 	lwc1	$f8,0x2c($t6)
/*  f12462c:	c724002c */ 	lwc1	$f4,0x2c($t9)
/*  f124630:	44815000 */ 	mtc1	$at,$f10
/*  f124634:	46044181 */ 	sub.s	$f6,$f8,$f4
/*  f124638:	460a303c */ 	c.lt.s	$f6,$f10
/*  f12463c:	00000000 */ 	nop
/*  f124640:	45000003 */ 	bc1f	.L0f124650
/*  f124644:	3c017f1b */ 	lui	$at,%hi(var7f1b5100)
/*  f124648:	1000000e */ 	b	.L0f124684
/*  f12464c:	c42e5100 */ 	lwc1	$f14,%lo(var7f1b5100)($at)
.L0f124650:
/*  f124650:	0fc2d5f6 */ 	jal	camGetScreenWidth
/*  f124654:	00000000 */ 	nop
/*  f124658:	8fb804dc */ 	lw	$t8,0x4dc($sp)
/*  f12465c:	8faf04e0 */ 	lw	$t7,0x4e0($sp)
/*  f124660:	3c013e80 */ 	lui	$at,0x3e80
/*  f124664:	c708002c */ 	lwc1	$f8,0x2c($t8)
/*  f124668:	c5e4002c */ 	lwc1	$f4,0x2c($t7)
/*  f12466c:	44815000 */ 	mtc1	$at,$f10
/*  f124670:	46044181 */ 	sub.s	$f6,$f8,$f4
/*  f124674:	44812000 */ 	mtc1	$at,$f4
/*  f124678:	460a3202 */ 	mul.s	$f8,$f6,$f10
/*  f12467c:	46040181 */ 	sub.s	$f6,$f0,$f4
/*  f124680:	46083383 */ 	div.s	$f14,$f6,$f8
.L0f124684:
/*  f124684:	02001025 */ 	or	$v0,$s0,$zero
/*  f124688:	3c0eb400 */ 	lui	$t6,0xb400
/*  f12468c:	ac4e0000 */ 	sw	$t6,0x0($v0)
/*  f124690:	8fb904dc */ 	lw	$t9,0x4dc($sp)
/*  f124694:	4458f800 */ 	cfc1	$t8,$31
/*  f124698:	240f0001 */ 	addiu	$t7,$zero,0x1
/*  f12469c:	44cff800 */ 	ctc1	$t7,$31
/*  f1246a0:	c72a002c */ 	lwc1	$f10,0x2c($t9)
/*  f1246a4:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1246a8:	02001825 */ 	or	$v1,$s0,$zero
/*  f1246ac:	46005124 */ 	cvt.w.s	$f4,$f10
/*  f1246b0:	3c19b200 */ 	lui	$t9,0xb200
/*  f1246b4:	444ff800 */ 	cfc1	$t7,$31
/*  f1246b8:	00000000 */ 	nop
/*  f1246bc:	31ef0078 */ 	andi	$t7,$t7,0x78
/*  f1246c0:	11e00012 */ 	beqz	$t7,.L0f12470c
/*  f1246c4:	3c014f00 */ 	lui	$at,0x4f00
/*  f1246c8:	44812000 */ 	mtc1	$at,$f4
/*  f1246cc:	240f0001 */ 	addiu	$t7,$zero,0x1
/*  f1246d0:	46045101 */ 	sub.s	$f4,$f10,$f4
/*  f1246d4:	44cff800 */ 	ctc1	$t7,$31
/*  f1246d8:	00000000 */ 	nop
/*  f1246dc:	46002124 */ 	cvt.w.s	$f4,$f4
/*  f1246e0:	444ff800 */ 	cfc1	$t7,$31
/*  f1246e4:	00000000 */ 	nop
/*  f1246e8:	31ef0078 */ 	andi	$t7,$t7,0x78
/*  f1246ec:	15e00005 */ 	bnez	$t7,.L0f124704
/*  f1246f0:	00000000 */ 	nop
/*  f1246f4:	440f2000 */ 	mfc1	$t7,$f4
/*  f1246f8:	3c018000 */ 	lui	$at,0x8000
/*  f1246fc:	10000007 */ 	b	.L0f12471c
/*  f124700:	01e17825 */ 	or	$t7,$t7,$at
.L0f124704:
/*  f124704:	10000005 */ 	b	.L0f12471c
/*  f124708:	240fffff */ 	addiu	$t7,$zero,-1
.L0f12470c:
/*  f12470c:	440f2000 */ 	mfc1	$t7,$f4
/*  f124710:	00000000 */ 	nop
/*  f124714:	05e0fffb */ 	bltz	$t7,.L0f124704
/*  f124718:	00000000 */ 	nop
.L0f12471c:
/*  f12471c:	3c01ce00 */ 	lui	$at,0xce00
/*  f124720:	01e17025 */ 	or	$t6,$t7,$at
/*  f124724:	ac4e0004 */ 	sw	$t6,0x4($v0)
/*  f124728:	44d8f800 */ 	ctc1	$t8,$31
/*  f12472c:	ac790000 */ 	sw	$t9,0x0($v1)
/*  f124730:	8fb804e0 */ 	lw	$t8,0x4e0($sp)
/*  f124734:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f124738:	02002025 */ 	or	$a0,$s0,$zero
/*  f12473c:	c706002c */ 	lwc1	$f6,0x2c($t8)
/*  f124740:	8fb804d4 */ 	lw	$t8,0x4d4($sp)
/*  f124744:	3c0fb400 */ 	lui	$t7,0xb400
/*  f124748:	4600320d */ 	trunc.w.s	$f8,$f6
/*  f12474c:	c70a002c */ 	lwc1	$f10,0x2c($t8)
/*  f124750:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f124754:	4600510d */ 	trunc.w.s	$f4,$f10
/*  f124758:	440e4000 */ 	mfc1	$t6,$f8
/*  f12475c:	00000000 */ 	nop
/*  f124760:	000ecc00 */ 	sll	$t9,$t6,0x10
/*  f124764:	440e2000 */ 	mfc1	$t6,$f4
/*  f124768:	00000000 */ 	nop
/*  f12476c:	032ec025 */ 	or	$t8,$t9,$t6
/*  f124770:	ac780004 */ 	sw	$t8,0x4($v1)
/*  f124774:	ac8f0000 */ 	sw	$t7,0x0($a0)
/*  f124778:	e7ae0198 */ 	swc1	$f14,0x198($sp)
/*  f12477c:	0fc2d5fe */ 	jal	camGetScreenLeft
/*  f124780:	afa4018c */ 	sw	$a0,0x18c($sp)
/*  f124784:	0fc54be8 */ 	jal	func0f152fa0
/*  f124788:	46000306 */ 	mov.s	$f12,$f0
/*  f12478c:	8fb9018c */ 	lw	$t9,0x18c($sp)
/*  f124790:	c7ac0198 */ 	lwc1	$f12,0x198($sp)
/*  f124794:	02001825 */ 	or	$v1,$s0,$zero
/*  f124798:	3c0eb200 */ 	lui	$t6,0xb200
/*  f12479c:	af220004 */ 	sw	$v0,0x4($t9)
/*  f1247a0:	ac6e0000 */ 	sw	$t6,0x0($v1)
/*  f1247a4:	afa30188 */ 	sw	$v1,0x188($sp)
/*  f1247a8:	0fc54be8 */ 	jal	func0f152fa0
/*  f1247ac:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1247b0:	8fa30188 */ 	lw	$v1,0x188($sp)
/*  f1247b4:	02002025 */ 	or	$a0,$s0,$zero
/*  f1247b8:	3c18b400 */ 	lui	$t8,0xb400
/*  f1247bc:	ac620004 */ 	sw	$v0,0x4($v1)
/*  f1247c0:	ac980000 */ 	sw	$t8,0x0($a0)
/*  f1247c4:	afa40184 */ 	sw	$a0,0x184($sp)
/*  f1247c8:	0fc2d5fe */ 	jal	camGetScreenLeft
/*  f1247cc:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1247d0:	0fc2d5f6 */ 	jal	camGetScreenWidth
/*  f1247d4:	e7a0005c */ 	swc1	$f0,0x5c($sp)
/*  f1247d8:	c7a6005c */ 	lwc1	$f6,0x5c($sp)
/*  f1247dc:	3c013e80 */ 	lui	$at,0x3e80
/*  f1247e0:	44815000 */ 	mtc1	$at,$f10
/*  f1247e4:	46060200 */ 	add.s	$f8,$f0,$f6
/*  f1247e8:	0fc54be8 */ 	jal	func0f152fa0
/*  f1247ec:	460a4301 */ 	sub.s	$f12,$f8,$f10
/*  f1247f0:	8faf0184 */ 	lw	$t7,0x184($sp)
/*  f1247f4:	02001825 */ 	or	$v1,$s0,$zero
/*  f1247f8:	3c19b200 */ 	lui	$t9,0xb200
/*  f1247fc:	ade20004 */ 	sw	$v0,0x4($t7)
/*  f124800:	ac790000 */ 	sw	$t9,0x0($v1)
/*  f124804:	afa30180 */ 	sw	$v1,0x180($sp)
/*  f124808:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f12480c:	0fc54be8 */ 	jal	func0f152fa0
/*  f124810:	4600a306 */ 	mov.s	$f12,$f20
/*  f124814:	8fa30180 */ 	lw	$v1,0x180($sp)
/*  f124818:	02002025 */ 	or	$a0,$s0,$zero
/*  f12481c:	3c0eb400 */ 	lui	$t6,0xb400
/*  f124820:	ac620004 */ 	sw	$v0,0x4($v1)
/*  f124824:	ac8e0000 */ 	sw	$t6,0x0($a0)
/*  f124828:	afa4017c */ 	sw	$a0,0x17c($sp)
/*  f12482c:	0fc2d5fe */ 	jal	camGetScreenLeft
/*  f124830:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f124834:	0fc54be8 */ 	jal	func0f152fa0
/*  f124838:	46000306 */ 	mov.s	$f12,$f0
/*  f12483c:	8fb8017c */ 	lw	$t8,0x17c($sp)
/*  f124840:	02001825 */ 	or	$v1,$s0,$zero
/*  f124844:	3c0fb200 */ 	lui	$t7,0xb200
/*  f124848:	af020004 */ 	sw	$v0,0x4($t8)
/*  f12484c:	ac6f0000 */ 	sw	$t7,0x0($v1)
/*  f124850:	afa30178 */ 	sw	$v1,0x178($sp)
/*  f124854:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f124858:	0fc54be8 */ 	jal	func0f152fa0
/*  f12485c:	4600a306 */ 	mov.s	$f12,$f20
/*  f124860:	8fa30178 */ 	lw	$v1,0x178($sp)
/*  f124864:	ac620004 */ 	sw	$v0,0x4($v1)
.L0f124868:
/*  f124868:	8fa804cc */ 	lw	$t0,0x4cc($sp)
/*  f12486c:	c7a003c0 */ 	lwc1	$f0,0x3c0($sp)
/*  f124870:	8fa704c8 */ 	lw	$a3,0x4c8($sp)
/*  f124874:	c504000c */ 	lwc1	$f4,0xc($t0)
/*  f124878:	8fa904c4 */ 	lw	$t1,0x4c4($sp)
/*  f12487c:	8fab04e0 */ 	lw	$t3,0x4e0($sp)
/*  f124880:	46002182 */ 	mul.s	$f6,$f4,$f0
/*  f124884:	3c017f1b */ 	lui	$at,%hi(var7f1b5104)
/*  f124888:	e7a603b0 */ 	swc1	$f6,0x3b0($sp)
/*  f12488c:	c4e8000c */ 	lwc1	$f8,0xc($a3)
/*  f124890:	c7b003b0 */ 	lwc1	$f16,0x3b0($sp)
/*  f124894:	46004282 */ 	mul.s	$f10,$f8,$f0
/*  f124898:	e7aa03b4 */ 	swc1	$f10,0x3b4($sp)
/*  f12489c:	c524000c */ 	lwc1	$f4,0xc($t1)
/*  f1248a0:	46002182 */ 	mul.s	$f6,$f4,$f0
/*  f1248a4:	c7a403b4 */ 	lwc1	$f4,0x3b4($sp)
/*  f1248a8:	4610203c */ 	c.lt.s	$f4,$f16
/*  f1248ac:	e7a603b8 */ 	swc1	$f6,0x3b8($sp)
/*  f1248b0:	c568000c */ 	lwc1	$f8,0xc($t3)
/*  f1248b4:	46004282 */ 	mul.s	$f10,$f8,$f0
/*  f1248b8:	c7a003b8 */ 	lwc1	$f0,0x3b8($sp)
/*  f1248bc:	45000002 */ 	bc1f	.L0f1248c8
/*  f1248c0:	e7aa03bc */ 	swc1	$f10,0x3bc($sp)
/*  f1248c4:	46002406 */ 	mov.s	$f16,$f4
.L0f1248c8:
/*  f1248c8:	4610003c */ 	c.lt.s	$f0,$f16
/*  f1248cc:	00000000 */ 	nop
/*  f1248d0:	45020003 */ 	bc1fl	.L0f1248e0
/*  f1248d4:	c7a003bc */ 	lwc1	$f0,0x3bc($sp)
/*  f1248d8:	46000406 */ 	mov.s	$f16,$f0
/*  f1248dc:	c7a003bc */ 	lwc1	$f0,0x3bc($sp)
.L0f1248e0:
/*  f1248e0:	4610003c */ 	c.lt.s	$f0,$f16
/*  f1248e4:	00000000 */ 	nop
/*  f1248e8:	45000002 */ 	bc1f	.L0f1248f4
/*  f1248ec:	00000000 */ 	nop
/*  f1248f0:	46000406 */ 	mov.s	$f16,$f0
.L0f1248f4:
/*  f1248f4:	c4205104 */ 	lwc1	$f0,%lo(var7f1b5104)($at)
/*  f1248f8:	3c013f00 */ 	lui	$at,0x3f00
/*  f1248fc:	44813000 */ 	mtc1	$at,$f6
/*  f124900:	c5080034 */ 	lwc1	$f8,0x34($t0)
/*  f124904:	46068402 */ 	mul.s	$f16,$f16,$f6
/*  f124908:	00000000 */ 	nop
/*  f12490c:	46104282 */ 	mul.s	$f10,$f8,$f16
/*  f124910:	e7aa039c */ 	swc1	$f10,0x39c($sp)
/*  f124914:	c4e40034 */ 	lwc1	$f4,0x34($a3)
/*  f124918:	46102182 */ 	mul.s	$f6,$f4,$f16
/*  f12491c:	e7a603a0 */ 	swc1	$f6,0x3a0($sp)
/*  f124920:	c5280034 */ 	lwc1	$f8,0x34($t1)
/*  f124924:	46104282 */ 	mul.s	$f10,$f8,$f16
/*  f124928:	c7a8039c */ 	lwc1	$f8,0x39c($sp)
/*  f12492c:	e7aa03a4 */ 	swc1	$f10,0x3a4($sp)
/*  f124930:	c5640034 */ 	lwc1	$f4,0x34($t3)
/*  f124934:	46102182 */ 	mul.s	$f6,$f4,$f16
/*  f124938:	e7a603a8 */ 	swc1	$f6,0x3a8($sp)
/*  f12493c:	c50a0020 */ 	lwc1	$f10,0x20($t0)
/*  f124940:	460a4102 */ 	mul.s	$f4,$f8,$f10
/*  f124944:	e7a4036c */ 	swc1	$f4,0x36c($sp)
/*  f124948:	c5060024 */ 	lwc1	$f6,0x24($t0)
/*  f12494c:	4604a03e */ 	c.le.s	$f20,$f4
/*  f124950:	46064282 */ 	mul.s	$f10,$f8,$f6
/*  f124954:	00000000 */ 	nop
/*  f124958:	46004182 */ 	mul.s	$f6,$f8,$f0
/*  f12495c:	e7aa0370 */ 	swc1	$f10,0x370($sp)
/*  f124960:	c7aa03a0 */ 	lwc1	$f10,0x3a0($sp)
/*  f124964:	e7a60374 */ 	swc1	$f6,0x374($sp)
/*  f124968:	c4e80020 */ 	lwc1	$f8,0x20($a3)
/*  f12496c:	46085182 */ 	mul.s	$f6,$f10,$f8
/*  f124970:	e7a60378 */ 	swc1	$f6,0x378($sp)
/*  f124974:	c4e80024 */ 	lwc1	$f8,0x24($a3)
/*  f124978:	46085182 */ 	mul.s	$f6,$f10,$f8
/*  f12497c:	00000000 */ 	nop
/*  f124980:	46005202 */ 	mul.s	$f8,$f10,$f0
/*  f124984:	e7a6037c */ 	swc1	$f6,0x37c($sp)
/*  f124988:	c7a603a4 */ 	lwc1	$f6,0x3a4($sp)
/*  f12498c:	e7a80380 */ 	swc1	$f8,0x380($sp)
/*  f124990:	c52a0020 */ 	lwc1	$f10,0x20($t1)
/*  f124994:	460a3202 */ 	mul.s	$f8,$f6,$f10
/*  f124998:	e7a80384 */ 	swc1	$f8,0x384($sp)
/*  f12499c:	c52a0024 */ 	lwc1	$f10,0x24($t1)
/*  f1249a0:	460a3202 */ 	mul.s	$f8,$f6,$f10
/*  f1249a4:	00000000 */ 	nop
/*  f1249a8:	46003282 */ 	mul.s	$f10,$f6,$f0
/*  f1249ac:	e7a80388 */ 	swc1	$f8,0x388($sp)
/*  f1249b0:	c7a803a8 */ 	lwc1	$f8,0x3a8($sp)
/*  f1249b4:	e7aa038c */ 	swc1	$f10,0x38c($sp)
/*  f1249b8:	c5660020 */ 	lwc1	$f6,0x20($t3)
/*  f1249bc:	46064282 */ 	mul.s	$f10,$f8,$f6
/*  f1249c0:	c7a803a8 */ 	lwc1	$f8,0x3a8($sp)
/*  f1249c4:	e7aa0390 */ 	swc1	$f10,0x390($sp)
/*  f1249c8:	c5660024 */ 	lwc1	$f6,0x24($t3)
/*  f1249cc:	46064282 */ 	mul.s	$f10,$f8,$f6
/*  f1249d0:	c7a803a8 */ 	lwc1	$f8,0x3a8($sp)
/*  f1249d4:	46004182 */ 	mul.s	$f6,$f8,$f0
/*  f1249d8:	c7a00370 */ 	lwc1	$f0,0x370($sp)
/*  f1249dc:	e7aa0394 */ 	swc1	$f10,0x394($sp)
/*  f1249e0:	45000003 */ 	bc1f	.L0f1249f0
/*  f1249e4:	e7a60398 */ 	swc1	$f6,0x398($sp)
/*  f1249e8:	10000003 */ 	b	.L0f1249f8
/*  f1249ec:	46002086 */ 	mov.s	$f2,$f4
.L0f1249f0:
/*  f1249f0:	c7a2036c */ 	lwc1	$f2,0x36c($sp)
/*  f1249f4:	46001087 */ 	neg.s	$f2,$f2
.L0f1249f8:
/*  f1249f8:	4600a03e */ 	c.le.s	$f20,$f0
/*  f1249fc:	00000000 */ 	nop
/*  f124a00:	45020004 */ 	bc1fl	.L0f124a14
/*  f124a04:	46000387 */ 	neg.s	$f14,$f0
/*  f124a08:	10000002 */ 	b	.L0f124a14
/*  f124a0c:	46000386 */ 	mov.s	$f14,$f0
/*  f124a10:	46000387 */ 	neg.s	$f14,$f0
.L0f124a14:
/*  f124a14:	c7a00378 */ 	lwc1	$f0,0x378($sp)
/*  f124a18:	4600a03e */ 	c.le.s	$f20,$f0
/*  f124a1c:	00000000 */ 	nop
/*  f124a20:	45020004 */ 	bc1fl	.L0f124a34
/*  f124a24:	46000307 */ 	neg.s	$f12,$f0
/*  f124a28:	10000002 */ 	b	.L0f124a34
/*  f124a2c:	46000306 */ 	mov.s	$f12,$f0
/*  f124a30:	46000307 */ 	neg.s	$f12,$f0
.L0f124a34:
/*  f124a34:	460c103c */ 	c.lt.s	$f2,$f12
/*  f124a38:	00000000 */ 	nop
/*  f124a3c:	45020009 */ 	bc1fl	.L0f124a64
/*  f124a40:	c7a0037c */ 	lwc1	$f0,0x37c($sp)
/*  f124a44:	4600a03e */ 	c.le.s	$f20,$f0
/*  f124a48:	00000000 */ 	nop
/*  f124a4c:	45020004 */ 	bc1fl	.L0f124a60
/*  f124a50:	46000087 */ 	neg.s	$f2,$f0
/*  f124a54:	10000002 */ 	b	.L0f124a60
/*  f124a58:	46000086 */ 	mov.s	$f2,$f0
/*  f124a5c:	46000087 */ 	neg.s	$f2,$f0
.L0f124a60:
/*  f124a60:	c7a0037c */ 	lwc1	$f0,0x37c($sp)
.L0f124a64:
/*  f124a64:	4600a03e */ 	c.le.s	$f20,$f0
/*  f124a68:	00000000 */ 	nop
/*  f124a6c:	45020004 */ 	bc1fl	.L0f124a80
/*  f124a70:	46000307 */ 	neg.s	$f12,$f0
/*  f124a74:	10000002 */ 	b	.L0f124a80
/*  f124a78:	46000306 */ 	mov.s	$f12,$f0
/*  f124a7c:	46000307 */ 	neg.s	$f12,$f0
.L0f124a80:
/*  f124a80:	460c703c */ 	c.lt.s	$f14,$f12
/*  f124a84:	e7b003ac */ 	swc1	$f16,0x3ac($sp)
/*  f124a88:	4502000b */ 	bc1fl	.L0f124ab8
/*  f124a8c:	c7a00384 */ 	lwc1	$f0,0x384($sp)
/*  f124a90:	4600a03e */ 	c.le.s	$f20,$f0
/*  f124a94:	00000000 */ 	nop
/*  f124a98:	45020005 */ 	bc1fl	.L0f124ab0
/*  f124a9c:	46000387 */ 	neg.s	$f14,$f0
/*  f124aa0:	46000386 */ 	mov.s	$f14,$f0
/*  f124aa4:	10000003 */ 	b	.L0f124ab4
/*  f124aa8:	e7b003ac */ 	swc1	$f16,0x3ac($sp)
/*  f124aac:	46000387 */ 	neg.s	$f14,$f0
.L0f124ab0:
/*  f124ab0:	e7b003ac */ 	swc1	$f16,0x3ac($sp)
.L0f124ab4:
/*  f124ab4:	c7a00384 */ 	lwc1	$f0,0x384($sp)
.L0f124ab8:
/*  f124ab8:	4600a03e */ 	c.le.s	$f20,$f0
/*  f124abc:	00000000 */ 	nop
/*  f124ac0:	45020004 */ 	bc1fl	.L0f124ad4
/*  f124ac4:	46000307 */ 	neg.s	$f12,$f0
/*  f124ac8:	10000002 */ 	b	.L0f124ad4
/*  f124acc:	46000306 */ 	mov.s	$f12,$f0
/*  f124ad0:	46000307 */ 	neg.s	$f12,$f0
.L0f124ad4:
/*  f124ad4:	460c103c */ 	c.lt.s	$f2,$f12
/*  f124ad8:	00000000 */ 	nop
/*  f124adc:	45020009 */ 	bc1fl	.L0f124b04
/*  f124ae0:	c7a00388 */ 	lwc1	$f0,0x388($sp)
/*  f124ae4:	4600a03e */ 	c.le.s	$f20,$f0
/*  f124ae8:	00000000 */ 	nop
/*  f124aec:	45020004 */ 	bc1fl	.L0f124b00
/*  f124af0:	46000087 */ 	neg.s	$f2,$f0
/*  f124af4:	10000002 */ 	b	.L0f124b00
/*  f124af8:	46000086 */ 	mov.s	$f2,$f0
/*  f124afc:	46000087 */ 	neg.s	$f2,$f0
.L0f124b00:
/*  f124b00:	c7a00388 */ 	lwc1	$f0,0x388($sp)
.L0f124b04:
/*  f124b04:	4600a03e */ 	c.le.s	$f20,$f0
/*  f124b08:	00000000 */ 	nop
/*  f124b0c:	45020004 */ 	bc1fl	.L0f124b20
/*  f124b10:	46000307 */ 	neg.s	$f12,$f0
/*  f124b14:	10000002 */ 	b	.L0f124b20
/*  f124b18:	46000306 */ 	mov.s	$f12,$f0
/*  f124b1c:	46000307 */ 	neg.s	$f12,$f0
.L0f124b20:
/*  f124b20:	460c703c */ 	c.lt.s	$f14,$f12
/*  f124b24:	00000000 */ 	nop
/*  f124b28:	45020009 */ 	bc1fl	.L0f124b50
/*  f124b2c:	c7a00390 */ 	lwc1	$f0,0x390($sp)
/*  f124b30:	4600a03e */ 	c.le.s	$f20,$f0
/*  f124b34:	00000000 */ 	nop
/*  f124b38:	45020004 */ 	bc1fl	.L0f124b4c
/*  f124b3c:	46000387 */ 	neg.s	$f14,$f0
/*  f124b40:	10000002 */ 	b	.L0f124b4c
/*  f124b44:	46000386 */ 	mov.s	$f14,$f0
/*  f124b48:	46000387 */ 	neg.s	$f14,$f0
.L0f124b4c:
/*  f124b4c:	c7a00390 */ 	lwc1	$f0,0x390($sp)
.L0f124b50:
/*  f124b50:	4600a03e */ 	c.le.s	$f20,$f0
/*  f124b54:	00000000 */ 	nop
/*  f124b58:	45020004 */ 	bc1fl	.L0f124b6c
/*  f124b5c:	46000307 */ 	neg.s	$f12,$f0
/*  f124b60:	10000002 */ 	b	.L0f124b6c
/*  f124b64:	46000306 */ 	mov.s	$f12,$f0
/*  f124b68:	46000307 */ 	neg.s	$f12,$f0
.L0f124b6c:
/*  f124b6c:	460c103c */ 	c.lt.s	$f2,$f12
/*  f124b70:	e7a20364 */ 	swc1	$f2,0x364($sp)
/*  f124b74:	4502000a */ 	bc1fl	.L0f124ba0
/*  f124b78:	c7a00394 */ 	lwc1	$f0,0x394($sp)
/*  f124b7c:	4600a03e */ 	c.le.s	$f20,$f0
/*  f124b80:	00000000 */ 	nop
/*  f124b84:	45020004 */ 	bc1fl	.L0f124b98
/*  f124b88:	46000087 */ 	neg.s	$f2,$f0
/*  f124b8c:	10000003 */ 	b	.L0f124b9c
/*  f124b90:	e7a00364 */ 	swc1	$f0,0x364($sp)
/*  f124b94:	46000087 */ 	neg.s	$f2,$f0
.L0f124b98:
/*  f124b98:	e7a20364 */ 	swc1	$f2,0x364($sp)
.L0f124b9c:
/*  f124b9c:	c7a00394 */ 	lwc1	$f0,0x394($sp)
.L0f124ba0:
/*  f124ba0:	4600a03e */ 	c.le.s	$f20,$f0
/*  f124ba4:	00000000 */ 	nop
/*  f124ba8:	45020004 */ 	bc1fl	.L0f124bbc
/*  f124bac:	46000307 */ 	neg.s	$f12,$f0
/*  f124bb0:	10000002 */ 	b	.L0f124bbc
/*  f124bb4:	46000306 */ 	mov.s	$f12,$f0
/*  f124bb8:	46000307 */ 	neg.s	$f12,$f0
.L0f124bbc:
/*  f124bbc:	460c703c */ 	c.lt.s	$f14,$f12
/*  f124bc0:	e7ae0368 */ 	swc1	$f14,0x368($sp)
/*  f124bc4:	4502000a */ 	bc1fl	.L0f124bf0
/*  f124bc8:	c7aa036c */ 	lwc1	$f10,0x36c($sp)
/*  f124bcc:	4600a03e */ 	c.le.s	$f20,$f0
/*  f124bd0:	00000000 */ 	nop
/*  f124bd4:	45020004 */ 	bc1fl	.L0f124be8
/*  f124bd8:	46000387 */ 	neg.s	$f14,$f0
/*  f124bdc:	10000003 */ 	b	.L0f124bec
/*  f124be0:	e7a00368 */ 	swc1	$f0,0x368($sp)
/*  f124be4:	46000387 */ 	neg.s	$f14,$f0
.L0f124be8:
/*  f124be8:	e7ae0368 */ 	swc1	$f14,0x368($sp)
.L0f124bec:
/*  f124bec:	c7aa036c */ 	lwc1	$f10,0x36c($sp)
.L0f124bf0:
/*  f124bf0:	c7a80370 */ 	lwc1	$f8,0x370($sp)
/*  f124bf4:	c7a60374 */ 	lwc1	$f6,0x374($sp)
/*  f124bf8:	e7aa0354 */ 	swc1	$f10,0x354($sp)
/*  f124bfc:	c7aa037c */ 	lwc1	$f10,0x37c($sp)
/*  f124c00:	c7a40378 */ 	lwc1	$f4,0x378($sp)
/*  f124c04:	e7a80358 */ 	swc1	$f8,0x358($sp)
/*  f124c08:	e7a6035c */ 	swc1	$f6,0x35c($sp)
/*  f124c0c:	e7aa0338 */ 	swc1	$f10,0x338($sp)
/*  f124c10:	e7a40334 */ 	swc1	$f4,0x334($sp)
/*  f124c14:	c7a80380 */ 	lwc1	$f8,0x380($sp)
/*  f124c18:	c7a40388 */ 	lwc1	$f4,0x388($sp)
/*  f124c1c:	c7aa038c */ 	lwc1	$f10,0x38c($sp)
/*  f124c20:	c7a60384 */ 	lwc1	$f6,0x384($sp)
/*  f124c24:	e7a8033c */ 	swc1	$f8,0x33c($sp)
/*  f124c28:	e7a40318 */ 	swc1	$f4,0x318($sp)
/*  f124c2c:	e7aa031c */ 	swc1	$f10,0x31c($sp)
/*  f124c30:	e7a60314 */ 	swc1	$f6,0x314($sp)
/*  f124c34:	c5080030 */ 	lwc1	$f8,0x30($t0)
/*  f124c38:	27a302f4 */ 	addiu	$v1,$sp,0x2f4
/*  f124c3c:	27a60334 */ 	addiu	$a2,$sp,0x334
/*  f124c40:	e7a80360 */ 	swc1	$f8,0x360($sp)
/*  f124c44:	c4e60030 */ 	lwc1	$f6,0x30($a3)
/*  f124c48:	27a70324 */ 	addiu	$a3,$sp,0x324
/*  f124c4c:	27a20354 */ 	addiu	$v0,$sp,0x354
/*  f124c50:	e7a60340 */ 	swc1	$f6,0x340($sp)
/*  f124c54:	c5240030 */ 	lwc1	$f4,0x30($t1)
/*  f124c58:	27a402d4 */ 	addiu	$a0,$sp,0x2d4
/*  f124c5c:	27a50314 */ 	addiu	$a1,$sp,0x314
/*  f124c60:	e7a40320 */ 	swc1	$f4,0x320($sp)
.L0f124c64:
/*  f124c64:	c4400000 */ 	lwc1	$f0,0x0($v0)
/*  f124c68:	c4ca0000 */ 	lwc1	$f10,0x0($a2)
/*  f124c6c:	c4a60000 */ 	lwc1	$f6,0x0($a1)
/*  f124c70:	24a50004 */ 	addiu	$a1,$a1,0x4
/*  f124c74:	46005201 */ 	sub.s	$f8,$f10,$f0
/*  f124c78:	00a7082b */ 	sltu	$at,$a1,$a3
/*  f124c7c:	24630004 */ 	addiu	$v1,$v1,0x4
/*  f124c80:	46003101 */ 	sub.s	$f4,$f6,$f0
/*  f124c84:	24c60004 */ 	addiu	$a2,$a2,0x4
/*  f124c88:	24420004 */ 	addiu	$v0,$v0,0x4
/*  f124c8c:	24840004 */ 	addiu	$a0,$a0,0x4
/*  f124c90:	e468fffc */ 	swc1	$f8,-0x4($v1)
/*  f124c94:	1420fff3 */ 	bnez	$at,.L0f124c64
/*  f124c98:	e484fffc */ 	swc1	$f4,-0x4($a0)
/*  f124c9c:	27a302f4 */ 	addiu	$v1,$sp,0x2f4
/*  f124ca0:	27a20354 */ 	addiu	$v0,$sp,0x354
/*  f124ca4:	27a402d4 */ 	addiu	$a0,$sp,0x2d4
/*  f124ca8:	27a70294 */ 	addiu	$a3,$sp,0x294
/*  f124cac:	27a502b4 */ 	addiu	$a1,$sp,0x2b4
/*  f124cb0:	27a80274 */ 	addiu	$t0,$sp,0x274
/*  f124cb4:	27a60254 */ 	addiu	$a2,$sp,0x254
/*  f124cb8:	c7b20410 */ 	lwc1	$f18,0x410($sp)
/*  f124cbc:	c7b0040c */ 	lwc1	$f16,0x40c($sp)
/*  f124cc0:	27a90264 */ 	addiu	$t1,$sp,0x264
.L0f124cc4:
/*  f124cc4:	c4800000 */ 	lwc1	$f0,0x0($a0)
/*  f124cc8:	c4620000 */ 	lwc1	$f2,0x0($v1)
/*  f124ccc:	c7a80418 */ 	lwc1	$f8,0x418($sp)
/*  f124cd0:	46120282 */ 	mul.s	$f10,$f0,$f18
/*  f124cd4:	3c013780 */ 	lui	$at,0x3780
/*  f124cd8:	24c60004 */ 	addiu	$a2,$a2,0x4
/*  f124cdc:	46024182 */ 	mul.s	$f6,$f8,$f2
/*  f124ce0:	44814000 */ 	mtc1	$at,$f8
/*  f124ce4:	24630004 */ 	addiu	$v1,$v1,0x4
/*  f124ce8:	24420004 */ 	addiu	$v0,$v0,0x4
/*  f124cec:	24840004 */ 	addiu	$a0,$a0,0x4
/*  f124cf0:	24e70004 */ 	addiu	$a3,$a3,0x4
/*  f124cf4:	24a50004 */ 	addiu	$a1,$a1,0x4
/*  f124cf8:	46065101 */ 	sub.s	$f4,$f10,$f6
/*  f124cfc:	25080004 */ 	addiu	$t0,$t0,0x4
/*  f124d00:	46082302 */ 	mul.s	$f12,$f4,$f8
/*  f124d04:	e4ecfffc */ 	swc1	$f12,-0x4($a3)
/*  f124d08:	c7aa0414 */ 	lwc1	$f10,0x414($sp)
/*  f124d0c:	460a1182 */ 	mul.s	$f6,$f2,$f10
/*  f124d10:	44815000 */ 	mtc1	$at,$f10
/*  f124d14:	00c9082b */ 	sltu	$at,$a2,$t1
/*  f124d18:	46008102 */ 	mul.s	$f4,$f16,$f0
/*  f124d1c:	46043201 */ 	sub.s	$f8,$f6,$f4
/*  f124d20:	460a4182 */ 	mul.s	$f6,$f8,$f10
/*  f124d24:	e4a6fffc */ 	swc1	$f6,-0x4($a1)
/*  f124d28:	c7a40484 */ 	lwc1	$f4,0x484($sp)
/*  f124d2c:	c4aafffc */ 	lwc1	$f10,-0x4($a1)
/*  f124d30:	46046202 */ 	mul.s	$f8,$f12,$f4
/*  f124d34:	c444fffc */ 	lwc1	$f4,-0x4($v0)
/*  f124d38:	e488fffc */ 	swc1	$f8,-0x4($a0)
/*  f124d3c:	c7a60484 */ 	lwc1	$f6,0x484($sp)
/*  f124d40:	e4c4fffc */ 	swc1	$f4,-0x4($a2)
/*  f124d44:	46065382 */ 	mul.s	$f14,$f10,$f6
/*  f124d48:	e46efffc */ 	swc1	$f14,-0x4($v1)
/*  f124d4c:	1420ffdd */ 	bnez	$at,.L0f124cc4
/*  f124d50:	e50efffc */ 	swc1	$f14,-0x4($t0)
/*  f124d54:	8fa304dc */ 	lw	$v1,0x4dc($sp)
/*  f124d58:	c568002c */ 	lwc1	$f8,0x2c($t3)
/*  f124d5c:	8fa204d4 */ 	lw	$v0,0x4d4($sp)
/*  f124d60:	c46a002c */ 	lwc1	$f10,0x2c($v1)
/*  f124d64:	c4640010 */ 	lwc1	$f4,0x10($v1)
/*  f124d68:	c44e0010 */ 	lwc1	$f14,0x10($v0)
/*  f124d6c:	460a4003 */ 	div.s	$f0,$f8,$f10
/*  f124d70:	c5660010 */ 	lwc1	$f6,0x10($t3)
/*  f124d74:	3c014780 */ 	lui	$at,0x4780
/*  f124d78:	44816000 */ 	mtc1	$at,$f12
/*  f124d7c:	46047201 */ 	sub.s	$f8,$f14,$f4
/*  f124d80:	240e0001 */ 	addiu	$t6,$zero,0x1
/*  f124d84:	46004282 */ 	mul.s	$f10,$f8,$f0
/*  f124d88:	460a3100 */ 	add.s	$f4,$f6,$f10
/*  f124d8c:	e7a40170 */ 	swc1	$f4,0x170($sp)
/*  f124d90:	c4660014 */ 	lwc1	$f6,0x14($v1)
/*  f124d94:	c4420014 */ 	lwc1	$f2,0x14($v0)
/*  f124d98:	c5680014 */ 	lwc1	$f8,0x14($t3)
/*  f124d9c:	46061281 */ 	sub.s	$f10,$f2,$f6
/*  f124da0:	46005102 */ 	mul.s	$f4,$f10,$f0
/*  f124da4:	46044180 */ 	add.s	$f6,$f8,$f4
/*  f124da8:	e7a6016c */ 	swc1	$f6,0x16c($sp)
/*  f124dac:	c4680018 */ 	lwc1	$f8,0x18($v1)
/*  f124db0:	c4500018 */ 	lwc1	$f16,0x18($v0)
/*  f124db4:	c56a0018 */ 	lwc1	$f10,0x18($t3)
/*  f124db8:	46088101 */ 	sub.s	$f4,$f16,$f8
/*  f124dbc:	46002182 */ 	mul.s	$f6,$f4,$f0
/*  f124dc0:	46065200 */ 	add.s	$f8,$f10,$f6
/*  f124dc4:	e7a80168 */ 	swc1	$f8,0x168($sp)
/*  f124dc8:	c46a001c */ 	lwc1	$f10,0x1c($v1)
/*  f124dcc:	c452001c */ 	lwc1	$f18,0x1c($v0)
/*  f124dd0:	c564001c */ 	lwc1	$f4,0x1c($t3)
/*  f124dd4:	460a9181 */ 	sub.s	$f6,$f18,$f10
/*  f124dd8:	46003202 */ 	mul.s	$f8,$f6,$f0
/*  f124ddc:	46082280 */ 	add.s	$f10,$f4,$f8
/*  f124de0:	460c7182 */ 	mul.s	$f6,$f14,$f12
/*  f124de4:	e7aa0164 */ 	swc1	$f10,0x164($sp)
/*  f124de8:	4459f800 */ 	cfc1	$t9,$31
/*  f124dec:	44cef800 */ 	ctc1	$t6,$31
/*  f124df0:	00000000 */ 	nop
/*  f124df4:	46003124 */ 	cvt.w.s	$f4,$f6
/*  f124df8:	444ef800 */ 	cfc1	$t6,$31
/*  f124dfc:	00000000 */ 	nop
/*  f124e00:	31ce0078 */ 	andi	$t6,$t6,0x78
/*  f124e04:	11c00012 */ 	beqz	$t6,.L0f124e50
/*  f124e08:	3c014f00 */ 	lui	$at,0x4f00
/*  f124e0c:	44812000 */ 	mtc1	$at,$f4
/*  f124e10:	240e0001 */ 	addiu	$t6,$zero,0x1
/*  f124e14:	46043101 */ 	sub.s	$f4,$f6,$f4
/*  f124e18:	44cef800 */ 	ctc1	$t6,$31
/*  f124e1c:	00000000 */ 	nop
/*  f124e20:	46002124 */ 	cvt.w.s	$f4,$f4
/*  f124e24:	444ef800 */ 	cfc1	$t6,$31
/*  f124e28:	00000000 */ 	nop
/*  f124e2c:	31ce0078 */ 	andi	$t6,$t6,0x78
/*  f124e30:	15c00005 */ 	bnez	$t6,.L0f124e48
/*  f124e34:	00000000 */ 	nop
/*  f124e38:	440e2000 */ 	mfc1	$t6,$f4
/*  f124e3c:	3c018000 */ 	lui	$at,0x8000
/*  f124e40:	10000007 */ 	b	.L0f124e60
/*  f124e44:	01c17025 */ 	or	$t6,$t6,$at
.L0f124e48:
/*  f124e48:	10000005 */ 	b	.L0f124e60
/*  f124e4c:	240effff */ 	addiu	$t6,$zero,-1
.L0f124e50:
/*  f124e50:	440e2000 */ 	mfc1	$t6,$f4
/*  f124e54:	00000000 */ 	nop
/*  f124e58:	05c0fffb */ 	bltz	$t6,.L0f124e48
/*  f124e5c:	00000000 */ 	nop
.L0f124e60:
/*  f124e60:	44d9f800 */ 	ctc1	$t9,$31
/*  f124e64:	240f0001 */ 	addiu	$t7,$zero,0x1
/*  f124e68:	afae0160 */ 	sw	$t6,0x160($sp)
/*  f124e6c:	460c1202 */ 	mul.s	$f8,$f2,$f12
/*  f124e70:	4458f800 */ 	cfc1	$t8,$31
/*  f124e74:	44cff800 */ 	ctc1	$t7,$31
/*  f124e78:	00000000 */ 	nop
/*  f124e7c:	460042a4 */ 	cvt.w.s	$f10,$f8
/*  f124e80:	444ff800 */ 	cfc1	$t7,$31
/*  f124e84:	00000000 */ 	nop
/*  f124e88:	31ef0078 */ 	andi	$t7,$t7,0x78
/*  f124e8c:	11e00012 */ 	beqz	$t7,.L0f124ed8
/*  f124e90:	3c014f00 */ 	lui	$at,0x4f00
/*  f124e94:	44815000 */ 	mtc1	$at,$f10
/*  f124e98:	240f0001 */ 	addiu	$t7,$zero,0x1
/*  f124e9c:	460a4281 */ 	sub.s	$f10,$f8,$f10
/*  f124ea0:	44cff800 */ 	ctc1	$t7,$31
/*  f124ea4:	00000000 */ 	nop
/*  f124ea8:	460052a4 */ 	cvt.w.s	$f10,$f10
/*  f124eac:	444ff800 */ 	cfc1	$t7,$31
/*  f124eb0:	00000000 */ 	nop
/*  f124eb4:	31ef0078 */ 	andi	$t7,$t7,0x78
/*  f124eb8:	15e00005 */ 	bnez	$t7,.L0f124ed0
/*  f124ebc:	00000000 */ 	nop
/*  f124ec0:	440f5000 */ 	mfc1	$t7,$f10
/*  f124ec4:	3c018000 */ 	lui	$at,0x8000
/*  f124ec8:	10000007 */ 	b	.L0f124ee8
/*  f124ecc:	01e17825 */ 	or	$t7,$t7,$at
.L0f124ed0:
/*  f124ed0:	10000005 */ 	b	.L0f124ee8
/*  f124ed4:	240fffff */ 	addiu	$t7,$zero,-1
.L0f124ed8:
/*  f124ed8:	440f5000 */ 	mfc1	$t7,$f10
/*  f124edc:	00000000 */ 	nop
/*  f124ee0:	05e0fffb */ 	bltz	$t7,.L0f124ed0
/*  f124ee4:	00000000 */ 	nop
.L0f124ee8:
/*  f124ee8:	3c014780 */ 	lui	$at,0x4780
/*  f124eec:	44d8f800 */ 	ctc1	$t8,$31
/*  f124ef0:	44810000 */ 	mtc1	$at,$f0
/*  f124ef4:	240e0001 */ 	addiu	$t6,$zero,0x1
/*  f124ef8:	afaf015c */ 	sw	$t7,0x15c($sp)
/*  f124efc:	46008182 */ 	mul.s	$f6,$f16,$f0
/*  f124f00:	4459f800 */ 	cfc1	$t9,$31
/*  f124f04:	44cef800 */ 	ctc1	$t6,$31
/*  f124f08:	00000000 */ 	nop
/*  f124f0c:	46003124 */ 	cvt.w.s	$f4,$f6
/*  f124f10:	444ef800 */ 	cfc1	$t6,$31
/*  f124f14:	00000000 */ 	nop
/*  f124f18:	31ce0078 */ 	andi	$t6,$t6,0x78
/*  f124f1c:	11c00012 */ 	beqz	$t6,.L0f124f68
/*  f124f20:	3c014f00 */ 	lui	$at,0x4f00
/*  f124f24:	44812000 */ 	mtc1	$at,$f4
/*  f124f28:	240e0001 */ 	addiu	$t6,$zero,0x1
/*  f124f2c:	46043101 */ 	sub.s	$f4,$f6,$f4
/*  f124f30:	44cef800 */ 	ctc1	$t6,$31
/*  f124f34:	00000000 */ 	nop
/*  f124f38:	46002124 */ 	cvt.w.s	$f4,$f4
/*  f124f3c:	444ef800 */ 	cfc1	$t6,$31
/*  f124f40:	00000000 */ 	nop
/*  f124f44:	31ce0078 */ 	andi	$t6,$t6,0x78
/*  f124f48:	15c00005 */ 	bnez	$t6,.L0f124f60
/*  f124f4c:	00000000 */ 	nop
/*  f124f50:	440e2000 */ 	mfc1	$t6,$f4
/*  f124f54:	3c018000 */ 	lui	$at,0x8000
/*  f124f58:	10000007 */ 	b	.L0f124f78
/*  f124f5c:	01c17025 */ 	or	$t6,$t6,$at
.L0f124f60:
/*  f124f60:	10000005 */ 	b	.L0f124f78
/*  f124f64:	240effff */ 	addiu	$t6,$zero,-1
.L0f124f68:
/*  f124f68:	440e2000 */ 	mfc1	$t6,$f4
/*  f124f6c:	00000000 */ 	nop
/*  f124f70:	05c0fffb */ 	bltz	$t6,.L0f124f60
/*  f124f74:	00000000 */ 	nop
.L0f124f78:
/*  f124f78:	44d9f800 */ 	ctc1	$t9,$31
/*  f124f7c:	240f0001 */ 	addiu	$t7,$zero,0x1
/*  f124f80:	afae0158 */ 	sw	$t6,0x158($sp)
/*  f124f84:	46009202 */ 	mul.s	$f8,$f18,$f0
/*  f124f88:	4458f800 */ 	cfc1	$t8,$31
/*  f124f8c:	44cff800 */ 	ctc1	$t7,$31
/*  f124f90:	00000000 */ 	nop
/*  f124f94:	460042a4 */ 	cvt.w.s	$f10,$f8
/*  f124f98:	444ff800 */ 	cfc1	$t7,$31
/*  f124f9c:	00000000 */ 	nop
/*  f124fa0:	31ef0078 */ 	andi	$t7,$t7,0x78
/*  f124fa4:	11e00012 */ 	beqz	$t7,.L0f124ff0
/*  f124fa8:	3c014f00 */ 	lui	$at,0x4f00
/*  f124fac:	44815000 */ 	mtc1	$at,$f10
/*  f124fb0:	240f0001 */ 	addiu	$t7,$zero,0x1
/*  f124fb4:	460a4281 */ 	sub.s	$f10,$f8,$f10
/*  f124fb8:	44cff800 */ 	ctc1	$t7,$31
/*  f124fbc:	00000000 */ 	nop
/*  f124fc0:	460052a4 */ 	cvt.w.s	$f10,$f10
/*  f124fc4:	444ff800 */ 	cfc1	$t7,$31
/*  f124fc8:	00000000 */ 	nop
/*  f124fcc:	31ef0078 */ 	andi	$t7,$t7,0x78
/*  f124fd0:	15e00005 */ 	bnez	$t7,.L0f124fe8
/*  f124fd4:	00000000 */ 	nop
/*  f124fd8:	440f5000 */ 	mfc1	$t7,$f10
/*  f124fdc:	3c018000 */ 	lui	$at,0x8000
/*  f124fe0:	10000007 */ 	b	.L0f125000
/*  f124fe4:	01e17825 */ 	or	$t7,$t7,$at
.L0f124fe8:
/*  f124fe8:	10000005 */ 	b	.L0f125000
/*  f124fec:	240fffff */ 	addiu	$t7,$zero,-1
.L0f124ff0:
/*  f124ff0:	440f5000 */ 	mfc1	$t7,$f10
/*  f124ff4:	00000000 */ 	nop
/*  f124ff8:	05e0fffb */ 	bltz	$t7,.L0f124fe8
/*  f124ffc:	00000000 */ 	nop
.L0f125000:
/*  f125000:	8fb904d8 */ 	lw	$t9,0x4d8($sp)
/*  f125004:	afaf0154 */ 	sw	$t7,0x154($sp)
/*  f125008:	44d8f800 */ 	ctc1	$t8,$31
/*  f12500c:	c4440028 */ 	lwc1	$f4,0x28($v0)
/*  f125010:	c7260028 */ 	lwc1	$f6,0x28($t9)
/*  f125014:	3c013e80 */ 	lui	$at,0x3e80
/*  f125018:	44815000 */ 	mtc1	$at,$f10
/*  f12501c:	46043201 */ 	sub.s	$f8,$f6,$f4
/*  f125020:	c7a40170 */ 	lwc1	$f4,0x170($sp)
/*  f125024:	460a4182 */ 	mul.s	$f6,$f8,$f10
/*  f125028:	460e2201 */ 	sub.s	$f8,$f4,$f14
/*  f12502c:	0fc54be8 */ 	jal	func0f152fa0
/*  f125030:	46064303 */ 	div.s	$f12,$f8,$f6
/*  f125034:	8fa304d4 */ 	lw	$v1,0x4d4($sp)
/*  f125038:	afa20150 */ 	sw	$v0,0x150($sp)
/*  f12503c:	c7aa016c */ 	lwc1	$f10,0x16c($sp)
/*  f125040:	c4640014 */ 	lwc1	$f4,0x14($v1)
/*  f125044:	8fae04d8 */ 	lw	$t6,0x4d8($sp)
/*  f125048:	3c013e80 */ 	lui	$at,0x3e80
/*  f12504c:	46045201 */ 	sub.s	$f8,$f10,$f4
/*  f125050:	c46a0028 */ 	lwc1	$f10,0x28($v1)
/*  f125054:	c5c60028 */ 	lwc1	$f6,0x28($t6)
/*  f125058:	460a3101 */ 	sub.s	$f4,$f6,$f10
/*  f12505c:	44813000 */ 	mtc1	$at,$f6
/*  f125060:	00000000 */ 	nop
/*  f125064:	46062282 */ 	mul.s	$f10,$f4,$f6
/*  f125068:	0fc54be8 */ 	jal	func0f152fa0
/*  f12506c:	460a4303 */ 	div.s	$f12,$f8,$f10
/*  f125070:	8fa304d4 */ 	lw	$v1,0x4d4($sp)
/*  f125074:	afa2014c */ 	sw	$v0,0x14c($sp)
/*  f125078:	c7a40168 */ 	lwc1	$f4,0x168($sp)
/*  f12507c:	c4660018 */ 	lwc1	$f6,0x18($v1)
/*  f125080:	8fb804d8 */ 	lw	$t8,0x4d8($sp)
/*  f125084:	3c013e80 */ 	lui	$at,0x3e80
/*  f125088:	46062201 */ 	sub.s	$f8,$f4,$f6
/*  f12508c:	c4640028 */ 	lwc1	$f4,0x28($v1)
/*  f125090:	c70a0028 */ 	lwc1	$f10,0x28($t8)
/*  f125094:	46045181 */ 	sub.s	$f6,$f10,$f4
/*  f125098:	44815000 */ 	mtc1	$at,$f10
/*  f12509c:	00000000 */ 	nop
/*  f1250a0:	460a3102 */ 	mul.s	$f4,$f6,$f10
/*  f1250a4:	0fc54be8 */ 	jal	func0f152fa0
/*  f1250a8:	46044303 */ 	div.s	$f12,$f8,$f4
/*  f1250ac:	8fa304d4 */ 	lw	$v1,0x4d4($sp)
/*  f1250b0:	afa20148 */ 	sw	$v0,0x148($sp)
/*  f1250b4:	c7a60164 */ 	lwc1	$f6,0x164($sp)
/*  f1250b8:	c46a001c */ 	lwc1	$f10,0x1c($v1)
/*  f1250bc:	8faf04d8 */ 	lw	$t7,0x4d8($sp)
/*  f1250c0:	3c013e80 */ 	lui	$at,0x3e80
/*  f1250c4:	460a3201 */ 	sub.s	$f8,$f6,$f10
/*  f1250c8:	c4660028 */ 	lwc1	$f6,0x28($v1)
/*  f1250cc:	c5e40028 */ 	lwc1	$f4,0x28($t7)
/*  f1250d0:	46062281 */ 	sub.s	$f10,$f4,$f6
/*  f1250d4:	44812000 */ 	mtc1	$at,$f4
/*  f1250d8:	00000000 */ 	nop
/*  f1250dc:	46045182 */ 	mul.s	$f6,$f10,$f4
/*  f1250e0:	0fc54be8 */ 	jal	func0f152fa0
/*  f1250e4:	46064303 */ 	div.s	$f12,$f8,$f6
/*  f1250e8:	8fa304dc */ 	lw	$v1,0x4dc($sp)
/*  f1250ec:	8fb904d4 */ 	lw	$t9,0x4d4($sp)
/*  f1250f0:	afa20144 */ 	sw	$v0,0x144($sp)
/*  f1250f4:	c46a0010 */ 	lwc1	$f10,0x10($v1)
/*  f1250f8:	c7240010 */ 	lwc1	$f4,0x10($t9)
/*  f1250fc:	c466002c */ 	lwc1	$f6,0x2c($v1)
/*  f125100:	3c013e80 */ 	lui	$at,0x3e80
/*  f125104:	46045201 */ 	sub.s	$f8,$f10,$f4
/*  f125108:	c72a002c */ 	lwc1	$f10,0x2c($t9)
/*  f12510c:	460a3101 */ 	sub.s	$f4,$f6,$f10
/*  f125110:	44813000 */ 	mtc1	$at,$f6
/*  f125114:	00000000 */ 	nop
/*  f125118:	46062282 */ 	mul.s	$f10,$f4,$f6
/*  f12511c:	0fc54be8 */ 	jal	func0f152fa0
/*  f125120:	460a4303 */ 	div.s	$f12,$f8,$f10
/*  f125124:	8fa304dc */ 	lw	$v1,0x4dc($sp)
/*  f125128:	8fae04d4 */ 	lw	$t6,0x4d4($sp)
/*  f12512c:	afa20130 */ 	sw	$v0,0x130($sp)
/*  f125130:	afa20140 */ 	sw	$v0,0x140($sp)
/*  f125134:	c4640014 */ 	lwc1	$f4,0x14($v1)
/*  f125138:	c5c60014 */ 	lwc1	$f6,0x14($t6)
/*  f12513c:	c46a002c */ 	lwc1	$f10,0x2c($v1)
/*  f125140:	3c013e80 */ 	lui	$at,0x3e80
/*  f125144:	46062201 */ 	sub.s	$f8,$f4,$f6
/*  f125148:	c5c4002c */ 	lwc1	$f4,0x2c($t6)
/*  f12514c:	46045181 */ 	sub.s	$f6,$f10,$f4
/*  f125150:	44815000 */ 	mtc1	$at,$f10
/*  f125154:	00000000 */ 	nop
/*  f125158:	460a3102 */ 	mul.s	$f4,$f6,$f10
/*  f12515c:	0fc54be8 */ 	jal	func0f152fa0
/*  f125160:	46044303 */ 	div.s	$f12,$f8,$f4
/*  f125164:	8fb804dc */ 	lw	$t8,0x4dc($sp)
/*  f125168:	8faf04d4 */ 	lw	$t7,0x4d4($sp)
/*  f12516c:	afa2012c */ 	sw	$v0,0x12c($sp)
/*  f125170:	afa2013c */ 	sw	$v0,0x13c($sp)
/*  f125174:	c7060018 */ 	lwc1	$f6,0x18($t8)
/*  f125178:	c5ea0018 */ 	lwc1	$f10,0x18($t7)
/*  f12517c:	c704002c */ 	lwc1	$f4,0x2c($t8)
/*  f125180:	3c013e80 */ 	lui	$at,0x3e80
/*  f125184:	460a3201 */ 	sub.s	$f8,$f6,$f10
/*  f125188:	c5e6002c */ 	lwc1	$f6,0x2c($t7)
/*  f12518c:	46062281 */ 	sub.s	$f10,$f4,$f6
/*  f125190:	44812000 */ 	mtc1	$at,$f4
/*  f125194:	00000000 */ 	nop
/*  f125198:	46045182 */ 	mul.s	$f6,$f10,$f4
/*  f12519c:	0fc54be8 */ 	jal	func0f152fa0
/*  f1251a0:	46064303 */ 	div.s	$f12,$f8,$f6
/*  f1251a4:	8fb904dc */ 	lw	$t9,0x4dc($sp)
/*  f1251a8:	8fae04d4 */ 	lw	$t6,0x4d4($sp)
/*  f1251ac:	3c013e80 */ 	lui	$at,0x3e80
/*  f1251b0:	c72a001c */ 	lwc1	$f10,0x1c($t9)
/*  f1251b4:	c5c4001c */ 	lwc1	$f4,0x1c($t6)
/*  f1251b8:	c726002c */ 	lwc1	$f6,0x2c($t9)
/*  f1251bc:	46045201 */ 	sub.s	$f8,$f10,$f4
/*  f1251c0:	c5ca002c */ 	lwc1	$f10,0x2c($t6)
/*  f1251c4:	afa20138 */ 	sw	$v0,0x138($sp)
/*  f1251c8:	afa20128 */ 	sw	$v0,0x128($sp)
/*  f1251cc:	460a3101 */ 	sub.s	$f4,$f6,$f10
/*  f1251d0:	44813000 */ 	mtc1	$at,$f6
/*  f1251d4:	00000000 */ 	nop
/*  f1251d8:	46062282 */ 	mul.s	$f10,$f4,$f6
/*  f1251dc:	0fc54be8 */ 	jal	func0f152fa0
/*  f1251e0:	460a4303 */ 	div.s	$f12,$f8,$f10
/*  f1251e4:	8fab015c */ 	lw	$t3,0x15c($sp)
/*  f1251e8:	8faa0160 */ 	lw	$t2,0x160($sp)
/*  f1251ec:	3c09ffff */ 	lui	$t1,0xffff
/*  f1251f0:	02001825 */ 	or	$v1,$s0,$zero
/*  f1251f4:	01697824 */ 	and	$t7,$t3,$t1
/*  f1251f8:	8fac0128 */ 	lw	$t4,0x128($sp)
/*  f1251fc:	8fad0138 */ 	lw	$t5,0x138($sp)
/*  f125200:	8fbf0158 */ 	lw	$ra,0x158($sp)
/*  f125204:	000fcc02 */ 	srl	$t9,$t7,0x10
/*  f125208:	0149c024 */ 	and	$t8,$t2,$t1
/*  f12520c:	3c08b400 */ 	lui	$t0,0xb400
/*  f125210:	03197025 */ 	or	$t6,$t8,$t9
/*  f125214:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f125218:	ac6e0004 */ 	sw	$t6,0x4($v1)
/*  f12521c:	ac680000 */ 	sw	$t0,0x0($v1)
/*  f125220:	02002025 */ 	or	$a0,$s0,$zero
/*  f125224:	3c0fb200 */ 	lui	$t7,0xb200
/*  f125228:	ac8f0000 */ 	sw	$t7,0x0($a0)
/*  f12522c:	8fb90154 */ 	lw	$t9,0x154($sp)
/*  f125230:	03e9c024 */ 	and	$t8,$ra,$t1
/*  f125234:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f125238:	03297024 */ 	and	$t6,$t9,$t1
/*  f12523c:	000e7c02 */ 	srl	$t7,$t6,0x10
/*  f125240:	030fc825 */ 	or	$t9,$t8,$t7
/*  f125244:	ac990004 */ 	sw	$t9,0x4($a0)
/*  f125248:	02002825 */ 	or	$a1,$s0,$zero
/*  f12524c:	aca80000 */ 	sw	$t0,0x0($a1)
/*  f125250:	8faf014c */ 	lw	$t7,0x14c($sp)
/*  f125254:	8fae0150 */ 	lw	$t6,0x150($sp)
/*  f125258:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f12525c:	01e9c824 */ 	and	$t9,$t7,$t1
/*  f125260:	01c9c024 */ 	and	$t8,$t6,$t1
/*  f125264:	00197402 */ 	srl	$t6,$t9,0x10
/*  f125268:	030e7825 */ 	or	$t7,$t8,$t6
/*  f12526c:	acaf0004 */ 	sw	$t7,0x4($a1)
/*  f125270:	3c05b200 */ 	lui	$a1,0xb200
/*  f125274:	02003025 */ 	or	$a2,$s0,$zero
/*  f125278:	acc50000 */ 	sw	$a1,0x0($a2)
/*  f12527c:	8fae0144 */ 	lw	$t6,0x144($sp)
/*  f125280:	8fb90148 */ 	lw	$t9,0x148($sp)
/*  f125284:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f125288:	01c97824 */ 	and	$t7,$t6,$t1
/*  f12528c:	0329c024 */ 	and	$t8,$t9,$t1
/*  f125290:	000fcc02 */ 	srl	$t9,$t7,0x10
/*  f125294:	03197025 */ 	or	$t6,$t8,$t9
/*  f125298:	acce0004 */ 	sw	$t6,0x4($a2)
/*  f12529c:	02001825 */ 	or	$v1,$s0,$zero
/*  f1252a0:	3179ffff */ 	andi	$t9,$t3,0xffff
/*  f1252a4:	000ac400 */ 	sll	$t8,$t2,0x10
/*  f1252a8:	03197025 */ 	or	$t6,$t8,$t9
/*  f1252ac:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1252b0:	ac6e0004 */ 	sw	$t6,0x4($v1)
/*  f1252b4:	ac680000 */ 	sw	$t0,0x0($v1)
/*  f1252b8:	02002025 */ 	or	$a0,$s0,$zero
/*  f1252bc:	ac850000 */ 	sw	$a1,0x0($a0)
/*  f1252c0:	8fb90154 */ 	lw	$t9,0x154($sp)
/*  f1252c4:	001fc400 */ 	sll	$t8,$ra,0x10
/*  f1252c8:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1252cc:	332effff */ 	andi	$t6,$t9,0xffff
/*  f1252d0:	030e7825 */ 	or	$t7,$t8,$t6
/*  f1252d4:	ac8f0004 */ 	sw	$t7,0x4($a0)
/*  f1252d8:	02003825 */ 	or	$a3,$s0,$zero
/*  f1252dc:	ace80000 */ 	sw	$t0,0x0($a3)
/*  f1252e0:	8faf014c */ 	lw	$t7,0x14c($sp)
/*  f1252e4:	8fb80150 */ 	lw	$t8,0x150($sp)
/*  f1252e8:	8faa0140 */ 	lw	$t2,0x140($sp)
/*  f1252ec:	31f9ffff */ 	andi	$t9,$t7,0xffff
/*  f1252f0:	00187400 */ 	sll	$t6,$t8,0x10
/*  f1252f4:	8fa8013c */ 	lw	$t0,0x13c($sp)
/*  f1252f8:	01d9c025 */ 	or	$t8,$t6,$t9
/*  f1252fc:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f125300:	acf80004 */ 	sw	$t8,0x4($a3)
/*  f125304:	02001825 */ 	or	$v1,$s0,$zero
/*  f125308:	3c0fb200 */ 	lui	$t7,0xb200
/*  f12530c:	ac6f0000 */ 	sw	$t7,0x0($v1)
/*  f125310:	8faf0144 */ 	lw	$t7,0x144($sp)
/*  f125314:	8fb90148 */ 	lw	$t9,0x148($sp)
/*  f125318:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f12531c:	31eeffff */ 	andi	$t6,$t7,0xffff
/*  f125320:	0019c400 */ 	sll	$t8,$t9,0x10
/*  f125324:	030ec825 */ 	or	$t9,$t8,$t6
/*  f125328:	0109c024 */ 	and	$t8,$t0,$t1
/*  f12532c:	ac790004 */ 	sw	$t9,0x4($v1)
/*  f125330:	00187402 */ 	srl	$t6,$t8,0x10
/*  f125334:	01497824 */ 	and	$t7,$t2,$t1
/*  f125338:	01eec825 */ 	or	$t9,$t7,$t6
/*  f12533c:	02002025 */ 	or	$a0,$s0,$zero
/*  f125340:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f125344:	3c1fb400 */ 	lui	$ra,0xb400
/*  f125348:	ac990004 */ 	sw	$t9,0x4($a0)
/*  f12534c:	02002825 */ 	or	$a1,$s0,$zero
/*  f125350:	00493024 */ 	and	$a2,$v0,$t1
/*  f125354:	ac9f0000 */ 	sw	$ra,0x0($a0)
/*  f125358:	00067c02 */ 	srl	$t7,$a2,0x10
/*  f12535c:	01a97024 */ 	and	$t6,$t5,$t1
/*  f125360:	01cfc825 */ 	or	$t9,$t6,$t7
/*  f125364:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f125368:	3c18b200 */ 	lui	$t8,0xb200
/*  f12536c:	acb80000 */ 	sw	$t8,0x0($a1)
/*  f125370:	acb90004 */ 	sw	$t9,0x4($a1)
/*  f125374:	02001825 */ 	or	$v1,$s0,$zero
/*  f125378:	ac7f0000 */ 	sw	$ra,0x0($v1)
/*  f12537c:	8fae012c */ 	lw	$t6,0x12c($sp)
/*  f125380:	8fb80130 */ 	lw	$t8,0x130($sp)
/*  f125384:	01e03025 */ 	or	$a2,$t7,$zero
/*  f125388:	01c9c824 */ 	and	$t9,$t6,$t1
/*  f12538c:	03097824 */ 	and	$t7,$t8,$t1
/*  f125390:	0019c402 */ 	srl	$t8,$t9,0x10
/*  f125394:	01f87025 */ 	or	$t6,$t7,$t8
/*  f125398:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f12539c:	02002025 */ 	or	$a0,$s0,$zero
/*  f1253a0:	01897824 */ 	and	$t7,$t4,$t1
/*  f1253a4:	ac6e0004 */ 	sw	$t6,0x4($v1)
/*  f1253a8:	01e6c025 */ 	or	$t8,$t7,$a2
/*  f1253ac:	3c19b200 */ 	lui	$t9,0xb200
/*  f1253b0:	ac990000 */ 	sw	$t9,0x0($a0)
/*  f1253b4:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1253b8:	ac980004 */ 	sw	$t8,0x4($a0)
/*  f1253bc:	02002825 */ 	or	$a1,$s0,$zero
/*  f1253c0:	000acc00 */ 	sll	$t9,$t2,0x10
/*  f1253c4:	310fffff */ 	andi	$t7,$t0,0xffff
/*  f1253c8:	032fc025 */ 	or	$t8,$t9,$t7
/*  f1253cc:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1253d0:	acb80004 */ 	sw	$t8,0x4($a1)
/*  f1253d4:	acbf0000 */ 	sw	$ra,0x0($a1)
/*  f1253d8:	02005825 */ 	or	$t3,$s0,$zero
/*  f1253dc:	304fffff */ 	andi	$t7,$v0,0xffff
/*  f1253e0:	000dcc00 */ 	sll	$t9,$t5,0x10
/*  f1253e4:	032fc025 */ 	or	$t8,$t9,$t7
/*  f1253e8:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f1253ec:	3c1fb200 */ 	lui	$ra,0xb200
/*  f1253f0:	ad7f0000 */ 	sw	$ra,0x0($t3)
/*  f1253f4:	ad780004 */ 	sw	$t8,0x4($t3)
/*  f1253f8:	02003825 */ 	or	$a3,$s0,$zero
/*  f1253fc:	3c0eb400 */ 	lui	$t6,0xb400
/*  f125400:	acee0000 */ 	sw	$t6,0x0($a3)
/*  f125404:	8fae012c */ 	lw	$t6,0x12c($sp)
/*  f125408:	8faf0130 */ 	lw	$t7,0x130($sp)
/*  f12540c:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f125410:	31d9ffff */ 	andi	$t9,$t6,0xffff
/*  f125414:	000fc400 */ 	sll	$t8,$t7,0x10
/*  f125418:	03197825 */ 	or	$t7,$t8,$t9
/*  f12541c:	acef0004 */ 	sw	$t7,0x4($a3)
/*  f125420:	3059ffff */ 	andi	$t9,$v0,0xffff
/*  f125424:	02004025 */ 	or	$t0,$s0,$zero
/*  f125428:	000cc400 */ 	sll	$t8,$t4,0x10
/*  f12542c:	03197825 */ 	or	$t7,$t8,$t9
/*  f125430:	3c013d00 */ 	lui	$at,0x3d00
/*  f125434:	ad0f0004 */ 	sw	$t7,0x4($t0)
/*  f125438:	ad1f0000 */ 	sw	$ra,0x0($t0)
/*  f12543c:	44817000 */ 	mtc1	$at,$f14
/*  f125440:	c7a40364 */ 	lwc1	$f4,0x364($sp)
/*  f125444:	c7a80368 */ 	lwc1	$f8,0x368($sp)
/*  f125448:	26100008 */ 	addiu	$s0,$s0,0x8
/*  f12544c:	460e2182 */ 	mul.s	$f6,$f4,$f14
/*  f125450:	c7a403ac */ 	lwc1	$f4,0x3ac($sp)
/*  f125454:	27a20204 */ 	addiu	$v0,$sp,0x204
/*  f125458:	460e4282 */ 	mul.s	$f10,$f8,$f14
/*  f12545c:	27a501f4 */ 	addiu	$a1,$sp,0x1f4
/*  f125460:	27a402d4 */ 	addiu	$a0,$sp,0x2d4
/*  f125464:	27a60214 */ 	addiu	$a2,$sp,0x214
/*  f125468:	e7a60234 */ 	swc1	$f6,0x234($sp)
/*  f12546c:	460e2182 */ 	mul.s	$f6,$f4,$f14
/*  f125470:	27a302f4 */ 	addiu	$v1,$sp,0x2f4
/*  f125474:	e7aa0238 */ 	swc1	$f10,0x238($sp)
/*  f125478:	3c013f80 */ 	lui	$at,0x3f80
/*  f12547c:	e7a6023c */ 	swc1	$f6,0x23c($sp)
.L0f125480:
/*  f125480:	c4800000 */ 	lwc1	$f0,0x0($a0)
/*  f125484:	4600a03e */ 	c.le.s	$f20,$f0
/*  f125488:	00000000 */ 	nop
/*  f12548c:	45020004 */ 	bc1fl	.L0f1254a0
/*  f125490:	46000307 */ 	neg.s	$f12,$f0
/*  f125494:	10000002 */ 	b	.L0f1254a0
/*  f125498:	46000306 */ 	mov.s	$f12,$f0
/*  f12549c:	46000307 */ 	neg.s	$f12,$f0
.L0f1254a0:
/*  f1254a0:	c4620000 */ 	lwc1	$f2,0x0($v1)
/*  f1254a4:	460e6202 */ 	mul.s	$f8,$f12,$f14
/*  f1254a8:	4602a03e */ 	c.le.s	$f20,$f2
/*  f1254ac:	00000000 */ 	nop
/*  f1254b0:	45000003 */ 	bc1f	.L0f1254c0
/*  f1254b4:	e4c80000 */ 	swc1	$f8,0x0($a2)
/*  f1254b8:	10000002 */ 	b	.L0f1254c4
/*  f1254bc:	46001306 */ 	mov.s	$f12,$f2
.L0f1254c0:
/*  f1254c0:	46001307 */ 	neg.s	$f12,$f2
.L0f1254c4:
/*  f1254c4:	460e6282 */ 	mul.s	$f10,$f12,$f14
/*  f1254c8:	24a50004 */ 	addiu	$a1,$a1,0x4
/*  f1254cc:	24630004 */ 	addiu	$v1,$v1,0x4
/*  f1254d0:	24840004 */ 	addiu	$a0,$a0,0x4
/*  f1254d4:	24c60004 */ 	addiu	$a2,$a2,0x4
/*  f1254d8:	14a2ffe9 */ 	bne	$a1,$v0,.L0f125480
/*  f1254dc:	e4aafffc */ 	swc1	$f10,-0x4($a1)
/*  f1254e0:	c7a20214 */ 	lwc1	$f2,0x214($sp)
/*  f1254e4:	c7a40234 */ 	lwc1	$f4,0x234($sp)
/*  f1254e8:	c7ac0218 */ 	lwc1	$f12,0x218($sp)
/*  f1254ec:	46021180 */ 	add.s	$f6,$f2,$f2
/*  f1254f0:	c7aa01f4 */ 	lwc1	$f10,0x1f4($sp)
/*  f1254f4:	c7ae021c */ 	lwc1	$f14,0x21c($sp)
/*  f1254f8:	44811000 */ 	mtc1	$at,$f2
/*  f1254fc:	46062200 */ 	add.s	$f8,$f4,$f6
/*  f125500:	c7a40238 */ 	lwc1	$f4,0x238($sp)
/*  f125504:	3c013a80 */ 	lui	$at,0x3a80
/*  f125508:	460c6180 */ 	add.s	$f6,$f12,$f12
/*  f12550c:	46085000 */ 	add.s	$f0,$f10,$f8
/*  f125510:	c7a801f8 */ 	lwc1	$f8,0x1f8($sp)
/*  f125514:	46062280 */ 	add.s	$f10,$f4,$f6
/*  f125518:	c7a4023c */ 	lwc1	$f4,0x23c($sp)
/*  f12551c:	460e7180 */ 	add.s	$f6,$f14,$f14
/*  f125520:	460a4400 */ 	add.s	$f16,$f8,$f10
/*  f125524:	c7aa01fc */ 	lwc1	$f10,0x1fc($sp)
/*  f125528:	46062200 */ 	add.s	$f8,$f4,$f6
/*  f12552c:	44812000 */ 	mtc1	$at,$f4
/*  f125530:	e7b001d8 */ 	swc1	$f16,0x1d8($sp)
/*  f125534:	4610003c */ 	c.lt.s	$f0,$f16
/*  f125538:	46085480 */ 	add.s	$f18,$f10,$f8
/*  f12553c:	45020004 */ 	bc1fl	.L0f125550
/*  f125540:	4612003c */ 	c.lt.s	$f0,$f18
/*  f125544:	46008006 */ 	mov.s	$f0,$f16
/*  f125548:	e7b001d8 */ 	swc1	$f16,0x1d8($sp)
/*  f12554c:	4612003c */ 	c.lt.s	$f0,$f18
.L0f125550:
/*  f125550:	e7b201dc */ 	swc1	$f18,0x1dc($sp)
/*  f125554:	45000003 */ 	bc1f	.L0f125564
/*  f125558:	00000000 */ 	nop
/*  f12555c:	46009006 */ 	mov.s	$f0,$f18
/*  f125560:	e7b201dc */ 	swc1	$f18,0x1dc($sp)
.L0f125564:
/*  f125564:	46040002 */ 	mul.s	$f0,$f0,$f4
/*  f125568:	4600103c */ 	c.lt.s	$f2,$f0
/*  f12556c:	00000000 */ 	nop
/*  f125570:	45020005 */ 	bc1fl	.L0f125588
/*  f125574:	e7a001d4 */ 	swc1	$f0,0x1d4($sp)
/*  f125578:	46001503 */ 	div.s	$f20,$f2,$f0
/*  f12557c:	10000003 */ 	b	.L0f12558c
/*  f125580:	e7a001d4 */ 	swc1	$f0,0x1d4($sp)
/*  f125584:	e7a001d4 */ 	swc1	$f0,0x1d4($sp)
.L0f125588:
/*  f125588:	46001506 */ 	mov.s	$f20,$f2
.L0f12558c:
/*  f12558c:	c7a60254 */ 	lwc1	$f6,0x254($sp)
/*  f125590:	e7b401c4 */ 	swc1	$f20,0x1c4($sp)
/*  f125594:	4606a302 */ 	mul.s	$f12,$f20,$f6
/*  f125598:	0fc54be8 */ 	jal	func0f152fa0
/*  f12559c:	00000000 */ 	nop
/*  f1255a0:	c7b401c4 */ 	lwc1	$f20,0x1c4($sp)
/*  f1255a4:	c7aa0258 */ 	lwc1	$f10,0x258($sp)
/*  f1255a8:	afa200e0 */ 	sw	$v0,0xe0($sp)
/*  f1255ac:	460aa302 */ 	mul.s	$f12,$f20,$f10
/*  f1255b0:	0fc54be8 */ 	jal	func0f152fa0
/*  f1255b4:	00000000 */ 	nop
/*  f1255b8:	c7b401c4 */ 	lwc1	$f20,0x1c4($sp)
/*  f1255bc:	c7a8025c */ 	lwc1	$f8,0x25c($sp)
/*  f1255c0:	afa200dc */ 	sw	$v0,0xdc($sp)
/*  f1255c4:	4608a302 */ 	mul.s	$f12,$f20,$f8
/*  f1255c8:	0fc54be8 */ 	jal	func0f152fa0
/*  f1255cc:	00000000 */ 	nop
/*  f1255d0:	c7b401c4 */ 	lwc1	$f20,0x1c4($sp)
/*  f1255d4:	c7a40260 */ 	lwc1	$f4,0x260($sp)
/*  f1255d8:	afa200d8 */ 	sw	$v0,0xd8($sp)
/*  f1255dc:	4604a302 */ 	mul.s	$f12,$f20,$f4
/*  f1255e0:	0fc54be8 */ 	jal	func0f152fa0
/*  f1255e4:	00000000 */ 	nop
/*  f1255e8:	c7b401c4 */ 	lwc1	$f20,0x1c4($sp)
/*  f1255ec:	c7a602d4 */ 	lwc1	$f6,0x2d4($sp)
/*  f1255f0:	afa200d4 */ 	sw	$v0,0xd4($sp)
/*  f1255f4:	4606a302 */ 	mul.s	$f12,$f20,$f6
/*  f1255f8:	0fc54be8 */ 	jal	func0f152fa0
/*  f1255fc:	00000000 */ 	nop
/*  f125600:	c7b401c4 */ 	lwc1	$f20,0x1c4($sp)
/*  f125604:	c7aa02d8 */ 	lwc1	$f10,0x2d8($sp)
/*  f125608:	afa200d0 */ 	sw	$v0,0xd0($sp)
/*  f12560c:	460aa302 */ 	mul.s	$f12,$f20,$f10
/*  f125610:	0fc54be8 */ 	jal	func0f152fa0
/*  f125614:	00000000 */ 	nop
/*  f125618:	c7b401c4 */ 	lwc1	$f20,0x1c4($sp)
/*  f12561c:	c7a802dc */ 	lwc1	$f8,0x2dc($sp)
/*  f125620:	afa200cc */ 	sw	$v0,0xcc($sp)
/*  f125624:	4608a302 */ 	mul.s	$f12,$f20,$f8
/*  f125628:	0fc54be8 */ 	jal	func0f152fa0
/*  f12562c:	00000000 */ 	nop
/*  f125630:	c7b401c4 */ 	lwc1	$f20,0x1c4($sp)
/*  f125634:	c7a402e0 */ 	lwc1	$f4,0x2e0($sp)
/*  f125638:	afa200c8 */ 	sw	$v0,0xc8($sp)
/*  f12563c:	4604a302 */ 	mul.s	$f12,$f20,$f4
/*  f125640:	0fc54be8 */ 	jal	func0f152fa0
/*  f125644:	00000000 */ 	nop
/*  f125648:	c7b401c4 */ 	lwc1	$f20,0x1c4($sp)
/*  f12564c:	c7a602f4 */ 	lwc1	$f6,0x2f4($sp)
/*  f125650:	afa200c4 */ 	sw	$v0,0xc4($sp)
/*  f125654:	4606a302 */ 	mul.s	$f12,$f20,$f6
/*  f125658:	0fc54be8 */ 	jal	func0f152fa0
/*  f12565c:	00000000 */ 	nop
/*  f125660:	c7b401c4 */ 	lwc1	$f20,0x1c4($sp)
/*  f125664:	c7aa02f8 */ 	lwc1	$f10,0x2f8($sp)
/*  f125668:	afa200b0 */ 	sw	$v0,0xb0($sp)
/*  f12566c:	460aa302 */ 	mul.s	$f12,$f20,$f10
/*  f125670:	0fc54be8 */ 	jal	func0f152fa0
/*  f125674:	00000000 */ 	nop
/*  f125678:	c7b401c4 */ 	lwc1	$f20,0x1c4($sp)
/*  f12567c:	c7a802fc */ 	lwc1	$f8,0x2fc($sp)
/*  f125680:	afa200ac */ 	sw	$v0,0xac($sp)
/*  f125684:	4608a302 */ 	mul.s	$f12,$f20,$f8
/*  f125688:	0fc54be8 */ 	jal	func0f152fa0
/*  f12568c:	00000000 */ 	nop
/*  f125690:	c7b401c4 */ 	lwc1	$f20,0x1c4($sp)
/*  f125694:	c7a40300 */ 	lwc1	$f4,0x300($sp)
/*  f125698:	afa200a8 */ 	sw	$v0,0xa8($sp)
/*  f12569c:	4604a302 */ 	mul.s	$f12,$f20,$f4
/*  f1256a0:	0fc54be8 */ 	jal	func0f152fa0
/*  f1256a4:	00000000 */ 	nop
/*  f1256a8:	c7b401c4 */ 	lwc1	$f20,0x1c4($sp)
/*  f1256ac:	c7a60274 */ 	lwc1	$f6,0x274($sp)
/*  f1256b0:	afa200a4 */ 	sw	$v0,0xa4($sp)
/*  f1256b4:	4606a302 */ 	mul.s	$f12,$f20,$f6
/*  f1256b8:	0fc54be8 */ 	jal	func0f152fa0
/*  f1256bc:	00000000 */ 	nop
/*  f1256c0:	c7b401c4 */ 	lwc1	$f20,0x1c4($sp)
/*  f1256c4:	c7aa0278 */ 	lwc1	$f10,0x278($sp)
/*  f1256c8:	afa200c0 */ 	sw	$v0,0xc0($sp)
/*  f1256cc:	460aa302 */ 	mul.s	$f12,$f20,$f10
/*  f1256d0:	0fc54be8 */ 	jal	func0f152fa0
/*  f1256d4:	00000000 */ 	nop
/*  f1256d8:	c7b401c4 */ 	lwc1	$f20,0x1c4($sp)
/*  f1256dc:	c7a8027c */ 	lwc1	$f8,0x27c($sp)
/*  f1256e0:	afa200bc */ 	sw	$v0,0xbc($sp)
/*  f1256e4:	4608a302 */ 	mul.s	$f12,$f20,$f8
/*  f1256e8:	0fc54be8 */ 	jal	func0f152fa0
/*  f1256ec:	00000000 */ 	nop
/*  f1256f0:	c7b401c4 */ 	lwc1	$f20,0x1c4($sp)
/*  f1256f4:	c7a40280 */ 	lwc1	$f4,0x280($sp)
/*  f1256f8:	afa200b8 */ 	sw	$v0,0xb8($sp)
/*  f1256fc:	4604a302 */ 	mul.s	$f12,$f20,$f4
/*  f125700:	0fc54be8 */ 	jal	func0f152fa0
/*  f125704:	00000000 */ 	nop
/*  f125708:	8fa800b8 */ 	lw	$t0,0xb8($sp)
/*  f12570c:	8fab00d4 */ 	lw	$t3,0xd4($sp)
/*  f125710:	8fbf00e0 */ 	lw	$ra,0xe0($sp)
/*  f125714:	3c0db400 */ 	lui	$t5,0xb400
/*  f125718:	ae0d0000 */ 	sw	$t5,0x0($s0)
/*  f12571c:	8fb800dc */ 	lw	$t8,0xdc($sp)
/*  f125720:	3c09ffff */ 	lui	$t1,0xffff
/*  f125724:	03e97024 */ 	and	$t6,$ra,$t1
/*  f125728:	0309c824 */ 	and	$t9,$t8,$t1
/*  f12572c:	00197c02 */ 	srl	$t7,$t9,0x10
/*  f125730:	01cfc025 */ 	or	$t8,$t6,$t7
/*  f125734:	ae180004 */ 	sw	$t8,0x4($s0)
/*  f125738:	26040008 */ 	addiu	$a0,$s0,0x8
/*  f12573c:	3c19b200 */ 	lui	$t9,0xb200
/*  f125740:	ac990000 */ 	sw	$t9,0x0($a0)
/*  f125744:	8fae00d8 */ 	lw	$t6,0xd8($sp)
/*  f125748:	0169c024 */ 	and	$t8,$t3,$t1
/*  f12574c:	0018cc02 */ 	srl	$t9,$t8,0x10
/*  f125750:	01c97824 */ 	and	$t7,$t6,$t1
/*  f125754:	01f97025 */ 	or	$t6,$t7,$t9
/*  f125758:	ac8e0004 */ 	sw	$t6,0x4($a0)
/*  f12575c:	24850008 */ 	addiu	$a1,$a0,0x8
/*  f125760:	acad0000 */ 	sw	$t5,0x0($a1)
/*  f125764:	8fb900cc */ 	lw	$t9,0xcc($sp)
/*  f125768:	8fb800d0 */ 	lw	$t8,0xd0($sp)
/*  f12576c:	24a60008 */ 	addiu	$a2,$a1,0x8
/*  f125770:	03297024 */ 	and	$t6,$t9,$t1
/*  f125774:	03097824 */ 	and	$t7,$t8,$t1
/*  f125778:	000ec402 */ 	srl	$t8,$t6,0x10
/*  f12577c:	01f8c825 */ 	or	$t9,$t7,$t8
/*  f125780:	acb90004 */ 	sw	$t9,0x4($a1)
/*  f125784:	8fac00c4 */ 	lw	$t4,0xc4($sp)
/*  f125788:	3c0eb200 */ 	lui	$t6,0xb200
/*  f12578c:	acce0000 */ 	sw	$t6,0x0($a2)
/*  f125790:	8faf00c8 */ 	lw	$t7,0xc8($sp)
/*  f125794:	0189c824 */ 	and	$t9,$t4,$t1
/*  f125798:	00197402 */ 	srl	$t6,$t9,0x10
/*  f12579c:	01e9c024 */ 	and	$t8,$t7,$t1
/*  f1257a0:	030e7825 */ 	or	$t7,$t8,$t6
/*  f1257a4:	accf0004 */ 	sw	$t7,0x4($a2)
/*  f1257a8:	24c30008 */ 	addiu	$v1,$a2,0x8
/*  f1257ac:	ac6d0000 */ 	sw	$t5,0x0($v1)
/*  f1257b0:	8fae00dc */ 	lw	$t6,0xdc($sp)
/*  f1257b4:	001fc400 */ 	sll	$t8,$ra,0x10
/*  f1257b8:	24700008 */ 	addiu	$s0,$v1,0x8
/*  f1257bc:	31cfffff */ 	andi	$t7,$t6,0xffff
/*  f1257c0:	030fc825 */ 	or	$t9,$t8,$t7
/*  f1257c4:	ac790004 */ 	sw	$t9,0x4($v1)
/*  f1257c8:	3c0eb200 */ 	lui	$t6,0xb200
/*  f1257cc:	ae0e0000 */ 	sw	$t6,0x0($s0)
/*  f1257d0:	8faf00d8 */ 	lw	$t7,0xd8($sp)
/*  f1257d4:	316effff */ 	andi	$t6,$t3,0xffff
/*  f1257d8:	26070008 */ 	addiu	$a3,$s0,0x8
/*  f1257dc:	000fcc00 */ 	sll	$t9,$t7,0x10
/*  f1257e0:	032ec025 */ 	or	$t8,$t9,$t6
/*  f1257e4:	ae180004 */ 	sw	$t8,0x4($s0)
/*  f1257e8:	aced0000 */ 	sw	$t5,0x0($a3)
/*  f1257ec:	8fb800cc */ 	lw	$t8,0xcc($sp)
/*  f1257f0:	8fb900d0 */ 	lw	$t9,0xd0($sp)
/*  f1257f4:	8fab00c0 */ 	lw	$t3,0xc0($sp)
/*  f1257f8:	330fffff */ 	andi	$t7,$t8,0xffff
/*  f1257fc:	00197400 */ 	sll	$t6,$t9,0x10
/*  f125800:	8fa600bc */ 	lw	$a2,0xbc($sp)
/*  f125804:	01cfc825 */ 	or	$t9,$t6,$t7
/*  f125808:	acf90004 */ 	sw	$t9,0x4($a3)
/*  f12580c:	3c0db200 */ 	lui	$t5,0xb200
/*  f125810:	24e30008 */ 	addiu	$v1,$a3,0x8
/*  f125814:	ac6d0000 */ 	sw	$t5,0x0($v1)
/*  f125818:	8fae00c8 */ 	lw	$t6,0xc8($sp)
/*  f12581c:	3199ffff */ 	andi	$t9,$t4,0xffff
/*  f125820:	24640008 */ 	addiu	$a0,$v1,0x8
/*  f125824:	000e7c00 */ 	sll	$t7,$t6,0x10
/*  f125828:	01f9c025 */ 	or	$t8,$t7,$t9
/*  f12582c:	ac780004 */ 	sw	$t8,0x4($v1)
/*  f125830:	3c0eb400 */ 	lui	$t6,0xb400
/*  f125834:	00c9c824 */ 	and	$t9,$a2,$t1
/*  f125838:	0019c402 */ 	srl	$t8,$t9,0x10
/*  f12583c:	ac8e0000 */ 	sw	$t6,0x0($a0)
/*  f125840:	01697824 */ 	and	$t7,$t3,$t1
/*  f125844:	01f87025 */ 	or	$t6,$t7,$t8
/*  f125848:	00497824 */ 	and	$t7,$v0,$t1
/*  f12584c:	ac8e0004 */ 	sw	$t6,0x4($a0)
/*  f125850:	000fc402 */ 	srl	$t8,$t7,0x10
/*  f125854:	0109c824 */ 	and	$t9,$t0,$t1
/*  f125858:	24850008 */ 	addiu	$a1,$a0,0x8
/*  f12585c:	03387025 */ 	or	$t6,$t9,$t8
/*  f125860:	acae0004 */ 	sw	$t6,0x4($a1)
/*  f125864:	acad0000 */ 	sw	$t5,0x0($a1)
/*  f125868:	8fbf00ac */ 	lw	$ra,0xac($sp)
/*  f12586c:	8fad00b0 */ 	lw	$t5,0xb0($sp)
/*  f125870:	00405025 */ 	or	$t2,$v0,$zero
/*  f125874:	24b00008 */ 	addiu	$s0,$a1,0x8
/*  f125878:	8fac00a8 */ 	lw	$t4,0xa8($sp)
/*  f12587c:	8fa700a4 */ 	lw	$a3,0xa4($sp)
/*  f125880:	02001025 */ 	or	$v0,$s0,$zero
/*  f125884:	3c0fb400 */ 	lui	$t7,0xb400
/*  f125888:	03e9c024 */ 	and	$t8,$ra,$t1
/*  f12588c:	00187402 */ 	srl	$t6,$t8,0x10
/*  f125890:	ac4f0000 */ 	sw	$t7,0x0($v0)
/*  f125894:	01a9c824 */ 	and	$t9,$t5,$t1
/*  f125898:	032e7825 */ 	or	$t7,$t9,$t6
/*  f12589c:	ac4f0004 */ 	sw	$t7,0x4($v0)
/*  f1258a0:	26030008 */ 	addiu	$v1,$s0,0x8
/*  f1258a4:	3c18b200 */ 	lui	$t8,0xb200
/*  f1258a8:	00e97024 */ 	and	$t6,$a3,$t1
/*  f1258ac:	000e7c02 */ 	srl	$t7,$t6,0x10
/*  f1258b0:	ac780000 */ 	sw	$t8,0x0($v1)
/*  f1258b4:	0189c824 */ 	and	$t9,$t4,$t1
/*  f1258b8:	032fc025 */ 	or	$t8,$t9,$t7
/*  f1258bc:	ac780004 */ 	sw	$t8,0x4($v1)
/*  f1258c0:	24700008 */ 	addiu	$s0,$v1,0x8
/*  f1258c4:	3c0eb400 */ 	lui	$t6,0xb400
/*  f1258c8:	ae0e0000 */ 	sw	$t6,0x0($s0)
/*  f1258cc:	000b7c00 */ 	sll	$t7,$t3,0x10
/*  f1258d0:	30d8ffff */ 	andi	$t8,$a2,0xffff
/*  f1258d4:	01f87025 */ 	or	$t6,$t7,$t8
/*  f1258d8:	ae0e0004 */ 	sw	$t6,0x4($s0)
/*  f1258dc:	26050008 */ 	addiu	$a1,$s0,0x8
/*  f1258e0:	3c19b200 */ 	lui	$t9,0xb200
/*  f1258e4:	acb90000 */ 	sw	$t9,0x0($a1)
/*  f1258e8:	314effff */ 	andi	$t6,$t2,0xffff
/*  f1258ec:	0008c400 */ 	sll	$t8,$t0,0x10
/*  f1258f0:	030ec825 */ 	or	$t9,$t8,$t6
/*  f1258f4:	acb90004 */ 	sw	$t9,0x4($a1)
/*  f1258f8:	24a30008 */ 	addiu	$v1,$a1,0x8
/*  f1258fc:	3c0fb400 */ 	lui	$t7,0xb400
/*  f125900:	ac6f0000 */ 	sw	$t7,0x0($v1)
/*  f125904:	33f9ffff */ 	andi	$t9,$ra,0xffff
/*  f125908:	000d7400 */ 	sll	$t6,$t5,0x10
/*  f12590c:	01d97825 */ 	or	$t7,$t6,$t9
/*  f125910:	ac6f0004 */ 	sw	$t7,0x4($v1)
/*  f125914:	24640008 */ 	addiu	$a0,$v1,0x8
/*  f125918:	3c18b300 */ 	lui	$t8,0xb300
/*  f12591c:	ac980000 */ 	sw	$t8,0x0($a0)
/*  f125920:	000ccc00 */ 	sll	$t9,$t4,0x10
/*  f125924:	30efffff */ 	andi	$t7,$a3,0xffff
/*  f125928:	032fc025 */ 	or	$t8,$t9,$t7
/*  f12592c:	ac980004 */ 	sw	$t8,0x4($a0)
/*  f125930:	24820008 */ 	addiu	$v0,$a0,0x8
.L0f125934:
/*  f125934:	8fbf0024 */ 	lw	$ra,0x24($sp)
/*  f125938:	d7b40018 */ 	ldc1	$f20,0x18($sp)
/*  f12593c:	8fb00020 */ 	lw	$s0,0x20($sp)
/*  f125940:	03e00008 */ 	jr	$ra
/*  f125944:	27bd04d0 */ 	addiu	$sp,$sp,0x4d0
);

/**
 * With this function stubbed, sun artifacts and glare do not render.
 */
void sky0f125948(struct bootbufferthingdeep *arg0, s32 x, s32 y)
{
	s32 viewleft = viGetViewLeft();
	s32 viewtop = viGetViewTop();
	s32 viewwidth = viGetViewWidth();
	s32 viewheight = viGetViewHeight();

	if (x >= viewleft && x < viewleft + viewwidth && y >= viewtop && y < viewtop + viewheight) {
		arg0->unk08 = &var800844f0[(s32)camGetScreenWidth() * y + x];
		arg0->unk0c.u16_2 = x;
		arg0->unk0c.u16_1 = y;
		arg0->unk00 = 1;
	}
}

f32 sky0f125a1c(struct bootbufferthingdeep *arg0)
{
	f32 sum = 0;
	s32 i;

	for (i = 0; i < 8; i++) {
		if (arg0[i].unk00 == 1 && arg0[i].unk02 == 0xfffc) {
			sum += 0.125f;
		}
	}

	return sum;
}

Gfx *skyRenderSuns(Gfx *gdl, bool xray)
{
	Mtxf *sp16c;
	Mtxf *sp168;
	s16 viewleft;
	s16 viewtop;
	s16 viewwidth;
	s16 viewheight;
	f32 viewleftf;
	f32 viewtopf;
	f32 viewwidthf;
	f32 viewheightf;
	struct bootbufferthing *thing;
	u8 colour[3];
	struct environment *env;
	struct sun *sun;
	s32 i;
	f32 sp134[2];
	f32 sp12c[2];
	s32 xscale;
	f32 sp124;
	bool onscreen;
	f32 radius;

	sp16c = camGetWorldToScreenMtxf();
	sp168 = camGetMtxF1754();
	env = envGetCurrent();

	xscale = 1;

	if (env->numsuns <= 0 || !var800844f0 || g_Vars.mplayerisrunning) {
		return gdl;
	}

#if !PAL
	if (g_ViRes == 1) {
		xscale = 2;
	}
#endif

	viewleft = viGetViewLeft();
	viewtop = viGetViewTop();
	viewwidth = viGetViewWidth();
	viewheight = viGetViewHeight();

	viewleftf = viewleft;
	viewtopf = viewtop;
	viewwidthf = viewwidth;
	viewheightf = viewheight;

	sun = env->suns;

	for (i = 0; i < env->numsuns; i++) {
		g_SunPositions[i].f[0] = sun->pos[0];
		g_SunPositions[i].f[1] = sun->pos[1];
		g_SunPositions[i].f[2] = sun->pos[2];

		colour[0] = sun->red;
		colour[1] = sun->green;
		colour[2] = sun->blue;

		if (!xray) {
			mtx4TransformVecInPlace(sp16c, &g_SunPositions[i]);
			mtx4TransformVecInPlace(sp168, &g_SunPositions[i]);

			if (g_SunPositions[i].f[2] > 1.0f) {
				g_SunScreenXPositions[i] = (g_SunPositions[i].f[0] / g_SunPositions[i].f[2] + 1.0f) * 0.5f * viewwidthf + viewleftf;
				g_SunScreenYPositions[i] = (-g_SunPositions[i].f[1] / g_SunPositions[i].f[2] + 1.0f) * 0.5f * viewheightf + viewtopf;
				radius = 60.0f / viGetFovY() * sun->texture_size;
				onscreen = false;

				if (g_SunScreenXPositions[i] >= viewleftf - radius
						&& g_SunScreenXPositions[i] < viewleftf + viewwidth + radius
						&& g_SunScreenYPositions[i] >= viewtopf - radius
						&& g_SunScreenYPositions[i] < viewtopf + viewheightf + radius) {
					// Sun is at least partially on screen
					if (g_SunScreenXPositions[i] >= viewleftf
							&& g_SunScreenXPositions[i] < viewleftf + viewwidthf
							&& g_SunScreenYPositions[i] >= viewtopf
							&& g_SunScreenYPositions[i] < viewtopf + viewheightf) {
						// Sun's centre point is on-screen
						f32 distfromedge;
						f32 mindistfromedge;
						thing = bbufGetIndex0Buffer();
						onscreen = true;
						mindistfromedge = 1000;

						if ((s32)g_SunScreenXPositions[i] < viewleft + 15) {
							distfromedge = g_SunScreenXPositions[i];

							if (distfromedge < mindistfromedge) {
								mindistfromedge = distfromedge;
							}
						}

						if (1);

						if ((s32)g_SunScreenYPositions[i] < viewtop + 15) {
							distfromedge = g_SunScreenYPositions[i];

							if (distfromedge < mindistfromedge) {
								mindistfromedge = distfromedge;
							}
						}

						if ((s32)g_SunScreenXPositions[i] > viewleft + viewwidth - 16) {
							distfromedge = viewleft + viewwidth - 1 - g_SunScreenXPositions[i];

							if (distfromedge < mindistfromedge) {
								mindistfromedge = distfromedge;
							}
						}

						if ((s32)g_SunScreenYPositions[i] > viewtop + viewheight - 16) {
							distfromedge = viewtop + viewheight - 1 - g_SunScreenYPositions[i];

							if (distfromedge < mindistfromedge) {
								mindistfromedge = distfromedge;
							}
						}

						mindistfromedge -= 1.0f;

						if (mindistfromedge < 0.0f) {
							mindistfromedge = 0.0f;
						}

						g_SunAlphaFracs[i] = mindistfromedge * (1.0f / 15.0f);

						if (g_SunAlphaFracs[i] > 1.0f) {
							g_SunAlphaFracs[i] = 1.0f;
						}

						sky0f125948(&(i + thing->unk00)->unk00[0], (s32)g_SunScreenXPositions[i] - 7, (s32)g_SunScreenYPositions[i] + 1);
						sky0f125948(&(i + thing->unk00)->unk00[1], (s32)g_SunScreenXPositions[i] - 5, (s32)g_SunScreenYPositions[i] - 3);
						sky0f125948(&(i + thing->unk00)->unk00[2], (s32)g_SunScreenXPositions[i] - 3, (s32)g_SunScreenYPositions[i] + 5);
						sky0f125948(&(i + thing->unk00)->unk00[3], (s32)g_SunScreenXPositions[i] - 1, (s32)g_SunScreenYPositions[i] - 7);
						sky0f125948(&(i + thing->unk00)->unk00[4], (s32)g_SunScreenXPositions[i] + 1, (s32)g_SunScreenYPositions[i] + 7);
						sky0f125948(&(i + thing->unk00)->unk00[5], (s32)g_SunScreenXPositions[i] + 3, (s32)g_SunScreenYPositions[i] - 5);
						sky0f125948(&(i + thing->unk00)->unk00[6], (s32)g_SunScreenXPositions[i] + 5, (s32)g_SunScreenYPositions[i] + 3);
						sky0f125948(&(i + thing->unk00)->unk00[7], (s32)g_SunScreenXPositions[i] + 7, (s32)g_SunScreenYPositions[i] - 1);
					}

					if (1);

					g_SunFlareTimers240[i] += g_Vars.lvupdate240;

					texSelect(&gdl, &g_TexLightGlareConfigs[5], 4, 0, 2, 1, NULL);

					gDPSetCycleType(gdl++, G_CYC_1CYCLE);
					gDPSetColorDither(gdl++, G_CD_DISABLE);
					gDPSetRenderMode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
					gDPSetTexturePersp(gdl++, G_TP_NONE);
					gDPSetAlphaCompare(gdl++, G_AC_NONE);
					gDPSetTextureLOD(gdl++, G_TL_TILE);
					gDPSetTextureConvert(gdl++, G_TC_FILT);
					gDPSetTextureLUT(gdl++, G_TT_NONE);
					gDPSetTextureFilter(gdl++, G_TF_BILERP);
					gDPSetCombineLERP(gdl++,
							ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0,
							ENVIRONMENT, 0, TEXEL0, 0, ENVIRONMENT, 0, TEXEL0, 0);
					gDPSetEnvColor(gdl++, colour[0], colour[1], colour[2], (s32)(g_SunAlphaFracs[i] * 255.0f));

					sp134[0] = g_SunScreenXPositions[i];
					sp134[1] = g_SunScreenYPositions[i];
					sp12c[0] = radius * 0.50f * xscale;
					sp12c[1] = radius * 0.50f;

					func0f0b2150(&gdl, sp134, sp12c, g_TexLightGlareConfigs[5].width, g_TexLightGlareConfigs[5].height, 0, 1, 1, 1, 0, 1);

					gDPPipeSync(gdl++);
					gDPSetColorDither(gdl++, G_CD_BAYER);
					gDPSetTexturePersp(gdl++, G_TP_PERSP);
					gDPSetTextureLOD(gdl++, G_TL_LOD);

					sp124 = sky0f125a1c(bbufGetIndex1Buffer()->unk00[i].unk00);
				}

				if (onscreen && sp124 > 0.0f) {
					g_SunFlareTimers240[i] += g_Vars.lvupdate240;
				} else {
					g_SunFlareTimers240[i] = 0;
				}
			}
		}

		sun++;
	}

	return gdl;
}

#if VERSION == VERSION_PAL_FINAL
GLOBAL_ASM(
glabel sky0f126384
.late_rodata
glabel var7f1b510c
.word 0x3c23d70a
glabel var7f1b5110
.word 0x3bda740e
glabel var7f1b5114
.word 0x3d23d70a
glabel var7f1b5118
.word 0x3c4ccccd
glabel var7f1b511c
.word 0x3dcccccd
.text
/*  f126e68:	27bdfe68 */ 	addiu	$sp,$sp,-408
/*  f126e6c:	3c0f8008 */ 	lui	$t7,0x8008
/*  f126e70:	afbf008c */ 	sw	$ra,0x8c($sp)
/*  f126e74:	afbe0088 */ 	sw	$s8,0x88($sp)
/*  f126e78:	afb70084 */ 	sw	$s7,0x84($sp)
/*  f126e7c:	afb60080 */ 	sw	$s6,0x80($sp)
/*  f126e80:	afb5007c */ 	sw	$s5,0x7c($sp)
/*  f126e84:	afb40078 */ 	sw	$s4,0x78($sp)
/*  f126e88:	afb30074 */ 	sw	$s3,0x74($sp)
/*  f126e8c:	afb20070 */ 	sw	$s2,0x70($sp)
/*  f126e90:	afb1006c */ 	sw	$s1,0x6c($sp)
/*  f126e94:	afb00068 */ 	sw	$s0,0x68($sp)
/*  f126e98:	f7be0060 */ 	sdc1	$f30,0x60($sp)
/*  f126e9c:	f7bc0058 */ 	sdc1	$f28,0x58($sp)
/*  f126ea0:	f7ba0050 */ 	sdc1	$f26,0x50($sp)
/*  f126ea4:	f7b80048 */ 	sdc1	$f24,0x48($sp)
/*  f126ea8:	f7b60040 */ 	sdc1	$f22,0x40($sp)
/*  f126eac:	f7b40038 */ 	sdc1	$f20,0x38($sp)
/*  f126eb0:	afa40198 */ 	sw	$a0,0x198($sp)
/*  f126eb4:	25efe070 */ 	addiu	$t7,$t7,-8080
/*  f126eb8:	8de10000 */ 	lw	$at,0x0($t7)
/*  f126ebc:	8de80004 */ 	lw	$t0,0x4($t7)
/*  f126ec0:	27ae0164 */ 	addiu	$t6,$sp,0x164
/*  f126ec4:	adc10000 */ 	sw	$at,0x0($t6)
/*  f126ec8:	adc80004 */ 	sw	$t0,0x4($t6)
/*  f126ecc:	8de8000c */ 	lw	$t0,0xc($t7)
/*  f126ed0:	8de10008 */ 	lw	$at,0x8($t7)
/*  f126ed4:	3c0a8008 */ 	lui	$t2,0x8008
/*  f126ed8:	adc8000c */ 	sw	$t0,0xc($t6)
/*  f126edc:	adc10008 */ 	sw	$at,0x8($t6)
/*  f126ee0:	8de10010 */ 	lw	$at,0x10($t7)
/*  f126ee4:	8de80014 */ 	lw	$t0,0x14($t7)
/*  f126ee8:	254ae088 */ 	addiu	$t2,$t2,-8056
/*  f126eec:	adc10010 */ 	sw	$at,0x10($t6)
/*  f126ef0:	adc80014 */ 	sw	$t0,0x14($t6)
/*  f126ef4:	8d4d0004 */ 	lw	$t5,0x4($t2)
/*  f126ef8:	8d410000 */ 	lw	$at,0x0($t2)
/*  f126efc:	27a9014c */ 	addiu	$t1,$sp,0x14c
/*  f126f00:	ad2d0004 */ 	sw	$t5,0x4($t1)
/*  f126f04:	ad210000 */ 	sw	$at,0x0($t1)
/*  f126f08:	8d410008 */ 	lw	$at,0x8($t2)
/*  f126f0c:	8d4d000c */ 	lw	$t5,0xc($t2)
/*  f126f10:	3c188008 */ 	lui	$t8,0x8008
/*  f126f14:	ad210008 */ 	sw	$at,0x8($t1)
/*  f126f18:	ad2d000c */ 	sw	$t5,0xc($t1)
/*  f126f1c:	8d4d0014 */ 	lw	$t5,0x14($t2)
/*  f126f20:	8d410010 */ 	lw	$at,0x10($t2)
/*  f126f24:	2718e0a0 */ 	addiu	$t8,$t8,-8032
/*  f126f28:	ad2d0014 */ 	sw	$t5,0x14($t1)
/*  f126f2c:	ad210010 */ 	sw	$at,0x10($t1)
/*  f126f30:	8f080004 */ 	lw	$t0,0x4($t8)
/*  f126f34:	8f010000 */ 	lw	$at,0x0($t8)
/*  f126f38:	27b90134 */ 	addiu	$t9,$sp,0x134
/*  f126f3c:	af280004 */ 	sw	$t0,0x4($t9)
/*  f126f40:	af210000 */ 	sw	$at,0x0($t9)
/*  f126f44:	8f010008 */ 	lw	$at,0x8($t8)
/*  f126f48:	8f08000c */ 	lw	$t0,0xc($t8)
/*  f126f4c:	4487b000 */ 	mtc1	$a3,$f22
/*  f126f50:	af210008 */ 	sw	$at,0x8($t9)
/*  f126f54:	af28000c */ 	sw	$t0,0xc($t9)
/*  f126f58:	8f080014 */ 	lw	$t0,0x14($t8)
/*  f126f5c:	8f010010 */ 	lw	$at,0x10($t8)
/*  f126f60:	4485d000 */ 	mtc1	$a1,$f26
/*  f126f64:	4486e000 */ 	mtc1	$a2,$f28
/*  f126f68:	af280014 */ 	sw	$t0,0x14($t9)
/*  f126f6c:	0c002e73 */ 	jal	viGetViewWidth
/*  f126f70:	af210010 */ 	sw	$at,0x10($t9)
/*  f126f74:	44822000 */ 	mtc1	$v0,$f4
/*  f126f78:	3c017f1b */ 	lui	$at,0x7f1b
/*  f126f7c:	c43463fc */ 	lwc1	$f20,0x63fc($at)
/*  f126f80:	468021a0 */ 	cvt.s.w	$f6,$f4
/*  f126f84:	3c013f00 */ 	lui	$at,0x3f00
/*  f126f88:	4481f000 */ 	mtc1	$at,$f30
/*  f126f8c:	00000000 */ 	nop
/*  f126f90:	461e3202 */ 	mul.s	$f8,$f6,$f30
/*  f126f94:	4608d281 */ 	sub.s	$f10,$f26,$f8
/*  f126f98:	46145102 */ 	mul.s	$f4,$f10,$f20
/*  f126f9c:	0c002e77 */ 	jal	viGetViewHeight
/*  f126fa0:	e7a40130 */ 	swc1	$f4,0x130($sp)
/*  f126fa4:	44823000 */ 	mtc1	$v0,$f6
/*  f126fa8:	3c16800b */ 	lui	$s6,0x800b
/*  f126fac:	26d6bb40 */ 	addiu	$s6,$s6,-17600
/*  f126fb0:	46803220 */ 	cvt.s.w	$f8,$f6
/*  f126fb4:	8ec50000 */ 	lw	$a1,0x0($s6)
/*  f126fb8:	27b50198 */ 	addiu	$s5,$sp,0x198
/*  f126fbc:	240c0002 */ 	li	$t4,0x2
/*  f126fc0:	240b0001 */ 	li	$t3,0x1
/*  f126fc4:	afab0014 */ 	sw	$t3,0x14($sp)
/*  f126fc8:	461e4282 */ 	mul.s	$f10,$f8,$f30
/*  f126fcc:	afac0010 */ 	sw	$t4,0x10($sp)
/*  f126fd0:	02a02025 */ 	move	$a0,$s5
/*  f126fd4:	24060004 */ 	li	$a2,0x4
/*  f126fd8:	00003825 */ 	move	$a3,$zero
/*  f126fdc:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f126fe0:	24a50048 */ 	addiu	$a1,$a1,0x48
/*  f126fe4:	460ae101 */ 	sub.s	$f4,$f28,$f10
/*  f126fe8:	46142182 */ 	mul.s	$f6,$f4,$f20
/*  f126fec:	0fc2cfb8 */ 	jal	texSelect
/*  f126ff0:	e7a6012c */ 	swc1	$f6,0x12c($sp)
/*  f126ff4:	8fa90198 */ 	lw	$t1,0x198($sp)
/*  f126ff8:	3c10ba00 */ 	lui	$s0,0xba00
/*  f126ffc:	36101402 */ 	ori	$s0,$s0,0x1402
/*  f127000:	252a0008 */ 	addiu	$t2,$t1,0x8
/*  f127004:	afaa0198 */ 	sw	$t2,0x198($sp)
/*  f127008:	ad200004 */ 	sw	$zero,0x4($t1)
/*  f12700c:	ad300000 */ 	sw	$s0,0x0($t1)
/*  f127010:	8fad0198 */ 	lw	$t5,0x198($sp)
/*  f127014:	3c0eba00 */ 	lui	$t6,0xba00
/*  f127018:	35ce0602 */ 	ori	$t6,$t6,0x602
/*  f12701c:	25af0008 */ 	addiu	$t7,$t5,0x8
/*  f127020:	afaf0198 */ 	sw	$t7,0x198($sp)
/*  f127024:	24190040 */ 	li	$t9,0x40
/*  f127028:	adb90004 */ 	sw	$t9,0x4($t5)
/*  f12702c:	adae0000 */ 	sw	$t6,0x0($t5)
/*  f127030:	8fb80198 */ 	lw	$t8,0x198($sp)
/*  f127034:	3c0cba00 */ 	lui	$t4,0xba00
/*  f127038:	358c0402 */ 	ori	$t4,$t4,0x402
/*  f12703c:	27080008 */ 	addiu	$t0,$t8,0x8
/*  f127040:	afa80198 */ 	sw	$t0,0x198($sp)
/*  f127044:	af000004 */ 	sw	$zero,0x4($t8)
/*  f127048:	af0c0000 */ 	sw	$t4,0x0($t8)
/*  f12704c:	8fab0198 */ 	lw	$t3,0x198($sp)
/*  f127050:	3c11b900 */ 	lui	$s1,0xb900
/*  f127054:	3c120050 */ 	lui	$s2,0x50
/*  f127058:	25690008 */ 	addiu	$t1,$t3,0x8
/*  f12705c:	afa90198 */ 	sw	$t1,0x198($sp)
/*  f127060:	365241c8 */ 	ori	$s2,$s2,0x41c8
/*  f127064:	3631031d */ 	ori	$s1,$s1,0x31d
/*  f127068:	ad710000 */ 	sw	$s1,0x0($t3)
/*  f12706c:	ad720004 */ 	sw	$s2,0x4($t3)
/*  f127070:	8faa0198 */ 	lw	$t2,0x198($sp)
/*  f127074:	3c0fba00 */ 	lui	$t7,0xba00
/*  f127078:	35ef1301 */ 	ori	$t7,$t7,0x1301
/*  f12707c:	254d0008 */ 	addiu	$t5,$t2,0x8
/*  f127080:	afad0198 */ 	sw	$t5,0x198($sp)
/*  f127084:	ad400004 */ 	sw	$zero,0x4($t2)
/*  f127088:	ad4f0000 */ 	sw	$t7,0x0($t2)
/*  f12708c:	8fae0198 */ 	lw	$t6,0x198($sp)
/*  f127090:	3c13b900 */ 	lui	$s3,0xb900
/*  f127094:	36730002 */ 	ori	$s3,$s3,0x2
/*  f127098:	25d90008 */ 	addiu	$t9,$t6,0x8
/*  f12709c:	afb90198 */ 	sw	$t9,0x198($sp)
/*  f1270a0:	adc00004 */ 	sw	$zero,0x4($t6)
/*  f1270a4:	add30000 */ 	sw	$s3,0x0($t6)
/*  f1270a8:	8fb80198 */ 	lw	$t8,0x198($sp)
/*  f1270ac:	3c0cba00 */ 	lui	$t4,0xba00
/*  f1270b0:	358c1001 */ 	ori	$t4,$t4,0x1001
/*  f1270b4:	27080008 */ 	addiu	$t0,$t8,0x8
/*  f1270b8:	afa80198 */ 	sw	$t0,0x198($sp)
/*  f1270bc:	af000004 */ 	sw	$zero,0x4($t8)
/*  f1270c0:	af0c0000 */ 	sw	$t4,0x0($t8)
/*  f1270c4:	8fab0198 */ 	lw	$t3,0x198($sp)
/*  f1270c8:	3c14ba00 */ 	lui	$s4,0xba00
/*  f1270cc:	36940903 */ 	ori	$s4,$s4,0x903
/*  f1270d0:	25690008 */ 	addiu	$t1,$t3,0x8
/*  f1270d4:	afa90198 */ 	sw	$t1,0x198($sp)
/*  f1270d8:	240a0c00 */ 	li	$t2,0xc00
/*  f1270dc:	ad6a0004 */ 	sw	$t2,0x4($t3)
/*  f1270e0:	ad740000 */ 	sw	$s4,0x0($t3)
/*  f1270e4:	8fad0198 */ 	lw	$t5,0x198($sp)
/*  f1270e8:	3c0eba00 */ 	lui	$t6,0xba00
/*  f1270ec:	35ce0e02 */ 	ori	$t6,$t6,0xe02
/*  f1270f0:	25af0008 */ 	addiu	$t7,$t5,0x8
/*  f1270f4:	afaf0198 */ 	sw	$t7,0x198($sp)
/*  f1270f8:	ada00004 */ 	sw	$zero,0x4($t5)
/*  f1270fc:	adae0000 */ 	sw	$t6,0x0($t5)
/*  f127100:	8fb90198 */ 	lw	$t9,0x198($sp)
/*  f127104:	3c08ba00 */ 	lui	$t0,0xba00
/*  f127108:	35080c02 */ 	ori	$t0,$t0,0xc02
/*  f12710c:	27380008 */ 	addiu	$t8,$t9,0x8
/*  f127110:	afb80198 */ 	sw	$t8,0x198($sp)
/*  f127114:	240c2000 */ 	li	$t4,0x2000
/*  f127118:	af2c0004 */ 	sw	$t4,0x4($t9)
/*  f12711c:	af280000 */ 	sw	$t0,0x0($t9)
/*  f127120:	8fab0198 */ 	lw	$t3,0x198($sp)
/*  f127124:	3c0dff36 */ 	lui	$t5,0xff36
/*  f127128:	3c0afcff */ 	lui	$t2,0xfcff
/*  f12712c:	25690008 */ 	addiu	$t1,$t3,0x8
/*  f127130:	afa90198 */ 	sw	$t1,0x198($sp)
/*  f127134:	354a9bff */ 	ori	$t2,$t2,0x9bff
/*  f127138:	35adff7f */ 	ori	$t5,$t5,0xff7f
/*  f12713c:	ad6d0004 */ 	sw	$t5,0x4($t3)
/*  f127140:	0c002eee */ 	jal	viGetFovY
/*  f127144:	ad6a0000 */ 	sw	$t2,0x0($t3)
/*  f127148:	8faf0198 */ 	lw	$t7,0x198($sp)
/*  f12714c:	3c17fb00 */ 	lui	$s7,0xfb00
/*  f127150:	3c013f00 */ 	lui	$at,0x3f00
/*  f127154:	25ee0008 */ 	addiu	$t6,$t7,0x8
/*  f127158:	afae0198 */ 	sw	$t6,0x198($sp)
/*  f12715c:	adf70000 */ 	sw	$s7,0x0($t7)
/*  f127160:	c7a801b0 */ 	lwc1	$f8,0x1b0($sp)
/*  f127164:	4481a000 */ 	mtc1	$at,$f20
/*  f127168:	3c01437f */ 	lui	$at,0x437f
/*  f12716c:	46164282 */ 	mul.s	$f10,$f8,$f22
/*  f127170:	44812000 */ 	mtc1	$at,$f4
/*  f127174:	2401ff00 */ 	li	$at,-256
/*  f127178:	240a0001 */ 	li	$t2,0x1
/*  f12717c:	27be0184 */ 	addiu	$s8,$sp,0x184
/*  f127180:	240e0001 */ 	li	$t6,0x1
/*  f127184:	24190001 */ 	li	$t9,0x1
/*  f127188:	46045182 */ 	mul.s	$f6,$f10,$f4
/*  f12718c:	03c02825 */ 	move	$a1,$s8
/*  f127190:	02a02025 */ 	move	$a0,$s5
/*  f127194:	4616a102 */ 	mul.s	$f4,$f20,$f22
/*  f127198:	27a6017c */ 	addiu	$a2,$sp,0x17c
/*  f12719c:	4600320d */ 	trunc.w.s	$f8,$f6
/*  f1271a0:	4604a180 */ 	add.s	$f6,$f20,$f4
/*  f1271a4:	44184000 */ 	mfc1	$t8,$f8
/*  f1271a8:	00000000 */ 	nop
/*  f1271ac:	330800ff */ 	andi	$t0,$t8,0xff
/*  f1271b0:	01016025 */ 	or	$t4,$t0,$at
/*  f1271b4:	adec0004 */ 	sw	$t4,0x4($t7)
/*  f1271b8:	c7aa01a8 */ 	lwc1	$f10,0x1a8($sp)
/*  f1271bc:	3c014270 */ 	lui	$at,0x4270
/*  f1271c0:	44812000 */ 	mtc1	$at,$f4
/*  f1271c4:	46065202 */ 	mul.s	$f8,$f10,$f6
/*  f1271c8:	8ec20000 */ 	lw	$v0,0x0($s6)
/*  f1271cc:	e7ba0184 */ 	swc1	$f26,0x184($sp)
/*  f1271d0:	e7bc0188 */ 	swc1	$f28,0x188($sp)
/*  f1271d4:	240f0001 */ 	li	$t7,0x1
/*  f1271d8:	24180001 */ 	li	$t8,0x1
/*  f1271dc:	46002283 */ 	div.s	$f10,$f4,$f0
/*  f1271e0:	46085182 */ 	mul.s	$f6,$f10,$f8
/*  f1271e4:	4600310d */ 	trunc.w.s	$f4,$f6
/*  f1271e8:	448a3000 */ 	mtc1	$t2,$f6
/*  f1271ec:	44092000 */ 	mfc1	$t1,$f4
/*  f1271f0:	46803620 */ 	cvt.s.w	$f24,$f6
/*  f1271f4:	44895000 */ 	mtc1	$t1,$f10
/*  f1271f8:	00000000 */ 	nop
/*  f1271fc:	46805220 */ 	cvt.s.w	$f8,$f10
/*  f127200:	46144082 */ 	mul.s	$f2,$f8,$f20
/*  f127204:	00000000 */ 	nop
/*  f127208:	46181102 */ 	mul.s	$f4,$f2,$f24
/*  f12720c:	e7a20180 */ 	swc1	$f2,0x180($sp)
/*  f127210:	e7a4017c */ 	swc1	$f4,0x17c($sp)
/*  f127214:	904d004d */ 	lbu	$t5,0x4d($v0)
/*  f127218:	9047004c */ 	lbu	$a3,0x4c($v0)
/*  f12721c:	afb80028 */ 	sw	$t8,0x28($sp)
/*  f127220:	afa00024 */ 	sw	$zero,0x24($sp)
/*  f127224:	afb90020 */ 	sw	$t9,0x20($sp)
/*  f127228:	afae001c */ 	sw	$t6,0x1c($sp)
/*  f12722c:	afaf0018 */ 	sw	$t7,0x18($sp)
/*  f127230:	afa00014 */ 	sw	$zero,0x14($sp)
/*  f127234:	0fc2c99c */ 	jal	func0f0b2150
/*  f127238:	afad0010 */ 	sw	$t5,0x10($sp)
/*  f12723c:	8ec50000 */ 	lw	$a1,0x0($s6)
/*  f127240:	24080002 */ 	li	$t0,0x2
/*  f127244:	240c0001 */ 	li	$t4,0x1
/*  f127248:	afac0014 */ 	sw	$t4,0x14($sp)
/*  f12724c:	afa80010 */ 	sw	$t0,0x10($sp)
/*  f127250:	02a02025 */ 	move	$a0,$s5
/*  f127254:	24060004 */ 	li	$a2,0x4
/*  f127258:	00003825 */ 	move	$a3,$zero
/*  f12725c:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f127260:	0fc2cfb8 */ 	jal	texSelect
/*  f127264:	24a5000c */ 	addiu	$a1,$a1,0xc
/*  f127268:	8fab0198 */ 	lw	$t3,0x198($sp)
/*  f12726c:	3c0fba00 */ 	lui	$t7,0xba00
/*  f127270:	35ef0602 */ 	ori	$t7,$t7,0x602
/*  f127274:	25690008 */ 	addiu	$t1,$t3,0x8
/*  f127278:	afa90198 */ 	sw	$t1,0x198($sp)
/*  f12727c:	ad600004 */ 	sw	$zero,0x4($t3)
/*  f127280:	ad700000 */ 	sw	$s0,0x0($t3)
/*  f127284:	8faa0198 */ 	lw	$t2,0x198($sp)
/*  f127288:	240e0040 */ 	li	$t6,0x40
/*  f12728c:	3c08ba00 */ 	lui	$t0,0xba00
/*  f127290:	254d0008 */ 	addiu	$t5,$t2,0x8
/*  f127294:	afad0198 */ 	sw	$t5,0x198($sp)
/*  f127298:	ad4e0004 */ 	sw	$t6,0x4($t2)
/*  f12729c:	ad4f0000 */ 	sw	$t7,0x0($t2)
/*  f1272a0:	8fb90198 */ 	lw	$t9,0x198($sp)
/*  f1272a4:	35080402 */ 	ori	$t0,$t0,0x402
/*  f1272a8:	3c0dba00 */ 	lui	$t5,0xba00
/*  f1272ac:	27380008 */ 	addiu	$t8,$t9,0x8
/*  f1272b0:	afb80198 */ 	sw	$t8,0x198($sp)
/*  f1272b4:	af200004 */ 	sw	$zero,0x4($t9)
/*  f1272b8:	af280000 */ 	sw	$t0,0x0($t9)
/*  f1272bc:	8fac0198 */ 	lw	$t4,0x198($sp)
/*  f1272c0:	35ad1301 */ 	ori	$t5,$t5,0x1301
/*  f1272c4:	3c08ba00 */ 	lui	$t0,0xba00
/*  f1272c8:	258b0008 */ 	addiu	$t3,$t4,0x8
/*  f1272cc:	afab0198 */ 	sw	$t3,0x198($sp)
/*  f1272d0:	ad920004 */ 	sw	$s2,0x4($t4)
/*  f1272d4:	ad910000 */ 	sw	$s1,0x0($t4)
/*  f1272d8:	8fa90198 */ 	lw	$t1,0x198($sp)
/*  f1272dc:	35081001 */ 	ori	$t0,$t0,0x1001
/*  f1272e0:	3c017f1b */ 	lui	$at,0x7f1b
/*  f1272e4:	252a0008 */ 	addiu	$t2,$t1,0x8
/*  f1272e8:	afaa0198 */ 	sw	$t2,0x198($sp)
/*  f1272ec:	ad200004 */ 	sw	$zero,0x4($t1)
/*  f1272f0:	ad2d0000 */ 	sw	$t5,0x0($t1)
/*  f1272f4:	8faf0198 */ 	lw	$t7,0x198($sp)
/*  f1272f8:	24090c00 */ 	li	$t1,0xc00
/*  f1272fc:	4480b000 */ 	mtc1	$zero,$f22
/*  f127300:	25ee0008 */ 	addiu	$t6,$t7,0x8
/*  f127304:	afae0198 */ 	sw	$t6,0x198($sp)
/*  f127308:	ade00004 */ 	sw	$zero,0x4($t7)
/*  f12730c:	adf30000 */ 	sw	$s3,0x0($t7)
/*  f127310:	8fb90198 */ 	lw	$t9,0x198($sp)
/*  f127314:	3c0fba00 */ 	lui	$t7,0xba00
/*  f127318:	35ef0e02 */ 	ori	$t7,$t7,0xe02
/*  f12731c:	27380008 */ 	addiu	$t8,$t9,0x8
/*  f127320:	afb80198 */ 	sw	$t8,0x198($sp)
/*  f127324:	af200004 */ 	sw	$zero,0x4($t9)
/*  f127328:	af280000 */ 	sw	$t0,0x0($t9)
/*  f12732c:	8fac0198 */ 	lw	$t4,0x198($sp)
/*  f127330:	3c18ba00 */ 	lui	$t8,0xba00
/*  f127334:	37180c02 */ 	ori	$t8,$t8,0xc02
/*  f127338:	258b0008 */ 	addiu	$t3,$t4,0x8
/*  f12733c:	afab0198 */ 	sw	$t3,0x198($sp)
/*  f127340:	ad890004 */ 	sw	$t1,0x4($t4)
/*  f127344:	ad940000 */ 	sw	$s4,0x0($t4)
/*  f127348:	8faa0198 */ 	lw	$t2,0x198($sp)
/*  f12734c:	24082000 */ 	li	$t0,0x2000
/*  f127350:	3c09fcff */ 	lui	$t1,0xfcff
/*  f127354:	254d0008 */ 	addiu	$t5,$t2,0x8
/*  f127358:	afad0198 */ 	sw	$t5,0x198($sp)
/*  f12735c:	ad400004 */ 	sw	$zero,0x4($t2)
/*  f127360:	ad4f0000 */ 	sw	$t7,0x0($t2)
/*  f127364:	8fae0198 */ 	lw	$t6,0x198($sp)
/*  f127368:	3c0aff36 */ 	lui	$t2,0xff36
/*  f12736c:	354aff7f */ 	ori	$t2,$t2,0xff7f
/*  f127370:	25d90008 */ 	addiu	$t9,$t6,0x8
/*  f127374:	afb90198 */ 	sw	$t9,0x198($sp)
/*  f127378:	adc80004 */ 	sw	$t0,0x4($t6)
/*  f12737c:	add80000 */ 	sw	$t8,0x0($t6)
/*  f127380:	8fac0198 */ 	lw	$t4,0x198($sp)
/*  f127384:	35299bff */ 	ori	$t1,$t1,0x9bff
/*  f127388:	27b4014c */ 	addiu	$s4,$sp,0x14c
/*  f12738c:	258b0008 */ 	addiu	$t3,$t4,0x8
/*  f127390:	afab0198 */ 	sw	$t3,0x198($sp)
/*  f127394:	ad8a0004 */ 	sw	$t2,0x4($t4)
/*  f127398:	ad890000 */ 	sw	$t1,0x0($t4)
/*  f12739c:	c43e6400 */ 	lwc1	$f30,0x6400($at)
/*  f1273a0:	3c014316 */ 	lui	$at,0x4316
/*  f1273a4:	e7bc01a0 */ 	swc1	$f28,0x1a0($sp)
/*  f1273a8:	4481e000 */ 	mtc1	$at,$f28
/*  f1273ac:	3c013f80 */ 	lui	$at,0x3f80
/*  f1273b0:	e7ba019c */ 	swc1	$f26,0x19c($sp)
/*  f1273b4:	4481d000 */ 	mtc1	$at,$f26
/*  f1273b8:	3c017f1b */ 	lui	$at,0x7f1b
/*  f1273bc:	e7b80094 */ 	swc1	$f24,0x94($sp)
/*  f1273c0:	c4386404 */ 	lwc1	$f24,0x6404($at)
/*  f1273c4:	8fb301ac */ 	lw	$s3,0x1ac($sp)
/*  f1273c8:	27b1014c */ 	addiu	$s1,$sp,0x14c
/*  f1273cc:	27b20164 */ 	addiu	$s2,$sp,0x164
/*  f1273d0:	27b00134 */ 	addiu	$s0,$sp,0x134
.PF0f1273d4:
/*  f1273d4:	2a61004b */ 	slti	$at,$s3,0x4b
/*  f1273d8:	1020000c */ 	beqz	$at,.PF0f12740c
/*  f1273dc:	266dffb5 */ 	addiu	$t5,$s3,-75
/*  f1273e0:	2a610019 */ 	slti	$at,$s3,0x19
/*  f1273e4:	10200007 */ 	beqz	$at,.PF0f127404
/*  f1273e8:	00000000 */ 	nop
/*  f1273ec:	44935000 */ 	mtc1	$s3,$f10
/*  f1273f0:	00000000 */ 	nop
/*  f1273f4:	46805220 */ 	cvt.s.w	$f8,$f10
/*  f1273f8:	46184082 */ 	mul.s	$f2,$f8,$f24
/*  f1273fc:	10000011 */ 	b	.PF0f127444
/*  f127400:	8e2f0000 */ 	lw	$t7,0x0($s1)
.PF0f127404:
/*  f127404:	1000000e */ 	b	.PF0f127440
/*  f127408:	4600d086 */ 	mov.s	$f2,$f26
.PF0f12740c:
/*  f12740c:	448d3000 */ 	mtc1	$t5,$f6
/*  f127410:	00000000 */ 	nop
/*  f127414:	46803120 */ 	cvt.s.w	$f4,$f6
/*  f127418:	4604e281 */ 	sub.s	$f10,$f28,$f4
/*  f12741c:	461e5202 */ 	mul.s	$f8,$f10,$f30
/*  f127420:	00000000 */ 	nop
/*  f127424:	46144082 */ 	mul.s	$f2,$f8,$f20
/*  f127428:	4616103c */ 	c.lt.s	$f2,$f22
/*  f12742c:	00000000 */ 	nop
/*  f127430:	45000002 */ 	bc1f	.PF0f12743c
/*  f127434:	00000000 */ 	nop
/*  f127438:	4600b086 */ 	mov.s	$f2,$f22
.PF0f12743c:
/*  f12743c:	46141080 */ 	add.s	$f2,$f2,$f20
.PF0f127440:
/*  f127440:	8e2f0000 */ 	lw	$t7,0x0($s1)
.PF0f127444:
/*  f127444:	c7aa0130 */ 	lwc1	$f10,0x130($sp)
/*  f127448:	8fa30198 */ 	lw	$v1,0x198($sp)
/*  f12744c:	448f3000 */ 	mtc1	$t7,$f6
/*  f127450:	c7a4019c */ 	lwc1	$f4,0x19c($sp)
/*  f127454:	8e4e0000 */ 	lw	$t6,0x0($s2)
/*  f127458:	46803020 */ 	cvt.s.w	$f0,$f6
/*  f12745c:	24780008 */ 	addiu	$t8,$v1,0x8
/*  f127460:	c7a601a0 */ 	lwc1	$f6,0x1a0($sp)
/*  f127464:	afb80198 */ 	sw	$t8,0x198($sp)
/*  f127468:	3c014f80 */ 	lui	$at,0x4f80
/*  f12746c:	460a0202 */ 	mul.s	$f8,$f0,$f10
/*  f127470:	c7aa012c */ 	lwc1	$f10,0x12c($sp)
/*  f127474:	ac770000 */ 	sw	$s7,0x0($v1)
/*  f127478:	8e020000 */ 	lw	$v0,0x0($s0)
/*  f12747c:	304800ff */ 	andi	$t0,$v0,0xff
/*  f127480:	46082301 */ 	sub.s	$f12,$f4,$f8
/*  f127484:	460a0102 */ 	mul.s	$f4,$f0,$f10
/*  f127488:	448e4000 */ 	mtc1	$t6,$f8
/*  f12748c:	44885000 */ 	mtc1	$t0,$f10
/*  f127490:	46804420 */ 	cvt.s.w	$f16,$f8
/*  f127494:	c7a801b0 */ 	lwc1	$f8,0x1b0($sp)
/*  f127498:	46043381 */ 	sub.s	$f14,$f6,$f4
/*  f12749c:	468051a0 */ 	cvt.s.w	$f6,$f10
/*  f1274a0:	46024282 */ 	mul.s	$f10,$f8,$f2
/*  f1274a4:	05010004 */ 	bgez	$t0,.PF0f1274b8
/*  f1274a8:	00000000 */ 	nop
/*  f1274ac:	44812000 */ 	mtc1	$at,$f4
/*  f1274b0:	00000000 */ 	nop
/*  f1274b4:	46043180 */ 	add.s	$f6,$f6,$f4
.PF0f1274b8:
/*  f1274b8:	460a3102 */ 	mul.s	$f4,$f6,$f10
/*  f1274bc:	02a02025 */ 	move	$a0,$s5
/*  f1274c0:	00027e02 */ 	srl	$t7,$v0,0x18
/*  f1274c4:	000f7600 */ 	sll	$t6,$t7,0x18
/*  f1274c8:	00024402 */ 	srl	$t0,$v0,0x10
/*  f1274cc:	310c00ff */ 	andi	$t4,$t0,0xff
/*  f1274d0:	00027a02 */ 	srl	$t7,$v0,0x8
/*  f1274d4:	4600220d */ 	trunc.w.s	$f8,$f4
/*  f1274d8:	46148482 */ 	mul.s	$f18,$f16,$f20
/*  f1274dc:	03c02825 */ 	move	$a1,$s8
/*  f1274e0:	27a6017c */ 	addiu	$a2,$sp,0x17c
/*  f1274e4:	440b4000 */ 	mfc1	$t3,$f8
/*  f1274e8:	00000000 */ 	nop
/*  f1274ec:	316900ff */ 	andi	$t1,$t3,0xff
/*  f1274f0:	012ec825 */ 	or	$t9,$t1,$t6
/*  f1274f4:	31e900ff */ 	andi	$t1,$t7,0xff
/*  f1274f8:	000c5c00 */ 	sll	$t3,$t4,0x10
/*  f1274fc:	032b5025 */ 	or	$t2,$t9,$t3
/*  f127500:	00097200 */ 	sll	$t6,$t1,0x8
/*  f127504:	014ec025 */ 	or	$t8,$t2,$t6
/*  f127508:	ac780004 */ 	sw	$t8,0x4($v1)
/*  f12750c:	c7a60094 */ 	lwc1	$f6,0x94($sp)
/*  f127510:	8ec20000 */ 	lw	$v0,0x0($s6)
/*  f127514:	e7ac0184 */ 	swc1	$f12,0x184($sp)
/*  f127518:	46069282 */ 	mul.s	$f10,$f18,$f6
/*  f12751c:	e7ae0188 */ 	swc1	$f14,0x188($sp)
/*  f127520:	e7b20180 */ 	swc1	$f18,0x180($sp)
/*  f127524:	240c0001 */ 	li	$t4,0x1
/*  f127528:	e7aa017c */ 	swc1	$f10,0x17c($sp)
/*  f12752c:	90480011 */ 	lbu	$t0,0x11($v0)
/*  f127530:	90470010 */ 	lbu	$a3,0x10($v0)
/*  f127534:	afac0028 */ 	sw	$t4,0x28($sp)
/*  f127538:	afa00024 */ 	sw	$zero,0x24($sp)
/*  f12753c:	afa00020 */ 	sw	$zero,0x20($sp)
/*  f127540:	afa0001c */ 	sw	$zero,0x1c($sp)
/*  f127544:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f127548:	afa00014 */ 	sw	$zero,0x14($sp)
/*  f12754c:	0fc2c99c */ 	jal	func0f0b2150
/*  f127550:	afa80010 */ 	sw	$t0,0x10($sp)
/*  f127554:	26100004 */ 	addiu	$s0,$s0,0x4
/*  f127558:	26310004 */ 	addiu	$s1,$s1,0x4
/*  f12755c:	1614ff9d */ 	bne	$s0,$s4,.PF0f1273d4
/*  f127560:	26520004 */ 	addiu	$s2,$s2,0x4
/*  f127564:	0c002e73 */ 	jal	viGetViewWidth
/*  f127568:	00000000 */ 	nop
/*  f12756c:	44822000 */ 	mtc1	$v0,$f4
/*  f127570:	3c013f00 */ 	lui	$at,0x3f00
/*  f127574:	4481c000 */ 	mtc1	$at,$f24
/*  f127578:	46802220 */ 	cvt.s.w	$f8,$f4
/*  f12757c:	c7aa019c */ 	lwc1	$f10,0x19c($sp)
/*  f127580:	46184182 */ 	mul.s	$f6,$f8,$f24
/*  f127584:	0c002e77 */ 	jal	viGetViewHeight
/*  f127588:	460a3501 */ 	sub.s	$f20,$f6,$f10
/*  f12758c:	44822000 */ 	mtc1	$v0,$f4
/*  f127590:	c7aa01a0 */ 	lwc1	$f10,0x1a0($sp)
/*  f127594:	46802220 */ 	cvt.s.w	$f8,$f4
/*  f127598:	46184182 */ 	mul.s	$f6,$f8,$f24
/*  f12759c:	460a3001 */ 	sub.s	$f0,$f6,$f10
/*  f1275a0:	4614a102 */ 	mul.s	$f4,$f20,$f20
/*  f1275a4:	00000000 */ 	nop
/*  f1275a8:	46000202 */ 	mul.s	$f8,$f0,$f0
/*  f1275ac:	0c0127b4 */ 	jal	sqrtf
/*  f1275b0:	46082300 */ 	add.s	$f12,$f4,$f8
/*  f1275b4:	3c014220 */ 	lui	$at,0x4220
/*  f1275b8:	44813000 */ 	mtc1	$at,$f6
/*  f1275bc:	3c017f1b */ 	lui	$at,0x7f1b
/*  f1275c0:	c4246408 */ 	lwc1	$f4,0x6408($at)
/*  f1275c4:	46003281 */ 	sub.s	$f10,$f6,$f0
/*  f1275c8:	3c19800a */ 	lui	$t9,0x800a
/*  f1275cc:	c7a601b0 */ 	lwc1	$f6,0x1b0($sp)
/*  f1275d0:	46045302 */ 	mul.s	$f12,$f10,$f4
/*  f1275d4:	4616603c */ 	c.lt.s	$f12,$f22
/*  f1275d8:	00000000 */ 	nop
/*  f1275dc:	45000002 */ 	bc1f	.PF0f1275e8
/*  f1275e0:	00000000 */ 	nop
/*  f1275e4:	4600b306 */ 	mov.s	$f12,$f22
.PF0f1275e8:
/*  f1275e8:	8f39a544 */ 	lw	$t9,-0x5abc($t9)
/*  f1275ec:	3c017f1b */ 	lui	$at,0x7f1b
/*  f1275f0:	c428640c */ 	lwc1	$f8,0x640c($at)
/*  f1275f4:	0333082a */ 	slt	$at,$t9,$s3
/*  f1275f8:	14200002 */ 	bnez	$at,.PF0f127604
/*  f1275fc:	46086300 */ 	add.s	$f12,$f12,$f8
/*  f127600:	4600b306 */ 	mov.s	$f12,$f22
.PF0f127604:
/*  f127604:	460cb03c */ 	c.lt.s	$f22,$f12
/*  f127608:	00000000 */ 	nop
/*  f12760c:	4502000d */ 	bc1fl	.PF0f127644
/*  f127610:	8fad0198 */ 	lw	$t5,0x198($sp)
/*  f127614:	460c3282 */ 	mul.s	$f10,$f6,$f12
/*  f127618:	3c01437f */ 	lui	$at,0x437f
/*  f12761c:	44812000 */ 	mtc1	$at,$f4
/*  f127620:	00000000 */ 	nop
/*  f127624:	46045202 */ 	mul.s	$f8,$f10,$f4
/*  f127628:	4600418d */ 	trunc.w.s	$f6,$f8
/*  f12762c:	44043000 */ 	mfc1	$a0,$f6
/*  f127630:	00000000 */ 	nop
/*  f127634:	00802825 */ 	move	$a1,$a0
/*  f127638:	0fc49f81 */ 	jal	sky0f127334
/*  f12763c:	00803025 */ 	move	$a2,$a0
/*  f127640:	8fad0198 */ 	lw	$t5,0x198($sp)
.PF0f127644:
/*  f127644:	3c09ba00 */ 	lui	$t1,0xba00
/*  f127648:	35290602 */ 	ori	$t1,$t1,0x602
/*  f12764c:	25af0008 */ 	addiu	$t7,$t5,0x8
/*  f127650:	afaf0198 */ 	sw	$t7,0x198($sp)
/*  f127654:	240a0040 */ 	li	$t2,0x40
/*  f127658:	adaa0004 */ 	sw	$t2,0x4($t5)
/*  f12765c:	ada90000 */ 	sw	$t1,0x0($t5)
/*  f127660:	8fae0198 */ 	lw	$t6,0x198($sp)
/*  f127664:	3c08ba00 */ 	lui	$t0,0xba00
/*  f127668:	35080402 */ 	ori	$t0,$t0,0x402
/*  f12766c:	25d80008 */ 	addiu	$t8,$t6,0x8
/*  f127670:	afb80198 */ 	sw	$t8,0x198($sp)
/*  f127674:	240c00c0 */ 	li	$t4,0xc0
/*  f127678:	adcc0004 */ 	sw	$t4,0x4($t6)
/*  f12767c:	adc80000 */ 	sw	$t0,0x0($t6)
/*  f127680:	8fb90198 */ 	lw	$t9,0x198($sp)
/*  f127684:	3c0dba00 */ 	lui	$t5,0xba00
/*  f127688:	35ad1301 */ 	ori	$t5,$t5,0x1301
/*  f12768c:	272b0008 */ 	addiu	$t3,$t9,0x8
/*  f127690:	afab0198 */ 	sw	$t3,0x198($sp)
/*  f127694:	3c0f0008 */ 	lui	$t7,0x8
/*  f127698:	af2f0004 */ 	sw	$t7,0x4($t9)
/*  f12769c:	af2d0000 */ 	sw	$t5,0x0($t9)
/*  f1276a0:	8fa90198 */ 	lw	$t1,0x198($sp)
/*  f1276a4:	3c0eba00 */ 	lui	$t6,0xba00
/*  f1276a8:	35ce1001 */ 	ori	$t6,$t6,0x1001
/*  f1276ac:	252a0008 */ 	addiu	$t2,$t1,0x8
/*  f1276b0:	afaa0198 */ 	sw	$t2,0x198($sp)
/*  f1276b4:	3c180001 */ 	lui	$t8,0x1
/*  f1276b8:	ad380004 */ 	sw	$t8,0x4($t1)
/*  f1276bc:	ad2e0000 */ 	sw	$t6,0x0($t1)
/*  f1276c0:	8fbf008c */ 	lw	$ra,0x8c($sp)
/*  f1276c4:	8fbe0088 */ 	lw	$s8,0x88($sp)
/*  f1276c8:	8fb70084 */ 	lw	$s7,0x84($sp)
/*  f1276cc:	8fb60080 */ 	lw	$s6,0x80($sp)
/*  f1276d0:	8fb5007c */ 	lw	$s5,0x7c($sp)
/*  f1276d4:	8fb40078 */ 	lw	$s4,0x78($sp)
/*  f1276d8:	8fb30074 */ 	lw	$s3,0x74($sp)
/*  f1276dc:	8fb20070 */ 	lw	$s2,0x70($sp)
/*  f1276e0:	8fb1006c */ 	lw	$s1,0x6c($sp)
/*  f1276e4:	8fb00068 */ 	lw	$s0,0x68($sp)
/*  f1276e8:	d7be0060 */ 	ldc1	$f30,0x60($sp)
/*  f1276ec:	d7bc0058 */ 	ldc1	$f28,0x58($sp)
/*  f1276f0:	d7ba0050 */ 	ldc1	$f26,0x50($sp)
/*  f1276f4:	d7b80048 */ 	ldc1	$f24,0x48($sp)
/*  f1276f8:	d7b60040 */ 	ldc1	$f22,0x40($sp)
/*  f1276fc:	d7b40038 */ 	ldc1	$f20,0x38($sp)
/*  f127700:	8fa20198 */ 	lw	$v0,0x198($sp)
/*  f127704:	03e00008 */ 	jr	$ra
/*  f127708:	27bd0198 */ 	addiu	$sp,$sp,0x198
);
#elif VERSION == VERSION_PAL_BETA
GLOBAL_ASM(
glabel sky0f126384
.late_rodata
glabel var7f1b510c
.word 0x3c23d70a
glabel var7f1b5110
.word 0x3bda740e
glabel var7f1b5114
.word 0x3d23d70a
glabel var7f1b5118
.word 0x3c4ccccd
glabel var7f1b511c
.word 0x3dcccccd
.text
/*  f127878:	27bdfe68 */ 	addiu	$sp,$sp,-408
/*  f12787c:	3c0f8008 */ 	lui	$t7,0x8008
/*  f127880:	afbf008c */ 	sw	$ra,0x8c($sp)
/*  f127884:	afbe0088 */ 	sw	$s8,0x88($sp)
/*  f127888:	afb70084 */ 	sw	$s7,0x84($sp)
/*  f12788c:	afb60080 */ 	sw	$s6,0x80($sp)
/*  f127890:	afb5007c */ 	sw	$s5,0x7c($sp)
/*  f127894:	afb40078 */ 	sw	$s4,0x78($sp)
/*  f127898:	afb30074 */ 	sw	$s3,0x74($sp)
/*  f12789c:	afb20070 */ 	sw	$s2,0x70($sp)
/*  f1278a0:	afb1006c */ 	sw	$s1,0x6c($sp)
/*  f1278a4:	afb00068 */ 	sw	$s0,0x68($sp)
/*  f1278a8:	f7be0060 */ 	sdc1	$f30,0x60($sp)
/*  f1278ac:	f7bc0058 */ 	sdc1	$f28,0x58($sp)
/*  f1278b0:	f7ba0050 */ 	sdc1	$f26,0x50($sp)
/*  f1278b4:	f7b80048 */ 	sdc1	$f24,0x48($sp)
/*  f1278b8:	f7b60040 */ 	sdc1	$f22,0x40($sp)
/*  f1278bc:	f7b40038 */ 	sdc1	$f20,0x38($sp)
/*  f1278c0:	afa40198 */ 	sw	$a0,0x198($sp)
/*  f1278c4:	25ef0070 */ 	addiu	$t7,$t7,0x70
/*  f1278c8:	8de10000 */ 	lw	$at,0x0($t7)
/*  f1278cc:	8de80004 */ 	lw	$t0,0x4($t7)
/*  f1278d0:	27ae0164 */ 	addiu	$t6,$sp,0x164
/*  f1278d4:	adc10000 */ 	sw	$at,0x0($t6)
/*  f1278d8:	adc80004 */ 	sw	$t0,0x4($t6)
/*  f1278dc:	8de8000c */ 	lw	$t0,0xc($t7)
/*  f1278e0:	8de10008 */ 	lw	$at,0x8($t7)
/*  f1278e4:	3c0a8008 */ 	lui	$t2,0x8008
/*  f1278e8:	adc8000c */ 	sw	$t0,0xc($t6)
/*  f1278ec:	adc10008 */ 	sw	$at,0x8($t6)
/*  f1278f0:	8de10010 */ 	lw	$at,0x10($t7)
/*  f1278f4:	8de80014 */ 	lw	$t0,0x14($t7)
/*  f1278f8:	254a0088 */ 	addiu	$t2,$t2,0x88
/*  f1278fc:	adc10010 */ 	sw	$at,0x10($t6)
/*  f127900:	adc80014 */ 	sw	$t0,0x14($t6)
/*  f127904:	8d4d0004 */ 	lw	$t5,0x4($t2)
/*  f127908:	8d410000 */ 	lw	$at,0x0($t2)
/*  f12790c:	27a9014c */ 	addiu	$t1,$sp,0x14c
/*  f127910:	ad2d0004 */ 	sw	$t5,0x4($t1)
/*  f127914:	ad210000 */ 	sw	$at,0x0($t1)
/*  f127918:	8d410008 */ 	lw	$at,0x8($t2)
/*  f12791c:	8d4d000c */ 	lw	$t5,0xc($t2)
/*  f127920:	3c188008 */ 	lui	$t8,0x8008
/*  f127924:	ad210008 */ 	sw	$at,0x8($t1)
/*  f127928:	ad2d000c */ 	sw	$t5,0xc($t1)
/*  f12792c:	8d4d0014 */ 	lw	$t5,0x14($t2)
/*  f127930:	8d410010 */ 	lw	$at,0x10($t2)
/*  f127934:	271800a0 */ 	addiu	$t8,$t8,0xa0
/*  f127938:	ad2d0014 */ 	sw	$t5,0x14($t1)
/*  f12793c:	ad210010 */ 	sw	$at,0x10($t1)
/*  f127940:	8f080004 */ 	lw	$t0,0x4($t8)
/*  f127944:	8f010000 */ 	lw	$at,0x0($t8)
/*  f127948:	27b90134 */ 	addiu	$t9,$sp,0x134
/*  f12794c:	af280004 */ 	sw	$t0,0x4($t9)
/*  f127950:	af210000 */ 	sw	$at,0x0($t9)
/*  f127954:	8f010008 */ 	lw	$at,0x8($t8)
/*  f127958:	8f08000c */ 	lw	$t0,0xc($t8)
/*  f12795c:	4487b000 */ 	mtc1	$a3,$f22
/*  f127960:	af210008 */ 	sw	$at,0x8($t9)
/*  f127964:	af28000c */ 	sw	$t0,0xc($t9)
/*  f127968:	8f080014 */ 	lw	$t0,0x14($t8)
/*  f12796c:	8f010010 */ 	lw	$at,0x10($t8)
/*  f127970:	4485d000 */ 	mtc1	$a1,$f26
/*  f127974:	4486e000 */ 	mtc1	$a2,$f28
/*  f127978:	af280014 */ 	sw	$t0,0x14($t9)
/*  f12797c:	0c002e6d */ 	jal	viGetViewWidth
/*  f127980:	af210010 */ 	sw	$at,0x10($t9)
/*  f127984:	44822000 */ 	mtc1	$v0,$f4
/*  f127988:	3c017f1b */ 	lui	$at,0x7f1b
/*  f12798c:	c43470fc */ 	lwc1	$f20,0x70fc($at)
/*  f127990:	468021a0 */ 	cvt.s.w	$f6,$f4
/*  f127994:	3c013f00 */ 	lui	$at,0x3f00
/*  f127998:	4481f000 */ 	mtc1	$at,$f30
/*  f12799c:	00000000 */ 	nop
/*  f1279a0:	461e3202 */ 	mul.s	$f8,$f6,$f30
/*  f1279a4:	4608d281 */ 	sub.s	$f10,$f26,$f8
/*  f1279a8:	46145102 */ 	mul.s	$f4,$f10,$f20
/*  f1279ac:	0c002e71 */ 	jal	viGetViewHeight
/*  f1279b0:	e7a40130 */ 	swc1	$f4,0x130($sp)
/*  f1279b4:	44823000 */ 	mtc1	$v0,$f6
/*  f1279b8:	3c16800b */ 	lui	$s6,0x800b
/*  f1279bc:	26d6fbc0 */ 	addiu	$s6,$s6,-1088
/*  f1279c0:	46803220 */ 	cvt.s.w	$f8,$f6
/*  f1279c4:	8ec50000 */ 	lw	$a1,0x0($s6)
/*  f1279c8:	27b50198 */ 	addiu	$s5,$sp,0x198
/*  f1279cc:	240c0002 */ 	li	$t4,0x2
/*  f1279d0:	240b0001 */ 	li	$t3,0x1
/*  f1279d4:	afab0014 */ 	sw	$t3,0x14($sp)
/*  f1279d8:	461e4282 */ 	mul.s	$f10,$f8,$f30
/*  f1279dc:	afac0010 */ 	sw	$t4,0x10($sp)
/*  f1279e0:	02a02025 */ 	move	$a0,$s5
/*  f1279e4:	24060004 */ 	li	$a2,0x4
/*  f1279e8:	00003825 */ 	move	$a3,$zero
/*  f1279ec:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f1279f0:	24a50048 */ 	addiu	$a1,$a1,0x48
/*  f1279f4:	460ae101 */ 	sub.s	$f4,$f28,$f10
/*  f1279f8:	46142182 */ 	mul.s	$f6,$f4,$f20
/*  f1279fc:	0fc2cf74 */ 	jal	texSelect
/*  f127a00:	e7a6012c */ 	swc1	$f6,0x12c($sp)
/*  f127a04:	8fa90198 */ 	lw	$t1,0x198($sp)
/*  f127a08:	3c10ba00 */ 	lui	$s0,0xba00
/*  f127a0c:	36101402 */ 	ori	$s0,$s0,0x1402
/*  f127a10:	252a0008 */ 	addiu	$t2,$t1,0x8
/*  f127a14:	afaa0198 */ 	sw	$t2,0x198($sp)
/*  f127a18:	ad200004 */ 	sw	$zero,0x4($t1)
/*  f127a1c:	ad300000 */ 	sw	$s0,0x0($t1)
/*  f127a20:	8fad0198 */ 	lw	$t5,0x198($sp)
/*  f127a24:	3c0eba00 */ 	lui	$t6,0xba00
/*  f127a28:	35ce0602 */ 	ori	$t6,$t6,0x602
/*  f127a2c:	25af0008 */ 	addiu	$t7,$t5,0x8
/*  f127a30:	afaf0198 */ 	sw	$t7,0x198($sp)
/*  f127a34:	24190040 */ 	li	$t9,0x40
/*  f127a38:	adb90004 */ 	sw	$t9,0x4($t5)
/*  f127a3c:	adae0000 */ 	sw	$t6,0x0($t5)
/*  f127a40:	8fb80198 */ 	lw	$t8,0x198($sp)
/*  f127a44:	3c0cba00 */ 	lui	$t4,0xba00
/*  f127a48:	358c0402 */ 	ori	$t4,$t4,0x402
/*  f127a4c:	27080008 */ 	addiu	$t0,$t8,0x8
/*  f127a50:	afa80198 */ 	sw	$t0,0x198($sp)
/*  f127a54:	af000004 */ 	sw	$zero,0x4($t8)
/*  f127a58:	af0c0000 */ 	sw	$t4,0x0($t8)
/*  f127a5c:	8fab0198 */ 	lw	$t3,0x198($sp)
/*  f127a60:	3c11b900 */ 	lui	$s1,0xb900
/*  f127a64:	3c120050 */ 	lui	$s2,0x50
/*  f127a68:	25690008 */ 	addiu	$t1,$t3,0x8
/*  f127a6c:	afa90198 */ 	sw	$t1,0x198($sp)
/*  f127a70:	365241c8 */ 	ori	$s2,$s2,0x41c8
/*  f127a74:	3631031d */ 	ori	$s1,$s1,0x31d
/*  f127a78:	ad710000 */ 	sw	$s1,0x0($t3)
/*  f127a7c:	ad720004 */ 	sw	$s2,0x4($t3)
/*  f127a80:	8faa0198 */ 	lw	$t2,0x198($sp)
/*  f127a84:	3c0fba00 */ 	lui	$t7,0xba00
/*  f127a88:	35ef1301 */ 	ori	$t7,$t7,0x1301
/*  f127a8c:	254d0008 */ 	addiu	$t5,$t2,0x8
/*  f127a90:	afad0198 */ 	sw	$t5,0x198($sp)
/*  f127a94:	ad400004 */ 	sw	$zero,0x4($t2)
/*  f127a98:	ad4f0000 */ 	sw	$t7,0x0($t2)
/*  f127a9c:	8fae0198 */ 	lw	$t6,0x198($sp)
/*  f127aa0:	3c13b900 */ 	lui	$s3,0xb900
/*  f127aa4:	36730002 */ 	ori	$s3,$s3,0x2
/*  f127aa8:	25d90008 */ 	addiu	$t9,$t6,0x8
/*  f127aac:	afb90198 */ 	sw	$t9,0x198($sp)
/*  f127ab0:	adc00004 */ 	sw	$zero,0x4($t6)
/*  f127ab4:	add30000 */ 	sw	$s3,0x0($t6)
/*  f127ab8:	8fb80198 */ 	lw	$t8,0x198($sp)
/*  f127abc:	3c0cba00 */ 	lui	$t4,0xba00
/*  f127ac0:	358c1001 */ 	ori	$t4,$t4,0x1001
/*  f127ac4:	27080008 */ 	addiu	$t0,$t8,0x8
/*  f127ac8:	afa80198 */ 	sw	$t0,0x198($sp)
/*  f127acc:	af000004 */ 	sw	$zero,0x4($t8)
/*  f127ad0:	af0c0000 */ 	sw	$t4,0x0($t8)
/*  f127ad4:	8fab0198 */ 	lw	$t3,0x198($sp)
/*  f127ad8:	3c14ba00 */ 	lui	$s4,0xba00
/*  f127adc:	36940903 */ 	ori	$s4,$s4,0x903
/*  f127ae0:	25690008 */ 	addiu	$t1,$t3,0x8
/*  f127ae4:	afa90198 */ 	sw	$t1,0x198($sp)
/*  f127ae8:	240a0c00 */ 	li	$t2,0xc00
/*  f127aec:	ad6a0004 */ 	sw	$t2,0x4($t3)
/*  f127af0:	ad740000 */ 	sw	$s4,0x0($t3)
/*  f127af4:	8fad0198 */ 	lw	$t5,0x198($sp)
/*  f127af8:	3c0eba00 */ 	lui	$t6,0xba00
/*  f127afc:	35ce0e02 */ 	ori	$t6,$t6,0xe02
/*  f127b00:	25af0008 */ 	addiu	$t7,$t5,0x8
/*  f127b04:	afaf0198 */ 	sw	$t7,0x198($sp)
/*  f127b08:	ada00004 */ 	sw	$zero,0x4($t5)
/*  f127b0c:	adae0000 */ 	sw	$t6,0x0($t5)
/*  f127b10:	8fb90198 */ 	lw	$t9,0x198($sp)
/*  f127b14:	3c08ba00 */ 	lui	$t0,0xba00
/*  f127b18:	35080c02 */ 	ori	$t0,$t0,0xc02
/*  f127b1c:	27380008 */ 	addiu	$t8,$t9,0x8
/*  f127b20:	afb80198 */ 	sw	$t8,0x198($sp)
/*  f127b24:	240c2000 */ 	li	$t4,0x2000
/*  f127b28:	af2c0004 */ 	sw	$t4,0x4($t9)
/*  f127b2c:	af280000 */ 	sw	$t0,0x0($t9)
/*  f127b30:	8fab0198 */ 	lw	$t3,0x198($sp)
/*  f127b34:	3c0dff36 */ 	lui	$t5,0xff36
/*  f127b38:	3c0afcff */ 	lui	$t2,0xfcff
/*  f127b3c:	25690008 */ 	addiu	$t1,$t3,0x8
/*  f127b40:	afa90198 */ 	sw	$t1,0x198($sp)
/*  f127b44:	354a9bff */ 	ori	$t2,$t2,0x9bff
/*  f127b48:	35adff7f */ 	ori	$t5,$t5,0xff7f
/*  f127b4c:	ad6d0004 */ 	sw	$t5,0x4($t3)
/*  f127b50:	0c002ee8 */ 	jal	viGetFovY
/*  f127b54:	ad6a0000 */ 	sw	$t2,0x0($t3)
/*  f127b58:	8faf0198 */ 	lw	$t7,0x198($sp)
/*  f127b5c:	3c17fb00 */ 	lui	$s7,0xfb00
/*  f127b60:	3c013f00 */ 	lui	$at,0x3f00
/*  f127b64:	25ee0008 */ 	addiu	$t6,$t7,0x8
/*  f127b68:	afae0198 */ 	sw	$t6,0x198($sp)
/*  f127b6c:	adf70000 */ 	sw	$s7,0x0($t7)
/*  f127b70:	c7a801b0 */ 	lwc1	$f8,0x1b0($sp)
/*  f127b74:	4481a000 */ 	mtc1	$at,$f20
/*  f127b78:	3c01437f */ 	lui	$at,0x437f
/*  f127b7c:	46164282 */ 	mul.s	$f10,$f8,$f22
/*  f127b80:	44812000 */ 	mtc1	$at,$f4
/*  f127b84:	2401ff00 */ 	li	$at,-256
/*  f127b88:	240a0001 */ 	li	$t2,0x1
/*  f127b8c:	27be0184 */ 	addiu	$s8,$sp,0x184
/*  f127b90:	240e0001 */ 	li	$t6,0x1
/*  f127b94:	24190001 */ 	li	$t9,0x1
/*  f127b98:	46045182 */ 	mul.s	$f6,$f10,$f4
/*  f127b9c:	03c02825 */ 	move	$a1,$s8
/*  f127ba0:	02a02025 */ 	move	$a0,$s5
/*  f127ba4:	4616a102 */ 	mul.s	$f4,$f20,$f22
/*  f127ba8:	27a6017c */ 	addiu	$a2,$sp,0x17c
/*  f127bac:	4600320d */ 	trunc.w.s	$f8,$f6
/*  f127bb0:	4604a180 */ 	add.s	$f6,$f20,$f4
/*  f127bb4:	44184000 */ 	mfc1	$t8,$f8
/*  f127bb8:	00000000 */ 	nop
/*  f127bbc:	330800ff */ 	andi	$t0,$t8,0xff
/*  f127bc0:	01016025 */ 	or	$t4,$t0,$at
/*  f127bc4:	adec0004 */ 	sw	$t4,0x4($t7)
/*  f127bc8:	c7aa01a8 */ 	lwc1	$f10,0x1a8($sp)
/*  f127bcc:	3c014270 */ 	lui	$at,0x4270
/*  f127bd0:	44812000 */ 	mtc1	$at,$f4
/*  f127bd4:	46065202 */ 	mul.s	$f8,$f10,$f6
/*  f127bd8:	8ec20000 */ 	lw	$v0,0x0($s6)
/*  f127bdc:	e7ba0184 */ 	swc1	$f26,0x184($sp)
/*  f127be0:	e7bc0188 */ 	swc1	$f28,0x188($sp)
/*  f127be4:	240f0001 */ 	li	$t7,0x1
/*  f127be8:	24180001 */ 	li	$t8,0x1
/*  f127bec:	46002283 */ 	div.s	$f10,$f4,$f0
/*  f127bf0:	46085182 */ 	mul.s	$f6,$f10,$f8
/*  f127bf4:	4600310d */ 	trunc.w.s	$f4,$f6
/*  f127bf8:	448a3000 */ 	mtc1	$t2,$f6
/*  f127bfc:	44092000 */ 	mfc1	$t1,$f4
/*  f127c00:	46803620 */ 	cvt.s.w	$f24,$f6
/*  f127c04:	44895000 */ 	mtc1	$t1,$f10
/*  f127c08:	00000000 */ 	nop
/*  f127c0c:	46805220 */ 	cvt.s.w	$f8,$f10
/*  f127c10:	46144082 */ 	mul.s	$f2,$f8,$f20
/*  f127c14:	00000000 */ 	nop
/*  f127c18:	46181102 */ 	mul.s	$f4,$f2,$f24
/*  f127c1c:	e7a20180 */ 	swc1	$f2,0x180($sp)
/*  f127c20:	e7a4017c */ 	swc1	$f4,0x17c($sp)
/*  f127c24:	904d004d */ 	lbu	$t5,0x4d($v0)
/*  f127c28:	9047004c */ 	lbu	$a3,0x4c($v0)
/*  f127c2c:	afb80028 */ 	sw	$t8,0x28($sp)
/*  f127c30:	afa00024 */ 	sw	$zero,0x24($sp)
/*  f127c34:	afb90020 */ 	sw	$t9,0x20($sp)
/*  f127c38:	afae001c */ 	sw	$t6,0x1c($sp)
/*  f127c3c:	afaf0018 */ 	sw	$t7,0x18($sp)
/*  f127c40:	afa00014 */ 	sw	$zero,0x14($sp)
/*  f127c44:	0fc2c958 */ 	jal	func0f0b2150
/*  f127c48:	afad0010 */ 	sw	$t5,0x10($sp)
/*  f127c4c:	8ec50000 */ 	lw	$a1,0x0($s6)
/*  f127c50:	24080002 */ 	li	$t0,0x2
/*  f127c54:	240c0001 */ 	li	$t4,0x1
/*  f127c58:	afac0014 */ 	sw	$t4,0x14($sp)
/*  f127c5c:	afa80010 */ 	sw	$t0,0x10($sp)
/*  f127c60:	02a02025 */ 	move	$a0,$s5
/*  f127c64:	24060004 */ 	li	$a2,0x4
/*  f127c68:	00003825 */ 	move	$a3,$zero
/*  f127c6c:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f127c70:	0fc2cf74 */ 	jal	texSelect
/*  f127c74:	24a5000c */ 	addiu	$a1,$a1,0xc
/*  f127c78:	8fab0198 */ 	lw	$t3,0x198($sp)
/*  f127c7c:	3c0fba00 */ 	lui	$t7,0xba00
/*  f127c80:	35ef0602 */ 	ori	$t7,$t7,0x602
/*  f127c84:	25690008 */ 	addiu	$t1,$t3,0x8
/*  f127c88:	afa90198 */ 	sw	$t1,0x198($sp)
/*  f127c8c:	ad600004 */ 	sw	$zero,0x4($t3)
/*  f127c90:	ad700000 */ 	sw	$s0,0x0($t3)
/*  f127c94:	8faa0198 */ 	lw	$t2,0x198($sp)
/*  f127c98:	240e0040 */ 	li	$t6,0x40
/*  f127c9c:	3c08ba00 */ 	lui	$t0,0xba00
/*  f127ca0:	254d0008 */ 	addiu	$t5,$t2,0x8
/*  f127ca4:	afad0198 */ 	sw	$t5,0x198($sp)
/*  f127ca8:	ad4e0004 */ 	sw	$t6,0x4($t2)
/*  f127cac:	ad4f0000 */ 	sw	$t7,0x0($t2)
/*  f127cb0:	8fb90198 */ 	lw	$t9,0x198($sp)
/*  f127cb4:	35080402 */ 	ori	$t0,$t0,0x402
/*  f127cb8:	3c0dba00 */ 	lui	$t5,0xba00
/*  f127cbc:	27380008 */ 	addiu	$t8,$t9,0x8
/*  f127cc0:	afb80198 */ 	sw	$t8,0x198($sp)
/*  f127cc4:	af200004 */ 	sw	$zero,0x4($t9)
/*  f127cc8:	af280000 */ 	sw	$t0,0x0($t9)
/*  f127ccc:	8fac0198 */ 	lw	$t4,0x198($sp)
/*  f127cd0:	35ad1301 */ 	ori	$t5,$t5,0x1301
/*  f127cd4:	3c08ba00 */ 	lui	$t0,0xba00
/*  f127cd8:	258b0008 */ 	addiu	$t3,$t4,0x8
/*  f127cdc:	afab0198 */ 	sw	$t3,0x198($sp)
/*  f127ce0:	ad920004 */ 	sw	$s2,0x4($t4)
/*  f127ce4:	ad910000 */ 	sw	$s1,0x0($t4)
/*  f127ce8:	8fa90198 */ 	lw	$t1,0x198($sp)
/*  f127cec:	35081001 */ 	ori	$t0,$t0,0x1001
/*  f127cf0:	3c017f1b */ 	lui	$at,0x7f1b
/*  f127cf4:	252a0008 */ 	addiu	$t2,$t1,0x8
/*  f127cf8:	afaa0198 */ 	sw	$t2,0x198($sp)
/*  f127cfc:	ad200004 */ 	sw	$zero,0x4($t1)
/*  f127d00:	ad2d0000 */ 	sw	$t5,0x0($t1)
/*  f127d04:	8faf0198 */ 	lw	$t7,0x198($sp)
/*  f127d08:	24090c00 */ 	li	$t1,0xc00
/*  f127d0c:	4480b000 */ 	mtc1	$zero,$f22
/*  f127d10:	25ee0008 */ 	addiu	$t6,$t7,0x8
/*  f127d14:	afae0198 */ 	sw	$t6,0x198($sp)
/*  f127d18:	ade00004 */ 	sw	$zero,0x4($t7)
/*  f127d1c:	adf30000 */ 	sw	$s3,0x0($t7)
/*  f127d20:	8fb90198 */ 	lw	$t9,0x198($sp)
/*  f127d24:	3c0fba00 */ 	lui	$t7,0xba00
/*  f127d28:	35ef0e02 */ 	ori	$t7,$t7,0xe02
/*  f127d2c:	27380008 */ 	addiu	$t8,$t9,0x8
/*  f127d30:	afb80198 */ 	sw	$t8,0x198($sp)
/*  f127d34:	af200004 */ 	sw	$zero,0x4($t9)
/*  f127d38:	af280000 */ 	sw	$t0,0x0($t9)
/*  f127d3c:	8fac0198 */ 	lw	$t4,0x198($sp)
/*  f127d40:	3c18ba00 */ 	lui	$t8,0xba00
/*  f127d44:	37180c02 */ 	ori	$t8,$t8,0xc02
/*  f127d48:	258b0008 */ 	addiu	$t3,$t4,0x8
/*  f127d4c:	afab0198 */ 	sw	$t3,0x198($sp)
/*  f127d50:	ad890004 */ 	sw	$t1,0x4($t4)
/*  f127d54:	ad940000 */ 	sw	$s4,0x0($t4)
/*  f127d58:	8faa0198 */ 	lw	$t2,0x198($sp)
/*  f127d5c:	24082000 */ 	li	$t0,0x2000
/*  f127d60:	3c09fcff */ 	lui	$t1,0xfcff
/*  f127d64:	254d0008 */ 	addiu	$t5,$t2,0x8
/*  f127d68:	afad0198 */ 	sw	$t5,0x198($sp)
/*  f127d6c:	ad400004 */ 	sw	$zero,0x4($t2)
/*  f127d70:	ad4f0000 */ 	sw	$t7,0x0($t2)
/*  f127d74:	8fae0198 */ 	lw	$t6,0x198($sp)
/*  f127d78:	3c0aff36 */ 	lui	$t2,0xff36
/*  f127d7c:	354aff7f */ 	ori	$t2,$t2,0xff7f
/*  f127d80:	25d90008 */ 	addiu	$t9,$t6,0x8
/*  f127d84:	afb90198 */ 	sw	$t9,0x198($sp)
/*  f127d88:	adc80004 */ 	sw	$t0,0x4($t6)
/*  f127d8c:	add80000 */ 	sw	$t8,0x0($t6)
/*  f127d90:	8fac0198 */ 	lw	$t4,0x198($sp)
/*  f127d94:	35299bff */ 	ori	$t1,$t1,0x9bff
/*  f127d98:	27b4014c */ 	addiu	$s4,$sp,0x14c
/*  f127d9c:	258b0008 */ 	addiu	$t3,$t4,0x8
/*  f127da0:	afab0198 */ 	sw	$t3,0x198($sp)
/*  f127da4:	ad8a0004 */ 	sw	$t2,0x4($t4)
/*  f127da8:	ad890000 */ 	sw	$t1,0x0($t4)
/*  f127dac:	c43e7100 */ 	lwc1	$f30,0x7100($at)
/*  f127db0:	3c014316 */ 	lui	$at,0x4316
/*  f127db4:	e7bc01a0 */ 	swc1	$f28,0x1a0($sp)
/*  f127db8:	4481e000 */ 	mtc1	$at,$f28
/*  f127dbc:	3c013f80 */ 	lui	$at,0x3f80
/*  f127dc0:	e7ba019c */ 	swc1	$f26,0x19c($sp)
/*  f127dc4:	4481d000 */ 	mtc1	$at,$f26
/*  f127dc8:	3c017f1b */ 	lui	$at,0x7f1b
/*  f127dcc:	e7b80094 */ 	swc1	$f24,0x94($sp)
/*  f127dd0:	c4387104 */ 	lwc1	$f24,0x7104($at)
/*  f127dd4:	8fb301ac */ 	lw	$s3,0x1ac($sp)
/*  f127dd8:	27b1014c */ 	addiu	$s1,$sp,0x14c
/*  f127ddc:	27b20164 */ 	addiu	$s2,$sp,0x164
/*  f127de0:	27b00134 */ 	addiu	$s0,$sp,0x134
.PB0f127de4:
/*  f127de4:	2a61004b */ 	slti	$at,$s3,0x4b
/*  f127de8:	1020000c */ 	beqz	$at,.PB0f127e1c
/*  f127dec:	266dffb5 */ 	addiu	$t5,$s3,-75
/*  f127df0:	2a610019 */ 	slti	$at,$s3,0x19
/*  f127df4:	10200007 */ 	beqz	$at,.PB0f127e14
/*  f127df8:	00000000 */ 	nop
/*  f127dfc:	44935000 */ 	mtc1	$s3,$f10
/*  f127e00:	00000000 */ 	nop
/*  f127e04:	46805220 */ 	cvt.s.w	$f8,$f10
/*  f127e08:	46184082 */ 	mul.s	$f2,$f8,$f24
/*  f127e0c:	10000011 */ 	b	.PB0f127e54
/*  f127e10:	8e2f0000 */ 	lw	$t7,0x0($s1)
.PB0f127e14:
/*  f127e14:	1000000e */ 	b	.PB0f127e50
/*  f127e18:	4600d086 */ 	mov.s	$f2,$f26
.PB0f127e1c:
/*  f127e1c:	448d3000 */ 	mtc1	$t5,$f6
/*  f127e20:	00000000 */ 	nop
/*  f127e24:	46803120 */ 	cvt.s.w	$f4,$f6
/*  f127e28:	4604e281 */ 	sub.s	$f10,$f28,$f4
/*  f127e2c:	461e5202 */ 	mul.s	$f8,$f10,$f30
/*  f127e30:	00000000 */ 	nop
/*  f127e34:	46144082 */ 	mul.s	$f2,$f8,$f20
/*  f127e38:	4616103c */ 	c.lt.s	$f2,$f22
/*  f127e3c:	00000000 */ 	nop
/*  f127e40:	45000002 */ 	bc1f	.PB0f127e4c
/*  f127e44:	00000000 */ 	nop
/*  f127e48:	4600b086 */ 	mov.s	$f2,$f22
.PB0f127e4c:
/*  f127e4c:	46141080 */ 	add.s	$f2,$f2,$f20
.PB0f127e50:
/*  f127e50:	8e2f0000 */ 	lw	$t7,0x0($s1)
.PB0f127e54:
/*  f127e54:	c7aa0130 */ 	lwc1	$f10,0x130($sp)
/*  f127e58:	8fa30198 */ 	lw	$v1,0x198($sp)
/*  f127e5c:	448f3000 */ 	mtc1	$t7,$f6
/*  f127e60:	c7a4019c */ 	lwc1	$f4,0x19c($sp)
/*  f127e64:	8e4e0000 */ 	lw	$t6,0x0($s2)
/*  f127e68:	46803020 */ 	cvt.s.w	$f0,$f6
/*  f127e6c:	24780008 */ 	addiu	$t8,$v1,0x8
/*  f127e70:	c7a601a0 */ 	lwc1	$f6,0x1a0($sp)
/*  f127e74:	afb80198 */ 	sw	$t8,0x198($sp)
/*  f127e78:	3c014f80 */ 	lui	$at,0x4f80
/*  f127e7c:	460a0202 */ 	mul.s	$f8,$f0,$f10
/*  f127e80:	c7aa012c */ 	lwc1	$f10,0x12c($sp)
/*  f127e84:	ac770000 */ 	sw	$s7,0x0($v1)
/*  f127e88:	8e020000 */ 	lw	$v0,0x0($s0)
/*  f127e8c:	304800ff */ 	andi	$t0,$v0,0xff
/*  f127e90:	46082301 */ 	sub.s	$f12,$f4,$f8
/*  f127e94:	460a0102 */ 	mul.s	$f4,$f0,$f10
/*  f127e98:	448e4000 */ 	mtc1	$t6,$f8
/*  f127e9c:	44885000 */ 	mtc1	$t0,$f10
/*  f127ea0:	46804420 */ 	cvt.s.w	$f16,$f8
/*  f127ea4:	c7a801b0 */ 	lwc1	$f8,0x1b0($sp)
/*  f127ea8:	46043381 */ 	sub.s	$f14,$f6,$f4
/*  f127eac:	468051a0 */ 	cvt.s.w	$f6,$f10
/*  f127eb0:	46024282 */ 	mul.s	$f10,$f8,$f2
/*  f127eb4:	05010004 */ 	bgez	$t0,.PB0f127ec8
/*  f127eb8:	00000000 */ 	nop
/*  f127ebc:	44812000 */ 	mtc1	$at,$f4
/*  f127ec0:	00000000 */ 	nop
/*  f127ec4:	46043180 */ 	add.s	$f6,$f6,$f4
.PB0f127ec8:
/*  f127ec8:	460a3102 */ 	mul.s	$f4,$f6,$f10
/*  f127ecc:	02a02025 */ 	move	$a0,$s5
/*  f127ed0:	00027e02 */ 	srl	$t7,$v0,0x18
/*  f127ed4:	000f7600 */ 	sll	$t6,$t7,0x18
/*  f127ed8:	00024402 */ 	srl	$t0,$v0,0x10
/*  f127edc:	310c00ff */ 	andi	$t4,$t0,0xff
/*  f127ee0:	00027a02 */ 	srl	$t7,$v0,0x8
/*  f127ee4:	4600220d */ 	trunc.w.s	$f8,$f4
/*  f127ee8:	46148482 */ 	mul.s	$f18,$f16,$f20
/*  f127eec:	03c02825 */ 	move	$a1,$s8
/*  f127ef0:	27a6017c */ 	addiu	$a2,$sp,0x17c
/*  f127ef4:	440b4000 */ 	mfc1	$t3,$f8
/*  f127ef8:	00000000 */ 	nop
/*  f127efc:	316900ff */ 	andi	$t1,$t3,0xff
/*  f127f00:	012ec825 */ 	or	$t9,$t1,$t6
/*  f127f04:	31e900ff */ 	andi	$t1,$t7,0xff
/*  f127f08:	000c5c00 */ 	sll	$t3,$t4,0x10
/*  f127f0c:	032b5025 */ 	or	$t2,$t9,$t3
/*  f127f10:	00097200 */ 	sll	$t6,$t1,0x8
/*  f127f14:	014ec025 */ 	or	$t8,$t2,$t6
/*  f127f18:	ac780004 */ 	sw	$t8,0x4($v1)
/*  f127f1c:	c7a60094 */ 	lwc1	$f6,0x94($sp)
/*  f127f20:	8ec20000 */ 	lw	$v0,0x0($s6)
/*  f127f24:	e7ac0184 */ 	swc1	$f12,0x184($sp)
/*  f127f28:	46069282 */ 	mul.s	$f10,$f18,$f6
/*  f127f2c:	e7ae0188 */ 	swc1	$f14,0x188($sp)
/*  f127f30:	e7b20180 */ 	swc1	$f18,0x180($sp)
/*  f127f34:	240c0001 */ 	li	$t4,0x1
/*  f127f38:	e7aa017c */ 	swc1	$f10,0x17c($sp)
/*  f127f3c:	90480011 */ 	lbu	$t0,0x11($v0)
/*  f127f40:	90470010 */ 	lbu	$a3,0x10($v0)
/*  f127f44:	afac0028 */ 	sw	$t4,0x28($sp)
/*  f127f48:	afa00024 */ 	sw	$zero,0x24($sp)
/*  f127f4c:	afa00020 */ 	sw	$zero,0x20($sp)
/*  f127f50:	afa0001c */ 	sw	$zero,0x1c($sp)
/*  f127f54:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f127f58:	afa00014 */ 	sw	$zero,0x14($sp)
/*  f127f5c:	0fc2c958 */ 	jal	func0f0b2150
/*  f127f60:	afa80010 */ 	sw	$t0,0x10($sp)
/*  f127f64:	26100004 */ 	addiu	$s0,$s0,0x4
/*  f127f68:	26310004 */ 	addiu	$s1,$s1,0x4
/*  f127f6c:	1614ff9d */ 	bne	$s0,$s4,.PB0f127de4
/*  f127f70:	26520004 */ 	addiu	$s2,$s2,0x4
/*  f127f74:	0c002e6d */ 	jal	viGetViewWidth
/*  f127f78:	00000000 */ 	nop
/*  f127f7c:	44822000 */ 	mtc1	$v0,$f4
/*  f127f80:	3c013f00 */ 	lui	$at,0x3f00
/*  f127f84:	4481c000 */ 	mtc1	$at,$f24
/*  f127f88:	46802220 */ 	cvt.s.w	$f8,$f4
/*  f127f8c:	c7aa019c */ 	lwc1	$f10,0x19c($sp)
/*  f127f90:	46184182 */ 	mul.s	$f6,$f8,$f24
/*  f127f94:	0c002e71 */ 	jal	viGetViewHeight
/*  f127f98:	460a3501 */ 	sub.s	$f20,$f6,$f10
/*  f127f9c:	44822000 */ 	mtc1	$v0,$f4
/*  f127fa0:	c7aa01a0 */ 	lwc1	$f10,0x1a0($sp)
/*  f127fa4:	46802220 */ 	cvt.s.w	$f8,$f4
/*  f127fa8:	46184182 */ 	mul.s	$f6,$f8,$f24
/*  f127fac:	460a3001 */ 	sub.s	$f0,$f6,$f10
/*  f127fb0:	4614a102 */ 	mul.s	$f4,$f20,$f20
/*  f127fb4:	00000000 */ 	nop
/*  f127fb8:	46000202 */ 	mul.s	$f8,$f0,$f0
/*  f127fbc:	0c012ae4 */ 	jal	sqrtf
/*  f127fc0:	46082300 */ 	add.s	$f12,$f4,$f8
/*  f127fc4:	3c014220 */ 	lui	$at,0x4220
/*  f127fc8:	44813000 */ 	mtc1	$at,$f6
/*  f127fcc:	3c017f1b */ 	lui	$at,0x7f1b
/*  f127fd0:	c4247108 */ 	lwc1	$f4,0x7108($at)
/*  f127fd4:	46003281 */ 	sub.s	$f10,$f6,$f0
/*  f127fd8:	3c19800a */ 	lui	$t9,0x800a
/*  f127fdc:	c7a601b0 */ 	lwc1	$f6,0x1b0($sp)
/*  f127fe0:	46045302 */ 	mul.s	$f12,$f10,$f4
/*  f127fe4:	4616603c */ 	c.lt.s	$f12,$f22
/*  f127fe8:	00000000 */ 	nop
/*  f127fec:	45000002 */ 	bc1f	.PB0f127ff8
/*  f127ff0:	00000000 */ 	nop
/*  f127ff4:	4600b306 */ 	mov.s	$f12,$f22
.PB0f127ff8:
/*  f127ff8:	8f39e504 */ 	lw	$t9,-0x1afc($t9)
/*  f127ffc:	3c017f1b */ 	lui	$at,0x7f1b
/*  f128000:	c428710c */ 	lwc1	$f8,0x710c($at)
/*  f128004:	0333082a */ 	slt	$at,$t9,$s3
/*  f128008:	14200002 */ 	bnez	$at,.PB0f128014
/*  f12800c:	46086300 */ 	add.s	$f12,$f12,$f8
/*  f128010:	4600b306 */ 	mov.s	$f12,$f22
.PB0f128014:
/*  f128014:	460cb03c */ 	c.lt.s	$f22,$f12
/*  f128018:	00000000 */ 	nop
/*  f12801c:	4502000d */ 	bc1fl	.PB0f128054
/*  f128020:	8fad0198 */ 	lw	$t5,0x198($sp)
/*  f128024:	460c3282 */ 	mul.s	$f10,$f6,$f12
/*  f128028:	3c01437f */ 	lui	$at,0x437f
/*  f12802c:	44812000 */ 	mtc1	$at,$f4
/*  f128030:	00000000 */ 	nop
/*  f128034:	46045202 */ 	mul.s	$f8,$f10,$f4
/*  f128038:	4600418d */ 	trunc.w.s	$f6,$f8
/*  f12803c:	44043000 */ 	mfc1	$a0,$f6
/*  f128040:	00000000 */ 	nop
/*  f128044:	00802825 */ 	move	$a1,$a0
/*  f128048:	0fc4a205 */ 	jal	sky0f127334
/*  f12804c:	00803025 */ 	move	$a2,$a0
/*  f128050:	8fad0198 */ 	lw	$t5,0x198($sp)
.PB0f128054:
/*  f128054:	3c09ba00 */ 	lui	$t1,0xba00
/*  f128058:	35290602 */ 	ori	$t1,$t1,0x602
/*  f12805c:	25af0008 */ 	addiu	$t7,$t5,0x8
/*  f128060:	afaf0198 */ 	sw	$t7,0x198($sp)
/*  f128064:	240a0040 */ 	li	$t2,0x40
/*  f128068:	adaa0004 */ 	sw	$t2,0x4($t5)
/*  f12806c:	ada90000 */ 	sw	$t1,0x0($t5)
/*  f128070:	8fae0198 */ 	lw	$t6,0x198($sp)
/*  f128074:	3c08ba00 */ 	lui	$t0,0xba00
/*  f128078:	35080402 */ 	ori	$t0,$t0,0x402
/*  f12807c:	25d80008 */ 	addiu	$t8,$t6,0x8
/*  f128080:	afb80198 */ 	sw	$t8,0x198($sp)
/*  f128084:	240c00c0 */ 	li	$t4,0xc0
/*  f128088:	adcc0004 */ 	sw	$t4,0x4($t6)
/*  f12808c:	adc80000 */ 	sw	$t0,0x0($t6)
/*  f128090:	8fb90198 */ 	lw	$t9,0x198($sp)
/*  f128094:	3c0dba00 */ 	lui	$t5,0xba00
/*  f128098:	35ad1301 */ 	ori	$t5,$t5,0x1301
/*  f12809c:	272b0008 */ 	addiu	$t3,$t9,0x8
/*  f1280a0:	afab0198 */ 	sw	$t3,0x198($sp)
/*  f1280a4:	3c0f0008 */ 	lui	$t7,0x8
/*  f1280a8:	af2f0004 */ 	sw	$t7,0x4($t9)
/*  f1280ac:	af2d0000 */ 	sw	$t5,0x0($t9)
/*  f1280b0:	8fa90198 */ 	lw	$t1,0x198($sp)
/*  f1280b4:	3c0eba00 */ 	lui	$t6,0xba00
/*  f1280b8:	35ce1001 */ 	ori	$t6,$t6,0x1001
/*  f1280bc:	252a0008 */ 	addiu	$t2,$t1,0x8
/*  f1280c0:	afaa0198 */ 	sw	$t2,0x198($sp)
/*  f1280c4:	3c180001 */ 	lui	$t8,0x1
/*  f1280c8:	ad380004 */ 	sw	$t8,0x4($t1)
/*  f1280cc:	ad2e0000 */ 	sw	$t6,0x0($t1)
/*  f1280d0:	8fbf008c */ 	lw	$ra,0x8c($sp)
/*  f1280d4:	8fbe0088 */ 	lw	$s8,0x88($sp)
/*  f1280d8:	8fb70084 */ 	lw	$s7,0x84($sp)
/*  f1280dc:	8fb60080 */ 	lw	$s6,0x80($sp)
/*  f1280e0:	8fb5007c */ 	lw	$s5,0x7c($sp)
/*  f1280e4:	8fb40078 */ 	lw	$s4,0x78($sp)
/*  f1280e8:	8fb30074 */ 	lw	$s3,0x74($sp)
/*  f1280ec:	8fb20070 */ 	lw	$s2,0x70($sp)
/*  f1280f0:	8fb1006c */ 	lw	$s1,0x6c($sp)
/*  f1280f4:	8fb00068 */ 	lw	$s0,0x68($sp)
/*  f1280f8:	d7be0060 */ 	ldc1	$f30,0x60($sp)
/*  f1280fc:	d7bc0058 */ 	ldc1	$f28,0x58($sp)
/*  f128100:	d7ba0050 */ 	ldc1	$f26,0x50($sp)
/*  f128104:	d7b80048 */ 	ldc1	$f24,0x48($sp)
/*  f128108:	d7b60040 */ 	ldc1	$f22,0x40($sp)
/*  f12810c:	d7b40038 */ 	ldc1	$f20,0x38($sp)
/*  f128110:	8fa20198 */ 	lw	$v0,0x198($sp)
/*  f128114:	03e00008 */ 	jr	$ra
/*  f128118:	27bd0198 */ 	addiu	$sp,$sp,0x198
);
#else
GLOBAL_ASM(
glabel sky0f126384
.late_rodata
glabel var7f1b510c
.word 0x3c23d70a
glabel var7f1b5110
.word 0x3bb60b61
glabel var7f1b5114
.word 0x3d088889
glabel var7f1b5118
.word 0x3c4ccccd
glabel var7f1b511c
.word 0x3dcccccd
.text
/*  f126384:	27bdfe70 */ 	addiu	$sp,$sp,-400
/*  f126388:	3c0f8008 */ 	lui	$t7,%hi(var8007dba0)
/*  f12638c:	afbf0084 */ 	sw	$ra,0x84($sp)
/*  f126390:	afb70080 */ 	sw	$s7,0x80($sp)
/*  f126394:	afb6007c */ 	sw	$s6,0x7c($sp)
/*  f126398:	afb50078 */ 	sw	$s5,0x78($sp)
/*  f12639c:	afb40074 */ 	sw	$s4,0x74($sp)
/*  f1263a0:	afb30070 */ 	sw	$s3,0x70($sp)
/*  f1263a4:	afb2006c */ 	sw	$s2,0x6c($sp)
/*  f1263a8:	afb10068 */ 	sw	$s1,0x68($sp)
/*  f1263ac:	afb00064 */ 	sw	$s0,0x64($sp)
/*  f1263b0:	f7be0058 */ 	sdc1	$f30,0x58($sp)
/*  f1263b4:	f7bc0050 */ 	sdc1	$f28,0x50($sp)
/*  f1263b8:	f7ba0048 */ 	sdc1	$f26,0x48($sp)
/*  f1263bc:	f7b80040 */ 	sdc1	$f24,0x40($sp)
/*  f1263c0:	f7b60038 */ 	sdc1	$f22,0x38($sp)
/*  f1263c4:	f7b40030 */ 	sdc1	$f20,0x30($sp)
/*  f1263c8:	afa40190 */ 	sw	$a0,0x190($sp)
/*  f1263cc:	25efdba0 */ 	addiu	$t7,$t7,%lo(var8007dba0)
/*  f1263d0:	8de10000 */ 	lw	$at,0x0($t7)
/*  f1263d4:	8de80004 */ 	lw	$t0,0x4($t7)
/*  f1263d8:	27ae015c */ 	addiu	$t6,$sp,0x15c
/*  f1263dc:	adc10000 */ 	sw	$at,0x0($t6)
/*  f1263e0:	adc80004 */ 	sw	$t0,0x4($t6)
/*  f1263e4:	8de8000c */ 	lw	$t0,0xc($t7)
/*  f1263e8:	8de10008 */ 	lw	$at,0x8($t7)
/*  f1263ec:	3c0a8008 */ 	lui	$t2,%hi(var8007dbb8)
/*  f1263f0:	adc8000c */ 	sw	$t0,0xc($t6)
/*  f1263f4:	adc10008 */ 	sw	$at,0x8($t6)
/*  f1263f8:	8de10010 */ 	lw	$at,0x10($t7)
/*  f1263fc:	8de80014 */ 	lw	$t0,0x14($t7)
/*  f126400:	254adbb8 */ 	addiu	$t2,$t2,%lo(var8007dbb8)
/*  f126404:	adc10010 */ 	sw	$at,0x10($t6)
/*  f126408:	adc80014 */ 	sw	$t0,0x14($t6)
/*  f12640c:	8d4d0004 */ 	lw	$t5,0x4($t2)
/*  f126410:	8d410000 */ 	lw	$at,0x0($t2)
/*  f126414:	27a90144 */ 	addiu	$t1,$sp,0x144
/*  f126418:	ad2d0004 */ 	sw	$t5,0x4($t1)
/*  f12641c:	ad210000 */ 	sw	$at,0x0($t1)
/*  f126420:	8d410008 */ 	lw	$at,0x8($t2)
/*  f126424:	8d4d000c */ 	lw	$t5,0xc($t2)
/*  f126428:	3c188008 */ 	lui	$t8,%hi(var8007dbd0)
/*  f12642c:	ad210008 */ 	sw	$at,0x8($t1)
/*  f126430:	ad2d000c */ 	sw	$t5,0xc($t1)
/*  f126434:	8d4d0014 */ 	lw	$t5,0x14($t2)
/*  f126438:	8d410010 */ 	lw	$at,0x10($t2)
/*  f12643c:	2718dbd0 */ 	addiu	$t8,$t8,%lo(var8007dbd0)
/*  f126440:	ad2d0014 */ 	sw	$t5,0x14($t1)
/*  f126444:	ad210010 */ 	sw	$at,0x10($t1)
/*  f126448:	8f080004 */ 	lw	$t0,0x4($t8)
/*  f12644c:	8f010000 */ 	lw	$at,0x0($t8)
/*  f126450:	27b9012c */ 	addiu	$t9,$sp,0x12c
/*  f126454:	af280004 */ 	sw	$t0,0x4($t9)
/*  f126458:	af210000 */ 	sw	$at,0x0($t9)
/*  f12645c:	8f010008 */ 	lw	$at,0x8($t8)
/*  f126460:	8f08000c */ 	lw	$t0,0xc($t8)
/*  f126464:	3c0c8007 */ 	lui	$t4,%hi(g_ViRes)
/*  f126468:	af210008 */ 	sw	$at,0x8($t9)
/*  f12646c:	af28000c */ 	sw	$t0,0xc($t9)
/*  f126470:	8f080014 */ 	lw	$t0,0x14($t8)
/*  f126474:	8f010010 */ 	lw	$at,0x10($t8)
/*  f126478:	4487b000 */ 	mtc1	$a3,$f22
/*  f12647c:	af280014 */ 	sw	$t0,0x14($t9)
/*  f126480:	af210010 */ 	sw	$at,0x10($t9)
/*  f126484:	8d8c06c8 */ 	lw	$t4,%lo(g_ViRes)($t4)
/*  f126488:	4485d000 */ 	mtc1	$a1,$f26
/*  f12648c:	4486e000 */ 	mtc1	$a2,$f28
/*  f126490:	24010001 */ 	addiu	$at,$zero,0x1
/*  f126494:	15810002 */ 	bne	$t4,$at,.L0f1264a0
/*  f126498:	24100001 */ 	addiu	$s0,$zero,0x1
/*  f12649c:	24100002 */ 	addiu	$s0,$zero,0x2
.L0f1264a0:
/*  f1264a0:	0c002f22 */ 	jal	viGetViewWidth
/*  f1264a4:	00000000 */ 	nop
/*  f1264a8:	44822000 */ 	mtc1	$v0,$f4
/*  f1264ac:	3c017f1b */ 	lui	$at,%hi(var7f1b510c)
/*  f1264b0:	c434510c */ 	lwc1	$f20,%lo(var7f1b510c)($at)
/*  f1264b4:	468021a0 */ 	cvt.s.w	$f6,$f4
/*  f1264b8:	3c013f00 */ 	lui	$at,0x3f00
/*  f1264bc:	4481f000 */ 	mtc1	$at,$f30
/*  f1264c0:	00000000 */ 	nop
/*  f1264c4:	461e3202 */ 	mul.s	$f8,$f6,$f30
/*  f1264c8:	4608d281 */ 	sub.s	$f10,$f26,$f8
/*  f1264cc:	46145102 */ 	mul.s	$f4,$f10,$f20
/*  f1264d0:	0c002f26 */ 	jal	viGetViewHeight
/*  f1264d4:	e7a40128 */ 	swc1	$f4,0x128($sp)
/*  f1264d8:	44823000 */ 	mtc1	$v0,$f6
/*  f1264dc:	3c16800b */ 	lui	$s6,%hi(g_TexLightGlareConfigs)
/*  f1264e0:	26d6b5a0 */ 	addiu	$s6,$s6,%lo(g_TexLightGlareConfigs)
/*  f1264e4:	46803220 */ 	cvt.s.w	$f8,$f6
/*  f1264e8:	8ec50000 */ 	lw	$a1,0x0($s6)
/*  f1264ec:	27b50190 */ 	addiu	$s5,$sp,0x190
/*  f1264f0:	240b0002 */ 	addiu	$t3,$zero,0x2
/*  f1264f4:	24090001 */ 	addiu	$t1,$zero,0x1
/*  f1264f8:	afa90014 */ 	sw	$t1,0x14($sp)
/*  f1264fc:	461e4282 */ 	mul.s	$f10,$f8,$f30
/*  f126500:	afab0010 */ 	sw	$t3,0x10($sp)
/*  f126504:	02a02025 */ 	or	$a0,$s5,$zero
/*  f126508:	24060004 */ 	addiu	$a2,$zero,0x4
/*  f12650c:	00003825 */ 	or	$a3,$zero,$zero
/*  f126510:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f126514:	24a50048 */ 	addiu	$a1,$a1,0x48
/*  f126518:	460ae101 */ 	sub.s	$f4,$f28,$f10
/*  f12651c:	46142182 */ 	mul.s	$f6,$f4,$f20
/*  f126520:	0fc2ce70 */ 	jal	texSelect
/*  f126524:	e7a60124 */ 	swc1	$f6,0x124($sp)
/*  f126528:	8faa0190 */ 	lw	$t2,0x190($sp)
/*  f12652c:	3c11ba00 */ 	lui	$s1,0xba00
/*  f126530:	36311402 */ 	ori	$s1,$s1,0x1402
/*  f126534:	254d0008 */ 	addiu	$t5,$t2,0x8
/*  f126538:	afad0190 */ 	sw	$t5,0x190($sp)
/*  f12653c:	ad400004 */ 	sw	$zero,0x4($t2)
/*  f126540:	ad510000 */ 	sw	$s1,0x0($t2)
/*  f126544:	8faf0190 */ 	lw	$t7,0x190($sp)
/*  f126548:	3c19ba00 */ 	lui	$t9,0xba00
/*  f12654c:	37390602 */ 	ori	$t9,$t9,0x602
/*  f126550:	25ee0008 */ 	addiu	$t6,$t7,0x8
/*  f126554:	afae0190 */ 	sw	$t6,0x190($sp)
/*  f126558:	24180040 */ 	addiu	$t8,$zero,0x40
/*  f12655c:	adf80004 */ 	sw	$t8,0x4($t7)
/*  f126560:	adf90000 */ 	sw	$t9,0x0($t7)
/*  f126564:	8fa80190 */ 	lw	$t0,0x190($sp)
/*  f126568:	3c0bba00 */ 	lui	$t3,0xba00
/*  f12656c:	356b0402 */ 	ori	$t3,$t3,0x402
/*  f126570:	250c0008 */ 	addiu	$t4,$t0,0x8
/*  f126574:	afac0190 */ 	sw	$t4,0x190($sp)
/*  f126578:	ad000004 */ 	sw	$zero,0x4($t0)
/*  f12657c:	ad0b0000 */ 	sw	$t3,0x0($t0)
/*  f126580:	8fa90190 */ 	lw	$t1,0x190($sp)
/*  f126584:	3c12b900 */ 	lui	$s2,0xb900
/*  f126588:	3c130050 */ 	lui	$s3,0x50
/*  f12658c:	252a0008 */ 	addiu	$t2,$t1,0x8
/*  f126590:	afaa0190 */ 	sw	$t2,0x190($sp)
/*  f126594:	367341c8 */ 	ori	$s3,$s3,0x41c8
/*  f126598:	3652031d */ 	ori	$s2,$s2,0x31d
/*  f12659c:	ad320000 */ 	sw	$s2,0x0($t1)
/*  f1265a0:	ad330004 */ 	sw	$s3,0x4($t1)
/*  f1265a4:	8fad0190 */ 	lw	$t5,0x190($sp)
/*  f1265a8:	3c0eba00 */ 	lui	$t6,0xba00
/*  f1265ac:	35ce1301 */ 	ori	$t6,$t6,0x1301
/*  f1265b0:	25af0008 */ 	addiu	$t7,$t5,0x8
/*  f1265b4:	afaf0190 */ 	sw	$t7,0x190($sp)
/*  f1265b8:	ada00004 */ 	sw	$zero,0x4($t5)
/*  f1265bc:	adae0000 */ 	sw	$t6,0x0($t5)
/*  f1265c0:	8fb90190 */ 	lw	$t9,0x190($sp)
/*  f1265c4:	3c14b900 */ 	lui	$s4,0xb900
/*  f1265c8:	36940002 */ 	ori	$s4,$s4,0x2
/*  f1265cc:	27380008 */ 	addiu	$t8,$t9,0x8
/*  f1265d0:	afb80190 */ 	sw	$t8,0x190($sp)
/*  f1265d4:	af200004 */ 	sw	$zero,0x4($t9)
/*  f1265d8:	af340000 */ 	sw	$s4,0x0($t9)
/*  f1265dc:	8fa80190 */ 	lw	$t0,0x190($sp)
/*  f1265e0:	3c0bba00 */ 	lui	$t3,0xba00
/*  f1265e4:	356b1001 */ 	ori	$t3,$t3,0x1001
/*  f1265e8:	250c0008 */ 	addiu	$t4,$t0,0x8
/*  f1265ec:	afac0190 */ 	sw	$t4,0x190($sp)
/*  f1265f0:	ad000004 */ 	sw	$zero,0x4($t0)
/*  f1265f4:	ad0b0000 */ 	sw	$t3,0x0($t0)
/*  f1265f8:	8fa90190 */ 	lw	$t1,0x190($sp)
/*  f1265fc:	3c0dba00 */ 	lui	$t5,0xba00
/*  f126600:	35ad0903 */ 	ori	$t5,$t5,0x903
/*  f126604:	252a0008 */ 	addiu	$t2,$t1,0x8
/*  f126608:	afaa0190 */ 	sw	$t2,0x190($sp)
/*  f12660c:	240f0c00 */ 	addiu	$t7,$zero,0xc00
/*  f126610:	ad2f0004 */ 	sw	$t7,0x4($t1)
/*  f126614:	ad2d0000 */ 	sw	$t5,0x0($t1)
/*  f126618:	8fae0190 */ 	lw	$t6,0x190($sp)
/*  f12661c:	3c18ba00 */ 	lui	$t8,0xba00
/*  f126620:	37180e02 */ 	ori	$t8,$t8,0xe02
/*  f126624:	25d90008 */ 	addiu	$t9,$t6,0x8
/*  f126628:	afb90190 */ 	sw	$t9,0x190($sp)
/*  f12662c:	adc00004 */ 	sw	$zero,0x4($t6)
/*  f126630:	add80000 */ 	sw	$t8,0x0($t6)
/*  f126634:	8fa80190 */ 	lw	$t0,0x190($sp)
/*  f126638:	3c0bba00 */ 	lui	$t3,0xba00
/*  f12663c:	356b0c02 */ 	ori	$t3,$t3,0xc02
/*  f126640:	250c0008 */ 	addiu	$t4,$t0,0x8
/*  f126644:	afac0190 */ 	sw	$t4,0x190($sp)
/*  f126648:	24092000 */ 	addiu	$t1,$zero,0x2000
/*  f12664c:	ad090004 */ 	sw	$t1,0x4($t0)
/*  f126650:	ad0b0000 */ 	sw	$t3,0x0($t0)
/*  f126654:	8faa0190 */ 	lw	$t2,0x190($sp)
/*  f126658:	3c0eff36 */ 	lui	$t6,0xff36
/*  f12665c:	3c0ffcff */ 	lui	$t7,0xfcff
/*  f126660:	254d0008 */ 	addiu	$t5,$t2,0x8
/*  f126664:	afad0190 */ 	sw	$t5,0x190($sp)
/*  f126668:	35ef9bff */ 	ori	$t7,$t7,0x9bff
/*  f12666c:	35ceff7f */ 	ori	$t6,$t6,0xff7f
/*  f126670:	ad4e0004 */ 	sw	$t6,0x4($t2)
/*  f126674:	0c002f9d */ 	jal	viGetFovY
/*  f126678:	ad4f0000 */ 	sw	$t7,0x0($t2)
/*  f12667c:	8fb90190 */ 	lw	$t9,0x190($sp)
/*  f126680:	3c17fb00 */ 	lui	$s7,0xfb00
/*  f126684:	3c013f00 */ 	lui	$at,0x3f00
/*  f126688:	27380008 */ 	addiu	$t8,$t9,0x8
/*  f12668c:	afb80190 */ 	sw	$t8,0x190($sp)
/*  f126690:	af370000 */ 	sw	$s7,0x0($t9)
/*  f126694:	c7a801a8 */ 	lwc1	$f8,0x1a8($sp)
/*  f126698:	4481a000 */ 	mtc1	$at,$f20
/*  f12669c:	3c01437f */ 	lui	$at,0x437f
/*  f1266a0:	46164282 */ 	mul.s	$f10,$f8,$f22
/*  f1266a4:	44812000 */ 	mtc1	$at,$f4
/*  f1266a8:	2401ff00 */ 	addiu	$at,$zero,-256
/*  f1266ac:	24180001 */ 	addiu	$t8,$zero,0x1
/*  f1266b0:	240e0001 */ 	addiu	$t6,$zero,0x1
/*  f1266b4:	24080001 */ 	addiu	$t0,$zero,0x1
/*  f1266b8:	02a02025 */ 	or	$a0,$s5,$zero
/*  f1266bc:	46045182 */ 	mul.s	$f6,$f10,$f4
/*  f1266c0:	27a5017c */ 	addiu	$a1,$sp,0x17c
/*  f1266c4:	27a60174 */ 	addiu	$a2,$sp,0x174
/*  f1266c8:	4616a102 */ 	mul.s	$f4,$f20,$f22
/*  f1266cc:	4600320d */ 	trunc.w.s	$f8,$f6
/*  f1266d0:	4604a180 */ 	add.s	$f6,$f20,$f4
/*  f1266d4:	440c4000 */ 	mfc1	$t4,$f8
/*  f1266d8:	00000000 */ 	nop
/*  f1266dc:	318b00ff */ 	andi	$t3,$t4,0xff
/*  f1266e0:	01614825 */ 	or	$t1,$t3,$at
/*  f1266e4:	af290004 */ 	sw	$t1,0x4($t9)
/*  f1266e8:	c7aa01a0 */ 	lwc1	$f10,0x1a0($sp)
/*  f1266ec:	3c014270 */ 	lui	$at,0x4270
/*  f1266f0:	44812000 */ 	mtc1	$at,$f4
/*  f1266f4:	46065202 */ 	mul.s	$f8,$f10,$f6
/*  f1266f8:	8ec20000 */ 	lw	$v0,0x0($s6)
/*  f1266fc:	e7ba017c */ 	swc1	$f26,0x17c($sp)
/*  f126700:	e7bc0180 */ 	swc1	$f28,0x180($sp)
/*  f126704:	24190001 */ 	addiu	$t9,$zero,0x1
/*  f126708:	46002283 */ 	div.s	$f10,$f4,$f0
/*  f12670c:	46085182 */ 	mul.s	$f6,$f10,$f8
/*  f126710:	4600310d */ 	trunc.w.s	$f4,$f6
/*  f126714:	44903000 */ 	mtc1	$s0,$f6
/*  f126718:	440d2000 */ 	mfc1	$t5,$f4
/*  f12671c:	46803620 */ 	cvt.s.w	$f24,$f6
/*  f126720:	448d5000 */ 	mtc1	$t5,$f10
/*  f126724:	00000000 */ 	nop
/*  f126728:	46805220 */ 	cvt.s.w	$f8,$f10
/*  f12672c:	46144082 */ 	mul.s	$f2,$f8,$f20
/*  f126730:	00000000 */ 	nop
/*  f126734:	46181102 */ 	mul.s	$f4,$f2,$f24
/*  f126738:	e7a20178 */ 	swc1	$f2,0x178($sp)
/*  f12673c:	e7a40174 */ 	swc1	$f4,0x174($sp)
/*  f126740:	904f004d */ 	lbu	$t7,0x4d($v0)
/*  f126744:	9047004c */ 	lbu	$a3,0x4c($v0)
/*  f126748:	afa80028 */ 	sw	$t0,0x28($sp)
/*  f12674c:	afa00024 */ 	sw	$zero,0x24($sp)
/*  f126750:	afb80020 */ 	sw	$t8,0x20($sp)
/*  f126754:	afb9001c */ 	sw	$t9,0x1c($sp)
/*  f126758:	afae0018 */ 	sw	$t6,0x18($sp)
/*  f12675c:	afa00014 */ 	sw	$zero,0x14($sp)
/*  f126760:	0fc2c854 */ 	jal	func0f0b2150
/*  f126764:	afaf0010 */ 	sw	$t7,0x10($sp)
/*  f126768:	8ec50000 */ 	lw	$a1,0x0($s6)
/*  f12676c:	240c0002 */ 	addiu	$t4,$zero,0x2
/*  f126770:	240b0001 */ 	addiu	$t3,$zero,0x1
/*  f126774:	afab0014 */ 	sw	$t3,0x14($sp)
/*  f126778:	afac0010 */ 	sw	$t4,0x10($sp)
/*  f12677c:	02a02025 */ 	or	$a0,$s5,$zero
/*  f126780:	24060004 */ 	addiu	$a2,$zero,0x4
/*  f126784:	00003825 */ 	or	$a3,$zero,$zero
/*  f126788:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f12678c:	0fc2ce70 */ 	jal	texSelect
/*  f126790:	24a5000c */ 	addiu	$a1,$a1,0xc
/*  f126794:	8fa90190 */ 	lw	$t1,0x190($sp)
/*  f126798:	3c0eba00 */ 	lui	$t6,0xba00
/*  f12679c:	35ce0602 */ 	ori	$t6,$t6,0x602
/*  f1267a0:	252a0008 */ 	addiu	$t2,$t1,0x8
/*  f1267a4:	afaa0190 */ 	sw	$t2,0x190($sp)
/*  f1267a8:	ad200004 */ 	sw	$zero,0x4($t1)
/*  f1267ac:	ad310000 */ 	sw	$s1,0x0($t1)
/*  f1267b0:	8fad0190 */ 	lw	$t5,0x190($sp)
/*  f1267b4:	24190040 */ 	addiu	$t9,$zero,0x40
/*  f1267b8:	3c0cba00 */ 	lui	$t4,0xba00
/*  f1267bc:	25af0008 */ 	addiu	$t7,$t5,0x8
/*  f1267c0:	afaf0190 */ 	sw	$t7,0x190($sp)
/*  f1267c4:	adb90004 */ 	sw	$t9,0x4($t5)
/*  f1267c8:	adae0000 */ 	sw	$t6,0x0($t5)
/*  f1267cc:	8fb80190 */ 	lw	$t8,0x190($sp)
/*  f1267d0:	358c0402 */ 	ori	$t4,$t4,0x402
/*  f1267d4:	3c0fba00 */ 	lui	$t7,0xba00
/*  f1267d8:	27080008 */ 	addiu	$t0,$t8,0x8
/*  f1267dc:	afa80190 */ 	sw	$t0,0x190($sp)
/*  f1267e0:	af000004 */ 	sw	$zero,0x4($t8)
/*  f1267e4:	af0c0000 */ 	sw	$t4,0x0($t8)
/*  f1267e8:	8fab0190 */ 	lw	$t3,0x190($sp)
/*  f1267ec:	35ef1301 */ 	ori	$t7,$t7,0x1301
/*  f1267f0:	3c0cba00 */ 	lui	$t4,0xba00
/*  f1267f4:	25690008 */ 	addiu	$t1,$t3,0x8
/*  f1267f8:	afa90190 */ 	sw	$t1,0x190($sp)
/*  f1267fc:	ad730004 */ 	sw	$s3,0x4($t3)
/*  f126800:	ad720000 */ 	sw	$s2,0x0($t3)
/*  f126804:	8faa0190 */ 	lw	$t2,0x190($sp)
/*  f126808:	358c1001 */ 	ori	$t4,$t4,0x1001
/*  f12680c:	3c017f1b */ 	lui	$at,%hi(var7f1b5110)
/*  f126810:	254d0008 */ 	addiu	$t5,$t2,0x8
/*  f126814:	afad0190 */ 	sw	$t5,0x190($sp)
/*  f126818:	ad400004 */ 	sw	$zero,0x4($t2)
/*  f12681c:	ad4f0000 */ 	sw	$t7,0x0($t2)
/*  f126820:	8fae0190 */ 	lw	$t6,0x190($sp)
/*  f126824:	3c0aba00 */ 	lui	$t2,0xba00
/*  f126828:	354a0903 */ 	ori	$t2,$t2,0x903
/*  f12682c:	25d90008 */ 	addiu	$t9,$t6,0x8
/*  f126830:	afb90190 */ 	sw	$t9,0x190($sp)
/*  f126834:	adc00004 */ 	sw	$zero,0x4($t6)
/*  f126838:	add40000 */ 	sw	$s4,0x0($t6)
/*  f12683c:	8fb80190 */ 	lw	$t8,0x190($sp)
/*  f126840:	240d0c00 */ 	addiu	$t5,$zero,0xc00
/*  f126844:	3c19ba00 */ 	lui	$t9,0xba00
/*  f126848:	27080008 */ 	addiu	$t0,$t8,0x8
/*  f12684c:	afa80190 */ 	sw	$t0,0x190($sp)
/*  f126850:	af000004 */ 	sw	$zero,0x4($t8)
/*  f126854:	af0c0000 */ 	sw	$t4,0x0($t8)
/*  f126858:	8fab0190 */ 	lw	$t3,0x190($sp)
/*  f12685c:	37390e02 */ 	ori	$t9,$t9,0xe02
/*  f126860:	3c0cba00 */ 	lui	$t4,0xba00
/*  f126864:	25690008 */ 	addiu	$t1,$t3,0x8
/*  f126868:	afa90190 */ 	sw	$t1,0x190($sp)
/*  f12686c:	ad6d0004 */ 	sw	$t5,0x4($t3)
/*  f126870:	ad6a0000 */ 	sw	$t2,0x0($t3)
/*  f126874:	8faf0190 */ 	lw	$t7,0x190($sp)
/*  f126878:	240b2000 */ 	addiu	$t3,$zero,0x2000
/*  f12687c:	358c0c02 */ 	ori	$t4,$t4,0xc02
/*  f126880:	25ee0008 */ 	addiu	$t6,$t7,0x8
/*  f126884:	afae0190 */ 	sw	$t6,0x190($sp)
/*  f126888:	ade00004 */ 	sw	$zero,0x4($t7)
/*  f12688c:	adf90000 */ 	sw	$t9,0x0($t7)
/*  f126890:	8fb80190 */ 	lw	$t8,0x190($sp)
/*  f126894:	3c0fff36 */ 	lui	$t7,0xff36
/*  f126898:	3c0dfcff */ 	lui	$t5,0xfcff
/*  f12689c:	27080008 */ 	addiu	$t0,$t8,0x8
/*  f1268a0:	afa80190 */ 	sw	$t0,0x190($sp)
/*  f1268a4:	af0b0004 */ 	sw	$t3,0x4($t8)
/*  f1268a8:	af0c0000 */ 	sw	$t4,0x0($t8)
/*  f1268ac:	8fa90190 */ 	lw	$t1,0x190($sp)
/*  f1268b0:	35ad9bff */ 	ori	$t5,$t5,0x9bff
/*  f1268b4:	35efff7f */ 	ori	$t7,$t7,0xff7f
/*  f1268b8:	252a0008 */ 	addiu	$t2,$t1,0x8
/*  f1268bc:	afaa0190 */ 	sw	$t2,0x190($sp)
/*  f1268c0:	ad2f0004 */ 	sw	$t7,0x4($t1)
/*  f1268c4:	ad2d0000 */ 	sw	$t5,0x0($t1)
/*  f1268c8:	c43e5110 */ 	lwc1	$f30,%lo(var7f1b5110)($at)
/*  f1268cc:	3c014334 */ 	lui	$at,0x4334
/*  f1268d0:	e7bc0198 */ 	swc1	$f28,0x198($sp)
/*  f1268d4:	4481e000 */ 	mtc1	$at,$f28
/*  f1268d8:	3c013f80 */ 	lui	$at,0x3f80
/*  f1268dc:	e7ba0194 */ 	swc1	$f26,0x194($sp)
/*  f1268e0:	4481d000 */ 	mtc1	$at,$f26
/*  f1268e4:	3c017f1b */ 	lui	$at,%hi(var7f1b5114)
/*  f1268e8:	e7b8009c */ 	swc1	$f24,0x9c($sp)
/*  f1268ec:	4480b000 */ 	mtc1	$zero,$f22
/*  f1268f0:	c4385114 */ 	lwc1	$f24,%lo(var7f1b5114)($at)
/*  f1268f4:	8fb301a4 */ 	lw	$s3,0x1a4($sp)
/*  f1268f8:	27b40144 */ 	addiu	$s4,$sp,0x144
/*  f1268fc:	27b2015c */ 	addiu	$s2,$sp,0x15c
/*  f126900:	27b10144 */ 	addiu	$s1,$sp,0x144
/*  f126904:	27b0012c */ 	addiu	$s0,$sp,0x12c
.L0f126908:
/*  f126908:	2a61005a */ 	slti	$at,$s3,0x5a
/*  f12690c:	1020000c */ 	beqz	$at,.L0f126940
/*  f126910:	266effa6 */ 	addiu	$t6,$s3,-90
/*  f126914:	2a61001e */ 	slti	$at,$s3,0x1e
/*  f126918:	10200007 */ 	beqz	$at,.L0f126938
/*  f12691c:	00000000 */ 	nop
/*  f126920:	44935000 */ 	mtc1	$s3,$f10
/*  f126924:	00000000 */ 	nop
/*  f126928:	46805220 */ 	cvt.s.w	$f8,$f10
/*  f12692c:	46184082 */ 	mul.s	$f2,$f8,$f24
/*  f126930:	10000011 */ 	b	.L0f126978
/*  f126934:	8e390000 */ 	lw	$t9,0x0($s1)
.L0f126938:
/*  f126938:	1000000e */ 	b	.L0f126974
/*  f12693c:	4600d086 */ 	mov.s	$f2,$f26
.L0f126940:
/*  f126940:	448e3000 */ 	mtc1	$t6,$f6
/*  f126944:	00000000 */ 	nop
/*  f126948:	46803120 */ 	cvt.s.w	$f4,$f6
/*  f12694c:	4604e281 */ 	sub.s	$f10,$f28,$f4
/*  f126950:	461e5202 */ 	mul.s	$f8,$f10,$f30
/*  f126954:	00000000 */ 	nop
/*  f126958:	46144082 */ 	mul.s	$f2,$f8,$f20
/*  f12695c:	4616103c */ 	c.lt.s	$f2,$f22
/*  f126960:	00000000 */ 	nop
/*  f126964:	45000002 */ 	bc1f	.L0f126970
/*  f126968:	00000000 */ 	nop
/*  f12696c:	4600b086 */ 	mov.s	$f2,$f22
.L0f126970:
/*  f126970:	46141080 */ 	add.s	$f2,$f2,$f20
.L0f126974:
/*  f126974:	8e390000 */ 	lw	$t9,0x0($s1)
.L0f126978:
/*  f126978:	c7aa0128 */ 	lwc1	$f10,0x128($sp)
/*  f12697c:	8fa30190 */ 	lw	$v1,0x190($sp)
/*  f126980:	44993000 */ 	mtc1	$t9,$f6
/*  f126984:	c7a40194 */ 	lwc1	$f4,0x194($sp)
/*  f126988:	8e580000 */ 	lw	$t8,0x0($s2)
/*  f12698c:	46803020 */ 	cvt.s.w	$f0,$f6
/*  f126990:	246c0008 */ 	addiu	$t4,$v1,0x8
/*  f126994:	c7a60198 */ 	lwc1	$f6,0x198($sp)
/*  f126998:	afac0190 */ 	sw	$t4,0x190($sp)
/*  f12699c:	3c014f80 */ 	lui	$at,0x4f80
/*  f1269a0:	460a0202 */ 	mul.s	$f8,$f0,$f10
/*  f1269a4:	c7aa0124 */ 	lwc1	$f10,0x124($sp)
/*  f1269a8:	ac770000 */ 	sw	$s7,0x0($v1)
/*  f1269ac:	8e020000 */ 	lw	$v0,0x0($s0)
/*  f1269b0:	304b00ff */ 	andi	$t3,$v0,0xff
/*  f1269b4:	46082301 */ 	sub.s	$f12,$f4,$f8
/*  f1269b8:	460a0102 */ 	mul.s	$f4,$f0,$f10
/*  f1269bc:	44984000 */ 	mtc1	$t8,$f8
/*  f1269c0:	448b5000 */ 	mtc1	$t3,$f10
/*  f1269c4:	46804420 */ 	cvt.s.w	$f16,$f8
/*  f1269c8:	c7a801a8 */ 	lwc1	$f8,0x1a8($sp)
/*  f1269cc:	46043381 */ 	sub.s	$f14,$f6,$f4
/*  f1269d0:	468051a0 */ 	cvt.s.w	$f6,$f10
/*  f1269d4:	46024282 */ 	mul.s	$f10,$f8,$f2
/*  f1269d8:	05610004 */ 	bgez	$t3,.L0f1269ec
/*  f1269dc:	00000000 */ 	nop
/*  f1269e0:	44812000 */ 	mtc1	$at,$f4
/*  f1269e4:	00000000 */ 	nop
/*  f1269e8:	46043180 */ 	add.s	$f6,$f6,$f4
.L0f1269ec:
/*  f1269ec:	460a3102 */ 	mul.s	$f4,$f6,$f10
/*  f1269f0:	02a02025 */ 	or	$a0,$s5,$zero
/*  f1269f4:	0002ce02 */ 	srl	$t9,$v0,0x18
/*  f1269f8:	0019c600 */ 	sll	$t8,$t9,0x18
/*  f1269fc:	00025c02 */ 	srl	$t3,$v0,0x10
/*  f126a00:	316900ff */ 	andi	$t1,$t3,0xff
/*  f126a04:	0002ca02 */ 	srl	$t9,$v0,0x8
/*  f126a08:	4600220d */ 	trunc.w.s	$f8,$f4
/*  f126a0c:	46148482 */ 	mul.s	$f18,$f16,$f20
/*  f126a10:	27a5017c */ 	addiu	$a1,$sp,0x17c
/*  f126a14:	27a60174 */ 	addiu	$a2,$sp,0x174
/*  f126a18:	440a4000 */ 	mfc1	$t2,$f8
/*  f126a1c:	00000000 */ 	nop
/*  f126a20:	314d00ff */ 	andi	$t5,$t2,0xff
/*  f126a24:	01b84025 */ 	or	$t0,$t5,$t8
/*  f126a28:	332d00ff */ 	andi	$t5,$t9,0xff
/*  f126a2c:	00095400 */ 	sll	$t2,$t1,0x10
/*  f126a30:	010a7825 */ 	or	$t7,$t0,$t2
/*  f126a34:	000dc200 */ 	sll	$t8,$t5,0x8
/*  f126a38:	01f86025 */ 	or	$t4,$t7,$t8
/*  f126a3c:	ac6c0004 */ 	sw	$t4,0x4($v1)
/*  f126a40:	c7a6009c */ 	lwc1	$f6,0x9c($sp)
/*  f126a44:	8ec20000 */ 	lw	$v0,0x0($s6)
/*  f126a48:	e7ac017c */ 	swc1	$f12,0x17c($sp)
/*  f126a4c:	46069282 */ 	mul.s	$f10,$f18,$f6
/*  f126a50:	e7ae0180 */ 	swc1	$f14,0x180($sp)
/*  f126a54:	e7b20178 */ 	swc1	$f18,0x178($sp)
/*  f126a58:	24090001 */ 	addiu	$t1,$zero,0x1
/*  f126a5c:	e7aa0174 */ 	swc1	$f10,0x174($sp)
/*  f126a60:	904b0011 */ 	lbu	$t3,0x11($v0)
/*  f126a64:	90470010 */ 	lbu	$a3,0x10($v0)
/*  f126a68:	afa90028 */ 	sw	$t1,0x28($sp)
/*  f126a6c:	afa00024 */ 	sw	$zero,0x24($sp)
/*  f126a70:	afa00020 */ 	sw	$zero,0x20($sp)
/*  f126a74:	afa0001c */ 	sw	$zero,0x1c($sp)
/*  f126a78:	afa00018 */ 	sw	$zero,0x18($sp)
/*  f126a7c:	afa00014 */ 	sw	$zero,0x14($sp)
/*  f126a80:	0fc2c854 */ 	jal	func0f0b2150
/*  f126a84:	afab0010 */ 	sw	$t3,0x10($sp)
/*  f126a88:	26100004 */ 	addiu	$s0,$s0,0x4
/*  f126a8c:	26310004 */ 	addiu	$s1,$s1,0x4
/*  f126a90:	1614ff9d */ 	bne	$s0,$s4,.L0f126908
/*  f126a94:	26520004 */ 	addiu	$s2,$s2,0x4
/*  f126a98:	0c002f22 */ 	jal	viGetViewWidth
/*  f126a9c:	00000000 */ 	nop
/*  f126aa0:	44822000 */ 	mtc1	$v0,$f4
/*  f126aa4:	3c013f00 */ 	lui	$at,0x3f00
/*  f126aa8:	4481c000 */ 	mtc1	$at,$f24
/*  f126aac:	46802220 */ 	cvt.s.w	$f8,$f4
/*  f126ab0:	c7aa0194 */ 	lwc1	$f10,0x194($sp)
/*  f126ab4:	46184182 */ 	mul.s	$f6,$f8,$f24
/*  f126ab8:	0c002f26 */ 	jal	viGetViewHeight
/*  f126abc:	460a3501 */ 	sub.s	$f20,$f6,$f10
/*  f126ac0:	44822000 */ 	mtc1	$v0,$f4
/*  f126ac4:	c7aa0198 */ 	lwc1	$f10,0x198($sp)
/*  f126ac8:	46802220 */ 	cvt.s.w	$f8,$f4
/*  f126acc:	46184182 */ 	mul.s	$f6,$f8,$f24
/*  f126ad0:	460a3001 */ 	sub.s	$f0,$f6,$f10
/*  f126ad4:	4614a102 */ 	mul.s	$f4,$f20,$f20
/*  f126ad8:	00000000 */ 	nop
/*  f126adc:	46000202 */ 	mul.s	$f8,$f0,$f0
/*  f126ae0:	0c012974 */ 	jal	sqrtf
/*  f126ae4:	46082300 */ 	add.s	$f12,$f4,$f8
/*  f126ae8:	3c014220 */ 	lui	$at,0x4220
/*  f126aec:	44813000 */ 	mtc1	$at,$f6
/*  f126af0:	3c017f1b */ 	lui	$at,%hi(var7f1b5118)
/*  f126af4:	c4245118 */ 	lwc1	$f4,%lo(var7f1b5118)($at)
/*  f126af8:	46003281 */ 	sub.s	$f10,$f6,$f0
/*  f126afc:	3c08800a */ 	lui	$t0,%hi(g_Vars+0x34)
/*  f126b00:	c7a601a8 */ 	lwc1	$f6,0x1a8($sp)
/*  f126b04:	46045302 */ 	mul.s	$f12,$f10,$f4
/*  f126b08:	4616603c */ 	c.lt.s	$f12,$f22
/*  f126b0c:	00000000 */ 	nop
/*  f126b10:	45000002 */ 	bc1f	.L0f126b1c
/*  f126b14:	00000000 */ 	nop
/*  f126b18:	4600b306 */ 	mov.s	$f12,$f22
.L0f126b1c:
/*  f126b1c:	8d089ff4 */ 	lw	$t0,%lo(g_Vars+0x34)($t0)
/*  f126b20:	3c017f1b */ 	lui	$at,%hi(var7f1b511c)
/*  f126b24:	c428511c */ 	lwc1	$f8,%lo(var7f1b511c)($at)
/*  f126b28:	0113082a */ 	slt	$at,$t0,$s3
/*  f126b2c:	14200002 */ 	bnez	$at,.L0f126b38
/*  f126b30:	46086300 */ 	add.s	$f12,$f12,$f8
/*  f126b34:	4600b306 */ 	mov.s	$f12,$f22
.L0f126b38:
/*  f126b38:	460cb03c */ 	c.lt.s	$f22,$f12
/*  f126b3c:	00000000 */ 	nop
/*  f126b40:	4502000d */ 	bc1fl	.L0f126b78
/*  f126b44:	8fae0190 */ 	lw	$t6,0x190($sp)
/*  f126b48:	460c3282 */ 	mul.s	$f10,$f6,$f12
/*  f126b4c:	3c01437f */ 	lui	$at,0x437f
/*  f126b50:	44812000 */ 	mtc1	$at,$f4
/*  f126b54:	00000000 */ 	nop
/*  f126b58:	46045202 */ 	mul.s	$f8,$f10,$f4
/*  f126b5c:	4600418d */ 	trunc.w.s	$f6,$f8
/*  f126b60:	44043000 */ 	mfc1	$a0,$f6
/*  f126b64:	00000000 */ 	nop
/*  f126b68:	00802825 */ 	or	$a1,$a0,$zero
/*  f126b6c:	0fc49ccd */ 	jal	sky0f127334
/*  f126b70:	00803025 */ 	or	$a2,$a0,$zero
/*  f126b74:	8fae0190 */ 	lw	$t6,0x190($sp)
.L0f126b78:
/*  f126b78:	3c0dba00 */ 	lui	$t5,0xba00
/*  f126b7c:	35ad0602 */ 	ori	$t5,$t5,0x602
/*  f126b80:	25d90008 */ 	addiu	$t9,$t6,0x8
/*  f126b84:	afb90190 */ 	sw	$t9,0x190($sp)
/*  f126b88:	240f0040 */ 	addiu	$t7,$zero,0x40
/*  f126b8c:	adcf0004 */ 	sw	$t7,0x4($t6)
/*  f126b90:	adcd0000 */ 	sw	$t5,0x0($t6)
/*  f126b94:	8fb80190 */ 	lw	$t8,0x190($sp)
/*  f126b98:	3c0bba00 */ 	lui	$t3,0xba00
/*  f126b9c:	356b0402 */ 	ori	$t3,$t3,0x402
/*  f126ba0:	270c0008 */ 	addiu	$t4,$t8,0x8
/*  f126ba4:	afac0190 */ 	sw	$t4,0x190($sp)
/*  f126ba8:	240900c0 */ 	addiu	$t1,$zero,0xc0
/*  f126bac:	af090004 */ 	sw	$t1,0x4($t8)
/*  f126bb0:	af0b0000 */ 	sw	$t3,0x0($t8)
/*  f126bb4:	8fa80190 */ 	lw	$t0,0x190($sp)
/*  f126bb8:	3c0eba00 */ 	lui	$t6,0xba00
/*  f126bbc:	35ce1301 */ 	ori	$t6,$t6,0x1301
/*  f126bc0:	250a0008 */ 	addiu	$t2,$t0,0x8
/*  f126bc4:	afaa0190 */ 	sw	$t2,0x190($sp)
/*  f126bc8:	3c190008 */ 	lui	$t9,0x8
/*  f126bcc:	ad190004 */ 	sw	$t9,0x4($t0)
/*  f126bd0:	ad0e0000 */ 	sw	$t6,0x0($t0)
/*  f126bd4:	8fad0190 */ 	lw	$t5,0x190($sp)
/*  f126bd8:	3c18ba00 */ 	lui	$t8,0xba00
/*  f126bdc:	37181001 */ 	ori	$t8,$t8,0x1001
/*  f126be0:	25af0008 */ 	addiu	$t7,$t5,0x8
/*  f126be4:	afaf0190 */ 	sw	$t7,0x190($sp)
/*  f126be8:	3c0c0001 */ 	lui	$t4,0x1
/*  f126bec:	adac0004 */ 	sw	$t4,0x4($t5)
/*  f126bf0:	adb80000 */ 	sw	$t8,0x0($t5)
/*  f126bf4:	8fbf0084 */ 	lw	$ra,0x84($sp)
/*  f126bf8:	8fb70080 */ 	lw	$s7,0x80($sp)
/*  f126bfc:	8fb6007c */ 	lw	$s6,0x7c($sp)
/*  f126c00:	8fb50078 */ 	lw	$s5,0x78($sp)
/*  f126c04:	8fb40074 */ 	lw	$s4,0x74($sp)
/*  f126c08:	8fb30070 */ 	lw	$s3,0x70($sp)
/*  f126c0c:	8fb2006c */ 	lw	$s2,0x6c($sp)
/*  f126c10:	8fb10068 */ 	lw	$s1,0x68($sp)
/*  f126c14:	8fb00064 */ 	lw	$s0,0x64($sp)
/*  f126c18:	d7be0058 */ 	ldc1	$f30,0x58($sp)
/*  f126c1c:	d7bc0050 */ 	ldc1	$f28,0x50($sp)
/*  f126c20:	d7ba0048 */ 	ldc1	$f26,0x48($sp)
/*  f126c24:	d7b80040 */ 	ldc1	$f24,0x40($sp)
/*  f126c28:	d7b60038 */ 	ldc1	$f22,0x38($sp)
/*  f126c2c:	d7b40030 */ 	ldc1	$f20,0x30($sp)
/*  f126c30:	8fa20190 */ 	lw	$v0,0x190($sp)
/*  f126c34:	03e00008 */ 	jr	$ra
/*  f126c38:	27bd0190 */ 	addiu	$sp,$sp,0x190
);
#endif

/**
 * Render a sun and its artifacts.
 */
//Gfx *sky0f126384(Gfx *gdl, f32 x, f32 y, f32 arg3, f32 size, s32 arg5, f32 arg6)
//{
//	f32 fa;
//	f32 fb;
//	f32 fc;
//	f32 sp17c[2];
//	f32 sp174[2];
//	s32 sp15c[] = { 16, 32, 12, 32, 24, 64 }; // diameters?
//	s32 sp144[] = { 60, 80, 225, 275, 470, 570 }; // distances from the sun?
//
//	u32 colours[] = { // 12c
//		0xff99ffff, // pinkish/purple
//		0x9999ffff, // blue
//		0x99ffffff, // very light blue
//		0x99ff99ff, // green
//		0xffff99ff, // yellow
//		0xff9999ff, // red
//	};
//
//	f32 sp128;
//	f32 sp124;
//	s32 scale;
//	s32 i;
//	f32 f2;
//	f32 f12;
//	f32 f20;
//	f32 f0;
//
//	scale = 1;
//
//	if (g_ViRes == VIRES_HI) {
//		scale = 2;
//	}
//
//	sp128 = (x - viGetViewWidth() * 0.5f) * 0.01f;
//	sp124 = (y - viGetViewHeight() * 0.5f) * 0.01f;
//
//	// Render the sun
//	texSelect(&gdl, &g_TexLightGlareConfigs[6], 4, 0, 2, 1, NULL);
//
//	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
//	gDPSetColorDither(gdl++, G_CD_BAYER);
//	gDPSetAlphaDither(gdl++, G_AD_PATTERN);
//	gDPSetRenderMode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
//	gDPSetTexturePersp(gdl++, G_TP_NONE);
//	gDPSetAlphaCompare(gdl++, G_AC_NONE);
//	gDPSetTextureLOD(gdl++, G_TL_TILE);
//	gDPSetTextureConvert(gdl++, G_TC_FILT);
//	gDPSetTextureLUT(gdl++, G_TT_NONE);
//	gDPSetTextureFilter(gdl++, G_TF_BILERP);
//	gDPSetCombineLERP(gdl++,
//			0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
//			0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);
//
//	fa = (size * (0.5f + 0.5f * arg3) * (60.0f / viGetFovY()));
//
//	gDPSetEnvColor(gdl++, 0xff, 0xff, 0xff, (s32)(arg6 * arg3 * 255.0f));
//
//	sp17c[0] = x;
//	sp17c[1] = y;
//	sp174[1] = fa;
//	sp174[0] = fa * scale;
//
//	func0f0b2150(&gdl, sp17c, sp174, g_TexLightGlareConfigs[6].width, g_TexLightGlareConfigs[6].height, 0, 1, 1, 1, 0, 1);
//
//	// Render the artifacts
//	texSelect(&gdl, &g_TexLightGlareConfigs[1], 4, 0, 2, 1, NULL);
//
//	gDPSetCycleType(gdl++, G_CYC_1CYCLE);
//	gDPSetColorDither(gdl++, G_CD_BAYER);
//	gDPSetAlphaDither(gdl++, G_AD_PATTERN);
//	gDPSetRenderMode(gdl++, G_RM_AA_XLU_SURF, G_RM_AA_XLU_SURF2);
//	gDPSetTexturePersp(gdl++, G_TP_NONE);
//	gDPSetAlphaCompare(gdl++, G_AC_NONE);
//	gDPSetTextureLOD(gdl++, G_TL_TILE);
//	gDPSetTextureConvert(gdl++, G_TC_FILT);
//	gDPSetTextureLUT(gdl++, G_TT_NONE);
//	gDPSetTextureFilter(gdl++, G_TF_BILERP);
//	gDPSetCombineLERP(gdl++,
//			0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0,
//			0, 0, 0, ENVIRONMENT, TEXEL0, 0, ENVIRONMENT, 0);
//
//	for (i = 0; i < 6; i++) {
//		// 90c
//		if (arg5 < 90) {
//			if (arg5 < 30) {
//				f2 = arg5 * 0.033333335071802f;
//			} else {
//				f2 = 1.0f;
//			}
//		} else {
//			f2 = (180.0f - (arg5 - 90)) * 0.0055555556900799f * 0.5f;
//
//			if (f2 < 0.0f) {
//				f2 = 0.0f;
//			}
//
//			f2 += 0.5f;
//		}
//
//		// 974
//		fb = x - sp144[i] * sp128;
//		fc = y - sp144[i] * sp124;
//		fa = sp15c[i] * 0.5f;
//
//		gDPSetEnvColor(gdl++, colours[i] >> 24, (colours[i] >> 16) & 0xff, (colours[i] >> 8) & 0xff, (s32)((colours[i] & 0xff) * (arg6 * f2)));
//
//		sp17c[0] = fb;
//		sp17c[1] = fc;
//
//		sp174[1] = fa;
//		sp174[0] = fa * scale;
//
//		func0f0b2150(&gdl, sp17c, sp174, g_TexLightGlareConfigs[1].width, g_TexLightGlareConfigs[1].height, 0, 0, 0, 0, 0, 1);
//	}
//
//	f20 = viGetViewWidth() * .5f - x;
//	f0 = viGetViewHeight() * .5f - y;
//
//	f12 = (40.0f - sqrtf(f20 * f20 + f0 * f0)) * 0.0125f;
//
//	if (f12 < 0.0f) {
//		f12 = 0.0f;
//	}
//
//	f12 += 0.1f;
//
//	if (arg5 <= g_Vars.lvupdate240) {
//		f12 = 0.0f;
//	}
//
//	if (f12 > 0.0f) {
//		sky0f127334(arg6 * f12 * 255.0f, arg6 * f12 * 255.0f, arg6 * f12 * 255.0f);
//	}
//
//	gDPSetColorDither(gdl++, G_CD_BAYER);
//	gDPSetAlphaDither(gdl++, G_AD_PATTERN | G_CD_DISABLE);
//	gDPSetTexturePersp(gdl++, G_TP_PERSP);
//	gDPSetTextureLOD(gdl++, G_TL_LOD);
//
//	return gdl;
//}

/**
 * Render a sun and its artifacts if on screen.
 */
Gfx *sky0f126c3c(Gfx *gdl, f32 x, f32 y, f32 z, f32 arg4, f32 arg5)
{
	struct coord sp64;

	sp64.x = x;
	sp64.y = y;
	sp64.z = z;

	mtx4TransformVecInPlace(camGetWorldToScreenMtxf(), &sp64);
	mtx4TransformVecInPlace(camGetMtxF1754(), &sp64);

	if (sp64.z > 1.0f) {
		f32 xpos;
		f32 ypos;
		s16 viewlefti = viGetViewLeft();
		s16 viewtopi = viGetViewTop();
		s16 viewwidthi = viGetViewWidth();
		s16 viewheighti = viGetViewHeight();
		f32 viewleft = viewlefti;
		f32 viewwidth = viewwidthi;
		f32 viewtop = viewtopi;
		f32 viewheight = viewheighti;

		xpos = viewleft + (sp64.f[0] / sp64.f[2] + 1.0f) * 0.5f * viewwidth;
		ypos = viewtop + (-sp64.f[1] / sp64.f[2] + 1.0f) * 0.5f * viewheight;

		if (xpos >= viewleft && xpos < viewleft + viewwidth
				&& ypos >= viewtop && ypos < viewtop + viewheight) {
			gdl = sky0f126384(gdl, xpos, ypos, arg5, arg4, TICKS(90), 1.0f);
		}
	}

	return gdl;
}

/**
 * Render lens flares during teleport.
 */
Gfx *sky0f126de8(Gfx *gdl)
{
	f32 sp154 = var80061630 * M_BADTAU;
	s32 i;
	f32 f20 = 0.0f;
	f32 f20_2;
	f32 f22;
	f32 f22_3;
	struct pad pad;
	struct coord spe0;
	f32 spd0[4];
	Mtxf mtx;
	f32 f24;
	f32 f30;

	if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_PREENTER) {
		f20 = g_Vars.currentplayer->teleporttime / 24.0f * 0.33f;
	} else if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_ENTERING) {
		f20 = g_Vars.currentplayer->teleporttime / 48.0f * 0.66f + 0.33f;
	}

	f30 = f20 * 6.0f;
	f22 = f20 * 1.3f;

	if (f22 > 1.0f) {
		f22 = 1.0f;
	}

	if (f30 > 1.0f) {
		f30 = 1.0f;
	}

	f20 *= 1.7f;

	if (f20 > 1.0f) {
		f20 = 1.0f;
	}

	padUnpack(g_Vars.currentplayer->teleportpad, PADFIELD_POS | PADFIELD_LOOK | PADFIELD_UP, &pad);

	g_TeleportToPos.x = pad.pos.x;
	g_TeleportToPos.y = pad.pos.y;
	g_TeleportToPos.z = pad.pos.z;
	g_TeleportToLook.x = pad.look.x;
	g_TeleportToLook.y = pad.look.y;
	g_TeleportToLook.z = pad.look.z;
	g_TeleportToUp.x = pad.up.x;
	g_TeleportToUp.y = pad.up.y;
	g_TeleportToUp.z = pad.up.z;

	f22 = -cosf(f22 * M_PI) * 0.5f + .5f;
	f24 = 100 * f22;

	for (i = 0; i < 5; i++) {
		spe0.x = g_TeleportToLook.f[0] * f24;
		spe0.y = g_TeleportToLook.f[1] * f24;
		spe0.z = g_TeleportToLook.f[2] * f24;

		f22_3 = sp154 + i * 1.2564370632172f;
		f20_2 = sinf(f22_3);

		spd0[0] = cosf(f22_3);
		spd0[1] = g_TeleportToUp.f[0] * f20_2;
		spd0[2] = g_TeleportToUp.f[1] * f20_2;
		spd0[3] = g_TeleportToUp.f[2] * f20_2;

		quaternionToMtx(spd0, &mtx);
		mtx4RotateVecInPlace(&mtx, &spe0);

		spe0.x += g_TeleportToPos.x;
		spe0.y += g_TeleportToPos.y;
		spe0.z += g_TeleportToPos.z;

		gdl = sky0f126c3c(gdl, spe0.x, spe0.y, spe0.z, f20 * 200, f30);
	}

	return gdl;
}

/**
 * Render teleport artifacts, and all suns and their artifacts.
 */
Gfx *sky0f12715c(Gfx *gdl)
{
	struct environment *env = envGetCurrent();
	struct sun *sun;
	s32 i;

	if (g_Vars.currentplayer->teleportstate == TELEPORTSTATE_PREENTER
			|| g_Vars.currentplayer->teleportstate == TELEPORTSTATE_ENTERING) {
		gdl = sky0f126de8(gdl);
	}

	if (env->numsuns <= 0 || !var800844f0 || g_Vars.mplayerisrunning) {
		return gdl;
	}

	sun = env->suns;

	for (i = 0; i < env->numsuns; i++) {
		if (sun->lens_flare && g_SunPositions[i].z > 1) {
			struct bootbufferthing *thing = bbufGetIndex1Buffer();
			f32 value = sky0f125a1c(thing->unk00[i].unk00);

			if (value > 0.0f) {
				gdl = sky0f126384(gdl, g_SunScreenXPositions[i], g_SunScreenYPositions[i], value, sun->orb_size, g_SunFlareTimers240[i], g_SunAlphaFracs[i]);
			}
		}

		sun++;
	}

	return gdl;
}

void sky0f127334(s32 arg0, s32 arg1, s32 arg2)
{
	g_Vars.currentplayer->unk1c28 = sqrtf(g_Vars.currentplayer->unk1c28 * g_Vars.currentplayer->unk1c28 + arg0 * arg0);
	g_Vars.currentplayer->unk1c2c = sqrtf(g_Vars.currentplayer->unk1c2c * g_Vars.currentplayer->unk1c2c + arg1 * arg1);
	g_Vars.currentplayer->unk1c30 = sqrtf(g_Vars.currentplayer->unk1c30 * g_Vars.currentplayer->unk1c30 + arg2 * arg2);

	if (g_Vars.currentplayer->unk1c28 > 0xcc) {
		g_Vars.currentplayer->unk1c28 = 0xcc;
	}

	if (g_Vars.currentplayer->unk1c2c > 0xcc) {
		g_Vars.currentplayer->unk1c2c = 0xcc;
	}

	if (g_Vars.currentplayer->unk1c30 > 0xcc) {
		g_Vars.currentplayer->unk1c30 = 0xcc;
	}
}

s32 sky0f127490(s32 arg0, s32 arg1)
{
	if (arg1 >= arg0) {
		if (arg1 - arg0 > 8) {
			return arg0 + 8;
		}

		return arg1;
	}

	if (arg0 - arg1 > 8) {
		return arg0 - 8;
	}

	return arg1;
}

Gfx *sky0f1274d8(Gfx *gdl)
{
	s32 value;
	u32 stack;

	g_Vars.currentplayer->unk1c28 = sky0f127490(g_Vars.currentplayer->unk1c34, g_Vars.currentplayer->unk1c28);
	g_Vars.currentplayer->unk1c2c = sky0f127490(g_Vars.currentplayer->unk1c38, g_Vars.currentplayer->unk1c2c);
	g_Vars.currentplayer->unk1c30 = sky0f127490(g_Vars.currentplayer->unk1c3c, g_Vars.currentplayer->unk1c30);

	value = (g_Vars.currentplayer->unk1c28 > g_Vars.currentplayer->unk1c2c && g_Vars.currentplayer->unk1c28 > g_Vars.currentplayer->unk1c30)
		? g_Vars.currentplayer->unk1c28
		: g_Vars.currentplayer->unk1c2c > g_Vars.currentplayer->unk1c30
		? g_Vars.currentplayer->unk1c2c
		: g_Vars.currentplayer->unk1c30;

	if (!g_InCutscene && EYESPYINACTIVE() && value > 0) {
		f32 r = g_Vars.currentplayer->unk1c28 * (255.0f / value);
		f32 g = g_Vars.currentplayer->unk1c2c * (255.0f / value);
		f32 b = g_Vars.currentplayer->unk1c30 * (255.0f / value);

		f32 a = (g_Vars.currentplayer->unk1c28
			+ g_Vars.currentplayer->unk1c2c
			+ g_Vars.currentplayer->unk1c30) * (1.0f / 3.0f);

		gDPSetTexturePersp(gdl++, G_TP_NONE);
		gDPSetColorDither(gdl++, G_CD_DISABLE);
		gDPSetRenderMode(gdl++, G_RM_CLD_SURF, G_RM_CLD_SURF2);
		gDPSetCombineMode(gdl++, G_CC_PRIMITIVE, G_CC_PRIMITIVE);

		if (USINGDEVICE(DEVICE_NIGHTVISION)) {
			r *= 0.5f;
			g *= 0.75f;
			b *= 0.5f;
		} else if (USINGDEVICE(DEVICE_IRSCANNER)) {
			r *= 0.75f;
			g *= 0.5f;
			b *= 0.5f;
		}

		gDPSetPrimColor(gdl++, 0, 0, (s32)r, (s32)g, (s32)b, (s32)a);

		gDPFillRectangle(gdl++,
				viGetViewLeft(),
				viGetViewTop(),
				viGetViewLeft() + viGetViewWidth(),
				viGetViewTop() + viGetViewHeight());

		gDPPipeSync(gdl++);
	}

	gDPSetColorDither(gdl++, G_CD_BAYER);
	gDPSetTexturePersp(gdl++, G_TP_PERSP);

	g_Vars.currentplayer->unk1c34 = g_Vars.currentplayer->unk1c28;
	g_Vars.currentplayer->unk1c38 = g_Vars.currentplayer->unk1c2c;
	g_Vars.currentplayer->unk1c3c = g_Vars.currentplayer->unk1c30;
	g_Vars.currentplayer->unk1c28 = 0;
	g_Vars.currentplayer->unk1c2c = 0;
	g_Vars.currentplayer->unk1c30 = 0;

	return gdl;
}
