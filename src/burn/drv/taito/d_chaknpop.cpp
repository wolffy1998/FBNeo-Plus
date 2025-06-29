// FB Alpha Chack'n Pop driver module
// Based on MAME driver by BUT

#include "tiles_generic.h"
#include "taito_m68705.h"
#include "z80_intf.h"
#include "ay8910.h"

static UINT8 *AllMem;
static UINT8 *RamEnd;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvMcuROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvColPROM;
static UINT8 *DrvZ80RAM;
static UINT8 *DrvRAM;
static UINT8 *DrvMcuRAM;
static UINT8 *DrvTxtRAM;
static UINT8 *DrvSprRAM;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *gfxmode;
static UINT8 *gfxflip;
static UINT8 *rambank;

static UINT8 DrvInputs[3];
static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[3];
static UINT8 DrvReset;

static struct BurnInputInfo ChaknpopInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 start"	},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy2 + 3,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy2 + 2,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy2 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p1 fire 2"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvJoy1 + 3,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 5,	"p2 start"	},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy3 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy3 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p2 fire 2"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,	"reset"		},
	{"Service",		BIT_DIGITAL,	DrvJoy1 + 6,	"service"	},
	{"Tilt",		BIT_DIGITAL,	DrvJoy1 + 7,	"tilt"		},
	{"Dip A",		BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Chaknpop)

static struct BurnDIPInfo ChaknpopDIPList[]=
{
	{0x13, 0xff, 0xff, 0x00, NULL			},
	{0x14, 0xff, 0xff, 0x4f, NULL			},
	{0x15, 0xff, 0xff, 0x7d, NULL			},

