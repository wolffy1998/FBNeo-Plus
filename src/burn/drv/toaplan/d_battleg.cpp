// FB Alpha Battle Garegga driver module
// Driver and emulation by Jan Klaassen

#include "toaplan.h"
#include "nmk112.h"

// Battle Garegga

static UINT8 DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static UINT8 DrvReset = 0;

static HoldCoin<2> hold_coin;

static UINT8 nIRQPending;

static INT32 nSoundCommand;

// Z80 ROM bank
static INT32 nCurrentBank;

INT32 Bgareggabl = 0; // used by toa_extratext.cpp
static INT32 Bgareggabla = 0;
static INT32 location_test = 0;

// Rom information

// Battle Garegga (Europe / USA / Japan / Asia) (Sat Feb 3 1996)
static struct BurnRomInfo bgareggaRomDesc[] = {
	{ "prg0.bin",     0x080000, 0xf80c2fc2, BRF_ESS | BRF_PRG }, //  0 CPU #0 code (even)
	{ "prg1.bin",     0x080000, 0x2ccfdd1e, BRF_ESS | BRF_PRG }, //  1				(odd)

	{ "rom4.bin",     0x200000, 0xb333d81f, BRF_GRA },			 //  2 GP9001 Tile data
	{ "rom3.bin",     0x200000, 0x51b9ebfb, BRF_GRA },			 //  3
	{ "rom2.bin",     0x200000, 0xb330e5e2, BRF_GRA },			 //  4
	{ "rom1.bin",     0x200000, 0x7eafdd70, BRF_GRA },			 //  5

	{ "text.u81",     0x008000, 0xe67fd534, BRF_GRA },			 //  6 Extra text layer tile data

	{ "snd.bin",      0x020000, 0x68632952, BRF_ESS | BRF_PRG }, //  7 Z80 program

	{ "rom5.bin",     0x100000, 0xf6d49863, BRF_SND },			 //  8 MSM6295 ADPCM data
};

STD_ROM_PICK(bgaregga)
STD_ROM_FN(bgaregga)


// Battle Garegga (location test) (Wed Jan 17 1996)
static struct BurnRomInfo bgareggatRomDesc[] = {
	{ "battlegaregga-prg0-8-1-17.bin",	0x080000, 0xc032176f, BRF_PRG | BRF_ESS }, 			//  0 CPU #0 code (even)
	{ "battlegaregga-prg1-8-1-17.bin",	0x080000, 0x3822f375, BRF_PRG | BRF_ESS }, 			//  1			 (odd)

	{ "rom4.bin",						0x200000, 0xb333d81f, BRF_GRA },		   			//  2 GP9001 Tile data
	{ "rom3.bin",						0x200000, 0x51b9ebfb, BRF_GRA },           			//  3
	{ "rom2.bin",						0x200000, 0xb330e5e2, BRF_GRA },           			//  4
	{ "rom1.bin",						0x200000, 0x7eafdd70, BRF_GRA },           			//  5

	{ "text.u81",						0x008000, 0xe67fd534, BRF_GRA },           			//  6 Extra text layer tile data

	{ "battlegaregga-snd-8-1-18-loke-ver.bin",	0x020000, 0xf5ea56f7, BRF_ESS | BRF_PRG }, 	//  7 Z80 program

	{ "rom5.bin",						0x100000, 0xf6d49863, BRF_SND },           			//  8 MSM6295 ADPCM data
};

STD_ROM_PICK(bgareggat)
STD_ROM_FN(bgareggat)


// Battle Garegga Zakk version (Europe / USA / Japan / Asia) (Sat Feb 3 1996)
static struct BurnRomInfo bgareggazRomDesc[] = {
	{ "garegga-prg0.bin",     	0x080000, 0xec2f86d0, BRF_ESS | BRF_PRG }, 	//  0 CPU #0 code (even)
	{ "garegga-prg1.bin",     	0x080000, 0x371a0e42, BRF_ESS | BRF_PRG }, 	//  1				(odd)

	{ "rom4.bin",     			0x200000, 0xb333d81f, BRF_GRA },			//  2 GP9001 Tile data
	{ "rom3.bin",     			0x200000, 0x51b9ebfb, BRF_GRA },			//  3
	{ "rom2.bin",     			0x200000, 0xb330e5e2, BRF_GRA },			//  4
	{ "rom1.bin",     			0x200000, 0x7eafdd70, BRF_GRA },			//  5

	{ "text.u81",     			0x008000, 0xe67fd534, BRF_GRA },			//  6 Extra text layer tile data

	{ "snd.bin",      			0x020000, 0x68632952, BRF_ESS | BRF_PRG }, 	//  7 Z80 program

	{ "rom5.bin",     			0x100000, 0xf6d49863, BRF_SND },			//  8 MSM6295 ADPCM data
};

STD_ROM_PICK(bgareggaz)
STD_ROM_FN(bgareggaz)


// Battle Garegga (Austria / Hong Kong) (Sat Feb 3 1996)
static struct BurnRomInfo bgareghkRomDesc[] = {
	{ "prg_0.rom",    0x080000, 0x26e0019e, BRF_ESS | BRF_PRG }, //  0 CPU #0 code (even)
	{ "prg_1.rom",    0x080000, 0x2ccfdd1e, BRF_ESS | BRF_PRG }, //  1				(odd)

	{ "rom4.bin",     0x200000, 0xb333d81f, BRF_GRA },			 //  2 GP9001 Tile data
	{ "rom3.bin",     0x200000, 0x51b9ebfb, BRF_GRA },			 //  3
	{ "rom2.bin",     0x200000, 0xb330e5e2, BRF_GRA },			 //  4
	{ "rom1.bin",     0x200000, 0x7eafdd70, BRF_GRA },			 //  5

	{ "text.u81",     0x008000, 0xe67fd534, BRF_GRA },			 //  6 Extra text layer tile data

	{ "snd.bin",      0x020000, 0x68632952, BRF_ESS | BRF_PRG }, //  7 Z80 program

	{ "rom5.bin",     0x100000, 0xf6d49863, BRF_SND },			 //  8 MSM6295 ADPCM data
};

STD_ROM_PICK(bgareghk)
STD_ROM_FN(bgareghk)


// Battle Garegga (Korea / Greece) (Wed Feb 7 1996)
// only the program ROMs' dumps were provided for this set.
// According to the dumper: 'In the Korea Region Setting, DIP SWITCH's 'STAGE EDIT' does not work and
// the C button (formation change) function in the in-game is also deleted.'
static struct BurnRomInfo bgareggakRomDesc[] = {
	
