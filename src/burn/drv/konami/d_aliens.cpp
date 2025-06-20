// FB Alpha Aliens driver module
// Based on MAME driver by Manuel Abadia

#include "tiles_generic.h"
#include "z80_intf.h"
#include "konami_intf.h"
#include "konamiic.h"
#include "burn_ym2151.h"
#include "k007232.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvKonROM;
static UINT8 *DrvZ80ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROMExp0;
static UINT8 *DrvGfxROMExp1;
static UINT8 *DrvSndROM;
static UINT8 *DrvBankRAM;
static UINT8 *DrvKonRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvZ80RAM;
static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *soundlatch;
static UINT8 *nDrvRamBank;
static UINT8 *nDrvKonamiBank;

static INT32 nCyclesExtra;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvDips[3];
static UINT8 DrvInputs[2];
static UINT8 DrvReset;

static struct BurnInputInfo AliensInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy1 + 6,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",			BIT_DIGITAL,	DrvJoy2 + 6,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Service",			BIT_DIGITAL,	DrvJoy3 + 4,	"service"	},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",			BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Aliens)

static struct BurnDIPInfo AliensDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL			},
	{0x13, 0xff, 0xff, 0x5e, NULL			},
	{0x14, 0xff, 0xff, 0xff, NULL			},

	{0   , 0xfe, 0   ,    16, "Coin A"		},
	{0x12, 0x01, 0x0f, 0x02, "4 Coins 1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x05, "3 Coins 1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x08, "2 Coins 1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x04, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0x0f, 0x01, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit"	  },
	{0x12, 0x01, 0x0f, 0x03, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0x0f, 0x07, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0x0f, 0x06, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0x0f, 0x09, "1 Coin  7 Credits"	},
	{0x12, 0x01, 0x0f, 0x00, "Free Play"		},

	{0   , 0xfe, 0   ,    16, "Coin B"		},
	{0x12, 0x01, 0xf0, 0x20, "4 Coins 1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x50, "3 Coins 1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x80, "2 Coins 1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x40, "3 Coins 2 Credits"	},
	{0x12, 0x01, 0xf0, 0x10, "4 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit"	  },
	{0x12, 0x01, 0xf0, 0x30, "3 Coins 4 Credits"	},
	{0x12, 0x01, 0xf0, 0x70, "2 Coins 3 Credits"	},
	{0x12, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"	},
	{0x12, 0x01, 0xf0, 0x60, "2 Coins 5 Credits"	},
	{0x12, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"	},
	{0x12, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"	},
	{0x12, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"	},
	{0x12, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"	},
	{0x12, 0x01, 0xf0, 0x90, "1 Coin  7 Credits"	},
	{0x12, 0x01, 0xf0, 0x00, "No Credits"		},

	{0   , 0xfe, 0   ,    4, "Lives"		},
	{0x13, 0x01, 0x03, 0x03, "1"			},
	{0x13, 0x01, 0x03, 0x02, "2"			},
	{0x13, 0x01, 0x03, 0x01, "3"			},
	{0x13, 0x01, 0x03, 0x00, "5"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"		},
	{0x13, 0x01, 0x60, 0x60, "Easy"			},
	{0x13, 0x01, 0x60, 0x40, "Normal"		},
	{0x13, 0x01, 0x60, 0x20, "Hard"			},
	{0x13, 0x01, 0x60, 0x00, "Very Hard"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"		},
	{0x13, 0x01, 0x80, 0x80, "Off"			},
	{0x13, 0x01, 0x80, 0x00, "On"			},

//	{0   , 0xfe, 0   ,    2, "Flip Screen"		},
//	{0x14, 0x01, 0x01, 0x01, "Off"			},
//	{0x14, 0x01, 0x01, 0x00, "On"			},

	{0   , 0xfe, 0   ,    2, "Service Mode"		},
	{0x14, 0x01, 0x04, 0x04, "Off"			},
	{0x14, 0x01, 0x04, 0x00, "On"			},
};

STDDIPINFO(Aliens)

static void set_ram_bank(INT32 data)
{
	nDrvRamBank[0] = data;

	if (data & 0x20) {
		konamiMapMemory(DrvPalRAM,  0x0000, 0x03ff, MAP_RAM);
	} else {
		konamiMapMemory(DrvBankRAM, 0x0000, 0x03ff, MAP_RAM);
	}
}

static void aliens_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x5f88:
			set_ram_bank(data & 0x20);
			K052109RMRDLine = data & 0x40;
		return;

		case 0x5f8c:
			*soundlatch = data;
			ZetSetVector(0xff);
			ZetSetIRQLine(0, CPU_IRQSTATUS_ACK);
		return;
	}

	if ((address & 0xc000) == 0x4000) {
		K052109_051960_w(address & 0x3fff, data);
		return;
	}
}

static UINT8 aliens_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x5f80:
			return DrvDips[2];

		case 0x5f81:
			return DrvInputs[0];

		case 0x5f82:
			return DrvInputs[1];

		case 0x5f83:
			return DrvDips[1];

		case 0x5f84:
			return DrvDips[0];

		case 0x5f88:
			// watchdog reset
			return 0;
	}

	if ((address & 0xc000) == 0x4000) {
		return K052109_051960_r(address & 0x3fff);
	}

	return 0;
}

static void __fastcall aliens_sound_write(UINT16 address, UINT8 data)
{
	if ((address & 0xfff0) == 0xe000) {
		K007232WriteReg(0, address & 0x0f, data);
		return;
	}

	switch (address)
	{
		case 0xa000:
			BurnYM2151SelectRegister(data);
		return;

		case 0xa001:
			BurnYM2151WriteRegister(data);
		return;
	}
}

static UINT8 __fastcall aliens_sound_read(UINT16 address)
{
	if ((address & 0xfff0) == 0xe000) {
		return K007232ReadReg(0, address & 0x0f);
	}

	switch (address)
	{
		case 0xa000:
		case 0xa001:
			return BurnYM2151Read();

		case 0xc000:
			ZetSetIRQLine(0, CPU_IRQSTATUS_NONE);
			return *soundlatch;
	}

	return 0;
}

static void DrvYM2151WritePort(UINT32, UINT32 data)
{
	INT32 bank_A = ((data >> 1) & 0x01);
	INT32 bank_B = ((data) & 0x01);

	k007232_set_bank(0, bank_A, bank_B );
}

static void DrvK007232VolCallback(INT32 v)
{
	K007232SetVolume(0, 0, (v >> 0x4) * 0x11, 0);
	K007232SetVolume(0, 1, 0, (v & 0x0f) * 0x11);
}

static void aliens_set_lines(INT32 lines)
{
	nDrvKonamiBank[0] = lines;

	INT32 nBank = (lines & 0x1f) * 0x2000;

	konamiMapMemory(DrvKonROM + 0x10000 + nBank, 0x2000, 0x3fff, MAP_ROM); 
}

static void K052109Callback(INT32 layer, INT32 bank, INT32 *code, INT32 *color, INT32 *, INT32 *)
{
	*code |= ((*color & 0x3f) << 8) | (bank << 14);
	*code &= 0xffff;
	*color = (layer << 2) + ((*color & 0xc0) >> 6);
}

static void K051960Callback(INT32 *code, INT32 *color,INT32 *priority, INT32 *shadow)
{
	switch (*color & 0x70)
	{
		case 0x10: *priority = 0x00; break; 
		case 0x00: *priority = 0xf0; break;
		case 0x40: *priority = 0xfc; break;
		case 0x20:
		case 0x60: *priority = 0xfe; break;
		case 0x50: *priority = 0xcc; break;
		case 0x30:
		case 0x70: *priority = 0xee; break;
	}

	*code |= (*color & 0x80) << 6;
	*code &= 0x3fff;
	*color = 16 + (*color & 0x0f);
	*shadow = 0;
}

static INT32 DrvDoReset()
{
	DrvReset = 0;

	memset (AllRam, 0, RamEnd - AllRam);

	konamiOpen(0);
	konamiReset();
	konamiClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	K007232Reset(0);
	BurnYM2151Reset();

	KonamiICReset();

	nCyclesExtra = 0;

	HiscoreReset();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvKonROM		= Next; Next += 0x040000;
	DrvZ80ROM		= Next; Next += 0x010000;

	DrvGfxROM0		= Next; Next += 0x200000;
	DrvGfxROM1		= Next; Next += 0x200000;
	DrvGfxROMExp0	= Next; Next += 0x400000;
	DrvGfxROMExp1	= Next; Next += 0x400000;

	DrvSndROM		= Next; Next += 0x040000;

	DrvPalette		= (UINT32*)Next; Next += 0x200 * sizeof(UINT32);

	AllRam			= Next;

	DrvBankRAM		= Next; Next += 0x000400;
	DrvKonRAM		= Next; Next += 0x001c00;
	DrvPalRAM		= Next; Next += 0x000400;

	DrvZ80RAM		= Next; Next += 0x000800;

	soundlatch		= Next; Next += 0x000001;

	nDrvRamBank		= Next; Next += 0x000001;
	nDrvKonamiBank	= Next; Next += 0x000001;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static INT32 DrvInit()
{
	GenericTilesInit();

	BurnAllocMemIndex();

	{
		if (BurnLoadRom(DrvKonROM  + 0x030000,  0, 1)) return 1;
		if (BurnLoadRom(DrvKonROM  + 0x010000,  1, 1)) return 1;
		memcpy (DrvKonROM + 0x08000, DrvKonROM + 0x38000, 0x8000);

		if (BurnLoadRom(DrvZ80ROM  + 0x000000,  2, 1)) return 1;

		if (BurnLoadRomExt(DrvGfxROM0 + 0x000000,  3, 4, LD_GROUP(2))) return 1;
		if (BurnLoadRomExt(DrvGfxROM0 + 0x000002,  4, 4, LD_GROUP(2))) return 1;
		if (BurnLoadRomExt(DrvGfxROM0 + 0x100000,  5, 4, LD_GROUP(2))) return 1;
		if (BurnLoadRomExt(DrvGfxROM0 + 0x100002,  6, 4, LD_GROUP(2))) return 1;

		if (BurnLoadRomExt(DrvGfxROM1 + 0x000000,  7, 4, LD_GROUP(2))) return 1;
		if (BurnLoadRomExt(DrvGfxROM1 + 0x000002,  8, 4, LD_GROUP(2))) return 1;
		if (BurnLoadRomExt(DrvGfxROM1 + 0x100000,  9, 4, LD_GROUP(2))) return 1;
		if (BurnLoadRomExt(DrvGfxROM1 + 0x100002, 10, 4, LD_GROUP(2))) return 1;

		if (BurnLoadRom(DrvSndROM  + 0x000000, 11, 1)) return 1;

		K052109GfxDecode(DrvGfxROM0, DrvGfxROMExp0, 0x200000);
		K051960GfxDecode(DrvGfxROM1, DrvGfxROMExp1, 0x200000);
	}

	konamiInit(0);
	konamiOpen(0);
	konamiMapMemory(DrvBankRAM,          0x0000, 0x03ff, MAP_RAM);
	konamiMapMemory(DrvKonRAM,           0x0400, 0x1fff, MAP_RAM);
	konamiMapMemory(DrvKonROM + 0x10000, 0x2000, 0x3fff, MAP_ROM);
	konamiMapMemory(DrvKonROM + 0x08000, 0x8000, 0xffff, MAP_ROM);
	konamiSetWriteHandler(aliens_main_write);
	konamiSetReadHandler(aliens_main_read);
	konamiSetlinesCallback(aliens_set_lines);
	konamiClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80ROM);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80ROM);
	ZetMapArea(0x8000, 0x87ff, 0, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 1, DrvZ80RAM);
	ZetMapArea(0x8000, 0x87ff, 2, DrvZ80RAM);
	ZetSetWriteHandler(aliens_sound_write);
	ZetSetReadHandler(aliens_sound_read);
	ZetClose();

	K052109Init(DrvGfxROM0, DrvGfxROMExp0, 0x1fffff);
	K052109SetCallback(K052109Callback);
	K052109AdjustScroll(8, 0);

	K051960Init(DrvGfxROM1, DrvGfxROMExp1, 0x1fffff);
	K051960SetCallback(K051960Callback);
	K051960SetSpriteOffset(8, 0);

	BurnYM2151InitBuffered(3579545, 1, NULL, 0);
	BurnYM2151SetPortHandler(&DrvYM2151WritePort);
	BurnYM2151SetAllRoutes(0.60, BURN_SND_ROUTE_BOTH);
	BurnTimerAttachZet(3579545);

	K007232Init(0, 3579545, DrvSndROM, 0x40000);
	K007232SetPortWriteHandler(0, DrvK007232VolCallback);
	K007232PCMSetAllRoutes(0, 0.20, BURN_SND_ROUTE_BOTH);

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	KonamiICExit();

	konamiExit();
	ZetExit();

	K007232Exit();
	BurnYM2151Exit();

	BurnFreeMemIndex();

	return 0;
}

static INT32 DrvDraw()
{
	KonamiRecalcPalette(DrvPalRAM, DrvPalette, 0x400);

	K052109UpdateScroll();

	KonamiClearBitmaps(DrvPalette[0x0040]);

	if (nBurnLayer & 1) K052109RenderLayer(1, 0, 1);
	if (nBurnLayer & 2) K052109RenderLayer(2, 0, 2);
	if (nBurnLayer & 4) K052109RenderLayer(0, 0, 4);

	if (nSpriteEnable & 1) K051960SpritesRender(-1, -1);

	KonamiBlendCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, sizeof ( DrvInputs ));

		for (INT32 i = 0; i < 8; i++) 
		{
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvDips[2] ^= (DrvJoy3[i] & 1) << i;
		}

		// Clear opposites
		if ((DrvInputs[0] & 0x03) == 0) DrvInputs[0] |= 0x03;
		if ((DrvInputs[0] & 0x0c) == 0) DrvInputs[0] |= 0x0c;
		if ((DrvInputs[1] & 0x03) == 0) DrvInputs[1] |= 0x03;
		if ((DrvInputs[1] & 0x0c) == 0) DrvInputs[1] |= 0x0c;
	}

	konamiNewFrame();
	ZetNewFrame();

	INT32 nInterleave = 256;
	INT32 nCyclesTotal[2] = { 6000000 / 60, 3579545 / 60 };
	INT32 nCyclesDone[2] = { nCyclesExtra, 0 };

	ZetOpen(0);
	konamiOpen(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		CPU_RUN(0, konami);
		CPU_RUN_TIMER(1);
	}

	if (K051960_irq_enabled) konamiSetIrqLine(KONAMI_IRQ_LINE, CPU_IRQSTATUS_AUTO);

	konamiClose();
	ZetClose();

	nCyclesExtra = nCyclesDone[0] - nCyclesTotal[0];

	if (pBurnSoundOut) {
		BurnYM2151Render(pBurnSoundOut, nBurnSoundLen);
		K007232Update(0, pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {
		*pnMin = 0x029704;
	}

	if (nAction & ACB_VOLATILE) {
		memset(&ba, 0, sizeof(ba));

		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);

		konamiCpuScan(nAction);
		ZetScan(nAction);

		BurnYM2151Scan(nAction, pnMin);
		K007232Scan(nAction, pnMin);

		KonamiICScan(nAction);

		SCAN_VAR(nCyclesExtra);
	}

	if (nAction & ACB_WRITE) {
		konamiOpen(0);
		set_ram_bank(nDrvRamBank[0]);
		aliens_set_lines(nDrvKonamiBank[0]);
		konamiClose();
	}

	return 0;
}


// Aliens (World set 1)

static struct BurnRomInfo aliensRomDesc[] = {
	{ "875_j02.e24",	0x10000, 0x56c20971, 1 | BRF_PRG | BRF_ESS }, //  0 Konami CPU Code
	{ "875_j01.c24",	0x20000, 0x6a529cd6, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "875_b03.g04",	0x08000, 0x1ac4d283, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "875b11.k13",		0x80000, 0x89c5c885, 3 | BRF_GRA },           //  3 Background Tiles
	{ "875b12.k19",		0x80000, 0xea6bdc17, 3 | BRF_GRA },           //  4
	{ "875b07.j13",		0x40000, 0xe9c56d66, 3 | BRF_GRA },           //  5
	{ "875b08.j19",		0x40000, 0xf9387966, 3 | BRF_GRA },           //  6

	{ "875b10.k08",		0x80000, 0x0b1035b1, 4 | BRF_GRA },           //  7 Sprites
	{ "875b09.k02",		0x80000, 0xe76b3c19, 4 | BRF_GRA },           //  8
	{ "875b06.j08",		0x40000, 0x081a0566, 4 | BRF_GRA },           //  9
	{ "875b05.j02",		0x40000, 0x19a261f2, 4 | BRF_GRA },           // 10

	{ "875b04.e05",		0x40000, 0x4e209ac8, 5 | BRF_SND },           // 11 K007232 Samples

	{ "821a08.h14",		0x00100, 0x7da55800, 6 | BRF_OPT },           // 12 Timing Proms
};

STD_ROM_PICK(aliens)
STD_ROM_FN(aliens)

struct BurnDriver BurnDrvAliens = {
	"aliens", NULL, NULL, NULL, "1990",
	"Aliens (World set 1)\0", NULL, "Konami", "GX875",
	L"\u5F02\u5F62 (\u4E16\u754C\u7248 \u7B2C\u4E00\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_KONAMI, GBF_PLATFORM | GBF_RUNGUN, 0,
	NULL, aliensRomInfo, aliensRomName, NULL, NULL, NULL, NULL, AliensInputInfo, AliensDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	288, 224, 4, 3
};


// Aliens (World set 2)

static struct BurnRomInfo aliens2RomDesc[] = {
	{ "875_p02.e24",	0x10000, 0x4edd707d, 1 | BRF_PRG | BRF_ESS }, //  0 Konami CPU Code
	{ "875_n01.c24",	0x20000, 0x106cf59c, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "875_b03.g04",	0x08000, 0x1ac4d283, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "875b11.k13",		0x80000, 0x89c5c885, 3 | BRF_GRA },           //  3 Background Tiles
	{ "875b12.k19",		0x80000, 0xea6bdc17, 3 | BRF_GRA },           //  4
	{ "875b07.j13",		0x40000, 0xe9c56d66, 3 | BRF_GRA },           //  5
	{ "875b08.j19",		0x40000, 0xf9387966, 3 | BRF_GRA },           //  6

	{ "875b10.k08",		0x80000, 0x0b1035b1, 4 | BRF_GRA },           //  7 Sprites
	{ "875b09.k02",		0x80000, 0xe76b3c19, 4 | BRF_GRA },           //  8
	{ "875b06.j08",		0x40000, 0x081a0566, 4 | BRF_GRA },           //  9
	{ "875b05.j02",		0x40000, 0x19a261f2, 4 | BRF_GRA },           // 10

	{ "875b04.e05",		0x40000, 0x4e209ac8, 5 | BRF_SND },           // 11 K007232 Samples

	{ "821a08.h14",		0x00100, 0x7da55800, 6 | BRF_OPT },           // 12 Timing Proms
};

STD_ROM_PICK(aliens2)
STD_ROM_FN(aliens2)

struct BurnDriver BurnDrvAliens2 = {
	"aliens2", "aliens", NULL, NULL, "1990",
	"Aliens (World set 2)\0", NULL, "Konami", "GX875",
	L"\u5F02\u5F62 (\u4E16\u754C\u7248 \u7B2C\u4E8C\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_KONAMI, GBF_PLATFORM | GBF_RUNGUN, 0,
	NULL, aliens2RomInfo, aliens2RomName, NULL, NULL, NULL, NULL, AliensInputInfo, AliensDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	288, 224, 4, 3
};


// Aliens (World set 3)

static struct BurnRomInfo aliens3RomDesc[] = {
	{ "875_w3_2.e24",	0x10000, 0xf917f7b5, 1 | BRF_PRG | BRF_ESS }, //  0 Konami CPU Code
	{ "875_c01.c24",	0x20000, 0x3c0006fb, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "875_b03.g04",	0x08000, 0x1ac4d283, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "875b11.k13",		0x80000, 0x89c5c885, 3 | BRF_GRA },           //  3 Background Tiles
	{ "875b12.k19",		0x80000, 0xea6bdc17, 3 | BRF_GRA },           //  4
	{ "875b07.j13",		0x40000, 0xe9c56d66, 3 | BRF_GRA },           //  5
	{ "875b08.j19",		0x40000, 0xf9387966, 3 | BRF_GRA },           //  6

	{ "875b10.k08",		0x80000, 0x0b1035b1, 4 | BRF_GRA },           //  7 Sprites
	{ "875b09.k02",		0x80000, 0xe76b3c19, 4 | BRF_GRA },           //  8
	{ "875b06.j08",		0x40000, 0x081a0566, 4 | BRF_GRA },           //  9
	{ "875b05.j02",		0x40000, 0x19a261f2, 4 | BRF_GRA },           // 10

	{ "875b04.e05",		0x40000, 0x4e209ac8, 5 | BRF_SND },           // 11 K007232 Samples

	{ "821a08.h14",		0x00100, 0x7da55800, 6 | BRF_OPT },           // 12 Timing Proms
};

STD_ROM_PICK(aliens3)
STD_ROM_FN(aliens3)

struct BurnDriver BurnDrvAliens3 = {
	"aliens3", "aliens", NULL, NULL, "1990",
	"Aliens (World set 3)\0", NULL, "Konami", "GX875",
	L"\u5F02\u5F62 (\u4E16\u754C\u7248 \u7B2C\u4E09\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_KONAMI, GBF_PLATFORM | GBF_RUNGUN, 0,
	NULL, aliens3RomInfo, aliens3RomName, NULL, NULL, NULL, NULL, AliensInputInfo, AliensDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	288, 224, 4, 3
};


// Aliens (World set 4)

static struct BurnRomInfo aliens4RomDesc[] = {
	{ "875_d02.e24",	0x10000, 0x1dc46780, 1 | BRF_PRG | BRF_ESS }, //  0 Konami CPU Code
	{ "875_c01.c24",	0x20000, 0x3c0006fb, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "875_b03.g04",	0x08000, 0x1ac4d283, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "875b11.k13",		0x80000, 0x89c5c885, 3 | BRF_GRA },           //  3 Background Tiles
	{ "875b12.k19",		0x80000, 0xea6bdc17, 3 | BRF_GRA },           //  4
	{ "875b07.j13",		0x40000, 0xe9c56d66, 3 | BRF_GRA },           //  5
	{ "875b08.j19",		0x40000, 0xf9387966, 3 | BRF_GRA },           //  6

	{ "875b10.k08",		0x80000, 0x0b1035b1, 4 | BRF_GRA },           //  7 Sprites
	{ "875b09.k02",		0x80000, 0xe76b3c19, 4 | BRF_GRA },           //  8
	{ "875b06.j08",		0x40000, 0x081a0566, 4 | BRF_GRA },           //  9
	{ "875b05.j02",		0x40000, 0x19a261f2, 4 | BRF_GRA },           // 10

	{ "875b04.e05",		0x40000, 0x4e209ac8, 5 | BRF_SND },           // 11 K007232 Samples

	{ "821a08.h14",		0x00100, 0x7da55800, 6 | BRF_OPT },           // 12 Timing Proms
};

STD_ROM_PICK(aliens4)
STD_ROM_FN(aliens4)

struct BurnDriver BurnDrvAliens4 = {
	"aliens4", "aliens", NULL, NULL, "1990",
	"Aliens (World set 4)\0", NULL, "Konami", "GX875",
	L"\u5F02\u5F62 (\u4E16\u754C\u7248 \u7B2C\u56DB\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_KONAMI, GBF_PLATFORM | GBF_RUNGUN, 0,
	NULL, aliens4RomInfo, aliens4RomName, NULL, NULL, NULL, NULL, AliensInputInfo, AliensDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	288, 224, 4, 3
};


// Aliens (US, set 1)

static struct BurnRomInfo aliensuRomDesc[] = {
	{ "875_n02.e24",	0x10000, 0x24dd612e, 1 | BRF_PRG | BRF_ESS }, //  0 Konami CPU Code
	{ "875_n01.c24",	0x20000, 0x106cf59c, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "875_b03.g04",	0x08000, 0x1ac4d283, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "875b11.k13",		0x80000, 0x89c5c885, 3 | BRF_GRA },           //  3 Background Tiles
	{ "875b12.k19",		0x80000, 0xea6bdc17, 3 | BRF_GRA },           //  4
	{ "875b07.j13",		0x40000, 0xe9c56d66, 3 | BRF_GRA },           //  5
	{ "875b08.j19",		0x40000, 0xf9387966, 3 | BRF_GRA },           //  6

	{ "875b10.k08",		0x80000, 0x0b1035b1, 4 | BRF_GRA },           //  7 Sprites
	{ "875b09.k02",		0x80000, 0xe76b3c19, 4 | BRF_GRA },           //  8
	{ "875b06.j08",		0x40000, 0x081a0566, 4 | BRF_GRA },           //  9
	{ "875b05.j02",		0x40000, 0x19a261f2, 4 | BRF_GRA },           // 10

	{ "875b04.e05",		0x40000, 0x4e209ac8, 5 | BRF_SND },           // 11 K007232 Samples

	{ "821a08.h14",		0x00100, 0x7da55800, 6 | BRF_OPT },           // 12 Timing Proms
};

STD_ROM_PICK(aliensu)
STD_ROM_FN(aliensu)

struct BurnDriver BurnDrvAliensu = {
	"aliensu", "aliens", NULL, NULL, "1990",
	"Aliens (US, set 1)\0", NULL, "Konami", "GX875",
	L"\u5F02\u5F62 (\u7F8E\u7248 \u7B2C\u4E00\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_KONAMI, GBF_PLATFORM | GBF_RUNGUN, 0,
	NULL, aliensuRomInfo, aliensuRomName, NULL, NULL, NULL, NULL, AliensInputInfo, AliensDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	288, 224, 4, 3
};


// Aliens (US, set 2)

static struct BurnRomInfo aliensu2RomDesc[] = {
	{ "875_h02.e24",	0x10000, 0x328ddb15, 1 | BRF_PRG | BRF_ESS }, //  0 Konami CPU Code
	{ "875_h01.c24",	0x20000, 0xba7f5489, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "875_b03.g04",	0x08000, 0x1ac4d283, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "875b11.k13",		0x80000, 0x89c5c885, 3 | BRF_GRA },           //  3 Background Tiles
	{ "875b12.k19",		0x80000, 0xea6bdc17, 3 | BRF_GRA },           //  4
	{ "875b07.j13",		0x40000, 0xe9c56d66, 3 | BRF_GRA },           //  5
	{ "875b08.j19",		0x40000, 0xf9387966, 3 | BRF_GRA },           //  6

	{ "875b10.k08",		0x80000, 0x0b1035b1, 4 | BRF_GRA },           //  7 Sprites
	{ "875b09.k02",		0x80000, 0xe76b3c19, 4 | BRF_GRA },           //  8
	{ "875b06.j08",		0x40000, 0x081a0566, 4 | BRF_GRA },           //  9
	{ "875b05.j02",		0x40000, 0x19a261f2, 4 | BRF_GRA },           // 10

	{ "875b04.e05",		0x40000, 0x4e209ac8, 5 | BRF_SND },           // 11 K007232 Samples

	{ "821a08.h14",		0x00100, 0x7da55800, 6 | BRF_OPT },           // 12 Timing Proms
};

STD_ROM_PICK(aliensu2)
STD_ROM_FN(aliensu2)

struct BurnDriver BurnDrvAliensu2 = {
	"aliensu2", "aliens", NULL, NULL, "1990",
	"Aliens (US, set 2)\0", NULL, "Konami", "GX875",
	L"\u5F02\u5F62 (\u7F8E\u7248 \u7B2C\u4E8C\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_KONAMI, GBF_PLATFORM | GBF_RUNGUN, 0,
	NULL, aliensu2RomInfo, aliensu2RomName, NULL, NULL, NULL, NULL, AliensInputInfo, AliensDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	288, 224, 4, 3
};


// Aliens (Japan set 1)

static struct BurnRomInfo aliensjRomDesc[] = {
	{ "875_m02.e24",	0x10000, 0x54a774e5, 1 | BRF_PRG | BRF_ESS }, //  0 Konami CPU Code
	{ "875_m01.c24",	0x20000, 0x1663d3dc, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "875_k03.g04",	0x08000, 0xbd86264d, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "875b11.k13",		0x80000, 0x89c5c885, 3 | BRF_GRA },           //  3 Background Tiles
	{ "875b12.k19",		0x80000, 0xea6bdc17, 3 | BRF_GRA },           //  4
	{ "875b07.j13",		0x40000, 0xe9c56d66, 3 | BRF_GRA },           //  5
	{ "875b08.j19",		0x40000, 0xf9387966, 3 | BRF_GRA },           //  6

	{ "875b10.k08",		0x80000, 0x0b1035b1, 4 | BRF_GRA },           //  7 Sprites
	{ "875b09.k02",		0x80000, 0xe76b3c19, 4 | BRF_GRA },           //  8
	{ "875b06.j08",		0x40000, 0x081a0566, 4 | BRF_GRA },           //  9
	{ "875b05.j02",		0x40000, 0x19a261f2, 4 | BRF_GRA },           // 10

	{ "875b04.e05",		0x40000, 0x4e209ac8, 5 | BRF_SND },           // 11 K007232 Samples

	{ "821a08.h14",		0x00100, 0x7da55800, 6 | BRF_OPT },           // 12 Timing Proms
};

STD_ROM_PICK(aliensj)
STD_ROM_FN(aliensj)

struct BurnDriver BurnDrvAliensj = {
	"aliensj", "aliens", NULL, NULL, "1990",
	"Aliens (Japan set 1)\0", NULL, "Konami", "GX875",
	L"\u5F02\u5F62 (\u65E5\u7248 \u7B2C\u4E00\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_KONAMI, GBF_PLATFORM | GBF_RUNGUN, 0,
	NULL, aliensjRomInfo, aliensjRomName, NULL, NULL, NULL, NULL, AliensInputInfo, AliensDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	288, 224, 4, 3
};


// Aliens (Japan set 2)

static struct BurnRomInfo aliensj2RomDesc[] = {
	{ "875_j2_2.e24",	0x10000, 0x4bb84952, 1 | BRF_PRG | BRF_ESS }, //  0 Konami CPU Code
	{ "875_m01.c24",	0x20000, 0x1663d3dc, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "875_k03.g04",	0x08000, 0xbd86264d, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "875b11.k13",		0x80000, 0x89c5c885, 3 | BRF_GRA },           //  3 Background Tiles
	{ "875b12.k19",		0x80000, 0xea6bdc17, 3 | BRF_GRA },           //  4
	{ "875b07.j13",		0x40000, 0xe9c56d66, 3 | BRF_GRA },           //  5
	{ "875b08.j19",		0x40000, 0xf9387966, 3 | BRF_GRA },           //  6

	{ "875b10.k08",		0x80000, 0x0b1035b1, 4 | BRF_GRA },           //  7 Sprites
	{ "875b09.k02",		0x80000, 0xe76b3c19, 4 | BRF_GRA },           //  8
	{ "875b06.j08",		0x40000, 0x081a0566, 4 | BRF_GRA },           //  9
	{ "875b05.j02",		0x40000, 0x19a261f2, 4 | BRF_GRA },           // 10

	{ "875b04.e05",		0x40000, 0x4e209ac8, 5 | BRF_SND },           // 11 K007232 Samples

	{ "821a08.h14",		0x00100, 0x7da55800, 6 | BRF_OPT },           // 12 Timing Proms
};

STD_ROM_PICK(aliensj2)
STD_ROM_FN(aliensj2)

struct BurnDriver BurnDrvAliensj2 = {
	"aliensj2", "aliens", NULL, NULL, "1990",
	"Aliens (Japan set 2)\0", NULL, "Konami", "GX875",
	L"\u5F02\u5F62 (\u65E5\u7248 \u7B2C\u4E8C\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_KONAMI, GBF_PLATFORM | GBF_RUNGUN, 0,
	NULL, aliensj2RomInfo, aliensj2RomName, NULL, NULL, NULL, NULL, AliensInputInfo, AliensDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	288, 224, 4, 3
};


// Aliens (Asia)

static struct BurnRomInfo aliensaRomDesc[] = {
	{ "875_r02.e24",	0x10000, 0x973e4f11, 1 | BRF_PRG | BRF_ESS }, //  0 Konami CPU Code
	{ "875_m01.c24",	0x20000, 0x1663d3dc, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "875_k03.g04",	0x08000, 0xbd86264d, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 Code

	{ "875b11.k13",		0x80000, 0x89c5c885, 3 | BRF_GRA },           //  3 Background Tiles
	{ "875b12.k19",		0x80000, 0xea6bdc17, 3 | BRF_GRA },           //  4
	{ "875b07.j13",		0x40000, 0xe9c56d66, 3 | BRF_GRA },           //  5
	{ "875b08.j19",		0x40000, 0xf9387966, 3 | BRF_GRA },           //  6

	{ "875b10.k08",		0x80000, 0x0b1035b1, 4 | BRF_GRA },           //  7 Sprites
	{ "875b09.k02",		0x80000, 0xe76b3c19, 4 | BRF_GRA },           //  8
	{ "875b06.j08",		0x40000, 0x081a0566, 4 | BRF_GRA },           //  9
	{ "875b05.j02",		0x40000, 0x19a261f2, 4 | BRF_GRA },           // 10

	{ "875b04.e05",		0x40000, 0x4e209ac8, 5 | BRF_SND },           // 11 K007232 Samples

	{ "821a08.h14",		0x00100, 0x7da55800, 6 | BRF_OPT },           // 12 Timing Proms
};

STD_ROM_PICK(aliensa)
STD_ROM_FN(aliensa)

struct BurnDriver BurnDrvAliensa = {
	"aliensa", "aliens", NULL, NULL, "1990",
	"Aliens (Asia)\0", NULL, "Konami", "GX875",
	L"\u5F02\u5F62 (\u4E9A\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_KONAMI, GBF_PLATFORM | GBF_RUNGUN, 0,
	NULL, aliensaRomInfo, aliensaRomName, NULL, NULL, NULL, NULL, AliensInputInfo, AliensDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &DrvRecalc, 0x200,
	288, 224, 4, 3
};
