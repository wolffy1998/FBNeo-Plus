// FB Neo Data East 8-bit driver module
// Based on MAME driver by Bryan McPhail and Stephane Humbert

#include "tiles_generic.h"
#include "m6502_intf.h"
#include "burn_ym2203.h"
#include "hd6309_intf.h"
#include "m6809_intf.h"
#include "mcs51.h"
#include "burn_ym3812.h"
#include "burn_ym3526.h"
#include "msm5205.h"
#include "bitswap.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *DrvMainROM;
static UINT8 *DrvSubROM;
static UINT8 *DrvMCURom;
static UINT8 *DrvM6502ROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *DrvColPROM;
static UINT8 *DrvMainRAM;
static UINT8 *DrvVidRAM;
static UINT8 *DrvPf0RAM;
static UINT8 *DrvPf1RAM;
static UINT8 *DrvPf0Ctrl;
static UINT8 *DrvPf1Ctrl;
static UINT8 *DrvRowRAM;
static UINT8 *DrvSprRAM;
static UINT8 *DrvSprBuf;
static UINT8 *DrvM6502RAM;
static UINT8 *DrvPalRAM;

static UINT32 *Palette;
static UINT32 *DrvPalette;
static UINT8  DrvRecalc;

static UINT8 *soundlatch;
static UINT8 *nmi_enable;
static UINT8 *interrupt_enable;
static UINT8 *flipscreen;

static INT32 realMCU = 0;
static UINT8 i8751_port0 = 0;
static UINT8 i8751_port1 = 0;
static UINT8 i8751_port2 = 0;
static INT32 i8751_return;
static INT32 i8751_value;
static INT32 vblank;
static INT32 RomBank;

static UINT8 DrvJoy1[8];
static UINT8 DrvJoy2[8];
static UINT8 DrvJoy3[8];
static UINT8 DrvJoy4[8];
static UINT8 DrvJoy5[8];
static UINT8 DrvInputs[5];
static UINT8 DrvDips[3];
static UINT8 DrvReset;

// Rotation stuff! -dink
static UINT8  DrvFakeInput[14]      = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // 0-5 legacy; 6-9 P1, 10-13 P2
static UINT8  nRotateHoldInput[2]   = {0, 0};
static INT32  nRotate[2]            = {0, 0};
static INT32  nRotateTarget[2]      = {0, 0};
static INT32  nRotateTry[2]         = {0, 0};
static UINT32 nRotateTime[2]        = {0, 0};
static UINT8  game_rotates = 0;
static UINT8  nAutoFireCounter[2] 	= {0, 0};

static struct BurnInputInfo GhostbInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy5 + 5,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 0,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",			BIT_DIGITAL,	DrvJoy5 + 6,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy4 + 1,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Ghostb)

static struct BurnDIPInfo GhostbDIPList[]=
{
	{0x11, 0xff, 0xff, 0xf0, NULL					},
	{0x12, 0xff, 0xff, 0xbf, NULL					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x11, 0x01, 0x20, 0x00, "Off"					},
	{0x11, 0x01, 0x20, 0x20, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x11, 0x01, 0x40, 0x40, "Off"					},
	{0x11, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x12, 0x01, 0x03, 0x01, "1"					},
	{0x12, 0x01, 0x03, 0x03, "3"					},
	{0x12, 0x01, 0x03, 0x02, "5"					},
	{0x12, 0x01, 0x03, 0x00, "Infinite (Cheat)"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x12, 0x01, 0x0c, 0x08, "Easy"					},
	{0x12, 0x01, 0x0c, 0x0c, "Normal"				},
	{0x12, 0x01, 0x0c, 0x04, "Hard"					},
	{0x12, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    4, "Max Scene Time"		},
	{0x12, 0x01, 0x30, 0x00, "4:00"					},
	{0x12, 0x01, 0x30, 0x10, "4:30"					},
	{0x12, 0x01, 0x30, 0x30, "5:00"					},
	{0x12, 0x01, 0x30, 0x20, "6:00"					},

	{0   , 0xfe, 0   ,    2, "Allow Continue"		},
	{0x12, 0x01, 0x40, 0x40, "No"					},
	{0x12, 0x01, 0x40, 0x00, "Yes"					},

	{0   , 0xfe, 0   ,    2, "Energy Bonus"			},
	{0x12, 0x01, 0x80, 0x80, "None"					},
	{0x12, 0x01, 0x80, 0x00, "+25%"					},
};

STDDIPINFO(Ghostb)

static struct BurnDIPInfo MeikyuuDIPList[]=
{
	{0x11, 0xff, 0xff, 0xf0, NULL					},
	{0x12, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x11, 0x01, 0x20, 0x00, "Off"					},
	{0x11, 0x01, 0x20, 0x20, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x11, 0x01, 0x40, 0x40, "Off"					},
	{0x11, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x12, 0x01, 0x03, 0x01, "1"					},
	{0x12, 0x01, 0x03, 0x03, "3"					},
	{0x12, 0x01, 0x03, 0x02, "5"					},
	{0x12, 0x01, 0x03, 0x00, "Infinite (Cheat)"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x12, 0x01, 0x0c, 0x08, "Easy"					},
	{0x12, 0x01, 0x0c, 0x0c, "Normal"				},
	{0x12, 0x01, 0x0c, 0x04, "Hard"					},
	{0x12, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Allow Continue"		},
	{0x12, 0x01, 0x40, 0x40, "No"					},
	{0x12, 0x01, 0x40, 0x00, "Yes"					},

	{0   , 0xfe, 0   ,    2, "Energy Bonus"			},
	{0x12, 0x01, 0x10, 0x10, "None"					},
	{0x12, 0x01, 0x10, 0x00, "+50%"					},

	{0   , 0xfe, 0   ,    2, "Freeze"				},
	{0x12, 0x01, 0x80, 0x80, "Off"					},
	{0x12, 0x01, 0x80, 0x00, "On"					},
};

STDDIPINFO(Meikyuu)

static struct BurnInputInfo Ghostb3InputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy5 + 5,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 0,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",			BIT_DIGITAL,	DrvJoy5 + 6,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy4 + 1,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"P3 Coin",			BIT_DIGITAL,	DrvJoy5 + 7,	"p3 coin"	},
	{"P3 Start",		BIT_DIGITAL,	DrvJoy4 + 2,	"p3 start"	},
	{"P3 Up",			BIT_DIGITAL,	DrvJoy3 + 0,	"p3 up"		},
	{"P3 Down",			BIT_DIGITAL,	DrvJoy3 + 1,	"p3 down"	},
	{"P3 Left",			BIT_DIGITAL,	DrvJoy3 + 2,	"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	DrvJoy3 + 3,	"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p3 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Ghostb3)

static struct BurnDIPInfo Ghostb3DIPList[]=
{
	{0x19, 0xff, 0xff, 0xf0, NULL					},
	{0x1a, 0xff, 0xff, 0xbf, NULL					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x19, 0x01, 0x20, 0x00, "Off"					},
	{0x19, 0x01, 0x20, 0x20, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x19, 0x01, 0x40, 0x40, "Off"					},
	{0x19, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x1a, 0x01, 0x03, 0x01, "1"					},
	{0x1a, 0x01, 0x03, 0x03, "3"					},
	{0x1a, 0x01, 0x03, 0x02, "5"					},
	{0x1a, 0x01, 0x03, 0x00, "Infinite (Cheat)"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x1a, 0x01, 0x0c, 0x08, "Easy"					},
	{0x1a, 0x01, 0x0c, 0x0c, "Normal"				},
	{0x1a, 0x01, 0x0c, 0x04, "Hard"					},
	{0x1a, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    4, "Max Scene Time"		},
	{0x1a, 0x01, 0x30, 0x00, "4:00"					},
	{0x1a, 0x01, 0x30, 0x10, "4:30"					},
	{0x1a, 0x01, 0x30, 0x30, "5:00"					},
	{0x1a, 0x01, 0x30, 0x20, "6:00"					},

	{0   , 0xfe, 0   ,    2, "Allow Continue"		},
	{0x1a, 0x01, 0x40, 0x40, "No"					},
	{0x1a, 0x01, 0x40, 0x00, "Yes"					},

