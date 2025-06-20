// FB Alpha Street Fighter driver module
// Based on MAME driver by Olivier Galibert

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "burn_ym2151.h"
#include "msm5205.h"
#include "dtimer.h"

static UINT8 *AllMem;
static UINT8 *MemEnd;
static UINT8 *AllRam;
static UINT8 *RamEnd;
static UINT8 *Drv68kRom;
static UINT8 *DrvZ80Rom0;
static UINT8 *DrvZ80Rom1;
static UINT8 *Drv68kPalRam;
static UINT8 *Drv68kVidRam;
static UINT8 *Drv68kRam;
static UINT8 *DrvZ80Ram0;
static UINT8 *Gfx0;
static UINT8 *Gfx1;
static UINT8 *Gfx2;
static UINT8 *Gfx3;
static UINT8 *Gfx4;
static UINT8 *Gfx1Trans;
static UINT8 *Gfx3Trans;

static UINT32 *DrvPalette;
static UINT8 DrvRecalc;

static UINT8 DrvJoy1[16];
static UINT8 DrvJoy2[16];
static UINT8 DrvJoy3[16];
static UINT8 DrvJoy4[16];
static UINT8 DrvJoy5[16];
static UINT8 DrvDips[ 4];
static UINT8 DrvReset;
static UINT16 DrvInputs[8];

static INT32 version;

static INT32 sf_fg_scroll_x;
static INT32 sf_bg_scroll_x;

static INT32 soundlatch;
static INT32 flipscreen;

static INT32 sf_active;
static INT32 sound2_bank;

static dtimer sndtimer;

static struct BurnInputInfo SfInputList[] = {
	{"P1 Coin"       , BIT_DIGITAL  , DrvJoy2 + 0,	"p1 coin"  },
	{"P1 Start"  ,    BIT_DIGITAL  , DrvJoy1 + 0,	"p1 start" },
	{"P1 Up",	  BIT_DIGITAL,   DrvJoy3 + 3,   "p1 up",   },
	{"P1 Down",	  BIT_DIGITAL,   DrvJoy3 + 2,   "p1 down", },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy3 + 1, 	"p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy3 + 0, 	"p1 right" },
	{"P1 Weak Punch"  , BIT_DIGITAL  , DrvJoy4 + 0,	"p1 fire 1"},
	{"P1 Medium Punch"  , BIT_DIGITAL  , DrvJoy4 + 1,	"p1 fire 2"},
	{"P1 Strong Punch"  , BIT_DIGITAL  , DrvJoy4 + 2,	"p1 fire 3"},
	{"P1 Weak Kick"  , BIT_DIGITAL  , DrvJoy4 + 8,	"p1 fire 4"},
	{"P1 Medium Kick"  , BIT_DIGITAL  , DrvJoy4 + 9,	"p1 fire 5"},
	{"P1 Strong Kick"  , BIT_DIGITAL  , DrvJoy4 + 10,	"p1 fire 6"},

	{"P2 Coin"       , BIT_DIGITAL  , DrvJoy2 + 1,	"p2 coin"  },
	{"P2 Start"  ,    BIT_DIGITAL  , DrvJoy1 + 1,	"p2 start" },
	{"P2 Up",	  BIT_DIGITAL,   DrvJoy3 + 11,  "p2 up",   },
	{"P2 Down",	  BIT_DIGITAL,   DrvJoy3 + 10,  "p2 down", },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy3 + 9, 	"p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy3 + 8, 	"p2 right" },
	{"P2 Weak Punch"  , BIT_DIGITAL  , DrvJoy5 + 0,	"p2 fire 1"},
	{"P2 Medium Punch"  , BIT_DIGITAL  , DrvJoy5 + 1,	"p2 fire 2"},
	{"P2 Strong Punch"  , BIT_DIGITAL  , DrvJoy5 + 2,	"p2 fire 3"},
	{"P2 Weak Kick"  , BIT_DIGITAL  , DrvJoy5 + 8,	"p2 fire 4"},
	{"P2 Medium Kick"  , BIT_DIGITAL  , DrvJoy5 + 9,	"p2 fire 5"},
	{"P2 Strong Kick"  , BIT_DIGITAL  , DrvJoy5 + 10,	"p2 fire 6"},

	{"Service"      , BIT_DIGITAL  , DrvJoy1 + 2,	"service"  },

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	"dip"	   },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	"dip"	   },
	{"Dip 3",	  BIT_DIPSWITCH, DrvDips + 2,	"dip"	   },
	{"Dip 4",	  BIT_DIPSWITCH, DrvDips + 3,	"dip"	   },
};

STDINPUTINFO(Sf)

static struct BurnInputInfo SfjInputList[] = {
	{"P1 Coin"       , BIT_DIGITAL  , DrvJoy2 + 0,	"p1 coin"  },
	{"P1 Start"  ,    BIT_DIGITAL  , DrvJoy1 + 0,	"p1 start" },
	{"P1 Up",	  BIT_DIGITAL,   DrvJoy3 + 3,   "p1 up",   },
	{"P1 Down",	  BIT_DIGITAL,   DrvJoy3 + 2,   "p1 down", },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy3 + 1, 	"p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy3 + 0, 	"p1 right" },
	{"P1 Weak Punch"  , BIT_DIGITAL  , DrvJoy3 + 8,	"p1 fire 1"},
	{"P1 Medium Punch"  , BIT_DIGITAL  , DrvJoy3 + 9,	"p1 fire 2"},
	{"P1 Strong Punch"  , BIT_DIGITAL  , DrvJoy3 + 10,	"p1 fire 3"},
	{"P1 Weak Kick"  , BIT_DIGITAL  , DrvJoy3 + 12,	"p1 fire 4"},
	{"P1 Medium Kick"  , BIT_DIGITAL  , DrvJoy3 + 13,	"p1 fire 5"},
	{"P1 Strong Kick"  , BIT_DIGITAL  , DrvJoy3 + 14,	"p1 fire 6"},

	{"P2 Coin"       , BIT_DIGITAL  , DrvJoy2 + 1,	"p2 coin"  },
	{"P2 Start"  ,    BIT_DIGITAL  , DrvJoy1 + 1,	"p2 start" },
	{"P2 Up",	  BIT_DIGITAL,   DrvJoy4 + 3,   "p1 up",   },
	{"P2 Down",	  BIT_DIGITAL,   DrvJoy4 + 2,   "p2 down", },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy4 + 1, 	"p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy4 + 0, 	"p2 right" },
	{"P2 Weak Punch"  , BIT_DIGITAL  , DrvJoy4 + 8,	"p2 fire 1"},
	{"P2 Medium Punch"  , BIT_DIGITAL  , DrvJoy4 + 9,	"p2 fire 2"},
	{"P2 Strong Punch"  , BIT_DIGITAL  , DrvJoy4 + 10,	"p2 fire 3"},
	{"P2 Weak Kick"  , BIT_DIGITAL  , DrvJoy4 + 12,	"p2 fire 4"},
	{"P2 Medium Kick"  , BIT_DIGITAL  , DrvJoy4 + 13,	"p2 fire 5"},
	{"P2 Strong Kick"  , BIT_DIGITAL  , DrvJoy4 + 14,	"p2 fire 6"},

	{"Service"      , BIT_DIGITAL  , DrvJoy1 + 2,	"service"  },

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	"dip"	   },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	"dip"	   },
	{"Dip 3",	  BIT_DIPSWITCH, DrvDips + 2,	"dip"	   },
	{"Dip 4",	  BIT_DIPSWITCH, DrvDips + 3,	"dip"	   },
};

STDINPUTINFO(Sfj)

static struct BurnInputInfo SfusInputList[] = {
	{"P1 Coin"       , BIT_DIGITAL  , DrvJoy2 + 0,	"p1 coin"  },
	{"P1 Start"  ,    BIT_DIGITAL  , DrvJoy1 + 0,	"p1 start" },
	{"P1 Up",	  BIT_DIGITAL,   DrvJoy3 + 3,   "p1 up",   },
	{"P1 Down",	  BIT_DIGITAL,   DrvJoy3 + 2,   "p1 down", },
	{"P1 Left"      , BIT_DIGITAL  , DrvJoy3 + 1, 	"p1 left"  },
	{"P1 Right"     , BIT_DIGITAL  , DrvJoy3 + 0, 	"p1 right" },
	{"P1 Weak Punch"  , BIT_DIGITAL  , DrvJoy3 + 4,	"p1 fire 1"},
	{"P1 Medium Punch"  , BIT_DIGITAL  , DrvJoy3 + 5,	"p1 fire 2"},
	{"P1 Strong Punch"  , BIT_DIGITAL  , DrvJoy2 + 9,	"p1 fire 3"},
	{"P1 Weak Kick"  , BIT_DIGITAL  , DrvJoy3 + 6,	"p1 fire 4"},
	{"P1 Medium Kick"  , BIT_DIGITAL  , DrvJoy3 + 7,	"p1 fire 5"},
	{"P1 Strong Kick"  , BIT_DIGITAL  , DrvJoy2 + 2,	"p1 fire 6"},

	{"P2 Coin"       , BIT_DIGITAL  , DrvJoy2 + 1,	"p2 coin"  },
	{"P2 Start"  ,    BIT_DIGITAL  , DrvJoy1 + 1,	"p2 start" },
	{"P2 Up",	  BIT_DIGITAL,   DrvJoy3 + 11,  "p1 up",   },
	{"P2 Down",	  BIT_DIGITAL,   DrvJoy3 + 10,  "p2 down", },
	{"P2 Left"      , BIT_DIGITAL  , DrvJoy3 + 9, 	"p2 left"  },
	{"P2 Right"     , BIT_DIGITAL  , DrvJoy3 + 8, 	"p2 right" },
	{"P2 Weak Punch"  , BIT_DIGITAL  , DrvJoy3 + 12,	"p2 fire 1"},
	{"P2 Medium Punch"  , BIT_DIGITAL  , DrvJoy3 + 13,	"p2 fire 2"},
	{"P2 Strong Punch"  , BIT_DIGITAL  , DrvJoy2 + 10,	"p2 fire 3"},
	{"P2 Weak Kick"  , BIT_DIGITAL  , DrvJoy3 + 14,	"p2 fire 4"},
	{"P2 Medium Kick"  , BIT_DIGITAL  , DrvJoy3 + 15,	"p2 fire 5"},
	{"P2 Strong Kick"  , BIT_DIGITAL  , DrvJoy2 + 8,	"p2 fire 6"},

	{"Service"      , BIT_DIGITAL  , DrvJoy1 + 2,	"service"  },

	{"Reset",	  BIT_DIGITAL  , &DrvReset,	"reset"    },
	{"Dip 1",	  BIT_DIPSWITCH, DrvDips + 0,	"dip"	   },
	{"Dip 2",	  BIT_DIPSWITCH, DrvDips + 1,	"dip"	   },
	{"Dip 3",	  BIT_DIPSWITCH, DrvDips + 2,	"dip"	   },
	{"Dip 4",	  BIT_DIPSWITCH, DrvDips + 3,	"dip"	   },
};

STDINPUTINFO(Sfus)

static struct BurnDIPInfo SfDIPList[]=
{
	{0x1a, 0xff, 0xff, 0xff, NULL				},
	{0x1b, 0xff, 0xff, 0xff, NULL				},
	{0x1c, 0xff, 0xff, 0xf8, NULL				},
	{0x1d, 0xff, 0xff, 0x7f, NULL				},

	{0   , 0xfe, 0   , 2   , "Attract Music"		},
	{0x1a, 0x01, 0x02, 0x00, "Off"				},
	{0x1a, 0x01, 0x02, 0x02, "On"				},

	{0   , 0xfe, 0   , 2   , "Speed"			},
	{0x1a, 0x01, 0x10, 0x00, "Slow"				},
	{0x1a, 0x01, 0x10, 0x10, "Normal"			},

	{0   , 0xfe, 0   , 2   , "Demo Sounds"			},
	{0x1a, 0x01, 0x20, 0x20, "Off"				},
	{0x1a, 0x01, 0x20, 0x00, "On"				},

