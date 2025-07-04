// FB Alpha Rohga Armor Force / Wizard Fire / Nitro Ball / Schmeiser Robo driver module
// Based on MAME driver by Bryan McPhail

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "h6280_intf.h"
#include "deco16ic.h"
#include "deco146.h"
#include "msm6295.h"
#include "burn_ym2151.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68KROM;
static UINT8 *DrvHucROM;
static UINT8 *DrvGfxROM0;
static UINT8 *DrvGfxROM1;
static UINT8 *DrvGfxROM2;
static UINT8 *DrvGfxROM3;
static UINT8 *DrvGfxROM4;
static UINT8 *DrvSndROM0;
static UINT8 *DrvSndROM1;
static UINT8 *Drv68KRAM;
static UINT8 *DrvHucRAM;
static UINT8 *DrvPalRAM;
static UINT8 *DrvPalBuf;
static UINT8 *DrvSprRAM;
static UINT8 *DrvSprRAM2;
static UINT8 *DrvSprBuf;
static UINT8 *DrvSprBuf2;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 *flipscreen;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvJoy3[16];
static UINT8 DrvJoyFS[4]; // fake start button(s) for input symmetry
static UINT8 DrvDips[3];
static UINT8 DrvReset;
static UINT16 DrvInputs[4];

static UINT16 *tempdraw[2];

static INT32 DrvOkiBank;

static INT32 WizdfireEnglish = 0;
static INT32 DrvHangzo = 0;

static INT32 nCyclesExtra;

static struct BurnInputInfo RohgaInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy2 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},
	{"P1 Button 3",		BIT_DIGITAL,	DrvJoy1 + 6,	"p1 fire 3"	},

	{"P2 Coin",			BIT_DIGITAL,	DrvJoy2 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 15,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy1 + 8,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy1 + 9,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy1 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},
	{"P2 Button 3",		BIT_DIGITAL,	DrvJoy1 + 14,	"p2 fire 3"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Service",			BIT_DIGITAL,	DrvJoy2 + 2,	"service"	},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
	{"Dip C",			BIT_DIPSWITCH,	DrvDips + 2,	"dip"		},
};

STDINPUTINFO(Rohga)

static struct BurnInputInfo WizdfireInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy2 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",			BIT_DIGITAL,	DrvJoy2 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 15,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy1 + 8,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy1 + 9,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy1 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Service",			BIT_DIGITAL,	DrvJoy2 + 2,	"service"	},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Wizdfire)

static struct BurnInputInfo NitrobalInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	DrvJoy2 + 0,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	DrvJoy1 + 7,	"p1 start"	},
	{"P1 Up",			BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"	},
	{"P1 Button 1",		BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"	},
	{"P1 Button 2",		BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"	},

	{"P2 Coin",			BIT_DIGITAL,	DrvJoy2 + 1,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	DrvJoy1 + 15,	"p2 start"	},
	{"P2 Up",			BIT_DIGITAL,	DrvJoy1 + 8,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	DrvJoy1 + 9,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	DrvJoy1 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	DrvJoy1 + 11,	"p2 right"	},
	{"P2 Button 1",		BIT_DIGITAL,	DrvJoy1 + 12,	"p2 fire 1"	},
	{"P2 Button 2",		BIT_DIGITAL,	DrvJoy1 + 13,	"p2 fire 2"	},

	{"P3 Coin",			BIT_DIGITAL,	DrvJoy3 + 7,	"p3 coin"	},
	{"P3 Start",		BIT_DIGITAL,	DrvJoyFS + 0,	"p3 start"	}, // when game is set to 3p mode via dips, all start buttons are disabled.  This entry just added for symmetry to keep kaillera happy
	{"P3 Up",			BIT_DIGITAL,	DrvJoy3 + 0,	"p3 up"		},
	{"P3 Down",			BIT_DIGITAL,	DrvJoy3 + 1,	"p3 down"	},
	{"P3 Left",			BIT_DIGITAL,	DrvJoy3 + 2,	"p3 left"	},
	{"P3 Right",		BIT_DIGITAL,	DrvJoy3 + 3,	"p3 right"	},
	{"P3 Button 1",		BIT_DIGITAL,	DrvJoy3 + 4,	"p3 fire 1"	},
	{"P3 Button 2",		BIT_DIGITAL,	DrvJoy3 + 5,	"p3 fire 2"	},

	{"Reset",			BIT_DIGITAL,	&DrvReset,		"reset"		},
	{"Service",			BIT_DIGITAL,	DrvJoy2 + 2,	"service"	},
	{"Dip A",			BIT_DIPSWITCH,	DrvDips + 0,	"dip"		},
	{"Dip B",			BIT_DIPSWITCH,	DrvDips + 1,	"dip"		},
};

STDINPUTINFO(Nitrobal)

static struct BurnDIPInfo RohgaDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL							},
	{0x15, 0xff, 0xff, 0x7f, NULL							},
	{0x16, 0xff, 0xff, 0xff, NULL							},

	{0   , 0xfe, 0   ,    8, "Coin A"						},
	{0x14, 0x01, 0x07, 0x00, "3 Coins 1 Credits"			},
	{0x14, 0x01, 0x07, 0x01, "2 Coins 1 Credits"			},
	{0x14, 0x01, 0x07, 0x07, "1 Coin  1 Credits"			},
	{0x14, 0x01, 0x07, 0x06, "1 Coin  2 Credits"			},
	{0x14, 0x01, 0x07, 0x05, "1 Coin  3 Credits"			},
	{0x14, 0x01, 0x07, 0x04, "1 Coin  4 Credits"			},
	{0x14, 0x01, 0x07, 0x03, "1 Coin  5 Credits"			},
	{0x14, 0x01, 0x07, 0x02, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    8, "Coin B"						},
	{0x14, 0x01, 0x38, 0x00, "3 Coins 1 Credits"			},
	{0x14, 0x01, 0x38, 0x08, "2 Coins 1 Credits"			},
	{0x14, 0x01, 0x38, 0x38, "1 Coin  1 Credits"			},
	{0x14, 0x01, 0x38, 0x30, "1 Coin  2 Credits"			},
	{0x14, 0x01, 0x38, 0x28, "1 Coin  3 Credits"			},
	{0x14, 0x01, 0x38, 0x20, "1 Coin  4 Credits"			},
	{0x14, 0x01, 0x38, 0x18, "1 Coin  5 Credits"			},
	{0x14, 0x01, 0x38, 0x10, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"					},
	{0x14, 0x01, 0x40, 0x40, "Off"							},
	{0x14, 0x01, 0x40, 0x00, "On"							},

	{0   , 0xfe, 0   ,    2, "2 Credits Start, 1 Continue"	},
	{0x14, 0x01, 0x80, 0x80, "Off"							},
	{0x14, 0x01, 0x80, 0x00, "On"							},

	{0   , 0xfe, 0   ,    4, "Player's Vitality"			},
	{0x15, 0x01, 0x30, 0x30, "Normal"						},
	{0x15, 0x01, 0x30, 0x20, "Low"							},
	{0x15, 0x01, 0x30, 0x10, "Lowest"						},
	{0x15, 0x01, 0x30, 0x00, "High"							},

	{0   , 0xfe, 0   ,    2, "Allow Continue"				},
	{0x15, 0x01, 0x40, 0x40, "On"							},
	{0x15, 0x01, 0x40, 0x00, "Off"							},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"					},
	{0x15, 0x01, 0x80, 0x80, "Off"							},
	{0x15, 0x01, 0x80, 0x00, "On"							},

	{0   , 0xfe, 0   ,    2, "Stage Clear Bonus"			},
	{0x16, 0x01, 0x01, 0x01, "Off"							},
	{0x16, 0x01, 0x01, 0x00, "On"							},

	{0   , 0xfe, 0   ,    4, "Enemy's Vitality"				},
	{0x16, 0x01, 0x0c, 0x08, "Low"							},
	{0x16, 0x01, 0x0c, 0x0c, "Normal"						},
	{0x16, 0x01, 0x0c, 0x04, "High"							},
	{0x16, 0x01, 0x0c, 0x00, "Highest"						},

	{0   , 0xfe, 0   ,    4, "Enemy Encounter Rate"			},
	{0x16, 0x01, 0x30, 0x20, "Low"							},
	{0x16, 0x01, 0x30, 0x30, "Normal"						},
	{0x16, 0x01, 0x30, 0x10, "High"							},
	{0x16, 0x01, 0x30, 0x00, "Highest"						},

	{0   , 0xfe, 0   ,    4, "Enemy's Weapon Speed"			},
	{0x16, 0x01, 0xc0, 0x80, "Slow"							},
	{0x16, 0x01, 0xc0, 0xc0, "Normal"						},
	{0x16, 0x01, 0xc0, 0x40, "Fast"							},
	{0x16, 0x01, 0xc0, 0x00, "Fastest"						},
};

STDDIPINFO(Rohga)

static struct BurnDIPInfo SchmeisrDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL							},
	{0x15, 0xff, 0xff, 0xff, NULL							},
	{0x16, 0xff, 0xff, 0xff, NULL							},

	{0   , 0xfe, 0   ,    8, "Coin A"						},
	{0x14, 0x01, 0x07, 0x00, "3 Coins 1 Credits"			},
	{0x14, 0x01, 0x07, 0x01, "2 Coins 1 Credits"			},
	{0x14, 0x01, 0x07, 0x07, "1 Coin  1 Credits"			},
	{0x14, 0x01, 0x07, 0x06, "1 Coin  2 Credits"			},
	{0x14, 0x01, 0x07, 0x05, "1 Coin  3 Credits"			},
	{0x14, 0x01, 0x07, 0x04, "1 Coin  4 Credits"			},
	{0x14, 0x01, 0x07, 0x03, "1 Coin  5 Credits"			},
	{0x14, 0x01, 0x07, 0x02, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    8, "Coin B"						},
	{0x14, 0x01, 0x38, 0x00, "3 Coins 1 Credits"			},
	{0x14, 0x01, 0x38, 0x08, "2 Coins 1 Credits"			},
	{0x14, 0x01, 0x38, 0x38, "1 Coin  1 Credits"			},
	{0x14, 0x01, 0x38, 0x30, "1 Coin  2 Credits"			},
	{0x14, 0x01, 0x38, 0x28, "1 Coin  3 Credits"			},
	{0x14, 0x01, 0x38, 0x20, "1 Coin  4 Credits"			},
	{0x14, 0x01, 0x38, 0x18, "1 Coin  5 Credits"			},
	{0x14, 0x01, 0x38, 0x10, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"					},
	{0x14, 0x01, 0x40, 0x40, "Off"							},
	{0x14, 0x01, 0x40, 0x00, "On"							},

	{0   , 0xfe, 0   ,    2, "Service Mode"					},
	{0x14, 0x01, 0x80, 0x80, "Off"							},
	{0x14, 0x01, 0x80, 0x00, "On"							},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"					},
	{0x15, 0x01, 0x04, 0x00, "Off"							},
	{0x15, 0x01, 0x04, 0x04, "On"							},

	{0   , 0xfe, 0   ,    2, "Freeze Screen"				},
	{0x16, 0x01, 0x20, 0x20, "Off"							},
	{0x16, 0x01, 0x20, 0x00, "On"							},
};

STDDIPINFO(Schmeisr)

static struct BurnDIPInfo WizdfireDIPList[]=
{
	{0x12, 0xff, 0xff, 0xff, NULL							},
	{0x13, 0xff, 0xff, 0x7f, NULL							},

	{0   , 0xfe, 0   ,    8, "Coin A"						},
	{0x12, 0x01, 0x07, 0x00, "3 Coins 1 Credits"			},
	{0x12, 0x01, 0x07, 0x01, "2 Coins 1 Credits"			},
	{0x12, 0x01, 0x07, 0x07, "1 Coin  1 Credits"			},
	{0x12, 0x01, 0x07, 0x06, "1 Coin  2 Credits"			},
	{0x12, 0x01, 0x07, 0x05, "1 Coin  3 Credits"			},
	{0x12, 0x01, 0x07, 0x04, "1 Coin  4 Credits"			},
	{0x12, 0x01, 0x07, 0x03, "1 Coin  5 Credits"			},
	{0x12, 0x01, 0x07, 0x02, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    8, "Coin B"						},
	{0x12, 0x01, 0x38, 0x00, "3 Coins 1 Credits"			},
	{0x12, 0x01, 0x38, 0x08, "2 Coins 1 Credits"			},
	{0x12, 0x01, 0x38, 0x38, "1 Coin  1 Credits"			},
	{0x12, 0x01, 0x38, 0x30, "1 Coin  2 Credits"			},
	{0x12, 0x01, 0x38, 0x28, "1 Coin  3 Credits"			},
	{0x12, 0x01, 0x38, 0x20, "1 Coin  4 Credits"			},
	{0x12, 0x01, 0x38, 0x18, "1 Coin  5 Credits"			},
	{0x12, 0x01, 0x38, 0x10, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"					},
	{0x12, 0x01, 0x40, 0x40, "Off"							},
	{0x12, 0x01, 0x40, 0x00, "On"							},

	{0   , 0xfe, 0   ,    2, "2 Credits Start, 1 Continue"	},
	{0x12, 0x01, 0x80, 0x80, "Off"							},
	{0x12, 0x01, 0x80, 0x00, "On"							},

	{0   , 0xfe, 0   ,    4, "Lives"						},
	{0x13, 0x01, 0x03, 0x00, "2"							},
	{0x13, 0x01, 0x03, 0x01, "3"							},
	{0x13, 0x01, 0x03, 0x03, "4"							},
	{0x13, 0x01, 0x03, 0x02, "5"							},

	{0   , 0xfe, 0   ,    4, "Difficulty"					},
	{0x13, 0x01, 0x0c, 0x08, "Easy"							},
	{0x13, 0x01, 0x0c, 0x0c, "Normal"						},
	{0x13, 0x01, 0x0c, 0x04, "Hard"							},
	{0x13, 0x01, 0x0c, 0x00, "Hardest"						},

	{0   , 0xfe, 0   ,    4, "Magic Gauge Speed"			},
	{0x13, 0x01, 0x30, 0x00, "Very Slow"					},
	{0x13, 0x01, 0x30, 0x10, "Slow"							},
	{0x13, 0x01, 0x30, 0x30, "Normal"						},
	{0x13, 0x01, 0x30, 0x20, "Fast"							},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"					},
	{0x13, 0x01, 0x80, 0x80, "Off"							},
	{0x13, 0x01, 0x80, 0x00, "On"							},
};

STDDIPINFO(Wizdfire)

static struct BurnDIPInfo NitrobalDIPList[]=
{
	DIP_OFFSET(0x1a)
	{0x00, 0xff, 0xff, 0xff, NULL							},
	{0x01, 0xff, 0xff, 0x7f, NULL							},