	{0   , 0xfe, 0   ,    2, "Energy Bonus"			},
	{0x1a, 0x01, 0x80, 0x80, "None"					},
	{0x1a, 0x01, 0x80, 0x00, "+50%"					},
};

STDDIPINFO(Ghostb3)

static struct BurnInputInfo CobracomInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",			BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Cobracom)

static struct BurnDIPInfo CobracomDIPList[]=
{
	{0x11, 0xff, 0xff, 0xff, NULL					},
	{0x12, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    4, "Coin A"				},
	{0x11, 0x01, 0x03, 0x00, "3 Coins 1 Credit"		},
	{0x11, 0x01, 0x03, 0x01, "2 Coins 1 Credit"		},
	{0x11, 0x01, 0x03, 0x03, "1 Coin  1 Credit"		},
	{0x11, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"				},
	{0x11, 0x01, 0x0c, 0x00, "3 Coins 1 Credit"		},
	{0x11, 0x01, 0x0c, 0x04, "2 Coins 1 Credit"		},
	{0x11, 0x01, 0x0c, 0x0c, "1 Coin  1 Credit"		},
	{0x11, 0x01, 0x0c, 0x08, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x11, 0x01, 0x20, 0x00, "Off"					},
	{0x11, 0x01, 0x20, 0x20, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x11, 0x01, 0x40, 0x40, "Off"					},
	{0x11, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Cabinet"				},
	{0x11, 0x01, 0x80, 0x00, "Upright"				},
	{0x11, 0x01, 0x80, 0x80, "Cocktail"				},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x12, 0x01, 0x03, 0x03, "3"					},
	{0x12, 0x01, 0x03, 0x02, "4"					},
	{0x12, 0x01, 0x03, 0x01, "5"					},
	{0x12, 0x01, 0x03, 0x00, "99 (Cheat)"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x12, 0x01, 0x0c, 0x08, "Easy"					},
	{0x12, 0x01, 0x0c, 0x0c, "Normal"				},
	{0x12, 0x01, 0x0c, 0x04, "Hard"					},
	{0x12, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Allow Continue"		},
	{0x12, 0x01, 0x10, 0x00, "No"					},
	{0x12, 0x01, 0x10, 0x10, "Yes"					},

	{0   , 0xfe, 0   ,    2, "Bonus Life"			},
	{0x12, 0x01, 0x20, 0x20, "50k, 150k"			},
	{0x12, 0x01, 0x20, 0x00, "100k, 200k"			},
};

STDDIPINFO(Cobracom)

static struct BurnInputInfo SrdarwinInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy5 + 5,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy1 + 2,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy1 + 3,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy1 + 1,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",			BIT_DIGITAL,	DrvJoy5 + 6,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 2,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy2 + 3,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy2 + 1,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Srdarwin)

static struct BurnDIPInfo SrdarwinDIPList[]=
{
	{0x11, 0xff, 0xff, 0xff, NULL					},
	{0x12, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    4, "Coin A"				},
	{0x11, 0x01, 0x03, 0x00, "1 Coin  6 Credits"	},
	{0x11, 0x01, 0x03, 0x01, "1 Coin  4 Credits"	},
	{0x11, 0x01, 0x03, 0x02, "1 Coin  3 Credits"	},
	{0x11, 0x01, 0x03, 0x03, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"				},
	{0x11, 0x01, 0x0c, 0x00, "4 Coins 1 Credit"		},
	{0x11, 0x01, 0x0c, 0x04, "3 Coins 1 Credit"		},
	{0x11, 0x01, 0x0c, 0x08, "2 Coins 1 Credit"		},
	{0x11, 0x01, 0x0c, 0x0c, "1 Coin  1 Credit"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x11, 0x01, 0x20, 0x00, "Off"					},
	{0x11, 0x01, 0x20, 0x20, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x11, 0x01, 0x40, 0x40, "Off"					},
	{0x11, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Cabinet"				},
	{0x11, 0x01, 0x80, 0x00, "Upright"				},
	{0x11, 0x01, 0x80, 0x80, "Cocktail"				},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x12, 0x01, 0x03, 0x01, "1"					},
	{0x12, 0x01, 0x03, 0x03, "3"					},
	{0x12, 0x01, 0x03, 0x02, "5"					},
	{0x12, 0x01, 0x03, 0x00, "28 (Cheat)"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x12, 0x01, 0x0c, 0x08, "Easy"					},
	{0x12, 0x01, 0x0c, 0x0c, "Normal"				},
	{0x12, 0x01, 0x0c, 0x04, "Hard"					},
	{0x12, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Bonus Life"			},
	{0x12, 0x01, 0x10, 0x10, "Every 50k"			},
	{0x12, 0x01, 0x10, 0x00, "Every 100k"			},

	{0   , 0xfe, 0   ,    2, "After Stage 10"		},
	{0x12, 0x01, 0x20, 0x20, "Back to Stage 1"		},
	{0x12, 0x01, 0x20, 0x00, "Game Over"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"		},
	{0x12, 0x01, 0x80, 0x00, "No"					},
	{0x12, 0x01, 0x80, 0x80, "Yes"					},
};

STDDIPINFO(Srdarwin)

static struct BurnDIPInfo SrdarwinjDIPList[]=
{
	{0x11, 0xff, 0xff, 0xff, NULL					},
	{0x12, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    4, "Coin A"				},
	{0x11, 0x01, 0x03, 0x00, "2 Coins 1 Credits"	},
	{0x11, 0x01, 0x03, 0x03, "1 Coin  1 Credits"	},
	{0x11, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},
	{0x11, 0x01, 0x03, 0x01, "1 Coin  3 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"				},
	{0x11, 0x01, 0x0c, 0x00, "2 Coins 1 Credits"	},
	{0x11, 0x01, 0x0c, 0x0c, "1 Coin  1 Credits"	},
	{0x11, 0x01, 0x0c, 0x08, "1 Coin  2 Credits"	},
	{0x11, 0x01, 0x0c, 0x04, "1 Coin  3 Credits"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x11, 0x01, 0x20, 0x00, "Off"					},
	{0x11, 0x01, 0x20, 0x20, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x11, 0x01, 0x40, 0x40, "Off"					},
	{0x11, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Cabinet"				},
	{0x11, 0x01, 0x80, 0x00, "Upright"				},
	{0x11, 0x01, 0x80, 0x80, "Cocktail"				},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x12, 0x01, 0x03, 0x01, "1"					},
	{0x12, 0x01, 0x03, 0x03, "3"					},
	{0x12, 0x01, 0x03, 0x02, "5"					},
	{0x12, 0x01, 0x03, 0x00, "28 (Cheat)"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x12, 0x01, 0x0c, 0x08, "Easy"					},
	{0x12, 0x01, 0x0c, 0x0c, "Normal"				},
	{0x12, 0x01, 0x0c, 0x04, "Hard"					},
	{0x12, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Bonus Life"			},
	{0x12, 0x01, 0x10, 0x10, "Every 50k"			},
	{0x12, 0x01, 0x10, 0x00, "Every 100k"			},

	{0   , 0xfe, 0   ,    2, "After Stage 10"		},
	{0x12, 0x01, 0x20, 0x20, "Back to Stage 1"		},
	{0x12, 0x01, 0x20, 0x00, "Game Over"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"		},
	{0x12, 0x01, 0x80, 0x00, "No"					},
	{0x12, 0x01, 0x80, 0x80, "Yes"					},
};

STDDIPINFO(Srdarwinj)

static struct BurnInputInfo GondoInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy5 + 5,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy4 + 0,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy3 + 0,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy3 + 1,	"p1 fire 2"	},
	{"P1 Fire 3 (Rotate)", BIT_DIGITAL,	DrvFakeInput + 4,  "p1 fire 3" },
	{"P1 Rotate Left",	BIT_DIGITAL,	DrvFakeInput + 0,  "p1 rotate left" },
	{"P1 Rotate Right",	BIT_DIGITAL,	DrvFakeInput + 1,  "p1 rotate right" },
	{"P1 Shoot Up"       	, BIT_DIGITAL  , DrvFakeInput + 6,  "p1 up 2" }, // 6
	{"P1 Shoot Down"      	, BIT_DIGITAL  , DrvFakeInput + 7,  "p1 down 2" }, // 7
	{"P1 Shoot Left"       	, BIT_DIGITAL  , DrvFakeInput + 8,  "p1 left 2" }, // 8
	{"P1 Shoot Right"      	, BIT_DIGITAL  , DrvFakeInput + 9,  "p1 right 2" }, // 9

	{"P2 Coin",			BIT_DIGITAL,	DrvJoy5 + 6,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy4 + 1,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy3 + 2,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 fire 2"	},
	{"P2 Fire 3 (Rotate)", BIT_DIGITAL,	DrvFakeInput + 5,  "p2 fire 3" },
	{"P2 Rotate Left",	BIT_DIGITAL,	DrvFakeInput + 2,  "p2 rotate left" },
	{"P2 Rotate Right",	BIT_DIGITAL,	DrvFakeInput + 3,  "p2 rotate right" },
	{"P2 Shoot Up"       	, BIT_DIGITAL  , DrvFakeInput + 10, "p2 up 2" },
	{"P2 Shoot Down"      	, BIT_DIGITAL  , DrvFakeInput + 11, "p2 down 2" },
	{"P2 Shoot Left"       	, BIT_DIGITAL  , DrvFakeInput + 12, "p2 left 2" },
	{"P2 Shoot Right"      	, BIT_DIGITAL  , DrvFakeInput + 13, "p2 right 2" },

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	// Auto-fire on right-stick
	{"Dip C", 			BIT_DIPSWITCH,	DrvDips + 2, 	"dip"       },
};

STDINPUTINFO(Gondo)

static struct BurnDIPInfo GondoDIPList[]=
{
	DIP_OFFSET(0x1f)
	{0x00, 0xff, 0xff, 0xff, NULL					},
	{0x01, 0xff, 0xff, 0xef, NULL					},
	{0x02, 0xff, 0xff, 0x00, NULL                   },

	{0   , 0xfe, 0   ,    4, "Coin A"				},
	{0x00, 0x01, 0x03, 0x00, "2 Coins 1 Credits"	},
	{0x00, 0x01, 0x03, 0x03, "1 Coin  1 Credits"	},
	{0x00, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},
	{0x00, 0x01, 0x03, 0x01, "1 Coin  3 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"				},
	{0x00, 0x01, 0x0c, 0x00, "2 Coins 1 Credits"	},
	{0x00, 0x01, 0x0c, 0x0c, "1 Coin  1 Credits"	},
	{0x00, 0x01, 0x0c, 0x08, "1 Coin  2 Credits"	},
	{0x00, 0x01, 0x0c, 0x04, "1 Coin  3 Credits"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x00, 0x01, 0x20, 0x00, "Off"					},
	{0x00, 0x01, 0x20, 0x20, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x00, 0x01, 0x40, 0x40, "Off"					},
	{0x00, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Swap Buttons"			},
	{0x00, 0x01, 0x80, 0x80, "Off"					},
	{0x00, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x01, 0x01, 0x03, 0x01, "1"					},
	{0x01, 0x01, 0x03, 0x03, "3"					},
	{0x01, 0x01, 0x03, 0x02, "5"					},
	{0x01, 0x01, 0x03, 0x00, "Infinite (Cheat)"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x01, 0x01, 0x0c, 0x08, "Easy"					},
	{0x01, 0x01, 0x0c, 0x0c, "Normal"				},
	{0x01, 0x01, 0x0c, 0x04, "Hard"					},
	{0x01, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Allow Continue"		},
	{0x01, 0x01, 0x10, 0x10, "No"					},
	{0x01, 0x01, 0x10, 0x00, "Yes"					},

	// Dip 3
	{0   , 0xfe, 0   , 2   , "Second Stick"           },
	{0x02, 0x01, 0x01, 0x00, "Moves & Shoots"         },
	{0x02, 0x01, 0x01, 0x01, "Moves"                  },
};

STDDIPINFO(Gondo)

static struct BurnInputInfo GaryoretInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy5 + 5,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",			BIT_DIGITAL,	DrvJoy5 + 6,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Garyoret)

static struct BurnDIPInfo GaryoretDIPList[]=
{
	{0x11, 0xff, 0xff, 0xff, NULL					},
	{0x12, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    4, "Coin A"				},
	{0x11, 0x01, 0x03, 0x00, "2 Coins 1 Credit"		},
	{0x11, 0x01, 0x03, 0x01, "1 Coin  3 Credits"	},
	{0x11, 0x01, 0x03, 0x03, "1 Coin  1 Credit"		},
	{0x11, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"				},
	{0x11, 0x01, 0x0c, 0x00, "2 Coins 1 Credit"		},
	{0x11, 0x01, 0x0c, 0x04, "1 Coin  3 Credits"	},
	{0x11, 0x01, 0x0c, 0x0c, "1 Coin  1 Credit"		},
	{0x11, 0x01, 0x0c, 0x08, "1 Coin  2 Credits"	},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x11, 0x01, 0x20, 0x00, "Off"					},
	{0x11, 0x01, 0x20, 0x20, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x11, 0x01, 0x40, 0x40, "Off"					},
	{0x11, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Lives"				},
	{0x12, 0x01, 0x01, 0x01, "3"					},
	{0x12, 0x01, 0x01, 0x00, "5"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x12, 0x01, 0x0c, 0x08, "Easy"					},
	{0x12, 0x01, 0x0c, 0x0c, "Normal"				},
	{0x12, 0x01, 0x0c, 0x04, "Hard"					},
	{0x12, 0x01, 0x0c, 0x00, "Hardest"				},
};

STDDIPINFO(Garyoret)

static struct BurnInputInfo OscarInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy3 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",			BIT_DIGITAL,	DrvJoy3 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy2 + 7,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Oscar)

static struct BurnDIPInfo OscarDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL					},
	{0x14, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    4, "Coin A"				},
	{0x13, 0x01, 0x03, 0x03, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x03, 0x02, "1 Coin  3 Credits"	},
	{0x13, 0x01, 0x03, 0x01, "1 Coin  4 Credits"	},
	{0x13, 0x01, 0x03, 0x00, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"				},
	{0x13, 0x01, 0x0c, 0x00, "4 Coins 1 Credits"	},
	{0x13, 0x01, 0x0c, 0x04, "3 Coins 1 Credits"	},
	{0x13, 0x01, 0x0c, 0x08, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x0c, 0x0c, "1 Coin  1 Credits"	},

	{0   , 0xfe, 0   ,    2, "Freeze Mode"			},
	{0x13, 0x01, 0x10, 0x10, "Off"					},
	{0x13, 0x01, 0x10, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x13, 0x01, 0x20, 0x00, "Off"					},
	{0x13, 0x01, 0x20, 0x20, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x13, 0x01, 0x40, 0x40, "Off"					},
	{0x13, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Cabinet"				},
	{0x13, 0x01, 0x80, 0x00, "Upright"				},
	{0x13, 0x01, 0x80, 0x80, "Cocktail"				},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x14, 0x01, 0x03, 0x01, "1"					},
	{0x14, 0x01, 0x03, 0x03, "3"					},
	{0x14, 0x01, 0x03, 0x02, "5"					},
	{0x14, 0x01, 0x03, 0x00, "Infinite (Cheat)"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x14, 0x01, 0x0c, 0x08, "Easy"					},
	{0x14, 0x01, 0x0c, 0x0c, "Normal"				},
	{0x14, 0x01, 0x0c, 0x04, "Hard"					},
	{0x14, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    4, "Bonus Life"			},
	{0x14, 0x01, 0x30, 0x30, "40k 100k 60k+"		},
	{0x14, 0x01, 0x30, 0x20, "60k 160k 100k+"		},
	{0x14, 0x01, 0x30, 0x10, "90k 240k 150k+"		},
	{0x14, 0x01, 0x30, 0x00, "50k only"				},

	{0   , 0xfe, 0   ,    2, "Invulnerability (Cheat)"},
	{0x14, 0x01, 0x40, 0x40, "Off"					},
	{0x14, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Allow Continue"		},
	{0x14, 0x01, 0x80, 0x00, "No"					},
	{0x14, 0x01, 0x80, 0x80, "Yes"					},
};

STDDIPINFO(Oscar)

static struct BurnDIPInfo OscarjDIPList[]=
{
	{0x13, 0xff, 0xff, 0xff, NULL					},
	{0x14, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    4, "Coin A"				},
	{0x13, 0x01, 0x03, 0x00, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x03, 0x03, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x03, 0x02, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x03, 0x01, "1 Coin  3 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"				},
	{0x13, 0x01, 0x0c, 0x00, "2 Coins 1 Credits"	},
	{0x13, 0x01, 0x0c, 0x0c, "1 Coin  1 Credits"	},
	{0x13, 0x01, 0x0c, 0x08, "1 Coin  2 Credits"	},
	{0x13, 0x01, 0x0c, 0x04, "1 Coin  3 Credits"	},

	{0   , 0xfe, 0   ,    2, "Freeze Mode"			},
	{0x13, 0x01, 0x10, 0x10, "Off"					},
	{0x13, 0x01, 0x10, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x13, 0x01, 0x20, 0x00, "Off"					},
	{0x13, 0x01, 0x20, 0x20, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x13, 0x01, 0x40, 0x40, "Off"					},
	{0x13, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Cabinet"				},
	{0x13, 0x01, 0x80, 0x00, "Upright"				},
	{0x13, 0x01, 0x80, 0x80, "Cocktail"				},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x14, 0x01, 0x03, 0x01, "1"					},
	{0x14, 0x01, 0x03, 0x03, "3"					},
	{0x14, 0x01, 0x03, 0x02, "5"					},
	{0x14, 0x01, 0x03, 0x00, "Infinite (Cheat)"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x14, 0x01, 0x0c, 0x08, "Easy"					},
	{0x14, 0x01, 0x0c, 0x0c, "Normal"				},
	{0x14, 0x01, 0x0c, 0x04, "Hard"					},
	{0x14, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    4, "Bonus Life"			},
	{0x14, 0x01, 0x30, 0x30, "40k 100k 60k+"		},
	{0x14, 0x01, 0x30, 0x20, "60k 160k 100k+"		},
	{0x14, 0x01, 0x30, 0x10, "90k 240k 150k+"		},
	{0x14, 0x01, 0x30, 0x00, "50k only"				},

	{0   , 0xfe, 0   ,    2, "Invulnerability (Cheat)"},
	{0x14, 0x01, 0x40, 0x40, "Off"					},
	{0x14, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Allow Continue"		},
	{0x14, 0x01, 0x80, 0x00, "No"					},
	{0x14, 0x01, 0x80, 0x80, "Yes"					},
};

STDDIPINFO(Oscarj)


static struct BurnInputInfo LastmisnInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy5 + 5,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",			BIT_DIGITAL,	DrvJoy5 + 6,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy2 + 6,	"p2 fire 3"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Lastmisn)

static struct BurnDIPInfo LastmisnDIPList[]=
{
	{0x13, 0xff, 0xff, 0xef, NULL						},
	{0x14, 0xff, 0xff, 0x77, NULL						},

	{0   , 0xfe, 0   ,    4, "Coin A"					},
	{0x13, 0x01, 0x03, 0x00, "1 Coin  5 Credits"		},
	{0x13, 0x01, 0x03, 0x03, "1 Coin  1 Credits"		},
	{0x13, 0x01, 0x03, 0x02, "1 Coin  2 Credits"		},
	{0x13, 0x01, 0x03, 0x01, "1 Coin  3 Credits"		},

	{0   , 0xfe, 0   ,    4, "Coin B"					},
	{0x13, 0x01, 0x0c, 0x00, "4 Coins 1 Credits"		},
	{0x13, 0x01, 0x0c, 0x0c, "1 Coin  1 Credits"		},
	{0x13, 0x01, 0x0c, 0x08, "2 Coins 1 Credits"		},
	{0x13, 0x01, 0x0c, 0x04, "3 Coins 1 Credits"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x13, 0x01, 0x10, 0x10, "Off"						},
	{0x13, 0x01, 0x10, 0x00, "On"						},

	{0   , 0xfe, 0   ,    2, "Cabinet"					},
	{0x13, 0x01, 0x20, 0x00, "Upright"					},
	{0x13, 0x01, 0x20, 0x20, "Cocktail"					},

	{0   , 0xfe, 0   ,    2, "Invulnerability (Cheat)"	},
	{0x13, 0x01, 0x40, 0x40, "Off"						},
	{0x13, 0x01, 0x40, 0x00, "On"						},

	{0   , 0xfe, 0   ,    2, "Infinite Lives (Cheat)"	},
	{0x13, 0x01, 0x80, 0x80, "Off"						},
	{0x13, 0x01, 0x80, 0x00, "On"						},

	{0   , 0xfe, 0   ,    2, "Lives"					},
	{0x14, 0x01, 0x01, 0x01, "3"						},
	{0x14, 0x01, 0x01, 0x00, "5"						},

	{0   , 0xfe, 0   ,    4, "Bonus Life"				},
	{0x14, 0x01, 0x06, 0x06, "30k, 70k then every 70k"	},
	{0x14, 0x01, 0x06, 0x04, "40k, 90k then every 90k"	},
	{0x14, 0x01, 0x06, 0x02, "40k and 80k"				},
	{0x14, 0x01, 0x06, 0x00, "50k only"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x14, 0x01, 0x18, 0x18, "Easy"						},
	{0x14, 0x01, 0x18, 0x10, "Normal"					},
	{0x14, 0x01, 0x18, 0x08, "Hard"						},
	{0x14, 0x01, 0x18, 0x00, "Hardest"					},

	{0   , 0xfe, 0   ,    2, "Allow Continue"			},
	{0x14, 0x01, 0x80, 0x80, "No"						},
	{0x14, 0x01, 0x80, 0x00, "Yes"						},
};

STDDIPINFO(Lastmisn)

static struct BurnDIPInfo LastmsnjDIPList[]=
{
	{0x13, 0xff, 0xff, 0xef, NULL						},
	{0x14, 0xff, 0xff, 0x7f, NULL						},

	{0   , 0xfe, 0   ,    4, "Coin A"					},
	{0x13, 0x01, 0x03, 0x00, "1 Coin  5 Credits"		},
	{0x13, 0x01, 0x03, 0x03, "1 Coin  1 Credits"		},
	{0x13, 0x01, 0x03, 0x02, "1 Coin  2 Credits"		},
	{0x13, 0x01, 0x03, 0x01, "1 Coin  3 Credits"		},

	{0   , 0xfe, 0   ,    4, "Coin B"					},
	{0x13, 0x01, 0x0c, 0x00, "4 Coins 1 Credits"		},
	{0x13, 0x01, 0x0c, 0x0c, "1 Coin  1 Credits"		},
	{0x13, 0x01, 0x0c, 0x08, "2 Coins 1 Credits"		},
	{0x13, 0x01, 0x0c, 0x04, "3 Coins 1 Credits"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"				},
	{0x13, 0x01, 0x10, 0x10, "Off"						},
	{0x13, 0x01, 0x10, 0x00, "On"						},

	{0   , 0xfe, 0   ,    2, "Cabinet"					},
	{0x13, 0x01, 0x20, 0x00, "Upright"					},
	{0x13, 0x01, 0x20, 0x20, "Cocktail"					},

	{0   , 0xfe, 0   ,    2, "Invulnerability (Cheat)"	},
	{0x13, 0x01, 0x40, 0x40, "Off"						},
	{0x13, 0x01, 0x40, 0x00, "On"						},

	{0   , 0xfe, 0   ,    2, "Infinite Lives (Cheat)"	},
	{0x13, 0x01, 0x80, 0x80, "Off"						},
	{0x13, 0x01, 0x80, 0x00, "On"						},

	{0   , 0xfe, 0   ,    2, "Lives"					},
	{0x14, 0x01, 0x01, 0x01, "3"						},
	{0x14, 0x01, 0x01, 0x00, "5"						},

	{0   , 0xfe, 0   ,    4, "Bonus Life"				},
	{0x14, 0x01, 0x06, 0x06, "30k 50k 50k+"				},
	{0x14, 0x01, 0x06, 0x04, "30k 70k 70k+"				},
	{0x14, 0x01, 0x06, 0x02, "50k 100k 100k+"			},
	{0x14, 0x01, 0x06, 0x00, "50k only"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"				},
	{0x14, 0x01, 0x18, 0x18, "Normal"					},
	{0x14, 0x01, 0x18, 0x10, "Difficult"				},
	{0x14, 0x01, 0x18, 0x08, "Very Difficult"			},
	{0x14, 0x01, 0x18, 0x00, "Top Difficult"			},

	{0   , 0xfe, 0   ,    2, "Allow Continue"			},
	{0x14, 0x01, 0x80, 0x80, "No"						},
	{0x14, 0x01, 0x80, 0x00, "Yes"						},
};

STDDIPINFO(Lastmsnj)

static struct BurnInputInfo ShackledInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy5 + 5,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",			BIT_DIGITAL,	DrvJoy5 + 6,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Shackled)

static struct BurnDIPInfo ShackledDIPList[]=
{
	{0x11, 0xff, 0xff, 0xff, NULL					},
	{0x12, 0xff, 0xff, 0x7f, NULL					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x11, 0x01, 0x01, 0x01, "Off"					},
	{0x11, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Leave Off"			},
	{0x11, 0x01, 0x10, 0x10, "Off"					},
	{0x11, 0x01, 0x10, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Freeze"				},
	{0x11, 0x01, 0x80, 0x80, "Off"					},
	{0x11, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    8, "Coin/Heart/Help/6-Help"},
	{0x12, 0x01, 0x07, 0x00, "2/100/50/200"			},
	{0x12, 0x01, 0x07, 0x01, "4/100/60/300"			},
	{0x12, 0x01, 0x07, 0x02, "6/200/70/300"			},
	{0x12, 0x01, 0x07, 0x03, "8/200/80/400"			},
	{0x12, 0x01, 0x07, 0x07, "10/200/100/500"		},
	{0x12, 0x01, 0x07, 0x06, "12/300/100/600"		},
	{0x12, 0x01, 0x07, 0x05, "18/400/200/700"		},
	{0x12, 0x01, 0x07, 0x04, "20/500/200/800"		},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x12, 0x01, 0x30, 0x30, "Normal"				},
	{0x12, 0x01, 0x30, 0x20, "Hard"					},
	{0x12, 0x01, 0x30, 0x10, "Very Hard"			},
	{0x12, 0x01, 0x30, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x12, 0x01, 0x80, 0x80, "Off"					},
	{0x12, 0x01, 0x80, 0x00, "On"					},
};

STDDIPINFO(Shackled)

static struct BurnDIPInfo BreywoodDIPList[]=
{
	{0x11, 0xff, 0xff, 0xff, NULL					},
	{0x12, 0xff, 0xff, 0x7f, NULL					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x11, 0x01, 0x01, 0x01, "Off"					},
	{0x11, 0x01, 0x01, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Leave Off"			},
	{0x11, 0x01, 0x10, 0x10, "Off"					},
	{0x11, 0x01, 0x10, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Freeze"				},
	{0x11, 0x01, 0x80, 0x80, "Off"					},
	{0x11, 0x01, 0x80, 0x00, "On"					},

	{0   , 0xfe, 0   ,    16, "Power"				},
	{0x12, 0x01, 0x0f, 0x07, "200"					},
	{0x12, 0x01, 0x0f, 0x0b, "300"					},
	{0x12, 0x01, 0x0f, 0x03, "400"					},
	{0x12, 0x01, 0x0f, 0x0d, "500"					},
	{0x12, 0x01, 0x0f, 0x05, "600"					},
	{0x12, 0x01, 0x0f, 0x09, "700"					},
	{0x12, 0x01, 0x0f, 0x01, "800"					},
	{0x12, 0x01, 0x0f, 0x0e, "900"					},
	{0x12, 0x01, 0x0f, 0x0f, "1000"					},
	{0x12, 0x01, 0x0f, 0x06, "2000"					},
	{0x12, 0x01, 0x0f, 0x0a, "3000"					},
	{0x12, 0x01, 0x0f, 0x02, "4000"					},
	{0x12, 0x01, 0x0f, 0x0c, "5000"					},
	{0x12, 0x01, 0x0f, 0x04, "6000"					},
	{0x12, 0x01, 0x0f, 0x08, "7000"					},
	{0x12, 0x01, 0x0f, 0x00, "8000"					},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x12, 0x01, 0x30, 0x30, "Normal"				},
	{0x12, 0x01, 0x30, 0x20, "Hard"					},
	{0x12, 0x01, 0x30, 0x10, "Very Hard"			},
	{0x12, 0x01, 0x30, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x12, 0x01, 0x80, 0x80, "Off"					},
	{0x12, 0x01, 0x80, 0x00, "On"					},
};

STDDIPINFO(Breywood)

static struct BurnInputInfo CsilverInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy5 + 5,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy3 + 2,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",			BIT_DIGITAL,	DrvJoy5 + 6,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy3 + 3,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Csilver)

static struct BurnDIPInfo CsilverDIPList[]=
{
	{0x11, 0xff, 0xff, 0xff, NULL					},
	{0x12, 0xff, 0xff, 0xff, NULL					},

	{0   , 0xfe, 0   ,    4, "Coin A"				},
	{0x11, 0x01, 0x03, 0x03, "1 Coin  2 Credits"	},
	{0x11, 0x01, 0x03, 0x02, "1 Coin  3 Credits"	},
	{0x11, 0x01, 0x03, 0x01, "1 Coin  4 Credits"	},
	{0x11, 0x01, 0x03, 0x00, "1 Coin  6 Credits"	},

	{0   , 0xfe, 0   ,    4, "Coin B"				},
	{0x11, 0x01, 0x0c, 0x00, "4 Coins 1 Credit"		},
	{0x11, 0x01, 0x0c, 0x04, "3 Coins 1 Credit"		},
	{0x11, 0x01, 0x0c, 0x08, "2 Coins 1 Credit"		},
	{0x11, 0x01, 0x0c, 0x0c, "1 Coin  1 Credit"		},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"			},
	{0x11, 0x01, 0x20, 0x00, "Off"					},
	{0x11, 0x01, 0x20, 0x20, "On"					},

	{0   , 0xfe, 0   ,    2, "Flip Screen"			},
	{0x11, 0x01, 0x40, 0x40, "Off"					},
	{0x11, 0x01, 0x40, 0x00, "On"					},

	{0   , 0xfe, 0   ,    2, "Cabinet"				},
	{0x11, 0x01, 0x80, 0x00, "Upright"				},
	{0x11, 0x01, 0x80, 0x80, "Cocktail"				},

	{0   , 0xfe, 0   ,    4, "Lives"				},
	{0x12, 0x01, 0x03, 0x01, "1"					},
	{0x12, 0x01, 0x03, 0x03, "3"					},
	{0x12, 0x01, 0x03, 0x02, "5"					},
	{0x12, 0x01, 0x03, 0x00, "255 (Cheat)"			},

	{0   , 0xfe, 0   ,    4, "Difficulty"			},
	{0x12, 0x01, 0x0c, 0x08, "Easy"					},
	{0x12, 0x01, 0x0c, 0x0c, "Normal"				},
	{0x12, 0x01, 0x0c, 0x04, "Hard"					},
	{0x12, 0x01, 0x0c, 0x00, "Hardest"				},

	{0   , 0xfe, 0   ,    2, "Allow Continue"		},
	{0x12, 0x01, 0x10, 0x00, "No"					},
	{0x12, 0x01, 0x10, 0x10, "Yes"					},

	{0   , 0xfe, 0   ,    2, "No Key for Door (Cheat)"},
	{0x12, 0x01, 0x40, 0x40, "Off"					},
	{0x12, 0x01, 0x40, 0x00, "On"					},
};

STDDIPINFO(Csilver)

static void DrvMCUSync(); // forward

// i8751 MCU
static void i8751_write(INT32 offset, INT32 data)
{
	if (realMCU == 0) return;

	DrvMCUSync();

	switch (offset)
	{
		case 0:
			i8751_value = (i8751_value & 0x00ff) | (data << 8);
			mcs51_set_irq_line(MCS51_INT1_LINE, CPU_IRQSTATUS_HOLD);
		break;

		case 1:
			i8751_value = (i8751_value & 0xff00) | (data << 0);
		break;
	}
}

static UINT8 i8751_hi()
{
	DrvMCUSync();

	return i8751_return >> 8;
}

static UINT8 i8751_lo()
{
	DrvMCUSync();

	return i8751_return & 0xff;
}

static UINT8 mcu_read_port(INT32 port)
{
	switch (port) {
		case MCS51_PORT_P0:
			return i8751_port0;
		case MCS51_PORT_P1:
			return i8751_port1;
		case MCS51_PORT_P2:
			return i8751_port2;
		case MCS51_PORT_P3:
			return DrvInputs[4] & 0xf0; // coin inputs
	}

	return 0xff;
}

// used: ghostb, gondo, garyoret
static void mcu_write_port_ghostb(INT32 port, UINT8 data)
{
	switch (port) {
		case MCS51_PORT_P0:
			i8751_port0 = data;
			break;
		case MCS51_PORT_P1:
			i8751_port1 = data;
			break;
		case MCS51_PORT_P2:
			if (~data & 0x10) {
				i8751_port0 = i8751_value >> 8;
			}
			if (~data & 0x20) {
				i8751_port1 = i8751_value & 0xff;
			}
			if (~data & 0x40) {
				i8751_return = (i8751_return & 0x00ff) | (i8751_port0 << 8);
			}
			if (~data & 0x80) {
				i8751_return = (i8751_return & 0xff00) | i8751_port1;
			}
			if (~data & 0x04 && *interrupt_enable) {
				HD6309SetIRQLine(0, CPU_IRQSTATUS_ACK);
			}
			i8751_port2 = data;
			break;
	}
}

static void mcu_write_port_srdarwin(INT32 port, UINT8 data)
{
	switch (port) {
		case MCS51_PORT_P0:
			i8751_port0 = data;
			break;
		case MCS51_PORT_P1:
			i8751_port1 = data;
			break;
		case MCS51_PORT_P2:
			if (~data & 0x10) {
				i8751_port0 = i8751_value >> 8;
			}
			if (~data & 0x20) {
				i8751_port0 = i8751_value & 0xff;
			}
			if (~data & 0x40) {
				i8751_return = (i8751_return & 0x00ff) | (i8751_port0 << 8);
			}
			if (~data & 0x80) {
				i8751_return = (i8751_return & 0xff00) | i8751_port0;
			}
			if (~data & 0x02) {
				M6809SetIRQLine(0, CPU_IRQSTATUS_NONE);
			}
			if (~data & 0x04) {
				M6809SetIRQLine(0, CPU_IRQSTATUS_ACK);
			}
			i8751_port2 = data;
			break;
	}
}

static void mcu_write_port_shackled(INT32 port, UINT8 data)
{
	switch (port) {
		case MCS51_PORT_P0:
			i8751_port0 = data;
			break;
		case MCS51_PORT_P1:
			i8751_port1 = data;
			break;
		case MCS51_PORT_P2:
			if (~data & 0x10) {
				i8751_port0 = i8751_value >> 8;
				mcs51_set_forced_input(0, i8751_port0);
			}
			if (~data & 0x20) {
				i8751_port1 = i8751_value & 0xff;
			}
			if (~data & 0x40) {
				i8751_return = (i8751_return & 0x00ff) | (i8751_port0 << 8);
			}
			if (~data & 0x80) {
				i8751_return = (i8751_return & 0xff00) | i8751_port1;
			}
			if (~data & 0x01) {
				mcs51_set_irq_line(MCS51_INT0_LINE, CPU_IRQSTATUS_NONE);
			}
			if (~data & 0x02) {
				mcs51_set_irq_line(MCS51_INT1_LINE, CPU_IRQSTATUS_NONE);
			}
			if (~data & 0x04) {
				M6809SetIRQLine(1, M6809_FIRQ_LINE, CPU_IRQSTATUS_HOLD);
			}
			i8751_port2 = data;
			break;
	}
}

static void mcu_write_port_csilver(INT32 port, UINT8 data)
{
	switch (port) {
		case MCS51_PORT_P0:
			i8751_port0 = data;
			break;

		case MCS51_PORT_P1:
			i8751_port1 = data;
			break;

		case MCS51_PORT_P2:
			if (~data & 0x10) {
				i8751_port0 = i8751_value >> 8;
			}
			if (~data & 0x20) {
				i8751_port1 = i8751_value & 0xff;
			}
			if (~data & 0x40) {
				i8751_return = (i8751_return & 0x00ff) | (i8751_port0 << 8);
				M6809SetIRQLine(0, 1, CPU_IRQSTATUS_HOLD);
			}
			if (~data & 0x80) {
				i8751_return = (i8751_return & 0xff00) | i8751_port1;
			}
			i8751_port2 = data;
			break;
	}
}

static void DrvMCUReset(); // forward
static INT32 mcu_game = 0;
static double mcu_divid = 0;
static INT32 (*pTotalCycles)();

static void DrvMCUInit(INT32 game)
{
	mcu_game = game;
	realMCU = 1;

	mcs51_init();
	mcs51_set_program_data(DrvMCURom);

	switch (game) {
		case 0: // gondo, garyoret, ghostb
			mcs51_set_write_handler(mcu_write_port_ghostb);
			mcs51_set_read_handler(mcu_read_port);
			pTotalCycles = HD6309TotalCycles;
			mcu_divid = 12;
			break;
		case 1: // srdarwin
			mcs51_set_write_handler(mcu_write_port_srdarwin);
			mcs51_set_read_handler(mcu_read_port);
			pTotalCycles = M6809TotalCycles;
			mcu_divid = 2;
			break;
		case 2: // shackled, lastmisn
			mcs51_set_write_handler(mcu_write_port_shackled);
			mcs51_set_read_handler(mcu_read_port);
			pTotalCycles = M6809TotalCycles;
			mcu_divid = 1;
			break;
		case 3: // csilver
			mcs51_set_write_handler(mcu_write_port_csilver);
			mcs51_set_read_handler(mcu_read_port);
			pTotalCycles = M6809TotalCycles;
			mcu_divid = 0.75;
			break;
	}
	DrvMCUReset();
}

static void DrvMCUExit() {
	if (realMCU) {
		mcs51_exit();
	}
	realMCU = 0;
}

static INT32 DrvMCUScan(INT32 nAction)
{
	if (realMCU) {
		mcs51_scan(nAction);
	}

	SCAN_VAR(i8751_port0);
	SCAN_VAR(i8751_port1);
	SCAN_VAR(i8751_port2);
	SCAN_VAR(i8751_value);
	SCAN_VAR(i8751_return);

	return 0;
}

static void DrvMCUSync()
{
	if (realMCU == 0) return;

	INT32 todo = (pTotalCycles() / mcu_divid) - mcs51TotalCycles();

	if (todo > 0) {
		mcs51Run(todo);
	}
}

static void DrvMCUReset()
{
	i8751_port0 = i8751_port1 = i8751_port2 = i8751_value = i8751_return = 0;
	if (realMCU) mcs51_reset();
}

static void bankswitch(INT32 data)
{
	RomBank = data;

	HD6309MapMemory(DrvMainROM + 0x10000 + ((data & 0x0f) * 0x4000), 0x4000, 0x7fff, MAP_ROM); // bank
}

static void ghostb_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x3800:
			*soundlatch = data;
			M6502SetIRQLine(M6502_INPUT_LINE_NMI, CPU_IRQSTATUS_AUTO);
		return;

		case 0x3840:
			bankswitch(data >> 4);

			if (~data & 1) HD6309SetIRQLine(0, CPU_IRQSTATUS_NONE);
			*interrupt_enable = data & 1;
			*nmi_enable	  = data & 2;
			*flipscreen	  = data & 8;
		return;

		case 0x3860:
		case 0x3861:
			i8751_write(address & 1, data);
		return;
	}

	if ((address & 0xffe0) == 0x3820) {
		DrvPf0Ctrl[address & 0x1f] = data;
		return;
	}
}

static UINT8 ghostb_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x3800:
			return DrvInputs[0];

		case 0x3801:
			return DrvInputs[1];

		case 0x3802:
			return DrvInputs[2];

		case 0x3803:
			return (DrvDips[0] & 0xf0) | (DrvInputs[3] & 0x07) | vblank;

		case 0x3820:
			return DrvDips[1];

		case 0x3840:
			return i8751_hi();

		case 0x3860:
			return i8751_lo();
	}

	return 0;
}

static void ghostb_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x2000:
		case 0x2001:
			BurnYM2203Write(0, address & 1, data);
		return;

		case 0x4000:
		case 0x4001:
			BurnYM3812Write(0, address & 1, data);
		return;
	}
}

static UINT8 ghostb_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0x3000:
		case 0x6000:
			return *soundlatch;
	}

	return 0;
}

inline static INT32 CsilverMSM5205SynchroniseStream(INT32 nSoundRate)
{
	if (M6809GetActive() == -1) return 0;
	return (INT64)((double)M6809TotalCycles() * nSoundRate / (1500000));
}

static void DrvYM3812FMIRQHandler(INT32, INT32 nStatus)
{
	if (M6502GetActive() == -1) return;

	M6502SetIRQLine(M6502_IRQ_LINE, (nStatus) ? CPU_IRQSTATUS_ACK : CPU_IRQSTATUS_NONE);
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	HD6309Open(0);
	HD6309Reset();
	HD6309Close();

	M6502Open(0);
	M6502Reset();
	M6502Close();

	DrvMCUReset();

	BurnYM3812Reset();
	BurnYM2203Reset();

	HiscoreReset();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	DrvMainROM		= Next; Next += 0x060000;
	DrvSubROM		= Next; Next += 0x030000;
	DrvM6502ROM		= Next; Next += 0x030000;
	DrvMCURom       = Next; Next += 0x001000;

	DrvGfxROM0		= Next; Next += 0x020000;
	DrvGfxROM1		= Next; Next += 0x100000;
	DrvGfxROM2		= Next; Next += 0x100000;
	DrvGfxROM3		= Next; Next += 0x100000;

	DrvColPROM		= Next; Next += 0x000800;

	Palette			= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);
	DrvPalette		= (UINT32*)Next; Next += 0x0400 * sizeof(UINT32);

	AllRam			= Next;

	DrvMainRAM		= Next; Next += 0x008000;
	DrvVidRAM		= Next; Next += 0x001800;
	DrvPf0RAM		= Next; Next += 0x002000;
	DrvPf1RAM		= Next; Next += 0x002000;
	DrvPf0Ctrl		= Next; Next += 0x000040;
	DrvPf1Ctrl		= Next; Next += 0x000040;
	DrvRowRAM		= Next; Next += 0x001400;
	DrvSprRAM		= Next; Next += 0x000800;
	DrvSprBuf		= Next; Next += 0x000800;
	DrvPalRAM		= Next; Next += 0x000800;

	DrvM6502RAM		= Next; Next += 0x002800;

	soundlatch		= Next; Next += 0x000001;
	nmi_enable		= Next; Next += 0x000001;
	interrupt_enable= Next; Next += 0x000001;
	flipscreen		= Next; Next += 0x000001;

	RamEnd			= Next;
	MemEnd			= Next;

	return 0;
}

static void DrvPaletteInit()
{
	for (INT32 i = 0; i < 0x400; i++)
	{
		INT32 bit0, bit1, bit2, bit3, r, g, b;

		bit0 = (DrvColPROM[i] >> 0) & 0x01;
		bit1 = (DrvColPROM[i] >> 1) & 0x01;
		bit2 = (DrvColPROM[i] >> 2) & 0x01;
		bit3 = (DrvColPROM[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (DrvColPROM[i] >> 4) & 0x01;
		bit1 = (DrvColPROM[i] >> 5) & 0x01;
		bit2 = (DrvColPROM[i] >> 6) & 0x01;
		bit3 = (DrvColPROM[i] >> 7) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		bit0 = (DrvColPROM[i + 0x400] >> 0) & 0x01;
		bit1 = (DrvColPROM[i + 0x400] >> 1) & 0x01;
		bit2 = (DrvColPROM[i + 0x400] >> 2) & 0x01;
		bit3 = (DrvColPROM[i + 0x400] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;

		Palette[i] = (r << 16) | (g << 8) | b;
	}
}

static INT32 DrvGfxDecode()
{
	INT32 Plane0[3] = { 0x6000*8,0x4000*8,0x2000*8 };
	INT32 Plane1[4] = { 0x60000*8,0x40000*8,0x20000*8,0x00000*8 };
	INT32 Plane2[4] = { 0x20000*8,0x00000*8,0x30000*8,0x10000*8 };
	INT32 XOffs0[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	INT32 XOffs1[16]= {16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),0,1,2,3,4,5,6,7 };
	INT32 XOffs2[16]= {7,6,5,4,3,2,1,0,7+(16*8), 6+(16*8), 5+(16*8), 4+(16*8), 3+(16*8), 2+(16*8), 1+(16*8), 0+(16*8) };
	INT32 YOffs0[16] = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x80000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x08000);

	GfxDecode(0x0400, 3,  8,  8, Plane0, XOffs0, YOffs0, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x80000);

	GfxDecode(0x1000, 4, 16, 16, Plane1, XOffs1, YOffs0, 0x100, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x40000);

	GfxDecode(0x0800, 4, 16, 16, Plane2, XOffs2, YOffs0, 0x100, tmp, DrvGfxROM2);

	BurnFree (tmp);

	return 0;
}

static INT32 DrvInit()
{
	BurnSetRefreshRate(58.00);

	BurnAllocMemIndex();

	INT32 is_ghostb = 0;

	{
		if (!strncmp(BurnDrvGetTextA(DRV_NAME), "ghostb", 6)) {
			if (BurnLoadRom(DrvMainROM   + 0x08000,  0, 1)) return 1;
			if (BurnLoadRom(DrvMainROM   + 0x10000,  1, 1)) return 1;
			if (BurnLoadRom(DrvMainROM   + 0x20000,  2, 1)) return 1;
			if (BurnLoadRom(DrvMainROM   + 0x30000,  3, 1)) return 1;
			if (BurnLoadRom(DrvMainROM   + 0x40000,  4, 1)) return 1;

			if (BurnLoadRom(DrvM6502ROM  + 0x08000,  5, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM0   + 0x00000,  6, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM1   + 0x00000,  7, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x10000,  8, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x20000,  9, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x30000, 10, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x40000, 11, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x50000, 12, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x60000, 13, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x70000, 14, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM2   + 0x00000, 15, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2   + 0x10000, 16, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2   + 0x20000, 17, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2   + 0x30000, 18, 1)) return 1;

			if (BurnLoadRom(DrvColPROM   + 0x00000, 19, 1)) return 1;
			if (BurnLoadRom(DrvColPROM   + 0x00400, 20, 1)) return 1;

			if (BurnLoadRom(DrvMCURom    + 0x00000, 21, 1)) return 1;

			is_ghostb = 1; // ghostb* uses deco222 cpu for audio
		} else if (!strncmp(BurnDrvGetTextA(DRV_NAME), "meikyuhbl", 9)) {
			// meikyuhbl - Meikyuu Hunter G bootleg
			if (BurnLoadRom(DrvMainROM   + 0x08000,  0, 1)) return 1;
			if (BurnLoadRom(DrvMainROM   + 0x10000,  1, 1)) return 1;
			if (BurnLoadRom(DrvMainROM   + 0x20000,  2, 1)) return 1;
			if (BurnLoadRom(DrvMainROM   + 0x30000,  3, 1)) return 1;

			if (BurnLoadRom(DrvM6502ROM  + 0x08000,  4, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM0   + 0x00000,  5, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM1   + 0x00000,  6, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x10000,  7, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x20000,  8, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x30000,  9, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x40000, 10, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x50000, 11, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x60000, 12, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x70000, 13, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM2   + 0x00000, 14, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2   + 0x10000, 15, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2   + 0x20000, 16, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2   + 0x30000, 17, 1)) return 1;

			if (BurnLoadRom(DrvColPROM   + 0x00000, 18, 1)) return 1;
			if (BurnLoadRom(DrvColPROM   + 0x00400, 19, 1)) return 1;
			for (UINT32 i = 0; i < 0x400; i++) {
				DrvColPROM[i] = (DrvColPROM[i] & 0xf) + (DrvColPROM[i + 0x400] << 4);
				DrvColPROM[i] = BITSWAP08(DrvColPROM[i],4,5,6,7,3,2,1,0);
			}
			if (BurnLoadRom(DrvColPROM   + 0x00400, 20, 1)) return 1;

			if (BurnLoadRom(DrvMCURom    + 0x00000, 21, 1)) return 1;

			// meikyuhbl uses M6502 for audio
		} else { // meikyuh - Meikyuu Hunter G
			if (BurnLoadRom(DrvMainROM   + 0x08000,  0, 1)) return 1;
			if (BurnLoadRom(DrvMainROM   + 0x10000,  1, 1)) return 1;
			if (BurnLoadRom(DrvMainROM   + 0x20000,  2, 1)) return 1;
			if (BurnLoadRom(DrvMainROM   + 0x30000,  3, 1)) return 1;

			if (BurnLoadRom(DrvM6502ROM  + 0x08000,  4, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM0   + 0x00000,  5, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM1   + 0x00000,  6, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x10000,  7, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x20000,  8, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x30000,  9, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x40000, 10, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x50000, 11, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x60000, 12, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x70000, 13, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM2   + 0x00000, 14, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2   + 0x10000, 15, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2   + 0x20000, 16, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2   + 0x30000, 17, 1)) return 1;

			if (BurnLoadRom(DrvColPROM   + 0x00000, 18, 1)) return 1;
			if (BurnLoadRom(DrvColPROM   + 0x00400, 19, 1)) return 1;

			if (BurnLoadRom(DrvMCURom    + 0x00000, 20, 1)) return 1;

			// meikyuh uses M6502 for audio
		}

		DrvPaletteInit();
		DrvGfxDecode();
	}

	HD6309Init(0);
	HD6309Open(0);
	HD6309MapMemory(DrvMainRAM,		0x0000, 0x17ff, MAP_RAM);
	HD6309MapMemory(DrvVidRAM,		0x1800, 0x1fff, MAP_RAM);
	HD6309MapMemory(DrvPf0RAM,		0x2000, 0x2bff, MAP_RAM);// 0 - 27ff
	HD6309MapMemory(DrvRowRAM,		0x2c00, 0x2fff, MAP_RAM);// c00-dff
	HD6309MapMemory(DrvSprRAM,		0x3000, 0x37ff, MAP_RAM);
	HD6309MapMemory(DrvMainROM + 0x10000, 0x4000, 0x7fff, MAP_ROM); // bank
	HD6309MapMemory(DrvMainROM + 0x08000, 0x8000, 0xffff, MAP_ROM);
	HD6309SetWriteHandler(ghostb_main_write);
	HD6309SetReadHandler(ghostb_main_read);
	HD6309Close();

	M6502Init(0, (is_ghostb) ? TYPE_DECO222 : TYPE_M6502);
	M6502Open(0);
	M6502MapMemory(DrvM6502RAM,          0x0000, 0x05ff, MAP_RAM);
	M6502MapMemory(DrvM6502ROM + 0x8000, 0x8000, 0xffff, MAP_ROM);
	M6502SetReadHandler(ghostb_sound_read);
	M6502SetWriteHandler(ghostb_sound_write);
	M6502Close();

	DrvMCUInit(0);

	BurnYM3812Init(1, 3000000, &DrvYM3812FMIRQHandler, 0);
	BurnTimerAttach(&M6502Config, 1500000);
	BurnYM3812SetRoute(0, BURN_SND_YM3812_ROUTE, 0.70, BURN_SND_ROUTE_BOTH);

	BurnYM2203Init(1, 1500000, NULL, 1);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_YM2203_ROUTE, 0.20, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_1, 0.23, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_2, 0.23, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_3, 0.23, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();

	M6502Exit();

	DrvMCUExit();

	BurnYM2203Exit();

	BurnFreeMemIndex();

	return 0;
}

static void draw_txt_layer()
{
	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		INT32 code  = DrvVidRAM[offs * 2 + 1] | (DrvVidRAM[offs * 2 + 0] << 8);
		INT32 color = (code >> 10) & 3;

		code &= 0x3ff;
		if (code == 0) continue; // ok?

		sy -= 8;
		if (sy < 0 || sy >= nScreenHeight) continue;

		Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 3, 0, 0, DrvGfxROM0);
	}
}

static void draw_sprites1(INT32 priority)
{
	for (INT32 offs = 0; offs < 0x800; offs += 8)
	{
		INT32 y = DrvSprBuf[offs + 1] | (DrvSprBuf[offs + 0] << 8);
		if (~y  & 0x8000) continue;

		INT32 fx = DrvSprBuf[offs+3];
		if (~fx & 0x0001) continue;

		INT32 x = DrvSprBuf[offs + 5] | (DrvSprBuf[offs + 4] << 8);

		INT32 extra  = fx & 0x0010;
		INT32 fy     = fx & 0x0002;
		INT32 colour = DrvSprBuf[offs+6] >> 4;

		INT32 sprite = DrvSprBuf[offs+7]+(DrvSprBuf[offs+6]<<8);
		sprite &= 0x0fff;

		if (priority == 1 &&  colour & 8) continue;
		if (priority == 2 && ~colour & 8) continue;

		if (extra) {y=y+16;sprite&=0xffe;}

		x=(x+16)&0x1ff;
		y=(y+16)&0x1ff;
		x=256 - x;
		y=256 - y;

		fx &= 0x0004;

		if (*flipscreen)
		{
			// disable coctail flipping, for 2p/2joy or netgame plays
			// also this math below breaks last mission 2p/coctail:
			//y=240-y;
			//x=240-x;
			//if (fx) fx=0; else fx=1;
			//if (fy) fy=0; else fy=1;
			//if (extra) y=y-16;
		}

		INT32 sprite2;
		/* Y Flip determines order of multi-sprite */
		if (extra && fy)
		{
			sprite2=sprite;
			sprite++;
		}
		else
			sprite2=sprite+1;

		y -= 8;

		Draw16x16MaskTile(pTransDraw, sprite & 0xfff, x, y, fx, fy, colour, 4, 0, 0x100, DrvGfxROM1);
		/* 1 more sprite drawn underneath */
		if (extra) {
			Draw16x16MaskTile(pTransDraw, sprite2 & 0xfff, x, y+16, fx, fy, colour, 4, 0, 0x100, DrvGfxROM1);
		}
	}
}

static void draw_bg_layer(UINT8 *ram, UINT8 *ctrl, UINT8 *gfx, INT32 coff, INT32 cmask, INT32 mask, INT32 t, INT32 priority)
{
	INT32 rowscroll = ctrl[0x00] & 0x04;
	INT32 scrolly = ((ctrl[0x12] << 8) + ctrl[0x13] + 8) & 0x1ff;
	INT32 scrollx = ((ctrl[0x10] << 8) | ctrl[0x11]) & 0x1ff;

	if (rowscroll || t != 0)
	{
		INT32 xscroll[256];

		if (rowscroll) {
			scrollx = 0;
			for (INT32 i = 0; i < 256; i++) {
				xscroll[i] = (((ctrl[0x10] << 8) | ctrl[0x11]) + ((DrvRowRAM[i*2]<<8)+DrvRowRAM[i*2+1]))&0x1ff;
			}
		}

		for (INT32 offs = 0; offs < 32 * 32; offs++)
		{
			INT32 sx = (offs & 0x1f) << 4;
			INT32 sy = (offs >> 5) << 4;

			sy -= scrolly;
			if (sy < -15) sy += 512;
			sx -= scrollx;
			if (sx < -15) sx += 512;

			if (sy >= nScreenHeight || sx >= nScreenWidth) continue;

			INT32 ofst = (offs & 0x0f) | ((offs >> 1) & 0x1f0) | ((offs & 0x10) << 5);

			INT32 code  = (ram[ofst * 2 + 0] << 8) | ram[ofst * 2 + 1];
			INT32 color = (code >> 12);

			if (priority && color < 8) continue;

			color &= cmask;

			{
				code &= mask;
				color <<= 4;
				color += coff;
				UINT16 *dst = pTransDraw;
				UINT8 *src = gfx + (code << 8);

				for (INT32 y = 0; y < 16; y++, sy++, src+=16)
				{
					if (sy < 0 || sy >= nScreenHeight) continue;

					dst = pTransDraw + sy * nScreenWidth;

					INT32 xx = sx;
					if (rowscroll) {
						xx -= xscroll[sy];
						if (xx < -15) xx += 512;
						if (xx >= nScreenWidth) continue;
					}

					if (t) {
						for (INT32 x = 0; x < 16; x++, xx++)
						{
							INT32 pxl = src[x];
							if (t & (1 << pxl)) continue;
							if (xx < 0 || xx >= nScreenWidth) continue;

							dst[xx] = pxl | color;
						}
					} else {
						for (INT32 x = 0; x < 16; x++, xx++)
						{
							if (xx < 0 || xx >= nScreenWidth) continue;

							dst[xx] = src[x] | color;
						}
					}
				}
			}
		}
	} else {
		for (INT32 offs = 0; offs < 32 * 32; offs++)
		{
			INT32 sx = (offs & 0x1f) << 4;
			INT32 sy = (offs >> 5) << 4;

			sx -= scrollx;
			sy -= scrolly;
			if (sx < -15) sx += 512;
			if (sy < -15) sy += 512;

			if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

			INT32 ofst = (offs & 0x0f) | ((offs >> 1) & 0x1f0) | ((offs & 0x10) << 5);

			INT32 code  = (ram[ofst * 2 + 0] << 8) | ram[ofst * 2 + 1];
			INT32 color = (code >> 12) & cmask;

			if (priority && color < 8) continue;

			if (t) {
				Render16x16Tile_Mask_Clip(pTransDraw, code & mask, sx, sy, color, 4, 0, coff, gfx);
			} else {
				Render16x16Tile_Clip(pTransDraw, code & mask, sx, sy, color, 4, coff, gfx);
			}
		}
	}
}


static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x400; i++) {
			INT32 d = Palette[i];
			DrvPalette[i] = BurnHighCol(d >> 16, (d >> 8) & 0xff, d & 0xff, 0);
		}
	}

	BurnTransferClear();

	draw_bg_layer(DrvPf0RAM, DrvPf0Ctrl, DrvGfxROM2, 0x200, 0x0f, 0x7ff, 0, 0);
	draw_sprites1(0);
	draw_txt_layer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	HD6309NewFrame();
	M6502NewFrame();
	mcs51NewFrame();

	{
		memset (DrvInputs, 0xff, 5);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
			DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
		}
	}

	INT32 nInterleave = 256;
	INT32 nCyclesTotal[3] = { 12000000 / 58, 1500000 / 58, 12000000 / 12 / 58 };
	INT32 nCyclesDone[3] = { 0, 0, 0 };

	M6502Open(0);
	HD6309Open(0);

	vblank = 8;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		if (i == 8) vblank = 0;
		if (i == 254) {
			vblank = 8;
			if (*nmi_enable) HD6309SetIRQLine(0x20, CPU_IRQSTATUS_AUTO);
		}

		CPU_RUN(0, HD6309);

		if (realMCU) {
			DrvMCUSync();
		}

		CPU_RUN_TIMER(1);
	}