	{ "prg0.bin",     	0x080000, 0x40a108a7, BRF_ESS | BRF_PRG }, 	//  0 CPU #0 code (even)
	{ "prg1.bin",    	0x080000, 0x45a6e48a, BRF_ESS | BRF_PRG }, 	//  1			  (odd)

	{ "rom4.bin",     	0x200000, 0xb333d81f, BRF_GRA },			//  2 GP9001 Tile data
	{ "rom3.bin",     	0x200000, 0x51b9ebfb, BRF_GRA },			//  3
	{ "rom2.bin",     	0x200000, 0xb330e5e2, BRF_GRA },			//  4
	{ "rom1.bin",     	0x200000, 0x7eafdd70, BRF_GRA },			//  5

	{ "text.u81",     	0x008000, 0xe67fd534, BRF_GRA },			//  6 Extra text layer tile data

	{ "snd.bin",      	0x020000, 0x68632952, BRF_ESS | BRF_PRG }, 	//  7 Z80 program

	{ "rom5.bin",     	0x100000, 0xf6d49863, BRF_SND },			 //  8 MSM6295 ADPCM data
};

STD_ROM_PICK(bgareggak)
STD_ROM_FN(bgareggak)


// Battle Garegga - New Version (Austria / Hong Kong) (Sat Mar 2 1996)
static struct BurnRomInfo bgaregnvRomDesc[] = {
	{ "prg_0.bin",    0x080000, 0x951ecc07, BRF_ESS | BRF_PRG }, //  0 CPU #0 code (even)
	{ "prg_1.bin",    0x080000, 0x729a60c6, BRF_ESS | BRF_PRG }, //  1			   (odd)

	{ "rom4.bin",     0x200000, 0xb333d81f, BRF_GRA },			 //  2 GP9001 Tile data
	{ "rom3.bin",     0x200000, 0x51b9ebfb, BRF_GRA },			 //  3
	{ "rom2.bin",     0x200000, 0xb330e5e2, BRF_GRA },			 //  4
	{ "rom1.bin",     0x200000, 0x7eafdd70, BRF_GRA },			 //  5

	{ "text.u81",     0x008000, 0xe67fd534, BRF_GRA },			 //  6 Extra text layer tile data

	{ "snd.bin",      0x020000, 0x68632952, BRF_ESS | BRF_PRG }, //  7 Z80 program

	{ "rom5.bin",     0x100000, 0xf6d49863, BRF_SND },			 //  8 MSM6295 ADPCM data
};

STD_ROM_PICK(bgaregnv)
STD_ROM_FN(bgaregnv)


// Battle Garegga - Type 2 (Europe / USA / Japan / Asia) (Sat Mar 2 1996)
static struct BurnRomInfo bgaregt2RomDesc[] = {
	{ "prg0",         0x080000, 0x84094099, BRF_ESS | BRF_PRG }, //  0 CPU #0 code (even)
	{ "prg1",         0x080000, 0x46f92fe4, BRF_ESS | BRF_PRG }, //  1				(odd)

	{ "rom4.bin",     0x200000, 0xb333d81f, BRF_GRA },			 //  2 GP9001 Tile data
	{ "rom3.bin",     0x200000, 0x51b9ebfb, BRF_GRA },			 //  3
	{ "rom2.bin",     0x200000, 0xb330e5e2, BRF_GRA },			 //  4
	{ "rom1.bin",     0x200000, 0x7eafdd70, BRF_GRA },			 //  5

	{ "text.u81",     0x008000, 0xe67fd534, BRF_GRA },			 //  6 Extra text layer tile data

	{ "snd.bin",      0x020000, 0x68632952, BRF_ESS | BRF_PRG }, //  7 Z80 program

	{ "rom5.bin",     0x100000, 0xf6d49863, BRF_SND },			 //  8 MSM6295 ADPCM data
};

STD_ROM_PICK(bgaregt2)
STD_ROM_FN(bgaregt2)


// Battle Garegga - Type 2 (Denmark / China) (Tue Apr 2 1996)
static struct BurnRomInfo bgaregcnRomDesc[] = {
	{ "u123",         0x080000, 0x88a4e66a, BRF_ESS | BRF_PRG }, //  0 CPU #0 code (even)
	{ "u65",          0x080000, 0x5dea32a3, BRF_ESS | BRF_PRG }, //  1				(odd)

	{ "rom4.bin",     0x200000, 0xb333d81f, BRF_GRA },			 //  2 GP9001 Tile data
	{ "rom3.bin",     0x200000, 0x51b9ebfb, BRF_GRA },			 //  3
	{ "rom2.bin",     0x200000, 0xb330e5e2, BRF_GRA },			 //  4
	{ "rom1.bin",     0x200000, 0x7eafdd70, BRF_GRA },			 //  5

	{ "text.u81",     0x008000, 0xe67fd534, BRF_GRA },			 //  6 Extra text layer tile data

	{ "snd.bin",      0x020000, 0x68632952, BRF_ESS | BRF_PRG }, //  7 Z80 program

	{ "rom5.bin",     0x100000, 0xf6d49863, BRF_SND },			 //  8 MSM6295 ADPCM data
};

STD_ROM_PICK(bgaregcn)
STD_ROM_FN(bgaregcn)


// Battle Garegga (Taiwan / Germany) (Thu Feb 1 1996)
static struct BurnRomInfo bgaregtwRomDesc[] = {
	{ "garegga_prg0.u123",	0x080000, 0x235b7405, BRF_ESS | BRF_PRG }, 	//  0 CPU #0 code (even)
	{ "garegga_prg1.u65", 	0x080000, 0xc29ccf6a, BRF_ESS | BRF_PRG }, 	//  1				(odd)

	{ "rom4.bin",     		0x200000, 0xb333d81f, BRF_GRA },			//  2 GP9001 Tile data
	{ "rom3.bin",     		0x200000, 0x51b9ebfb, BRF_GRA },			//  3
	{ "rom2.bin",     		0x200000, 0xb330e5e2, BRF_GRA },			//  4
	{ "rom1.bin",     		0x200000, 0x7eafdd70, BRF_GRA },			//  5

	{ "text.u81",     		0x008000, 0xe67fd534, BRF_GRA },			//  6 Extra text layer tile data

	{ "snd.bin",      		0x020000, 0x68632952, BRF_ESS | BRF_PRG }, 	//  7 Z80 program

	{ "rom5.bin",     		0x100000, 0xf6d49863, BRF_SND },			//  8 MSM6295 ADPCM data
};

STD_ROM_PICK(bgaregtw)
STD_ROM_FN(bgaregtw)