	{0   , 0xfe, 0   ,    8, "Coin A"						},
	{0x00, 0x01, 0x07, 0x00, "3 Coins 1 Credits"			},
	{0x00, 0x01, 0x07, 0x01, "2 Coins 1 Credits"			},
	{0x00, 0x01, 0x07, 0x07, "1 Coin  1 Credits"			},
	{0x00, 0x01, 0x07, 0x06, "1 Coin  2 Credits"			},
	{0x00, 0x01, 0x07, 0x05, "1 Coin  3 Credits"			},
	{0x00, 0x01, 0x07, 0x04, "1 Coin  4 Credits"			},
	{0x00, 0x01, 0x07, 0x03, "1 Coin  5 Credits"			},
	{0x00, 0x01, 0x07, 0x02, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    8, "Coin B"						},
	{0x00, 0x01, 0x38, 0x00, "3 Coins 1 Credits"			},
	{0x00, 0x01, 0x38, 0x08, "2 Coins 1 Credits"			},
	{0x00, 0x01, 0x38, 0x38, "1 Coin  1 Credits"			},
	{0x00, 0x01, 0x38, 0x30, "1 Coin  2 Credits"			},
	{0x00, 0x01, 0x38, 0x28, "1 Coin  3 Credits"			},
	{0x00, 0x01, 0x38, 0x20, "1 Coin  4 Credits"			},
	{0x00, 0x01, 0x38, 0x18, "1 Coin  5 Credits"			},
	{0x00, 0x01, 0x38, 0x10, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"					},
	{0x00, 0x01, 0x40, 0x40, "Off"							},
	{0x00, 0x01, 0x40, 0x00, "On"							},

	{0   , 0xfe, 0   ,    2, "2 Credits Start, 1 Continue"	},
	{0x00, 0x01, 0x80, 0x80, "Off"							},
	{0x00, 0x01, 0x80, 0x00, "On"							},

	{0   , 0xfe, 0   ,    4, "Lives"						},
	{0x01, 0x01, 0x03, 0x01, "1"							},
	{0x01, 0x01, 0x03, 0x00, "2"							},
	{0x01, 0x01, 0x03, 0x03, "3"							},
	{0x01, 0x01, 0x03, 0x02, "4"							},

	{0   , 0xfe, 0   ,    4, "Difficulty?"					},
	{0x01, 0x01, 0x0c, 0x08, "Easy"							},
	{0x01, 0x01, 0x0c, 0x0c, "Normal"						},
	{0x01, 0x01, 0x0c, 0x04, "Hard"							},
	{0x01, 0x01, 0x0c, 0x00, "Hardest"						},

	{0   , 0xfe, 0   ,    2, "Split Coin Chutes"			},
	{0x01, 0x01, 0x10, 0x10, "Off"							},
	{0x01, 0x01, 0x10, 0x00, "On"							},

	{0   , 0xfe, 0   ,    2, "Players"						},
	{0x01, 0x01, 0x20, 0x20, "2"							},
	{0x01, 0x01, 0x20, 0x00, "3"							},

	{0   , 0xfe, 0   ,    2, "Shot Button to Start"			},
	{0x01, 0x01, 0x40, 0x40, "Off"							},
	{0x01, 0x01, 0x40, 0x00, "On"							},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"					},
	{0x01, 0x01, 0x80, 0x80, "Off"							},
	{0x01, 0x01, 0x80, 0x00, "On"							},
};

STDDIPINFO(Nitrobal)

static struct BurnDIPInfo HangzoDIPList[]=
{
	{0x14, 0xff, 0xff, 0xff, NULL							},
	{0x15, 0xff, 0xff, 0x7f, NULL							},
	{0x16, 0xff, 0xff, 0xff, NULL							},

	{0   , 0xfe, 0   ,    8, "Coin A"						},
	{0x14, 0x01, 0x07, 0x00, "3 Coins 1 Credits"			},
	{0x14, 0x01, 0x07, 0x01, "2 Coins 1 Credits"			},
	{0x14, 0x01, 0x07, 0x07, "1 Coin  1 Credits"			},
	{0x14, 0x01, 0x07, 0x06, "1 Coin  2 Credits"			},
	{0x14, 0x01, 0x07, 0x05, "1 Coin  3 Credits"			},
	{0x14, 0x01, 0x07, 0x04, "1 Coin  4 Credits"			},
	{0x14, 0x01, 0x07, 0x03, "1 Coin  5 Credits"			},
	{0x14, 0x01, 0x07, 0x02, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    8, "Coin B"						},
	{0x14, 0x01, 0x38, 0x00, "3 Coins 1 Credits"			},
	{0x14, 0x01, 0x38, 0x08, "2 Coins 1 Credits"			},
	{0x14, 0x01, 0x38, 0x38, "1 Coin  1 Credits"			},
	{0x14, 0x01, 0x38, 0x30, "1 Coin  2 Credits"			},
	{0x14, 0x01, 0x38, 0x28, "1 Coin  3 Credits"			},
	{0x14, 0x01, 0x38, 0x20, "1 Coin  4 Credits"			},
	{0x14, 0x01, 0x38, 0x18, "1 Coin  5 Credits"			},
	{0x14, 0x01, 0x38, 0x10, "1 Coin  6 Credits"			},

	{0   , 0xfe, 0   ,    2, "Flip Screen"					},
	{0x14, 0x01, 0x40, 0x40, "Off"							},
	{0x14, 0x01, 0x40, 0x00, "On"							},

	{0   , 0xfe, 0   ,    2, "2 Credits Start, 1 Continue"	},
	{0x14, 0x01, 0x80, 0x80, "Off"							},
	{0x14, 0x01, 0x80, 0x00, "On"							},

	{0   , 0xfe, 0   ,    4, "Lives"						},
	{0x15, 0x01, 0x03, 0x01, "1"							},
	{0x15, 0x01, 0x03, 0x00, "2"							},
	{0x15, 0x01, 0x03, 0x03, "3"							},
	{0x15, 0x01, 0x03, 0x02, "4"							},

	{0   , 0xfe, 0   ,    2, "Allow Continue"				},
	{0x15, 0x01, 0x40, 0x00, "Off"							},
	{0x15, 0x01, 0x40, 0x40, "On"							},

	{0   , 0xfe, 0   ,    2, "Demo Sounds"					},
	{0x15, 0x01, 0x80, 0x80, "Off"							},
	{0x15, 0x01, 0x80, 0x00, "On"							},

	{0   , 0xfe, 0   ,    2, "Freeze Player"				},
	{0x16, 0x01, 0x02, 0x02, "Off"							},
	{0x16, 0x01, 0x02, 0x00, "On"							},

	{0   , 0xfe, 0   ,    2, "Freeze Frame"					},
	{0x16, 0x01, 0x04, 0x04, "Off"							},
	{0x16, 0x01, 0x04, 0x00, "On"							},

	{0   , 0xfe, 0   ,    2, "Freeze Screen"				},
	{0x16, 0x01, 0x20, 0x20, "Off"							},
	{0x16, 0x01, 0x20, 0x00, "On"							},

	{0   , 0xfe, 0   ,    2, "Debug Mode"					},
	{0x16, 0x01, 0x80, 0x80, "Off"							},
	{0x16, 0x01, 0x80, 0x00, "On"							},
};

STDDIPINFO(Hangzo)

static void __fastcall rohga_main_write_word(UINT32 address, UINT16 data)
{
	deco16_write_control_word(0, address, 0x200000, data)
	deco16_write_control_word(1, address, 0x240000, data)

	switch (address)
	{
		case 0x300000:
			memcpy (DrvSprBuf, DrvSprRAM, 0x800);
		return;

		case 0x31000a:
			memcpy (DrvPalBuf, DrvPalRAM, 0x2000);
		return;

		case 0x321100: // schmeisr
			SekSetIRQLine(6, CPU_IRQSTATUS_NONE);
		return;

		case 0x322000:
			deco16_priority = data;
		return;
	}

	if (address >= 0x280000 && address <= 0x283fff) {
		deco146_104_prot_ww(0, address, data);
		return;
	}
}

static void __fastcall rohga_main_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x300000:
		case 0x300001:
			memcpy (DrvSprBuf, DrvSprRAM, 0x800);
		return;

		case 0x31000a:
		case 0x31000b:
			memcpy (DrvPalBuf, DrvPalRAM, 0x2000);
		return;

		case 0x321100: // schmeisr
		case 0x321101:
			SekSetIRQLine(6, CPU_IRQSTATUS_NONE);
		return;

		case 0x322000:
		case 0x322001:
			deco16_priority = data;
		return;
	}

	if (address >= 0x280000 && address <= 0x283fff) {
		deco146_104_prot_wb(0, address, data);
		return;
	}
}

static UINT16 __fastcall rohga_main_read_word(UINT32 address)
{
	switch (address)
	{
		case 0x2c0000:
		case 0x300000: // schmeisr
			return DrvDips[2];

		case 0x310002: // schmeisr
			return (DrvInputs[1] & 0x07) | (deco16_vblank & 0x08);

		case 0x321100:
			SekSetIRQLine(6, CPU_IRQSTATUS_NONE);
			return 0;
	}

	if (address >= 0x280000 && address <= 0x283fff) {
		return deco146_104_prot_rw(0, address);
	}

	return 0;
}

static UINT8 __fastcall rohga_main_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0x2c0000:
		case 0x2c0001:
		case 0x300000: // schmeisr
		case 0x300001:
			return DrvDips[2];

		case 0x310002: // schmeisr
		case 0x310003:
			return (DrvInputs[1] & 0x07) | (deco16_vblank & 0x08);

		case 0x321100:
		case 0x321101:
			SekSetIRQLine(6, CPU_IRQSTATUS_NONE);
			return 0;
	}

	if (address >= 0x280000 && address <= 0x283fff) {
		return deco146_104_prot_rb(0, address);
	}

	return 0;
}

static void __fastcall wizdfire_main_write_word(UINT32 address, UINT16 data)
{
	deco16_write_control_word(0, address, 0x300000, data)
	deco16_write_control_word(1, address, 0x310000, data)

	switch (address)
	{
		case 0x350000:
			memcpy (DrvSprBuf,  DrvSprRAM, 0x800);
		return;

		case 0x370000:
			memcpy (DrvSprBuf2, DrvSprRAM2, 0x800);
		return;

		case 0x390008:
			memcpy (DrvPalBuf, DrvPalRAM, 0x2000);
		return;

		case 0x320000:
			deco16_priority = data;
		return;

		case 0x320004:
			SekSetIRQLine(6, CPU_IRQSTATUS_NONE);
		return;
	}

	if ((address >= 0xff4000 && address <= 0xff7fff) || // wizdfire
	    (address >= 0xfe4000 && address <= 0xfe7fff)) { // nitrobal
		deco146_104_prot_ww(0, address, data);
		return;
	}
}

static void __fastcall wizdfire_main_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0x350000:
		case 0x350001:
			memcpy (DrvSprBuf,  DrvSprRAM, 0x800);
		return;

		case 0x370000:
		case 0x370001:
			memcpy (DrvSprBuf2, DrvSprRAM2, 0x800);
		return;

		case 0x390008:
		case 0x390009:
			memcpy (DrvPalBuf, DrvPalRAM, 0x2000);
		return;

		case 0x320000:
		case 0x320001:
			deco16_priority = data;
		return;


		case 0x320004:
		case 0x320005:
			SekSetIRQLine(6, CPU_IRQSTATUS_NONE);
		return;
	}

	if ((address >= 0xff4000 && address <= 0xff7fff) || // wizdfire
	    (address >= 0xfe4000 && address <= 0xfe7fff)) { // nitrobal
		deco146_104_prot_wb(0, address, data);
		return;
	}
}

static UINT16 __fastcall wizdfire_main_read_word(UINT32 address)
{
	if (address == 0x320000) return DrvInputs[3];

	if ((address >= 0xff4000 && address <= 0xff7fff) || // wizdfire
	    (address >= 0xfe4000 && address <= 0xfe7fff)) { // nitrobal
		return deco146_104_prot_rw(0, address);
	}

	return 0;
}

static UINT8 __fastcall wizdfire_main_read_byte(UINT32 address)
{
	if (address == 0x320000 || address == 0x320001) return DrvInputs[3] >> ((~address & 1) << 3);

	if ((address >= 0xff4000 && address <= 0xff7fff) || // wizdfire
	    (address >= 0xfe4000 && address <= 0xfe7fff)) { // nitrobal
		return deco146_104_prot_rb(0, address);
	}

	return 0;
}

static void DrvYM2151WritePort(UINT32, UINT32 data)
{
	// WizdfireEnglish selects the second 0x80000 part of DrvSndROM0
	MSM6295SetBank(0, DrvSndROM0 + WizdfireEnglish + (data & 1) * 0x40000, 0, 0x3ffff);
	MSM6295SetBank(1, DrvSndROM1 + ((data & 0x02) >> 1) * 0x40000, 0, 0x3ffff);

	DrvOkiBank = data;
}

static INT32 rohga_bank_callback( const INT32 bank )
{
	return ((bank >> 4) & 0x3) << 12;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	SekOpen(0);
	SekReset();
	SekClose();

	deco16SoundReset();

	deco16Reset();

	DrvYM2151WritePort(0, (DrvHangzo) ? 0 : 3);

	nCyclesExtra = 0;

	HiscoreReset();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KROM	= Next; Next += 0x200000;
	DrvHucROM	= Next; Next += 0x010000;

	DrvGfxROM0	= Next; Next += 0x080000;
	DrvGfxROM1	= Next; Next += 0x400000;
	DrvGfxROM2	= Next; Next += 0x400000;
	DrvGfxROM3	= Next; Next += 0x800000;
	DrvGfxROM4	= Next; Next += 0x800000;

	MSM6295ROM	= Next;
	DrvSndROM0	= Next; Next += 0x100000;
	DrvSndROM1	= Next; Next += 0x100000;

	tempdraw[0]	= (UINT16*)Next; Next += 320 * 240 * sizeof(UINT16);
	tempdraw[1]	= (UINT16*)Next; Next += 320 * 240 * sizeof(UINT16);

	DrvPalette	= (UINT32*)Next; Next += 0x0800 * sizeof(UINT32);

	AllRam		= Next;

	Drv68KRAM	= Next; Next += 0x024000;
	DrvHucRAM	= Next; Next += 0x002000;
	DrvSprRAM2	= Next; Next += 0x000800;
	DrvSprRAM	= Next; Next += 0x000800;
	DrvSprBuf2	= Next; Next += 0x000800;
	DrvSprBuf	= Next; Next += 0x000800;

	DrvPalRAM	= Next; Next += 0x002000;
	DrvPalBuf	= Next; Next += 0x002000;

	flipscreen	= Next; Next += 0x000001;

	RamEnd		= Next;

	MemEnd		= Next;

	return 0;
}