	HD6309Close();
	M6502Close();

	if (pBurnSoundOut) {
		BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		DrvDraw();
	}

	memcpy (DrvSprBuf, DrvSprRAM, 0x800);

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {
		*pnMin = 0x029722;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd-AllRam;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		M6502Scan(nAction);
		BurnYM2203Scan(nAction, pnMin);

		DrvMCUScan(nAction);
		SCAN_VAR(vblank);
		SCAN_VAR(RomBank);
	}

	return 0;
}

// The Real Ghostbusters (US 2 Players, revision 2)

static struct BurnRomInfo ghostbRomDesc[] = {
	{ "dz01-22.1d",		0x08000, 0xfc65fdf2, 1 }, //  0 maincpu
	{ "dz02.3d",		0x10000, 0x8e117541, 1 }, //  1
	{ "dz03.4d",		0x10000, 0x5606a8f4, 1 }, //  2
	{ "dz04-21.6d",		0x10000, 0x7d46582f, 1 }, //  3
	{ "dz05-21.7d",		0x10000, 0x23e1c758, 1 }, //  4

	{ "dz06.5f",		0x08000, 0x798f56df, 2 }, //  5 audiocpu

	{ "dz00.16b",		0x08000, 0x992b4f31, 3 }, //  6 gfx1

	{ "dz15.14f",		0x10000, 0xa01a5fd9, 4 }, //  7 gfx2
	{ "dz16.15f",		0x10000, 0x5a9a344a, 4 }, //  8
	{ "dz12.9f",		0x10000, 0x817fae99, 4 }, //  9
	{ "dz14.12f",		0x10000, 0x0abbf76d, 4 }, // 10
	{ "dz11.8f",		0x10000, 0xa5e19c24, 4 }, // 11
	{ "dz13.1f",		0x10000, 0x3e7c0405, 4 }, // 12
	{ "dz17.17f",		0x10000, 0x40361b8b, 4 }, // 13
	{ "dz18.18f",		0x10000, 0x8d219489, 4 }, // 14

	{ "dz07.12f",		0x10000, 0xe7455167, 5 }, // 15 gfx3
	{ "dz08.14f",		0x10000, 0x32f9ddfe, 5 }, // 16
	{ "dz09.15f",		0x10000, 0xbb6efc02, 5 }, // 17
	{ "dz10.17f",		0x10000, 0x6ef9963b, 5 }, // 18

	{ "dz19a.10d",		0x00400, 0x47e1f83b, 6 }, // 19 proms
	{ "dz20a.11d",		0x00400, 0xd8fe2d99, 6 }, // 20

	{ "dz-1.1b", 		0x01000, 0x9f5f3cb5, 7 }, // 21 i8751 microcontroller
};

STD_ROM_PICK(ghostb)
STD_ROM_FN(ghostb)

static INT32 GhostbExit()
{
	HD6309Exit();
	BurnYM3812Exit();

	return DrvExit();
}

static INT32 GhostbScan(INT32 nAction, INT32 *pnMin)
{
	if (nAction & ACB_DRIVER_DATA) {
		HD6309Scan(nAction);
		BurnYM3812Scan(nAction, pnMin);

		SCAN_VAR(RomBank);

		if (nAction & ACB_WRITE) {
			HD6309Open(0);
			bankswitch(RomBank);
			HD6309Close();
		}
	}

	return DrvScan(nAction, pnMin);
}

struct BurnDriver BurnDrvGhostb = {
	"ghostb", NULL, NULL, NULL, "1987",
	"The Real Ghostbusters (US 2 Players, revision 2)\0", NULL, "Data East USA", "DEC8",
	L"\u6349\u9B3C\u6562\u6B7B\u961F (\u7F8E\u7248 2P \u7248, \u4FEE\u8BA2\u7248 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SHOOT, 0,
	NULL, ghostbRomInfo, ghostbRomName, NULL, NULL, NULL, NULL, GhostbInputInfo, GhostbDIPInfo,
	DrvInit, GhostbExit, DrvFrame, DrvDraw, GhostbScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};


// The Real Ghostbusters (US 2 Players)

static struct BurnRomInfo ghostb2aRomDesc[] = {
	{ "dz01.1d",		0x08000, 0x7c5bb4b1, 1 }, //  0 maincpu
	{ "dz02.3d",		0x10000, 0x8e117541, 1 }, //  1
	{ "dz03.4d",		0x10000, 0x5606a8f4, 1 }, //  2
	{ "dz04.6d",		0x10000, 0xd09bad99, 1 }, //  3
	{ "dz05.7d",		0x10000, 0x0315f691, 1 }, //  4

	{ "dz06.5f",		0x08000, 0x798f56df, 2 }, //  5 audiocpu

	{ "dz00.16b",		0x08000, 0x992b4f31, 3 }, //  6 gfx1

	{ "dz15.14f",		0x10000, 0xa01a5fd9, 4 }, //  7 gfx2
	{ "dz16.15f",		0x10000, 0x5a9a344a, 4 }, //  8
	{ "dz12.9f",		0x10000, 0x817fae99, 4 }, //  9
	{ "dz14.12f",		0x10000, 0x0abbf76d, 4 }, // 10
	{ "dz11.8f",		0x10000, 0xa5e19c24, 4 }, // 11
	{ "dz13.1f",		0x10000, 0x3e7c0405, 4 }, // 12
	{ "dz17.17f",		0x10000, 0x40361b8b, 4 }, // 13
	{ "dz18.18f",		0x10000, 0x8d219489, 4 }, // 14

	{ "dz07.12f",		0x10000, 0xe7455167, 5 }, // 15 gfx3
	{ "dz08.14f",		0x10000, 0x32f9ddfe, 5 }, // 16
	{ "dz09.15f",		0x10000, 0xbb6efc02, 5 }, // 17
	{ "dz10.17f",		0x10000, 0x6ef9963b, 5 }, // 18

	{ "dz19a.10d",		0x00400, 0x47e1f83b, 6 }, // 19 proms
	{ "dz20a.11d",		0x00400, 0xd8fe2d99, 6 }, // 20

	{ "dz-1.1b",        0x01000, 0x9f5f3cb5, 7 }, // 21 i8751 microcontroller
};

STD_ROM_PICK(ghostb2a)
STD_ROM_FN(ghostb2a)

struct BurnDriver BurnDrvGhostb2a = {
	"ghostb2a", "ghostb", NULL, NULL, "1987",
	"The Real Ghostbusters (US 2 Players)\0", NULL, "Data East USA", "DEC8",
	L"\u6349\u9B3C\u6562\u6B7B\u961F (\u7F8E\u7248 2P \u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SHOOT, 0,
	NULL, ghostb2aRomInfo, ghostb2aRomName, NULL, NULL, NULL, NULL, GhostbInputInfo, GhostbDIPInfo,
	DrvInit, GhostbExit, DrvFrame, DrvDraw, GhostbScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};


// The Real Ghostbusters (US 3 Players, revision 3B?)

static struct BurnRomInfo ghostb3RomDesc[] = {
	{ "dz01-3b.1d",		0x08000, 0xc8cc862a, 1 }, //  0 maincpu
	{ "dz02.3d",		0x10000, 0x8e117541, 1 }, //  1
	{ "dz03.4d",		0x10000, 0x5606a8f4, 1 }, //  2
	{ "dz04-1.6d",		0x10000, 0x3c3eb09f, 1 }, //  3
	{ "dz05-1.7d",		0x10000, 0xb4971d33, 1 }, //  4

	{ "dz06.5f",		0x08000, 0x798f56df, 2 }, //  5 audiocpu

	{ "dz00.16b",		0x08000, 0x992b4f31, 3 }, //  6 gfx1

	{ "dz15.14f",		0x10000, 0xa01a5fd9, 4 }, //  7 gfx2
	{ "dz16.15f",		0x10000, 0x5a9a344a, 4 }, //  8
	{ "dz12.9f",		0x10000, 0x817fae99, 4 }, //  9
	{ "dz14.12f",		0x10000, 0x0abbf76d, 4 }, // 10
	{ "dz11.8f",		0x10000, 0xa5e19c24, 4 }, // 11
	{ "dz13.1f",		0x10000, 0x3e7c0405, 4 }, // 12
	{ "dz17.17f",		0x10000, 0x40361b8b, 4 }, // 13
	{ "dz18.18f",		0x10000, 0x8d219489, 4 }, // 14

	{ "dz07.12f",		0x10000, 0xe7455167, 5 }, // 15 gfx3
	{ "dz08.14f",		0x10000, 0x32f9ddfe, 5 }, // 16
	{ "dz09.15f",		0x10000, 0xbb6efc02, 5 }, // 17
	{ "dz10.17f",		0x10000, 0x6ef9963b, 5 }, // 18

	{ "dz19a.10d",		0x00400, 0x47e1f83b, 6 }, // 19 proms
	{ "dz20a.11d",		0x00400, 0xd8fe2d99, 6 }, // 20

	{ "dz-1.1b",        0x01000, 0x9f5f3cb5, 7 }, // 21 i8751 microcontroller
};

STD_ROM_PICK(ghostb3)
STD_ROM_FN(ghostb3)

struct BurnDriver BurnDrvGhostb3 = {
	"ghostb3", "ghostb", NULL, NULL, "1987",
	"The Real Ghostbusters (US 3 Players, revision 3B?)\0", NULL, "Data East USA", "DEC8",
	L"\u6349\u9B3C\u6562\u6B7B\u961F (\u7F8E\u7248 3P \u7248, \u4FEE\u8BA2\u7248 3B?)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SHOOT, 0,
	NULL, ghostb3RomInfo, ghostb3RomName, NULL, NULL, NULL, NULL, Ghostb3InputInfo, Ghostb3DIPInfo,
	DrvInit, GhostbExit, DrvFrame, DrvDraw, GhostbScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};

// DZ-1 is the verified correct MCU code for the ghostb3a set below - program ROMs work on a real PCB, but stall here in the demo mode
// Both DZ01-2 & DZ04- ROMs have been verified correct from multiple sources

// The Real Ghostbusters (US 3 Players, revision 2)

static struct BurnRomInfo ghostb3aRomDesc[] = {
	{ "dz01-2.1d",		0x08000, 0x1b16890e, 1 }, //  0 maincpu
	{ "dz02-.3d",		0x10000, 0x8e117541, 1 }, //  1
	{ "dz03-.4d",		0x10000, 0x5606a8f4, 1 }, //  2
	{ "dz04-.6d",		0x10000, 0x490b4525, 1 }, //  3
	{ "dz05-.7d",		0x10000, 0xb4971d33, 1 }, //  4

	{ "dz06.5f",		0x08000, 0x798f56df, 2 }, //  5 audiocpu

	{ "dz00.16b",		0x08000, 0x992b4f31, 3 }, //  6 gfx1

	{ "dz15.14f",		0x10000, 0xa01a5fd9, 4 }, //  7 gfx2
	{ "dz16.15f",		0x10000, 0x5a9a344a, 4 }, //  8
	{ "dz12.9f",		0x10000, 0x817fae99, 4 }, //  9
	{ "dz14.12f",		0x10000, 0x0abbf76d, 4 }, // 10
	{ "dz11.8f",		0x10000, 0xa5e19c24, 4 }, // 11
	{ "dz13.1f",		0x10000, 0x3e7c0405, 4 }, // 12
	{ "dz17.17f",		0x10000, 0x40361b8b, 4 }, // 13
	{ "dz18.18f",		0x10000, 0x8d219489, 4 }, // 14

	{ "dz07.12f",		0x10000, 0xe7455167, 5 }, // 15 gfx3
	{ "dz08.14f",		0x10000, 0x32f9ddfe, 5 }, // 16
	{ "dz09.15f",		0x10000, 0xbb6efc02, 5 }, // 17
	{ "dz10.17f",		0x10000, 0x6ef9963b, 5 }, // 18

	{ "dz19a.10d",		0x00400, 0x47e1f83b, 6 }, // 19 proms
	{ "dz20a.11d",		0x00400, 0xd8fe2d99, 6 }, // 20

	{ "dz-1.1b",        0x01000, 0x9f5f3cb5, 7 }, // 21 i8751 microcontroller
};

STD_ROM_PICK(ghostb3a)
STD_ROM_FN(ghostb3a)

struct BurnDriverD BurnDrvGhostb3a = {
	"ghostb3a", "ghostb", NULL, NULL, "1987",
	"The Real Ghostbusters (US 3 Players, revision 2)\0", NULL, "Data East USA", "DEC8",
	L"\0", NULL, NULL, NULL,
	BDF_GAME_NOT_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SHOOT, 0,
	NULL, ghostb3aRomInfo, ghostb3aRomName, NULL, NULL, NULL, NULL, GhostbInputInfo, Ghostb3DIPInfo,
	DrvInit, GhostbExit, DrvFrame, DrvDraw, GhostbScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};


// Meikyuu Hunter G (Japan)

static struct BurnRomInfo meikyuhRomDesc[] = {
	{ "dw01-5.1d",	0x08000, 0x87610c39, 1 }, //  0 maincpu
	{ "dw02.3d",	0x10000, 0x40c9b0b8, 1 }, //  1
	{ "dw03.4d",	0x10000, 0x5606a8f4, 1 }, //  2
	{ "dw04.6d",	0x10000, 0x235c0c36, 1 }, //  3

	{ "dw05.5f",	0x08000, 0xc28c4d82, 2 }, //  4 audiocpu

	{ "dw00.16b",	0x08000, 0x3d25f15c, 3 }, //  5 gfx1

	{ "dw14.14f",	0x10000, 0x9b0dbfa9, 4 }, //  6 gfx2
	{ "dw15.15f",	0x10000, 0x95683fda, 4 }, //  7
	{ "dw11.9f",	0x10000, 0x1b1fcca7, 4 }, //  8
	{ "dw13.12f",	0x10000, 0xe7413056, 4 }, //  9
	{ "dw10.8f",	0x10000, 0x57667546, 4 }, // 10
	{ "dw12.1f",	0x10000, 0x4c548db8, 4 }, // 11
	{ "dw16.17f",	0x10000, 0xe5bcf927, 4 }, // 12
	{ "dw17.18f",	0x10000, 0x9e10f723, 4 }, // 13

	{ "dw06.12f",	0x10000, 0xb65e029d, 5 }, // 14 gfx3
	{ "dw07.14f",	0x10000, 0x668d995d, 5 }, // 15
	{ "dw08.15f",	0x10000, 0xbb2cf4a0, 5 }, // 16
	{ "dw09.17f",	0x10000, 0x6a528d13, 5 }, // 17

	{ "dw18.9d",	0x00400, 0x75f1945f, 6 }, // 18 proms
	{ "dw19.10d",	0x00400, 0xcc16f3fa, 6 }, // 19

	{ "dw.1b",		0x01000, 0x28e9ced9, 7 }, // 20 i8751 microcontroller
};

STD_ROM_PICK(meikyuh)
STD_ROM_FN(meikyuh)

struct BurnDriver BurnDrvMeikyuh = {
	"meikyuh", NULL, NULL, NULL, "1987",
	"Meikyuu Hunter G (Japan)\0", NULL, "Data East Corporation", "DEC8",
	L"\u6349\u9B3C\u6562\u6B7B\u961F G (\u65E5\u7248, \u7B2C\u4E00\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SHOOT, 0,
	NULL, meikyuhRomInfo, meikyuhRomName, NULL, NULL, NULL, NULL, GhostbInputInfo, MeikyuuDIPInfo,
	DrvInit, GhostbExit, DrvFrame, DrvDraw, GhostbScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};


// Meikyuu Hunter G (Japan, bootleg)

static struct BurnRomInfo meikyuhblRomDesc[] = {
	{ "27256.1d",	0x08000, 0xd5b5e8a2, 1 }, //  0 maincpu
	{ "24512.3d",	0x10000, 0x40c9b0b8, 1 }, //  1
	{ "24512.4d",	0x10000, 0x5606a8f4, 1 }, //  2
	{ "27512.6d",	0x10000, 0x8ca6055d, 1 }, //  3

	{ "27256.5f",	0x08000, 0xc28c4d82, 2 }, //  4 audiocpu

	{ "27256.16b",	0x08000, 0x3d25f15c, 3 }, //  5 gfx1

	{ "92.6m",		0x10000, 0x9b0dbfa9, 4 }, //  6 gfx2
	{ "93.6o",		0x10000, 0x95683fda, 4 }, //  7
	{ "89.6i",		0x10000, 0x1b1fcca7, 4 }, //  8
	{ "91.6l",		0x10000, 0xe7413056, 4 }, //  9
	{ "88.6h",		0x10000, 0x57667546, 4 }, // 10
	{ "90.6k",		0x10000, 0x4c548db8, 4 }, // 11
	{ "94.6p",		0x10000, 0xe5bcf927, 4 }, // 12
	{ "95.6r",		0x10000, 0x9e10f723, 4 }, // 13

	{ "27512.12f",	0x10000, 0xb65e029d, 5 }, // 14 gfx3
	{ "27512.14f",	0x10000, 0x668d995d, 5 }, // 15
	{ "27512.15f",	0x10000, 0xbb2cf4a0, 5 }, // 16
	{ "27512.17f",	0x10000, 0x6a528d13, 5 }, // 17

	{ "82s137.12d",	0x00400, 0xbf922733, 6 }, // 18 proms
	{ "82s137.13d",	0x00400, 0x4ccc328e, 6 }, // 19
	{ "82s137.10d",	0x00400, 0xcc16f3fa, 6 }, // 20

	{ "dw.1b",		0x01000, 0x28e9ced9, 7 }, // 21 i8751 microcontroller
};

STD_ROM_PICK(meikyuhbl)
STD_ROM_FN(meikyuhbl)

struct BurnDriver BurnDrvMeikyuhbl = {
	"meikyuhbl", "meikyuh", NULL, NULL, "1987",
	"Meikyuu Hunter G (Japan, bootleg)\0", NULL, "bootleg", "DEC8",
	L"\u6349\u9B3C\u6562\u6B7B\u961F (\u65E5\u7248, \u76D7\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SHOOT, 0,
	NULL, meikyuhblRomInfo, meikyuhblRomName, NULL, NULL, NULL, NULL, GhostbInputInfo, MeikyuuDIPInfo,
	DrvInit, GhostbExit, DrvFrame, DrvDraw, GhostbScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};


static void m6809_bankswitch(INT32 data)
{
	RomBank = data;

	M6809MapMemory(DrvMainROM + 0x10000 + ((data & 0x0f) * 0x4000), 0x4000, 0x7fff, MAP_ROM); // bank
}

static void cobra_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x3e00:
			*soundlatch = data;
			M6502SetIRQLine(M6502_INPUT_LINE_NMI, CPU_IRQSTATUS_AUTO);
		return;

		case 0x3c00:
			m6809_bankswitch(data);
		return;

		case 0x3c02:
			memcpy (DrvSprBuf, DrvSprRAM, 0x800);
		return;
	}

	if ((address & 0xffe0) == 0x3800) {
		DrvPf0Ctrl[address & 0x1f] = data;
		return;
	}

	if ((address & 0xffe0) == 0x3a00) {
		DrvPf1Ctrl[address & 0x1f] = data;
		return;
	}
}

static UINT8 cobra_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x3800:
			return DrvInputs[0];

		case 0x3801:
			return DrvInputs[1];

		case 0x3802:
			return DrvDips[0];

		case 0x3803:
			return DrvDips[1];

		case 0x3a00:
			return (DrvInputs[2] & 0x7f) | vblank;
	}

	return 0;
}

static INT32 CobraDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	M6809Open(0);
	M6809Reset();
	M6809Close();

	M6502Open(0);
	M6502Reset();
	M6502Close();

	DrvMCUReset();

	BurnYM2203Reset();
	BurnYM3812Reset();

	return 0;
}

static INT32 CobraGfxDecode()
{
	// 0, 0x40, 0x80, 0xc0 for offs

	INT32 Plane0[2] = { 0x4000*8,0x0000*8 };
	INT32 Plane1[4] = { 0x60000*8,0x40000*8,0x20000*8,0x00000*8 };
	INT32 XOffs0[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	INT32 XOffs1[16]= { 16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),0,1,2,3,4,5,6,7 };
	INT32 YOffs0[16] = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x80000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x08000);

	GfxDecode(0x0400, 2,  8,  8, Plane0, XOffs0, YOffs0, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x80000);

	GfxDecode(0x1000, 4, 16, 16, Plane1, XOffs1, YOffs0, 0x100, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x80000);

	GfxDecode(0x1000, 4, 16, 16, Plane1, XOffs1, YOffs0, 0x100, tmp, DrvGfxROM2);

	memcpy (tmp, DrvGfxROM3, 0x80000);

	GfxDecode(0x1000, 4, 16, 16, Plane1, XOffs1, YOffs0, 0x100, tmp, DrvGfxROM3);

	BurnFree (tmp);

	return 0;
}