// 1945 Part-2 (Chinese hack of Battle Garegga)
static struct BurnRomInfo bgareggablRomDesc[] = {
	{ "xt-8m.bin",    0x100000, 0x4a6657cb, BRF_ESS | BRF_PRG }, //  0 CPU #0 code

	{ "6@-322",       0x400000, 0x37fe48ed, BRF_GRA },			 //  1 GP9001 Tile data // rom4.bin + rom3.bin
	{ "5@-322",       0x400000, 0x5a06c031, BRF_GRA },			 //  2					// rom2.bin + rom1.bin

	{ "1@-256",       0x008000, 0x760dcd14, BRF_GRA },			 //  3 Extra text layer tile data

	{ "snd.bin",      0x020000, 0x68632952, BRF_ESS | BRF_PRG }, //  4 Z80 program

	{ "rom5.bin",     0x100000, 0xf6d49863, BRF_SND },			 //  5 MSM6295 ADPCM data
	
	{ "2@-256",       0x008000, 0x456dd16e, BRF_GRA },			 //  6 (looks like garbage)
};

STD_ROM_PICK(bgareggabl)
STD_ROM_FN(bgareggabl)


// Lei Shen Zhuan Thunder Deity Biography (Chinese hack of Battle Garegga)
static struct BurnRomInfo bgareggablaRomDesc[] = {
	{ "27c8100.mon-sys",    0x100000, 0xd334e5aa, BRF_ESS | BRF_PRG }, 	//  0 CPU #0 code
	
	{ "rom4.bin",     		0x200000, 0xb333d81f, BRF_GRA },			//  1 GP9001 Tile data
	{ "rom3.bin",     		0x200000, 0x51b9ebfb, BRF_GRA },			//  2
	{ "rom2.bin",     		0x200000, 0xb330e5e2, BRF_GRA },			//  3
	{ "rom1.bin",     		0x200000, 0x7eafdd70, BRF_GRA },			//  4

	{ "text.bin",     		0x008000, 0x00d100bd, BRF_GRA },			//  5 Extra text layer tile data

	{ "snd.bin",      		0x020000, 0x68632952, BRF_ESS | BRF_PRG }, 	//  6 Z80 program

	{ "rom5.bin",     		0x100000, 0xf6d49863, BRF_SND },			//  7 MSM6295 ADPCM data
	
	{ "base.bin",     		0x008000, 0x456dd16e, BRF_GRA },			//  8 (looks like garbage)
};

STD_ROM_PICK(bgareggabla)
STD_ROM_FN(bgareggabla)


// Battle Garegga (Japan, bootleg) (Sat Feb 3 1996)
static struct BurnRomInfo bgareggabljRomDesc[] = {
	{ "sys.bin",      	0x200000, 0xb2a1225f, BRF_ESS | BRF_PRG }, 	//  0 CPU #0 code

	{ "322_2.bin",      0x400000, 0x37fe48ed, BRF_GRA },			//  1 GP9001 Tile data // rom4.bin + rom3.bin
	{ "322_1.bin",      0x400000, 0x5a06c031, BRF_GRA },			//  2					// rom2.bin + rom1.bin

	{ "text.bin",       0x008000, 0xe67fd534, BRF_GRA },			//  3 Extra text layer tile data

	{ "snd.bin",      	0x020000, 0x68632952, BRF_ESS | BRF_PRG }, 	//  4 Z80 program

	{ "rom5.bin",     	0x100000, 0xf6d49863, BRF_SND },			 //  5 MSM6295 ADPCM data
	
	{ "base.bin",       0x008000, 0x456dd16e, BRF_GRA },			 //  6 (looks like garbage)
};

STD_ROM_PICK(bgareggablj)
STD_ROM_FN(bgareggablj)


static struct BurnInputInfo battlegInputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p1 coin"	},
	{"P1 Start",	BIT_DIGITAL,	DrvButton + 5,	"p1 start"	},

	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",	BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 4,	"p2 coin"	},
	{"P2 Start",	BIT_DIGITAL,	DrvButton + 6,	"p2 start"	},

	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",	BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Service",		BIT_DIGITAL,	DrvButton + 0,	"service"	},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"		},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"		},
	{"Dip C",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"		},
};

STDINPUTINFO(battleg)

static struct BurnDIPInfo bgareggaDIPList[] = {

	// Defaults
	{0x14,	0xFF, 0xFF,	0x00, NULL},
	{0x15,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0,		0xFE, 0,	2,	  "Test mode"},
	{0x14,	0x01, 0x01,	0x00, "Normal"},
	{0x14,	0x01, 0x01,	0x01, "Test"},
	{0,		0xFE, 0,	2,	  "Starting coin"},
	{0x14,	0x01, 0x02,	0x00, "1 credit"},
	{0x14,	0x01, 0x02,	0x02, "2 credits"},
	{0,		0xFE, 0,	8,	  "Coin 1"},
	{0x14,	0x01, 0x1C, 0x00, "1 coin 1 credit"},
	{0x14,	0x01, 0x1C, 0x04, "1 coin 2 credits"},
	{0x14,	0x01, 0x1C, 0x08, "1 coin 3 credits"},
	{0x14,	0x01, 0x1C, 0x0C, "1 coin 3 credits"},
	{0x14,	0x01, 0x1C, 0x10, "2 coins 1 credit"},
	{0x14,	0x01, 0x1C, 0x14, "3 coins 1 credit"},
	{0x14,	0x01, 0x1C, 0x18, "4 coins 1 credit"},
	{0x14,	0x01, 0x1C, 0x1C, "Free Play"},
	// 0x1C: Free play settings active
	// Normal
	{0,		0xFE, 0,	7,	  "Coin 2"},
	{0x14,	0x82, 0xE0,	0x00, "1 coin 1 credit"},
	{0x14,	0x00, 0x1C, 0x1C, NULL},
	{0x14,	0x82, 0xE0,	0x20, "1 coin 2 credits"},
	{0x14,	0x00, 0x1C, 0x1C, NULL},
	{0x14,	0x82, 0xE0,	0x40, "1 coin 3 credits"},
	{0x14,	0x00, 0x1C, 0x1C, NULL},
	{0x14,	0x82, 0xE0,	0x60, "1 coin 3 credits"},
	{0x14,	0x00, 0x1C, 0x1C, NULL},
	{0x14,	0x82, 0xE0,	0x80, "2 coins 1 credit"},
	{0x14,	0x00, 0x1C, 0x1C, NULL},
	{0x14,	0x82, 0xE0,	0xA0, "3 coins 1 credit"},
	{0x14,	0x00, 0x1C, 0x1C, NULL},
	{0x14,	0x82, 0xE0,	0xC0, "4 coins 1 credit"},
	{0x14,	0x00, 0x1C, 0x1C, NULL},
	{0x14,	0x82, 0xE0,	0x04, "1 coin 1 credit"},
	{0x14,	0x00, 0x1C, 0x1C, NULL},
	// Free play
	{0,		0xFE, 0,	2,	  "Stick mode"},
	{0x14,	0x02, 0x20,	0x00, "Special"},
	{0x14,	0x00, 0x1C, 0x1C, NULL},
	{0x14,	0x02, 0x20,	0x20, "Normal"},
	{0x14,	0x00, 0x1C, 0x1C, NULL},
	{0,		0xFE, 0,	2,	  "Effect"},
	{0x14,	0x02, 0x40,	0x00, "Off"},
	{0x14,	0x00, 0x1C, 0x1C, NULL},
	{0x14,	0x02, 0x40,	0x40, "On"},
	{0x14,	0x00, 0x1C, 0x1C, NULL},
	{0,		0xFE, 0,	2,	  "Music"},
	{0x14,	0x02, 0x80,	0x00, "Off"},
	{0x14,	0x00, 0x1C, 0x1C, NULL},
	{0x14,	0x02, 0x80,	0x80, "On"},
	{0x14,	0x00, 0x1C, 0x1C, NULL},