static INT32 DrvSpriteDecode()
{
	INT32 Plane[6] = { 0x400000*8+8, 0x400000*8, 0x200000*8+8, 0x200000*8, 8, 0 };
	INT32 XOffs[16] = { 7,6,5,4,3,2,1,0, 32*8+7, 32*8+6, 32*8+5, 32*8+4, 32*8+3, 32*8+2, 32*8+1, 32*8+0 };
	INT32 YOffs[16] = { 15*16, 14*16, 13*16, 12*16, 11*16, 10*16, 9*16, 8*16,	7*16, 6*16, 5*16, 4*16, 3*16, 2*16, 1*16, 0*16};

	UINT8 *tmp = (UINT8*)BurnMalloc(0x600000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, DrvGfxROM3, 0x600000);

	GfxDecode(0x8000, 6, 16, 16, Plane, XOffs, YOffs, 0x200, tmp, DrvGfxROM3);

	BurnFree (tmp);

	return 0;
}

static UINT16 deco_104_port_a_cb()
{
	return DrvInputs[0];
}

static UINT16 deco_104_port_b_cb()
{
	return (DrvInputs[1] & ~8) | deco16_vblank;
}

static UINT16 deco_104_port_c_cb()
{
	return DrvInputs[2];
}

static void soundlatch_write(UINT16 data)
{
	deco16_soundlatch = data & 0xff;
	h6280SetIRQLine(0, CPU_IRQSTATUS_ACK);
}

static INT32 RohgaInit()
{
	BurnSetRefreshRate(58.00);

	BurnAllocMemIndex();

	{
		if (BurnLoadRom(Drv68KROM  + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x000000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x100001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x100000,  3, 2)) return 1;

		if (BurnLoadRom(DrvHucROM  + 0x000000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  5, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x000001,  6, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x080000,  8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x000000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x100000, 10, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x100000, 12, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x200000, 13, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x300000, 14, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x400000, 15, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x500000, 16, 1)) return 1;

		if (BurnLoadRom(DrvSndROM0 + 0x000000, 17, 1)) return 1;
		if (BurnLoadRom(DrvSndROM1 + 0x000000, 18, 1)) return 1;

		deco56_decrypt_gfx(DrvGfxROM0, 0x020000);
		deco56_decrypt_gfx(DrvGfxROM1, 0x100000);

		deco16_tile_decode(DrvGfxROM0, DrvGfxROM0, 0x020000, 1);
		deco16_tile_decode(DrvGfxROM1, DrvGfxROM1, 0x100000, 0);
		deco16_tile_decode(DrvGfxROM2, DrvGfxROM2, 0x200000, 0);
		DrvSpriteDecode();
	}	

	deco16Init(0, 0, 1|4);
	deco16_set_graphics(DrvGfxROM0, 0x20000 * 2, DrvGfxROM1, 0x100000 * 2, DrvGfxROM2, 0x200000 * 2);
	deco16_set_color_base(2, 512);
	deco16_set_color_base(3, 768);
	deco16_set_global_offsets(0, 8);
	deco16_set_bank_callback(0, rohga_bank_callback);
	deco16_set_bank_callback(1, rohga_bank_callback);
	deco16_set_bank_callback(2, rohga_bank_callback);
	deco16_set_bank_callback(3, rohga_bank_callback);

	// 146_104 prot
	deco_104_init();
	deco_146_104_set_port_a_cb(deco_104_port_a_cb);
	deco_146_104_set_port_b_cb(deco_104_port_b_cb);
	deco_146_104_set_port_c_cb(deco_104_port_c_cb);
	deco_146_104_set_soundlatch_cb(soundlatch_write);

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x1fffff, MAP_ROM);
	SekMapMemory(deco16_pf_ram[0],		0x3c0000, 0x3c1fff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[1],		0x3c2000, 0x3c2fff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[2],		0x3c4000, 0x3c4fff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[3],		0x3c6000, 0x3c6fff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[0],	0x3c8000, 0x3c9fff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],	0x3ca000, 0x3cafff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],	0x3cb000, 0x3cbfff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[2],	0x3cc000, 0x3ccfff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[2],	0x3cd000, 0x3cdfff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[3],	0x3ce000, 0x3cefff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[3],	0x3cf000, 0x3cffff, MAP_RAM);
	SekMapMemory(DrvSprRAM,			0x3d0000, 0x3d07ff, MAP_RAM);
	SekMapMemory(DrvPalRAM,			0x3e0000, 0x3e1fff, MAP_RAM);
	SekMapMemory(Drv68KRAM,			0x3f0000, 0x3f3fff, MAP_RAM);
	SekSetWriteWordHandler(0,		rohga_main_write_word);
	SekSetWriteByteHandler(0,		rohga_main_write_byte);
	SekSetReadWordHandler(0,		rohga_main_read_word);
	SekSetReadByteHandler(0,		rohga_main_read_byte);
	SekClose();

	deco16SoundInit(DrvHucROM, DrvHucRAM, 2685000, 0, DrvYM2151WritePort, 0.48, 1006875, 0.70, 2013750, 0.30);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.48, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.48, BURN_SND_ROUTE_RIGHT);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 WizdfireInit()
{
	BurnSetRefreshRate(58.00);

	BurnAllocMemIndex();

	{
		if (BurnLoadRom(Drv68KROM  + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x000000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x040001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x040000,  3, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x080001,  4, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x080000,  5, 2)) return 1;

		if (BurnLoadRom(DrvHucROM  + 0x000000,  6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  7, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x000001,  8, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x100000, 10, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x000000, 11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x080000, 12, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 13, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x000001, 14, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x200000, 15, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x200001, 16, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM4 + 0x000000, 17, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x000001, 18, 2)) return 1;

		if (BurnLoadRom(DrvSndROM0 + 0x000000, 19, 1)) return 1;
		if (BurnLoadRom(DrvSndROM1 + 0x000000, 20, 1)) return 1;

		deco74_decrypt_gfx(DrvGfxROM0, 0x020000);
		deco74_decrypt_gfx(DrvGfxROM1, 0x200000);
		deco74_decrypt_gfx(DrvGfxROM2, 0x100000);

		deco16_tile_decode(DrvGfxROM0, DrvGfxROM0, 0x020000, 1);
		deco16_tile_decode(DrvGfxROM1, DrvGfxROM1, 0x200000, 0);
		deco16_tile_decode(DrvGfxROM2, DrvGfxROM2, 0x100000, 0);
		deco16_sprite_decode(DrvGfxROM3, 0x400000);
		deco16_sprite_decode(DrvGfxROM4, 0x100000);
	}	

	deco16Init(0, 0, 1);
	deco16_set_graphics(DrvGfxROM0, 0x20000 * 2, DrvGfxROM1, 0x200000 * 2, DrvGfxROM2, 0x100000 * 2);
	deco16_set_color_base(2, 512);
	deco16_set_color_base(3, 768);
	deco16_set_global_offsets(0, 8);
	deco16_set_bank_callback(0, rohga_bank_callback);
	deco16_set_bank_callback(1, rohga_bank_callback);
	deco16_set_bank_callback(2, rohga_bank_callback);
	deco16_set_bank_callback(3, rohga_bank_callback);

	// 146_104 prot
	deco_104_init();
	deco_146_104_set_interface_scramble_reverse();
	deco_146_104_set_port_a_cb(deco_104_port_a_cb);
	deco_146_104_set_port_b_cb(deco_104_port_b_cb);
	deco_146_104_set_port_c_cb(deco_104_port_c_cb);
	deco_146_104_set_soundlatch_cb(soundlatch_write);

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x1fffff, MAP_ROM);
	SekMapMemory(deco16_pf_ram[0],		0x200000, 0x200fff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[1],		0x202000, 0x202fff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[2],		0x208000, 0x208fff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[3],		0x20a000, 0x20afff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[2],	0x20c000, 0x20c7ff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[3],	0x20e000, 0x20e7ff, MAP_RAM);
	SekMapMemory(DrvSprRAM,			0x340000, 0x3407ff, MAP_RAM);
	SekMapMemory(DrvSprRAM2,		0x360000, 0x3607ff, MAP_RAM);
	SekMapMemory(DrvPalRAM,			0x380000, 0x381fff, MAP_RAM);
	SekMapMemory(Drv68KRAM,			0xfdc000, 0xfe3fff, MAP_RAM);
	SekMapMemory(Drv68KRAM + 0x8000,	0xfe4800, 0xffffff, MAP_RAM);
	SekSetWriteWordHandler(0,		wizdfire_main_write_word);
	SekSetWriteByteHandler(0,		wizdfire_main_write_byte);
	SekSetReadWordHandler(0,		wizdfire_main_read_word);
	SekSetReadByteHandler(0,		wizdfire_main_read_byte);
	SekClose();

	deco16SoundInit(DrvHucROM, DrvHucRAM, 2685000, 0, DrvYM2151WritePort, 0.80, 1006875, 2.00, 2013750, 0.40);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.80, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.80, BURN_SND_ROUTE_RIGHT);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 WizdfireEnglishInit()
{
	WizdfireEnglish = 0x80000;

	return WizdfireInit();
}