static INT32 CobraInit()
{
	BurnSetRefreshRate(58.00);

	BurnAllocMemIndex();

	{
		if (!strcmp(BurnDrvGetTextA(DRV_NAME), "cobracomib") ||
			!strcmp(BurnDrvGetTextA(DRV_NAME), "cobracomjb"))
		{
			if (BurnLoadRom(DrvMainROM + 0x08000,  0, 1)) return 1;
			if (BurnLoadRom(DrvMainROM + 0x10000,  1, 1)) return 1;
			if (BurnLoadRom(DrvMainROM + 0x20000,  2, 1)) return 1;

			if (BurnLoadRom(DrvM6502ROM  + 0x08000,  3, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM0   + 0x00000,  4, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM1   + 0x00000,  5, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x20000,  6, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x40000,  7, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x60000,  8, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM2   + 0x00000,  9, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2   + 0x40000, 10, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2   + 0x20000, 11, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2   + 0x60000, 12, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM3   + 0x00000, 13, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM3   + 0x20000, 14, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM3   + 0x40000, 15, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM3   + 0x60000, 16, 1)) return 1;
		} else {
			if (BurnLoadRom(DrvMainROM + 0x08000,  0, 1)) return 1;
			if (BurnLoadRom(DrvMainROM + 0x10000,  1, 1)) return 1;
			if (BurnLoadRom(DrvMainROM + 0x20000,  2, 1)) return 1;

			if (BurnLoadRom(DrvM6502ROM  + 0x08000,  3, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM0   + 0x00000,  4, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM1   + 0x00000,  5, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x20000,  6, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x40000,  7, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1   + 0x60000,  8, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM2   + 0x00000,  9, 1)) return 1;
			memcpy (DrvGfxROM2 + 0x40000, DrvGfxROM2 + 0x08000, 0x08000);
			if (BurnLoadRom(DrvGfxROM2   + 0x20000, 10, 1)) return 1;
			memcpy (DrvGfxROM2 + 0x60000, DrvGfxROM2 + 0x28000, 0x08000);

			if (BurnLoadRom(DrvGfxROM3   + 0x00000, 11, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM3   + 0x20000, 12, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM3   + 0x40000, 13, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM3   + 0x60000, 14, 1)) return 1;
		}

		CobraGfxDecode();
	}

	M6809Init(0);
	M6809Open(0);
	M6809MapMemory(DrvMainRAM,		0x0000, 0x07ff, MAP_RAM);
	M6809MapMemory(DrvPf0RAM,		0x0800, 0x0fff, MAP_RAM);
	M6809MapMemory(DrvPf1RAM,		0x1000, 0x1fff, MAP_RAM);
	M6809MapMemory(DrvVidRAM,		0x2000, 0x27ff, MAP_RAM);
	M6809MapMemory(DrvSprRAM,		0x2800, 0x2fff, MAP_RAM);
	M6809MapMemory(DrvPalRAM,		0x3000, 0x37ff, MAP_RAM);
	M6809MapMemory(DrvMainROM + 0x10000,  0x4000, 0x7fff, MAP_ROM);
	M6809MapMemory(DrvMainROM + 0x08000,  0x8000, 0xffff, MAP_ROM);
	M6809SetWriteHandler(cobra_main_write);
	M6809SetReadHandler(cobra_main_read);
	M6809Close();

	M6502Init(0, TYPE_M6502);
	M6502Open(0);
	M6502MapMemory(DrvM6502RAM,          0x0000, 0x05ff, MAP_RAM);
	M6502MapMemory(DrvM6502ROM + 0x8000, 0x8000, 0xffff, MAP_ROM);
	M6502SetReadHandler(ghostb_sound_read);
	M6502SetWriteHandler(ghostb_sound_write);
	M6502Close();

	BurnYM2203Init(1, 1500000, NULL, 0);
	BurnTimerAttach(&M6502Config, 1500000);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_YM2203_ROUTE, 0.50, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_1, 0.53, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_2, 0.53, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_3, 0.53, BURN_SND_ROUTE_BOTH);

	BurnYM3812Init(1, 3000000, &DrvYM3812FMIRQHandler, 1);
	BurnYM3812SetRoute(0, BURN_SND_YM3812_ROUTE, 0.70, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	CobraDoReset();

	return 0;
}

static void draw_cobra_txt_layer()
{
	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		INT32 code  = DrvVidRAM[offs * 2 + 1] | (DrvVidRAM[offs * 2 + 0] << 8);
		INT32 color = (code >> 13);

		code &= 0x3ff;
		if (code == 0) continue; // ok?

		sy -= 8;
		if (sy < 0 || sy >= nScreenHeight) continue;

		Render8x8Tile_Mask(pTransDraw, code, sx, sy, color, 2, 0, 0, DrvGfxROM0);
	}
}

static void draw_sprites2(INT32 colmask, INT32 priority, INT32 color_offset)
{
	for (INT32 offs = 0; offs < 0x800; offs += 8)
	{
		INT32 y = DrvSprBuf[offs+1]+(DrvSprBuf[offs]<<8);
 		if ((y&0x8000) == 0) continue;
		INT32 x = DrvSprBuf[offs+5]+(DrvSprBuf[offs+4]<<8);
		INT32 colour = (x >> 12);
		INT32 flash=x&0x800;
		if (flash && (GetCurrentFrame() & 1)) continue;

		if (priority==1 &&  (colour&4)) continue;
		if (priority==2 && !(colour&4)) continue;

		colour &= colmask;
		colour += color_offset;

		INT32 fx = y & 0x2000;
		INT32 fy = y & 0x4000;
		INT32 multi = (1 << ((y & 0x1800) >> 11)) - 1;

		INT32 sprite = DrvSprBuf[offs+3]+(DrvSprBuf[offs+2]<<8);
		sprite &= 0x0fff;

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 256) x -= 512;
		if (y >= 256) y -= 512;
		x = 240 - x;
		y = 240 - y;

		INT32 mult, inc;
		sprite &= ~multi;
		if (fy)
			inc = -1;
		else
		{
			sprite += multi;
			inc = 1;
		}

		if (*flipscreen)
		{
			y=240-y;
			x=240-x;
			if (fx) fx=0; else fx=1;
			if (fy) fy=0; else fy=1;
			mult=16;
		}
		else mult=-16;

		y -= 8;

		while (multi >= 0)
		{
			Draw16x16MaskTile(pTransDraw, (sprite - multi * inc) & 0xfff, x, y + mult * multi, fx, fy, colour, 4, 0, 0, DrvGfxROM1);

			multi--;
		}
	}
}

static INT32 CobraDraw()
{
	if (DrvRecalc) {
		UINT8 r,g,b;
		for (INT32 i = 0; i < 0x200; i+=2) {
			INT32 d = (DrvPalRAM[i + 0] << 8) | DrvPalRAM[i + 1];
			r = (d >> 0) & 0x0f;
			g = (d >> 4) & 0x0f;
			b = (d >> 8) & 0x0f;

			r |= r << 4;
			g |= g << 4;
			b |= b << 4;

			DrvPalette[i/2] = BurnHighCol(r, g, b, 0);
		}
	}

	DrvPf0Ctrl[0] &= ~0x04; // disable line scrolling since this game
	DrvPf1Ctrl[1] &= ~0x04; // doesn't have ram for the offsets

	BurnTransferClear();

	if (nBurnLayer & 1) draw_bg_layer(DrvPf0RAM, DrvPf0Ctrl, DrvGfxROM2, 0x80, 0x03, 0xfff, 0, 0);
	if (nSpriteEnable & 1) draw_sprites2(0x03, 1, 0x04);
	if (nBurnLayer & 2) draw_bg_layer(DrvPf1RAM, DrvPf1Ctrl, DrvGfxROM3, 0xc0, 0x03, 0xfff, 1, 0);
	if (nSpriteEnable & 2) draw_sprites2(0x03, 2, 0x04);
	if (nBurnLayer & 4) draw_cobra_txt_layer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 CobraFrame()
{
	if (DrvReset) {
		CobraDoReset();
	}

	M6809NewFrame();
	M6502NewFrame();

	{
		memset (DrvInputs, 0xff, 5);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
			DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
		}
	}

	INT32 nInterleave = 32;
	INT32 nCyclesTotal[2] = { 2000000 / 58, 1500000 / 58 };
	INT32 nCyclesDone[2] = { 0, 0 };

	M6502Open(0);
	M6809Open(0);

	vblank = 0x80;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		if (i == 1) vblank = 0x80;
		if (i == 31) {
			vblank = 0;
			M6809SetIRQLine(0x20 /*NMI*/, CPU_IRQSTATUS_AUTO);
		}
		CPU_RUN(0, M6809);

		CPU_RUN_TIMER(1);
	}

	M6809Close();
	M6502Close();

	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		CobraDraw();
	}

	return 0;
}

// Cobra-Command (World/US revision 5)

static struct BurnRomInfo cobracomRomDesc[] = {
	{ "el11-5.5j",	0x08000, 0xaf0a8b05, 1 }, //  0 maincpu
	{ "el12-4.7j",	0x10000, 0x7a44ef38, 1 }, //  1
	{ "el13.9j",	0x10000, 0x04505acb, 1 }, //  2

	{ "el10-4.1f",	0x08000, 0xedfad118, 2 }, //  3 audiocpu

	{ "el14.14j",	0x08000, 0x47246177, 3 }, //  4 gfx1

	{ "el00-4.2a",	0x10000, 0x122da2a8, 4 }, //  5 gfx2
	{ "el01-4.3a",	0x10000, 0x27bf705b, 4 }, //  6
	{ "el02-4.5a",	0x10000, 0xc86fede6, 4 }, //  7
	{ "el03-4.6a",	0x10000, 0x1d8a855b, 4 }, //  8

	{ "el08.7d",	0x10000, 0xcb0dcf4c, 5 }, //  9 gfx4
	{ "el09.9d",	0x10000, 0x1fae5be7, 5 }, // 10

	{ "el05.15a",	0x10000, 0x1c4f6033, 6 }, // 11 gfx3
	{ "el06.16a",	0x10000, 0xd24ba794, 6 }, // 12
	{ "el04.13a",	0x10000, 0xd80a49ce, 6 }, // 13
	{ "el07.18a",	0x10000, 0x6d771fc3, 6 }, // 14

	{ "eh15.12f",	0x00200, 0x279e540c, 7 }, // 15 proms

	{ "pt-0.16h",	0x00117, 0xadd5074f, 8 }, // 16 plds
};

STD_ROM_PICK(cobracom)
STD_ROM_FN(cobracom)

static INT32 CobraExit()
{
	M6809Exit();
	BurnYM3812Exit();

	return DrvExit();
}

static INT32 CobraScan(INT32 nAction, INT32 *pnMin)
{
	if (nAction & ACB_DRIVER_DATA) {
		M6809Scan(nAction);
		BurnYM3812Scan(nAction, pnMin);
		SCAN_VAR(RomBank);

		if (nAction & ACB_WRITE) {
			M6809Open(0);
			m6809_bankswitch(RomBank);
			M6809Close();
		}
	}

	return DrvScan(nAction, pnMin);
}

struct BurnDriver BurnDrvCobracom = {
	"cobracom", NULL, NULL, NULL, "1988",
	"Cobra-Command (World/US revision 5)\0", NULL, "Data East Corporation", "DEC8",
	L"\u773C\u775B\u86C7\u6307\u4EE4 (\u4E16\u754C\u7248/\u7F8E\u7248 \u4FEE\u8BA2\u7248 5)\0"
, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_HORSHOOT, 0,
	NULL, cobracomRomInfo, cobracomRomName, NULL, NULL, NULL, NULL, CobracomInputInfo, CobracomDIPInfo,
	CobraInit, CobraExit, CobraFrame, CobraDraw, CobraScan, &DrvRecalc, 0x100,
	256, 240, 4, 3
};


// Cobra-Command (World/US revision 4)

static struct BurnRomInfo cobracomaRomDesc[] = {
	{ "el11-4.5j",	0x08000, 0x6dca6734, 1 }, //  0 maincpu
	{ "el12-4.7j",	0x10000, 0x7a44ef38, 1 }, //  1
	{ "el13.9j",	0x10000, 0x04505acb, 1 }, //  2

	{ "el10-4.1f",	0x08000, 0xedfad118, 2 }, //  3 audiocpu

	{ "el14.14j",	0x08000, 0x47246177, 3 }, //  4 gfx1

	{ "el00-4.2a",	0x10000, 0x122da2a8, 4 }, //  5 gfx2
	{ "el01-4.3a",	0x10000, 0x27bf705b, 4 }, //  6
	{ "el02-4.5a",	0x10000, 0xc86fede6, 4 }, //  7
	{ "el03-4.6a",	0x10000, 0x1d8a855b, 4 }, //  8

	{ "el08.7d",	0x10000, 0xcb0dcf4c, 5 }, //  9 gfx4
	{ "el09.9d",	0x10000, 0x1fae5be7, 5 }, // 10

	{ "el05.15a",	0x10000, 0x1c4f6033, 6 }, // 11 gfx3
	{ "el06.16a",	0x10000, 0xd24ba794, 6 }, // 12
	{ "el04.13a",	0x10000, 0xd80a49ce, 6 }, // 13
	{ "el07.18a",	0x10000, 0x6d771fc3, 6 }, // 14

	{ "eh15.12f",	0x00200, 0x279e540c, 7 }, // 15 proms

	{ "pt-0.16h",	0x00117, 0xadd5074f, 8 }, // 16 plds
};

STD_ROM_PICK(cobracoma)
STD_ROM_FN(cobracoma)

struct BurnDriver BurnDrvCobracoma = {
	"cobracoma", "cobracom", NULL, NULL, "1988",
	"Cobra-Command (World/US revision 4)\0", NULL, "Data East Corporation", "DEC8",
	L"\u773C\u775B\u86C7\u6307\u4EE4 (\u4E16\u754C\u7248/\u7F8E\u7248 \u4FEE\u8BA2\u7248 4)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_HORSHOOT, 0,
	NULL, cobracomaRomInfo, cobracomaRomName, NULL, NULL, NULL, NULL, CobracomInputInfo, CobracomDIPInfo,
	CobraInit, CobraExit, CobraFrame, CobraDraw, CobraScan, &DrvRecalc, 0x100,
	256, 240, 4, 3
};


// Cobra-Command (World/US)

static struct BurnRomInfo cobracombRomDesc[] = {
	{ "el11.5j",	0x08000, 0xc6a102e3, 1 }, //  0 maincpu
	{ "el12.7j",	0x10000, 0x72b2dab4, 1 }, //  1
	{ "el13.9j",	0x10000, 0x04505acb, 1 }, //  2

	{ "el10.1f",	0x08000, 0x62ca5e89, 2 }, //  3 audiocpu

	{ "el14.14j",	0x08000, 0x47246177, 3 }, //  4 gfx1

	{ "el00.2a",	0x10000, 0xd96b6797, 4 }, //  5 gfx2
	{ "el01.3a",	0x10000, 0x3fef9c02, 4 }, //  6
	{ "el02.5a",	0x10000, 0xbfae6c34, 4 }, //  7
	{ "el03.6a",	0x10000, 0xd56790f8, 4 }, //  8

	{ "el08.7d",	0x10000, 0xcb0dcf4c, 5 }, //  9 gfx4
	{ "el09.9d",	0x10000, 0x1fae5be7, 5 }, // 10

	{ "el05.15a",	0x10000, 0x1c4f6033, 6 }, // 11 gfx3
	{ "el06.16a",	0x10000, 0xd24ba794, 6 }, // 12
	{ "el04.13a",	0x10000, 0xd80a49ce, 6 }, // 13
	{ "el07.18a",	0x10000, 0x6d771fc3, 6 }, // 14

	{ "eh15.12f",	0x00200, 0x279e540c, 7 }, // 15 proms

	{ "pt-0.16h",	0x00117, 0xadd5074f, 8 }, // 16 plds
};

STD_ROM_PICK(cobracomb)
STD_ROM_FN(cobracomb)

struct BurnDriver BurnDrvCobracomb = {
	"cobracomb", "cobracom", NULL, NULL, "1988",
	"Cobra-Command (World/US)\0", NULL, "Data East Corporation", "DEC8",
	L"\u773C\u775B\u86C7\u6307\u4EE4 (\u4E16\u754C\u7248/\u7F8E\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_HORSHOOT, 0,
	NULL, cobracombRomInfo, cobracombRomName, NULL, NULL, NULL, NULL, CobracomInputInfo, CobracomDIPInfo,
	CobraInit, CobraExit, CobraFrame, CobraDraw, CobraScan, &DrvRecalc, 0x100,
	256, 240, 4, 3
};


// Cobra-Command (Japan)

static struct BurnRomInfo cobracmjRomDesc[] = {
	{ "eh11.5j",	0x08000, 0x868637e1, 1 }, //  0 maincpu
	{ "eh12.7j",	0x10000, 0x7c878a83, 1 }, //  1
	{ "el13.9j",	0x10000, 0x04505acb, 1 }, //  2

	{ "eh10.1f",	0x08000, 0x62ca5e89, 2 }, //  3 audiocpu

	{ "eh14.14j",	0x08000, 0x47246177, 3 }, //  4 gfx1

	{ "eh00.2a",	0x10000, 0xd96b6797, 4 }, //  5 gfx2
	{ "eh01.3a",	0x10000, 0x3fef9c02, 4 }, //  6
	{ "eh02.5a",	0x10000, 0xbfae6c34, 4 }, //  7
	{ "eh03.6a",	0x10000, 0xd56790f8, 4 }, //  8

	{ "eh08.7d",	0x10000, 0xcb0dcf4c, 5 }, //  9 gfx4
	{ "eh09.9d",	0x10000, 0x1fae5be7, 5 }, // 10

	{ "eh05.15a",	0x10000, 0x1c4f6033, 6 }, // 11 gfx3
	{ "eh06.16a",	0x10000, 0xd24ba794, 6 }, // 12
	{ "eh04.13a",	0x10000, 0xd80a49ce, 6 }, // 13
	{ "eh07.18a",	0x10000, 0x6d771fc3, 6 }, // 14

	{ "eh15.12f",	0x00200, 0x279e540c, 7 }, // 15 proms

	{ "pt-0.16h",	0x00117, 0xadd5074f, 8 }, // 16 plds
};

STD_ROM_PICK(cobracmj)
STD_ROM_FN(cobracmj)

struct BurnDriver BurnDrvCobracmj = {
	"cobracomj", "cobracom", NULL, NULL, "1988",
	"Cobra-Command (Japan)\0", NULL, "Data East Corporation", "DEC8",
	L"\u773C\u775B\u86C7\u6307\u4EE4 (\u65E5\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_HORSHOOT, 0,
	NULL, cobracmjRomInfo, cobracmjRomName, NULL, NULL, NULL, NULL, CobracomInputInfo, CobracomDIPInfo,
	CobraInit, CobraExit, CobraFrame, CobraDraw, CobraScan, &DrvRecalc, 0x100,
	256, 240, 4, 3
};


// Cobra-Command (Japan?, set 2)

static struct BurnRomInfo cobracmjaRomDesc[] = {
	{ "el-11.rom",	0x08000, 0xc6a102e3, 1 }, //  0 maincpu
	{ "el-12.rom",	0x10000, 0x72b2dab4, 1 }, //  1
	{ "el-13.rom",	0x10000, 0x04505acb, 1 }, //  2

	{ "el-10.rom",	0x08000, 0x62ca5e89, 2 }, //  3 audiocpu

	{ "el-14.rom",	0x08000, 0x47246177, 3 }, //  4 gfx1

	{ "eh-00.rom",	0x10000, 0xd96b6797, 4 }, //  5 gfx2
	{ "eh-01.rom",	0x10000, 0x3fef9c02, 4 }, //  6
	{ "eh-02.rom",	0x10000, 0xbfae6c34, 4 }, //  7
	{ "eh-03.rom",	0x10000, 0xd56790f8, 4 }, //  8

	{ "el-08.rom",	0x10000, 0xcb0dcf4c, 5 }, //  9 gfx4
	{ "el-09.rom",	0x10000, 0x1fae5be7, 5 }, // 10

	{ "el-05.rom",	0x10000, 0x1c4f6033, 6 }, // 11 gfx3
	{ "el-06.rom",	0x10000, 0xd24ba794, 6 }, // 12
	{ "el-04.rom",	0x10000, 0xd80a49ce, 6 }, // 13
	{ "el-07.rom",	0x10000, 0x6d771fc3, 6 }, // 14
};

STD_ROM_PICK(cobracmja)
STD_ROM_FN(cobracmja)

struct BurnDriver BurnDrvCobracmja = {
	"cobracomja", "cobracom", NULL, NULL, "1988",
	"Cobra-Command (Japan?, set 2)\0", NULL, "Data East Corporation", "DEC8",
	L"\u773C\u775B\u86C7\u6307\u4EE4 (\u65E5\u7248?, \u7B2C\u4E8C\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_HORSHOOT, 0,
	NULL, cobracmjaRomInfo, cobracmjaRomName, NULL, NULL, NULL, NULL, CobracomInputInfo, CobracomDIPInfo,
	CobraInit, CobraExit, CobraFrame, CobraDraw, CobraScan, &DrvRecalc, 0x100,
	256, 240, 4, 3
};


// Cobra-Command (Italian bootleg)
// f205v id 835

static struct BurnRomInfo cobracomibRomDesc[] = {
	{ "a1.bin",		0x08000, 0xbb38b64c, 1 }, //  0 maincpu
	{ "a2.bin",		0x10000, 0x7c878a83, 1 }, //  1
	{ "a3.bin",		0x10000, 0x04505acb, 1 }, //  2

	{ "a5.bin",		0x08000, 0x62ca5e89, 2 }, //  3 audiocpu

	{ "a4.bin",	    0x08000, 0x47246177, 3 }, //  4 gfx1

	{ "5.bin",		0x10000, 0xd96b6797, 4 }, //  5 gfx2
	{ "6.bin",		0x10000, 0x3fef9c02, 4 }, //  6
	{ "7.bin",		0x10000, 0xbfae6c34, 4 }, //  7
	{ "8.bin",		0x10000, 0xeaeb700e, 4 }, //  8

	{ "a9.bin",		0x08000, 0xc991298f, 5 }, //  9 gfx4
	{ "a8.bin",		0x08000, 0xf5e267e5, 5 }, // 10
	{ "a7.bin",		0x08000, 0x6bcc5982, 5 }, // 11
	{ "a6.bin",		0x08000, 0xc90443b5, 5 }, // 12

	{ "1.bin",		0x10000, 0x1c4f6033, 6 }, // 13 gfx3
	{ "2.bin",		0x10000, 0xd24ba794, 6 }, // 14
	{ "3.bin",		0x10000, 0xd80a49ce, 6 }, // 15
	{ "4.bin",		0x10000, 0x6d771fc3, 6 }, // 16
};

STD_ROM_PICK(cobracomib)
STD_ROM_FN(cobracomib)

struct BurnDriver BurnDrvCobracomib = {
	"cobracomib", "cobracom", NULL, NULL, "1988",
	"Cobra-Command (Italian bootleg)\0", NULL, "bootleg", "DEC8",
	L"\u773C\u775B\u86C7\u6307\u4EE4 (\u610F\u7248 \u76D7\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_HORSHOOT, 0,
	NULL, cobracomibRomInfo, cobracomibRomName, NULL, NULL, NULL, NULL, CobracomInputInfo, CobracomDIPInfo,
	CobraInit, CobraExit, CobraFrame, CobraDraw, CobraScan, &DrvRecalc, 0x100,
	256, 240, 4, 3
};

// Cobra-Command (Japan, bootleg)
// f205v id 375
// bootleg of the Japanese release manufactured in Italy. Contents are the same but for PROMs and PAL
// main PCB is marked: "LC" on component side ("LC" is the Italian for "Lato Componenti" which translates to "Components Side")
// main PCB is marked: "LS" on solder side ("LS" is the Italian for "Lato Saldature" which translates to "Solders Side")

static struct BurnRomInfo cobracomjbRomDesc[] = {
	{ "cobra4.bin",			0x08000, 0x868637e1, 1 }, //  0 maincpu
	{ "cobra3.bin",			0x10000, 0x7c878a83, 1 }, //  1
	{ "cobra2.bin",			0x10000, 0x04505acb, 1 }, //  2

	{ "cobra5.bin",			0x08000, 0x62ca5e89, 2 }, //  3 audiocpu

	{ "cobra1.bin",	    	0x08000, 0x47246177, 3 }, //  4 gfx1

	{ "cob17.bin",			0x10000, 0xd96b6797, 4 }, //  5 gfx2
	{ "cob16.bin",			0x10000, 0x3fef9c02, 4 }, //  6
	{ "cob15.bin",			0x10000, 0xbfae6c34, 4 }, //  7
	{ "cob14.bin",			0x10000, 0xd56790f8, 4 }, //  8

	{ "cobra6.bin",			0x08000, 0xc991298f, 5 }, //  9 gfx4
	{ "cobra7.bin",			0x08000, 0xf5e267e5, 5 }, // 10
	{ "cobra8.bin",			0x08000, 0x6bcc5982, 5 }, // 11
	{ "cobra9.bin",			0x08000, 0xc90443b5, 5 }, // 12

	{ "cob13.bin",			0x10000, 0x1c4f6033, 6 }, // 13 gfx3
	{ "cob12.bin",			0x10000, 0xd24ba794, 6 }, // 14
	{ "cob11.bin",			0x10000, 0xd80a49ce, 6 }, // 15
	{ "cob10.bin",			0x10000, 0x6d771fc3, 6 }, // 16

	{ "63s481n.3",			0x00200, 0x279e540c, 7 }, // 17 proms
	{ "am27s21pc.1",		0x00100, 0x9f6aa3e5, 7 }, // 18
	{ "am27s21pc.2",		0x00100, 0xaf46d1ee, 7 }, // 19
	{ "am27s21pc.3",		0x00100, 0x1e4189e8, 7 }, // 20

	{ "pal16l8a-2cn.bin",	0x00104, 0x3ef8cf68, 8 }, // 21 plds
};

STD_ROM_PICK(cobracomjb)
STD_ROM_FN(cobracomjb)

struct BurnDriver BurnDrvCobracomjb = {
	"cobracomjb", "cobracom", NULL, NULL, "1988",
	"Cobra-Command (Japan, bootleg)\0", NULL, "bootleg", "DEC8",
	L"\u773C\u775B\u86C7\u6307\u4EE4 (\u65E5\u7248, \u76D7\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_HORSHOOT, 0,
	NULL, cobracomjbRomInfo, cobracomjbRomName, NULL, NULL, NULL, NULL, CobracomInputInfo, CobracomDIPInfo,
	CobraInit, CobraExit, CobraFrame, CobraDraw, CobraScan, &DrvRecalc, 0x100,
	256, 240, 4, 3
};

static void srdarwin_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x1800:
		case 0x1801:
			i8751_write(address & 1, data);
		return;

		case 0x1802:
			// hmm?
		return;

		case 0x1804:
			memcpy (DrvSprBuf, DrvSprRAM, 0x200);
		return;

		case 0x1805:
			m6809_bankswitch(data >> 5);
			DrvPf0Ctrl[0x10] = data & 0x01;
		return;

		case 0x1806:
			DrvPf0Ctrl[0x11] = data;
		return;

		case 0x2000:
			*soundlatch = data;
			M6502SetIRQLine(M6502_INPUT_LINE_NMI, CPU_IRQSTATUS_AUTO);
		return;

		case 0x2001:
			*flipscreen = data;
		return;
	}
}

static UINT8 srdarwin_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x2000:
			return i8751_hi();

		case 0x2001:
			return i8751_lo();

		case 0x3800:
			return DrvDips[0];

		case 0x3801:
			return DrvInputs[0];

		case 0x3802:
			return (DrvInputs[1] & ~0x40) | vblank;

		case 0x3803:
			return DrvDips[1];
	}

	return 0;
}

static INT32 SrdarwinGfxDecode()
{
	INT32 Plane0[2]  = { 0, 4 };
	INT32 Plane1[4]  = { 0x10000*8,0x20000*8,0x00000*8 };
	INT32 Plane2[4] = { 0x4000*8, 0x4000*8+4, 0, 4 };

	INT32 XOffs0[8]  = { 0x2000*8+0, 0x2000*8+1, 0x2000*8+2, 0x2000*8+3, 0, 1, 2, 3 };
	INT32 XOffs1[16] = { 16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),0,1,2,3,4,5,6,7 };
	INT32 XOffs2[16] = { 0, 1, 2, 3, 1024*8*8+0, 1024*8*8+1, 1024*8*8+2, 1024*8*8+3,
			16*8+0, 16*8+1, 16*8+2, 16*8+3, 16*8+1024*8*8+0, 16*8+1024*8*8+1, 16*8+1024*8*8+2, 16*8+1024*8*8+3 };

	INT32 YOffs[16]  = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x80000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x08000);

	GfxDecode(0x0400, 2,  8,  8, Plane0, XOffs0, YOffs, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x80000);

	GfxDecode(0x0800, 3, 16, 16, Plane1, XOffs1, YOffs, 0x100, tmp, DrvGfxROM1);

	for (INT32 i = 0; i < 4; i++) {
		memcpy (tmp, DrvGfxROM2 + i * 0x10000, 0x08000);
		GfxDecode(0x0100, 4, 16, 16, Plane2, XOffs2, YOffs, 0x100, tmp, DrvGfxROM2 + i * 0x10000);
	}

	BurnFree (tmp);

	return 0;
}