	{0   , 0xfe, 0   , 2   , "Freeze"			},
	{0x1a, 0x01, 0x40, 0x40, "Off"				},
	{0x1a, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   , 8   , "Coin A"			},
	{0x1b, 0x01, 0x07, 0x00, "4 Coins 1 Credit"		},
	{0x1b, 0x01, 0x07, 0x01, "3 Coins 1 Credit"		},
	{0x1b, 0x01, 0x07, 0x02, "2 Coins 1 Credit"		},
	{0x1b, 0x01, 0x07, 0x07, "1 Coin  1 Credit"		},
	{0x1b, 0x01, 0x07, 0x06, "1 Coin  2 Credits"		},
	{0x1b, 0x01, 0x07, 0x05, "1 Coin  3 Credits"		},
	{0x1b, 0x01, 0x07, 0x04, "1 Coin  4 Credits"		},
	{0x1b, 0x01, 0x07, 0x03, "1 Coin  6 Credits"		},

	{0   , 0xfe, 0   , 8   , "Coin B"			},
	{0x1b, 0x01, 0x38, 0x00, "4 Coins 1 Credit"		},
	{0x1b, 0x01, 0x38, 0x08, "3 Coins 1 Credit"		},
	{0x1b, 0x01, 0x38, 0x10, "2 Coins 1 Credit"		},
	{0x1b, 0x01, 0x38, 0x38, "1 Coin  1 Credit"		},
	{0x1b, 0x01, 0x38, 0x30, "1 Coin  2 Credits"		},
	{0x1b, 0x01, 0x38, 0x28, "1 Coin  3 Credits"		},
	{0x1b, 0x01, 0x38, 0x20, "1 Coin  4 Credits"		},
	{0x1b, 0x01, 0x38, 0x18, "1 Coin  6 Credits"		},

	{0   , 0xfe, 0   , 6   , "Buy-In Feature"		},
	{0x1c, 0x01, 0x38, 0x38, "5th Stage Maximum"	},
	{0x1c, 0x01, 0x38, 0x30, "4th Stage Maximum"	},
	{0x1c, 0x01, 0x38, 0x28, "3rd Stage Maximum"	},
	{0x1c, 0x01, 0x38, 0x20, "2nd Stage Maximum"	},
	{0x1c, 0x01, 0x38, 0x18, "1st Stage Maximum"	},
	{0x1c, 0x01, 0x38, 0x08, "None"				},

	{0   , 0xfe, 0   , 2   , "Number of Countries Selected"		},
	{0x1c, 0x01, 0x40, 0x40, "4"				},
	{0x1c, 0x01, 0x40, 0x00, "2"				},

	{0   , 0xfe, 0   , 6   , "Game Continuation"	},
	{0x1d, 0x01, 0x07, 0x07, "5th Stage Maximum"	},
	{0x1d, 0x01, 0x07, 0x06, "4th Stage Maximum"	},
	{0x1d, 0x01, 0x07, 0x05, "3rd Stage Maximum"	},
	{0x1d, 0x01, 0x07, 0x04, "2nd Stage Maximum"	},
	{0x1d, 0x01, 0x07, 0x03, "1st Stage Maximum"	},
	{0x1d, 0x01, 0x07, 0x02, "None"				},

	{0   , 0xfe, 0   , 4   , "Round Time Count"		},
	{0x1d, 0x01, 0x18, 0x18, "100"				},
	{0x1d, 0x01, 0x18, 0x10, "150"				},
	{0x1d, 0x01, 0x18, 0x08, "200"				},
	{0x1d, 0x01, 0x18, 0x00, "250"				},

	{0   , 0xfe, 0   , 4   , "Difficulty"			},
	{0x1d, 0x01, 0x60, 0x60, "Normal"			},
	{0x1d, 0x01, 0x60, 0x40, "Easy"				},
	{0x1d, 0x01, 0x60, 0x20, "Difficult"			},
	{0x1d, 0x01, 0x60, 0x00, "Very difficult"		},
};

STDDIPINFO(Sf)

static struct BurnDIPInfo SfusDIPList[]=
{
	{0x1a, 0xff, 0xff, 0xff, NULL				},
	{0x1b, 0xff, 0xff, 0xff, NULL				},
	{0x1c, 0xff, 0xff, 0xf8, NULL				},
	{0x1d, 0xff, 0xff, 0x7f, NULL				},

	{0   , 0xfe, 0   , 2   , "Flip Screen"			},
	{0x1a, 0x01, 0x01, 0x01, "Off"				},
	{0x1a, 0x01, 0x01, 0x00, "On"				},

	{0   , 0xfe, 0   , 2   , "Attract Music"		},
	{0x1a, 0x01, 0x02, 0x00, "Off"				},
	{0x1a, 0x01, 0x02, 0x02, "On"				},

	{0   , 0xfe, 0   , 2   , "Speed"			},
	{0x1a, 0x01, 0x10, 0x00, "Slow"				},
	{0x1a, 0x01, 0x10, 0x10, "Normal"			},

	{0   , 0xfe, 0   , 2   , "Demo Sounds"			},
	{0x1a, 0x01, 0x20, 0x20, "Off"				},
	{0x1a, 0x01, 0x20, 0x00, "On"				},

	{0   , 0xfe, 0   , 2   , "Freeze"			},
	{0x1a, 0x01, 0x40, 0x40, "Off"				},
	{0x1a, 0x01, 0x40, 0x00, "On"				},

	{0   , 0xfe, 0   , 8   , "Coin A"			},
	{0x1b, 0x01, 0x07, 0x00, "4 Coins 1 Credit"		},
	{0x1b, 0x01, 0x07, 0x01, "3 Coins 1 Credit"		},
	{0x1b, 0x01, 0x07, 0x02, "2 Coins 1 Credit"		},
	{0x1b, 0x01, 0x07, 0x07, "1 Coin  1 Credit"		},
	{0x1b, 0x01, 0x07, 0x06, "1 Coin  2 Credits"		},
	{0x1b, 0x01, 0x07, 0x05, "1 Coin  3 Credits"		},
	{0x1b, 0x01, 0x07, 0x04, "1 Coin  4 Credits"		},
	{0x1b, 0x01, 0x07, 0x03, "1 Coin  6 Credits"		},

	{0   , 0xfe, 0   , 8   , "Coin B"			},
	{0x1b, 0x01, 0x38, 0x00, "4 Coins 1 Credit"		},
	{0x1b, 0x01, 0x38, 0x08, "3 Coins 1 Credit"		},
	{0x1b, 0x01, 0x38, 0x10, "2 Coins 1 Credit"		},
	{0x1b, 0x01, 0x38, 0x38, "1 Coin  1 Credit"		},
	{0x1b, 0x01, 0x38, 0x30, "1 Coin  2 Credits"		},
	{0x1b, 0x01, 0x38, 0x28, "1 Coin  3 Credits"		},
	{0x1b, 0x01, 0x38, 0x20, "1 Coin  4 Credits"		},
	{0x1b, 0x01, 0x38, 0x18, "1 Coin  6 Credits"		},

	{0   , 0xfe, 0   , 6   , "Buy-In Feature"		},
	{0x1c, 0x01, 0x38, 0x38, "5th Stage Maximum"	},
	{0x1c, 0x01, 0x38, 0x30, "4th Stage Maximum"	},
	{0x1c, 0x01, 0x38, 0x28, "3rd Stage Maximum"	},
	{0x1c, 0x01, 0x38, 0x20, "2nd Stage Maximum"	},
	{0x1c, 0x01, 0x38, 0x18, "1st Stage Maximum"	},
	{0x1c, 0x01, 0x38, 0x08, "None"				},

	{0   , 0xfe, 0   , 2   , "Number of Countries Selected"		},
	{0x1c, 0x01, 0x40, 0x00, "4"				},
	{0x1c, 0x01, 0x40, 0x40, "2"				},

	{0   , 0xfe, 0   , 6   , "Game Continuation"	},
	{0x1d, 0x01, 0x07, 0x07, "5th Stage Maximum"	},
	{0x1d, 0x01, 0x07, 0x06, "4th Stage Maximum"	},
	{0x1d, 0x01, 0x07, 0x05, "3rd Stage Maximum"	},
	{0x1d, 0x01, 0x07, 0x04, "2nd Stage Maximum"	},
	{0x1d, 0x01, 0x07, 0x03, "1st Stage Maximum"	},
	{0x1d, 0x01, 0x07, 0x02, "None"				},

	{0   , 0xfe, 0   , 4   , "Round Time Count"			},
	{0x1d, 0x01, 0x18, 0x18, "100"				},
	{0x1d, 0x01, 0x18, 0x10, "150"				},
	{0x1d, 0x01, 0x18, 0x08, "200"				},
	{0x1d, 0x01, 0x18, 0x00, "250"				},

	{0   , 0xfe, 0   , 4   , "Difficulty"			},
	{0x1d, 0x01, 0x60, 0x60, "Normal"			},
	{0x1d, 0x01, 0x60, 0x40, "Easy"				},
	{0x1d, 0x01, 0x60, 0x20, "Difficult"			},
	{0x1d, 0x01, 0x60, 0x00, "Very difficult"		},
};

STDDIPINFO(Sfus)

static void protection_w()
{
	static const INT32 maplist[4][10] = {
		{ 1, 0, 3, 2, 4, 5, 6, 7, 8, 9 },
		{ 4, 5, 6, 7, 1, 0, 3, 2, 8, 9 },
		{ 3, 2, 1, 0, 6, 7, 4, 5, 8, 9 },
		{ 6, 7, 4, 5, 3, 2, 1, 0, 8, 9 }
	};
	INT32 map;

	map = maplist[SekReadByte(0xffc006)][(SekReadByte(0xffc003)<<1) + (SekReadWord(0xffc004)>>8)];

	switch (SekReadByte(0xffc684))
	{
		case 1:
		{
			INT32 base;

			base = 0x1b6e8+0x300e*map;

			SekWriteLong(0xffc01c, 0x16bfc+0x270*map);
			SekWriteLong(0xffc020, base+0x80);
			SekWriteLong(0xffc024, base);
			SekWriteLong(0xffc028, base+0x86);
			SekWriteLong(0xffc02c, base+0x8e);
			SekWriteLong(0xffc030, base+0x20e);
			SekWriteLong(0xffc034, base+0x30e);
			SekWriteLong(0xffc038, base+0x38e);
			SekWriteLong(0xffc03c, base+0x40e);
			SekWriteLong(0xffc040, base+0x80e);
			SekWriteLong(0xffc044, base+0xc0e);
			SekWriteLong(0xffc048, base+0x180e);
			SekWriteLong(0xffc04c, base+0x240e);
			SekWriteLong(0xffc050, 0x19548+0x60*map);
			SekWriteLong(0xffc054, 0x19578+0x60*map);
			break;
		}

		case 2:
		{
			static const INT32 delta1[10] = {
				0x1f80, 0x1c80, 0x2700, 0x2400, 0x2b80, 0x2e80, 0x3300, 0x3600, 0x3a80, 0x3d80
			};
			static const INT32 delta2[10] = {
				0x2180, 0x1800, 0x3480, 0x2b00, 0x3e00, 0x4780, 0x5100, 0x5a80, 0x6400, 0x6d80
			};

			INT32 d1 = delta1[map] + 0xc0;
			INT32 d2 = delta2[map];

			SekWriteWord(0xffc680, d1);
			SekWriteWord(0xffc682, d2);
			SekWriteWord(0xffc00c, 0xc0);
			SekWriteWord(0xffc00e, 0);

			sf_fg_scroll_x = d1;
			sf_bg_scroll_x = d2;

			break;
		}

		case 4:
		{
			INT32 pos = SekReadByte(0xffc010);
			pos = (pos+1) & 3;
			SekWriteByte(0xffc010, pos);

			if(!pos) {
				INT32 d1 = SekReadWord(0xffc682);
				INT32 off = SekReadWord(0xffc00e);
				if(off!=512) {
					off++;
					d1++;
				} else {
					off = 0;
					d1 -= 512;
				}
				SekWriteWord(0xffc682, d1);
				SekWriteWord(0xffc00e, off);

				sf_bg_scroll_x = d1;
			}
			break;
		}
	}
}