static INT32 SchmeisrInit()
{
	BurnSetRefreshRate(58.00);

	BurnAllocMemIndex();

	{
		if (BurnLoadRom(Drv68KROM  + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x000000,  1, 2)) return 1;

		if (BurnLoadRom(DrvHucROM  + 0x000000,  2, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  3, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x080000,  4, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x000000,  5, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x100000,  6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3 + 0x000000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x100000,  8, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x200000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x300000, 10, 1)) return 1;

		if (BurnLoadRom(DrvSndROM0 + 0x000000, 11, 1)) return 1;
		if (BurnLoadRom(DrvSndROM1 + 0x000000, 12, 1)) return 1;

		deco74_decrypt_gfx(DrvGfxROM1, 0x100000);

		memcpy (DrvGfxROM0 + 0x000000, DrvGfxROM1 + 0x000000, 0x020000);
		memcpy (DrvGfxROM0 + 0x020000, DrvGfxROM1 + 0x080000, 0x020000);

		deco16_tile_decode(DrvGfxROM0, DrvGfxROM0, 0x040000, 1);
		deco16_tile_decode(DrvGfxROM1, DrvGfxROM1, 0x100000, 0);
		deco16_tile_decode(DrvGfxROM2, DrvGfxROM2, 0x200000, 0);
		DrvSpriteDecode();
	}	

	deco16Init(0, 0, 1);
	deco16_set_graphics(DrvGfxROM0, 0x40000 * 2, DrvGfxROM1, 0x100000 * 2, DrvGfxROM2, 0x200000 * 2);
	deco16_set_color_base(2, 512);
	deco16_set_color_base(3, 768);
	deco16_set_global_offsets(0, 8);
	deco16_set_bank_callback(0, rohga_bank_callback);
	deco16_set_bank_callback(1, rohga_bank_callback);
	deco16_set_bank_callback(2, rohga_bank_callback);
	deco16_set_bank_callback(3, rohga_bank_callback);

	// 146_104 prot
	deco_104_init();
	deco_146_104_set_port_a_cb(deco_104_port_a_cb);
	deco_146_104_set_port_b_cb(deco_104_port_b_cb);
	deco_146_104_set_port_c_cb(deco_104_port_c_cb);
	deco_146_104_set_soundlatch_cb(soundlatch_write);

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x1fffff, MAP_ROM);
	SekMapMemory(deco16_pf_ram[0],		0x3c0000, 0x3c1fff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[1],		0x3c2000, 0x3c2fff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[2],		0x3c4000, 0x3c4fff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[3],		0x3c6000, 0x3c6fff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[0],	0x3c8000, 0x3c8fff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[0],	0x3c9000, 0x3c9fff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],	0x3ca000, 0x3cafff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],	0x3cb000, 0x3cbfff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[2],	0x3cc000, 0x3ccfff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[2],	0x3cd000, 0x3cdfff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[3],	0x3ce000, 0x3cefff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[3],	0x3cf000, 0x3cffff, MAP_RAM);
	SekMapMemory(DrvSprRAM,			0x3d0000, 0x3d07ff, MAP_RAM);
	SekMapMemory(DrvPalRAM,			0x3e0000, 0x3e1fff, MAP_RAM);
	SekMapMemory(DrvPalRAM,			0x3e2000, 0x3e3fff, MAP_RAM);
	SekMapMemory(Drv68KRAM,			0xff0000, 0xff7fff, MAP_RAM);
	SekSetWriteWordHandler(0,		rohga_main_write_word);
	SekSetWriteByteHandler(0,		rohga_main_write_byte);
	SekSetReadWordHandler(0,		rohga_main_read_word);
	SekSetReadByteHandler(0,		rohga_main_read_byte);
	SekClose();

	deco16SoundInit(DrvHucROM, DrvHucRAM, 2685000, 0, DrvYM2151WritePort, 0.80, 1006875, 1.00, 2013750, 0.40);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.80, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.80, BURN_SND_ROUTE_RIGHT);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 HangzoInit()
{
	BurnSetRefreshRate(58.00);

	BurnAllocMemIndex();

	{
		if (BurnLoadRom(Drv68KROM  + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x000000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x040001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x040000,  3, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x080001,  4, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x080000,  5, 2)) return 1;

		if (BurnLoadRom(DrvHucROM  + 0x000000,  6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  7, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x080000,  8, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x000000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x100000, 10, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x100000, 12, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x200000, 13, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x300000, 14, 1)) return 1;

		if (BurnLoadRom(DrvSndROM0 + 0x000000, 15, 1)) return 1;
		if (BurnLoadRom(DrvSndROM1 + 0x000000, 16, 1)) return 1;

		memcpy (DrvGfxROM0 + 0x000000, DrvGfxROM1 + 0x000000, 0x020000);
		memcpy (DrvGfxROM0 + 0x020000, DrvGfxROM1 + 0x080000, 0x020000);

		deco16_tile_decode(DrvGfxROM0, DrvGfxROM0, 0x040000, 1);
		deco16_tile_decode(DrvGfxROM1, DrvGfxROM1, 0x100000, 0);
		deco16_tile_decode(DrvGfxROM2, DrvGfxROM2, 0x200000, 0);
		DrvSpriteDecode();
	}

	deco16Init(0, 0, 1|4); // full width & height
	deco16_set_graphics(DrvGfxROM0, 0x40000 * 2, DrvGfxROM1, 0x100000 * 2, DrvGfxROM2, 0x200000 * 2);
	deco16_set_color_base(2, 512);
	deco16_set_color_base(3, 768);
	deco16_set_global_offsets(0, 8);
	deco16_set_bank_callback(0, rohga_bank_callback);
	deco16_set_bank_callback(1, rohga_bank_callback);
	deco16_set_bank_callback(2, rohga_bank_callback);
	deco16_set_bank_callback(3, rohga_bank_callback);

	// 146_104 prot
	deco_104_init();
	deco_146_104_set_port_a_cb(deco_104_port_a_cb);
	deco_146_104_set_port_b_cb(deco_104_port_b_cb);
	deco_146_104_set_port_c_cb(deco_104_port_c_cb);
	deco_146_104_set_soundlatch_cb(soundlatch_write);

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x1fffff, MAP_ROM);
	SekMapMemory(deco16_pf_ram[0],		0x3c0000, 0x3c1fff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[1],		0x3c2000, 0x3c2fff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[2],		0x3c4000, 0x3c4fff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[3],		0x3c6000, 0x3c6fff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[0],	0x3c8000, 0x3c9fff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],	0x3ca000, 0x3cafff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],	0x3cb000, 0x3cbfff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[2],	0x3cc000, 0x3ccfff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[2],	0x3cd000, 0x3cdfff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[3],	0x3ce000, 0x3cefff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[3],	0x3cf000, 0x3cffff, MAP_RAM);
	SekMapMemory(DrvSprRAM,			0x3d0000, 0x3d07ff, MAP_RAM);
	SekMapMemory(DrvPalRAM,			0x3e0000, 0x3e1fff, MAP_RAM);
	SekMapMemory(DrvPalRAM,			0x3e2000, 0x3e3fff, MAP_RAM);
	SekMapMemory(Drv68KRAM,			0x3f0000, 0x3f3fff, MAP_RAM);
	SekSetWriteWordHandler(0,		rohga_main_write_word);
	SekSetWriteByteHandler(0,		rohga_main_write_byte);
	SekSetReadWordHandler(0,		rohga_main_read_word);
	SekSetReadByteHandler(0,		rohga_main_read_byte);
	SekClose();

	deco16SoundInit(DrvHucROM, DrvHucRAM, 2685000, 0, DrvYM2151WritePort, 0.55, 1006875, 0.65, 2013750, 0.20);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.55, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.55, BURN_SND_ROUTE_RIGHT);

	DrvHangzo = 1;

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 NitrobalInit()
{
	BurnSetRefreshRate(58.00);

	BurnAllocMemIndex();

	{
		if (BurnLoadRom(Drv68KROM  + 0x000001,  0, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x000000,  1, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x040001,  2, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x040000,  3, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x080001,  4, 2)) return 1;
		if (BurnLoadRom(Drv68KROM  + 0x080000,  5, 2)) return 1;

		if (BurnLoadRom(DrvHucROM  + 0x000000,  6, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM0 + 0x000000,  7, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM0 + 0x000001,  8, 2)) return 1;

		if (BurnLoadRom(DrvGfxROM1 + 0x000000,  9, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM1 + 0x080000, 10, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM2 + 0x000000, 11, 1)) return 1;
		if (BurnLoadRom(DrvGfxROM2 + 0x100000, 12, 1)) return 1;

		if (BurnLoadRom(DrvGfxROM3 + 0x000000, 13, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x000001, 14, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x200000, 15, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM3 + 0x200001, 16, 2)) return 1;
		BurnByteswap(DrvGfxROM3, 0x400000);

		if (BurnLoadRom(DrvGfxROM4 + 0x000000, 17, 2)) return 1;
		if (BurnLoadRom(DrvGfxROM4 + 0x000001, 18, 2)) return 1;
		BurnByteswap(DrvGfxROM4, 0x080000);

		if (BurnLoadRom(DrvSndROM0 + 0x000000, 19, 1)) return 1;
		if (BurnLoadRom(DrvSndROM1 + 0x000000, 20, 1)) return 1;

		deco56_decrypt_gfx(DrvGfxROM0, 0x020000);
		deco56_decrypt_gfx(DrvGfxROM1, 0x100000);
		deco74_decrypt_gfx(DrvGfxROM2, 0x200000);

		deco16_tile_decode(DrvGfxROM0, DrvGfxROM0, 0x020000, 1);
		deco16_tile_decode(DrvGfxROM1, DrvGfxROM1, 0x100000, 0);
		deco16_tile_decode(DrvGfxROM2, DrvGfxROM2, 0x200000, 0);
		deco16_sprite_decode(DrvGfxROM3, 0x400000);
		deco16_sprite_decode(DrvGfxROM4, 0x080000);
	}	

	deco16Init(0, 0, 0);
	deco16_set_graphics(DrvGfxROM0, 0x20000 * 2, DrvGfxROM1, 0x100000 * 2, DrvGfxROM2, 0x200000 * 2);
	deco16_set_color_base(2, 512);
	deco16_set_color_mask(2, 0);
	deco16_set_color_base(3, 512);
	deco16_set_color_mask(3, 0);
	deco16_set_global_offsets(0, 8);
	deco16_set_bank_callback(0, rohga_bank_callback);
	deco16_set_bank_callback(1, rohga_bank_callback);
	deco16_set_bank_callback(2, rohga_bank_callback);
	deco16_set_bank_callback(3, rohga_bank_callback);

	// 146_104 prot
	deco_146_init();
	deco_146_104_set_interface_scramble_reverse();
	deco_146_104_set_use_magic_read_address_xor(1);
	deco_146_104_set_port_a_cb(deco_104_port_a_cb);
	deco_146_104_set_port_b_cb(deco_104_port_b_cb);
	deco_146_104_set_port_c_cb(deco_104_port_c_cb);
	deco_146_104_set_soundlatch_cb(soundlatch_write);

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KROM,			0x000000, 0x1fffff, MAP_ROM);
	SekMapMemory(deco16_pf_ram[0],		0x200000, 0x200fff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[0],		0x201000, 0x201fff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[1],		0x202000, 0x2027ff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[1],		0x202800, 0x202fff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[2],		0x208000, 0x2087ff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[2],		0x208800, 0x208fff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[3],		0x20a000, 0x20a7ff, MAP_RAM);
	SekMapMemory(deco16_pf_ram[3],		0x20a800, 0x20afff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[0],	0x204000, 0x2047ff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[1],	0x206000, 0x2067ff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[2],	0x20c000, 0x20c7ff, MAP_RAM);
	SekMapMemory(deco16_pf_rowscroll[3],	0x20e000, 0x20e7ff, MAP_RAM);
	SekMapMemory(DrvSprRAM,			0x340000, 0x3407ff, MAP_RAM);
	SekMapMemory(DrvSprRAM2,		0x360000, 0x3607ff, MAP_RAM);
	SekMapMemory(DrvPalRAM,			0x380000, 0x381fff, MAP_RAM);
	SekMapMemory(Drv68KRAM,			0xfec000, 0xff3fff, MAP_RAM);
	SekMapMemory(Drv68KRAM + 0x8000,	0xff8000, 0xffffff, MAP_RAM);
	SekSetWriteWordHandler(0,		wizdfire_main_write_word);
	SekSetWriteByteHandler(0,		wizdfire_main_write_byte);
	SekSetReadWordHandler(0,		wizdfire_main_read_word);
	SekSetReadByteHandler(0,		wizdfire_main_read_byte);
	SekClose();

	deco16SoundInit(DrvHucROM, DrvHucRAM, 2685000, 0, DrvYM2151WritePort, 0.50, 1006875, 0.50, 2013750, 0.30);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.50, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.50, BURN_SND_ROUTE_RIGHT);

	GenericTilesInit();

	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	GenericTilesExit();
	deco16Exit();

	SekExit();

	deco16SoundExit();

	BurnFreeMemIndex();

	WizdfireEnglish = 0;
	DrvHangzo = 0;

	return 0;
}

static void rohga_draw_sprites(UINT8 *ram, INT32 is_schmeisr)
{
	UINT16 *spriteptr = (UINT16*)ram;

	for (INT32 offs = 0x400 - 4; offs >= 0; offs -= 4)
	{
		INT32 inc, mult, pri = 0;
		INT32 sprite = BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 1]);

		if (!sprite) continue;

		INT32 x = BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 2]);

		switch (x & 0x6000)
		{
			case 0x0000: pri = 0; break;
			case 0x4000: pri = 0xf0; break;
			case 0x6000: pri = 0xf0 | 0xcc; break;
			case 0x2000: pri = 0; break;
		}

		INT32 y = BURN_ENDIAN_SWAP_INT16(spriteptr[offs]);

		if ((y & 0x1000) && (nCurrentFrame & 1)) continue; // flash

		INT32 colour = (((x >> 9) & 0xf) << 6);
		if (is_schmeisr) colour += ((x & 0x8000) >> 15) << 4;

		INT32 fx = y & 0x2000;
		INT32 fy = y & 0x4000;
		INT32 multi = (1 << ((y & 0x0600) >> 9)) - 1;

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;

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
			x = 304 - x;
			y = 240 - y;
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;
			mult = -16;
		}
		else
			mult = +16;

		while (multi >= 0)
		{
			deco16_draw_prio_sprite(pTransDraw, DrvGfxROM3, (sprite - multi * inc) & 0x7fff, colour + 0x400, x, y + mult * multi, fx, fy, pri);

			multi--;
		}
	}
}

static void wizdfire_draw_sprites(UINT8 *ram, UINT8 *gfx, INT32 coloff, INT32 mode, INT32 bank, INT32 alpha_on)
{
	UINT16 *spriteptr = (UINT16*)ram;

	for (INT32 offs = 0; offs < 0x400; offs += 4)
	{
		INT32 inc, mult, prio = 0, alpha = 0xff;

		INT32 sprite = BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 1]);
		if (!sprite) continue;

		INT32 x = BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 2]);

		switch (mode)
		{
			case 4:
				if ((x & 0xc000) != 0xc000)
					continue;
				prio = 0x08;
			break;
			case 3:
				if ((x & 0xc000) != 0x8000)
					continue;
				prio = 0x10;
			break;
			case 2:
				if ((x & 0x8000) != 0x8000)
					continue;
				prio = 0x20;
			break;
			case 1:
			case 0:
			default:
				if ((x & 0x8000) != 0)
					continue;
				prio = 0x40;
			break;
		}

		INT32 y = BURN_ENDIAN_SWAP_INT16(spriteptr[offs]);

		if ((y & 0x1000) && (nCurrentFrame & 1)) continue; // flash

		INT32 colour = (x >> 9) & 0x1f;

		if (bank == 4 && colour & 0x10)
		{
			alpha = 0x80;
			colour &= 0xf;
		}

		INT32 fx = y & 0x2000;
		INT32 fy = y & 0x4000;
		INT32 multi = (1 << ((y & 0x0600) >> 9)) - 1;

		x = x & 0x01ff;
		y = y & 0x01ff;
		if (x >= 320) x -= 512;
		if (y >= 256) y -= 512;

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
			x = 304 - x;
			y = 240 - y;
			mult = -16;
		} else {
			mult = +16;
			fx = !fx;
			fy = !fy;
		}

		if (bank == 3) {
			sprite &= 0x7fff;
		} else {
			sprite &= 0x0fff;
		}

		while (multi >= 0)
		{
			if (alpha_on) {
				deco16_draw_alphaprio_sprite(DrvPalette, gfx, sprite - multi * inc, (colour << 4) + coloff, x, y + mult * multi, fx, fy, prio|0xf0, 1, alpha, 1 /* special mode for wizdfire */);
			} else {
				if (prio >= 0x20) // "over alpha" solid sprites.  see mix_alpha_tilemap()
					deco16_draw_prio_sprite_dumb(pTransDraw, gfx, sprite - multi * inc, (colour << 4) + coloff, x, y + mult * multi, fx, fy, 0xf0, -1);
				else
					deco16_draw_prio_sprite(pTransDraw, gfx, sprite - multi * inc, (colour << 4) + coloff, x, y + mult * multi, fx, fy, -1);
			}

			multi--;
		}
	}
}

static void nitrobal_draw_sprites(UINT8 *ram, INT32 gfxbank, INT32 alpha_on)
{
	UINT16 *spriteptr = (UINT16*)ram;

	INT32 offs = 0x3fc;
	INT32 end = -4;
	INT32 inc = -4;
	UINT8 *gfx;

	while (offs != end)
	{
		INT32 x, y, sprite, colour, fx, fy, w, h, sx, sy, x_mult, y_mult, tilemap_pri, sprite_pri, coloff;
		INT32 alpha = 0xff;

		sprite = BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 3]);
		if (!sprite)
		{
			offs += inc;
			continue;
		}

		sx = BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 1]);

		h = (BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 2]) & 0xf000) >> 12;
		w = (BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 2]) & 0x0f00) >>  8;

		sy = BURN_ENDIAN_SWAP_INT16(spriteptr[offs]);
		if ((sy & 0x2000) && (nCurrentFrame & 1))
		{
			offs += inc;
			continue;
		}

		colour = (BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 2]) >> 0) & 0x1f;

		if (gfxbank == 3)
		{
			switch (BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 2]) & 0xe0)
			{ // deco16_priority == 0x20 in alien world, 0 in ghost town
				case 0xc0: tilemap_pri = 8;   break;
				case 0x80: tilemap_pri = 64;  break; // was 32, fixes horseshoe "ring" under bouncers in first level
				case 0x20: tilemap_pri = 32;  break;
				case 0x40: tilemap_pri = 8;   break;
				case 0xa0: tilemap_pri = (deco16_priority) ? 8 : 32;  break; // was 32
				case 0x00: tilemap_pri = 72; break; // was 128, this and above fixes the archways over the player in alien world
				default:   tilemap_pri = 128; break;
			}

			sprite_pri = 1;

			gfx = DrvGfxROM3;
			coloff = 0x400;
		}
		else
		{
			if (deco16_priority)
				tilemap_pri = 8;
			else
				tilemap_pri = 64;

			sprite_pri = 2;

			gfx = DrvGfxROM4;
			coloff = 0x600;

			if (colour & 0x10) {
				alpha = 0x80;
				colour &= 0x0f;
			}
		}

		fx = (BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 0]) & 0x4000);
		fy = (BURN_ENDIAN_SWAP_INT16(spriteptr[offs + 0]) & 0x8000);

		if (!*flipscreen)
		{
			if (fx) fx = 0; else fx = 1;
			if (fy) fy = 0; else fy = 1;

			sx = sx & 0x01ff;
			sy = sy & 0x01ff;
			if (sx > 0x180) sx = -(0x200 - sx);
			if (sy > 0x180) sy = -(0x200 - sy);

			if (fx) { x_mult = -16; sx += 16 * w; } else { x_mult = 16; sx -= 16; }
			if (fy) { y_mult = -16; sy += 16 * h; } else { y_mult = 16; sy -= 16; }
		}
		else
		{
			sx = sx & 0x01ff;
			sy = sy & 0x01ff;
			if (sx & 0x100) sx = -(0x100 - (sx & 0xff));
			if (sy & 0x100) sy = -(0x100 - (sy & 0xff));
			sx = 304 - sx;
			sy = 240 - sy;
			if (sx >= 432) sx -= 512;
			if (sy >= 384) sy -= 512;
			if (fx) { x_mult = -16; sx += 16; } else { x_mult = 16; sx -= 16 * w; }
			if (fy) { y_mult = -16; sy += 16; } else { y_mult = 16; sy -= 16 * h; }
		}

		if (gfxbank == 3) {
			sprite &= 0x7fff;
		} else {
			sprite &= 0x0fff;
		}

		for (x = 0; x < w; x++)
		{
			for (y = 0; y < h; y++)
			{
				if (!alpha_on) {
					deco16_draw_prio_sprite_nitrobal(pTransDraw, gfx, sprite + y + h * x, (colour << 4) + coloff, sx + x_mult * (w-x), sy + y_mult * (h-y), fx, fy, tilemap_pri, sprite_pri);
				} else {
					deco16_draw_alphaprio_sprite(DrvPalette, gfx, sprite + y + h * x, (colour << 4) + coloff, sx + x_mult * (w-x), sy + y_mult * (h-y), fx, fy, tilemap_pri, sprite_pri, alpha);
				}
			}
		}

		offs += inc;
	}
}