	// DIP 2
	{0,		0xFE, 0,	4,	  "Difficulty"},
	{0x15,	0x01, 0x03, 0x00, "Normal"},
	{0x15,	0x01, 0x03, 0x01, "Easy"},
	{0x15,	0x01, 0x03, 0x02, "Hard"},
	{0x15,	0x01, 0x03, 0x03, "Very hard"},
	{0,		0xFE, 0,	2,	  "Screen flip"},
	{0x15,	0x01, 0x04, 0x00, "Off"},
	{0x15,	0x01, 0x04, 0x04, "On"},
	{0,		0xFE, 0,	2,	  "Demo sound"},
	{0x15,	0x01, 0x08, 0x00, "On"},
	{0x15,	0x01, 0x08, 0x08, "Off"},
	{0,		0xFE, 0,	8,	  "Player counts"},
	{0x15,	0x01, 0x70, 0x00, "3"},
	{0x15,	0x01, 0x70, 0x10, "4"},
	{0x15,	0x01, 0x70, 0x20, "2"},
	{0x15,	0x01, 0x70, 0x30, "1"},
	{0x15,	0x01, 0x70, 0x40, "5"},
	{0x15,	0x01, 0x70, 0x50, "6"},
	{0x15,	0x01, 0x70, 0x60, "Multiple"},
	{0x15,	0x01, 0x70, 0x70, "Invincible"},
    // Nippon and U.S.A. regions
	{0,		0xFE, 0,	2,	  "Extra player"},
    {0x15,	0x02, 0x80, 0x00, "1000000 each"},
	{0x16,	0x00, 0x01, 0x00, NULL},
	{0x15,	0x02, 0x80, 0x80, "1000000 2000000"},
	{0x16,	0x00, 0x01, 0x00, NULL},
    // Europe/Denmark and Asia regions
	{0,		0xFE, 0,	2,	  "Extra player"},
	{0x15,	0x02, 0x80, 0x80, "2000000 each"},
	{0x16,	0x00, 0x01, 0x01, NULL},
	{0x15,	0x02, 0x80, 0x00, "No extra"},
	{0x16,	0x00, 0x01, 0x01, NULL},

	// DIP 3
	{0,		0xFE, 0,	2,	  "Continue play"},
	{0x16,	0x01, 0x04, 0x00, "On"},
	{0x16,	0x01, 0x04, 0x04, "Off"},
};

static struct BurnDIPInfo bgareggacnRegionDIPList[] = {
	// DIP 3
	{0,		0xFE, 0,	2,	  "Stage edit"},
	{0x16,	0x01, 0x08, 0x00, "Disable"},
	{0x16,	0x01, 0x08, 0x08, "Enable"},
	
	// Region
	{0x16,	0xFF, 0x0F,	0x01, NULL},
	{0,		0xFE, 0,	2,	  "Region"},
	{0x16,	0x01, 0x03, 0x01, "Denmark (German Tuning license)"},
	{0x16,	0x01, 0x03, 0x03, "China"},
	{0x16,	0x01, 0x03, 0x00, "Japan [illegal setting]"},
	{0x16,	0x01, 0x03, 0x02, "U.S.A. (Fabtek license) [illegal setting]"},
};

static struct BurnDIPInfo bgareggaRegionDIPList[] = {
	// DIP 3
	{0,		0xFE, 0,	2,	  "Stage edit"},
	{0x16,	0x01, 0x08, 0x00, "Disable"},
	{0x16,	0x01, 0x08, 0x08, "Enable"},

	// Region
	{0x16,	0xFF, 0x0F,	0x00, NULL},
	{0,		0xFE, 0,	4,	  "Region"},
	{0x16,	0x01, 0x03, 0x00, "Japan"},
	{0x16,	0x01, 0x03, 0x01, "Europe (German Tuning license"},
	{0x16,	0x01, 0x03, 0x02, "U.S.A. (Fabtek license)"},
	{0x16,	0x01, 0x03, 0x03, "Asia"},
};

static struct BurnDIPInfo bgareggakRegionDIPList[] = {
	// DIP 3
	{0,		0xFE, 0,	2,	  "Stage edit"},
	{0x16,	0x01, 0x08, 0x00, "Disable"},
	{0x16,	0x01, 0x08, 0x08, "Enable"},

	// Region
	{0x16,	0xFF, 0x0F,	0x01, NULL},
	{0,		0xFE, 0,	2,	  "Region"},
	{0x16,	0x01, 0x03, 0x01, "Greece"},
	{0x16,	0x01, 0x03, 0x03, "Korea"},
};

static struct BurnDIPInfo bgareggahkRegionDIPList[] = {
	// DIP 3
	{0,		0xFE, 0,	2,	  "Stage edit"},
	{0x16,	0x01, 0x08, 0x00, "Disable"},
	{0x16,	0x01, 0x08, 0x08, "Enable"},
	
	// Region
	{0x16,	0xFF, 0x0F,	0x01, NULL},
	{0,		0xFE, 0,	2,	  "Region"},
	{0x16,	0x01, 0x03, 0x01, "Austria (German Tuning license"},
	{0x16,	0x01, 0x03, 0x03, "Hong Kong (metrotainment license"},
	{0x16,	0x01, 0x03, 0x00, "Japan [illegal setting]"},
	{0x16,	0x01, 0x03, 0x02, "U.S.A. (Fabtek license) [illegal setting]"},
};