static INT32 SrdarwinInit()
{
	BurnSetRefreshRate(58.00);

	BurnAllocMemIndex();

	{
		if (BurnLoadRom(DrvMainROM + 0x20000,  0, 1)) return 1;
		if (BurnLoadRom(DrvMainROM + 0x10000,  1, 1)) return 1;
		memcpy (DrvMainROM + 0x8000, DrvMainROM + 0x28000, 0x08000);

		if (BurnLoadRom(DrvM6502ROM  + 0x08000,  2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0   + 0x00000,  3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1   + 0x00000,  4, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x08000,  5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x10000,  6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x18000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x20000,  8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x28000,  9, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3   + 0x00000, 10, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x00000, DrvGfxROM3 + 0x00000, 0x04000);
		memcpy (DrvGfxROM2 + 0x10000, DrvGfxROM3 + 0x04000, 0x04000);
		memcpy (DrvGfxROM2 + 0x20000, DrvGfxROM3 + 0x08000, 0x04000);
		memcpy (DrvGfxROM2 + 0x30000, DrvGfxROM3 + 0x0c000, 0x04000);
		if (BurnLoadRom(DrvGfxROM3   + 0x00000, 11, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x04000, DrvGfxROM3 + 0x00000, 0x04000);
		memcpy (DrvGfxROM2 + 0x14000, DrvGfxROM3 + 0x04000, 0x04000);
		memcpy (DrvGfxROM2 + 0x24000, DrvGfxROM3 + 0x08000, 0x04000);
		memcpy (DrvGfxROM2 + 0x34000, DrvGfxROM3 + 0x0c000, 0x04000);

		if (BurnLoadRom(DrvMCURom    + 0x00000, 12, 1)) return 1;

		SrdarwinGfxDecode();
	}

	M6809Init(0);
	M6809Open(0);
	M6809MapMemory(DrvMainRAM,		0x0000, 0x05ff, MAP_RAM);
	M6809MapMemory(DrvSprRAM,		0x0600, 0x07ff, MAP_RAM);
	M6809MapMemory(DrvVidRAM,		0x0800, 0x0fff, MAP_RAM);
	M6809MapMemory(DrvMainRAM + 0x1000,	0x1000, 0x13ff, MAP_RAM);
	M6809MapMemory(DrvPf0RAM,		0x1400, 0x17ff, MAP_RAM);
	M6809MapMemory(DrvPalRAM,		0x2800, 0x28ff, MAP_RAM);
	M6809MapMemory(DrvPalRAM + 0x100,	0x3000, 0x30ff, MAP_RAM);
	M6809MapMemory(DrvMainROM + 0x10000,  0x4000, 0x7fff, MAP_ROM);
	M6809MapMemory(DrvMainROM + 0x08000,  0x8000, 0xffff, MAP_ROM);
	M6809SetWriteHandler(srdarwin_main_write);
	M6809SetReadHandler(srdarwin_main_read);
	M6809Close();

	M6502Init(0, TYPE_DECO222);
	M6502Open(0);
	M6502MapMemory(DrvM6502RAM,          0x0000, 0x05ff, MAP_RAM);
	M6502MapMemory(DrvM6502ROM + 0x8000, 0x8000, 0xffff, MAP_ROM);
	M6502SetReadHandler(ghostb_sound_read);
	M6502SetWriteHandler(ghostb_sound_write);
	M6502Close();

	DrvMCUInit(1); // srdarwin

	BurnYM2203Init(1, 1500000, NULL, 0);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_YM2203_ROUTE, 0.20, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_1, 0.23, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_2, 0.23, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_3, 0.23, BURN_SND_ROUTE_BOTH);
	BurnTimerAttach(&M6502Config, 1500000);

	BurnYM3812Init(1, 3000000, &DrvYM3812FMIRQHandler, 1);
	BurnYM3812SetRoute(0, BURN_SND_YM3812_ROUTE, 0.70, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	CobraDoReset();

	return 0;
}

static void Srdarwin_draw_layer(INT32 layer)
{
	INT32 scrollx = (DrvPf0Ctrl[0x10] << 8) | DrvPf0Ctrl[0x11];

	UINT16 layer0t[4] = { 0x0000, 0xff00, 0xff00, 0xffff };
	UINT16 layer1t[4] = { 0xffff, 0x00ff, 0x00ff, 0x0000 };
	UINT16 *layert = layer ? layer1t : layer0t;

	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 4;
		INT32 sy = (offs >> 5) << 4;

		sy -= 8;
		if (sy >= nScreenHeight) continue;
		sx -= scrollx;
		if (sx < -15) sx += 512;
		if (sx >= nScreenWidth) continue;

		INT32 code  = DrvPf0RAM[offs * 2 + 1] | (DrvPf0RAM[offs * 2 + 0] << 8);
		INT32 color = (code >> 12) & 3;

		{
			code &= 0x3ff;
			color <<= 4;

			INT32 t = layert[color>>4];
			UINT8 *src = DrvGfxROM2 + (code << 8);
			UINT16 *dst;

			for (INT32 y = 0; y < 16; y++, sy++, src+=16)
			{
				if (sy < 0 || sy >= nScreenHeight) continue;

				dst = pTransDraw + sy * nScreenWidth;

				INT32 xx = sx;

				for (INT32 x = 0; x < 16; x++, xx++)
				{
					INT32 pxl = src[x];

					if (t & (1 << pxl)) continue;
					if (xx < 0 || xx >= nScreenWidth) continue;

					dst[xx] = pxl | color;
				}
			}
		}
	}
}

static void srdarwin_txt_draw()
{
	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;
		sy -= 8;
		if (sy < 0 || sy >= nScreenHeight) continue;

		INT32 code = DrvVidRAM[offs];

		Render8x8Tile_Mask(pTransDraw, code, sx, sy, 0, 2, 0, 0x80, DrvGfxROM0);
	}
}

static void srdarwin_draw_sprites(INT32 pri)
{
	for (INT32 offs = 0; offs < 0x200; offs += 4)
	{
		INT32 multi, fx, fy, sx, sy, sy2, code, color;

		color = (DrvSprBuf[offs+1] & 0x03) + ((DrvSprBuf[offs+1] & 0x08) >> 1);
		if (pri == 0 && color != 0) continue;
		if (pri == 1 && color == 0) continue;

		code = DrvSprBuf[offs+3] + ( (DrvSprBuf[offs+1] & 0xe0) << 3 );
		if (!code) continue;

		sy = DrvSprBuf[offs];
		if (sy == 0xf8) continue;

		sx = (241 - DrvSprBuf[offs+2]);

		fx = (DrvSprBuf[offs+1] & 0x04) >> 2;
		fy = *flipscreen;
		multi = DrvSprBuf[offs+1] & 0x10;

#if 0
		// disable coctail flipping for sprites - allows for 2p netgame or 2p/2joy
		if (*flipscreen)
		{
			sy=240-sy;
			sx=240-sx;
			if (fx) fx=0; else fx=1;
			sy2=sy-16;
		}
		else sy2=sy+16;
#endif
		// coctail unflipping
		if (*flipscreen)
		{
			fy ^= 1;
		}
		sy2=sy+16;
		// end coctail unflipping

		sy -= 8;
		sy2 -= 8;

		Draw16x16MaskTile(pTransDraw, code, sx, sy, fx, fy, color, 3, 0, 0x40, DrvGfxROM1);

		if (multi) {
			code++;
			Draw16x16MaskTile(pTransDraw, code, sx, sy2, fx, fy, color, 3, 0, 0x40, DrvGfxROM1);
		}
	}
}

static INT32 SrdarwinDraw()
{
	if (DrvRecalc) {
		UINT8 r,g,b;
		for (INT32 i = 0; i < 0x100; i++) {
			INT32 d = (DrvPalRAM[i + 0]) | (DrvPalRAM[i + 0x100] << 8);
			r = (d >> 0) & 0x0f;
			g = (d >> 4) & 0x0f;
			b = (d >> 8) & 0x0f;

			r |= r << 4;
			g |= g << 4;
			b |= b << 4;

			DrvPalette[i] = BurnHighCol(r, g, b, 0);
		}
	}
	BurnTransferClear();
	if (nBurnLayer & 1) Srdarwin_draw_layer(0);
	if (nSpriteEnable & 1) srdarwin_draw_sprites(0);
	if (nBurnLayer & 2) Srdarwin_draw_layer(1);
	if (nSpriteEnable & 2) srdarwin_draw_sprites(1);

	if (nBurnLayer & 4) srdarwin_txt_draw();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 SrdarwinFrame()
{
	if (DrvReset) {
		CobraDoReset();
	}

	M6809NewFrame();
	M6502NewFrame();
	mcs51NewFrame();

	{
		memset (DrvInputs, 0xff, sizeof(DrvInputs));
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
		}
	}

	INT32 nInterleave = 256;
	INT32 nCyclesTotal[3] = { 2000000 / 58, 1500000 / 58, 12000000 / 12 / 58 };
	INT32 nCyclesDone[3] = { 0, 0, 0 };

	M6502Open(0);
	M6809Open(0);

	vblank = 0;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		if (i == 8) vblank = 0x40;
		if (i == 254) {
			vblank = 0;
			M6809SetIRQLine(0x20 /*NMI*/, CPU_IRQSTATUS_AUTO);
		}

		CPU_RUN(0, M6809);

		if (realMCU) {
			DrvMCUSync();
		}

		CPU_RUN_TIMER(1);
	}

	M6809Close();
	M6502Close();

	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		BurnYM3812Update(pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		SrdarwinDraw();
	}

	return 0;
}

// SRD: Super Real Darwin (World)

static struct BurnRomInfo srdarwinRomDesc[] = {
	{ "dy_01-e.b14",	0x10000, 0x176e9299, 1 }, //  0 maincpu
	{ "dy_00.b16",		0x10000, 0x2bf6b461, 1 }, //  1

	{ "dy_04.d7",		0x08000, 0x2ae3591c, 2 }, //  2 audiocpu

	{ "dy_05.b6",		0x04000, 0x8780e8a3, 3 }, //  3 gfx1

	{ "dy_07.h16",		0x08000, 0x97eaba60, 4 }, //  4 gfx2
	{ "dy_06.h14",		0x08000, 0xc279541b, 4 }, //  5
	{ "dy_09.k13",		0x08000, 0xd30d1745, 4 }, //  6
	{ "dy_08.k11",		0x08000, 0x71d645fd, 4 }, //  7
	{ "dy_11.k16",		0x08000, 0xfd9ccc5b, 4 }, //  8
	{ "dy_10.k14",		0x08000, 0x88770ab8, 4 }, //  9

	{ "dy_03.b4",		0x10000, 0x44f2a4f9, 5 }, // 10 gfx3
	{ "dy_02.b5",		0x10000, 0x522d9a9e, 5 }, // 11

	{ "dy-e.d11",		0x01000, 0x11cd6ca4, 6 }, // 12 i8751 microcontroller

	{ "dy_12.f4",		0x00100, 0xebfaaed9, 7 }, // 13 proms
};

STD_ROM_PICK(srdarwin)
STD_ROM_FN(srdarwin)

struct BurnDriver BurnDrvSrdarwin = {
	"srdarwin", NULL, NULL, NULL, "1987",
	"SRD: Super Real Darwin (World)\0", NULL, "Data East Corporation", "DEC8",
	L"\u8D85\u771F\u5B9E\u8FBE\u5C14\u6587\u6218\u673A (\u4E16\u754C\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_VERSHOOT, 0,
	NULL, srdarwinRomInfo, srdarwinRomName, NULL, NULL, NULL, NULL, SrdarwinInputInfo, SrdarwinDIPInfo,
	SrdarwinInit, CobraExit, SrdarwinFrame, SrdarwinDraw, CobraScan, &DrvRecalc, 0x100,
	240, 256, 3, 4
};


// SRD: Super Real Darwin (Japan)

static struct BurnRomInfo srdarwnjRomDesc[] = {
	{ "dy_01.b14",			0x10000, 0x1eeee4ff, 1 }, //  0 maincpu
	{ "dy_00.b16",			0x10000, 0x2bf6b461, 1 }, //  1

	{ "dy_04.d7",			0x08000, 0x2ae3591c, 2 }, //  2 audiocpu

	{ "dy_05.b6",			0x04000, 0x8780e8a3, 3 }, //  3 gfx1

	{ "dy_07.h16",			0x08000, 0x97eaba60, 4 }, //  4 gfx2
	{ "dy_06.h14",			0x08000, 0xc279541b, 4 }, //  5
	{ "dy_09.k13",			0x08000, 0xd30d1745, 4 }, //  6
	{ "dy_08.k11",			0x08000, 0x71d645fd, 4 }, //  7
	{ "dy_11.k16",			0x08000, 0xfd9ccc5b, 4 }, //  8
	{ "dy_10.k14",			0x08000, 0x88770ab8, 4 }, //  9

	{ "dy_03.b4",			0x10000, 0x44f2a4f9, 5 }, // 10 gfx3
	{ "dy_02.b5",			0x10000, 0x522d9a9e, 5 }, // 11

	{ "dy.d11",				0x01000, 0x4ac2ca9d, 6 }, // 12 i8751 microcontroller

	{ "dy_12.f4",			0x00100, 0xebfaaed9, 7 }, // 13 proms
};

STD_ROM_PICK(srdarwnj)
STD_ROM_FN(srdarwnj)

struct BurnDriver BurnDrvSrdarwnj = {
	"srdarwinj", "srdarwin", NULL, NULL, "1987",
	"SRD: Super Real Darwin (Japan)\0", NULL, "Data East Corporation", "DEC8",
	L"\u8D85\u771F\u5B9E\u8FBE\u5C14\u6587\u6218\u673A (\u65E5\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_VERSHOOT, 0,
	NULL, srdarwnjRomInfo, srdarwnjRomName, NULL, NULL, NULL, NULL, SrdarwinInputInfo, SrdarwinjDIPInfo,
	SrdarwinInit, CobraExit, SrdarwinFrame, SrdarwinDraw, CobraScan, &DrvRecalc, 0x100,
	240, 256, 3, 4
};

// Rotation-handler code (gondomania)

static void RotateReset() {
	for (INT32 playernum = 0; playernum < 2; playernum++) {
		nRotate[playernum] = 0; // start out pointing straight up (0=up)
		nRotateTarget[playernum] = -1;
		nRotateTime[playernum] = 0;
		nRotateHoldInput[0] = nRotateHoldInput[1] = 0;
	}
}

static UINT32 RotationTimer(void) {
    return nCurrentFrame;
}

static void RotateRight(INT32 *v) {
    (*v)--;
    if (*v < 0) *v = 11;
}

static void RotateLeft(INT32 *v) {
    (*v)++;
    if (*v > 11) *v = 0;
}

static UINT8 Joy2Rotate(UINT8 *joy) { // joy position to rotational position
	if (joy[0] && joy[2]) return 7;    // up left
	if (joy[0] && joy[3]) return 1;    // up right

	if (joy[1] && joy[2]) return 5;    // down left
	if (joy[1] && joy[3]) return 3;    // down right

	if (joy[0]) return 0;    // up
	if (joy[1]) return 4;    // down
	if (joy[2]) return 6;    // left
	if (joy[3]) return 2;    // right

	return 0xff;
}

static int dialRotation(INT32 playernum) {
    // p1 = 0, p2 = 1
	UINT8 player[2] = { 0, 0 };
	static UINT8 lastplayer[2][2] = { { 0, 0 }, { 0, 0 } };

    if ((playernum != 0) && (playernum != 1)) {
		//bprintf(PRINT_NORMAL, _T("Strange Rotation address => %06X\n"), playernum);
        return 0;
    }
    if (playernum == 0) {
        player[0] = DrvFakeInput[0]; player[1] = DrvFakeInput[1];
    }
    if (playernum == 1) {
        player[0] = DrvFakeInput[2]; player[1] = DrvFakeInput[3];
    }

    if (player[0] && (player[0] != lastplayer[playernum][0] || (RotationTimer() > nRotateTime[playernum]+0xf))) {
		RotateLeft(&nRotate[playernum]);
        //bprintf(PRINT_NORMAL, _T("Player %d Rotate Left => %06X\n"), playernum+1, nRotate[playernum]);
		nRotateTime[playernum] = RotationTimer();
		nRotateTarget[playernum] = -1;
    }

	if (player[1] && (player[1] != lastplayer[playernum][1] || (RotationTimer() > nRotateTime[playernum]+0xf))) {
        RotateRight(&nRotate[playernum]);
        //bprintf(PRINT_NORMAL, _T("Player %d Rotate Right => %06X\n"), playernum+1, nRotate[playernum]);
        nRotateTime[playernum] = RotationTimer();
		nRotateTarget[playernum] = -1;
	}

	lastplayer[playernum][0] = player[0];
	lastplayer[playernum][1] = player[1];

	return nRotate[playernum];
}

static UINT8 *rotate_gunpos[2] = {NULL, NULL};
static UINT8 rotate_gunpos_multiplier = 1;

// Gun-rotation memory locations - do not remove this tag. - dink :)
// game     p1           p2           clockwise value in memory  multiplier
// gondou   0x00007d     0x00007f     0 1 2 3 4 5 6 7            1

static void RotateSetGunPosRAM(UINT8 *p1, UINT8 *p2, UINT8 multiplier) {
	rotate_gunpos[0] = p1;
	rotate_gunpos[1] = p2;
	rotate_gunpos_multiplier = multiplier;
}

static INT32 get_distance(INT32 from, INT32 to) {
// this function finds the easiest way to get from "from" to "to", wrapping at 0 and 7
	INT32 countA = 0;
	INT32 countB = 0;
	INT32 fromtmp = from / rotate_gunpos_multiplier;
	INT32 totmp = to / rotate_gunpos_multiplier;

	while (1) {
		fromtmp++;
		countA++;
		if(fromtmp>7) fromtmp = 0;
		if(fromtmp == totmp || countA > 32) break;
	}

	fromtmp = from / rotate_gunpos_multiplier;
	totmp = to / rotate_gunpos_multiplier;

	while (1) {
		fromtmp--;
		countB++;
		if(fromtmp<0) fromtmp = 7;
		if(fromtmp == totmp || countB > 32) break;
	}

	if (countA > countB) {
		return 1; // go negative
	} else {
		return 0; // go positive
	}
}

static void RotateDoTick() {
	// since the game only allows for 1 rotation every other frame, we have to
	// do this.
	if (nCurrentFrame&1) return;

	for (INT32 i = 0; i < 2; i++) {
		if (rotate_gunpos[i] && (nRotateTarget[i] != -1) && (nRotateTarget[i] != (*rotate_gunpos[i] & 0xff))) {
			if (get_distance(nRotateTarget[i], *rotate_gunpos[i] & 0xff)) {
				RotateRight(&nRotate[i]); // --
			} else {
				RotateLeft(&nRotate[i]);  // ++
			}
			bprintf(0, _T("p%X target %X mempos %X nRotate %X.\n"), i, nRotateTarget[0], *rotate_gunpos[0] & 0xff, nRotate[0]);
			nRotateTry[i]++;
			if (nRotateTry[i] > 10) nRotateTarget[i] = -1; // don't get stuck in a loop if something goes horribly wrong here.
		} else {
			nRotateTarget[i] = -1;
		}
	}
}

static void SuperJoy2Rotate() {
	UINT8 FakeDrvInputPort0[4] = {0, 0, 0, 0};
	UINT8 FakeDrvInputPort1[4] = {0, 0, 0, 0};
	UINT8 NeedsSecondStick[2] = {0, 0};

	// prepare for right-stick rotation
	// this is not especially readable though
	for (INT32 i = 0; i < 2; i++) {
		for (INT32 n = 0; n < 4; n++) {
			UINT8* RotationInput = (!i) ? &FakeDrvInputPort0[0] : &FakeDrvInputPort1[0];
			RotationInput[n] = DrvFakeInput[6 + i*4 + n];
			NeedsSecondStick[i] |= RotationInput[n];
		}
	}

	for (INT32 i = 0; i < 2; i++) { // p1 = 0, p2 = 1
		if (!NeedsSecondStick[i])
			nAutoFireCounter[i] = 0;
		if (NeedsSecondStick[i]) { // or using Second Stick
			UINT8 rot = Joy2Rotate(((!i) ? &FakeDrvInputPort0[0] : &FakeDrvInputPort1[0]));
			if (rot != 0xff) {
				nRotateTarget[i] = rot * rotate_gunpos_multiplier;
			}
			nRotateTry[i] = 0;

			if (~DrvDips[2] & 1) {
				// fake auto-fire - there's probably a more elegant solution for this
				// P1: DrvJoy3 + 0, P2: DrvJoy3 + 2
				UINT8 indexmask = ((!i) ? 0x01 : 0x04); 
				if (nAutoFireCounter[i]++ & 0x4)
				{
					DrvInputs[2] &= ~indexmask; // remove the fire bit &= ~0x10; //
				}
				else
				{
					DrvInputs[2] |= indexmask; // turn on the fire bit
				}
			}
		}
		else if (DrvFakeInput[4 + i]) { //  rotate-button had been pressed
			UINT8 rot = Joy2Rotate(((!i) ? &DrvJoy1[0] : &DrvJoy2[0]));
			if (rot != 0xff) {
				nRotateTarget[i] = rot * rotate_gunpos_multiplier;
			}
			//DrvInput[i] &= ~0xf; // cancel out directionals since they are used to rotate here.
			DrvInputs[i] = (DrvInputs[i] & ~0xf) | (nRotateHoldInput[i] & 0xf); // for midnight resistance! be able to duck + change direction of gun.
			nRotateTry[i] = 0;
		} else { // cache joystick UDLR if the rotate button isn't pressed.
			// This feature is for Midnight Resistance, if you are crawling on the
			// ground and need to rotate your gun WITHOUT getting up.
			nRotateHoldInput[i] = DrvInputs[i];
		}
	}

	RotateDoTick();
}

// end Rotation-handler

static UINT8 gondo_in(INT32 plnum, INT32 offset)
{
	INT32 rotate[2];
	rotate[0] = 1 << dialRotation(0);
	rotate[1] = 1 << dialRotation(1);

	switch (offset) {
		case 0: return ~(rotate[plnum] & 0xff);
		case 1: return ((~rotate[plnum] >> 4) & 0xf0) | (DrvInputs[plnum] & 0x0f);
	}

	return 0xff;
}


static void gondo_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x3810:
			*soundlatch = data;
			M6502SetIRQLine(M6502_INPUT_LINE_NMI, CPU_IRQSTATUS_AUTO);
		return;

		case 0x3818:
			DrvPf0Ctrl[0x11] = data;
		return;

		case 0x3820:
			DrvPf0Ctrl[0x13] = data;
		return;

		case 0x3828:
			DrvPf0Ctrl[0x10] = (data >> 0) & 1;
			DrvPf0Ctrl[0x12] = (data >> 1) & 1;
		return;

		case 0x3830:
			bankswitch(data >> 4);
			*interrupt_enable = data & 1;
			if (~data & 1) {
				HD6309SetIRQLine(0, CPU_IRQSTATUS_NONE);
			}
			*nmi_enable	  = data & 2;
			*flipscreen	  = data & 8;
		return;

		case 0x3838: // gondo
		case 0x3839:
			i8751_write(address & 1, data);
		return;

		case 0x383a: // garyoret
		case 0x383b:
			i8751_write(address & 1, data);
		return;
	}
}

static UINT8 gondo_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x3800:
			return DrvDips[0];

		case 0x3801:
			return DrvDips[1];

		case 0x380a:
		case 0x380b:
			return gondo_in(0, address & 1);

		case 0x380c:
		case 0x380d:
			return gondo_in(1, address & 1);

		case 0x380e:
			return (DrvInputs[3] & 0x7f) | vblank;

		case 0x380f:
			return DrvInputs[2];

		case 0x3838:
			return i8751_hi();

		case 0x3839:
			return i8751_lo();
	}

	return 0;
}

static UINT8 garyoret_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x3800:
			return DrvDips[0];

		case 0x3801:
			return DrvDips[1];

		case 0x380a:
			return (DrvInputs[1] & 0x7f) | vblank;

		case 0x380b:
			return DrvInputs[0];

		case 0x383a:
			return i8751_hi();

		case 0x383b:
			return i8751_lo();
	}

	return 0;
}

static void gondo_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x0800:
		case 0x0801:
		case 0x2000:
		case 0x2001:
			BurnYM2203Write(0, address & 1, data);
		return;

		case 0x1000:
		case 0x1001:
		case 0x4000:
		case 0x4001:
			BurnYM3526Write(address & 1, data);
		return;
	}
}

static INT32 GondoDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	HD6309Open(0);
	HD6309Reset();
	HD6309Close();

	M6502Open(0);
	M6502Reset();
	M6502Close();

	BurnYM2203Reset();
	BurnYM3526Reset();

	DrvMCUReset();

	RotateReset();

	return 0;
}

static INT32 GondoGfxDecode()
{
	INT32 Plane0[3] = { 0x6000*8,0x4000*8,0x2000*8 };
	INT32 Plane1[4] = { 0x60000*8,0x40000*8,0x20000*8,0x00000*8 };
	INT32 XOffs0[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	INT32 XOffs1[16]= {16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),0,1,2,3,4,5,6,7 };
	INT32 YOffs0[16] = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x80000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x08000);

	GfxDecode(0x0400, 3,  8,  8, Plane0, XOffs0, YOffs0, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x80000);

	GfxDecode(0x1000, 4, 16, 16, Plane1, XOffs1, YOffs0, 0x100, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x80000);

	GfxDecode(0x1000, 4, 16, 16, Plane1, XOffs1, YOffs0, 0x100, tmp, DrvGfxROM2);

	BurnFree (tmp);

	return 0;
}