static void draw_combined_playfield_step1()
{
	UINT8 *tptr = deco16_pf_rowscroll[3];
	deco16_pf_rowscroll[3] = deco16_pf_rowscroll[2];

	deco16_draw_layer(2, tempdraw[0], 0x10000);
	deco16_draw_layer(3, tempdraw[1], 0x10000);

	deco16_pf_rowscroll[3] = tptr;
}

static void draw_combined_playfield(INT32 color, INT32 priority) // opaque
{
	UINT16 *src0 = tempdraw[0];
	UINT16 *src1 = tempdraw[1];
	UINT16 *dest = pTransDraw;
	UINT8 *prio = deco16_prio_map;

	for (INT32 y = 0; y < nScreenHeight; y++) {
		for (INT32 x = 0; x < nScreenWidth; x++) {
			dest[x] = color | (src0[x] & 0x0f) | ((src1[x] & 0x0f) << 4);
			prio[x] = priority;
		}
		src0 += nScreenWidth;
		src1 += nScreenWidth;
		dest += nScreenWidth;
		prio += 512;
	}
}

static void update_rohga(INT32 is_schmeisr)
{
//	if (DrvRecalc) {
		deco16_palette_recalculate(DrvPalette, DrvPalBuf);
		DrvRecalc = 0;
//	}

	deco16_pf12_update();
	deco16_pf34_update();

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x300;
	}

	if ((deco16_priority & 0x03) == 0) {
		draw_combined_playfield_step1();
	}

	deco16_clear_prio_map();

	switch (deco16_priority & 3)
	{
		case 0:
			if (deco16_priority & 4)
			{
				draw_combined_playfield(0x200, DECO16_LAYER_PRIORITY(3));
			}
			else
			{
				deco16_draw_layer(3, pTransDraw, DECO16_LAYER_OPAQUE | DECO16_LAYER_PRIORITY(0x01));
				deco16_draw_layer(2, pTransDraw, DECO16_LAYER_PRIORITY(0x02));
			}
			deco16_draw_layer(1, pTransDraw, DECO16_LAYER_PRIORITY(0x04));
			break;

		case 1:
			deco16_draw_layer(3, pTransDraw, DECO16_LAYER_OPAQUE | DECO16_LAYER_PRIORITY(0x01));
			deco16_draw_layer(1, pTransDraw, DECO16_LAYER_PRIORITY(0x02));
			deco16_draw_layer(2, pTransDraw, DECO16_LAYER_PRIORITY(0x04));
			break;

		case 2:
			deco16_draw_layer(1, pTransDraw, DECO16_LAYER_OPAQUE | DECO16_LAYER_PRIORITY(0x01));
			deco16_draw_layer(3, pTransDraw, DECO16_LAYER_PRIORITY(0x02));
			deco16_draw_layer(2, pTransDraw, DECO16_LAYER_PRIORITY(0x04));
			break;
	}

	if (nSpriteEnable & 1) rohga_draw_sprites(DrvSprBuf, is_schmeisr);

	deco16_draw_layer(0, pTransDraw, 0);

	BurnTransferCopy(DrvPalette);
}

static INT32 RohgaDraw()
{
	update_rohga(0);

	return 0;
}

static INT32 SchmeisrDraw()
{
	update_rohga(1);

	return 0;
}

static inline UINT32 alpha_blend(UINT32 d, UINT32 s, UINT32 p)
{
	INT32 a = 256 - p;

	return (((((s & 0xff00ff) * p) + ((d & 0xff00ff) * a)) & 0xff00ff00) +
		((((s & 0x00ff00) * p) + ((d & 0x00ff00) * a)) & 0x00ff0000)) / 256;
}

static void mix_alpha_tilemap()
{
	UINT16 *src0 = tempdraw[0];
	UINT32 *dest = (UINT32*)pBurnDraw;
	UINT8 *prio = deco16_prio_map;

	for (INT32 y = 0; y < nScreenHeight; y++) {
		for (INT32 x = 0; x < nScreenWidth; x++) {
			if (prio[x] < 0xf0 && src0[x] != 0x00)
				dest[x] = alpha_blend(dest[x], DrvPalette[src0[x]], 0x80);
		}
		src0 += nScreenWidth;
		dest += nScreenWidth;
		prio += 512;
	}

	memset(tempdraw[0], 0, 320 * 240 * sizeof(UINT16));
}

static INT32 WizdfireDraw()
{
//	if (DrvRecalc) {
		deco16_palette_recalculate(DrvPalette, DrvPalBuf);
		DrvRecalc = 0;
//	}

	deco16_pf12_update();
	deco16_pf34_update();

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x200;
	}

	deco16_clear_prio_map();

	if (nBurnLayer & 1) deco16_draw_layer(3, pTransDraw, DECO16_LAYER_OPAQUE);

	if (nSpriteEnable & 1) wizdfire_draw_sprites(DrvSprBuf, DrvGfxROM3, 0x400, 4, 3, 0);

	if (nBurnLayer & 2) deco16_draw_layer(1, pTransDraw, 0);

	if (nSpriteEnable & 2) wizdfire_draw_sprites(DrvSprBuf, DrvGfxROM3, 0x400, 3, 3, 0);

	if (nBurnLayer & 4) deco16_draw_layer(2, ((deco16_priority & 0x1f) == 0x1f && nBurnBpp == 4) ? tempdraw[0] : pTransDraw, 0); // tilemap draw alpha 0x80...

	if (nSpriteEnable & 4) wizdfire_draw_sprites(DrvSprBuf, DrvGfxROM3, 0x400, 0, 3, 0);

	if (nBurnBpp != 4) {
		if (nSpriteEnable & 8) wizdfire_draw_sprites(DrvSprBuf2, DrvGfxROM4, 0x600, 2, 4, 0);
		if (nSpriteEnable & 16) wizdfire_draw_sprites(DrvSprBuf2, DrvGfxROM4, 0x600, 1, 4, 0);
	}
	if (nBurnLayer & 8) deco16_draw_layer(0, pTransDraw, DECO16_LAYER_PRIORITY(0xff));

	BurnTransferCopy(DrvPalette);

	if (nBurnBpp == 4) {
		if (nSpriteEnable & 8) wizdfire_draw_sprites(DrvSprBuf2, DrvGfxROM4, 0x600, 2, 4, 1);
		if (nSpriteEnable & 16) wizdfire_draw_sprites(DrvSprBuf2, DrvGfxROM4, 0x600, 1, 4, 1);
		if ((deco16_priority & 0x1f) == 0x1f && nBurnBpp == 4) mix_alpha_tilemap();
	}

	return 0;
}

static INT32 NitrobalDraw()
{
//	if (DrvRecalc) {
		deco16_palette_recalculate(DrvPalette, DrvPalBuf);
		DrvRecalc = 0;
//	}

	deco16_pf12_update();
	deco16_pf34_update();

	for (INT32 i = 0; i < nScreenWidth * nScreenHeight; i++) {
		pTransDraw[i] = 0x200;
	}

	deco16_clear_prio_map();

	draw_combined_playfield_step1();

	draw_combined_playfield(0x200, 0);

	deco16_draw_layer(1, pTransDraw, 16);

	if (nBurnBpp != 4) { // regular (non-alpha) draw for anything !32bpp
		nitrobal_draw_sprites(DrvSprBuf , 3, 0);
		nitrobal_draw_sprites(DrvSprBuf2, 4, 0);
	}

	deco16_draw_layer(0, pTransDraw, DECO16_LAYER_PRIORITY(0xff));

	BurnTransferCopy(DrvPalette);

	if (nBurnBpp == 4) { // draw alpha sprites if 32bpp
		nitrobal_draw_sprites(DrvSprBuf , 3, 1);
		nitrobal_draw_sprites(DrvSprBuf2, 4, 1);
	}

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	{
		memset (DrvInputs, 0xff, 4 * sizeof(UINT16));
		for (INT32 i = 0; i < 16; i++) {
			DrvInputs[0] ^= (DrvJoy1[i] & 1) << i;
			DrvInputs[1] ^= (DrvJoy2[i] & 1) << i;
			DrvInputs[3] ^= (DrvJoy3[i] & 1) << i;
		}
		DrvInputs[2] = (DrvDips[1] << 8) | (DrvDips[0] << 0);
	}

	INT32 nInterleave = 256;
	INT32 nCyclesTotal[2] = { 14000000 / 58, 2685000 / 58 };
	INT32 nCyclesDone[2] = { nCyclesExtra, 0 };

	h6280NewFrame();

	SekOpen(0);
	h6280Open(0);

	deco16_vblank = 0;

	for (INT32 i = 0; i < nInterleave; i++)
	{
		CPU_RUN(0, Sek);
		CPU_RUN_TIMER(1);

		if (i == 248) {
			SekSetIRQLine(6, CPU_IRQSTATUS_ACK);
			deco16_vblank = 0x08;

			if (pBurnDraw) {
				BurnDrvRedraw();
			}
		}
	}

	h6280Close();
	SekClose();

	nCyclesExtra = nCyclesDone[0] - nCyclesTotal[0];

	if (pBurnSoundOut) {
		deco16SoundUpdate(pBurnSoundOut, nBurnSoundLen);
	}

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
		SekScan(nAction);

		deco16SoundScan(nAction, pnMin);

		deco16Scan();

		SCAN_VAR(DrvOkiBank);
		SCAN_VAR(nCyclesExtra);
	}

	if (nAction & ACB_WRITE) {
		DrvYM2151WritePort(0, DrvOkiBank);
	}

	return 0;
}


// Rohga: Armor Force (Asia/Europe v5.0)

static struct BurnRomInfo rohgaRomDesc[] = {
	{ "ht-00-1.2a",		0x040000, 0x1ed84a67, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ht-03-1.2d",		0x040000, 0x84e7ebf6, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "mam00.8a",		0x080000, 0x0fa440a6, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "mam07.8d",		0x080000, 0xf8bc7f20, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "ha04.18p",		0x010000, 0xeb6608eb, 2 | BRF_PRG | BRF_ESS }, //  4 Huc6280 Code

	{ "ha01.13a",		0x010000, 0xfb8f8519, 3 | BRF_GRA },           //  5 Characters
	{ "ha02.14a",		0x010000, 0xaa47c17f, 3 | BRF_GRA },           //  6

	{ "mam01.10a",		0x080000, 0xdbf4fbcc, 4 | BRF_GRA },           //  7 Foreground Tiles
	{ "mam02.11a",		0x080000, 0xb1fac481, 4 | BRF_GRA },           //  8

	{ "mam08.17d",		0x100000, 0xca97a83f, 5 | BRF_GRA },           //  9 Background Tiles
	{ "mam09.18d",		0x100000, 0x3f57d56f, 5 | BRF_GRA },           // 10

	{ "mam05.19a",		0x100000, 0x307a2cd1, 6 | BRF_GRA },           // 11 Sprites
	{ "mam06.20a",		0x100000, 0xa1119a2d, 6 | BRF_GRA },           // 12
	{ "mam10.19d",		0x100000, 0x99f48f9f, 6 | BRF_GRA },           // 13
	{ "mam11.20d",		0x100000, 0xc3f12859, 6 | BRF_GRA },           // 14
	{ "mam03.17a",		0x100000, 0xfc4dfd48, 6 | BRF_GRA },           // 15
	{ "mam04.18a",		0x100000, 0x7d3b38bf, 6 | BRF_GRA },           // 16

	{ "mam13.15p",		0x080000, 0x525b9461, 8 | BRF_SND },           // 17 OKI M6295 Samples 0

	{ "mam12.14p",		0x080000, 0x6f00b791, 7 | BRF_SND },           // 18 OKI M6295 Samples 1

	{ "hb-00.11p",		0x000200, 0xb7a7baad, 0 | BRF_OPT },           // 19 Unused PROMs
};

STD_ROM_PICK(rohga)
STD_ROM_FN(rohga)

struct BurnDriver BurnDrvRohga = {
	"rohga", NULL, NULL, NULL, "1991",
	"Rohga: Armor Force (Asia/Europe v5.0)\0", NULL, "Data East Corporation", "DECO IC16",
	L"\u7A7A\u7259 2001 (\u4E9A\u7248/\u6B27\u7248 v5.0)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT | GBF_PLATFORM, 0,
	NULL, rohgaRomInfo, rohgaRomName, NULL, NULL, NULL, NULL, RohgaInputInfo, RohgaDIPInfo,
	RohgaInit, DrvExit, DrvFrame, RohgaDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Rohga Armor Force (Asia/Europe v3.0 set 1)

static struct BurnRomInfo rohga1RomDesc[] = {
	{ "jd00.bin",		0x040000, 0xe046c77a, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "jd03.bin",		0x040000, 0x2c5120b8, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "mam00.8a",		0x080000, 0x0fa440a6, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "mam07.8d",		0x080000, 0xf8bc7f20, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "ha04.18p",		0x010000, 0xeb6608eb, 2 | BRF_PRG | BRF_ESS }, //  4 Huc6280 Code

	{ "ha01.13a",		0x010000, 0xfb8f8519, 3 | BRF_GRA },           //  5 Characters
	{ "ha02.14a",		0x010000, 0xaa47c17f, 3 | BRF_GRA },           //  6

	{ "mam01.10a",		0x080000, 0xdbf4fbcc, 4 | BRF_GRA },           //  7 Foreground Tiles
	{ "mam02.11a",		0x080000, 0xb1fac481, 4 | BRF_GRA },           //  8

	{ "mam08.17d",		0x100000, 0xca97a83f, 5 | BRF_GRA },           //  9 Background Tiles
	{ "mam09.18d",		0x100000, 0x3f57d56f, 5 | BRF_GRA },           // 10

	{ "mam05.19a",		0x100000, 0x307a2cd1, 6 | BRF_GRA },           // 11 Sprites
	{ "mam06.20a",		0x100000, 0xa1119a2d, 6 | BRF_GRA },           // 12
	{ "mam10.19d",		0x100000, 0x99f48f9f, 6 | BRF_GRA },           // 13
	{ "mam11.20d",		0x100000, 0xc3f12859, 6 | BRF_GRA },           // 14
	{ "mam03.17a",		0x100000, 0xfc4dfd48, 6 | BRF_GRA },           // 15
	{ "mam04.18a",		0x100000, 0x7d3b38bf, 6 | BRF_GRA },           // 16

	{ "mam13.15p",		0x080000, 0x525b9461, 8 | BRF_SND },           // 17 OKI M6295 Samples 0