static struct BurnDIPInfo bgareggatwRegionDIPList[] = {
	// DIP 3
	{0,		0xFE, 0,	2,	  "Stage edit"},
	{0x16,	0x01, 0x08, 0x00, "Disable"},
	{0x16,	0x01, 0x08, 0x08, "Enable"},
	
	// Region
	{0x16,	0xFF, 0x0F,	0x01, NULL},
	{0,		0xFE, 0,	2,	  "Region"},
	{0x16,	0x01, 0x03, 0x01, "Germany (German Tuning license)"},
	{0x16,	0x01, 0x03, 0x03, "Taiwan (Liang Hwa license)"},
	{0x16,	0x01, 0x03, 0x00, "Japan [illegal setting]"},
	{0x16,	0x01, 0x03, 0x02, "U.S.A. (Fabtek license) [illegal setting]"},
};

STDDIPINFOEXT(bgaregga, bgaregga, bgareggaRegion)
STDDIPINFOEXT(bgareggacn, bgaregga, bgareggacnRegion)
STDDIPINFOEXT(bgareggak, bgaregga, bgareggakRegion)
STDDIPINFOEXT(bgareggahk, bgaregga, bgareggahkRegion)
STDDIPINFOEXT(bgareggatw, bgaregga, bgareggatwRegion)

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01;
static UINT8 *Ram01, *Ram02, *RamPal;

static INT32 nColCount = 0x0800;
static INT32 nMSM6295ROMSize = 0x100000;

// This routine is called first to determine how much memory is needed (MemEnd-(UINT8 *)0),
// and then afterwards to set up all the pointers
static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	Rom01		= Next; Next += 0x100000;			//
	RomZ80		= Next; Next += 0x020000;			// Z80 ROM
	GP9001ROM[0]= Next; Next += nGP9001ROMSize[0];	// GP9001 tile data
	ExtraTROM	= Next; Next += 0x010000;			// Extra Text layer tile data
	MSM6295ROM	= Next; Next += nMSM6295ROMSize;	// ADPCM data
	RamStart	= Next;
	Ram01		= Next; Next += 0x010000;			// CPU #0 work RAM
	Ram02		= Next; Next += 0x000800;			//
	ExtraTRAM	= Next; Next += 0x002000;			// Extra tile layer
	ExtraTScroll= Next; Next += 0x001000;			//
	ExtraTSelect= Next; Next += 0x001000;			//
	RamPal		= Next; Next += 0x001000;			// palette
	RamZ80		= Next; Next += 0x004000;			// Z80 RAM
	GP9001RAM[0]= Next; Next += 0x004000;
	GP9001Reg[0]= (UINT16*)Next; Next += 0x0100 * sizeof(UINT16);
	RamEnd		= Next;
	ToaPalette	= (UINT32 *)Next; Next += nColCount * sizeof(UINT32);
	MemEnd		= Next;

	return 0;
}

static void drvZ80Bankswitch(INT32 nBank)
{
	nBank &= 0x07;
	if (nBank != nCurrentBank) {
		UINT8* nStartAddress = RomZ80 + (nBank << 14);
		ZetMapArea(0x8000, 0xBFFF, 0, nStartAddress);
		ZetMapArea(0x8000, 0xBFFF, 2, nStartAddress);

		nCurrentBank = nBank;
	}
}

// Scan ram
static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x029497;
	}

	if (nAction & ACB_VOLATILE) {		// Scan volatile data
		memset(&ba, 0, sizeof(ba));
		ba.Data		= RamStart;
		ba.nLen		= RamEnd-RamStart;
		ba.szName	= "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);				// scan 68000 states
		ZetScan(nAction);				// Scan Z80

		MSM6295Scan(nAction, pnMin);
		BurnYM2151Scan(nAction, pnMin);
		NMK112_Scan(nAction);

		ToaScanGP9001(nAction, pnMin);

		SCAN_VAR(nCurrentBank);
		SCAN_VAR(nSoundCommand);
		SCAN_VAR(nIRQPending);

		hold_coin.scan();

		if (nAction & ACB_WRITE) {
			INT32 nBank = nCurrentBank;
			nCurrentBank = -1;
			ZetOpen(0);
			drvZ80Bankswitch(nBank);
			ZetClose();
		}
	}

	return 0;
}

static INT32 LoadRoms()
{
	// Load 68000 ROM
	if (ToaLoadCode(Rom01, 0, 2)) {
		return 1;
	}

	// Load GP9001 tile data
	ToaLoadGP9001Tiles(GP9001ROM[0], 2, 4, nGP9001ROMSize[0]);

	// Load Extra text layer tile data
	BurnLoadRom(ExtraTROM, 6, 1);

	// Load the Z80 ROM
	BurnLoadRom(RomZ80, 7, 1);

	// Load ADPCM data
	BurnLoadRom(MSM6295ROM, 8, 1);

	return 0;
}

static INT32 LoadRomsBl()
{
	// Load 68000 ROM
	if (BurnLoadRom(Rom01, 0, 1)) {
		return 1;
	}

	// Load GP9001 tile data
	ToaLoadGP9001Tiles(GP9001ROM[0], 1, 2, nGP9001ROMSize[0]);

	// Load Extra text layer tile data
	BurnLoadRom(ExtraTROM, 3, 1);

	// Load the Z80 ROM
	BurnLoadRom(RomZ80, 4, 1);

	// Load ADPCM data
	BurnLoadRom(MSM6295ROM, 5, 1);

	return 0;
}

static INT32 LoadRomsBla()
{
	// Load 68000 ROM
	if (BurnLoadRom(Rom01, 0, 1)) {
		return 1;
	}

	// Load GP9001 tile data
	ToaLoadGP9001Tiles(GP9001ROM[0], 1, 4, nGP9001ROMSize[0]);

	// Load Extra text layer tile data
	BurnLoadRom(ExtraTROM, 5, 1);

	// Load the Z80 ROM
	BurnLoadRom(RomZ80, 6, 1);

	// Load ADPCM data
	BurnLoadRom(MSM6295ROM, 7, 1);

	return 0;
}

static UINT8 __fastcall battlegZ80Read(UINT16 nAddress)
{
//	bprintf(0, _T("z80 read %4X\n"), nAddress);
	switch (nAddress) {
		case 0xE001:
			return BurnYM2151Read();

		case 0xE004:
			return MSM6295Read(0);

		case 0xE01C:
			return nSoundCommand;

		case 0xE01D:
			// Bit 0 enables/disables interrupt processing (0 = enabled)
			return 0;
	}
	return 0;
}