	{0   , 0xfe, 0   ,    16, "Coin A"		},
	{0x13, 0x01, 0x0f, 0x0f, "9 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x0e, "8 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x0d, "7 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x0c, "6 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x0b, "5 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x0a, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x09, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x08, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x0f, 0x00, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x0f, 0x01, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x0f, 0x02, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x0f, 0x03, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x0f, 0x04, "1 Coin  5 Credits"	},
	{0x13, 0x01, 0x0f, 0x05, "1 Coin  6 Credits"	},
	{0x13, 0x01, 0x0f, 0x06, "1 Coin  7 Credits"	},
	{0x13, 0x01, 0x0f, 0x07, "1 Coin  8 Credits"	},

	{0   , 0xfe, 0   ,    16, "Coin B"		},
	{0x13, 0x01, 0xf0, 0xf0, "9 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0xe0, "8 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0xd0, "7 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0xc0, "6 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0xb0, "5 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0xa0, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x90, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x80, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0xf0, 0x00, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0xf0, 0x10, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0xf0, 0x20, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0xf0, 0x30, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0xf0, 0x40, "1 Coin  5 Credits"	},
	{0x13, 0x01, 0xf0, 0x50, "1 Coin  6 Credits"	},
	{0x13, 0x01, 0xf0, 0x60, "1 Coin  7 Credits"	},
	{0x13, 0x01, 0xf0, 0x70, "1 Coin  8 Credits"	},

	{0   , 0xfe, 0   ,    4, "Bonus Life"		},
	{0x14, 0x01, 0x03, 0x00, "80k and every 100k"	},
	{0x14, 0x01, 0x03, 0x01, "60k and every 100k"	},
	{0x14, 0x01, 0x03, 0x02, "40k and every 100k"	},
	{0x14, 0x01, 0x03, 0x03, "20k and every 100k"	},

	{0   , 0xfe, 0   ,    2, "Free Play"		},
	{0x14, 0x01, 0x04, 0x04, "Off"			},
	{0x14, 0x01, 0x04, 0x00, "On"			},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x14, 0x01, 0x18, 0x00, "6"			},
	{0x14, 0x01, 0x18, 0x08, "3"			},
	{0x14, 0x01, 0x18, 0x10, "2"			},
	{0x14, 0x01, 0x18, 0x18, "1"			},

	{0   , 0xfe, 0   ,    2, "Training/Difficulty"	},
	{0x14, 0x01, 0x20, 0x20, "Off/Every 10 Min."	},
	{0x14, 0x01, 0x20, 0x00, "On/Every 7 Min."	},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
//	{0x14, 0x01, 0x40, 0x40, "Off"			},
//	{0x14, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Cabinet"		},
	{0x14, 0x01, 0x80, 0x00, "Upright"		},
	{0x14, 0x01, 0x80, 0x80, "Cocktail"		},

	{0   , 0xfe, 0   ,    2, "Language"		},
	{0x15, 0x01, 0x01, 0x00, "English"		},
	{0x15, 0x01, 0x01, 0x01, "Japanese"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x15, 0x01, 0x02, 0x02, "Off"			},
	{0x15, 0x01, 0x02, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Super Chack'n"	},
	{0x15, 0x01, 0x04, 0x04, "pi"			},
	{0x15, 0x01, 0x04, 0x00, "1st Chance"		},

	{0   , 0xfe, 0   ,    2, "Endless (Cheat)"	},
	{0x15, 0x01, 0x08, 0x08, "Off"			},
	{0x15, 0x01, 0x08, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Credit Info"		},
	{0x15, 0x01, 0x10, 0x00, "Off"			},
	{0x15, 0x01, 0x10, 0x10, "On"			},

	{0   , 0xfe, 0   ,    2, "Show Year"		},
	{0x15, 0x01, 0x20, 0x00, "Off"			},
	{0x15, 0x01, 0x20, 0x20, "On"			},

	{0   , 0xfe, 0   ,    2, "Infinite (Cheat)"	},
	{0x15, 0x01, 0x40, 0x40, "Off"			},
	{0x15, 0x01, 0x40, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Coinage"		},
	{0x15, 0x01, 0x80, 0x00, "1 Way"		},
	{0x15, 0x01, 0x80, 0x80, "2 Way"		},
};

STDDIPINFO(Chaknpop)

static UINT8 __fastcall chaknpop_read(UINT16 address)
{
	switch (address)
	{
		case 0x8800:
			return standard_taito_mcu_read();

		case 0x8801: {
			INT32 res = 0;
			if (!main_sent) res |= 0x01;
			if (mcu_sent) res |= 0x02;
			return res;
		}
			
		case 0x8802:
			return 0xff;

		case 0x8804:
		case 0x8805:
			return AY8910Read(0);

		case 0x8806:
		case 0x8807:
			return AY8910Read(1);

		case 0x8808:
			return DrvDips[2];

		case 0x8809:
			return DrvInputs[1];

		case 0x880a:
			return DrvInputs[0];

		case 0x880b:
			return DrvInputs[2];

		case 0x880c:
			return gfxmode[0];
	}

	return 0;
}

static void bankswitch(INT32 bank)
{
	if (rambank[0] != bank) {
		rambank[0] = bank;

		bank = bank ? 0x4000 : 0;
	
		ZetMapArea(0xc000, 0xffff, 0, DrvZ80RAM + bank);
		ZetMapArea(0xc000, 0xffff, 1, DrvZ80RAM + bank);
		ZetMapArea(0xc000, 0xffff, 2, DrvZ80RAM + bank);
	}
}

static void __fastcall chaknpop_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x8800:
			standard_taito_mcu_write(data);
		return;

		case 0x8801:
		case 0x8802: // MCU port b & c
		return;

		case 0x8804:
		case 0x8805:
			AY8910Write(0, address & 1, data);
		return;

		case 0x8806:
		case 0x8807:
			AY8910Write(1, address & 1, data);
		return;

		case 0x880c: {
			gfxflip[0] = data & 0x01;
			gfxflip[1] = data & 0x02;
			bankswitch(data & 0x04);
			gfxmode[0] = data;
		}
		return;

		case 0x880d: // coin lockout
		return;
	}
}

static UINT8 ay8910_0_read_port_A(UINT32)
{
	return DrvDips[0];
}

static UINT8 ay8910_0_read_port_B(UINT32)
{
	return DrvDips[1];
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	ZetOpen(0);
	ZetReset();
	ZetClose();
	
	m67805_taito_reset();

	AY8910Reset(0);
	AY8910Reset(1);

	rambank[0] = 0xff;
	ZetOpen(0);
	bankswitch(0);
	ZetClose();

	HiscoreReset();

	return 0;
}

static void DrvPaletteInit()
{
	for (INT32 i = 0; i < 1024; i++)
	{
		INT32 bit0, bit1, bit2;

		INT32 col = (DrvColPROM[i] & 0x0f) + ((DrvColPROM[i + 1024] & 0x0f) << 4);

		bit0 = (col >> 0) & 0x01;
		bit1 = (col >> 1) & 0x01;
		bit2 = (col >> 2) & 0x01;
		INT32 r = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = (col >> 3) & 0x01;
		bit1 = (col >> 4) & 0x01;
		bit2 = (col >> 5) & 0x01;
		INT32 g = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		bit0 = 0;
		bit1 = (col >> 6) & 0x01;
		bit2 = (col >> 7) & 0x01;
		INT32 b = 0x21 * bit0 + 0x47 * bit1 + 0x97 * bit2;

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static INT32 DrvGfxDecode()
{
	INT32 Plane[2]  = { 0, 0x2000*8 };
	INT32 XOffs[16] = { 0, 1, 2, 3, 4, 5, 6, 7 , 8*8+0, 8*8+1, 8*8+2, 8*8+3, 8*8+4, 8*8+5, 8*8+6, 8*8+7 };
	INT32 YOffs[16] = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 16*8, 17*8, 18*8, 19*8, 20*8, 21*8, 22*8, 23*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x4000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x4000);

	GfxDecode(0x0100, 2, 16, 16, Plane, XOffs, YOffs, 0x100, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x4000);

	GfxDecode(0x0400, 2,  8,  8, Plane, XOffs, YOffs, 0x040, tmp, DrvGfxROM1);

	BurnFree (tmp);

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvZ80ROM		= Next; Next += 0x00c000;

	DrvMcuROM		= Next; Next += 0x000800;

	DrvGfxROM0		= Next; Next += 0x010000;
	DrvGfxROM1		= Next; Next += 0x010000;

	DrvColPROM		= Next; Next += 0x000800;

	DrvPalette		= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);

	AllRam			= Next;

	DrvRAM			= Next; Next += 0x000800;
	DrvZ80RAM		= Next; Next += 0x008000;
	DrvTxtRAM		= Next; Next += 0x000400;
	DrvSprRAM		= Next; Next += 0x000100;
	DrvMcuRAM		= Next; Next += 0x000800;

	gfxflip			= Next; Next += 0x000002;
	gfxmode			= Next; Next += 0x000001;
	rambank			= Next; Next += 0x000001;

	RamEnd			= Next;

	MemEnd			= Next;

	return 0;
}

static INT32 DrvInit()
{
	AllMem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((AllMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(AllMem, 0, nLen);
	MemIndex();

	{
		if (BurnLoadRom(DrvZ80ROM  + 0x0000,  0, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM  + 0x2000,  1, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM  + 0x4000,  2, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM  + 0x6000,  3, 1)) return 1;
		if (BurnLoadRom(DrvZ80ROM  + 0xa000,  4, 1)) return 1;

		if (BurnLoadRom(DrvMcuROM  + 0x0000,  5, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x0000,  6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x2000,  7, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x0000,  8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x2000,  9, 1)) return 1;

		if (BurnLoadRom(DrvColPROM + 0x0000, 10, 1)) return 1;
		if (BurnLoadRom(DrvColPROM + 0x0400, 11, 1)) return 1;

		DrvGfxDecode();
		DrvPaletteInit();
	}

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvRAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvRAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvRAM);
	ZetMapArea(0x9000, 0x93ff, 0, DrvTxtRAM);
	ZetMapArea(0x9000, 0x93ff, 1, DrvTxtRAM);
	ZetMapArea(0x9000, 0x93ff, 2, DrvTxtRAM);
	ZetMapArea(0x9800, 0x98ff, 0, DrvSprRAM);
	ZetMapArea(0x9800, 0x98ff, 1, DrvSprRAM);
	ZetMapArea(0x9800, 0x98ff, 2, DrvSprRAM);
	ZetMapArea(0xa000, 0xbfff, 0, DrvZ80ROM + 0xa000);
	ZetMapArea(0xa000, 0xbfff, 2, DrvZ80ROM + 0xa000);
	bankswitch(0);
	ZetSetWriteHandler(chaknpop_write);
	ZetSetReadHandler(chaknpop_read);
	ZetClose();
	
	m67805_taito_init(DrvMcuROM, DrvMcuRAM, &standard_m68705_interface);

	AY8910Init(0, 1536000, 0);
	AY8910Init(1, 1536000, 1);
	AY8910SetPorts(0, &ay8910_0_read_port_A, &ay8910_0_read_port_B, NULL, NULL);
	AY8910SetAllRoutes(0, 0.15, BURN_SND_ROUTE_BOTH);
	AY8910SetAllRoutes(1, 0.10, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();
	ZetExit();
	m67805_taito_exit();
	AY8910Exit(0);
	AY8910Exit(1);

	BurnFree (AllMem);

	return 0;
}

static void draw_sprites()
{
	for (INT32 offs = 0; offs < 0xc0; offs += 4)
	{
		INT32 sy    = 241 - DrvSprRAM[offs + 0x40];
		INT32 attr  = DrvSprRAM[offs + 0x41];
		INT32 color = DrvSprRAM[offs + 0x42] & 0x07;
		INT32 sx    = DrvSprRAM[offs + 0x43];
		INT32 flipx = attr & 0x40;
		INT32 flipy = attr & 0x80;
		INT32 code  = (attr & 0x3f) | ((DrvSprRAM[offs + 0x42] & 0x38) << 3);

		if (gfxflip[0]) {
			sx = 240 - sx;
			flipx = !flipx;
		}

		if (gfxflip[1]) {
			sy = 242 - sy;
			flipy = !flipy;
		}

		if (flipy) {
			if (flipx) {
				Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, code, sx, sy - 16, color, 2, 0, 0, DrvGfxROM0);
			} else {
				Render16x16Tile_Mask_FlipY_Clip(pTransDraw, code, sx, sy - 16, color, 2, 0, 0, DrvGfxROM0);
			}
		} else {
			if (flipx) {
				Render16x16Tile_Mask_FlipX_Clip(pTransDraw, code, sx, sy - 16, color, 2, 0, 0, DrvGfxROM0);
			} else {
				Render16x16Tile_Mask_Clip(pTransDraw, code, sx, sy - 16, color, 2, 0, 0, DrvGfxROM0);
			}
		}
	}
}

static void draw_layer()
{
	for (INT32 offs = 32 * 2; offs < 32 * 32 - 32 * 2; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = ((offs >> 5) << 3) - 16;

		INT32 code  = DrvTxtRAM[offs];
		INT32 color = (code == 0x74) ? DrvSprRAM[0x0b] : DrvSprRAM[0x01];

		if (gfxmode[0] & 0x20 && code >= 0xc0) code += 0xc0;

		code |= (gfxmode[0] & 0x80) << 2;

		if (gfxflip[1]) sy = ((sy+16)^0xff)-16;
		if (gfxflip[0]) sx ^= 0xff;

		if (gfxflip[1]) {
			if (gfxflip[0]) {
				Render8x8Tile_FlipXY_Clip(pTransDraw, code, sx, sy, color+8, 2, 0, DrvGfxROM1);
			} else {
				Render8x8Tile_FlipY_Clip(pTransDraw, code, sx, sy, color+8, 2, 0, DrvGfxROM1);
			}
		} else {
			if (gfxflip[0]) {
				Render8x8Tile_FlipX_Clip(pTransDraw, code, sx, sy, color+8, 2, 0, DrvGfxROM1);
			} else {
				Render8x8Tile_Clip(pTransDraw, code, sx, sy, color+8, 2, 0, DrvGfxROM1);
			}
		}
	}
}

static void draw_bitmap()
{
	INT32 dx = gfxflip[0] ? -1 : 1;

	for (INT32 offs = 0+(16*32); offs < 0x2000-(16*32); offs++)
	{
		INT32 x = ((offs & 0x1f) << 3) + 7;
		INT32 y = (offs >> 5) + 16;

		if (!gfxflip[0]) x = 255 - x;
		if (!gfxflip[1]) y = 255 - y;

		if (y < 0 || y >= nScreenHeight) continue;

		for (INT32 i = 0x80; i > 0; i >>= 1, x += dx)
		{
			if (x >= nScreenWidth || x < 0) continue;

			UINT32 color = 0;

			if (DrvZ80RAM[0x0000+offs] & i) color |= 0x200;
			if (DrvZ80RAM[0x4000+offs] & i) color |= 0x100;
			if (DrvZ80RAM[0x2000+offs] & i) color |= 0x080;
			if (DrvZ80RAM[0x6000+offs] & i) color |= 0x040;

			if (color) {
				pTransDraw[y * nScreenWidth + x] |= color;
			}
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvPaletteInit();
		DrvRecalc = 0;
	}

	BurnTransferClear();
	if (nBurnLayer & 1) draw_layer();
	if (nBurnLayer & 2) draw_sprites();
	if (nBurnLayer & 4) draw_bitmap();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 3);

		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
		}
	}
	
	INT32 nInterleave = 100;
	INT32 nCyclesTotal[2] = { 2350000 / 60, 3072000 / 60 };
	INT32 nCyclesDone[2]  = { 0, 0 };
	
	ZetNewFrame();
	
	for (INT32 i = 0; i < nInterleave; i++)
	{
		INT32 nSegment = nCyclesTotal[0] / nInterleave;

		ZetOpen(0);
		nCyclesDone[0] += ZetRun(nSegment);
		if (i == (nInterleave - 1)) ZetSetIRQLine(0, CPU_IRQSTATUS_AUTO);
		ZetClose();
		
		m6805Open(0);
		nSegment = nCyclesTotal[1] / nInterleave;
		nCyclesDone[1] += m6805Run(nSegment);
		m6805Close();
	}
	
	if (pBurnSoundOut) {
		AY8910Render(pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029702;
	}

	if (nAction & ACB_VOLATILE) {
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		ZetScan(nAction);
		m68705_taito_scan(nAction);
		AY8910Scan(nAction, pnMin);
	}

	if (nAction & ACB_WRITE) {
		INT32 bank = rambank[0];
		rambank[0] = 0xff;
		ZetOpen(0);
		bankswitch(bank);
		ZetClose();
	}

	return 0;
}


// Chack'n Pop

static struct BurnRomInfo chaknpopRomDesc[] = {
	{ "ao4_01.ic28",	0x2000, 0x386fe1c8, 1 | BRF_ESS | BRF_PRG }, //  0 Z80 code
	{ "ao4_02.ic27",	0x2000, 0x5562a6a7, 1 | BRF_ESS | BRF_PRG }, //  1
	{ "ao4_03.ic26",	0x2000, 0x3e2f0a9c, 1 | BRF_ESS | BRF_PRG }, //  2
	{ "ao4_04.ic25",	0x2000, 0x5209c7d4, 1 | BRF_ESS | BRF_PRG }, //  3
	{ "ao4_05.ic3",		0x2000, 0x8720e024, 1 | BRF_ESS | BRF_PRG }, //  4

	{ "ao4_06.ic23",	0x0800, 0x9c78c24c, 2 | BRF_ESS | BRF_PRG }, //  5 m68705 code

	{ "ao4_08.ic14",	0x2000, 0x5575a021, 3 | BRF_GRA },           //  6 Sprites
	{ "ao4_07.ic15",	0x2000, 0xae687c18, 3 | BRF_GRA },           //  7

	{ "ao4_09.ic98",	0x2000, 0x757a723a, 4 | BRF_GRA },           //  8 Tiles
	{ "ao4_10.ic97",	0x2000, 0x3e3fd608, 4 | BRF_GRA },           //  9

	{ "ao4-11.ic96",	0x0400, 0x9bf0e85f, 5 | BRF_GRA },           // 10 Color PROMs
	{ "ao4-12.ic95",	0x0400, 0x954ce8fc, 5 | BRF_GRA },           // 11
};

STD_ROM_PICK(chaknpop)
STD_ROM_FN(chaknpop)

struct BurnDriver BurnDrvChaknpop = {
	"chaknpop", NULL, NULL, NULL, "1983",
	"Chack'n Pop\0", NULL, "Taito Corporation", "Miscellaneous",
	L"\u5C0F\u9E21\u5927\u4F5C\u6218\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_MISC, GBF_MAZE, 0,
	NULL, chaknpopRomInfo, chaknpopRomName, NULL, NULL, NULL, NULL, ChaknpopInputInfo, ChaknpopDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x400,
	256, 224, 4, 3
};