	{ "mam12.14p",		0x080000, 0x6f00b791, 7 | BRF_SND },           // 18 OKI M6295 Samples 1

	{ "hb-00.11p",		0x000200, 0xb7a7baad, 0 | BRF_OPT },           // 19 Unused PROMs
};

STD_ROM_PICK(rohga1)
STD_ROM_FN(rohga1)

struct BurnDriver BurnDrvRohga1 = {
	"rohga1", "rohga", NULL, NULL, "1991",
	"Rohga Armor Force (Asia/Europe v3.0 set 1)\0", NULL, "Data East Corporation", "DECO IC16",
	L"\u7A7A\u7259 2001 (\u4E9A\u7248/\u6B27\u7248 v3.0 \u7B2C\u4E00\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT | GBF_PLATFORM, 0,
	NULL, rohga1RomInfo, rohga1RomName, NULL, NULL, NULL, NULL, RohgaInputInfo, RohgaDIPInfo,
	RohgaInit, DrvExit, DrvFrame, RohgaDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Rohga: Armor Force (Asia/Europe v3.0 set 2)

static struct BurnRomInfo rohga2RomDesc[] = {
	{ "hts-00-3.2a",	0x040000, 0x154f02ec, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "hts-03-3.2d",	0x040000, 0x5e69d3d8, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "mam00.8a",		0x080000, 0x0fa440a6, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "mam07.8d",		0x080000, 0xf8bc7f20, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "ha04.18p",		0x010000, 0xeb6608eb, 2 | BRF_PRG | BRF_ESS }, //  4 Huc6280 Code

	{ "ha01.13a",		0x010000, 0xfb8f8519, 3 | BRF_GRA },           //  5 Characters
	{ "ha02.14a",		0x010000, 0xaa47c17f, 3 | BRF_GRA },           //  6

	{ "mam01.10a",		0x080000, 0xdbf4fbcc, 4 | BRF_GRA },           //  7 Foreground Tiles
	{ "mam02.11a",		0x080000, 0xb1fac481, 4 | BRF_GRA },           //  8

	{ "mam08.17d",		0x100000, 0xca97a83f, 5 | BRF_GRA },           //  9 Background Tiles
	{ "mam09.18d",		0x100000, 0x3f57d56f, 5 | BRF_GRA },           // 10

	{ "mam05.19a",		0x100000, 0x307a2cd1, 6 | BRF_GRA },           // 11 Sprites
	{ "mam06.20a",		0x100000, 0xa1119a2d, 6 | BRF_GRA },           // 12
	{ "mam10.19d",		0x100000, 0x99f48f9f, 6 | BRF_GRA },           // 13
	{ "mam11.20d",		0x100000, 0xc3f12859, 6 | BRF_GRA },           // 14
	{ "mam03.17a",		0x100000, 0xfc4dfd48, 6 | BRF_GRA },           // 15
	{ "mam04.18a",		0x100000, 0x7d3b38bf, 6 | BRF_GRA },           // 16

	{ "mam13.15p",		0x080000, 0x525b9461, 8 | BRF_SND },           // 17 OKI M6295 Samples 0

	{ "mam12.14p",		0x080000, 0x6f00b791, 7 | BRF_SND },           // 18 OKI M6295 Samples 1

	{ "hb-00.11p",		0x000200, 0xb7a7baad, 0 | BRF_OPT },           // 19 Unused PROMs
};

STD_ROM_PICK(rohga2)
STD_ROM_FN(rohga2)

struct BurnDriver BurnDrvRohga2 = {
	"rohga2", "rohga", NULL, NULL, "1991",
	"Rohga: Armor Force (Asia/Europe v3.0 set 2)\0", NULL, "Data East Corporation", "DECO IC16",
	L"\u7A7A\u7259 2001 (\u4E9A\u7248/\u6B27\u7248 v3.0 \u7B2C\u4E8C\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT | GBF_PLATFORM, 0,
	NULL, rohga2RomInfo, rohga2RomName, NULL, NULL, NULL, NULL, RohgaInputInfo, RohgaDIPInfo,
	RohgaInit, DrvExit, DrvFrame, RohgaDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Rohga: Armor Force (Hong Kong v3.0)

static struct BurnRomInfo rohgahRomDesc[] = {
	{ "jd00-2.2a",		0x040000, 0xec70646a, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "jd03-2.2d",		0x040000, 0x11d4c9a2, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "mam00.8a",		0x080000, 0x0fa440a6, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "mam07.8d",		0x080000, 0xf8bc7f20, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "ha04.18p",		0x010000, 0xeb6608eb, 2 | BRF_PRG | BRF_ESS }, //  4 Huc6280 Code

	{ "ha01.13a",		0x010000, 0xfb8f8519, 3 | BRF_GRA },           //  5 Characters
	{ "ha02.14a",		0x010000, 0xaa47c17f, 3 | BRF_GRA },           //  6

	{ "mam01.10a",		0x080000, 0xdbf4fbcc, 4 | BRF_GRA },           //  7 Foreground Tiles
	{ "mam02.11a",		0x080000, 0xb1fac481, 4 | BRF_GRA },           //  8

	{ "mam08.17d",		0x100000, 0xca97a83f, 5 | BRF_GRA },           //  9 Background Tiles
	{ "mam09.18d",		0x100000, 0x3f57d56f, 5 | BRF_GRA },           // 10

	{ "mam05.19a",		0x100000, 0x307a2cd1, 6 | BRF_GRA },           // 11 Sprites
	{ "mam06.20a",		0x100000, 0xa1119a2d, 6 | BRF_GRA },           // 12
	{ "mam10.19d",		0x100000, 0x99f48f9f, 6 | BRF_GRA },           // 13
	{ "mam11.20d",		0x100000, 0xc3f12859, 6 | BRF_GRA },           // 14
	{ "mam03.17a",		0x100000, 0xfc4dfd48, 6 | BRF_GRA },           // 15
	{ "mam04.18a",		0x100000, 0x7d3b38bf, 6 | BRF_GRA },           // 16

	{ "mam13.15p",		0x080000, 0x525b9461, 8 | BRF_SND },           // 17 OKI M6295 Samples 0

	{ "mam12.14p",		0x080000, 0x6f00b791, 7 | BRF_SND },           // 18 OKI M6295 Samples 1

	{ "hb-00.11p",		0x000200, 0xb7a7baad, 0 | BRF_OPT },           // 19 Unused PROMs
};

STD_ROM_PICK(rohgah)
STD_ROM_FN(rohgah)

struct BurnDriver BurnDrvRohgah = {
	"rohgah", "rohga", NULL, NULL, "1991",
	"Rohga: Armor Force (Hong Kong v3.0)\0", NULL, "Data East Corporation", "DECO IC16",
	L"\u7A7A\u7259 2001 (\u6E2F\u7248 v3.0)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT | GBF_PLATFORM, 0,
	NULL, rohgahRomInfo, rohgahRomName, NULL, NULL, NULL, NULL, RohgaInputInfo, RohgaDIPInfo,
	RohgaInit, DrvExit, DrvFrame, RohgaDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Rohga: Armor Force (US v1.0)

static struct BurnRomInfo rohgauRomDesc[] = {
	{ "ha00.2a",		0x040000, 0xd8d13052, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "ha03.2d",		0x040000, 0x5f683bbf, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "mam00.8a",		0x080000, 0x0fa440a6, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "mam07.8d",		0x080000, 0xf8bc7f20, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "ha04.18p",		0x010000, 0xeb6608eb, 2 | BRF_PRG | BRF_ESS }, //  4 Huc6280 Code

	{ "ha01.13a",		0x010000, 0xfb8f8519, 3 | BRF_GRA },           //  5 Characters
	{ "ha02.14a",		0x010000, 0xaa47c17f, 3 | BRF_GRA },           //  6

	{ "mam01.10a",		0x080000, 0xdbf4fbcc, 4 | BRF_GRA },           //  7 Foreground Tiles
	{ "mam02.11a",		0x080000, 0xb1fac481, 4 | BRF_GRA },           //  8

	{ "mam08.17d",		0x100000, 0xca97a83f, 5 | BRF_GRA },           //  9 Background Tiles
	{ "mam09.18d",		0x100000, 0x3f57d56f, 5 | BRF_GRA },           // 10

	{ "mam05.19a",		0x100000, 0x307a2cd1, 6 | BRF_GRA },           // 11 Sprites
	{ "mam06.20a",		0x100000, 0xa1119a2d, 6 | BRF_GRA },           // 12
	{ "mam10.19d",		0x100000, 0x99f48f9f, 6 | BRF_GRA },           // 13
	{ "mam11.20d",		0x100000, 0xc3f12859, 6 | BRF_GRA },           // 14
	{ "mam03.17a",		0x100000, 0xfc4dfd48, 6 | BRF_GRA },           // 15
	{ "mam04.18a",		0x100000, 0x7d3b38bf, 6 | BRF_GRA },           // 16

	{ "mam13.15p",		0x080000, 0x525b9461, 8 | BRF_SND },           // 17 OKI M6295 Samples 0

	{ "mam12.14p",		0x080000, 0x6f00b791, 7 | BRF_SND },           // 18 OKI M6295 Samples 1

	{ "hb-00.11p",		0x000200, 0xb7a7baad, 0 | BRF_OPT },           // 19 Unused PROMs
};

STD_ROM_PICK(rohgau)
STD_ROM_FN(rohgau)

struct BurnDriver BurnDrvRohgau = {
	"rohgau", "rohga", NULL, NULL, "1991",
	"Rohga: Armor Force (US v1.0)\0", NULL, "Data East Corporation", "DECO IC16",
	L"\u7A7A\u7259 2001 (\u7F8E\u7248 v1.0)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT | GBF_PLATFORM, 0,
	NULL, rohgauRomInfo, rohgauRomName, NULL, NULL, NULL, NULL, RohgaInputInfo, RohgaDIPInfo,
	RohgaInit, DrvExit, DrvFrame, RohgaDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Wolf Fang: Kuhga 2001 (Japan)

static struct BurnRomInfo wolffangRomDesc[] = {
	{ "hw_00-1.2a",		0x040000, 0x69dc611e, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "hw_03-1.2d",		0x040000, 0xb66d9680, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "mam00.8a",		0x080000, 0x0fa440a6, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "mam07.8d",		0x080000, 0xf8bc7f20, 1 | BRF_PRG | BRF_ESS }, //  3

	{ "hw_04-.18p",		0x010000, 0xeb6608eb, 2 | BRF_PRG | BRF_ESS }, //  4 Huc6280 Code

	{ "hw_01-.13a",		0x010000, 0xd9810ca4, 3 | BRF_GRA },           //  5 Characters
	{ "hw_02-.14a",		0x010000, 0x2a27ac8e, 3 | BRF_GRA },           //  6

	{ "mam01.10a",		0x080000, 0xdbf4fbcc, 4 | BRF_GRA },           //  7 Foreground Tiles
	{ "mam02.11a",		0x080000, 0xb1fac481, 4 | BRF_GRA },           //  8

	{ "mam08.17d",		0x100000, 0xca97a83f, 5 | BRF_GRA },           //  9 Background Tiles
	{ "mam09.18d",		0x100000, 0x3f57d56f, 5 | BRF_GRA },           // 10

	{ "mam05.19a",		0x100000, 0x307a2cd1, 6 | BRF_GRA },           // 11 Sprites
	{ "mam06.20a",		0x100000, 0xa1119a2d, 6 | BRF_GRA },           // 12
	{ "mam10.19d",		0x100000, 0x99f48f9f, 6 | BRF_GRA },           // 13
	{ "mam11.20d",		0x100000, 0xc3f12859, 6 | BRF_GRA },           // 14
	{ "mam03.17a",		0x100000, 0xfc4dfd48, 6 | BRF_GRA },           // 15
	{ "mam04.18a",		0x100000, 0x7d3b38bf, 6 | BRF_GRA },           // 16

	{ "mam13.15p",		0x080000, 0x525b9461, 8 | BRF_SND },           // 17 OKI M6295 Samples 0

	{ "mam12.14p",		0x080000, 0x6f00b791, 7 | BRF_SND },           // 18 OKI M6295 Samples 1

	{ "hb-00.11p",		0x000200, 0xb7a7baad, 0 | BRF_OPT },           // 19 Unused PROMs
};

STD_ROM_PICK(wolffang)
STD_ROM_FN(wolffang)

struct BurnDriver BurnDrvWolffang = {
	"wolffang", "rohga", NULL, NULL, "1991",
	"Wolf Fang: Kuhga 2001 (Japan)\0", NULL, "Data East Corporation", "DECO IC16",
	L"\u7A7A\u7259 2001 (\u65E5\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT | GBF_PLATFORM, 0,
	NULL, wolffangRomInfo, wolffangRomName, NULL, NULL, NULL, NULL, RohgaInputInfo, RohgaDIPInfo,
	RohgaInit, DrvExit, DrvFrame, RohgaDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Wizard Fire (Over Sea v2.1)

static struct BurnRomInfo wizdfireRomDesc[] = {
	{ "je-01.3d",		0x020000, 0xb6d62367, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "je-00.3a",		0x020000, 0xf33de278, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "je-03.5d",		0x020000, 0x5217d404, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "je-02.5a",		0x020000, 0x36a1ce28, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "mas13",			0x080000, 0x7e5256ce, 1 | BRF_PRG | BRF_ESS }, //  4
 	{ "mas12",			0x080000, 0x005bd499, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "je-06.20r",		0x010000, 0x79042546, 2 | BRF_PRG | BRF_ESS }, //  6 Huc6280 Code

	{ "je-04.10d",		0x010000, 0x73cba800, 3 | BRF_GRA },           //  7 Characters
	{ "je-05.12d",		0x010000, 0x22e2c49d, 3 | BRF_GRA },           //  8

	{ "mas00",			0x100000, 0x3d011034, 4 | BRF_GRA },           //  9 Foreground Tiles
 	{ "mas01",			0x100000, 0x6d0c9d0b, 4 | BRF_GRA },           // 10

	{ "mas02",			0x080000, 0xaf00e620, 5 | BRF_GRA },           // 11 Background Tiles
	{ "mas03",			0x080000, 0x2fe61ea2, 5 | BRF_GRA },           // 12

	{ "mas04",			0x100000, 0x1e56953b, 6 | BRF_GRA },           // 13 Sprite Bank A
	{ "mas05",			0x100000, 0x3826b8f8, 6 | BRF_GRA },           // 14
	{ "mas06",			0x100000, 0x3b8bbd45, 6 | BRF_GRA },           // 15
	{ "mas07",			0x100000, 0x31303769, 6 | BRF_GRA },           // 16

	{ "mas08",			0x080000, 0xe224fb7a, 7 | BRF_GRA },           // 17 Sprite Bank B
	{ "mas09",			0x080000, 0x5f6deb41, 7 | BRF_GRA },           // 18