static void __fastcall battlegZ80Write(UINT16 nAddress, UINT8 nValue)
{
//	bprintf(0, _T("z80 wrote %4X with %2X\n"), nAddress, nValue);
	switch (nAddress) {
		case 0xE000:
			BurnYM2151SelectRegister(nValue);
			break;
		case 0xE001:
			BurnYM2151WriteRegister(nValue);
			break;

		case 0xE004:
			MSM6295Write(0, nValue);
			break;

		case 0xE006:
		case 0xE008:
			NMK112_okibank_write((nAddress - 0xe006) + 0, nValue & 0xf);
			NMK112_okibank_write((nAddress - 0xe006) + 1, nValue >> 4);
			break;

		case 0xE00A: {
			drvZ80Bankswitch(nValue);
			break;
  		}

		case 0xE00C:
			// Once a sound command is processed, it is written to this address
			// bprintf(0, _T("z80 wrote %4X -> %2X\n"), nAddress, nValue);
			break;

	}
}

static INT32 DrvZ80Init()
{
	ZetInit(0);
	ZetOpen(0);

	ZetSetReadHandler(battlegZ80Read);
	ZetSetWriteHandler(battlegZ80Write);

	// ROM bank 1
	ZetMapArea    (0x0000, 0x7FFF, 0, RomZ80 + 0x0000); // Direct Read from ROM
	ZetMapArea    (0x0000, 0x7FFF, 2, RomZ80 + 0x0000); // Direct Fetch from ROM
	// ROM bank 2
	ZetMapArea    (0x8000, 0xBFFF, 0, RomZ80 + 0x8000); // Direct Read from ROM
	ZetMapArea    (0x8000, 0xBFFF, 2, RomZ80 + 0x8000); // Direct Fetch from ROM
	// RAM
	ZetMapArea    (0xC000, 0xDFFF, 0, RamZ80);			// Direct Read from RAM
	ZetMapArea    (0xC000, 0xDFFF, 1, RamZ80);			// Direct Write to RAM
	ZetMapArea    (0xC000, 0xDFFF, 2, RamZ80);			//
	// Ports
	ZetMemCallback(0xE000, 0xE0FF, 0);					// Read
	ZetMemCallback(0xE000, 0xE0FF, 1);					// Write

	ZetClose();

	nCurrentBank = 2;

	return 0;
}

static UINT8 __fastcall battlegReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {

		case 0x218021:								// The 68K has access to the Z80 RAM
			return RamZ80[0x10];					// Only these addresses are used, however
		case 0x218023:								//
			return RamZ80[0x11];					//

		case 0x21C021:								// Player 1 inputs
			return DrvInput[0];
		case 0x21C025:								// Player 2 inputs
			return DrvInput[1];
		case 0x21C029:								// Other inputs
			return DrvInput[2];
		case 0x21C02D:								// Dipswitch A
			return DrvInput[3];
		case 0x21C031:			   					// Dipswitch B
			return DrvInput[4];
		case 0x21C035:								// Dipswitch C - Territory
			return DrvInput[5];

//		default:
//			bprintf(0, _T("Attempt to read byte value of location %x\n"), sekAddress);
	}
	return 0;
}

static UINT16 __fastcall battlegReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {

		case 0x21C03C:
			return ToaScanlineRegister();
			//return (location_test) ? ToaScanlineRegisterLoctest() : ToaScanlineRegister();

		case 0x300004:
			return ToaGP9001ReadRAM_Hi(0);
		case 0x300006:
			return ToaGP9001ReadRAM_Lo(0);

//		default:
//			bprintf(0, _T("Attempt to read word value of location %x\n"), sekAddress);
	}
	return 0;
}

static void __fastcall battlegWriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {

		case 0x218021:								// The 68K has access to the Z80 RAM
			RamZ80[0x10] = byteValue;				// Only these addresses are used, however
			break;

		case 0x21C01D:								// Coin control
			break;

		case 0x600001:
			nSoundCommand = byteValue;

			// Trigger Z80 interrupt, and allow the Z80 to process it
			ZetSetIRQLine(0xff, CPU_IRQSTATUS_AUTO);
			nCyclesDone[1] += ZetRun(0x0200);
			break;

//		default:
//			bprintf(0, _T("Attempt to write byte value %x to location %x\n"), byteValue, sekAddress);
	}
}

static void __fastcall battlegWriteWord(UINT32 sekAddress, UINT16 wordValue)
{
	switch (sekAddress) {

		case 0x300000:								// Set GP9001 VRAM address-pointer
			ToaGP9001SetRAMPointer(wordValue);
			break;

		case 0x300004:
		case 0x300006:
			ToaGP9001WriteRAM(wordValue, 0);
			break;

		case 0x300008:
			ToaGP9001SelectRegister(wordValue);
			break;

		case 0x30000C: {
			ToaGP9001WriteRegister(wordValue);
			break;
		}

//		default:
//			bprintf(0, _T("Attempt to write word value %x to location %x\n"), wordValue, sekAddress);
	}
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	nIRQPending = 0;
	SekSetIRQLine(0, CPU_IRQSTATUS_NONE);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	MSM6295Reset(0);
	BurnYM2151Reset();
	NMK112Reset();

	hold_coin.reset();

	HiscoreReset();

	return 0;
}