static inline void palette_update(INT32 offset)
{
	UINT16 data = *((UINT16*)(Drv68kPalRam + (offset & 0x7fe)));

	INT32 r = (data >> 8) & 0x0f;
	INT32 g = (data >> 4) & 0x0f;
	INT32 b = (data >> 0) & 0x0f;

	DrvPalette[(offset & 0x7fe)/2] = BurnHighCol((r*16)+r, (g*16)+g, (b*16)+b, 0);
}

static void __fastcall sf_write_word(UINT32 address, UINT16 data)
{
	if ((address & 0xfff800) == 0xb00000) {
		UINT16 *pal = (UINT16*)(Drv68kPalRam + (address & 0x7ff));

		*pal = data;

		palette_update(address);

		return;
	}

	switch (address)
	{
		case 0xc00010:
		return;

		case 0xc00014:
			sf_fg_scroll_x = data;
		return;

		case 0xc00018:
			sf_bg_scroll_x = data;
		return;

		case 0xc0001c:
		{
			soundlatch = data & 0xff;

			ZetNmi(0);
		}
		return;

		case 0xc0001e:
			if (version == 3) {
				protection_w();
			}
		return;
	}
}

static void __fastcall sf_write_byte(UINT32 address, UINT8 data)
{
	switch (address)
	{
		case 0xc0001b:
		{
			sf_active = data & 0xff;
			flipscreen = data & 0x04;
		}
		return;
	}
}

static UINT16 __fastcall sf_read_word(UINT32 address)
{
	switch (address)
	{
		case 0xc00000:
			return DrvInputs[3];

		case 0xc00002:
			return DrvInputs[4];

		case 0xc00004:
			return DrvInputs[5];

		case 0xc00006:
			return DrvInputs[6];

		case 0xc00008:
			return DrvInputs[0];

		case 0xc0000a:
			return DrvInputs[1];

		case 0xc0000c:
			return DrvInputs[2];

		case 0xc0000e:
			return 0xffff;
	}

	return 0;
}

static UINT8 __fastcall sf_read_byte(UINT32 address)
{
	switch (address)
	{
		case 0xc00000:
		case 0xc00001:
			return DrvInputs[3] >> ((~address & 1) << 3);

		case 0xc00002:
		case 0xc00003:
			return DrvInputs[4] >> ((~address & 1) << 3);

		case 0xc00004:
		case 0xc00005:
			return DrvInputs[5] >> ((~address & 1) << 3);

		case 0xc00006:
		case 0xc00007:
			return DrvInputs[6] >> ((~address & 1) << 3);

		case 0xc00008:
		case 0xc00009:
			return DrvInputs[0] >> ((~address & 1) << 3);

		case 0xc0000a:
		case 0xc0000b:
			return DrvInputs[1] >> ((~address & 1) << 3);

		case 0xc0000c:
		case 0xc0000d:
			return DrvInputs[2] >> ((~address & 1) << 3);

		case 0xc0000e:
		case 0xc0000f:
			return 0xff;
	}

	return 0;
}

static void __fastcall sf_sound_write(UINT16 address, UINT8 data)
{
	switch (address)
	{
		case 0xe000:
		case 0xe001:
			BurnYM2151Write(address & 1, data);
		return;
	}
}

static UINT8 __fastcall sf_sound_read(UINT16 address)
{
	switch (address)
	{
		case 0xc800:
			return soundlatch;

		case 0xe001:
			return BurnYM2151Read();
	}

	return 0;
}

static void sound2_bank_w(INT32 data)
{
	sound2_bank = data;

	data = (data * 0x8000) & (0x40000-1);

	ZetMapMemory(DrvZ80Rom1 + 0x8000 + data, 0x8000, 0xffff, MAP_ROM);
}

static void __fastcall sf_sound2_out(UINT16 port, UINT8 data)
{
	switch (port & 0xff)
	{
		case 0x00:
			MSM5205ResetWrite(0, data & 0x80);
			MSM5205DataWrite(0, data);
			MSM5205VCLKWrite(0, 1);
			MSM5205VCLKWrite(0, 0);
		return;

		case 0x01:
			MSM5205ResetWrite(1, data & 0x80);
			MSM5205DataWrite(1, data);
			MSM5205VCLKWrite(1, 1);
			MSM5205VCLKWrite(1, 0);
		return;

		case 0x02:
			sound2_bank_w(data);
		return;
	}
}

static UINT8 __fastcall sf_sound2_in(UINT16 port)
{
	switch (port & 0xff)
	{
		case 0x01:
			return soundlatch;
	}

	return 0;
}

static INT32 DrvSynchroniseStream(INT32 nSoundRate)
{
	return (INT64)(double)ZetTotalCycles() * nSoundRate / 3579545;
}

static INT32 DrvGfxDecode()
{
	static INT32 Gfx0Planes[4]   = { 4, 0, 0x200004, 0x200000 };
	static INT32 Gfx1Planes[4]   = { 4, 0, 0x400004, 0x400000 };
	static INT32 Gfx2Planes[4]   = { 4, 0, 0x700004, 0x700000 };
	static INT32 Gfx3Planes[2]   = { 4, 0 };

	static INT32 GfxXOffsets[16] = { 0, 1, 2, 3, 8, 9, 10, 11, 256, 257, 258, 259, 264, 265, 266, 267 };
	static INT32 GfxYOffsets[16] = { 0x00, 0x10, 0x20, 0x30, 0x40, 0x50, 0x60, 0x70,
				       0x80, 0x90, 0xa0, 0xb0, 0xc0, 0xd0, 0xe0, 0xf0 };

	UINT8 *tmp = (UINT8*)BurnMalloc(0x1C0000);
	if (tmp == NULL) {
		return 1;
	}

	memcpy (tmp, Gfx0, 0x080000);

	GfxDecode(0x1000, 4, 16, 16, Gfx0Planes, GfxXOffsets, GfxYOffsets, 0x200, tmp, Gfx0);

	memcpy (tmp, Gfx1, 0x100000);

	GfxDecode(0x2000, 4, 16, 16, Gfx1Planes, GfxXOffsets, GfxYOffsets, 0x200, tmp, Gfx1);

	memcpy (tmp, Gfx2, 0x1c0000);

	GfxDecode(0x3800, 4, 16, 16, Gfx2Planes, GfxXOffsets, GfxYOffsets, 0x200, tmp, Gfx2);

	memcpy (tmp, Gfx3, 0x004000);

	GfxDecode(0x0400, 2,  8,  8, Gfx3Planes, GfxXOffsets, GfxYOffsets, 0x080, tmp, Gfx3);

	memset (Gfx1Trans, 1, 0x2000);
	for (INT32 i = 0; i < 0x2000 * 0x100; i++) {
		if (Gfx1[i] != 15) Gfx1Trans[i/0x100] = 0;
	}

	memset (Gfx3Trans, 1, 0x400);
	for (INT32 i = 0; i < 0x400 * 0x40; i++) {
		if (Gfx3[i] != 3) Gfx3Trans[i/0x40] = 0;
	}

	BurnFree (tmp);

	return 0;
}

static INT32 DrvDoReset()
{
	memset (AllRam, 0, RamEnd - AllRam);

	sf_fg_scroll_x = 0;
	sf_bg_scroll_x = 0;

	soundlatch = 0;
	flipscreen = 0;

	sf_active = 0;

	sound2_bank = 0;

	SekOpen(0);
	SekReset();
	SekClose();

	ZetReset(0);
	ZetReset(1);

	timerReset();

	BurnYM2151Reset();
	MSM5205Reset();

	HiscoreReset();

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68kRom	= Next; Next += 0x060000;
	DrvZ80Rom0	= Next; Next += 0x008000;
	DrvZ80Rom1	= Next; Next += 0x040000;

	Gfx0		= Next; Next += 0x100000;
	Gfx1		= Next; Next += 0x200000;
	Gfx1Trans	= Next; Next += 0x002000;
	Gfx2		= Next; Next += 0x380000;
	Gfx3		= Next; Next += 0x010000;
	Gfx3Trans	= Next; Next += 0x000400;
	Gfx4		= Next; Next += 0x040000;

	DrvPalette	= (UINT32*)Next; Next += 0x00401 * sizeof(UINT32);

	AllRam          = Next;

	Drv68kPalRam    = Next; Next += 0x000800;
	Drv68kVidRam    = Next; Next += 0x001000;
	Drv68kRam       = Next; Next += 0x008000;

	DrvZ80Ram0	= Next; Next += 0x000800;

	RamEnd          = Next;

	MemEnd		= Next;

	return 0;
}

void sfYM2151IrqHandler(INT32 Irq)
{
	ZetSetIRQLine(0xff, (Irq) ? CPU_IRQSTATUS_ACK : CPU_IRQSTATUS_NONE);
}

void msm_irq_cb(int param)
{
	ZetSetIRQLine(0, CPU_IRQSTATUS_HOLD);
}

static INT32 DrvInit(INT32 initver)
{
	BurnAllocMemIndex();

	if (initver == 4)
	{
		if (BurnLoadRom(Drv68kRom + 1, 0, 2)) return 1;
		if (BurnLoadRom(Drv68kRom + 0, 1, 2)) return 1;

		if (BurnLoadRom(DrvZ80Rom0, 2, 1)) return 1;
		if (BurnLoadRom(DrvZ80Rom1, 3, 1)) return 1;

		for (INT32 i = 0; i <  4; i++) {
			if (BurnLoadRom(Gfx0 + i * 0x20000, i +  4, 1)) return 1;
		}

		for (INT32 i = 0; i <  8; i++) {
			if (BurnLoadRom(Gfx1 + i * 0x20000, i +  8, 1)) return 1;
		}

		for (INT32 i = 0; i < 6; i++) {
			if (BurnLoadRom(Gfx2 + 0x00000 + i * 0x10000, i + 16, 1)) return 1;
			if (BurnLoadRom(Gfx2 + 0xe0000 + i * 0x10000, i + 22, 1)) return 1;
		}

		if (BurnLoadRom(Gfx3, 28, 1)) return 1;

		for (INT32 i = 0; i <  4; i++) {
			if (BurnLoadRom(Gfx4 + i * 0x10000, i + 29, 1)) return 1;
		}
	} else {
		for (INT32 i = 0; i < 6; i+=2) {
			if (BurnLoadRom(Drv68kRom + 1 + i * 0x10000, i +  0, 2)) return 1;
			if (BurnLoadRom(Drv68kRom + 0 + i * 0x10000, i +  1, 2)) return 1;
		}

		if (BurnLoadRom(DrvZ80Rom0, 6, 1)) return 1;

		if (BurnLoadRom(DrvZ80Rom1 + 0x00000, 7, 1)) return 1;
		if (BurnLoadRom(DrvZ80Rom1 + 0x20000, 8, 1)) return 1;

		for (INT32 i = 0; i <  4; i++) {
			if (BurnLoadRom(Gfx0 + i * 0x20000, i +  9, 1)) return 1;
		}

		for (INT32 i = 0; i <  8; i++) {
			if (BurnLoadRom(Gfx1 + i * 0x20000, i + 13, 1)) return 1;
		}

		for (INT32 i = 0; i < 14; i++) {
			if (BurnLoadRom(Gfx2 + i * 0x20000, i + 21, 1)) return 1;
		}

		if (BurnLoadRom(Gfx3, 35, 1)) return 1;

		for (INT32 i = 0; i <  4; i++) {
			if (BurnLoadRom(Gfx4 + i * 0x10000, i + 36, 1)) return 1;
		}
	}
	
	if (strcmp(BurnDrvGetTextA(DRV_NAME), "sfjan") == 0) memcpy (Gfx3, Gfx3 + 0x4000, 0x4000);
	if (DrvGfxDecode()) return 1;

	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68kRom,		0x000000, 0x04ffff, MAP_ROM);
	SekMapMemory(Drv68kVidRam,	0x800000, 0x800fff, MAP_RAM);
	SekMapMemory(Drv68kRam,		0xff8000, 0xffffff, MAP_RAM);
	SekSetWriteByteHandler(0, sf_write_byte);
	SekSetWriteWordHandler(0, sf_write_word);
	SekSetReadByteHandler(0, sf_read_byte);
	SekSetReadWordHandler(0, sf_read_word);
	SekClose();

	ZetInit(0);
	ZetOpen(0);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom0);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom0);
	ZetMapArea(0xc000, 0xc7ff, 0, DrvZ80Ram0);
	ZetMapArea(0xc000, 0xc7ff, 1, DrvZ80Ram0);
	ZetMapArea(0xc000, 0xc7ff, 2, DrvZ80Ram0);
	ZetSetWriteHandler(sf_sound_write);
	ZetSetReadHandler(sf_sound_read);
	ZetClose();

	ZetInit(1);
	ZetOpen(1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom1);
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom1);
	ZetSetOutHandler(sf_sound2_out);
	ZetSetInHandler(sf_sound2_in);
	ZetClose();

	BurnYM2151InitBuffered(3579545, 1, NULL, 0);
	BurnYM2151SetIrqHandler(&sfYM2151IrqHandler);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_1, 0.60, BURN_SND_ROUTE_LEFT);
	BurnYM2151SetRoute(BURN_SND_YM2151_YM2151_ROUTE_2, 0.60, BURN_SND_ROUTE_RIGHT);
	BurnTimerAttachZet(3579545);

	timerInit();
	timerAdd(sndtimer, 0, msm_irq_cb);
	sndtimer.start(3579545 / 8000, -1, 1, 1);

	MSM5205Init(0, DrvSynchroniseStream, 384000, NULL, MSM5205_SEX_4B, 1);
	MSM5205Init(1, DrvSynchroniseStream, 384000, NULL, MSM5205_SEX_4B, 1);
	MSM5205SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);
	MSM5205SetRoute(1, 1.00, BURN_SND_ROUTE_BOTH);