	{ "mas10",			0x100000, 0xf4b4c8a1, 8 | BRF_SND },           // 19 OKI M6295 Samples 0

	{ "mas11",			0x080000, 0xc2f0a4f2, 9 | BRF_SND },           // 20 OKI M6295 Samples 1

	{ "mb7122h.16l",	0x000400, 0x2bee57cc, 0 | BRF_OPT },           // 21 Unused PROMs
};

STD_ROM_PICK(wizdfire)
STD_ROM_FN(wizdfire)

struct BurnDriver BurnDrvWizdfire = {
	"wizdfire", NULL, NULL, NULL, "1992",
	"Wizard Fire (Over Sea v2.1)\0", NULL, "Data East Corporation", "DECO IC16",
	L"\u6697\u9ED1\u5C01\u5370 II - \u5DEB\u5E08\u4E4B\u706B (\u6D77\u5916\u7248 v2.1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, wizdfireRomInfo, wizdfireRomName, NULL, NULL, NULL, NULL, WizdfireInputInfo, WizdfireDIPInfo,
	WizdfireEnglishInit, DrvExit, DrvFrame, WizdfireDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Wizard Fire (Over Sea v1.0)
 
 static struct BurnRomInfo wizdfireaRomDesc[] = {
 	{ "mm-01.3d",		0x020000, 0xebaecff7, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
 	{ "mm-00.3a",		0x020000, 0x56e05ec0, 1 | BRF_PRG | BRF_ESS }, //  1
 	{ "je-03.5d",		0x020000, 0x5217d404, 1 | BRF_PRG | BRF_ESS }, //  2
 	{ "je-02.5a",		0x020000, 0x36a1ce28, 1 | BRF_PRG | BRF_ESS }, //  3
 	{ "mas13",			0x080000, 0x7e5256ce, 1 | BRF_PRG | BRF_ESS }, //  4
 	{ "mas12",			0x080000, 0x005bd499, 1 | BRF_PRG | BRF_ESS }, //  5
 
 	{ "je-06.20r",		0x010000, 0x79042546, 2 | BRF_PRG | BRF_ESS }, //  6 Huc6280 Code
 
 	{ "je-04.10d",		0x010000, 0x73cba800, 3 | BRF_GRA },           //  7 Characters
 	{ "je-05.12d",		0x010000, 0x22e2c49d, 3 | BRF_GRA },           //  8
 
 	{ "mas00",			0x100000, 0x3d011034, 4 | BRF_GRA },           //  9 Foreground Tiles
 	{ "mas01",			0x100000, 0x6d0c9d0b, 4 | BRF_GRA },           // 10
 
 	{ "mas02",			0x080000, 0xaf00e620, 5 | BRF_GRA },           // 11 Background Tiles
 	{ "mas03",			0x080000, 0x2fe61ea2, 5 | BRF_GRA },           // 12
 
 	{ "mas04",			0x100000, 0x1e56953b, 6 | BRF_GRA },           // 13 Sprite Bank A
 	{ "mas05",			0x100000, 0x3826b8f8, 6 | BRF_GRA },           // 14
 	{ "mas06",			0x100000, 0x3b8bbd45, 6 | BRF_GRA },           // 15
 	{ "mas07",			0x100000, 0x31303769, 6 | BRF_GRA },           // 16
 
 	{ "mas08",			0x080000, 0xe224fb7a, 7 | BRF_GRA },           // 17 Sprite Bank B
 	{ "mas09",			0x080000, 0x5f6deb41, 7 | BRF_GRA },           // 18
 
 	{ "mas10",			0x100000, 0xf4b4c8a1, 8 | BRF_SND },           // 19 OKI M6295 Samples 0
 
 	{ "mas11",			0x080000, 0xc2f0a4f2, 9 | BRF_SND },           // 20 OKI M6295 Samples 1
 
 	{ "mb7122h.16l",	0x000400, 0x2bee57cc, 0 | BRF_OPT },           // 21 Unused PROMs
 };
 
 STD_ROM_PICK(wizdfirea)
 STD_ROM_FN(wizdfirea)
 
 struct BurnDriver BurnDrvWizdfirea = {
 	"wizdfirea", "wizdfire", NULL, NULL, "1992",
 	"Wizard Fire (Over Sea v1.0)\0", NULL, "Data East Corporation", "DECO IC16",
	L"\u6697\u9ED1\u5C01\u5370 II - \u5DEB\u5E08\u4E4B\u706B (\u6D77\u5916\u7248 v1.0)\0", NULL, NULL, NULL,
 	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
 	NULL, wizdfireaRomInfo, wizdfireaRomName, NULL, NULL, NULL, NULL, WizdfireInputInfo, WizdfireDIPInfo,
 	WizdfireEnglishInit, DrvExit, DrvFrame, WizdfireDraw, DrvScan, &DrvRecalc, 0x800,
 	320, 240, 4, 3
 };
 
 
// Wizard Fire (US v1.1)

static struct BurnRomInfo wizdfireuRomDesc[] = {
	{ "jf-01.3d",		0x020000, 0xbde42a41, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "jf-00.3a",		0x020000, 0xbca3c995, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "jf-03.5d",		0x020000, 0x5217d404, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "jf-02.5a",		0x020000, 0x36a1ce28, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "mas13",			0x080000, 0x7e5256ce, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "mas12",			0x080000, 0x005bd499, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "jf-06.20r",		0x010000, 0x79042546, 2 | BRF_PRG | BRF_ESS }, //  6 Huc6280 Code

	{ "jf-04.10d",		0x010000, 0x73cba800, 3 | BRF_GRA },           //  7 Characters
	{ "jf-05.12d",		0x010000, 0x22e2c49d, 3 | BRF_GRA },           //  8

	{ "mas00",			0x100000, 0x3d011034, 4 | BRF_GRA },           //  9 Foreground Tiles
	{ "mas01",			0x100000, 0x6d0c9d0b, 4 | BRF_GRA },           // 10

	{ "mas02",			0x080000, 0xaf00e620, 5 | BRF_GRA },           // 11 Background Tiles
	{ "mas03",			0x080000, 0x2fe61ea2, 5 | BRF_GRA },           // 12

	{ "mas04",			0x100000, 0x1e56953b, 6 | BRF_GRA },           // 13 Sprite Bank A
	{ "mas05",			0x100000, 0x3826b8f8, 6 | BRF_GRA },           // 14
	{ "mas06",			0x100000, 0x3b8bbd45, 6 | BRF_GRA },           // 15
	{ "mas07",			0x100000, 0x31303769, 6 | BRF_GRA },           // 16

	{ "mas08",			0x080000, 0xe224fb7a, 7 | BRF_GRA },           // 17 Sprite Bank B
	{ "mas09",			0x080000, 0x5f6deb41, 7 | BRF_GRA },           // 18

	{ "mas10",			0x100000, 0xf4b4c8a1, 8 | BRF_SND },           // 19 OKI M6295 Samples 0

	{ "mas11",			0x080000, 0xc2f0a4f2, 9 | BRF_SND },           // 20 OKI M6295 Samples 1

	{ "mb7122h.16l",	0x000400, 0x2bee57cc, 0 | BRF_OPT },           // 21 Unused PROMs
};

STD_ROM_PICK(wizdfireu)
STD_ROM_FN(wizdfireu)

struct BurnDriver BurnDrvWizdfireu = {
	"wizdfireu", "wizdfire", NULL, NULL, "1992",
	"Wizard Fire (US v1.1)\0", NULL, "Data East Corporation", "DECO IC16",
	L"\u6697\u9ED1\u5C01\u5370 II - \u5DEB\u5E08\u4E4B\u706B (\u7F8E\u7248 v1.1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, wizdfireuRomInfo, wizdfireuRomName, NULL, NULL, NULL, NULL, WizdfireInputInfo, WizdfireDIPInfo,
	WizdfireEnglishInit, DrvExit, DrvFrame, WizdfireDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Dark Seal 2 (Japan v2.1)

static struct BurnRomInfo darkseal2RomDesc[] = {
	{ "jb-01-3.3d",		0x020000, 0x82308c01, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "jb-00-3.3a",		0x020000, 0x1d38113a, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "jb-03.5d",		0x020000, 0x5217d404, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "jb-02.5a",		0x020000, 0x36a1ce28, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "mas13",			0x080000, 0x7e5256ce, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "mas12",			0x080000, 0x005bd499, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "jb-06.20r",		0x010000, 0x2066a1dd, 2 | BRF_PRG | BRF_ESS }, //  6 Huc6280 Code

	{ "jb-04.10d",		0x010000, 0x73cba800, 3 | BRF_GRA },           //  7 Characters
	{ "jb-05.12d",		0x010000, 0x22e2c49d, 3 | BRF_GRA },           //  8

	{ "mas00",			0x100000, 0x3d011034, 4 | BRF_GRA },           //  9 Foreground Tiles
	{ "mas01",			0x100000, 0x6d0c9d0b, 4 | BRF_GRA },           // 10

	{ "mas02",			0x080000, 0xaf00e620, 5 | BRF_GRA },           // 11 Background Tiles
	{ "mas03",			0x080000, 0x2fe61ea2, 5 | BRF_GRA },           // 12

	{ "mas04",			0x100000, 0x1e56953b, 6 | BRF_GRA },           // 13 Sprite Bank A
	{ "mas05",			0x100000, 0x3826b8f8, 6 | BRF_GRA },           // 14
	{ "mas06",			0x100000, 0x3b8bbd45, 6 | BRF_GRA },           // 15
	{ "mas07",			0x100000, 0x31303769, 6 | BRF_GRA },           // 16

	{ "mas08",			0x080000, 0xe224fb7a, 7 | BRF_GRA },           // 17 Sprite Bank B
	{ "mas09",			0x080000, 0x5f6deb41, 7 | BRF_GRA },           // 18

	{ "mas10",			0x100000, 0xf4b4c8a1, 8 | BRF_SND },           // 19 OKI M6295 Samples 0

	{ "mas11",			0x080000, 0xc2f0a4f2, 9 | BRF_SND },           // 20 OKI M6295 Samples 1

	{ "mb7122h.16l",	0x000400, 0x2bee57cc, 0 | BRF_OPT },           // 21 Unused PROMs
};

STD_ROM_PICK(darkseal2)
STD_ROM_FN(darkseal2)

struct BurnDriver BurnDrvDarkseal2 = {
	"darkseal2", "wizdfire", NULL, NULL, "1992",
	"Dark Seal 2 (Japan v2.1)\0", NULL, "Data East Corporation", "DECO IC16",
	L"\u6697\u9ED1\u5C01\u5370 II - \u5DEB\u5E08\u4E4B\u706B (\u65E5\u7248 v2.1)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, darkseal2RomInfo, darkseal2RomName, NULL, NULL, NULL, NULL, WizdfireInputInfo, WizdfireDIPInfo,
	WizdfireInit, DrvExit, DrvFrame, WizdfireDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Schmeiser Robo (Japan)

static struct BurnRomInfo schmeisrRomDesc[] = {
	{ "sr001j.8a",		0x080000, 0xed31f3ff, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "sr006j.8d",		0x080000, 0x9e9cfa5d, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "sr013.18p",		0x010000, 0x4ac00cbb, 2 | BRF_PRG | BRF_ESS }, //  2 Huc6280 Code

	{ "sr002-74.9a",	0x080000, 0x97e15c7b, 3 | BRF_GRA },           //  3 Foreground Tiles
	{ "sr003-74.11a",	0x080000, 0xea367971, 3 | BRF_GRA },           //  4

	{ "sr007.17d",		0x100000, 0x886f80c7, 4 | BRF_GRA },           //  5 Background Tiles
	{ "sr008.18d",		0x100000, 0xa74cbc90, 4 | BRF_GRA },           //  6

	{ "sr004.19a",		0x100000, 0xe25434a1, 5 | BRF_GRA },           //  7 Sprites
	{ "sr005.20a",		0x100000, 0x1630033b, 5 | BRF_GRA },           //  8
	{ "sr009.19d",		0x100000, 0x7b9d982f, 5 | BRF_GRA },           //  9
	{ "sr010.20d",		0x100000, 0x6e9e5352, 5 | BRF_GRA },           // 10

	{ "sr012.15p",		0x080000, 0x38843d4d, 6 | BRF_SND },           // 12 OKI M6295 Samples 0

	{ "sr011.14p",		0x080000, 0x81805616, 7 | BRF_SND },           // 11 OKI M6295 Samples 1

	{ "hb-00.11p",		0x000200, 0xb7a7baad, 0 | BRF_OPT },           // 13 Unused PROMs
};

STD_ROM_PICK(schmeisr)
STD_ROM_FN(schmeisr)

struct BurnDriver BurnDrvSchmeisr = {
	"schmeisr", NULL, NULL, NULL, "1993",
	"Schmeiser Robo (Japan)\0",NULL, "Hot-B", "DECO IC16",
	L"\u683C\u6597\u673A\u5668\u4EBA (\u65E5\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_PREFIX_DATAEAST, GBF_VSFIGHT, 0,
	NULL, schmeisrRomInfo, schmeisrRomName, NULL, NULL, NULL, NULL, RohgaInputInfo, SchmeisrDIPInfo,
	SchmeisrInit, DrvExit, DrvFrame, SchmeisrDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Hangzo (Japan, prototype)
/* Found on a Data East DE-0353-3 PCB */

static struct BurnRomInfo hangzoRomDesc[] = {
	{ "pro0h 12.18.2a.27c1001",	0x20000, 0xac8087db, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "pro0h 12.18.2d.27c1001",	0x20000, 0xa6b7f4f4, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "pro1h 12.10.4a.27c010",	0x20000, 0x0d04f43d, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "pro1l 12.10.4d.27c010",	0x20000, 0x2e323918, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "pro2h 12.10.6a.27c010",	0x20000, 0xbb3185a6, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "pro2l 12.10.6d.27c010",	0x20000, 0x11ce97bb, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "snd 12.18.18p.27c512",	0x10000, 0x97c592dc, 2 | BRF_PRG | BRF_ESS }, //  6 Huc6280 Code

	{ "bk1l 12.10.9a.574200",	0x80000, 0x5199729b, 3 | BRF_GRA },           //  7 Foreground Tiles
	{ "bk1h 12.10.11a.574200",	0x80000, 0x85887bd8, 3 | BRF_GRA },           //  8

	{ "bk23l 12.10.17d.574200",	0x80000, 0xed4e47c6, 4 | BRF_GRA },           //  9 Background Tiles
	{ "bk23h 12.10.18d.574200",	0x80000, 0x6a725fb2, 4 | BRF_GRA },           // 10

	{ "obj01l 12.10.19a.27c4000",	0x80000, 0xc141e310, 5 | BRF_GRA },		 // 11 Sprites
	{ "obj01h 12.10.20a.27c4000",	0x80000, 0x6a7b4252, 5 | BRF_GRA },       // 12
	{ "obj23l 12.10.19d.27c4000",	0x80000, 0x0db6df6c, 5 | BRF_GRA },       // 13
	{ "obj23h 12.10.20d.27c4000",	0x80000, 0x165031a1, 5 | BRF_GRA },       // 14