static INT32 battlegInit()
{
	INT32 nLen;

#ifdef DRIVER_ROTATION
	bToaRotateScreen = true;
#endif

	nGP9001ROMSize[0] = 0x800000;

	// Find out how much memory is needed
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) {
		return 1;
	}
	memset(Mem, 0, nLen);										// blank all memory
	MemIndex();													// Index the allocated memory

	// Load the roms into memory
	if (Bgareggabla) {
		if (LoadRomsBla()) {
			return 1;
		}
	} else {
		if (Bgareggabl) {
			if (LoadRomsBl()) {
				return 1;
			}
		} else {
			if (LoadRoms()) {
				return 1;
			}
		}
	}

	{
		SekInit(0, 0x68000);										// Allocate 68000
		SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom01,			0x000000, 0x0FFFFF, MAP_ROM);	// CPU 0 ROM
		SekMapMemory(Ram01,			0x100000, 0x10FFFF, MAP_RAM);
		SekMapMemory(RamPal,		0x400000, 0x400FFF, MAP_RAM);	// Palette RAM
		SekMapMemory(Ram02,			0x401000, 0x4017FF, MAP_RAM);	// Unused
		SekMapMemory(ExtraTRAM,		0x500000, 0x501FFF, MAP_RAM);
		SekMapMemory(ExtraTSelect,	0x502000, 0x502FFF, MAP_RAM);	// 0x502000 - Scroll; 0x502200 - RAM
		SekMapMemory(ExtraTScroll,	0x503000, 0x503FFF, MAP_RAM);	// 0x203000 - Offset; 0x503200 - RAM

		SekSetReadWordHandler(0, battlegReadWord);
		SekSetReadByteHandler(0, battlegReadByte);
		SekSetWriteWordHandler(0, battlegWriteWord);
		SekSetWriteByteHandler(0, battlegWriteByte);

		SekClose();
	}

	nSpriteXOffset = 0x0024;
	nSpriteYOffset = 0x0001;

	nLayer0XOffset = -0x01D6;
	nLayer1XOffset = -0x01D8;
	nLayer2XOffset = -0x01DA;

	ToaInitGP9001();

	nExtraTXOffset = 0x2C;
	ToaExtraTextInit();
	
	if (Bgareggabl) nExtraTXOffset = 0;

	DrvZ80Init();												// Initialize Z80

	BurnYM2151Init(32000000 / 8);
	BurnYM2151SetAllRoutes(0.50, BURN_SND_ROUTE_BOTH);
	MSM6295Init(0, 32000000 / 16 / 132, 1);
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);

	NMK112_init(0, MSM6295ROM, NULL, 0x100000, 0); // only 1

	nToaPalLen = nColCount;
	ToaPalSrc = RamPal;
	ToaPalInit();

	// mar 2 1996 & apr 2 1996 ver:	0x0009AC - 0x0009B8 & 0x001F5E - 0x001F64 & 0x003A1C - 0x003A22
	// feb 2 1996 ver:				0x0009AC - 0x0009B8 & 0x001F2E - 0x001F34 & 0x0039EC - 0x0039F2

	DrvDoReset();												// Reset machine

	return 0;
}

static INT32 BgareggatInit()
{
	location_test = 1;

	return battlegInit();
}

static INT32 BgareggablInit()
{
	Bgareggabl = 1;
	
	return battlegInit();
}

static INT32 BgareggablaInit()
{
	Bgareggabl = 1;
	Bgareggabla = 1;
	
	return battlegInit();
}

static INT32 DrvExit()
{
	MSM6295Exit(0);
	BurnYM2151Exit();

	ToaPalExit();
	ToaExitGP9001();
	ToaExtraTextExit();
	ToaZExit();				// Z80 exit
	SekExit();				// Deallocate 68000s

	BurnFree(Mem);
	
	Bgareggabl = 0;
	Bgareggabla = 0;
	location_test = 0;

	return 0;
}

static INT32 DrvDraw()
{
	ToaClearScreen(0);

	ToaGetBitmap();
	ToaRenderGP9001();						// Render GP9001 graphics
	ToaExtraTextLayer();					// Render extra text layer

	ToaPalUpdate();							// Update the palette

	return 0;
}

static INT32 DrvFrame()
{
	INT32 nInterleave = 8;

	if (DrvReset) {														// Reset machine
		DrvDoReset();
	}

	// Compile digital inputs
	DrvInput[0] = 0x00;													// Buttons
	DrvInput[1] = 0x00;													// Player 1
	DrvInput[2] = 0x00;													// Player 2
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
		DrvInput[2] |= (DrvButton[i] & 1) << i;
	}
	ToaClearOpposites(&DrvInput[0]);
	ToaClearOpposites(&DrvInput[1]);

	hold_coin.check(0, DrvInput[2], 1 << 3, 1);
	hold_coin.check(1, DrvInput[2], 1 << 4, 1);

	SekNewFrame();

	nCyclesTotal[0] = (INT32)((INT64)16000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = TOA_Z80_SPEED / 60;
	nCyclesDone[0] = nCyclesDone[1] = 0;

	SekOpen(0);

	SekSetCyclesScanline(nCyclesTotal[0] / 262);

	nToaCyclesDisplayStart = nCyclesTotal[0] - ((nCyclesTotal[0] * (TOA_VBLANK_LINES + 240)) / 262); // 0
	nToaCyclesVBlankStart = nCyclesTotal[0] - ((nCyclesTotal[0] * TOA_VBLANK_LINES) / 262);
	bool bVBlank = false;

	INT32 nSoundBufferPos = 0;

	ZetOpen(0);
	for (INT32 i = 1; i <= nInterleave; i++) {
		// Run 68000
		INT32 nCurrentCPU = 0;
		INT32 nNext = i * nCyclesTotal[nCurrentCPU] / nInterleave;

		// Trigger VBlank interrupt
		if (!bVBlank && nNext > nToaCyclesVBlankStart) {
			if (nCyclesDone[nCurrentCPU] < nToaCyclesVBlankStart) {
				nCyclesSegment = nToaCyclesVBlankStart - nCyclesDone[nCurrentCPU];
				nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
			}

			nIRQPending = 1;
			SekSetIRQLine(4, CPU_IRQSTATUS_AUTO);

			ToaBufferGP9001Sprites();

			if (pBurnDraw) {
				DrvDraw();												// Draw screen if needed
			}

			bVBlank = true;
		}

		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
		nIRQPending = 0;

		// Run Z80
		nCurrentCPU = 1;
		nNext = i * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);

		// Render sound segment
		if (pBurnSoundOut) {
			INT32 nSegmentLength = (nBurnSoundLen * i / nInterleave) - nSoundBufferPos;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			BurnYM2151Render(pSoundBuf, nSegmentLength);
			MSM6295Render(0, pSoundBuf, nSegmentLength);
			nSoundBufferPos += nSegmentLength;
		}
	}

	SekClose();
	
	{
		// Make sure the buffer is entirely filled.
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			if (nSegmentLength) {
				BurnYM2151Render(pSoundBuf, nSegmentLength);
				MSM6295Render(0, pSoundBuf, nSegmentLength);
			}
			BurnSoundTweakVolume(pBurnSoundOut, nBurnSoundLen, 0.65);
		}
	}
	
	ZetClose();

	return 0;
}