//	MSM5205LPFilter(0, 1);
//	MSM5205LPFilter(1, 1);
	GenericTilesInit();

	DrvDoReset();

	version = initver;

	return 0;
}

static INT32 DrvExit()
{
	BurnYM2151Exit();
	SekExit();
	ZetExit();

	timerExit();

	MSM5205Exit();

	version = 0;

	GenericTilesExit();

	BurnFreeMemIndex();

	sf_fg_scroll_x = 0;
	sf_bg_scroll_x = 0;

	soundlatch = 0;
	flipscreen = 0;

	sf_active = 0;
	sound2_bank = 0;

	return 0;
}

static void draw_background()
{
	UINT8 *base = Gfx4 + ((sf_bg_scroll_x >> 4) << 5);

	for (INT32 offs = 0; offs < 0x200; offs++, base += 2)
	{
		INT32 sx = (offs & 0x1f0) - (sf_bg_scroll_x & 0x0f);
		INT32 sy = (offs << 4) & 0xf0;

		if (sx < 49 || sx >= 448 || sy < 16 || sy > 239) continue;

		INT32 attr  = base[0x10000];
		INT32 color = base[0x00000];
		INT32 code  = (base[0x10001] << 8) | base[1];

		INT32 flipx = attr & 1;
		INT32 flipy = attr & 2;

		if (flipscreen) {
			flipx = !flipx;
			flipy = !flipy;
			sx = ((sx + (sf_bg_scroll_x & 0x0f)) ^ 0x1f0) + (sf_bg_scroll_x & 0x0f);
			sy ^= 0xf0;
		}

		sx -= 64;
		sy -= 16;

		Draw16x16Tile(pTransDraw, code, sx, sy, flipx, flipy, color, 4, 0, Gfx0);
	}
}

static void draw_foreground()
{
	UINT8 *base = 0x20000 + Gfx4 + ((sf_fg_scroll_x >> 4) << 5);

	for (INT32 offs = 0; offs < 0x200; offs++, base += 2)
	{
		INT32 sx = (offs & 0x1f0) - (sf_fg_scroll_x & 0x0f);
		INT32 sy = (offs << 4) & 0xf0;

		if (sx < 49 || sx >= 448 || sy < 16 || sy > 239) continue;

		INT32 code  = (base[0x10001] << 8) | base[1];
		if (Gfx1Trans[code]) continue;

		INT32 attr  = base[0x10000];
		INT32 color = base[0x00000];

		INT32 flipx = attr & 1;
		INT32 flipy = attr & 2;

		if (flipscreen) {
			flipx = !flipx;
			flipy = !flipy;
			sx = ((sx + (sf_fg_scroll_x & 0x0f)) ^ 0x1f0) + (sf_fg_scroll_x & 0x0f);
			sy ^= 0xf0;
		}

		sx -= 64;
		sy -= 16;

		Draw16x16MaskTile(pTransDraw, code, sx, sy, flipx, flipy, color, 4, 15, 0x100, Gfx1);
	}
}

static void draw_characters()
{
	UINT16 *fg_ram = (UINT16*)(Drv68kVidRam);

	for (INT32 offs = 0x100 / 2; offs < 0x0f00 / 2; offs++)
	{
		INT32 sx = (offs & 0x3f) << 3;
		INT32 sy = (offs >> 6) << 3;

		if (sx < 64 || sx > 447) continue;

		INT32 attr = BURN_ENDIAN_SWAP_INT16(fg_ram[offs]);
		INT32 code = attr & 0x3ff;
		if (Gfx3Trans[code]) continue;

		INT32 color = attr >> 12;

		INT32 flipx = attr & 0x400;
		INT32 flipy = attr & 0x800;

		if (flipscreen) {
			flipx = !flipx;
			flipy = !flipy;
			sx ^= 0x1f8;
			sy ^= 0x0f8;
		}

		sx -= 64;
		sy -= 16;

		Draw8x8MaskTile(pTransDraw, code, sx, sy, flipx, flipy, color, 2, 3, 0x300, Gfx3);
	}
}

static inline INT32 sf_invert(INT32 nb)
{
	static const INT32 delta[4] = {0x00, 0x18, 0x18, 0x00};
	return nb ^ delta[(nb >> 3) & 3];
}

static void draw_sprites()
{
	UINT16 *sf_objectram = (UINT16*)(Drv68kRam + 0x6000);

	for (INT32 offs = 0x1000-0x20;offs >= 0;offs -= 0x20)
	{
		INT32 c = BURN_ENDIAN_SWAP_INT16(sf_objectram[offs]);
		INT32 attr = BURN_ENDIAN_SWAP_INT16(sf_objectram[offs+1]);
		INT32 sy = BURN_ENDIAN_SWAP_INT16(sf_objectram[offs+2]);
		INT32 sx = BURN_ENDIAN_SWAP_INT16(sf_objectram[offs+3]);
		INT32 color = attr & 0x000f;
		INT32 flipx = attr & 0x0100;
		INT32 flipy = attr & 0x0200;

		if (attr & 0x400)	// large sprite
		{
			INT32 c1,c2,c3,c4,t;

			if (flipscreen)
			{
				sx = 480 - sx;
				sy = 224 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			if (sy > 239 || sx < 33 || sx > 447) continue;

			sx -= 64;
			sy -= 16;

			c1 = c;
			c2 = c+1;
			c3 = c+16;
			c4 = c+17;

			if (flipx)
			{
				t = c1; c1 = c2; c2 = t;
				t = c3; c3 = c4; c4 = t;
			}
			if (flipy)
			{
				t = c1; c1 = c3; c3 = t;
				t = c2; c2 = c4; c4 = t;
			}

			if (flipy) {
				if (flipx) {
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, sf_invert(c1), sx     , sy     , color, 4, 15, 0x200, Gfx2);
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, sf_invert(c2), sx + 16, sy     , color, 4, 15, 0x200, Gfx2);
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, sf_invert(c3), sx     , sy + 16, color, 4, 15, 0x200, Gfx2);
					Render16x16Tile_Mask_FlipXY_Clip(pTransDraw, sf_invert(c4), sx + 16, sy + 16, color, 4, 15, 0x200, Gfx2);
				} else {
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, sf_invert(c1), sx     , sy     , color, 4, 15, 0x200, Gfx2);
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, sf_invert(c2), sx + 16, sy     , color, 4, 15, 0x200, Gfx2);
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, sf_invert(c3), sx     , sy + 16, color, 4, 15, 0x200, Gfx2);
					Render16x16Tile_Mask_FlipY_Clip(pTransDraw, sf_invert(c4), sx + 16, sy + 16, color, 4, 15, 0x200, Gfx2);
				}
			} else {
				if (flipx) {
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, sf_invert(c1), sx     , sy     , color, 4, 15, 0x200, Gfx2);
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, sf_invert(c2), sx + 16, sy     , color, 4, 15, 0x200, Gfx2);
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, sf_invert(c3), sx     , sy + 16, color, 4, 15, 0x200, Gfx2);
					Render16x16Tile_Mask_FlipX_Clip(pTransDraw, sf_invert(c4), sx + 16, sy + 16, color, 4, 15, 0x200, Gfx2);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, sf_invert(c1), sx     , sy     , color, 4, 15, 0x200, Gfx2);
					Render16x16Tile_Mask_Clip(pTransDraw, sf_invert(c2), sx + 16, sy     , color, 4, 15, 0x200, Gfx2);
					Render16x16Tile_Mask_Clip(pTransDraw, sf_invert(c3), sx     , sy + 16, color, 4, 15, 0x200, Gfx2);
					Render16x16Tile_Mask_Clip(pTransDraw, sf_invert(c4), sx + 16, sy + 16, color, 4, 15, 0x200, Gfx2);
				}
			}
		}
		else
		{
			if (flipscreen)
			{
				sx = 496 - sx;
				sy = 240 - sy;
				flipx = !flipx;
				flipy = !flipy;
			}

			if (sy < 1 || sy > 239 || sx < 49 || sx > 447) continue;

			sx -= 64;
			sy -= 16;

			Draw16x16MaskTile(pTransDraw, sf_invert(c), sx, sy, flipx, flipy, color, 4, 15, 0x200, Gfx2);
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		for (INT32 i = 0; i < 0x800; i+=2) {
			palette_update(i);
		}
		DrvPalette[0x400] = BurnHighCol(0xff,0,0xff,0);
		DrvRecalc = 0;
	}

	if (nBurnLayer & 8) {
		if (sf_active & 0x20)
			draw_background();
		else
			BurnTransferClear();
	} else {
			BurnTransferClear(0x0400); // magenta
	}

	if (sf_active & 0x40 && nBurnLayer & 4)
		draw_foreground();

	if (sf_active & 0x80 && nBurnLayer & 1)
		draw_sprites();

	if (sf_active & 0x08 && nBurnLayer & 2)
		draw_characters();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static void DrvMakeInputs()
{
	DrvInputs[0] = (DrvDips[0] << 8) | DrvDips[1];
	DrvInputs[1] = (DrvDips[2] << 4) | DrvDips[3] | 0xf800;

	DrvInputs[2] = 0xff7f;
	DrvInputs[3] = 0xffff;
	DrvInputs[4] = 0xffff;
	DrvInputs[5] = 0xffff;
	DrvInputs[6] = 0xffff;

	for (INT32 i = 0; i < 16; i++) {
		DrvInputs[2] ^= (DrvJoy1[i] & 1) << i;
		DrvInputs[3] ^= (DrvJoy2[i] & 1) << i;
		DrvInputs[4] ^= (DrvJoy3[i] & 1) << i;
		DrvInputs[5] ^= (DrvJoy4[i] & 1) << i;
	}

	// clear opposites
	{
		if (!(DrvInputs[4] & 0x0003)) DrvInputs[4] |= 0x0003; // p1 left/right
		if (!(DrvInputs[4] & 0x000c)) DrvInputs[4] |= 0x000c; // p1 up/down
		if (!(DrvInputs[4] & 0x0300)) DrvInputs[4] |= 0x0300; // p2 left/right
		if (!(DrvInputs[4] & 0x0c00)) DrvInputs[4] |= 0x0c00; // p2 up/down
	}

	// World & prototype -- simulate analogue buttons
	if (version == 1 || version == 4)
	{
		static const INT32 scale[8] = { 0x00, 0x60, 0xe0, 0xfe, 0xfe, 0xfe, 0xfe, 0xfe };

		INT32 t = (DrvJoy4[0] & 1) | ((DrvJoy4[1] & 1) << 1) | ((DrvJoy4[2] & 1) << 2);
			DrvInputs[5] = scale[t];
		    t = (DrvJoy5[0] & 1) | ((DrvJoy5[1] & 1) << 1) | ((DrvJoy5[2] & 1) << 2);
			DrvInputs[5] |= scale[t] << 8;

		    t = (DrvJoy4[8] & 1) | ((DrvJoy4[9] & 1) << 1) | ((DrvJoy4[10] & 1) << 2);
			DrvInputs[6]  = scale[t];

		    t = (DrvJoy5[8] & 1) | ((DrvJoy5[9] & 1) << 1) | ((DrvJoy5[10] & 1) << 2);
			DrvInputs[6] |= scale[t] << 8;
	}
}