static INT32 GondoInit()
{
	BurnSetRefreshRate(58.00);

	BurnAllocMemIndex();

	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "garyoret")) {
		if (BurnLoadRom(DrvMainROM + 0x08000,  0, 1)) return 1;
		if (BurnLoadRom(DrvMainROM + 0x10000,  1, 1)) return 1;
		if (BurnLoadRom(DrvMainROM + 0x20000,  2, 1)) return 1;
		if (BurnLoadRom(DrvMainROM + 0x30000,  3, 1)) return 1;
		if (BurnLoadRom(DrvMainROM + 0x40000,  4, 1)) return 1;

		if (BurnLoadRom(DrvM6502ROM  + 0x08000,   5, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0   + 0x00000,   6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1   + 0x00000,   7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x10000,  8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x20000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x30000, 10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x40000, 11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x50000, 12, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x60000, 13, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x70000, 14, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3   + 0x00000, 15, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x00000, DrvGfxROM3 + 0x00000, 0x08000);
		memcpy (DrvGfxROM2 + 0x10000, DrvGfxROM3 + 0x08000, 0x08000);
		if (BurnLoadRom(DrvGfxROM3   + 0x00000, 16, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x08000, DrvGfxROM3 + 0x00000, 0x08000);
		memcpy (DrvGfxROM2 + 0x18000, DrvGfxROM3 + 0x08000, 0x08000);
		if (BurnLoadRom(DrvGfxROM3   + 0x00000, 17, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x20000, DrvGfxROM3 + 0x00000, 0x08000);
		memcpy (DrvGfxROM2 + 0x30000, DrvGfxROM3 + 0x08000, 0x08000);
		if (BurnLoadRom(DrvGfxROM3   + 0x00000, 18, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x28000, DrvGfxROM3 + 0x00000, 0x08000);
		memcpy (DrvGfxROM2 + 0x38000, DrvGfxROM3 + 0x08000, 0x08000);
		if (BurnLoadRom(DrvGfxROM3   + 0x00000, 19, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x40000, DrvGfxROM3 + 0x00000, 0x08000);
		memcpy (DrvGfxROM2 + 0x50000, DrvGfxROM3 + 0x08000, 0x08000);
		if (BurnLoadRom(DrvGfxROM3   + 0x00000, 20, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x48000, DrvGfxROM3 + 0x00000, 0x08000);
		memcpy (DrvGfxROM2 + 0x58000, DrvGfxROM3 + 0x08000, 0x08000);
		if (BurnLoadRom(DrvGfxROM3   + 0x00000, 21, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x60000, DrvGfxROM3 + 0x00000, 0x08000);
		memcpy (DrvGfxROM2 + 0x70000, DrvGfxROM3 + 0x08000, 0x08000);
		if (BurnLoadRom(DrvGfxROM3   + 0x00000, 22, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x68000, DrvGfxROM3 + 0x00000, 0x08000);
		memcpy (DrvGfxROM2 + 0x78000, DrvGfxROM3 + 0x08000, 0x08000);

		if (BurnLoadRom(DrvMCURom  + 0x00000, 23, 1)) return 1;
	} else {
		if (BurnLoadRom(DrvMainROM + 0x08000,  0, 1)) return 1;
		if (BurnLoadRom(DrvMainROM + 0x10000,  1, 1)) return 1;
		if (BurnLoadRom(DrvMainROM + 0x20000,  2, 1)) return 1;
		if (BurnLoadRom(DrvMainROM + 0x30000,  3, 1)) return 1;

		if (BurnLoadRom(DrvM6502ROM  + 0x08000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0   + 0x00000,  5, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1   + 0x00000,  6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x10000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x20000,  8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x30000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x40000, 10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x50000, 11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x60000, 12, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x70000, 13, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3   + 0x00000, 14, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x00000, DrvGfxROM3 + 0x00000, 0x08000);
		memcpy (DrvGfxROM2 + 0x10000, DrvGfxROM3 + 0x08000, 0x08000);
		if (BurnLoadRom(DrvGfxROM2   + 0x08000, 15, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3   + 0x00000, 16, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x20000, DrvGfxROM3 + 0x00000, 0x08000);
		memcpy (DrvGfxROM2 + 0x30000, DrvGfxROM3 + 0x08000, 0x08000);
		if (BurnLoadRom(DrvGfxROM2   + 0x28000, 17, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3   + 0x00000, 18, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x40000, DrvGfxROM3 + 0x00000, 0x08000);
		memcpy (DrvGfxROM2 + 0x50000, DrvGfxROM3 + 0x08000, 0x08000);
		if (BurnLoadRom(DrvGfxROM2   + 0x48000, 19, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3   + 0x00000, 20, 1)) return 1;
		memcpy (DrvGfxROM2 + 0x60000, DrvGfxROM3 + 0x00000, 0x08000);
		memcpy (DrvGfxROM2 + 0x70000, DrvGfxROM3 + 0x08000, 0x08000);
		if (BurnLoadRom(DrvGfxROM2   + 0x68000, 21, 1)) return 1;

		if (BurnLoadRom(DrvMCURom    + 0x00000, 22, 1)) return 1;
	}

	GondoGfxDecode();

	HD6309Init(0);
	HD6309Open(0);
	HD6309MapMemory(DrvMainRAM,		 0x0000, 0x17ff, MAP_RAM);
	HD6309MapMemory(DrvVidRAM,		 0x1800, 0x1fff, MAP_RAM);
	HD6309MapMemory(DrvPf0RAM,		 0x2000, 0x27ff, MAP_RAM);
	HD6309MapMemory(DrvPalRAM,		 0x2800, 0x2fff, MAP_RAM);//xxxxBBBBGGGGRRRR_split
	HD6309MapMemory(DrvSprRAM,		 0x3000, 0x37ff, MAP_RAM);
	HD6309MapMemory(DrvMainROM + 0x10000, 0x4000, 0x7fff, MAP_ROM); // bank
	HD6309MapMemory(DrvMainROM + 0x08000, 0x8000, 0xffff, MAP_ROM);
	HD6309SetWriteHandler(gondo_main_write);
	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "garyoret")) {
		HD6309SetReadHandler(garyoret_main_read);
	} else {
		HD6309SetReadHandler(gondo_main_read);

		RotateSetGunPosRAM(DrvMainRAM + 0x007d, DrvMainRAM + 0x007f, 1);
		game_rotates = 1;
	}
	HD6309Close();

	M6502Init(0, TYPE_M6502);
	M6502Open(0);
	M6502MapMemory(DrvM6502RAM,          0x0000, 0x05ff, MAP_RAM);
	M6502MapMemory(DrvM6502ROM + 0x8000, 0x8000, 0xffff, MAP_ROM);
	M6502SetReadHandler(ghostb_sound_read);
	M6502SetWriteHandler(gondo_sound_write);
	M6502Close();

	if (DrvMCURom[0] != 0x00) {
		// "gondo" parent missing mcu dump (as of may 2020)
		DrvMCUInit(0);
	}

	BurnYM3526Init(3000000, &DrvYM3812FMIRQHandler, 0);
	BurnTimerAttach(&M6502Config, 1500000);
	BurnYM3526SetRoute(BURN_SND_YM3526_ROUTE, 0.70, BURN_SND_ROUTE_BOTH);

	BurnYM2203Init(1, 1500000, NULL, 1);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_YM2203_ROUTE, 0.20, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_1, 0.23, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_2, 0.23, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_3, 0.23, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	GondoDoReset();

	return 0;
}

static INT32 GondoExit()
{
	HD6309Exit();
	BurnYM3526Exit();

	game_rotates = 0;

	return DrvExit();
}

static void gondo_draw_txt_layer()
{
	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		sy -= 8;
		if (sy < 0 || sy >= nScreenWidth) continue;

		INT32 code = DrvVidRAM[offs * 2 + 1] | (DrvVidRAM[offs * 2 + 0] << 8);
		INT32 color = (code >> 12) & 7;

		Render8x8Tile_Mask_Clip(pTransDraw, code & 0xfff, sx, sy, color, 3, 0, 0, DrvGfxROM0);
	}
}

static void gondo_draw_layer(INT32 priority, INT32 layer)
{
	INT32 scrolly = ((DrvPf0Ctrl[0x12] << 8) + DrvPf0Ctrl[0x13] + 8) & 0x1ff;
	INT32 scrollx = ((DrvPf0Ctrl[0x10] << 8) | DrvPf0Ctrl[0x11]);

	UINT16 trans = layer ? 0x00ff : 0xff00;

	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 4;
		INT32 sy = (offs >> 5) << 4;

		sx -= scrollx;
		if (sx < -15) sx+=512;
		sy -= scrolly;
		if (sy < -15) sy+=512;

		if (sx >= nScreenWidth || sy >= nScreenWidth) continue;

		INT32 code = DrvPf0RAM[offs * 2 + 1] | (DrvPf0RAM[offs * 2 + 0] << 8);
		INT32 color = code >> 12;

		if (priority && color < 8) continue;

		{
			code &= 0xfff;
			color <<= 4;
			color |= 0x300;

			UINT8 *src = DrvGfxROM2 + (code << 8);
			UINT16 *dst;

			for (INT32 y = 0; y < 16; y++, src+=16, sy++)
			{
				if (sy < 0 || sy >= nScreenHeight) continue;

				dst = pTransDraw + sy * nScreenWidth;

				INT32 xx = sx;
				for (INT32 x = 0; x < 16; x++, xx++)
				{
					if (xx < 0 || xx >= nScreenWidth) continue;

					INT32 pxl = src[x];

					if (trans & (1 << pxl)) continue;

					dst[xx] = pxl | color;
				}
			}
		}
	}
}

static void GondoPaletteRecalc()
{
	UINT8 r,g,b;
	for (INT32 i = 0; i < 0x400; i++) {
		INT32 d = (DrvPalRAM[i + 0]) | (DrvPalRAM[i + 0x400] << 8);
		r = (d >> 0) & 0x0f;
		g = (d >> 4) & 0x0f;
		b = (d >> 8) & 0x0f;

		r |= r << 4;
		g |= g << 4;
		b |= b << 4;

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static INT32 GondoDraw()
{
	if (DrvRecalc) {
		GondoPaletteRecalc();
	}
	BurnTransferClear();

	if (nBurnLayer & 1) gondo_draw_layer(0, 0);
	if (nSpriteEnable & 1) draw_sprites1(2);
	if (nBurnLayer & 2) gondo_draw_layer(0, 1);
	if (nSpriteEnable & 2) draw_sprites1(1);
	if (nBurnLayer & 4) gondo_draw_txt_layer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 GaryoretDraw()
{
	if (DrvRecalc) {
		GondoPaletteRecalc();
	}

	gondo_draw_layer(0, 1);
	gondo_draw_layer(0, 0);
	draw_sprites1(0);
	gondo_draw_layer(1, 1);
	gondo_draw_layer(1, 0);

	gondo_draw_txt_layer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

#if 0
static inline void DrvClearOppositesLOW(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x00) {
		*nJoystickInputs |= 0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x00) {
		*nJoystickInputs |= 0x0c;
	}
}
#endif

static inline void DrvClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}


static INT32 GondoFrame()
{
	if (DrvReset) {
		GondoDoReset();
	}

	HD6309NewFrame();
	M6502NewFrame();
	if (realMCU) mcs51NewFrame();

	{
		memset (&DrvInputs[0], 0x00, 2); // p1,p2 udlr active high for the rotation handler, converted to active low after rotation handler.
		memset (&DrvInputs[2], 0xff, 3);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
			DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
		}

		DrvClearOpposites(&DrvInputs[0]);
		DrvClearOpposites(&DrvInputs[1]);

		if (game_rotates) {
			SuperJoy2Rotate();
		}
		DrvInputs[0] ^= 0xff; // convert to active low
		DrvInputs[1] ^= 0xff;
	}

	INT32 nInterleave = 272;
	INT32 nCyclesTotal[3] = { 12000000 / 58, 1500000 / 58, 12000000 / 12 / 58 };
	INT32 nCyclesDone[2] = { 0, 0 };

	M6502Open(0);
	HD6309Open(0);

	for (INT32 i = 0; i < nInterleave; i++)
	{
		if (i == 7) {
			vblank = 0;
		}

		if (i == 270) {
			if (pBurnDraw) {
				BurnDrvRedraw();
			}

			memcpy (DrvSprBuf, DrvSprRAM, 0x800);

			vblank = 0x80;
			if (*nmi_enable) HD6309SetIRQLine(0x20, CPU_IRQSTATUS_AUTO);
		}

		CPU_RUN(0, HD6309);

		if (realMCU) {
			DrvMCUSync();
		}

		CPU_RUN_TIMER(1);
	}

	HD6309Close();
	M6502Close();

	if (pBurnSoundOut) {
		BurnYM3526Update(pBurnSoundOut, nBurnSoundLen);
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
	}

	return 0;
}

static INT32 GondoScan(INT32 nAction, INT32 *pnMin)
{
	if (nAction & ACB_DRIVER_DATA) {
		HD6309Scan(nAction);
		BurnYM3526Scan(nAction, pnMin);
		SCAN_VAR(RomBank);

		if (game_rotates) {
			SCAN_VAR(nRotate);
			SCAN_VAR(nRotateTarget);
			SCAN_VAR(nRotateTry);
			SCAN_VAR(nRotateHoldInput);
			SCAN_VAR(nAutoFireCounter);
			SCAN_VAR(nRotateTime);
		}

		if (nAction & ACB_WRITE) {
			HD6309Open(0);
			bankswitch(RomBank);
			HD6309Close();
		}
	}

	return DrvScan(nAction, pnMin);
}


// Gondomania (World)

static struct BurnRomInfo gondoRomDesc[] = {
	{ "dt00-e.f3",		0x08000, 0x912a7eee, 1 }, //  0 maincpu
	{ "dt01.f5",		0x10000, 0xc39bb877, 1 }, //  1
	{ "dt02.f6",		0x10000, 0x925307a4, 1 }, //  2
	{ "dt03-e.f7",		0x10000, 0xee7475eb, 1 }, //  3

	{ "dt05-e.h5",		0x08000, 0xec08aa29, 2 }, //  4 audiocpu

	{ "dt14-e.b18",		0x08000, 0x00cbe9c8, 3 }, //  5 gfx1

	{ "dt19.f13",		0x10000, 0xda2abe4b, 4 }, //  6 gfx2
	{ "dt20-e.f15",		0x08000, 0x0eef7f56, 4 }, //  7
	{ "dt16.f9",		0x10000, 0xe9955d8f, 4 }, //  8
	{ "dt18-e.f12",		0x08000, 0x2b2d1468, 4 }, //  9
	{ "dt15.f8",		0x10000, 0xa54b2eb6, 4 }, // 10
	{ "dt17-e.f11",		0x08000, 0x75ae349a, 4 }, // 11
	{ "dt21.f16",		0x10000, 0x1c5f682d, 4 }, // 12
	{ "dt22-e.f18",		0x08000, 0xc8ffb148, 4 }, // 13

	{ "dt08.h10",		0x10000, 0xaec483f5, 5 }, // 14 gfx3
	{ "dt09.h12",		0x08000, 0x446f0ce0, 5 }, // 15
	{ "dt06.h7",		0x10000, 0x3fe1527f, 5 }, // 16
	{ "dt07.h9",		0x08000, 0x61f9bce5, 5 }, // 17
	{ "dt12.h16",		0x10000, 0x1a72ca8d, 5 }, // 18
	{ "dt13.h18",		0x08000, 0xccb81aec, 5 }, // 19
	{ "dt10.h13",		0x10000, 0xcfcfc9ed, 5 }, // 20
	{ "dt11.h15",		0x08000, 0x53e9cf17, 5 }, // 21

	{ "dt-e.b1",     	0x01000, 0x0d0532ec, 6 }, // 22 i8751 microcontroller

	{ "ds-23.b10",		0x00400, 0xdcbfec4e, 7 }, // 23 proms
};

STD_ROM_PICK(gondo)
STD_ROM_FN(gondo)

struct BurnDriver BurnDrvGondo = {
	"gondo", NULL, NULL, NULL, "1987",
	"Gondomania (World)\0", NULL, "Data East Corporation", "DEC8",
	L"\u9B54\u5883\u6218\u58EB - \u7EDD\u5BF9\u5408\u4F53 (\u4E16\u754C\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_VERSHOOT, 0,
	NULL, gondoRomInfo, gondoRomName, NULL, NULL, NULL, NULL, GondoInputInfo, GondoDIPInfo,
	GondoInit, GondoExit, GondoFrame, GondoDraw, GondoScan, &DrvRecalc, 0x400,
	240, 256, 3, 4
};


// Gondomania (US)

static struct BurnRomInfo gondouRomDesc[] = {
	{ "dt00.f3",		0x08000, 0xa8cf9118, 1 }, //  0 maincpu
	{ "dt01.f5",		0x10000, 0xc39bb877, 1 }, //  1
	{ "dt02.f6",		0x10000, 0xbb5e674b, 1 }, //  2
	{ "dt03.f7",		0x10000, 0x99c32b13, 1 }, //  3

	{ "dt05.h5",		0x08000, 0xec08aa29, 2 }, //  4 audiocpu

	{ "dt14.b18",		0x08000, 0x4bef16e1, 3 }, //  5 gfx1

	{ "dt19.f13",		0x10000, 0xda2abe4b, 4 }, //  6 gfx2
	{ "dt20.f15",		0x08000, 0x42d01002, 4 }, //  7
	{ "dt16.f9",		0x10000, 0xe9955d8f, 4 }, //  8
	{ "dt18.f12",		0x08000, 0xc0c5df1c, 4 }, //  9
	{ "dt15.f8",		0x10000, 0xa54b2eb6, 4 }, // 10
	{ "dt17.f11",		0x08000, 0x3bbcff0d, 4 }, // 11
	{ "dt21.f16",		0x10000, 0x1c5f682d, 4 }, // 12
	{ "dt22.f18",		0x08000, 0xc1876a5f, 4 }, // 13

	{ "dt08.h10",		0x10000, 0xaec483f5, 5 }, // 14 gfx3
	{ "dt09.h12",		0x08000, 0x446f0ce0, 5 }, // 15
	{ "dt06.h7",		0x10000, 0x3fe1527f, 5 }, // 16
	{ "dt07.h9",		0x08000, 0x61f9bce5, 5 }, // 17
	{ "dt12.h16",		0x10000, 0x1a72ca8d, 5 }, // 18
	{ "dt13.h18",		0x08000, 0xccb81aec, 5 }, // 19
	{ "dt10.h13",		0x10000, 0xcfcfc9ed, 5 }, // 20
	{ "dt11.h15",		0x08000, 0x53e9cf17, 5 }, // 21

	{ "dt-a.b1",     	0x01000, 0x03abceeb, 6 }, // 22 i8751 microcontroller

	{ "ds-23.b10",		0x00400, 0xdcbfec4e, 7 }, // 23 proms
};

STD_ROM_PICK(gondou)
STD_ROM_FN(gondou)

struct BurnDriver BurnDrvGondou = {
	"gondou", "gondo", NULL, NULL, "1987",
	"Gondomania (US)\0", NULL, "Data East USA", "DEC8",
	L"\u9B54\u5883\u6218\u58EB - \u7EDD\u5BF9\u5408\u4F53 (\u7F8E\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_VERSHOOT, 0,
	NULL, gondouRomInfo, gondouRomName, NULL, NULL, NULL, NULL, GondoInputInfo, GondoDIPInfo,
	GondoInit, GondoExit, GondoFrame, GondoDraw, GondoScan, &DrvRecalc, 0x400,
	240, 256, 3, 4
};


// Makyou Senshi (Japan)

static struct BurnRomInfo makyosenRomDesc[] = {
	{ "ds00.f3",		0x08000, 0x33bb16fe, 1 }, //  0 maincpu
	{ "ds01.f5",		0x10000, 0xc39bb877, 1 }, //  1
	{ "ds02.f6",		0x10000, 0x925307a4, 1 }, //  2
	{ "ds03.f7",		0x10000, 0x9c0fcbf6, 1 }, //  3

	{ "ds05.h5",		0x08000, 0xe6e28ca9, 2 }, //  4 audiocpu

	{ "ds14.b18",		0x08000, 0x00cbe9c8, 3 }, //  5 gfx1

	{ "ds19.f13",		0x10000, 0xda2abe4b, 4 }, //  6 gfx2
	{ "ds20.f15",		0x08000, 0x0eef7f56, 4 }, //  7
	{ "ds16.f9",		0x10000, 0xe9955d8f, 4 }, //  8
	{ "ds18.f12",		0x08000, 0x2b2d1468, 4 }, //  9
	{ "ds15.f8",		0x10000, 0xa54b2eb6, 4 }, // 10
	{ "ds17.f11",		0x08000, 0x75ae349a, 4 }, // 11
	{ "ds21.f16",		0x10000, 0x1c5f682d, 4 }, // 12
	{ "ds22.f18",		0x08000, 0xc8ffb148, 4 }, // 13

	{ "ds08.h10",		0x10000, 0xaec483f5, 5 }, // 14 gfx3
	{ "ds09.h12",		0x08000, 0x446f0ce0, 5 }, // 15
	{ "ds06.h7",		0x10000, 0x3fe1527f, 5 }, // 16
	{ "ds07.h9",		0x08000, 0x61f9bce5, 5 }, // 17
	{ "ds12.h16",		0x10000, 0x1a72ca8d, 5 }, // 18
	{ "ds13.h18",		0x08000, 0xccb81aec, 5 }, // 19
	{ "ds10.h13",		0x10000, 0xcfcfc9ed, 5 }, // 20
	{ "ds11.h15",		0x08000, 0x53e9cf17, 5 }, // 21

	{ "ds.b1",     		0x01000, 0x08f36e35, 6 }, // 22 i8751 microcontroller

	{ "ds-23.b10",		0x00400, 0xdcbfec4e, 7 }, // 23 proms

	{ "pal16r4nc.u10",	0x00104, 0x00000000, 8 | BRF_NODUMP }, // 24 plds
	{ "pal16r4nc.g11",	0x00104, 0x00000000, 8 | BRF_NODUMP }, // 25
	{ "pal16r4nc.s1",	0x00104, 0x00000000, 8 | BRF_NODUMP }, // 26
};

STD_ROM_PICK(makyosen)
STD_ROM_FN(makyosen)

struct BurnDriver BurnDrvMakyosen = {
	"makyosen", "gondo", NULL, NULL, "1987",
	"Makyou Senshi (Japan)\0", NULL, "Data East Corporation", "DEC8",
	L"\u9B54\u5883\u6218\u58EB - \u7EDD\u5BF9\u5408\u4F53 (\u65E5\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_VERSHOOT, 0,
	NULL, makyosenRomInfo, makyosenRomName, NULL, NULL, NULL, NULL, GondoInputInfo, GondoDIPInfo,
	GondoInit, GondoExit, GondoFrame, GondoDraw, GondoScan, &DrvRecalc, 0x400,
	240, 256, 3, 4
};


// Garyo Retsuden (Japan)

static struct BurnRomInfo garyoretRomDesc[] = {
	{ "dv00",		0x08000, 0xcceaaf05, 1 }, //  0 maincpu
	{ "dv01",		0x10000, 0xc33fc18a, 1 }, //  1
	{ "dv02",		0x10000, 0xf9e26ce7, 1 }, //  2
	{ "dv03",		0x10000, 0x55d8d699, 1 }, //  3
	{ "dv04",		0x10000, 0xed3d00ee, 1 }, //  4

	{ "dv05",		0x08000, 0xc97c347f, 2 }, //  5 audiocpu

	{ "dv14",		0x08000, 0xfb2bc581, 3 }, //  6 gfx1

	{ "dv22",		0x10000, 0xcef0367e, 4 }, //  7 gfx2
	{ "dv21",		0x08000, 0x90042fb7, 4 }, //  8
	{ "dv20",		0x10000, 0x451a2d8c, 4 }, //  9
	{ "dv19",		0x08000, 0x14e1475b, 4 }, // 10
	{ "dv18",		0x10000, 0x7043bead, 4 }, // 11
	{ "dv17",		0x08000, 0x28f449d7, 4 }, // 12
	{ "dv16",		0x10000, 0x37e4971e, 4 }, // 13
	{ "dv15",		0x08000, 0xca41b6ac, 4 }, // 14

	{ "dv08",		0x10000, 0x89c13e15, 5 }, // 15 gfx3
	{ "dv09",		0x10000, 0x6a345a23, 5 }, // 16
	{ "dv06",		0x10000, 0x1eb52a20, 5 }, // 17
	{ "dv07",		0x10000, 0xe7346ef8, 5 }, // 18
	{ "dv12",		0x10000, 0x46ba5af4, 5 }, // 19
	{ "dv13",		0x10000, 0xa7af6dfd, 5 }, // 20
	{ "dv10",		0x10000, 0x68b6d75c, 5 }, // 21
	{ "dv11",		0x10000, 0xb5948aee, 5 }, // 22

	{ "dv__.mcu",	0x01000, 0x37cacec6, 6 | BRF_ESS }, // 23 mcu
};

STD_ROM_PICK(garyoret)
STD_ROM_FN(garyoret)

struct BurnDriver BurnDrvGaryoret = {
	"garyoret", NULL, NULL, NULL, "1987",
	"Garyo Retsuden (Japan)\0", NULL, "Data East Corporation", "DEC8",
	L"\u5367\u9F99\u5217\u4F20 (\u65E5\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_RUNGUN, 0,
	NULL, garyoretRomInfo, garyoretRomName, NULL, NULL, NULL, NULL, GaryoretInputInfo, GaryoretDIPInfo,
	GondoInit, GondoExit, GondoFrame, GaryoretDraw, GondoScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};

static UINT8 oscar_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x3c00: // in0
			return DrvInputs[0];

		case 0x3c01:
			return DrvInputs[1];

		case 0x3c02:
			return (DrvInputs[2] & 0x7f) | vblank;

		case 0x3c03:
			return DrvDips[0];

		case 0x3c04:
			return DrvDips[1];
	}

	return 0;
}

static void oscar_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x3c80:
			memcpy (DrvSprBuf, DrvSprRAM, 0x800);
		return;

		case 0x3d00:
			bankswitch(data);
		return;

		case 0x3d80:
			*soundlatch = data;
			M6502SetIRQLine(M6502_INPUT_LINE_NMI, CPU_IRQSTATUS_AUTO);
		return;

		case 0x3e80:
			HD6309SetIRQLine(1, 0, CPU_IRQSTATUS_ACK);
		return;

		case 0x3e81:
			HD6309SetIRQLine(0, CPU_IRQSTATUS_NONE);
		return;

		case 0x3e82:
			HD6309SetIRQLine(0, CPU_IRQSTATUS_ACK);
		return;

		case 0x3e83:
			HD6309SetIRQLine(1, 0, CPU_IRQSTATUS_NONE);
		return;
	}

	if ((address & 0xffe0) == 0x3c00) {
		DrvPf0Ctrl[address & 0x1f] = data;
		return;
	}
}

static void oscar_sub_write(UINT16 address, UINT8 )
{
	switch (address)
	{
		case 0x3e80:
			HD6309SetIRQLine(0, CPU_IRQSTATUS_ACK);
		return;

		case 0x3e81:
			HD6309SetIRQLine(0, 0, CPU_IRQSTATUS_NONE);
		return;

		case 0x3e82:
			HD6309SetIRQLine(0, 0, CPU_IRQSTATUS_ACK);
		return;

		case 0x3e83:
			HD6309SetIRQLine(0, CPU_IRQSTATUS_NONE);
		return;
	}
}

static INT32 OscarDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	HD6309Open(0);
	HD6309Reset();
	HD6309Close();

	HD6309Open(1);
	HD6309Reset();
	HD6309Close();

	M6502Open(0);
	M6502Reset();
	M6502Close();

	BurnYM2203Reset();
	BurnYM3526Reset();

	DrvMCUReset();

	return 0;
}

static INT32 OscarGfxDecode()
{
	INT32 Plane0[3] = { 0x3000*8,0x2000*8,0x1000*8 };
	INT32 Plane1[4] = { 0x60000*8,0x40000*8,0x20000*8,0x00000*8 };
	INT32 XOffs0[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	INT32 XOffs1[16]= {16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),0,1,2,3,4,5,6,7 };
	INT32 YOffs0[16] = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x80000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x08000);

	GfxDecode(0x0400, 3,  8,  8, Plane0, XOffs0, YOffs0, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x80000);

	GfxDecode(0x1000, 4, 16, 16, Plane1, XOffs1, YOffs0, 0x100, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x80000);

	GfxDecode(0x1000, 4, 16, 16, Plane1, XOffs1, YOffs0, 0x100, tmp, DrvGfxROM2);

	BurnFree (tmp);

	return 0;
}

static INT32 OscarInit()
{
	BurnSetRefreshRate(57.44);

	BurnAllocMemIndex();

	{
		if (BurnLoadRom(DrvMainROM   + 0x08000,  0, 1)) return 1;
		if (BurnLoadRom(DrvMainROM   + 0x10000,  1, 1)) return 1;

		if (BurnLoadRom(DrvSubROM    + 0x00000,   2, 1)) return 1;

		if (BurnLoadRom(DrvM6502ROM  + 0x08000,   3, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0   + 0x00000,   4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1   + 0x00000,   5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x20000,   6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x40000,   7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x60000,   8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2   + 0x00000,   9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2   + 0x20000,  10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2   + 0x40000,  11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2   + 0x60000,  12, 1)) return 1;

		OscarGfxDecode();
	}

	HD6309Init(0);
	HD6309Open(0);
	HD6309MapMemory(DrvMainRAM,		 0x0000, 0x1fff, MAP_RAM); // all shared?
	HD6309MapMemory(DrvVidRAM,		 0x2000, 0x28ff, MAP_RAM);
	HD6309MapMemory(DrvPf0RAM,		 0x2800, 0x2fff, MAP_RAM);
	HD6309MapMemory(DrvSprRAM,		 0x3000, 0x37ff, MAP_RAM);
	HD6309MapMemory(DrvPalRAM,		 0x3800, 0x3bff, MAP_RAM); // xxxxBBBBGGGGRRRR_be_w
	HD6309MapMemory(DrvMainROM + 0x10000, 0x4000, 0x7fff, MAP_ROM); // bank
	HD6309MapMemory(DrvMainROM + 0x08000, 0x8000, 0xffff, MAP_ROM);
	HD6309SetWriteHandler(oscar_main_write);
	HD6309SetReadHandler(oscar_main_read);
	HD6309Close();

	HD6309Init(1);
	HD6309Open(1);
	HD6309MapMemory(DrvMainRAM,			0x0000, 0x0eff, MAP_RAM); // all shared? AM_RANGE(0x0f00, 0x0fff) AM_RAM not?
	HD6309MapMemory(DrvPalRAM + 0x400,	0x0f00, 0x0fff, MAP_RAM); // not really pal...
	HD6309MapMemory(DrvMainRAM + 0x1000, 0x1000, 0x1fff, MAP_RAM); // all shared? AM_RANGE(0x0f00, 0x0fff) AM_RAM not?
	HD6309MapMemory(DrvSubROM + 0x04000, 0x4000, 0xffff, MAP_ROM);
	HD6309SetWriteHandler(oscar_sub_write); // 0x3e80, 0x3e83 used...
	HD6309Close();

	M6502Init(0, TYPE_DECO222);
	M6502Open(0);
	M6502MapMemory(DrvM6502RAM,          0x0000, 0x05ff, MAP_RAM);
	M6502MapMemory(DrvM6502ROM + 0x8000, 0x8000, 0xffff, MAP_ROM);
	M6502SetReadHandler(ghostb_sound_read);
	M6502SetWriteHandler(gondo_sound_write);
	M6502Close();

	BurnYM3526Init(3000000, &DrvYM3812FMIRQHandler, 0);
	BurnTimerAttach(&M6502Config, 1500000);
	BurnYM3526SetRoute(BURN_SND_YM3526_ROUTE, 0.50, BURN_SND_ROUTE_BOTH);

	BurnYM2203Init(1, 1500000, NULL, 1);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_YM2203_ROUTE, 0.23, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_1, 0.10, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_2, 0.10, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_3, 0.10, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	OscarDoReset();

	return 0;
}

static void oscar_draw_txt_layer()
{
	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		sy -= 8;
		if (sy < 0 || sy >= nScreenHeight) continue;

		INT32 code = (DrvVidRAM[offs * 2 + 1]) | (DrvVidRAM[offs * 2 + 0] << 8);
		INT32 color = code >> 14;

		Render8x8Tile_Mask(pTransDraw, code & 0x3ff, sx, sy, color, 3, 0, 0x100, DrvGfxROM0);
	}
}