struct BurnDriver BurnDrvBgaregga = {
	"bgaregga", NULL, NULL, NULL, "1996",
	"Battle Garegga (Europe / USA / Japan / Asia) (Sat Feb 3 1996)\0", NULL, "Raizing / Eighting", "Toaplan GP9001 based",
	L"\u7A7A\u6218\u4E4B\u8DEF (\u6B27\u7F8E\u65E5\u4E9A)(1996-02-03 \u5468\u516D)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | TOA_ROTATE_GRAPHICS_CCW | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, bgareggaRomInfo, bgareggaRomName, NULL, NULL, NULL, NULL, battlegInputInfo, bgareggaDIPInfo,
	battlegInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvBgareggat = {
	"bgareggat", "bgaregga", NULL, NULL, "1996",
	"Battle Garegga (location test) (Wed Jan 17 1996)\0", NULL, "Raizing / Eighting", "Toaplan GP9001 based",
	L"\u7A7A\u6218\u4E4B\u8DEF (\u5B9A\u4F4D\u6D4B\u8BD5)(1996-01-17 \u5468\u4E09)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW | BDF_PROTOTYPE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, bgareggatRomInfo, bgareggatRomName, NULL, NULL, NULL, NULL, battlegInputInfo, bgareggaDIPInfo,
	BgareggatInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvBgareggaz = {
	"bgareggaz", "bgaregga", NULL, NULL, "2019",
	"Battle Garegga Zakk version (Europe / USA / Japan / Asia) (Sat Feb 3 1996)\0", NULL, "Raizing / Eighting", "Toaplan GP9001 based",
	L"\u7A7A\u6218\u4E4B\u8DEF\u624E\u514B\u7248 (\u6B27\u7F8E\u65E5\u4E9A)(1996-02-03 \u5468\u516D)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, bgareggazRomInfo, bgareggazRomName, NULL, NULL, NULL, NULL, battlegInputInfo, bgareggaDIPInfo,
	battlegInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvBgaregcn = {
	"bgareggacn", "bgaregga", NULL, NULL, "1996",
	"Battle Garegga - Type 2 (Denmark / China) (Tue Apr 2 1996)\0", NULL, "Raizing / Eighting", "Toaplan GP9001 based",
	L"\u7A7A\u6218\u4E4B\u8DEF - Type 2 (\u4E2D\u4E39\u7248)(1996-04-02 \u5468\u56DB)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, bgaregcnRomInfo, bgaregcnRomName, NULL, NULL, NULL, NULL, battlegInputInfo, bgareggacnDIPInfo,
	battlegInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvBgaregt2 = {
	"bgareggat2", "bgaregga", NULL, NULL, "1996",
	"Battle Garegga - Type 2 (Europe / USA / Japan / Asia) (Sat Mar 2 1996)\0", NULL, "Raizing / Eighting", "Toaplan GP9001 based",
	L"\u7A7A\u6218\u4E4B\u8DEF - Type 2 (\u6B27\u7F8E\u65E5\u4E9A)(1996-03-02 \u5468\u516D)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, bgaregt2RomInfo, bgaregt2RomName, NULL, NULL, NULL, NULL, battlegInputInfo, bgareggaDIPInfo,
	battlegInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvBgaregnv = {
	"bgaregganv", "bgaregga", NULL, NULL, "1996",
	"Battle Garegga - New Version (Austria / Hong Kong) (Sat Mar 2 1996)\0", NULL, "Raizing / Eighting", "Toaplan GP9001 based",
	L"\u7A7A\u6218\u4E4B\u8DEF - \u65B0\u7248\u672C (\u6E2F\u6FB3\u7248)(1996-03-02 \u5468\u516D)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, bgaregnvRomInfo, bgaregnvRomName, NULL, NULL, NULL, NULL, battlegInputInfo, bgareggahkDIPInfo,
	battlegInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvBgareggak = {
	"bgareggak", "bgaregga", NULL, NULL, "1996",
	"Battle Garegga (Korea / Greece) (Wed Feb 7 1996)\0", NULL, "Raizing / Eighting", "Toaplan GP9001 based",
	L"\u7A7A\u6218\u4E4B\u8DEF (\u97E9/\u5E0C\u814A\u7248)(1996-02-07 \u5468\u4E09)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, bgareggakRomInfo, bgareggakRomName, NULL, NULL, NULL, NULL, battlegInputInfo, bgareggakDIPInfo,
	battlegInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvBgareghk = {
	"bgareggahk", "bgaregga", NULL, NULL, "1996",
	"Battle Garegga (Austria / Hong Kong) (Sat Feb 3 1996)\0", NULL, "Raizing / Eighting", "Toaplan GP9001 based",
	L"\u7A7A\u6218\u4E4B\u8DEF (\u6E2F\u6FB3\u7248)(1996-02-03 \u5468\u516D)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, bgareghkRomInfo, bgareghkRomName, NULL, NULL, NULL, NULL, battlegInputInfo, bgareggahkDIPInfo,
	battlegInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvBgaregtw = {
	"bgareggatw", "bgaregga", NULL, NULL, "1996",
	"Battle Garegga (Taiwan / Germany) (Thu Feb 1 1996)\0", NULL, "Raizing / Eighting", "Toaplan GP9001 based",
	L"\u7A7A\u6218\u4E4B\u8DEF (\u53F0\u5FB7\u7248)(1996-02-01 \u5468\u56DB)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, bgaregtwRomInfo, bgaregtwRomName, NULL, NULL, NULL, NULL, battlegInputInfo, bgareggatwDIPInfo,
	battlegInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvBgareggabl = {
	"bgareggabl", "bgaregga", NULL, NULL, "1998",
	"1945 Er Dai / 1945 Part-2 (Chinese hack of Battle Garegga)\0", NULL, "bootleg (Melody)", "Toaplan GP9001 based",
	L"1945 \u4E8C\u4EE3 (\u4E2D\u6587)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, bgareggablRomInfo, bgareggablRomName, NULL, NULL, NULL, NULL, battlegInputInfo, bgareggaDIPInfo,
	BgareggablInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvBgareggabla = {
	"bgareggabla", "bgaregga", NULL, NULL, "1997",
	"Leishen Chuan / Thunder Deity Biography (Chinese hack of Battle Garegga)\0", NULL, "bootleg (Melody)", "Toaplan GP9001 based",
	L"\u96F7\u795E\u4F20 (\u4E2D\u6587\u6539\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, bgareggablaRomInfo, bgareggablaRomName, NULL, NULL, NULL, NULL, battlegInputInfo, bgareggaDIPInfo,
	BgareggablaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};

struct BurnDriver BurnDrvBgareggablj = {
	"bgareggablj", "bgaregga", NULL, NULL, "1996",
	"Battle Garegga (Japan, bootleg) (Sat Feb 3 1996)\0", NULL, "bootleg", "Toaplan GP9001 based",
	L"\u7A7A\u6218\u4E4B\u8DEF (\u65E5\u7248, \u76D7\u7248)(1996-02-03 \u5468\u516D)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | TOA_ROTATE_GRAPHICS_CCW | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TOAPLAN_RAIZING, GBF_VERSHOOT, 0,
	NULL, bgareggabljRomInfo, bgareggabljRomName, NULL, NULL, NULL, NULL, battlegInputInfo, bgareggaDIPInfo,
	BgareggablInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	240, 320, 3, 4
};