static INT32 DrvFrame()
{
	if (DrvReset) {
		DrvDoReset();
	}

	SekNewFrame();
	ZetNewFrame();

	DrvMakeInputs();

	INT32 nInterleave = 256 * 2;
	INT32 nCyclesTotal[4];
	INT32 nCyclesDone[4] = { 0, 0, 0, 0 };

	nCyclesTotal[0] = (INT32)((INT64)8000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[1] = (INT32)((INT64)3579545 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesTotal[2] = 3579545 / 60; // must be fixed
	nCyclesTotal[3] = 3579545 / 60; // ""

	SekOpen(0);

	for (INT32 i = 0; i < nInterleave; i++) {
		CPU_RUN(0, Sek);

		ZetOpen(0);
		CPU_RUN_TIMER(1);
		ZetClose();

		ZetOpen(1);
		CPU_RUN(2, Zet);
		CPU_RUN(3, timer);
		ZetClose();
	}

	SekSetIRQLine((version == 4) ? 6 : 1, CPU_IRQSTATUS_AUTO);

	SekClose();

	if (pBurnSoundOut) {
		BurnYM2151Render(pBurnSoundOut, nBurnSoundLen);
		MSM5205Render(0, pBurnSoundOut, nBurnSoundLen);
		MSM5205Render(1, pBurnSoundOut, nBurnSoundLen);
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
		*pnMin = 0x029521;
	}

	if (nAction & ACB_VOLATILE) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = AllRam;
		ba.nLen	  = RamEnd - AllRam;
		ba.szName = "All RAM";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		ZetScan(nAction);

		MSM5205Scan(nAction, pnMin);
		BurnYM2151Scan(nAction, pnMin);

		timerScan();

		SCAN_VAR(sf_fg_scroll_x);
		SCAN_VAR(sf_bg_scroll_x);
		SCAN_VAR(soundlatch);
		SCAN_VAR(flipscreen);
		SCAN_VAR(sf_active);
		SCAN_VAR(sound2_bank);
	}

	if (nAction & ACB_WRITE) {
		ZetOpen(1);
		sound2_bank_w(sound2_bank);
		ZetClose();
	}

	return 0;
}


// Street Fighter (US, set 1)

static struct BurnRomInfo sfRomDesc[] = {
	{ "sfd-19.2a",     	0x10000, 0xfaaf6255, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "sfd-22.2c",     	0x10000, 0xe1fe3519, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sfd-20.3a",     	0x10000, 0x44b915bd, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "sfd-23.3c",     	0x10000, 0x79c43ff8, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "sfd-21.4a",     	0x10000, 0xe8db799b, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "sfd-24.4c",     	0x10000, 0x466a3440, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "sf-02.7k",  		0x08000, 0x4a9ac534, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 #0 Code

	{ "sfu-00.1h",     	0x20000, 0xa7cce903, 3 | BRF_PRG | BRF_ESS }, //  7 Z80 #1 Code
	{ "sf-01.1k",  		0x20000, 0x86e0f0d5, 3 | BRF_PRG | BRF_ESS }, //  8

	{ "sf-39.2k",  		0x20000, 0xcee3d292, 4 | BRF_GRA },           //  9 Background Tiles
	{ "sf-38.1k",  		0x20000, 0x2ea99676, 4 | BRF_GRA },           // 10
	{ "sf-41.4k",  		0x20000, 0xe0280495, 4 | BRF_GRA },           // 11
	{ "sf-40.3k",  		0x20000, 0xc70b30de, 4 | BRF_GRA },           // 12

	{ "sf-25.1d",  		0x20000, 0x7f23042e, 5 | BRF_GRA },           // 13 Foreground Tiles
	{ "sf-28.1e",  		0x20000, 0x92f8b91c, 5 | BRF_GRA },           // 14
	{ "sf-30.1g",  		0x20000, 0xb1399856, 5 | BRF_GRA },           // 15
	{ "sf-34.1h",  		0x20000, 0x96b6ae2e, 5 | BRF_GRA },           // 16
	{ "sf-26.2d",  		0x20000, 0x54ede9f5, 5 | BRF_GRA },           // 17
	{ "sf-29.2e",  		0x20000, 0xf0649a67, 5 | BRF_GRA },           // 18
	{ "sf-31.2g",  		0x20000, 0x8f4dd71a, 5 | BRF_GRA },           // 19
	{ "sf-35.2h",  		0x20000, 0x70c00fb4, 5 | BRF_GRA },           // 20

	{ "sf-15.1m",  		0x20000, 0xfc0113db, 6 | BRF_GRA },           // 21 Sprites
	{ "sf-16.2m",  		0x20000, 0x82e4a6d3, 6 | BRF_GRA },           // 22
	{ "sf-11.1k",  		0x20000, 0xe112df1b, 6 | BRF_GRA },           // 23
	{ "sf-12.2k",  		0x20000, 0x42d52299, 6 | BRF_GRA },           // 24
	{ "sf-07.1h",  		0x20000, 0x49f340d9, 6 | BRF_GRA },           // 25
	{ "sf-08.2h",  		0x20000, 0x95ece9b1, 6 | BRF_GRA },           // 26
	{ "sf-03.1f",  		0x20000, 0x5ca05781, 6 | BRF_GRA },           // 27
	{ "sf-17.3m",  		0x20000, 0x69fac48e, 6 | BRF_GRA },           // 28
	{ "sf-18.4m",  		0x20000, 0x71cfd18d, 6 | BRF_GRA },           // 29
	{ "sf-13.3k",  		0x20000, 0xfa2eb24b, 6 | BRF_GRA },           // 30
	{ "sf-14.4k",  		0x20000, 0xad955c95, 6 | BRF_GRA },           // 31
	{ "sf-09.3h",  		0x20000, 0x41b73a31, 6 | BRF_GRA },           // 32
	{ "sf-10.4h",  		0x20000, 0x91c41c50, 6 | BRF_GRA },           // 33
	{ "sf-05.3f",  		0x20000, 0x538c7cbe, 6 | BRF_GRA },           // 34

	{ "sf-27.4d",  		0x04000, 0x2b09b36d, 7 | BRF_GRA },           // 35 Characters

	{ "sf-37.4h",  		0x10000, 0x23d09d3d, 8 | BRF_GRA },           // 36 Tilemaps
	{ "sf-36.3h",  		0x10000, 0xea16df6c, 8 | BRF_GRA },           // 37
	{ "sf-32.3g",  		0x10000, 0x72df2bd9, 8 | BRF_GRA },           // 38
	{ "sf-33.4g",  		0x10000, 0x3e99d3d5, 8 | BRF_GRA },           // 39

	{ "mb7114h.12k", 	0x0100, 0x75af3553, 0 | BRF_OPT },            // 40 Proms
	{ "mb7114h.11h", 	0x0100, 0xc0e56586, 0 | BRF_OPT },            // 41
	{ "mb7114h.12j", 	0x0100, 0x4c734b64, 0 | BRF_OPT },            // 42
	{ "mmi-7603.13h",	0x0020, 0x06bcda53, 0 | BRF_OPT },            // 43
};

STD_ROM_PICK(sf)
STD_ROM_FN(sf)

static INT32 SfusInit()
{
	return DrvInit(2);
}

struct BurnDriver BurnDrvsf = {
	"sf", NULL, NULL, NULL, "1987",
	"Street Fighter (US, set 1)\0", NULL, "Capcom", "Miscellaneous",
	L"\u8857\u5934\u9738\u738B (\u7F8E\u7248, \u7B2C\u4E00\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VSFIGHT, FBF_SF,
	NULL, sfRomInfo, sfRomName, NULL, NULL, NULL, NULL, SfusInputInfo, SfusDIPInfo,
	SfusInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 
	&DrvRecalc, 0x401, 384, 224, 4, 3
};


// Street Fighter (World, pneumatic buttons)

static struct BurnRomInfo sfanRomDesc[] = {
	{ "sfe-19.2a",     	0x10000, 0x8346c3ca, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "sfe-22.2c",     	0x10000, 0x3a4bfaa8, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sfe-20.3a",     	0x10000, 0xb40e67ee, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "sfe-23.3c",     	0x10000, 0x477c3d5b, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "sfe-21.4a",     	0x10000, 0x2547192b, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "sfe-24.4c",     	0x10000, 0x79680f4e, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "sf-02.7k",  		0x08000, 0x4a9ac534, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 #0 Code

	{ "sfu-00.1h",     	0x20000, 0xa7cce903, 3 | BRF_PRG | BRF_ESS }, //  7 Z80 #1 Code
	{ "sf-01.2k",  		0x20000, 0x86e0f0d5, 3 | BRF_PRG | BRF_ESS }, //  8

	{ "sf-39.2k",  		0x20000, 0xcee3d292, 4 | BRF_GRA },           //  9 Background Tiles
	{ "sf-38.1k",  		0x20000, 0x2ea99676, 4 | BRF_GRA },           // 10
	{ "sf-41.4k",  		0x20000, 0xe0280495, 4 | BRF_GRA },           // 11
	{ "sf-40.3k",  		0x20000, 0xc70b30de, 4 | BRF_GRA },           // 12

	{ "sf-25.1d",  		0x20000, 0x7f23042e, 5 | BRF_GRA },           // 13 Foreground Tiles
	{ "sf-28.1e",  		0x20000, 0x92f8b91c, 5 | BRF_GRA },           // 14
	{ "sf-30.1g",  		0x20000, 0xb1399856, 5 | BRF_GRA },           // 15
	{ "sf-34.1h",  		0x20000, 0x96b6ae2e, 5 | BRF_GRA },           // 16
	{ "sf-26.2d",  		0x20000, 0x54ede9f5, 5 | BRF_GRA },           // 17
	{ "sf-29.2e",  		0x20000, 0xf0649a67, 5 | BRF_GRA },           // 18
	{ "sf-31.2g",  		0x20000, 0x8f4dd71a, 5 | BRF_GRA },           // 19
	{ "sf-35.2h",  		0x20000, 0x70c00fb4, 5 | BRF_GRA },           // 20

	{ "sf-15.1m",  		0x20000, 0xfc0113db, 6 | BRF_GRA },           // 21 Sprites
	{ "sf-16.2m",  		0x20000, 0x82e4a6d3, 6 | BRF_GRA },           // 22
	{ "sf-11.1k",  		0x20000, 0xe112df1b, 6 | BRF_GRA },           // 23
	{ "sf-12.2k",  		0x20000, 0x42d52299, 6 | BRF_GRA },           // 24
	{ "sf-07.1h",  		0x20000, 0x49f340d9, 6 | BRF_GRA },           // 25
	{ "sf-08.2h",  		0x20000, 0x95ece9b1, 6 | BRF_GRA },           // 26
	{ "sf-03.1f",  		0x20000, 0x5ca05781, 6 | BRF_GRA },           // 27
	{ "sf-17.3m",  		0x20000, 0x69fac48e, 6 | BRF_GRA },           // 28
	{ "sf-18.4m",  		0x20000, 0x71cfd18d, 6 | BRF_GRA },           // 29
	{ "sf-13.3k",  		0x20000, 0xfa2eb24b, 6 | BRF_GRA },           // 30
	{ "sf-14.4k",  		0x20000, 0xad955c95, 6 | BRF_GRA },           // 31
	{ "sf-09.3h",  		0x20000, 0x41b73a31, 6 | BRF_GRA },           // 32
	{ "sf-10.4h",  		0x20000, 0x91c41c50, 6 | BRF_GRA },           // 33
	{ "sf-05.3f",  		0x20000, 0x538c7cbe, 6 | BRF_GRA },           // 34

	{ "sf-27.4d",  		0x04000, 0x2b09b36d, 7 | BRF_GRA },           // 35 Characters

	{ "sf-37.4h",  		0x10000, 0x23d09d3d, 8 | BRF_GRA },           // 36 Tilemaps
	{ "sf-36.3h",  		0x10000, 0xea16df6c, 8 | BRF_GRA },           // 37
	{ "sf-32.3g",  		0x10000, 0x72df2bd9, 8 | BRF_GRA },           // 38
	{ "sf-33.4g",  		0x10000, 0x3e99d3d5, 8 | BRF_GRA },           // 39

	{ "sfb05.mb7114h.12k", 	0x0100, 0x75af3553, 0 | BRF_OPT },        // 40 Proms
	{ "sfb10.mb7114h.11h", 	0x0100, 0xc0e56586, 0 | BRF_OPT },        // 41
	{ "sfb04.mb7114h.12j", 	0x0100, 0x4c734b64, 0 | BRF_OPT },        // 42
	{ "sfb00.mb7051.13h",	0x0020, 0x06bcda53, 0 | BRF_OPT },        // 43
};

STD_ROM_PICK(sfan)
STD_ROM_FN(sfan)

static INT32 SfanInit()
{
	return DrvInit(1);
}

struct BurnDriver BurnDrvsfan = {
	"sfan", "sf", NULL, NULL, "1987",
	"Street Fighter (World, pneumatic buttons)\0", NULL, "Capcom", "Miscellaneous",
	L"\u8857\u5934\u9738\u738B (\u4E16\u754C\u7248, \u6C14\u52A8\u6309\u94AE)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VSFIGHT, FBF_SF,
	NULL, sfanRomInfo, sfanRomName, NULL, NULL, NULL, NULL, SfInputInfo, SfDIPInfo,
	SfanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 
	&DrvRecalc, 0x401, 384, 224, 4, 3
};


// Street Fighter (US, set 2) (protected)

static struct BurnRomInfo sfuaRomDesc[] = {
	{ "sfs19u.1a",  	0x10000, 0xc8e41c49, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "sfs22u.1b",  	0x10000, 0x667e9309, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sfs20u.2a",  	0x10000, 0x303065bf, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "sfs23u.2b",  	0x10000, 0xde6927a3, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "sfs21u.3a",  	0x10000, 0x004a418b, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "sfs24u.3b",  	0x10000, 0x2b4545ff, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "sf-02.7k",  		0x08000, 0x4a9ac534, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 #0 Code

	{ "sfu-00.1h",     	0x20000, 0xa7cce903, 3 | BRF_PRG | BRF_ESS }, //  7 Z80 #1 Code
	{ "sf-01.1k",  		0x20000, 0x86e0f0d5, 3 | BRF_PRG | BRF_ESS }, //  8

	{ "sf-39.2k",  		0x20000, 0xcee3d292, 4 | BRF_GRA },           //  9 Background Tiles
	{ "sf-38.1k",  		0x20000, 0x2ea99676, 4 | BRF_GRA },           // 10
	{ "sf-41.4k",  		0x20000, 0xe0280495, 4 | BRF_GRA },           // 11
	{ "sf-40.3k",  		0x20000, 0xc70b30de, 4 | BRF_GRA },           // 12

	{ "sf-25.1d",  		0x20000, 0x7f23042e, 5 | BRF_GRA },           // 13 Foreground Tiles
	{ "sf-28.1e",  		0x20000, 0x92f8b91c, 5 | BRF_GRA },           // 14
	{ "sf-30.1g",  		0x20000, 0xb1399856, 5 | BRF_GRA },           // 15
	{ "sf-34.1h",  		0x20000, 0x96b6ae2e, 5 | BRF_GRA },           // 16
	{ "sf-26.2d",  		0x20000, 0x54ede9f5, 5 | BRF_GRA },           // 17
	{ "sf-29.2e",  		0x20000, 0xf0649a67, 5 | BRF_GRA },           // 18
	{ "sf-31.2g",  		0x20000, 0x8f4dd71a, 5 | BRF_GRA },           // 19
	{ "sf-35.2h",  		0x20000, 0x70c00fb4, 5 | BRF_GRA },           // 20

	{ "sf-15.1m",  		0x20000, 0xfc0113db, 6 | BRF_GRA },           // 21 Sprites
	{ "sf-16.2m",  		0x20000, 0x82e4a6d3, 6 | BRF_GRA },           // 22
	{ "sf-11.1k",  		0x20000, 0xe112df1b, 6 | BRF_GRA },           // 23
	{ "sf-12.2k",  		0x20000, 0x42d52299, 6 | BRF_GRA },           // 24
	{ "sf-07.1h",  		0x20000, 0x49f340d9, 6 | BRF_GRA },           // 25
	{ "sf-08.2h",  		0x20000, 0x95ece9b1, 6 | BRF_GRA },           // 26
	{ "sf-03.1f",  		0x20000, 0x5ca05781, 6 | BRF_GRA },           // 27
	{ "sf-17.3m",  		0x20000, 0x69fac48e, 6 | BRF_GRA },           // 28
	{ "sf-18.4m",  		0x20000, 0x71cfd18d, 6 | BRF_GRA },           // 29
	{ "sf-13.3k",  		0x20000, 0xfa2eb24b, 6 | BRF_GRA },           // 30
	{ "sf-14.4k",  		0x20000, 0xad955c95, 6 | BRF_GRA },           // 31
	{ "sf-09.3h",  		0x20000, 0x41b73a31, 6 | BRF_GRA },           // 32
	{ "sf-10.4h",  		0x20000, 0x91c41c50, 6 | BRF_GRA },           // 33
	{ "sf-05.3f",  		0x20000, 0x538c7cbe, 6 | BRF_GRA },           // 34

	{ "sf-27.4d",  		0x04000, 0x2b09b36d, 7 | BRF_GRA },           // 35 Characters

	{ "sf-37.4h",  		0x10000, 0x23d09d3d, 8 | BRF_GRA },           // 36 Tilemaps
	{ "sf-36.3h",  		0x10000, 0xea16df6c, 8 | BRF_GRA },           // 37
	{ "sf-32.3g",  		0x10000, 0x72df2bd9, 8 | BRF_GRA },           // 38
	{ "sf-33.4g",  		0x10000, 0x3e99d3d5, 8 | BRF_GRA },           // 39

	{ "sfb05.bin",   	0x0100, 0x864199ad, 0 | BRF_OPT },            // 40 Proms
	{ "sfb00.bin",   	0x0100, 0xbd3f8c5d, 0 | BRF_OPT },            // 41
	{ "mb7114h.12j", 	0x0100, 0x4c734b64, 0 | BRF_OPT },            // 42
	{ "mmi-7603.13h",	0x0020, 0x06bcda53, 0 | BRF_OPT },            // 43
	
	{ "sf_s.id8751h-8.14f",  	0x01000, 0x6588891f, 0 | BRF_OPT },	  // i8751 MCU
};

STD_ROM_PICK(sfua)
STD_ROM_FN(sfua)

static INT32 SfuaInit()
{
	return DrvInit(3);
}

struct BurnDriver BurnDrvsfua = {
	"sfua", "sf", NULL, NULL, "1987",
	"Street Fighter (US, set 2) (protected)\0", NULL, "Capcom", "Miscellaneous",
	L"\u8857\u5934\u9738\u738B (\u7F8E\u7248, \u7B2C\u4E8C\u5957) (\u5DE5\u7A0B\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VSFIGHT, FBF_SF,
	NULL, sfuaRomInfo, sfuaRomName, NULL, NULL, NULL, NULL, SfjInputInfo, SfusDIPInfo,
	SfuaInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 
	&DrvRecalc, 0x401, 384, 224, 4, 3
};


// Street Fighter (Japan) (protected)

static struct BurnRomInfo sfjRomDesc[] = {
	{ "sf-19.2a",  		0x10000, 0x116027d7, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "sf-22.2c",  		0x10000, 0xd3cbd09e, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sf-20.3a",  		0x10000, 0xfe07e83f, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "sf-23.3c",  		0x10000, 0x1e435d33, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "sf-21.4a",  		0x10000, 0xe086bc4c, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "sf-24.4c",  		0x10000, 0x13a6696b, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "sf-02.7k",  		0x08000, 0x4a9ac534, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 #0 Code

	{ "sf-00.1h",  		0x20000, 0x4b733845, 3 | BRF_PRG | BRF_ESS }, //  7 Z80 #1 Code
	{ "sf-01.1k",  		0x20000, 0x86e0f0d5, 3 | BRF_PRG | BRF_ESS }, //  8

	{ "sf-39.2k",  		0x20000, 0xcee3d292, 4 | BRF_GRA },           //  9 Background Tiles
	{ "sf-38.1k",  		0x20000, 0x2ea99676, 4 | BRF_GRA },           // 10
	{ "sf-41.4k",  		0x20000, 0xe0280495, 4 | BRF_GRA },           // 11
	{ "sf-40.3k",  		0x20000, 0xc70b30de, 4 | BRF_GRA },           // 12

	{ "sf-25.1d",  		0x20000, 0x7f23042e, 5 | BRF_GRA },           // 13 Foreground Tiles
	{ "sf-28.1e",  		0x20000, 0x92f8b91c, 5 | BRF_GRA },           // 14
	{ "sf-30.1g",  		0x20000, 0xb1399856, 5 | BRF_GRA },           // 15
	{ "sf-34.1h",  		0x20000, 0x96b6ae2e, 5 | BRF_GRA },           // 16
	{ "sf-26.2d",  		0x20000, 0x54ede9f5, 5 | BRF_GRA },           // 17
	{ "sf-29.2e",  		0x20000, 0xf0649a67, 5 | BRF_GRA },           // 18
	{ "sf-31.2g",  		0x20000, 0x8f4dd71a, 5 | BRF_GRA },           // 19
	{ "sf-35.2h",  		0x20000, 0x70c00fb4, 5 | BRF_GRA },           // 20

	{ "sf-15.1m",  		0x20000, 0xfc0113db, 6 | BRF_GRA },           // 21 Sprites
	{ "sf-16.2m",  		0x20000, 0x82e4a6d3, 6 | BRF_GRA },           // 22
	{ "sf-11.1k",  		0x20000, 0xe112df1b, 6 | BRF_GRA },           // 23
	{ "sf-12.2k",  		0x20000, 0x42d52299, 6 | BRF_GRA },           // 24
	{ "sf-07.1h",  		0x20000, 0x49f340d9, 6 | BRF_GRA },           // 25
	{ "sf-08.2h",  		0x20000, 0x95ece9b1, 6 | BRF_GRA },           // 26
	{ "sf-03.1f",  		0x20000, 0x5ca05781, 6 | BRF_GRA },           // 27
	{ "sf-17.3m",  		0x20000, 0x69fac48e, 6 | BRF_GRA },           // 28
	{ "sf-18.4m",  		0x20000, 0x71cfd18d, 6 | BRF_GRA },           // 29
	{ "sf-13.3k",  		0x20000, 0xfa2eb24b, 6 | BRF_GRA },           // 30
	{ "sf-14.4k",  		0x20000, 0xad955c95, 6 | BRF_GRA },           // 31
	{ "sf-09.3h",  		0x20000, 0x41b73a31, 6 | BRF_GRA },           // 32
	{ "sf-10.4h",  		0x20000, 0x91c41c50, 6 | BRF_GRA },           // 33
	{ "sf-05.3f",  		0x20000, 0x538c7cbe, 6 | BRF_GRA },           // 34

	{ "sf-27.4d",  		0x04000, 0x2b09b36d, 7 | BRF_GRA },           // 35 Characters

	{ "sf-37.4h",  		0x10000, 0x23d09d3d, 8 | BRF_GRA },           // 36 Tilemaps
	{ "sf-36.3h",  		0x10000, 0xea16df6c, 8 | BRF_GRA },           // 37
	{ "sf-32.3g",  		0x10000, 0x72df2bd9, 8 | BRF_GRA },           // 38
	{ "sf-33.4g",  		0x10000, 0x3e99d3d5, 8 | BRF_GRA },           // 39

	{ "sfb05.bin",   	0x0100, 0x864199ad, 0 | BRF_OPT },            // 40 Proms
	{ "sfb00.bin",   	0x0100, 0xbd3f8c5d, 0 | BRF_OPT },            // 41
	{ "mb7114h.12j", 	0x0100, 0x4c734b64, 0 | BRF_OPT },            // 42
	{ "mmi-7603.13h",	0x0020, 0x06bcda53, 0 | BRF_OPT },            // 43
	
	{ "sf_s.id8751h-8.14f",  	0x01000, 0x6588891f, 0 | BRF_OPT },	  // i8751 MCU
};

STD_ROM_PICK(sfj)
STD_ROM_FN(sfj)

static INT32 SfjInit()
{
	return DrvInit(3);
}

struct BurnDriver BurnDrvsfj = {
	"sfj", "sf", NULL, NULL, "1987",
	"Street Fighter (Japan) (protected)\0", NULL, "Capcom", "Miscellaneous",
	L"\u8857\u5934\u9738\u738B (\u65E5\u7248) (\u5DE5\u7A0B\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VSFIGHT, FBF_SF,
	NULL, sfjRomInfo, sfjRomName, NULL, NULL, NULL, NULL, SfjInputInfo, SfusDIPInfo,
	SfjInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 
	&DrvRecalc, 0x401, 384, 224, 4, 3
};


// Street Fighter (Japan, pneumatic buttons)

static struct BurnRomInfo sfjanRomDesc[] = {
	{ "sf_n_19a.2a.27c512",     	0x10000, 0x8346c3ca, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "sf_n_22a.2c.27c512",     	0x10000, 0x3a4bfaa8, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sf_n_20a.3a.27c512",     	0x10000, 0x7e00b1dd, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "sf_n_23a.3c.27c512",     	0x10000, 0x1cf3c108, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "sf_n_21a.4a.27c512",     	0x10000, 0x2547192b, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "sf_n_24a.4c.27c512",     	0x10000, 0x79680f4e, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "sf-02.bin",  	0x08000, 0x4a9ac534, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 #0 Code // sf_02a.7k.27256

	{ "sf-00.bin",     	0x20000, 0x4b733845, 3 | BRF_PRG | BRF_ESS }, //  7 Z80 #1 Code // sf_n_00.1h.27c1000
	{ "sf-01.bin",  	0x20000, 0x86e0f0d5, 3 | BRF_PRG | BRF_ESS }, //  8				// sf_n_01.1k.27c1000

	{ "sf-39.2k",  		0x20000, 0xcee3d292, 4 | BRF_GRA },           //  9 Background Tiles
	{ "sf-38.1k",  		0x20000, 0x2ea99676, 4 | BRF_GRA },           // 10
	{ "sf-41.4k",  		0x20000, 0xe0280495, 4 | BRF_GRA },           // 11
	{ "sf-40.3k",  		0x20000, 0xc70b30de, 4 | BRF_GRA },           // 12

	{ "sf-25.1d",  		0x20000, 0x7f23042e, 5 | BRF_GRA },           // 13 Foreground Tiles
	{ "sf-28.1e",  		0x20000, 0x92f8b91c, 5 | BRF_GRA },           // 14
	{ "sf-30.1g",  		0x20000, 0xb1399856, 5 | BRF_GRA },           // 15
	{ "sf-34.1h",  		0x20000, 0x96b6ae2e, 5 | BRF_GRA },           // 16
	{ "sf-26.2d",  		0x20000, 0x54ede9f5, 5 | BRF_GRA },           // 17
	{ "sf-29.2e",  		0x20000, 0xf0649a67, 5 | BRF_GRA },           // 18
	{ "sf-31.2g",  		0x20000, 0x8f4dd71a, 5 | BRF_GRA },           // 19
	{ "sf-35.2h",  		0x20000, 0x70c00fb4, 5 | BRF_GRA },           // 20

	{ "sf-15.1m",  		0x20000, 0xfc0113db, 6 | BRF_GRA },           // 21 Sprites
	{ "sf-16.2m",  		0x20000, 0x82e4a6d3, 6 | BRF_GRA },           // 22
	{ "sf-11.1k",  		0x20000, 0xe112df1b, 6 | BRF_GRA },           // 23
	{ "sf-12.2k",  		0x20000, 0x42d52299, 6 | BRF_GRA },           // 24
	{ "sf-07.1h",  		0x20000, 0x49f340d9, 6 | BRF_GRA },           // 25
	{ "sf-08.2h",  		0x20000, 0x95ece9b1, 6 | BRF_GRA },           // 26
	{ "sf-03.1f",  		0x20000, 0x5ca05781, 6 | BRF_GRA },           // 27
	{ "sf-17.3m",  		0x20000, 0x69fac48e, 6 | BRF_GRA },           // 28
	{ "sf-18.4m",  		0x20000, 0x71cfd18d, 6 | BRF_GRA },           // 29
	{ "sf-13.3k",  		0x20000, 0xfa2eb24b, 6 | BRF_GRA },           // 30
	{ "sf-14.4k",  		0x20000, 0xad955c95, 6 | BRF_GRA },           // 31
	{ "sf-09.3h",  		0x20000, 0x41b73a31, 6 | BRF_GRA },           // 32
	{ "sf-10.4h",  		0x20000, 0x91c41c50, 6 | BRF_GRA },           // 33
	{ "sf-05.3f",  		0x20000, 0x538c7cbe, 6 | BRF_GRA },           // 34

	{ "sf_27.4d.27256", 0x08000, 0x59416e03, 7 | BRF_GRA },           // 35 Characters // First half FF, second half matches known

	{ "sf-37.4h",  		0x10000, 0x23d09d3d, 8 | BRF_GRA },           // 36 Tilemaps
	{ "sf-36.3h",  		0x10000, 0xea16df6c, 8 | BRF_GRA },           // 37
	{ "sf-32.3g",  		0x10000, 0x72df2bd9, 8 | BRF_GRA },           // 38
	{ "sf-33.4g",  		0x10000, 0x3e99d3d5, 8 | BRF_GRA },           // 39

	{ "sfb05.12k", 		0x0100, 0x864199ad, 0 | BRF_OPT },        	  // 40 Proms
	{ "sfb00.13h", 		0x0100, 0xbd3f8c5d, 0 | BRF_OPT },        	  // 41
	{ "sfb04.12j", 		0x0100, 0x4c734b64, 0 | BRF_OPT },        	  // 42
	{ "mmi-7603.13h",	0x0020, 0x06bcda53, 0 | BRF_OPT },        	  // 43
};

STD_ROM_PICK(sfjan)
STD_ROM_FN(sfjan)

static INT32 SfjanInit()
{
	return DrvInit(1);
}

struct BurnDriver BurnDrvsfjan = {
	"sfjan", "sf", NULL, NULL, "1987",
	"Street Fighter (Japan, pneumatic buttons)\0", NULL, "Capcom", "Miscellaneous",
	L"\u8857\u5934\u9738\u738B (\u4E16\u754C\u7248, \u6C14\u52A8\u6309)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VSFIGHT, FBF_SF,
	NULL, sfjanRomInfo, sfjanRomName, NULL, NULL, NULL, NULL, SfInputInfo, SfDIPInfo,
	SfjanInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 
	&DrvRecalc, 0x401, 384, 224, 4, 3
};


// Street Fighter (prototype)

static struct BurnRomInfo sfpRomDesc[] = {
	{ "prg8.2a",     	0x20000, 0xd48d06a3, 1 | BRF_PRG | BRF_ESS }, //  0 68K Code
	{ "prg0.2c",     	0x20000, 0xe8606c1a, 1 | BRF_PRG | BRF_ESS }, //  1

	{ "sound.9j",    	0x08000, 0x43cd32ae, 2 | BRF_PRG | BRF_ESS }, //  2 Z80 #0 Code

	{ "voice.1g",    	0x10000, 0x3f23c180, 3 | BRF_PRG | BRF_ESS }, //  3 Z80 #1 Code

	{ "bkchr.2k",    	0x20000, 0xe4d47aca, 4 | BRF_GRA },           //  4 Background Tiles
	{ "bkchr.1k",    	0x20000, 0x5a1cbc1b, 4 | BRF_GRA },           //  5
	{ "bkchr.4k",    	0x20000, 0xc351bd48, 4 | BRF_GRA },           //  6
	{ "bkchr.3k",    	0x20000, 0x6bb2b050, 4 | BRF_GRA },           //  7

	{ "mchr.1d",     	0x20000, 0xab06a60b, 5 | BRF_GRA },           //  8 Foreground Tiles
	{ "mchr.1e",     	0x20000, 0xd221387d, 5 | BRF_GRA },           //  9
	{ "mchr.1g",     	0x20000, 0x1e4c1712, 5 | BRF_GRA },           // 10
	{ "mchr.1h",     	0x20000, 0xa381f529, 5 | BRF_GRA },           // 11
	{ "mchr.2d",     	0x20000, 0xe52303c4, 5 | BRF_GRA },           // 12
	{ "mchr.2e",     	0x20000, 0x23b9a6a1, 5 | BRF_GRA },           // 13
	{ "mchr.2g",     	0x20000, 0x1283ac09, 5 | BRF_GRA },           // 14
	{ "mchr.2h",     	0x20000, 0xcc6bf05c, 5 | BRF_GRA },           // 15

	{ "b1m.bin",     	0x10000, 0x64758232, 6 | BRF_GRA },           // 16 Sprites
	{ "b2m.bin",     	0x10000, 0xd958f5ad, 6 | BRF_GRA },           // 17
	{ "b1k.bin",     	0x10000, 0xe766f5fe, 6 | BRF_GRA },           // 18
	{ "b2k.bin",     	0x10000, 0xe71572d3, 6 | BRF_GRA },           // 19
	{ "b1h.bin",     	0x10000, 0x8494f38c, 6 | BRF_GRA },           // 20
	{ "b2h.bin",     	0x10000, 0x1fc5f049, 6 | BRF_GRA },           // 21
	{ "b3m.bin",     	0x10000, 0xd136802e, 6 | BRF_GRA },           // 22
	{ "b4m.bin",     	0x10000, 0xb4fa85d3, 6 | BRF_GRA },           // 23
	{ "b3k.bin",     	0x10000, 0x40e11cc8, 6 | BRF_GRA },           // 24
	{ "b4k.bin",     	0x10000, 0x5ca9716e, 6 | BRF_GRA },           // 25
	{ "b3h.bin",     	0x10000, 0x8c3d9173, 6 | BRF_GRA },           // 26
	{ "b4h.bin",     	0x10000, 0xa2df66f8, 6 | BRF_GRA },           // 27

	{ "vram.4d",     	0x04000, 0xbfadfb32, 7 | BRF_GRA },           // 28 Characters

	{ "bks1j10.5h",  	0x10000, 0x4934aacd, 8 | BRF_GRA },           // 29 Tilemaps
	{ "bks1j18.3h",  	0x10000, 0x551ffc88, 8 | BRF_GRA },           // 30
	{ "ms1j10.3g",   	0x10000, 0xf92958b8, 8 | BRF_GRA },           // 31
	{ "ms1j18.5g",   	0x10000, 0x89e35dc1, 8 | BRF_GRA },           // 32

	{ "sfb05.bin",    	0x0100, 0x864199ad, 0 | BRF_OPT },            // 33 Proms
	{ "sfb00.bin",    	0x0100, 0xbd3f8c5d, 0 | BRF_OPT },            // 34
	{ "mb7114h.12j",  	0x0100, 0x4c734b64, 0 | BRF_OPT },            // 35
	{ "mmi-7603.13h", 	0x0020, 0x06bcda53, 0 | BRF_OPT },            // 36
};

STD_ROM_PICK(sfp)
STD_ROM_FN(sfp)

static INT32 SfpInit()
{
	return DrvInit(4);
}

struct BurnDriver BurnDrvsfp = {
	"sfp", "sf", NULL, NULL, "1987",
	"Street Fighter (prototype)\0", NULL, "Capcom", "Miscellaneous",
	L"\u8857\u5934\u9738\u738B (\u5DE5\u7A0B\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_PROTOTYPE | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VSFIGHT, FBF_SF,
	NULL, sfpRomInfo, sfpRomName, NULL, NULL, NULL, NULL, SfInputInfo, SfDIPInfo,
	SfpInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 
	&DrvRecalc, 0x401, 384, 224, 4, 3
};


// Street Fighter (World) (protected)

static struct BurnRomInfo sfwRomDesc[] = {
	{ "sfs19e.2a",  	0x10000, 0x56eabd3a, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
	{ "sfs22e.2c",  	0x10000, 0x34541285, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "sfs20e.3a",  	0x10000, 0xea8d3a70, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "sfs23e.3c",  	0x10000, 0x21e7d1c7, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "sfs21e.4a",  	0x10000, 0xe0d3f410, 1 | BRF_PRG | BRF_ESS }, //  4
	{ "sfs24e.4c",  	0x10000, 0xbcd20105, 1 | BRF_PRG | BRF_ESS }, //  5

	{ "sf-02.7k",  		0x08000, 0x4a9ac534, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 #0 Code

	{ "sfu-00.1h", 		0x20000, 0xa7cce903, 3 | BRF_PRG | BRF_ESS }, //  7 Z80 #1 Code
	{ "sf-01.2k",  		0x20000, 0x86e0f0d5, 3 | BRF_PRG | BRF_ESS }, //  8

	{ "sf-39.2k",  		0x20000, 0xcee3d292, 4 | BRF_GRA },           //  9 Background Tiles
	{ "sf-38.1k",  		0x20000, 0x2ea99676, 4 | BRF_GRA },           // 10
	{ "sf-41.4k",  		0x20000, 0xe0280495, 4 | BRF_GRA },           // 11
	{ "sf-40.3k",  		0x20000, 0xc70b30de, 4 | BRF_GRA },           // 12

	{ "sf-25.1d",  		0x20000, 0x7f23042e, 5 | BRF_GRA },           // 13 Foreground Tiles
	{ "sf-28.1e",  		0x20000, 0x92f8b91c, 5 | BRF_GRA },           // 14
	{ "sf-30.1g",  		0x20000, 0xb1399856, 5 | BRF_GRA },           // 15
	{ "sf-34.1h",  		0x20000, 0x96b6ae2e, 5 | BRF_GRA },           // 16
	{ "sf-26.2d",  		0x20000, 0x54ede9f5, 5 | BRF_GRA },           // 17
	{ "sf-29.2e",  		0x20000, 0xf0649a67, 5 | BRF_GRA },           // 18
	{ "sf-31.2g",  		0x20000, 0x8f4dd71a, 5 | BRF_GRA },           // 19
	{ "sf-35.2h",  		0x20000, 0x70c00fb4, 5 | BRF_GRA },           // 20

	{ "sf-15.1m",  		0x20000, 0xfc0113db, 6 | BRF_GRA },           // 21 Sprites
	{ "sf-16.2m",  		0x20000, 0x82e4a6d3, 6 | BRF_GRA },           // 22
	{ "sf-11.1k",  		0x20000, 0xe112df1b, 6 | BRF_GRA },           // 23
	{ "sf-12.2k",  		0x20000, 0x42d52299, 6 | BRF_GRA },           // 24
	{ "sf-07.1h",  		0x20000, 0x49f340d9, 6 | BRF_GRA },           // 25
	{ "sf-08.2h",  		0x20000, 0x95ece9b1, 6 | BRF_GRA },           // 26
	{ "sf-03.1f",  		0x20000, 0x5ca05781, 6 | BRF_GRA },           // 27
	{ "sf-17.3m",  		0x20000, 0x69fac48e, 6 | BRF_GRA },           // 28
	{ "sf-18.4m",  		0x20000, 0x71cfd18d, 6 | BRF_GRA },           // 29
	{ "sf-13.3k",  		0x20000, 0xfa2eb24b, 6 | BRF_GRA },           // 30
	{ "sf-14.4k",  		0x20000, 0xad955c95, 6 | BRF_GRA },           // 31
	{ "sf-09.3h",  		0x20000, 0x41b73a31, 6 | BRF_GRA },           // 32
	{ "sf-10.4h",  		0x20000, 0x91c41c50, 6 | BRF_GRA },           // 33
	{ "sf-05.3f",  		0x20000, 0x538c7cbe, 6 | BRF_GRA },           // 34

	{ "sf-27.4d",  		0x04000, 0x2b09b36d, 7 | BRF_GRA },           // 35 Characters

	{ "sf-37.4h",  		0x10000, 0x23d09d3d, 8 | BRF_GRA },           // 36 Tilemaps
	{ "sf-36.3h",  		0x10000, 0xea16df6c, 8 | BRF_GRA },           // 37
	{ "sf-32.3g",  		0x10000, 0x72df2bd9, 8 | BRF_GRA },           // 38
	{ "sf-33.4g",  		0x10000, 0x3e99d3d5, 8 | BRF_GRA },           // 39

	{ "sfb05.mb7114h.12k",   	0x0100, 0x75af3553, 0 | BRF_OPT },    // 40 Proms
	{ "sfb10.mb7114h.11h",   	0x0100, 0xc0e56586, 0 | BRF_OPT },    // 41
	{ "sfb04.mb7114h.12j", 		0x0100, 0x4c734b64, 0 | BRF_OPT },    // 42
	{ "sfb00.mb7051.13h",		0x0020, 0x06bcda53, 0 | BRF_OPT },    // 43
	
	{ "sf.14e",  				0x01000, 0x6588891f, 0 | BRF_OPT },	  // i8751 MCU
};

STD_ROM_PICK(sfw)
STD_ROM_FN(sfw)

static INT32 SfwInit()
{
	return DrvInit(3);
}

struct BurnDriver BurnDrvsfw = {
	"sfw", "sf", NULL, NULL, "1987",
	"Street Fighter (World) (protected)\0", NULL, "Capcom", "Miscellaneous",
	L"\u8857\u5934\u9738\u738B (\u4E16\u754C\u7248) (\u5DE5\u7A0B\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VSFIGHT, FBF_SF,
	NULL, sfwRomInfo, sfwRomName, NULL, NULL, NULL, NULL, SfjInputInfo, SfusDIPInfo,
	SfwInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 
	&DrvRecalc, 0x401, 384, 224, 4, 3
};


 // Street Fighter (Plus Plus)
 // Hack by Yumeji 2014-08-01
 
 static struct BurnRomInfo sfppRomDesc[] = {
 	{ "sfpp-19.2a",     0x10000, 0x9bcaeada, 1 | BRF_PRG | BRF_ESS }, //  0 68k Code
 	{ "sfpp-22.2c",     0x10000, 0x001cffb9, 1 | BRF_PRG | BRF_ESS }, //  1
 	{ "sfpp-20.3a",     0x10000, 0x2a2abcf9, 1 | BRF_PRG | BRF_ESS }, //  2
 	{ "sfpp-23.3c",     0x10000, 0x67303dea, 1 | BRF_PRG | BRF_ESS }, //  3
 	{ "sfpp-21.4a",     0x10000, 0x7a3e0c4e, 1 | BRF_PRG | BRF_ESS }, //  4
 	{ "sfpp-24.4c",     0x10000, 0xa8ad79f5, 1 | BRF_PRG | BRF_ESS }, //  5
 
 	{ "sf-02.7k",  		0x08000, 0x4a9ac534, 2 | BRF_PRG | BRF_ESS }, //  6 Z80 #0 Code
 
 	{ "sfpp-00.1h",     0x20000, 0x4b733845, 3 | BRF_PRG | BRF_ESS }, //  7 Z80 #1 Code
 	{ "sf-01.1k",  		0x20000, 0x86e0f0d5, 3 | BRF_PRG | BRF_ESS }, //  8
 
 	{ "sf-39.2k",  		0x20000, 0xcee3d292, 4 | BRF_GRA },           //  9 Background Tiles
 	{ "sf-38.1k",  		0x20000, 0x2ea99676, 4 | BRF_GRA },           // 10
 	{ "sf-41.4k",  		0x20000, 0xe0280495, 4 | BRF_GRA },           // 11
 	{ "sf-40.3k",  		0x20000, 0xc70b30de, 4 | BRF_GRA },           // 12
 
 	{ "sf-25.1d",  		0x20000, 0x7f23042e, 5 | BRF_GRA },           // 13 Foreground Tiles
 	{ "sf-28.1e",  		0x20000, 0x92f8b91c, 5 | BRF_GRA },           // 14
 	{ "sf-30.1g",  		0x20000, 0xb1399856, 5 | BRF_GRA },           // 15
 	{ "sf-34.1h",  		0x20000, 0x96b6ae2e, 5 | BRF_GRA },           // 16
 	{ "sf-26.2d",  		0x20000, 0x54ede9f5, 5 | BRF_GRA },           // 17
 	{ "sf-29.2e",  		0x20000, 0xf0649a67, 5 | BRF_GRA },           // 18
 	{ "sf-31.2g",  		0x20000, 0x8f4dd71a, 5 | BRF_GRA },           // 19
 	{ "sf-35.2h",  		0x20000, 0x70c00fb4, 5 | BRF_GRA },           // 20
 
 	{ "sf-15.1m",  		0x20000, 0xfc0113db, 6 | BRF_GRA },           // 21 Sprites
 	{ "sf-16.2m",  		0x20000, 0x82e4a6d3, 6 | BRF_GRA },           // 22
 	{ "sf-11.1k",  		0x20000, 0xe112df1b, 6 | BRF_GRA },           // 23
 	{ "sf-12.2k",  		0x20000, 0x42d52299, 6 | BRF_GRA },           // 24
 	{ "sf-07.1h",  		0x20000, 0x49f340d9, 6 | BRF_GRA },           // 25
 	{ "sf-08.2h",  		0x20000, 0x95ece9b1, 6 | BRF_GRA },           // 26
 	{ "sf-03.1f",  		0x20000, 0x5ca05781, 6 | BRF_GRA },           // 27
 	{ "sf-17.3m",  		0x20000, 0x69fac48e, 6 | BRF_GRA },           // 28
 	{ "sf-18.4m",  		0x20000, 0x71cfd18d, 6 | BRF_GRA },           // 29
 	{ "sf-13.3k",  		0x20000, 0xfa2eb24b, 6 | BRF_GRA },           // 30
 	{ "sf-14.4k",  		0x20000, 0xad955c95, 6 | BRF_GRA },           // 31
 	{ "sf-09.3h",  		0x20000, 0x41b73a31, 6 | BRF_GRA },           // 32
 	{ "sf-10.4h",  		0x20000, 0x91c41c50, 6 | BRF_GRA },           // 33
 	{ "sf-05.3f",  		0x20000, 0x538c7cbe, 6 | BRF_GRA },           // 34
 
 	{ "sf-27.4d",  		0x04000, 0x2b09b36d, 7 | BRF_GRA },           // 35 Characters
 
 	{ "sf-37.4h",  		0x10000, 0x23d09d3d, 8 | BRF_GRA },           // 36 Tilemaps
 	{ "sf-36.3h",  		0x10000, 0xea16df6c, 8 | BRF_GRA },           // 37
 	{ "sf-32.3g",  		0x10000, 0x72df2bd9, 8 | BRF_GRA },           // 38
 	{ "sf-33.4g",  		0x10000, 0x3e99d3d5, 8 | BRF_GRA },           // 39
 
 	{ "mb7114h.12k", 	0x0100, 0x75af3553, 0 | BRF_OPT },            // 40 Proms
 	{ "mb7114h.11h", 	0x0100, 0xc0e56586, 0 | BRF_OPT },            // 41
 	{ "mb7114h.12j", 	0x0100, 0x4c734b64, 0 | BRF_OPT },            // 42
 	{ "mmi-7603.13h",	0x0020, 0x06bcda53, 0 | BRF_OPT },            // 43
 };
 
 STD_ROM_PICK(sfpp)
 STD_ROM_FN(sfpp)
 
 struct BurnDriver BurnDrvsfpp = {
 	"sfpp", "sf", NULL, NULL, "2014",
 	"Street Fighter (Plus Plus)\0", NULL, "Yumeji", "Miscellaneous",
	L"\u8857\u5934\u9738\u738B (Plus Plus)\0", NULL, NULL, NULL,
 	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED | BDF_CLONE | BDF_HACK, 2, HARWARE_CAPCOM_MISC, GBF_VSFIGHT, FBF_SF,
 	NULL, sfppRomInfo, sfppRomName, NULL, NULL, NULL, NULL, SfusInputInfo, SfusDIPInfo,
 	SfusInit, DrvExit, DrvFrame, DrvDraw, DrvScan, 
 	&DrvRecalc, 0x401, 384, 224, 4, 3
 };