static INT32 OscarDraw()
{
	if (DrvRecalc) {
		UINT8 r,g,b;
		for (INT32 i = 0; i < 0x400; i+=2) {
			INT32 d = (DrvPalRAM[i + 0] << 8) | DrvPalRAM[i + 1];
			r = (d >> 0) & 0x0f;
			g = (d >> 4) & 0x0f;
			b = (d >> 8) & 0x0f;

			r |= r << 4;
			g |= g << 4;
			b |= b << 4;

			DrvPalette[i/2] = BurnHighCol(r, g, b, 0);
		}
	}

	BurnTransferClear();

	if (nBurnLayer & 1) draw_bg_layer(DrvPf0RAM, DrvPf0Ctrl, DrvGfxROM2, 0x180, 0x07, 0x7ff, 0x0000, 0);
	if (nBurnLayer & 2) draw_bg_layer(DrvPf0RAM, DrvPf0Ctrl, DrvGfxROM2, 0x180, 0x07, 0x7ff, 0x0000, 8);
	if (nBurnLayer & 4) draw_bg_layer(DrvPf0RAM, DrvPf0Ctrl, DrvGfxROM2, 0x180, 0x07, 0x7ff, 0x00ff, 0);
	if (nSpriteEnable & 1) draw_sprites2(0xf, 0, 0);
	if (nBurnLayer & 8) draw_bg_layer(DrvPf0RAM, DrvPf0Ctrl, DrvGfxROM2, 0x180, 0x07, 0x7ff, 0x00ff, 8);
	if (nSpriteEnable & 2) oscar_draw_txt_layer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 OscarFrame()
{
	if (DrvReset) {
		OscarDoReset();
	}

	HD6309NewFrame();
	M6502NewFrame();

	{
		UINT8 old_coins = DrvInputs[2] & 0x7;

		memset (DrvInputs, 0xff, 5);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
			DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
		}
		if (old_coins == 0x7 && (DrvInputs[2] & 0x7) != 0x7) {
			HD6309SetIRQLine(0, 0x20, CPU_IRQSTATUS_AUTO);
		}
	}

	INT32 nInterleave = 256;
	INT32 nCyclesTotal[3] = { (INT32)(6000000 / 57.44), (INT32)(6000000 / 57.44), (INT32)(1500000 / 57.44) };
	INT32 nCyclesDone[3] = { 0, 0, 0 };

	M6502Open(0);

	vblank = 0x80;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		if (i == 8) vblank = 0;
		if (i == 247) vblank = 0x80;

		HD6309Open(0);
		CPU_RUN(0, HD6309);
		HD6309Close();

		HD6309Open(1);
		CPU_RUN(1, HD6309);
		HD6309Close();

		CPU_RUN_TIMER(2);
	}

	M6502Close();

	if (pBurnSoundOut) {
		BurnYM3526Update(pBurnSoundOut, nBurnSoundLen);
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		OscarDraw();
	}

	return 0;
}


// Psycho-Nics Oscar (World revision 0)

static struct BurnRomInfo oscarRomDesc[] = {
	{ "du10",		0x08000, 0x120040d8, 1 }, //  0 maincpu
	{ "ed09",		0x10000, 0xe2d4bba9, 1 }, //  1

	{ "du11",		0x10000, 0xff45c440, 2 }, //  2 sub

	{ "ed12",		0x08000, 0x432031c5, 3 }, //  3 audiocpu

	{ "ed08",		0x04000, 0x308ac264, 4 }, //  4 gfx1

	{ "ed04",		0x10000, 0x416a791b, 5 }, //  5 gfx2
	{ "ed05",		0x10000, 0xfcdba431, 5 }, //  6
	{ "ed06",		0x10000, 0x7d50bebc, 5 }, //  7
	{ "ed07",		0x10000, 0x8fdf0fa5, 5 }, //  8

	{ "ed01",		0x10000, 0xd3a58e9e, 6 }, //  9 gfx3
	{ "ed03",		0x10000, 0x4fc4fb0f, 6 }, // 10
	{ "ed00",		0x10000, 0xac201f2d, 6 }, // 11
	{ "ed02",		0x10000, 0x7ddc5651, 6 }, // 12
};

STD_ROM_PICK(oscar)
STD_ROM_FN(oscar)

struct BurnDriver BurnDrvOscar = {
	"oscar", NULL, NULL, NULL, "1987",
	"Psycho-Nics Oscar (World revision 0)\0", NULL, "Data East Corporation", "DEC8",
	L"\u94A2\u94C1\u91D1\u521A (\u4E16\u754C\u7248 \u4FEE\u8BA2\u7248 0)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_RUNGUN, 0,
	NULL, oscarRomInfo, oscarRomName, NULL, NULL, NULL, NULL, OscarInputInfo, OscarDIPInfo,
	OscarInit, GondoExit, OscarFrame, OscarDraw, GondoScan, &DrvRecalc, 0x200,
	256, 240, 4, 3
};


// Psycho-Nics Oscar (US)

static struct BurnRomInfo oscaruRomDesc[] = {
	{ "ed10",		0x08000, 0xf9b0d4d4, 1 }, //  0 maincpu
	{ "ed09",		0x10000, 0xe2d4bba9, 1 }, //  1

	{ "ed11",		0x10000, 0x10e5d919, 2 }, //  2 sub

	{ "ed12",		0x08000, 0x432031c5, 3 }, //  3 audiocpu

	{ "ed08",		0x04000, 0x308ac264, 4 }, //  4 gfx1

	{ "ed04",		0x10000, 0x416a791b, 5 }, //  5 gfx2
	{ "ed05",		0x10000, 0xfcdba431, 5 }, //  6
	{ "ed06",		0x10000, 0x7d50bebc, 5 }, //  7
	{ "ed07",		0x10000, 0x8fdf0fa5, 5 }, //  8

	{ "ed01",		0x10000, 0xd3a58e9e, 6 }, //  9 gfx3
	{ "ed03",		0x10000, 0x4fc4fb0f, 6 }, // 10
	{ "ed00",		0x10000, 0xac201f2d, 6 }, // 11
	{ "ed02",		0x10000, 0x7ddc5651, 6 }, // 12

	{ "du-13.bin",		0x00200, 0xbea1f87e, 8 }, // 14 proms
};

STD_ROM_PICK(oscaru)
STD_ROM_FN(oscaru)

struct BurnDriver BurnDrvOscaru = {
	"oscaru", "oscar", NULL, NULL, "1988",
	"Psycho-Nics Oscar (US)\0", NULL, "Data East USA", "DEC8",
	L"\u94A2\u94C1\u91D1\u521A (\u7F8E\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_RUNGUN, 0,
	NULL, oscaruRomInfo, oscaruRomName, NULL, NULL, NULL, NULL, OscarInputInfo, OscarDIPInfo,
	OscarInit, GondoExit, OscarFrame, OscarDraw, GondoScan, &DrvRecalc, 0x200,
	256, 240, 4, 3
};


// Psycho-Nics Oscar (Japan revision 1)

static struct BurnRomInfo oscarj1RomDesc[] = {
	{ "du10-1",		0x08000, 0x4ebc9f7a, 1 }, //  0 maincpu
	{ "ed09",		0x10000, 0xe2d4bba9, 1 }, //  1

	{ "du11",		0x10000, 0xff45c440, 2 }, //  2 sub

	{ "ed12",		0x08000, 0x432031c5, 3 }, //  3 audiocpu

	{ "ed08",		0x04000, 0x308ac264, 4 }, //  4 gfx1

	{ "ed04",		0x10000, 0x416a791b, 5 }, //  5 gfx2
	{ "ed05",		0x10000, 0xfcdba431, 5 }, //  6
	{ "ed06",		0x10000, 0x7d50bebc, 5 }, //  7
	{ "ed07",		0x10000, 0x8fdf0fa5, 5 }, //  8

	{ "ed01",		0x10000, 0xd3a58e9e, 6 }, //  9 gfx3
	{ "ed03",		0x10000, 0x4fc4fb0f, 6 }, // 10
	{ "ed00",		0x10000, 0xac201f2d, 6 }, // 11
	{ "ed02",		0x10000, 0x7ddc5651, 6 }, // 12

	{ "du-13.bin",		0x00200, 0xbea1f87e, 8 }, // 14 proms
};

STD_ROM_PICK(oscarj1)
STD_ROM_FN(oscarj1)

struct BurnDriver BurnDrvOscarj1 = {
	"oscarj1", "oscar", NULL, NULL, "1987",
	"Psycho-Nics Oscar (Japan revision 1)\0", NULL, "Data East Corporation", "DEC8",
	L"\u94A2\u94C1\u91D1\u521A (\u65E5\u7248 \u4FEE\u8BA2\u7248 1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_RUNGUN, 0,
	NULL, oscarj1RomInfo, oscarj1RomName, NULL, NULL, NULL, NULL, OscarInputInfo, OscarjDIPInfo,
	OscarInit, GondoExit, OscarFrame, OscarDraw, GondoScan, &DrvRecalc, 0x200,
	256, 240, 4, 3
};


// Psycho-Nics Oscar (Japan revision 2)

static struct BurnRomInfo oscarj2RomDesc[] = {
	{ "du10-2.h12",		0x08000, 0x114e898d, 1 }, //  0 maincpu
	{ "du09.h10",		0x10000, 0xe2d4bba9, 1 }, //  1

	{ "du11.h16",		0x10000, 0xff45c440, 2 }, //  2 sub

	{ "du12.k5",		0x08000, 0x432031c5, 3 }, //  3 audiocpu

	{ "du08.e5",		0x04000, 0x308ac264, 4 }, //  4 gfx1

	{ "du04.a11",		0x10000, 0x416a791b, 5 }, //  5 gfx2
	{ "du05.a14",		0x10000, 0xfcdba431, 5 }, //  6
	{ "du06.a16",		0x10000, 0x7d50bebc, 5 }, //  7
	{ "du07.a20",		0x10000, 0x8fdf0fa5, 5 }, //  8

	{ "du01.a3",		0x10000, 0xd3a58e9e, 6 }, //  9 gfx3
	{ "du03.a6",		0x10000, 0x4fc4fb0f, 6 }, // 10
	{ "du00.a1",		0x10000, 0xac201f2d, 6 }, // 11
	{ "du02.a5",		0x10000, 0x7ddc5651, 6 }, // 12

	{ "du-13.c8",		0x00200, 0xbea1f87e, 8 }, // 14 proms
};

STD_ROM_PICK(oscarj2)
STD_ROM_FN(oscarj2)

struct BurnDriver BurnDrvOscarj2 = {
	"oscarj2", "oscar", NULL, NULL, "1987",
	"Psycho-Nics Oscar (Japan revision 2)\0", NULL, "Data East Corporation", "DEC8",
	L"\u94A2\u94C1\u91D1\u521A (\u65E5\u7248 \u4FEE\u8BA2\u7248 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_RUNGUN, 0,
	NULL, oscarj2RomInfo, oscarj2RomName, NULL, NULL, NULL, NULL, OscarInputInfo, OscarjDIPInfo,
	OscarInit, GondoExit, OscarFrame, OscarDraw, GondoScan, &DrvRecalc, 0x200,
	256, 240, 4, 3
};

static INT32 stopsubcpu = 0;
static INT32 nLastMiss = 0;

static void lastmiss_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x1803:
			M6809SetIRQLine(0, 0, CPU_IRQSTATUS_HOLD); // main irq
		return;

		case 0x1804:
			M6809SetIRQLine(1, 0, CPU_IRQSTATUS_HOLD); // sub irq
		return;

		case 0x1805:
			memcpy (DrvSprBuf, DrvSprRAM, 0x800);
		return;

		case 0x1807:
			*flipscreen = data;
		return;

		case 0x180c:
			*soundlatch = data;
			M6502SetIRQLine(M6502_INPUT_LINE_NMI, CPU_IRQSTATUS_AUTO);
		return;

		// main cpu only!

		case 0x1809:
			DrvPf0Ctrl[0x11] = data;
		return;

		case 0x180b:
			DrvPf0Ctrl[0x13] = data;
		return;

		case 0x180d:
			if (M6809GetActive() == 0) {
				m6809_bankswitch(data & 0x0f);

				if (~data & 0x80 && nLastMiss) {
					M6809Reset(1);
					stopsubcpu = 1;
				} else {
					stopsubcpu = 0;
				}
			}

			DrvPf0Ctrl[0x10] = (data >> 5) & 1;
			DrvPf0Ctrl[0x12] = (data >> 6) & 1;
		return;

		case 0x180e:
		case 0x180f:
			i8751_write(address & 1, data);
		return;
	}
}

static UINT8 lastmiss_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x1800:
			return DrvInputs[0];

		case 0x1801:
			return DrvInputs[1];

		case 0x1802:
			return (DrvInputs[2] & 0x7f) | vblank;

		case 0x1803:
			return DrvDips[0];

		case 0x1804:
			return DrvDips[1];

		case 0x1806:
			return i8751_hi();

		case 0x1807:
			return i8751_lo();
	}

	return 0;
}

static INT32 LastmissDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	M6809Open(0);
	M6809Reset();
	BurnYM2203Reset();
	M6809Close();

	M6809Open(1);
	M6809Reset();
	M6809Close();

	DrvMCUReset();

	M6502Open(0);
	M6502Reset();
	BurnYM3526Reset();
	M6502Close();

	stopsubcpu = 0;

	return 0;
}

static INT32 LastmissGfxDecode()
{
	INT32 Plane0[3] = { 0x6000*8,0x4000*8,0x2000*8 };
	INT32 Plane1[4] = { 0x60000*8,0x40000*8,0x20000*8,0x00000*8 };
	INT32 XOffs0[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
	INT32 XOffs1[16]= {16*8, 1+(16*8), 2+(16*8), 3+(16*8), 4+(16*8), 5+(16*8), 6+(16*8), 7+(16*8),0,1,2,3,4,5,6,7 };
	INT32 YOffs0[16] = { 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 ,8*8,9*8,10*8,11*8,12*8,13*8,14*8,15*8 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x80000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM0, 0x08000);

	GfxDecode(0x0400, 3,  8,  8, Plane0, XOffs0, YOffs0, 0x040, tmp, DrvGfxROM0);

	memcpy (tmp, DrvGfxROM1, 0x80000);

	GfxDecode(0x1000, 4, 16, 16, Plane1, XOffs1, YOffs0, 0x100, tmp, DrvGfxROM1);

	memcpy (tmp, DrvGfxROM2, 0x80000);

	GfxDecode(0x1000, 4, 16, 16, Plane1, XOffs1, YOffs0, 0x100, tmp, DrvGfxROM2);

	BurnFree (tmp);

	return 0;
}

static INT32 LastmissInit()
{
	BurnSetRefreshRate(58.00);

	BurnAllocMemIndex();

	{
		if (!strncmp(BurnDrvGetTextA(DRV_NAME), "lastm", 5)) {
			if (BurnLoadRom(DrvMainROM  + 0x08000,  0, 1)) return 1;
			if (BurnLoadRom(DrvMainROM  + 0x10000,  1, 1)) return 1;

			if (BurnLoadRom(DrvSubROM   + 0x00000,  2, 1)) return 1;

			if (BurnLoadRom(DrvM6502ROM + 0x08000,  3, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM3  + 0x00000,  4, 1)) return 1;
			memcpy (DrvGfxROM0 + 0x00000, DrvGfxROM3 + 0x00000, 0x02000);
			memcpy (DrvGfxROM0 + 0x06000, DrvGfxROM3 + 0x02000, 0x02000);
			memcpy (DrvGfxROM0 + 0x04000, DrvGfxROM3 + 0x04000, 0x02000);
			memcpy (DrvGfxROM0 + 0x02000, DrvGfxROM3 + 0x06000, 0x02000);

			if (BurnLoadRom(DrvGfxROM1  + 0x00000,  5, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1  + 0x20000,  6, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1  + 0x40000,  7, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1  + 0x60000,  8, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM2  + 0x00000,  9, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2  + 0x20000, 10, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2  + 0x40000, 11, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2  + 0x60000, 12, 1)) return 1;

			if (BurnLoadRom(DrvMCURom   + 0x00000, 13, 1)) return 1;

			nLastMiss = 1;
		} else {
			if (BurnLoadRom(DrvMainROM  + 0x08000,  0, 1)) return 1;
			if (BurnLoadRom(DrvMainROM  + 0x10000,  1, 1)) return 1;
			if (BurnLoadRom(DrvMainROM  + 0x20000,  2, 1)) return 1;
			if (BurnLoadRom(DrvMainROM  + 0x30000,  3, 1)) return 1;
			if (BurnLoadRom(DrvMainROM  + 0x40000,  4, 1)) return 1;
			if (BurnLoadRom(DrvMainROM  + 0x48000,  4, 1)) return 1; // reload

			if (BurnLoadRom(DrvSubROM   + 0x00000,  5, 1)) return 1;

			if (BurnLoadRom(DrvM6502ROM + 0x08000,  6, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM0  + 0x00000,  7, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM1  + 0x00000,  8, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1  + 0x10000,  9, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1  + 0x20000, 10, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1  + 0x30000, 11, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1  + 0x40000, 12, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1  + 0x50000, 13, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1  + 0x60000, 14, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM1  + 0x70000, 15, 1)) return 1;

			if (BurnLoadRom(DrvGfxROM2  + 0x00000, 16, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2  + 0x20000, 17, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2  + 0x40000, 18, 1)) return 1;
			if (BurnLoadRom(DrvGfxROM2  + 0x60000, 19, 1)) return 1;

			if (BurnLoadRom(DrvMCURom   + 0x00000, 20, 1)) return 1;
		}

		LastmissGfxDecode();
	}

	M6809Init(0);
	M6809Open(0);
	M6809MapMemory(DrvMainRAM,		 0x0000, 0x0fff, MAP_RAM);
	M6809MapMemory(DrvPalRAM,		 0x1000, 0x17ff, MAP_RAM);
	M6809MapMemory(DrvVidRAM,		 0x2000, 0x27ff, MAP_RAM);
	M6809MapMemory(DrvSprRAM,		 0x2800, 0x2fff, MAP_RAM);
	M6809MapMemory(DrvMainRAM + 0x1000,	 0x3000, 0x37ff, MAP_RAM);
	M6809MapMemory(DrvPf0RAM,		 0x3800, 0x3fff, MAP_RAM);
	M6809MapMemory(DrvMainROM + 0x10000,     0x4000, 0x7fff, MAP_ROM);
	M6809MapMemory(DrvMainROM + 0x08000,     0x8000, 0xffff, MAP_ROM);
	M6809SetWriteHandler(lastmiss_main_write);
	M6809SetReadHandler(lastmiss_main_read);
	M6809Close();

	M6809Init(1);
	M6809Open(1);
	M6809MapMemory(DrvMainRAM,		 0x0000, 0x0fff, MAP_RAM);
	M6809MapMemory(DrvPalRAM,		 0x1000, 0x17ff, MAP_RAM);
	M6809MapMemory(DrvVidRAM,		 0x2000, 0x27ff, MAP_RAM);
	M6809MapMemory(DrvSprRAM,		 0x2800, 0x2fff, MAP_RAM);
	M6809MapMemory(DrvMainRAM + 0x1000,	 0x3000, 0x37ff, MAP_RAM);
	M6809MapMemory(DrvPf0RAM,		 0x3800, 0x3fff, MAP_RAM);
	M6809MapMemory(DrvSubROM + 0x04000,      0x4000, 0xffff, MAP_ROM);
	M6809SetWriteHandler(lastmiss_main_write);
	M6809SetReadHandler(lastmiss_main_read);
	M6809Close();

	M6502Init(0, TYPE_M6502);
	M6502Open(0);
	M6502MapMemory(DrvM6502RAM,          0x0000, 0x05ff, MAP_RAM);
	M6502MapMemory(DrvM6502ROM + 0x8000, 0x8000, 0xffff, MAP_ROM);
	M6502SetReadHandler(ghostb_sound_read);
	M6502SetWriteHandler(gondo_sound_write);
	M6502Close();

	DrvMCUInit(2);

	BurnYM3526Init(3000000, &DrvYM3812FMIRQHandler, 0);
	BurnTimerAttach(&M6502Config, 1500000);
	BurnYM3526SetRoute(BURN_SND_YM3526_ROUTE, 0.70, BURN_SND_ROUTE_BOTH);

	BurnYM2203Init(1, 1500000, NULL, 1);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_YM2203_ROUTE, 0.20, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_1, 0.23, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_2, 0.23, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_3, 0.23, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	LastmissDoReset();

	return 0;
}

static INT32 LastmissExit()
{
	M6809Exit();
	BurnYM3526Exit();
	nLastMiss = 0;
	return DrvExit();
}

static void lastmiss_draw_bg_layer(INT32 priority, INT32 t)
{
	INT32 scrolly = ((DrvPf0Ctrl[0x12] << 8) + DrvPf0Ctrl[0x13] + 8) & 0x1ff;
	INT32 scrollx = ((DrvPf0Ctrl[0x10] << 8) | DrvPf0Ctrl[0x11]) & 0x1ff;

	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 4;
		INT32 sy = (offs >> 5) << 4;

		sx -= scrollx;
		if (sx < -15) sx += 512;
		sy -= scrolly;
		if (sy < -15) sy += 512;

		if (sx >= nScreenWidth || sy >= nScreenHeight) continue;

		INT32 ofst = (offs & 0x0f) | ((offs >> 1) & 0xf0) | ((offs & 0x10) << 4) | (offs & 0x200);

		INT32 code  = (DrvPf0RAM[ofst * 2 + 1]) | (DrvPf0RAM[ofst * 2 + 0] << 8);
		INT32 color = code >> 12;

		if (priority && color < 8) continue;

		{
			color = (color << 4) | 0x300;
			UINT8 *src = DrvGfxROM2 + ((code & 0xfff) << 8);
			UINT16 *dst;

			for (INT32 y = 0; y < 16; y++, sy++, src+=16)
			{
				if (sy < 0 || sy >= nScreenHeight) continue;

				dst = pTransDraw + sy * nScreenWidth;

				INT32 xx = sx;
				for (INT32 x = 0; x < 16; x++, xx++)
				{
					INT32 pxl = src[x];
					if (t & (1 << pxl)) continue;

					if (xx >= nScreenWidth || xx < 0) continue;

					dst[xx] = pxl | color;
				}
			}
		}
	}
}

static void lastmiss_draw_txt_layer()
{
	for (INT32 offs = 0; offs < 32 * 32; offs++)
	{
		INT32 sx = (offs & 0x1f) << 3;
		INT32 sy = (offs >> 5) << 3;

		sy -= 8;
		if (sy < 0 || sy >= nScreenHeight) continue;

		INT32 code  = (DrvVidRAM[offs * 2 + 1]) | (DrvVidRAM[offs * 2 + 0] << 8);
		INT32 color = code >> 14;

		Render8x8Tile_Mask(pTransDraw, code & 0xfff, sx, sy, color, 3, 0, 0, DrvGfxROM0);
	}
}

static void DrvRecalcSplit(INT32 len)
{
	UINT8 r,g,b;
	for (INT32 i = 0; i < len; i++) {
		INT32 d = (DrvPalRAM[i + 0]) | (DrvPalRAM[i + len] << 8);
		r = (d >> 0) & 0x0f;
		g = (d >> 4) & 0x0f;
		b = (d >> 8) & 0x0f;

		r |= r << 4;
		g |= g << 4;
		b |= b << 4;

		DrvPalette[i] = BurnHighCol(r, g, b, 0);
	}
}

static INT32 LastmissDraw()
{
	if (DrvRecalc) {
		DrvRecalcSplit(0x400);
	}

	BurnTransferClear();
	if (nBurnLayer & 1) lastmiss_draw_bg_layer(0,0);
	if (nSpriteEnable & 1) draw_sprites1(0);
	if (nBurnLayer & 2) lastmiss_draw_txt_layer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 ShackledDraw()
{
	if (DrvRecalc) {
		DrvRecalcSplit(0x400);
	}

	BurnTransferClear();
	if (nBurnLayer & 1) lastmiss_draw_bg_layer(0, 0xfff0);
	if (nBurnLayer & 2) lastmiss_draw_bg_layer(0, 0x000f);
	if (nBurnLayer & 4) lastmiss_draw_bg_layer(1, 0xfff0);
	if (nSpriteEnable & 1) draw_sprites1(0);
	if (nBurnLayer & 8) lastmiss_draw_bg_layer(1, 0x000f);
	if (nSpriteEnable & 2) lastmiss_draw_txt_layer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 LastmissFrame()
{
	if (DrvReset) {
		LastmissDoReset();
	}

	M6809NewFrame();
	M6502NewFrame();
	mcs51NewFrame();

	{
		UINT8 last_coins = DrvInputs[4];
		memset (DrvInputs, 0xff, 5);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
			DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
		}
		if (DrvInputs[4] != last_coins && DrvInputs[4] != 0xff) {
			mcs51_set_irq_line(MCS51_INT0_LINE, CPU_IRQSTATUS_ACK);
		}
	}

	INT32 nInterleave = 256 * 10; // shackled needs very tight sync
	INT32 nCyclesTotal[3] = { 2000000 / 58, 2000000 / 58, 1500000 / 58 };
	INT32 nCyclesDone[3] = { 0, 0, 0 };

	M6502Open(0);

	vblank = 0x80;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		if (i == 8 * 10) vblank = 0x00;
		if ((nLastMiss && i == 248 * 10) ||
			(!nLastMiss && i == 200 * 10)) {
			// shackled(us) needs a longer vbl time for continue to work. -dink
			vblank = 0x80;

			if (pBurnDraw) {
				BurnDrvRedraw();
			}
		}

		M6809Open(0);
		CPU_RUN(0, M6809);
		CPU_RUN_TIMER(2); // why here?
		M6809Close();

		M6809Open(1);
		if (stopsubcpu) {
			CPU_IDLE(1, M6809);
		} else {
			CPU_RUN(1, M6809);
		}
		if (realMCU) {
			DrvMCUSync();
		}
		M6809Close();
	}

	M6502Close();

	if (pBurnSoundOut) {
		BurnYM3526Update(pBurnSoundOut, nBurnSoundLen);
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
	}

	return 0;
}

static INT32 LastmissScan(INT32 nAction, INT32 *pnMin)
{
	if (nAction & ACB_DRIVER_DATA) {
		M6809Scan(nAction);
		BurnYM3526Scan(nAction, pnMin);

		SCAN_VAR(stopsubcpu);
		SCAN_VAR(RomBank);

		if (nAction & ACB_WRITE) {
			M6809Open(0);
			m6809_bankswitch(RomBank);
			M6809Close();
		}
	}

	return DrvScan(nAction, pnMin);
}


// Last Mission (World revision 8)

static struct BurnRomInfo lastmisnRomDesc[] = {
	{ "last_mission_dl03-8.13h",	0x08000, 0xa4f8d54b, 1 }, //  0 maincpu
	{ "last_mission_dl04-5.7h",		0x10000, 0x7dea1552, 1 }, //  1

	{ "last_mission_dl02-5.18h",	0x10000, 0xec9b5daf, 2 }, //  2 sub

	{ "last_mission_dl05-.5h",		0x08000, 0x1a5df8c0, 3 }, //  3 audiocpu

	{ "last_mission_dl01-.2a",		0x08000, 0xf3787a5d, 4 }, //  4 gfx1

	{ "last_mission_dl11-.13f",		0x08000, 0x36579d3b, 5 }, //  5 gfx2
	{ "last_mission_dl12-.9f",		0x08000, 0x2ba6737e, 5 }, //  6
	{ "last_mission_dl13-.8f",		0x08000, 0x39a7dc93, 5 }, //  7
	{ "last_mission_dl10-.16f",		0x08000, 0xfe275ea8, 5 }, //  8

	{ "last_mission_dl09-.12k",		0x10000, 0x6a5a0c5d, 6 }, //  9 gfx3
	{ "last_mission_dl08-.14k",		0x10000, 0x3b38cfce, 6 }, // 10
	{ "last_mission_dl07-.15k",		0x10000, 0x1b60604d, 6 }, // 11
	{ "last_mission_dl06-.17k",		0x10000, 0xc43c26a7, 6 }, // 12

	{ "last_mission_dl00-e.18a",	0x01000, 0xe97481c6, 7 }, // 13 i8751 microcontroller

	{ "dl-14.9c",					0x00100, 0x2e55aa12, 8 }, // 14 proms
};

STD_ROM_PICK(lastmisn)
STD_ROM_FN(lastmisn)

struct BurnDriver BurnDrvLastmisn = {
	"lastmisn", NULL, NULL, NULL, "1986",
	"Last Mission (World revision 8)\0", NULL, "Data East Corporation", "DEC8",
	L"\u6700\u7EC8\u4EFB\u52A1 (\u4E16\u754C\u7248 \u4FEE\u8BA2\u7248 8)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SHOOT, 0,
	NULL, lastmisnRomInfo, lastmisnRomName, NULL, NULL, NULL, NULL, LastmisnInputInfo, LastmisnDIPInfo,
	LastmissInit, LastmissExit, LastmissFrame, LastmissDraw, LastmissScan, &DrvRecalc, 0x400,
	240, 256, 3, 4
};


// Last Mission (US revision 6)

static struct BurnRomInfo lastmsnu6RomDesc[] = {
	{ "last_mission_dl03-6.13h",	0x08000, 0x47751a5e, 1 }, //  0 maincpu
	{ "last_mission_dl04-5.7h",		0x10000, 0x7dea1552, 1 }, //  1

	{ "last_mission_dl02-5.18h",	0x10000, 0xec9b5daf, 2 }, //  2 sub

	{ "last_mission_dl05-.5h",		0x08000, 0x1a5df8c0, 3 }, //  3 audiocpu

	{ "last_mission_dl01-.2a",		0x08000, 0xf3787a5d, 4 }, //  4 gfx1

	{ "last_mission_dl11-.13f",		0x08000, 0x36579d3b, 5 }, //  5 gfx2
	{ "last_mission_dl12-.9f",		0x08000, 0x2ba6737e, 5 }, //  6
	{ "last_mission_dl13-.8f",		0x08000, 0x39a7dc93, 5 }, //  7
	{ "last_mission_dl10-.16f",		0x08000, 0xfe275ea8, 5 }, //  8