	{ "pcm8k 11.5.15p.27c020",	0x40000, 0x02682a9a, 6 | BRF_SND },           // 15 OKI M6295 Samples 0

	{ "pcm16k 11.5.14p.574000",	0x80000, 0x5b95c6c7, 7 | BRF_SND },           // 16 OKI M6295 Samples 1

	{ "hb-00.11p",		0x00200, 0xb7a7baad, 0 | BRF_OPT },         	      // 17 Unused PROMs
};

STD_ROM_PICK(hangzo)
STD_ROM_FN(hangzo)

struct BurnDriver BurnDrvHangzo = {
	"hangzo", NULL, NULL, NULL, "1992",
	"Hangzo (Japan, prototype)\0", NULL, "Hot-B", "DECO IC16",
	L"\u5FCD\u8005\u9664\u5996\u4F20 (\u65E5\u7248, \u5DE5\u7A0B\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_PROTOTYPE, 2, HARDWARE_PREFIX_DATAEAST, GBF_SCRFIGHT, 0,
	NULL, hangzoRomInfo, hangzoRomName, NULL, NULL, NULL, NULL, RohgaInputInfo, HangzoDIPInfo,
	HangzoInit, DrvExit, DrvFrame, SchmeisrDraw, DrvScan, &DrvRecalc, 0x800,
	320, 240, 4, 3
};


// Nitro Ball (World, set 1)

static struct BurnRomInfo nitrobalRomDesc[] = {
	{ "jl01-4.d3",		0x020000, 0x0414e409, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "jl00-4.b3",		0x020000, 0xdd9e2bcc, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "jl03-4.d5",		0x020000, 0xea264ac5, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "jl02-4.b5",		0x020000, 0x74047997, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "jl05-2.d6",		0x040000, 0xb820fa20, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "jl04-2.b6",		0x040000, 0x1fd8995b, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "jl08.r20",		0x010000, 0x93d93fe1, 2 | BRF_PRG | BRF_ESS }, //  6 Huc6280 Code

	{ "jl06.d10",		0x010000, 0x91cf668e, 3 | BRF_GRA },           //  7 Characters
	{ "jl07.d12",		0x010000, 0xe61d0e42, 3 | BRF_GRA },           //  8

	{ "mav00.b10",		0x080000, 0x34785d97, 4 | BRF_GRA },           //  9 Foreground Tiles
	{ "mav01.b12",		0x080000, 0x8b531b16, 4 | BRF_GRA },           // 10

	{ "mav02.b16",		0x100000, 0x20723bf7, 5 | BRF_GRA },           // 11 Background Tiles
	{ "mav03.e16",		0x100000, 0xef6195f0, 5 | BRF_GRA },           // 12

	{ "mav05.e19",		0x100000, 0xd92d769c, 6 | BRF_GRA },           // 13 Sprite Bank A
	{ "mav04.b19",		0x100000, 0x8ba48385, 6 | BRF_GRA },           // 14
	{ "mav07.e20",		0x080000, 0x5fc10ccd, 6 | BRF_GRA },           // 15
	{ "mav06.b20",		0x080000, 0xae6201a5, 6 | BRF_GRA },           // 16

	{ "mav09.e23",		0x040000, 0x1ce7b51a, 7 | BRF_GRA },           // 17 Sprite Bank B
	{ "mav08.b23",		0x040000, 0x64966576, 7 | BRF_GRA },           // 18

	{ "mav10.r17",		0x080000, 0x8ad734b0, 8 | BRF_SND },           // 19 OKI M6295 Samples 0

	{ "mav11.r19",		0x080000, 0xef513908, 9 | BRF_SND },           // 20 OKI M6295 Samples 1

	{ "jn-00.17l",		0x000400, 0x6ac77b84, 0 | BRF_OPT },           // 21 Unused PROMs
};

STD_ROM_PICK(nitrobal)
STD_ROM_FN(nitrobal)

struct BurnDriver BurnDrvNitrobal = {
	"nitrobal", NULL, NULL, NULL, "1992",
	"Nitro Ball (World, set 1)\0", NULL, "Data East Corporation", "DECO IC16",
	L"\u5730\u72F1\u82F1\u96C4\u5F39\u73E0\u95E8 (\u4E16\u754C\u7248, \u7B2C\u4E00\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 3, HARDWARE_PREFIX_DATAEAST, GBF_RUNGUN, 0,
	NULL, nitrobalRomInfo, nitrobalRomName, NULL, NULL, NULL, NULL, NitrobalInputInfo, NitrobalDIPInfo,
	NitrobalInit, DrvExit, DrvFrame, NitrobalDraw, DrvScan, &DrvRecalc, 0x800,
	240, 320, 3, 4
};


// Nitro Ball (World, set 2)

static struct BurnRomInfo nitrobalaRomDesc[] = {
	// roms had no labels
	{ "3d",				0x020000, 0x48f77c19, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "3b",				0x020000, 0xfb1284e9, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "5d",				0x020000, 0xac47367a, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "5b",				0x020000, 0xa8e9d7dd, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "6d",				0x040000, 0x59e620cc, 1 | BRF_PRG | BRF_ESS }, //  4 // 7ee3 - 7f06 are 0xff instead of 0x00 in the nitrobal set, why?
	{ "6b",				0x040000, 0x1fd8995b, 1 | BRF_PRG | BRF_ESS }, //  5 // this half of the pair matches
	/* Two empty rom slots at d7, b7 */

	{ "jl08.r20",		0x010000, 0x93d93fe1, 2 | BRF_PRG | BRF_ESS }, //  6 Huc6280 Code

	{ "jl06.d10",		0x010000, 0x91cf668e, 3 | BRF_GRA },           //  7 Characters
	{ "jl07.d12",		0x010000, 0xe61d0e42, 3 | BRF_GRA },           //  8

	{ "mav00.b10",		0x080000, 0x34785d97, 4 | BRF_GRA },           //  9 Foreground Tiles
	{ "mav01.b12",		0x080000, 0x8b531b16, 4 | BRF_GRA },           // 10

	{ "mav02.b16",		0x100000, 0x20723bf7, 5 | BRF_GRA },           // 11 Background Tiles
	{ "mav03.e16",		0x100000, 0xef6195f0, 5 | BRF_GRA },           // 12

	{ "mav05.e19",		0x100000, 0xd92d769c, 6 | BRF_GRA },           // 13 Sprite Bank A
	{ "mav04.b19",		0x100000, 0x8ba48385, 6 | BRF_GRA },           // 14
	{ "mav07.e20",		0x080000, 0x5fc10ccd, 6 | BRF_GRA },           // 15
	{ "mav06.b20",		0x080000, 0xae6201a5, 6 | BRF_GRA },           // 16

	{ "mav09.e23",		0x040000, 0x1ce7b51a, 7 | BRF_GRA },           // 17 Sprite Bank B
	{ "mav08.b23",		0x040000, 0x64966576, 7 | BRF_GRA },           // 18

	{ "mav10.r17",		0x080000, 0x8ad734b0, 8 | BRF_SND },           // 19 OKI M6295 Samples 0

	{ "mav11.r19",		0x080000, 0xef513908, 9 | BRF_SND },           // 20 OKI M6295 Samples 1

	{ "jn-00.17l",		0x000400, 0x6ac77b84, 0 | BRF_OPT },           // 21 Unused PROMs
};

STD_ROM_PICK(nitrobala)
STD_ROM_FN(nitrobala)

struct BurnDriver BurnDrvNitrobala = {
	"nitrobala", "nitrobal", NULL, NULL, "1992",
	"Nitro Ball (World, set 2)\0", NULL, "Data East Corporation", "DECO IC16",
	L"\u5730\u72F1\u82F1\u96C4\u5F39\u73E0\u95E8 (\u4E16\u754C\u7248, \u7B2C\u4E8C\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 3, HARDWARE_PREFIX_DATAEAST, GBF_RUNGUN, 0,
	NULL, nitrobalaRomInfo, nitrobalaRomName, NULL, NULL, NULL, NULL, NitrobalInputInfo, NitrobalDIPInfo,
	NitrobalInit, DrvExit, DrvFrame, NitrobalDraw, DrvScan, &DrvRecalc, 0x800,
	240, 320, 3, 4
};


// Nitro Ball (World, set 3)

static struct BurnRomInfo nitrobalbRomDesc[] = {
	{ "mm-01.d3",		0x020000, 0x81ca7ef0, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "mm-00.b3",		0x020000, 0xe3416c4f, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "mm-03.d5",		0x020000, 0xed9792aa, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "mm-02.b5",		0x020000, 0x9a568ed3, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "mm-05.d6",		0x040000, 0xb820fa20, 1 | BRF_PRG | BRF_ESS }, //  4 
	{ "mm-04.b6",		0x040000, 0x1fd8995b, 1 | BRF_PRG | BRF_ESS }, //  5 
	/* Two empty rom slots at d7, b7 */

	{ "mm-08.r20",		0x010000, 0x93d93fe1, 2 | BRF_PRG | BRF_ESS }, //  6 Huc6280 Code

	{ "mm-06.d10",		0x010000, 0x91cf668e, 3 | BRF_GRA },           //  7 Characters
	{ "mm-07.d12",		0x010000, 0xe61d0e42, 3 | BRF_GRA },           //  8

	{ "mav00.b10",		0x080000, 0x34785d97, 4 | BRF_GRA },           //  9 Foreground Tiles
	{ "mav01.b12",		0x080000, 0x8b531b16, 4 | BRF_GRA },           // 10

	{ "mav02.b16",		0x100000, 0x20723bf7, 5 | BRF_GRA },           // 11 Background Tiles
	{ "mav03.e16",		0x100000, 0xef6195f0, 5 | BRF_GRA },           // 12

	{ "mav05.e19",		0x100000, 0xd92d769c, 6 | BRF_GRA },           // 13 Sprite Bank A
	{ "mav04.b19",		0x100000, 0x8ba48385, 6 | BRF_GRA },           // 14
	{ "mav07.e20",		0x080000, 0x5fc10ccd, 6 | BRF_GRA },           // 15
	{ "mav06.b20",		0x080000, 0xae6201a5, 6 | BRF_GRA },           // 16

	{ "mav09.e23",		0x040000, 0x1ce7b51a, 7 | BRF_GRA },           // 17 Sprite Bank B
	{ "mav08.b23",		0x040000, 0x64966576, 7 | BRF_GRA },           // 18

	{ "mav10.r17",		0x080000, 0x8ad734b0, 8 | BRF_SND },           // 19 OKI M6295 Samples 0

	{ "mav11.r19",		0x080000, 0xef513908, 9 | BRF_SND },           // 20 OKI M6295 Samples 1

	{ "jn-00.17l",		0x000400, 0x6ac77b84, 0 | BRF_OPT },           // 21 Unused PROMs
};

STD_ROM_PICK(nitrobalb)
STD_ROM_FN(nitrobalb)

struct BurnDriver BurnDrvNitrobalb = {
	"nitrobalb", "nitrobal", NULL, NULL, "1992",
	"Nitro Ball (World, set 3)\0", NULL, "Data East Corporation", "DECO IC16",
	L"\u5730\u72F1\u82F1\u96C4\u5F39\u73E0\u95E8 (\u4E16\u754C\u7248, \u7B2C\u4E09\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 3, HARDWARE_PREFIX_DATAEAST, GBF_RUNGUN, 0,
	NULL, nitrobalbRomInfo, nitrobalbRomName, NULL, NULL, NULL, NULL, NitrobalInputInfo, NitrobalDIPInfo,
	NitrobalInit, DrvExit, DrvFrame, NitrobalDraw, DrvScan, &DrvRecalc, 0x800,
	240, 320, 3, 4
};

// Gun Ball (Japan)

static struct BurnRomInfo gunballRomDesc[] = {
	{ "jc01.3d",		0x020000, 0x61bfa998, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "jc00.3b",		0x020000, 0x73ba8f74, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "jc03.5d",		0x020000, 0x19231612, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "jc02.5b",		0x020000, 0xa254f34c, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "jc05-3.6d",		0x040000, 0xf750a709, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "jc04-3.6b",		0x040000, 0xad711767, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "jl08.r20",		0x010000, 0x93d93fe1, 2 | BRF_PRG | BRF_ESS }, //  6 Huc6280 Code

	{ "jl06.d10",		0x010000, 0x91cf668e, 3 | BRF_GRA },           //  7 Characters
	{ "jl07.d12",		0x010000, 0xe61d0e42, 3 | BRF_GRA },           //  8

	{ "mav00.b10",		0x080000, 0x34785d97, 4 | BRF_GRA },           //  9 Foreground Tiles
	{ "mav01.b12",		0x080000, 0x8b531b16, 4 | BRF_GRA },           // 10

	{ "mav02.b16",		0x100000, 0x20723bf7, 5 | BRF_GRA },           // 11 Background Tiles
	{ "mav03.e16",		0x100000, 0xef6195f0, 5 | BRF_GRA },           // 12

	{ "mav05.e19",		0x100000, 0xd92d769c, 6 | BRF_GRA },           // 13 Sprite Bank A
	{ "mav04.b19",		0x100000, 0x8ba48385, 6 | BRF_GRA },           // 14
	{ "mav07.e20",		0x080000, 0x5fc10ccd, 6 | BRF_GRA },           // 15
	{ "mav06.b20",		0x080000, 0xae6201a5, 6 | BRF_GRA },           // 16

	{ "mav09.e23",		0x040000, 0x1ce7b51a, 7 | BRF_GRA },           // 17 Sprite Bank B
	{ "mav08.b23",		0x040000, 0x64966576, 7 | BRF_GRA },           // 18

	{ "mav10.r17",		0x080000, 0x8ad734b0, 8 | BRF_SND },           // 19 OKI M6295 Samples 0

	{ "mav11.r19",		0x080000, 0xef513908, 9 | BRF_SND },           // 20 OKI M6295 Samples 1

	{ "jn-00.17l",		0x000400, 0x6ac77b84, 0 | BRF_OPT },           // 21 Unused PROMs
};

STD_ROM_PICK(gunball)
STD_ROM_FN(gunball)

struct BurnDriver BurnDrvGunball = {
	"gunball", "nitrobal", NULL, NULL, "1992",
	"Gun Ball (Japan)\0", NULL, "Data East Corporation", "DECO IC16",
	L"\u5730\u72F1\u82F1\u96C4\u5F39\u73E0\u95E8 (\u65E5\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 3, HARDWARE_PREFIX_DATAEAST, GBF_RUNGUN, 0,
	NULL, gunballRomInfo, gunballRomName, NULL, NULL, NULL, NULL, NitrobalInputInfo, NitrobalDIPInfo,
	NitrobalInit, DrvExit, DrvFrame, NitrobalDraw, DrvScan, &DrvRecalc, 0x800,
	240, 320, 3, 4
};