	{ "last_mission_dl09-.12k",		0x10000, 0x6a5a0c5d, 6 }, //  9 gfx3
	{ "last_mission_dl08-.14k",		0x10000, 0x3b38cfce, 6 }, // 10
	{ "last_mission_dl07-.15k",		0x10000, 0x1b60604d, 6 }, // 11
	{ "last_mission_dl06-.17k",		0x10000, 0xc43c26a7, 6 }, // 12

	{ "last_mission_dl00-a.18a",	0x01000, 0xe97481c6, 7 }, // 13 i8751 microcontroller

	{ "dl-14.9c",					0x00100, 0x2e55aa12, 8 }, // 14 proms
};

STD_ROM_PICK(lastmsnu6)
STD_ROM_FN(lastmsnu6)

struct BurnDriver BurnDrvLastmsnu6 = {
	"lastmisnu6", "lastmisn", NULL, NULL, "1986",
	"Last Mission (US revision 6)\0", NULL, "Data East USA", "DEC8",
	L"\u6700\u7EC8\u4EFB\u52A1 (\u7F8E\u7248 \u4FEE\u8BA2\u7248 6)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SHOOT, 0,
	NULL, lastmsnu6RomInfo, lastmsnu6RomName, NULL, NULL, NULL, NULL, LastmisnInputInfo, LastmisnDIPInfo,
	LastmissInit, LastmissExit, LastmissFrame, LastmissDraw, LastmissScan, &DrvRecalc, 0x400,
	240, 256, 3, 4
};


// Last Mission (US revision 5)

static struct BurnRomInfo lastmsnu5RomDesc[] = {
	{ "last_mission_dl03-5.13h",	0x08000, 0x357f5f6b, 1 }, //  0 maincpu
	{ "last_mission_dl04-5.7h",		0x10000, 0x7dea1552, 1 }, //  1

	{ "last_mission_dl02-5.18h",	0x10000, 0xec9b5daf, 2 }, //  2 sub

	{ "last_mission_dl05-.5h",		0x08000, 0x1a5df8c0, 3 }, //  3 audiocpu

	{ "last_mission_dl01-.2a",		0x08000, 0xf3787a5d, 4 }, //  4 gfx1

	{ "last_mission_dl11-.13f",		0x08000, 0x36579d3b, 5 }, //  5 gfx2
	{ "last_mission_dl12-.9f",		0x08000, 0x2ba6737e, 5 }, //  6
	{ "last_mission_dl13-.8f",		0x08000, 0x39a7dc93, 5 }, //  7
	{ "last_mission_dl10-.16f",		0x08000, 0xfe275ea8, 5 }, //  8

	{ "last_mission_dl09-.12k",		0x10000, 0x6a5a0c5d, 6 }, //  9 gfx3
	{ "last_mission_dl08-.14k",		0x10000, 0x3b38cfce, 6 }, // 10
	{ "last_mission_dl07-.15k",		0x10000, 0x1b60604d, 6 }, // 11
	{ "last_mission_dl06-.17k",		0x10000, 0xc43c26a7, 6 }, // 12

	{ "last_mission_dl00-a.18a",	0x01000, 0xe97481c6, 7 }, // 13 i8751 microcontroller

	{ "dl-14.9c",					0x00100, 0x2e55aa12, 8 }, // 14 proms
};

STD_ROM_PICK(lastmsnu5)
STD_ROM_FN(lastmsnu5)

struct BurnDriver BurnDrvLastmsnu5 = {
	"lastmisnu5", "lastmisn", NULL, NULL, "1986",
	"Last Mission (US revision 5)\0", NULL, "Data East USA", "DEC8",
	L"\u6700\u7EC8\u4EFB\u52A1 (\u7F8E\u7248 \u4FEE\u8BA2\u7248 5)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SHOOT, 0,
	NULL, lastmsnu5RomInfo, lastmsnu5RomName, NULL, NULL, NULL, NULL, LastmisnInputInfo, LastmisnDIPInfo,
	LastmissInit, LastmissExit, LastmissFrame, LastmissDraw, LastmissScan, &DrvRecalc, 0x400,
	240, 256, 3, 4
};


// Last Mission (Japan)

static struct BurnRomInfo lastmsnjRomDesc[] = {
	{ "dl03-.13h",					0x08000, 0x4be5e7e1, 1 }, //  0 maincpu
	{ "dl04-.7h",					0x10000, 0xf026adf9, 1 }, //  1

	{ "dl02-.18h",					0x10000, 0xd0de2b5d, 2 }, //  2 sub

	{ "last_mission_dl05-.5h",		0x08000, 0x1a5df8c0, 3 }, //  3 audiocpu

	{ "last_mission_dl01-.2a",		0x08000, 0xf3787a5d, 4 }, //  4 gfx1

	{ "last_mission_dl11-.13f",		0x08000, 0x36579d3b, 5 }, //  5 gfx2
	{ "last_mission_dl12-.9f",		0x08000, 0x2ba6737e, 5 }, //  6
	{ "last_mission_dl13-.8f",		0x08000, 0x39a7dc93, 5 }, //  7
	{ "last_mission_dl10-.16f",		0x08000, 0xfe275ea8, 5 }, //  8

	{ "last_mission_dl09-.12k",		0x10000, 0x6a5a0c5d, 6 }, //  9 gfx3
	{ "last_mission_dl08-.14k",		0x10000, 0x3b38cfce, 6 }, // 10
	{ "last_mission_dl07-.15k",		0x10000, 0x1b60604d, 6 }, // 11
	{ "last_mission_dl06-.17k",		0x10000, 0xc43c26a7, 6 }, // 12

	// created from dump of the US version
	{ "last_mission_japan.18a",		0x01000, 0x0d58c3a1, 7 }, // 13 mcu

	{ "dl-14.9c",					0x00100, 0x2e55aa12, 8 }, // 14 proms
};

STD_ROM_PICK(lastmsnj)
STD_ROM_FN(lastmsnj)

struct BurnDriver BurnDrvLastmsnj = {
	"lastmisnj", "lastmisn", NULL, NULL, "1986",
	"Last Mission (Japan)\0", NULL, "Data East Corporation", "DEC8",
	L"\u6700\u7EC8\u4EFB\u52A1 (\u65E5\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SHOOT, 0,
	NULL, lastmsnjRomInfo, lastmsnjRomName, NULL, NULL, NULL, NULL, LastmisnInputInfo, LastmsnjDIPInfo,
	LastmissInit, LastmissExit, LastmissFrame, LastmissDraw, LastmissScan, &DrvRecalc, 0x400,
	240, 256, 3, 4
};


// Shackled (US)

static struct BurnRomInfo shackledRomDesc[] = {
	{ "dk-02.13h",		0x08000, 0x87f8fa85, 1 }, //  0 maincpu
	{ "dk-06.7h",		0x10000, 0x69ad62d1, 1 }, //  1
	{ "dk-05.9h",		0x10000, 0x598dd128, 1 }, //  2
	{ "dk-04.10h",		0x10000, 0x36d305d4, 1 }, //  3
	{ "dk-03.11h",		0x08000, 0x6fd90fd1, 1 }, //  4

	{ "dk-01.18h",		0x10000, 0x71fe3bda, 2 }, //  5 sub

	{ "dk-07.5h",		0x08000, 0x887e4bcc, 3 }, //  6 audiocpu

	{ "dk-00.2a",		0x08000, 0x69b975aa, 5 }, //  7 gfx1

	{ "dk-12.15k",		0x10000, 0x615c2371, 6 }, //  8 gfx2
	{ "dk-13.14k",		0x10000, 0x479aa503, 6 }, // 09
	{ "dk-14.13k",		0x10000, 0xcdc24246, 6 }, // 10
	{ "dk-15.11k",		0x10000, 0x88db811b, 6 }, // 11
	{ "dk-16.10k",		0x10000, 0x061a76bd, 6 }, // 12
	{ "dk-17.9k",		0x10000, 0xa6c5d8af, 6 }, // 13
	{ "dk-18.8k",		0x10000, 0x4d466757, 6 }, // 14
	{ "dk-19.6k",		0x10000, 0x1911e83e, 6 }, // 15

	{ "dk-11.12k",		0x10000, 0x5cf5719f, 7 }, // 16 gfx3
	{ "dk-10.14k",		0x10000, 0x408e6d08, 7 }, // 17
	{ "dk-09.15k",		0x10000, 0xc1557fac, 7 }, // 18
	{ "dk-08.17k",		0x10000, 0x5e54e9f5, 7 }, // 19

	{ "dk-e.18a",		0x01000, 0x1af06149, 4 | BRF_ESS }, // 20 mcu

	{ "dk-20.9c",		0x00100, 0xff3cd588, 0 | BRF_OPT }, // priority PROM
};

STD_ROM_PICK(shackled)
STD_ROM_FN(shackled)

struct BurnDriver BurnDrvShackled = {
	"shackled", NULL, NULL, NULL, "1986",
	"Shackled (US)\0", NULL, "Data East USA", "DEC8",
	L"\u94FE\u72F1\u8C1C\u57CE (\u7F8E\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_MAZE | GBF_RUNGUN, 0,
	NULL, shackledRomInfo, shackledRomName, NULL, NULL, NULL, NULL, ShackledInputInfo, ShackledDIPInfo,
	LastmissInit, LastmissExit, LastmissFrame, ShackledDraw, LastmissScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};


// Breywood (Japan revision 2)

static struct BurnRomInfo breywoodRomDesc[] = {
	{ "dj02-2.13h",		0x08000, 0xc19856b9, 1 }, //  0 maincpu
	{ "dj06-2.7h",		0x10000, 0x2860ea02, 1 }, //  1
	{ "dj05-2.9h",		0x10000, 0x0fdd915e, 1 }, //  2
	{ "dj04-2.10h",		0x10000, 0x71036579, 1 }, //  3
	{ "dj03-2.11h",		0x08000, 0x308f4893, 1 }, //  4

	{ "dj1-2y.18h",		0x10000, 0x3d9fb623, 2 }, //  5 sub

	{ "dj07-1.5h",		0x08000, 0x4a471c38, 3 }, //  6 audiocpu

	{ "dj-00.2a",		0x08000, 0x815a891a, 5 }, //  7 gfx1

	{ "dj12.15k",		0x10000, 0x2b7634f2, 6 }, //  8 gfx2
	{ "dj13.14k",		0x10000, 0x4530a952, 6 }, //  9
	{ "dj14.13k",		0x10000, 0x87c28833, 6 }, // 10
	{ "dj15.11k",		0x10000, 0xbfb43a4d, 6 }, // 11
	{ "dj16.10k",		0x10000, 0xf9848cc4, 6 }, // 12
	{ "dj17.9k",		0x10000, 0xbaa3d218, 6 }, // 13
	{ "dj18.8k",		0x10000, 0x12afe533, 6 }, // 14
	{ "dj19.6k",		0x10000, 0x03373755, 6 }, // 15

	{ "dj11.12k",		0x10000, 0x067e2a43, 7 }, // 16 gfx3
	{ "dj10.14k",		0x10000, 0xc19733aa, 7 }, // 17
	{ "dj09.15k",		0x10000, 0xe37d5dbe, 7 }, // 18
	{ "dj08.17k",		0x10000, 0xbeee880f, 7 }, // 19

	{ "dj.18a",			0x01000, 0x4cb20332, 4 }, // 20 i8751 microcontroller

	{ "dk-20.9c",		0x00100, 0xff3cd588, 0 | BRF_OPT }, // priority PROM
};

STD_ROM_PICK(breywood)
STD_ROM_FN(breywood)

struct BurnDriver BurnDrvBreywood = {
	"breywood", "shackled", NULL, NULL, "1986",
	"Breywood (Japan revision 2)\0", NULL, "Data East Corporation", "DEC8",
	L"\u94FE\u72F1\u8C1C\u57CE (\u65E5\u7248 \u4FEE\u8BA2\u7248 2)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_MAZE | GBF_RUNGUN, 0,
	NULL, breywoodRomInfo, breywoodRomName, NULL, NULL, NULL, NULL, ShackledInputInfo, BreywoodDIPInfo,
	LastmissInit, LastmissExit, LastmissFrame, ShackledDraw, LastmissScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};


static INT32 MSM5205Next = 0;
static UINT8 MSM5205Last = 0;
static INT32 Toggle = 0;
static INT32 SndRomBank = 0;

static void csilver_main_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x1803:
			M6809SetIRQLine(0, 0, CPU_IRQSTATUS_HOLD);
		return;

		case 0x1804:
			M6809SetIRQLine(1, 0, CPU_IRQSTATUS_HOLD);
		return;

		case 0x1805:
			memcpy (DrvSprBuf, DrvSprRAM, 0x800);
		return;

		case 0x1807:
			*flipscreen = data;
		return;

		case 0x180c:
			*soundlatch = data;
			M6502SetIRQLine(M6502_INPUT_LINE_NMI, CPU_IRQSTATUS_AUTO);
		return;

		case 0x1808:
		case 0x1809: // maincpu
		case 0x180a:
		case 0x180b: // scroll
			DrvPf0Ctrl[0x10 + (address & 3)] = data;
		return;


		case 0x180d: // main
			if (M6809GetActive() == 0) m6809_bankswitch(data);
		return;

		case 0x180e: // main
		case 0x180f:
			if (M6809GetActive() == 0) {
				i8751_write(address & 1, data);
			}
		return;
	}
}

static UINT8 csilver_main_read(UINT16 address)
{
	switch (address)
	{
		case 0x1800:
			return DrvInputs[1];

		case 0x1801:
			return DrvInputs[0];

		case 0x1803:
			return (DrvInputs[2] & 0x7f) | vblank;

		case 0x1804:
			return DrvDips[1];

		case 0x1805:
			return DrvDips[0];

		case 0x1c00:
			return i8751_hi();

		case 0x1e00:
			return i8751_lo();
	}

	return 0x00;
}

static void csilver_sound_bank(INT32 data)
{
	SndRomBank = data;

	M6502MapMemory(DrvM6502ROM + 0x10000 + ((data & 8) >> 3) * 0x4000, 0x4000, 0x7fff, MAP_ROM);
}

static void csilver_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0x0800:
		case 0x0801:
			BurnYM2203Write(0, address & 1, data);
		return;

		case 0x1000:
		case 0x1001:
			BurnYM3526Write(address & 1, data);
		return;

		case 0x1800:
			MSM5205Next = data;
			if (MSM5205Last == 0x8 && MSM5205Next == 0x8) { // clears up hissing & clicking noise
				MSM5205ResetWrite(0, 1);
			} else MSM5205ResetWrite(0, 0);
			MSM5205Last = data;
		return;

		case 0x2000:
			csilver_sound_bank(data);
		return;
	}
}

static UINT8 csilver_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0x3000:
			return *soundlatch;

		case 0x3400:
			MSM5205ResetWrite(0, 0);
			return 0;
	}

	return 0;
}

static void CsilverADPCMInt()
{
	Toggle ^= 1;
	if (Toggle)	M6502SetIRQLine(M6502_IRQ_LINE, CPU_IRQSTATUS_AUTO);

	MSM5205DataWrite(0, MSM5205Next >> 4);
	MSM5205Next <<= 4;
}

static INT32 CsilverDoReset()
{
	INT32 nRet = LastmissDoReset();

	MSM5205Reset();

	return nRet;
}

static INT32 CsilverInit()
{
	BurnSetRefreshRate(58.00);

	BurnAllocMemIndex();

	{
		if (BurnLoadRom(DrvMainROM + 0x08000,  0, 1)) return 1;
		if (BurnLoadRom(DrvMainROM + 0x10000,  1, 1)) return 1;
		if (BurnLoadRom(DrvMainROM + 0x20000,  2, 1)) return 1;

		if (BurnLoadRom(DrvSubROM + 0x00000,   3, 1)) return 1;

		if (BurnLoadRom(DrvM6502ROM  + 0x00000,   4, 1)) return 1;
		memcpy (DrvM6502ROM + 0x10000, DrvM6502ROM + 0x00000, 0x8000);

		if (BurnLoadRom(DrvGfxROM0   + 0x00000,   5, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1   + 0x00000,   6, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x20000,   7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1   + 0x40000,   8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2   + 0x00000,   9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2   + 0x10000,  10, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2   + 0x20000,  11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2   + 0x30000,  12, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2   + 0x40000,  13, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2   + 0x50000,  14, 1)) return 1;

		if (BurnLoadRom(DrvMCURom    + 0x00000,  15, 1)) return 1;

		LastmissGfxDecode();
	}

	M6809Init(0);
	M6809Open(0);
	M6809MapMemory(DrvMainRAM + 0x00000,		0x0000, 0x0fff, MAP_RAM);
	M6809MapMemory(DrvPalRAM,			0x1000, 0x17ff, MAP_RAM); // xxxxbbbbggggrrrr
	M6809MapMemory(DrvVidRAM,			0x2000, 0x27ff, MAP_RAM);
	M6809MapMemory(DrvSprRAM,			0x2800, 0x2fff, MAP_RAM);
	M6809MapMemory(DrvMainRAM + 0x01000,		0x3000, 0x37ff, MAP_RAM);
	M6809MapMemory(DrvPf0RAM,			0x3800, 0x3fff, MAP_RAM);
	M6809MapMemory(DrvMainROM + 0x10000,		0x4000, 0x7fff, MAP_ROM);
	M6809MapMemory(DrvMainROM + 0x08000,		0x8000, 0xffff, MAP_ROM);
	M6809SetWriteHandler(csilver_main_write);
	M6809SetReadHandler(csilver_main_read);
	M6809Close();

	M6809Init(1);
	M6809Open(1);
	M6809MapMemory(DrvMainRAM + 0x00000,		0x0000, 0x0fff, MAP_RAM);
	M6809MapMemory(DrvPalRAM,			0x1000, 0x17ff, MAP_RAM); // xxxxbbbbggggrrrr
	M6809MapMemory(DrvVidRAM,			0x2000, 0x27ff, MAP_RAM);
	M6809MapMemory(DrvSprRAM,			0x2800, 0x2fff, MAP_RAM);
	M6809MapMemory(DrvMainRAM + 0x01000,		0x3000, 0x37ff, MAP_RAM);
	M6809MapMemory(DrvPf0RAM,			0x3800, 0x3fff, MAP_ROM);
	M6809MapMemory(DrvSubROM + 0x04000,		0x4000, 0xffff, MAP_ROM);
	M6809SetWriteHandler(csilver_main_write);
	M6809SetReadHandler(csilver_main_read);
	M6809Close();

	M6502Init(0, TYPE_M6502);
	M6502Open(0);
	M6502MapMemory(DrvM6502RAM,          0x0000, 0x07ff, MAP_RAM);
	M6502MapMemory(DrvM6502ROM + 0x4000, 0x4000, 0x7fff, MAP_ROM);
	M6502MapMemory(DrvM6502ROM + 0x8000, 0x8000, 0xffff, MAP_ROM);
	M6502SetReadHandler(csilver_sound_read);
	M6502SetWriteHandler(csilver_sound_write);
	M6502Close();

	if (DrvMCURom[0] != 0x00) {
		DrvMCUInit(3);
	}

	BurnYM3526Init(3000000, &DrvYM3812FMIRQHandler, 0);
	BurnTimerAttach(&M6502Config, 1500000);
	BurnYM3526SetRoute(BURN_SND_YM3526_ROUTE, 0.70, BURN_SND_ROUTE_BOTH);

	BurnYM2203Init(1, 1500000, NULL, 1);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_YM2203_ROUTE, 0.20, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_1, 0.23, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_2, 0.23, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_3, 0.23, BURN_SND_ROUTE_BOTH);

	MSM5205Init(0, CsilverMSM5205SynchroniseStream, 384000, CsilverADPCMInt, MSM5205_S48_4B, 1);
	MSM5205SetRoute(0, 0.88, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();

	CsilverDoReset();

	return 0;
}

static INT32 CsilverFrame()
{
	if (DrvReset) {
		CsilverDoReset();
	}

	M6809NewFrame();
	M6502NewFrame();
	if (realMCU) mcs51NewFrame();

	{
		memset (DrvInputs, 0xff, 5);
		for (INT32 i = 0; i < 8; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[2] ^= (DrvJoy3[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy4[i] & 1) << i;
			DrvInputs[4] ^= (DrvJoy5[i] & 1) << i;
		}
	}

	INT32 nInterleave = 256;
	MSM5205NewFrame(0, 1500000, nInterleave);
	INT32 nCyclesTotal[3] = { 1500000 / 58, 1500000 / 58, 1500000 / 58 };
	INT32 nCyclesDone[3] = { 0, 0, 0 };

	M6502Open(0);

	vblank = 0x80;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		if (i == 8) vblank = 0x00;

		M6809Open(0);
		CPU_RUN(0, M6809);
		CPU_RUN_TIMER(2); // M6502, but sound syncs to M6809
		M6809Close();

		M6809Open(1);
		CPU_RUN(1, M6809);
		if (i == 248) {
			vblank = 0x80;
			M6809SetIRQLine(0x20, CPU_IRQSTATUS_AUTO);
		}
		MSM5205UpdateScanline(i);
		if (realMCU) {
			DrvMCUSync();
		}
		M6809Close();
	}

	M6502Close();

	if (pBurnSoundOut) {
		BurnYM3526Update(pBurnSoundOut, nBurnSoundLen);
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		MSM5205Render(0, pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		BurnDrvRedraw();
	}

	return 0;
}

static INT32 CsilverScan(INT32 nAction, INT32 *pnMin)
{
	if (nAction & ACB_DRIVER_DATA) {
		M6809Scan(nAction);
		BurnYM3526Scan(nAction, pnMin);
		MSM5205Scan(nAction, pnMin);

		SCAN_VAR(MSM5205Next);
		SCAN_VAR(MSM5205Last);
		SCAN_VAR(Toggle);
		SCAN_VAR(SndRomBank);
		SCAN_VAR(RomBank);

		if (nAction & ACB_WRITE) {
			M6809Open(0);
			m6809_bankswitch(RomBank);
			M6809Close();

			M6502Open(0);
			csilver_sound_bank(SndRomBank);
			M6502Close();
		}
	}

	return DrvScan(nAction, pnMin);
}

// Captain Silver (World)

static struct BurnRomInfo csilverRomDesc[] = {
	{ "dx03-12.18d",	0x08000, 0x2d926e7c, 1 }, //  0 maincpu
	{ "dx01.12d",		0x10000, 0x570fb50c, 1 }, //  1
	{ "dx02.13d",		0x10000, 0x58625890, 1 }, //  2

	{ "dx04-1.19d",		0x10000, 0x29432691, 2 }, //  3 sub

	{ "dx05.3f",		0x10000, 0xeb32cf25, 3 }, //  4 audiocpu

	{ "dx00.3d",		0x08000, 0xf01ef985, 4 }, //  5 gfx1

	{ "dx14.15k",		0x10000, 0x80f07915, 5 }, //  6 gfx2
	{ "dx13.13k",		0x10000, 0xd32c02e7, 5 }, //  7
	{ "dx12.10k",		0x10000, 0xac78b76b, 5 }, //  8

	{ "dx06.5f",		0x10000, 0xb6fb208c, 6 }, //  9 gfx3
	{ "dx07.7f",		0x10000, 0xee3e1817, 6 }, // 10
	{ "dx08.8f",		0x10000, 0x705900fe, 6 }, // 11
	{ "dx09.10f",		0x10000, 0x3192571d, 6 }, // 12
	{ "dx10.12f",		0x10000, 0x3ef77a32, 6 }, // 13
	{ "dx11.13f",		0x10000, 0x9cf3d5b8, 6 }, // 14

	{ "dx-8.19a",		0x01000, 0xc0266263, 7 }, // 15 i8751 microcontroller

	// BPROM type MB7122E for priority (Not yet used), location on alternate board unknown
	{ "dx-15.b10",		0x00400, 0xdcbfec4e, 0 }, // 16 proms
};

STD_ROM_PICK(csilver)
STD_ROM_FN(csilver)

static INT32 CsilverExit()
{
	MSM5205Exit();

	MSM5205Next = 0;
	Toggle = 0;

	return LastmissExit();
}

struct BurnDriver BurnDrvCsilver = {
	"csilver", NULL, NULL, NULL, "1987",
	"Captain Silver (World)\0", NULL, "Data East Corporation", "DEC8",
	L"\u9521\u9C81\u5DF4\u8239\u957F (\u4E16\u754C\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, csilverRomInfo, csilverRomName, NULL, NULL, NULL, NULL, CsilverInputInfo, CsilverDIPInfo,
	CsilverInit, CsilverExit, CsilverFrame, LastmissDraw, CsilverScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};


// Captain Silver (Japan, revision 2)

static struct BurnRomInfo csilverjRomDesc[] = {
	{ "dx03-2.18d",			0x08000, 0x02dd8cfc, 1 }, //  0 maincpu
	{ "dx01.12d",			0x10000, 0x570fb50c, 1 }, //  1
	{ "dx02.13d",			0x10000, 0x58625890, 1 }, //  2

	{ "dx04-1.19d",			0x10000, 0x29432691, 2 }, //  3 sub

	{ "dx05.3f",			0x10000, 0xeb32cf25, 3 }, //  4 audiocpu

	{ "dx00.3d",			0x08000, 0xf01ef985, 4 }, //  5 gfx1

	{ "dx14.15k",			0x10000, 0x80f07915, 5 }, //  6 gfx2
	{ "dx13.13k",			0x10000, 0xd32c02e7, 5 }, //  7
	{ "dx12.10k",			0x10000, 0xac78b76b, 5 }, //  8

	{ "dx06.5f",			0x10000, 0xb6fb208c, 6 }, //  9 gfx3
	{ "dx07.7f",			0x10000, 0xee3e1817, 6 }, // 10
	{ "dx08.8f",			0x10000, 0x705900fe, 6 }, // 11
	{ "dx09.10f",			0x10000, 0x3192571d, 6 }, // 12
	{ "dx10.12f",			0x10000, 0x3ef77a32, 6 }, // 13
	{ "dx11.13f",			0x10000, 0x9cf3d5b8, 6 }, // 14

	{ "id8751h_japan.mcu",	0x01000, 0x6e801217, 7 }, // 15 i8751 microcontroller

	// BPROM type MB7122E for priority (Not yet used), location on alternate board unknown
	{ "dx-15.b10",			0x00400, 0xdcbfec4e, 0 }, // 16 proms
};

STD_ROM_PICK(csilverj)
STD_ROM_FN(csilverj)

struct BurnDriver BurnDrvCsilverj = {
	"csilverj", "csilver", NULL, NULL, "1987",
	"Captain Silver (Japan, revision 2)\0", NULL, "Data East Corporation", "DEC8",
	L"\u9521\u9C81\u5DF4\u8239\u957F (\u65E5\u7248 \u4FEE\u8BA2\u7248 3)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, csilverjRomInfo, csilverjRomName, NULL, NULL, NULL, NULL, CsilverInputInfo, CsilverDIPInfo,
	CsilverInit, CsilverExit, CsilverFrame, LastmissDraw, CsilverScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};


// Captain Silver (Japan, revision 1)
/* Same IC positions to World set */

static struct BurnRomInfo csilverjaRomDesc[] = {
	{ "dx03-1.18d",			0x08000, 0xd42905be, 1 }, //  0 maincpu
	{ "dx01.12d",			0x10000, 0x570fb50c, 1 }, //  1
	{ "dx02.13d",			0x10000, 0x58625890, 1 }, //  2

	{ "dx04-1.19d",			0x10000, 0x29432691, 2 }, //  3 sub

	{ "dx05.3f",			0x10000, 0xeb32cf25, 3 }, //  4 audiocpu

	{ "dx00.3d",			0x08000, 0xf01ef985, 4 }, //  5 gfx1

	{ "dx14.15k",			0x10000, 0x80f07915, 5 }, //  6 gfx2
	{ "dx13.13k",			0x10000, 0xd32c02e7, 5 }, //  7
	{ "dx12.10k",			0x10000, 0xac78b76b, 5 }, //  8

	{ "dx06.5f",			0x10000, 0xb6fb208c, 6 }, //  9 gfx3
	{ "dx07.7f",			0x10000, 0xee3e1817, 6 }, // 10
	{ "dx08.8f",			0x10000, 0x705900fe, 6 }, // 11
	{ "dx09.10f",			0x10000, 0x3192571d, 6 }, // 12
	{ "dx10.12f",			0x10000, 0x3ef77a32, 6 }, // 13
	{ "dx11.13f",			0x10000, 0x9cf3d5b8, 6 }, // 14

	{ "id8751h_japan.mcu",	0x01000, 0x6e801217, 7 }, // 15 i8751 microcontroller

	// BPROM type MB7122E for priority (Not yet used), location on alternate board unknown
	{ "dx-15.b10",			0x00400, 0xdcbfec4e, 0 }, // 16 proms
};

STD_ROM_PICK(csilverja)
STD_ROM_FN(csilverja)

struct BurnDriver BurnDrvCsilverja = {
	"csilverja", "csilver", NULL, NULL, "1987",
	"Captain Silver (Japan, revision 1)\0", NULL, "Data East Corporation", "DEC8",
	L"\u9521\u9C81\u5DF4\u8239\u957F (\u65E5\u7248 \u4FEE\u8BA2\u7248 1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, csilverjaRomInfo, csilverjaRomName, NULL, NULL, NULL, NULL, CsilverInputInfo, CsilverDIPInfo,
	CsilverInit, CsilverExit, CsilverFrame, LastmissDraw, CsilverScan, &DrvRecalc, 0x400,
	256, 240, 4, 3
};