// FB Alpha Last Duel driver module
// Based on MAME driver by Bryan McPhail

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "burn_ym2203.h"
#include "msm6295.h"

static UINT8 DrvInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvDip[3]        = {0, 0, 0};
static UINT8 DrvInput[3]      = {0x00, 0x00, 0x00};
static UINT8 DrvReset         = 0;

static UINT8 *AllMem              = NULL;
static UINT8 *MemEnd              = NULL;
static UINT8 *RamStart            = NULL;
static UINT8 *RamEnd              = NULL;
static UINT8 *Drv68KRom           = NULL;
static UINT8 *Drv68KRam           = NULL;
static UINT8 *DrvZ80Rom           = NULL;
static UINT8 *DrvZ80Ram           = NULL;
static UINT8 *DrvVideoRam         = NULL;
static UINT8 *DrvSpriteRam        = NULL;
static UINT8 *DrvSpriteRamBuffer  = NULL;
static UINT8 *DrvPaletteRam       = NULL;
static UINT8 *DrvScroll1Ram       = NULL;
static UINT8 *DrvScroll2Ram       = NULL;
static UINT8 *DrvChars            = NULL;
static UINT8 *DrvBgTiles          = NULL;
static UINT8 *DrvFgTiles          = NULL;
static UINT8 *DrvSprites          = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT32 *DrvPalette         = NULL;

static UINT16 DrvFgScrollX;
static UINT16 DrvFgScrollY;
static UINT16 DrvBgScrollX;
static UINT16 DrvBgScrollY;
static UINT16 DrvTmapPriority;
static UINT8 DrvZ80RomBank;
static UINT8 DrvSoundLatch;

static UINT8 DrvSpritePriMask;
static UINT8 DrvSpriteFlipYMask;

static INT32 nExtraCycles;

static struct BurnInputInfo DrvInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL  , DrvInputPort2 + 4, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL  , DrvInputPort2 + 2, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort0 + 7, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort0 + 6, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 fire 3" },

	{"P2 Coin"           , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort1 + 7, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort1 + 6, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 3, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 2, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 fire 3" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort2 + 7, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH, DrvDip + 2       , "dip"       },
};

STDINPUTINFO(Drv)

static struct BurnInputInfo LastduelInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL  , DrvInputPort2 + 7, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL  , DrvInputPort2 + 0, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort0 + 3, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort0 + 2, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort0 + 5, "p1 fire 2" },

	{"P2 Coin"           , BIT_DIGITAL  , DrvInputPort2 + 6, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort1 + 3, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort1 + 2, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort1 + 1, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort1 + 0, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort2 + 5, "service"   },
	{"Diagnostics"       , BIT_DIGITAL  , DrvInputPort2 + 3, "diag"      },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
	{"Dip 3"             , BIT_DIPSWITCH, DrvDip + 2       , "dip"       },
};

STDINPUTINFO(Lastduel)

static inline void DrvClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x03) {
		*nJoystickInputs &= ~0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x0c) {
		*nJoystickInputs &= ~0x0c;
	}
}

static inline void DrvMakeInputs()
{
	// Reset Inputs
	DrvInput[0] = DrvInput[1] = DrvInput[2] = 0x00;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvInputPort0[i] & 1) << i;
		DrvInput[1] |= (DrvInputPort1[i] & 1) << i;
		DrvInput[2] |= (DrvInputPort2[i] & 1) << i;
	}

	// Clear Opposites
	DrvClearOpposites(&DrvInput[0]);
	DrvClearOpposites(&DrvInput[1]);
}

static struct BurnDIPInfo DrvDIPList[]=
{
	DIP_OFFSET(0x14)

	// Default Values
	{0x00, 0xff, 0xff, 0xff, NULL                     },
	{0x01, 0xff, 0xff, 0xff, NULL                     },
	{0x02, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x00, 0x01, 0x80, 0x80, "Off"                    },
	{0x00, 0x01, 0x80, 0x00, "On"                     },

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Allow continue"         },
	{0x01, 0x01, 0x01, 0x00, "No"                     },
	{0x01, 0x01, 0x01, 0x01, "Yes"                    },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x01, 0x01, 0x02, 0x02, "Off"                    },
	{0x01, 0x01, 0x02, 0x00, "On"                     },

	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x01, 0x01, 0x0c, 0x08, "Easy"                   },
	{0x01, 0x01, 0x0c, 0x0c, "Normal"                 },
	{0x01, 0x01, 0x0c, 0x04, "Difficult"              },
	{0x01, 0x01, 0x0c, 0x00, "Very Difficult"         },

	{0   , 0xfe, 0   , 3   , "Cabinet"                },
	{0x01, 0x01, 0x30, 0x30, "Upright (One Player)"   },
	{0x01, 0x01, 0x30, 0x00, "Upright (Two Players)"  },
	{0x01, 0x01, 0x30, 0x10, "Cocktail"               },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x01, 0x01, 0x40, 0x00, "Off"                    },
	{0x01, 0x01, 0x40, 0x40, "On"                     },

	{0   , 0xfe, 0   , 2   , "Background Music"       },
	{0x01, 0x01, 0x80, 0x00, "Off"                    },
	{0x01, 0x01, 0x80, 0x80, "On"                     },

	// Dip3
	{0   , 0xfe, 0   , 16  , "Coin A"                 },
	{0x02, 0x01, 0xf0, 0x20, "6 Coins 1 Play"         },
	{0x02, 0x01, 0xf0, 0x40, "5 Coins 1 Play"         },
	{0x02, 0x01, 0xf0, 0x50, "4 Coins 1 Play"         },
	{0x02, 0x01, 0xf0, 0x70, "3 Coins 1 Play"         },
	{0x02, 0x01, 0xf0, 0x10, "8 Coins 3 Plays"        },
	{0x02, 0x01, 0xf0, 0x90, "2 Coins 1 Play"         },
	{0x02, 0x01, 0xf0, 0x30, "5 Coins 3 Plays"        },
	{0x02, 0x01, 0xf0, 0x60, "3 Coins 2 Plays"        },
	{0x02, 0x01, 0xf0, 0xf0, "1 Coin  1 Play"         },
	{0x02, 0x01, 0xf0, 0x80, "2 Coins 3 Plays"        },
	{0x02, 0x01, 0xf0, 0xe0, "1 Coin  2 Plays"        },
	{0x02, 0x01, 0xf0, 0xd0, "1 Coin  3 Plays"        },
	{0x02, 0x01, 0xf0, 0xc0, "1 Coin  4 Plays"        },
	{0x02, 0x01, 0xf0, 0xb0, "1 Coin  5 Plays"        },
	{0x02, 0x01, 0xf0, 0xa0, "1 Coin  6 Plays"        },
	{0x02, 0x01, 0xf0, 0x00, "Freeplay"               },

	{0   , 0xfe, 0   , 15  , "Coin B"                 },
	{0x02, 0x01, 0x0f, 0x02, "6 Coins 1 Play"         },
	{0x02, 0x01, 0x0f, 0x04, "5 Coins 1 Play"         },
	{0x02, 0x01, 0x0f, 0x05, "4 Coins 1 Play"         },
	{0x02, 0x01, 0x0f, 0x07, "3 Coins 1 Play"         },
	{0x02, 0x01, 0x0f, 0x01, "8 Coins 3 Plays"        },
	{0x02, 0x01, 0x0f, 0x09, "2 Coins 1 Play"         },
	{0x02, 0x01, 0x0f, 0x03, "5 Coins 3 Plays"        },
	{0x02, 0x01, 0x0f, 0x06, "3 Coins 2 Plays"        },
	{0x02, 0x01, 0x0f, 0x0f, "1 Coin  1 Play"         },
	{0x02, 0x01, 0x0f, 0x08, "2 Coins 3 Plays"        },
	{0x02, 0x01, 0x0f, 0x0e, "1 Coin  2 Plays"        },
	{0x02, 0x01, 0x0f, 0x0d, "1 Coin  3 Plays"        },
	{0x02, 0x01, 0x0f, 0x0c, "1 Coin  4 Plays"        },
	{0x02, 0x01, 0x0f, 0x0b, "1 Coin  5 Plays"        },
	{0x02, 0x01, 0x0f, 0x0a, "1 Coin  6 Plays"        },
};

STDDIPINFO(Drv)

static struct BurnDIPInfo LastduelDIPList[]=
{
	DIP_OFFSET(0x13)

	// Default Values
	{0x00, 0xff, 0xff, 0xff, NULL                     },
	{0x01, 0xff, 0xff, 0xff, NULL                     },
	{0x02, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x00, 0x01, 0x07, 0x00, "4 Coins 1 Play"         },
	{0x00, 0x01, 0x07, 0x01, "3 Coins 1 Play"         },
	{0x00, 0x01, 0x07, 0x02, "2 Coins 1 Play"         },
	{0x00, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x00, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x00, 0x01, 0x07, 0x05, "1 Coin  3 Plays"        },
	{0x00, 0x01, 0x07, 0x04, "1 Coin  4 Plays"        },
	{0x00, 0x01, 0x07, 0x03, "1 Coin  6 Plays"        },

	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x00, 0x01, 0x38, 0x00, "4 Coins 1 Play"         },
	{0x00, 0x01, 0x38, 0x08, "3 Coins 1 Play"         },
	{0x00, 0x01, 0x38, 0x10, "2 Coins 1 Play"         },
	{0x00, 0x01, 0x38, 0x38, "1 Coin  1 Play"         },
	{0x00, 0x01, 0x38, 0x30, "1 Coin  2 Plays"        },
	{0x00, 0x01, 0x38, 0x28, "1 Coin  3 Plays"        },
	{0x00, 0x01, 0x38, 0x20, "1 Coin  4 Plays"        },
	{0x00, 0x01, 0x38, 0x18, "1 Coin  6 Plays"        },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x01, 0x01, 0x03, 0x02, "Easy"                   },
	{0x01, 0x01, 0x03, 0x03, "Normal"                 },
	{0x01, 0x01, 0x03, 0x01, "Difficult"              },
	{0x01, 0x01, 0x03, 0x00, "Very Difficult"         },

	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x01, 0x01, 0x30, 0x20, "20000, 60000, 80000"    },
	{0x01, 0x01, 0x30, 0x30, "30000, 80000, 80000"    },
	{0x01, 0x01, 0x30, 0x10, "40000, 80000, 80000"    },
	{0x01, 0x01, 0x30, 0x00, "40000, 80000, 100000"   },

	// Dip3
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x02, 0x01, 0x03, 0x03, "3"                      },
	{0x02, 0x01, 0x03, 0x02, "4"                      },
	{0x02, 0x01, 0x03, 0x01, "6"                      },
	{0x02, 0x01, 0x03, 0x00, "8"                      },

	{0   , 0xfe, 0   , 2   , "Type"                   },
	{0x02, 0x01, 0x04, 0x04, "Car"                    },
	{0x02, 0x01, 0x04, 0x00, "Plane"                  },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x02, 0x01, 0x20, 0x00, "Off"                    },
	{0x02, 0x01, 0x20, 0x20, "On"                     },

	{0   , 0xfe, 0   , 2   , "Allow continue"         },
	{0x02, 0x01, 0x40, 0x00, "No"                     },
	{0x02, 0x01, 0x40, 0x40, "Yes"                    },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x02, 0x01, 0x80, 0x80, "Off"                    },
	{0x02, 0x01, 0x80, 0x00, "On"                     },

};

STDDIPINFO(Lastduel)

static struct BurnRomInfo DrvRomDesc[] = {
	{ "mg_04.8b",      0x20000, 0xb112257d, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "mg_03.7b",      0x20000, 0xb2672465, BRF_ESS | BRF_PRG }, //	 1
	{ "mg_02.6b",      0x20000, 0x9f5ebe16, BRF_ESS | BRF_PRG }, //	 2
	{ "mg_01.5b",      0x20000, 0x1cea2af0, BRF_ESS | BRF_PRG }, //	 3

	{ "mg_05.14j",     0x10000, 0x2fbfc945, BRF_ESS | BRF_PRG }, //  4	Z80 Program

	{ "mg_06.10k",     0x08000, 0x382ee59b, BRF_GRA },	     	 //  5	Characters

	{ "ls-12.7l",      0x40000, 0x6c1b2c6c, BRF_GRA },	     	 //  6	BG Tiles

	{ "ls-11.2l",      0x80000, 0x6bf81c64, BRF_GRA },	     	 //  7	FG Tiles

	{ "mg_m11.rom0",   0x10000, 0xee319a64, BRF_GRA },	     	 //  8	Sprites
	{ "mg_m07.rom2",   0x10000, 0xe5c0b211, BRF_GRA },	     	 //  9
	{ "mg_m12.rom1",   0x10000, 0x887ef120, BRF_GRA },	     	 // 10
	{ "mg_m08.rom3",   0x10000, 0x59709aa3, BRF_GRA },	     	 // 11
	{ "mg_m13.rom0",   0x10000, 0xeae07db4, BRF_GRA },	     	 // 12
	{ "mg_m09.rom2",   0x10000, 0x40ee83eb, BRF_GRA },	     	 // 13
	{ "mg_m14.rom1",   0x10000, 0x21e5424c, BRF_GRA },	     	 // 14
	{ "mg_m10.rom3",   0x10000, 0xb64afb54, BRF_GRA },	     	 // 15

	{ "ls-06.10e",     0x20000, 0x88d39a5b, BRF_SND },	     	 // 16	Samples
	{ "ls-05.12e",     0x20000, 0xb06e03b5, BRF_SND },	     	 // 17

	{ "29.14k",        0x00100, 0x7f862e1e, BRF_GRA },	    	 // 18	PROM (Priority)
};

STD_ROM_PICK(Drv)
STD_ROM_FN(Drv)

static struct BurnRomInfo DrvjRomDesc[] = {
	{ "mdj_04.8b",     0x20000, 0x9ebbebb1, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "mdj_03.7b",     0x20000, 0xa5579c2d, BRF_ESS | BRF_PRG }, //	 1
	{ "mg_02.6b",      0x20000, 0x9f5ebe16, BRF_ESS | BRF_PRG }, //	 2
	{ "mg_01.5b",      0x20000, 0x1cea2af0, BRF_ESS | BRF_PRG }, //	 3

	{ "mg_05.14j",     0x10000, 0x2fbfc945, BRF_ESS | BRF_PRG }, //  4	Z80 Program

	{ "mg_06.10k",     0x08000, 0x382ee59b, BRF_GRA },	     	 //  5	Characters

	{ "ls-12.7l",      0x40000, 0x6c1b2c6c, BRF_GRA },	     	 //  6	BG Tiles

	{ "ls-11.2l",      0x80000, 0x6bf81c64, BRF_GRA },	     	 //  7	FG Tiles

	{ "mg_m11.rom0",   0x10000, 0xee319a64, BRF_GRA },	     	 //  8	Sprites
	{ "mg_m07.rom2",   0x10000, 0xe5c0b211, BRF_GRA },	     	 //  9
	{ "mg_m12.rom1",   0x10000, 0x887ef120, BRF_GRA },	     	 // 10
	{ "mg_m08.rom3",   0x10000, 0x59709aa3, BRF_GRA },	     	 // 11
	{ "mg_m13.rom0",   0x10000, 0xeae07db4, BRF_GRA },	     	 // 12
	{ "mg_m09.rom2",   0x10000, 0x40ee83eb, BRF_GRA },	     	 // 13
	{ "mg_m14.rom1",   0x10000, 0x21e5424c, BRF_GRA },	     	 // 14
	{ "mg_m10.rom3",   0x10000, 0xb64afb54, BRF_GRA },	     	 // 15

	{ "ls-06.10e",     0x20000, 0x88d39a5b, BRF_SND },	     	 // 16	Samples
	{ "ls-05.12e",     0x20000, 0xb06e03b5, BRF_SND },	     	 // 17

	{ "29.14k",        0x00100, 0x7f862e1e, BRF_GRA },	     	 // 18	PROM (Priority)
};

STD_ROM_PICK(Drvj)
STD_ROM_FN(Drvj)

static struct BurnRomInfo DrvuRomDesc[] = {
	{ "mdu_04.8b",     0x20000, 0x7f7f8329, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "mdu_03.7b",     0x20000, 0x11fa542f, BRF_ESS | BRF_PRG }, //	 1
	{ "mde_02.6b",     0x20000, 0x9f5ebe16, BRF_ESS | BRF_PRG }, //	 2
	{ "mde_01.5b",     0x20000, 0x1cea2af0, BRF_ESS | BRF_PRG }, //	 3

	{ "mde_05.14j",    0x10000, 0x2fbfc945, BRF_ESS | BRF_PRG }, //  4	Z80 Program

	{ "mdu_06.10k",    0x08000, 0x54bfdc02, BRF_GRA },	     	 //  5	Characters

	{ "ls-12.7l",      0x40000, 0x6c1b2c6c, BRF_GRA },	     	 //  6	BG Tiles

	{ "ls-11.2l",      0x80000, 0x6bf81c64, BRF_GRA },	     	 //  7	FG Tiles

	{ "11.rom0",       0x10000, 0xee319a64, BRF_GRA },	     	 //  8	Sprites
	{ "07u.rom2",      0x10000, 0x7152b212, BRF_GRA },	     	 //  9
	{ "12.rom1",       0x10000, 0x887ef120, BRF_GRA },	     	 // 10
	{ "08u.rom3",      0x10000, 0x72e5d525, BRF_GRA },	     	 // 11
	{ "13.rom0",       0x10000, 0xeae07db4, BRF_GRA },	     	 // 12
	{ "09u.rom2",      0x10000, 0x7b5175cb, BRF_GRA },	     	 // 13
	{ "14.rom1",       0x10000, 0x21e5424c, BRF_GRA },	     	 // 14
	{ "10u.rom3",      0x10000, 0x6db7ca64, BRF_GRA },	     	 // 15

	{ "ls-06.10e",     0x20000, 0x88d39a5b, BRF_SND },	     	 // 16	Samples
	{ "ls-05.12e",     0x20000, 0xb06e03b5, BRF_SND },	     	 // 17

	{ "29.14k",        0x00100, 0x7f862e1e, BRF_GRA },	     	 // 18	PROM (Priority)
};

STD_ROM_PICK(Drvu)
STD_ROM_FN(Drvu)

static struct BurnRomInfo Leds2011RomDesc[] = {
	{ "lse_04.8b",     0x20000, 0x166c0576, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "lse_03.7b",     0x20000, 0x0c8647b6, BRF_ESS | BRF_PRG }, //	 1
	{ "ls-02.6b",      0x20000, 0x05c0285e, BRF_ESS | BRF_PRG }, //	 2
	{ "ls-01.5b",      0x20000, 0x8bf934dd, BRF_ESS | BRF_PRG }, //	 3

	{ "ls-07.14j",     0x10000, 0x98af7838, BRF_ESS | BRF_PRG }, //  4	Z80 Program

	{ "ls-08.10k",     0x08000, 0x8803cf49, BRF_GRA },	     	 //  5	Characters

	{ "ls-12.7l",      0x40000, 0x6c1b2c6c, BRF_GRA },	     	 //  6	BG Tiles

	{ "ls-11.2l",      0x80000, 0x6bf81c64, BRF_GRA },	     	 //  7	FG Tiles

	{ "ls-10.13a",     0x40000, 0xdb2c5883, BRF_GRA },	     	 //  8  Sprites
	{ "ls-09.5a",      0x40000, 0x89949efb, BRF_GRA },	     	 //  9

	{ "ls-06.10e",     0x20000, 0x88d39a5b, BRF_SND },	     	 // 10	Samples
	{ "ls-05.12e",     0x20000, 0xb06e03b5, BRF_SND },	     	 // 11

	{ "29.14k",        0x00100, 0x7f862e1e, BRF_GRA },	     	 // 12	PROM (Priority)
};

STD_ROM_PICK(Leds2011)
STD_ROM_FN(Leds2011)

static struct BurnRomInfo Leds2011uRomDesc[] = {
	{ "lsu-04.8b",     0x20000, 0x56a2f079, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "lsu-03.7b",     0x20000, 0x9b6408c0, BRF_ESS | BRF_PRG }, //	 1
	{ "ls-02.6b",      0x20000, 0x05c0285e, BRF_ESS | BRF_PRG }, //	 2
	{ "ls-01.5b",      0x20000, 0x8bf934dd, BRF_ESS | BRF_PRG }, //	 3

	{ "ls-07.14j",     0x10000, 0x98af7838, BRF_ESS | BRF_PRG }, //  4	Z80 Program

	{ "ls-08.10k",     0x08000, 0x8803cf49, BRF_GRA },	     	 //  5	Characters

	{ "ls-12.7l",      0x40000, 0x6c1b2c6c, BRF_GRA },	     	 //  6	BG Tiles

	{ "ls-11.2l",      0x80000, 0x6bf81c64, BRF_GRA },	     	 //  7	FG Tiles

	{ "ls-10.13a",     0x40000, 0xdb2c5883, BRF_GRA },	     	 //  8  Sprites
	{ "ls-09.5a",      0x40000, 0x89949efb, BRF_GRA },	     	 //  9

	{ "ls-06.10e",     0x20000, 0x88d39a5b, BRF_SND },	     	 // 10	Samples
	{ "ls-05.12e",     0x20000, 0xb06e03b5, BRF_SND },	     	 // 11

	{ "29.14k",        0x00100, 0x7f862e1e, BRF_GRA },	     	 // 12	PROM (Priority)
};

STD_ROM_PICK(Leds2011u)
STD_ROM_FN(Leds2011u)

static struct BurnRomInfo LastduelRomDesc[] = {
	{ "ldu_06b.13k",   0x20000, 0x0e71acaf, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "ldu_05b.12k",   0x20000, 0x47a85bea, BRF_ESS | BRF_PRG }, //	 1
	{ "ldu_04b.11k",   0x10000, 0xaa4bf001, BRF_ESS | BRF_PRG }, //	 2
	{ "ldu_03b.9k",    0x10000, 0xbbaac8ab, BRF_ESS | BRF_PRG }, //	 3

	{ "ld_02.16h",     0x10000, 0x91834d0c, BRF_ESS | BRF_PRG }, //  4	Z80 Program

	{ "ld_01.12f",     0x08000, 0xad3c6f87, BRF_GRA },	     	 //  5	Characters

	{ "ld-15.6p",      0x20000, 0xd977a175, BRF_GRA },	     	 //  6	BG Tiles
	{ "ld-13.6m",      0x20000, 0xbc25729f, BRF_GRA },	     	 //  7

	{ "ld-14.15n",     0x80000, 0xd0653739, BRF_GRA },	     	 //  8	FG Tiles

	{ "ld-09.12a",     0x20000, 0x6efadb74, BRF_GRA },	     	 //  9  Sprites
	{ "ld-10.17a",     0x20000, 0xb8d3b2e3, BRF_GRA },	     	 // 10
	{ "ld-11.12b",     0x20000, 0x49d4dbbd, BRF_GRA },	     	 // 11
	{ "ld-12.17b",     0x20000, 0x313e5338, BRF_GRA },	     	 // 12


	{ "ld.3d",         0x00100, 0x729a1ddc, BRF_GRA },  	     // 13	PROM (Priority)
};

STD_ROM_PICK(Lastduel)
STD_ROM_FN(Lastduel)

static struct BurnRomInfo LastdueloRomDesc[] = {
	{ "ldu_06.13k",    0x20000, 0x4228a00b, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "ldu_05.12k",    0x20000, 0x7260434f, BRF_ESS | BRF_PRG }, //	 1
	{ "ldu_04.11k",    0x10000, 0x429fb964, BRF_ESS | BRF_PRG }, //	 2
	{ "ldu_03.9k",     0x10000, 0x5aa4df72, BRF_ESS | BRF_PRG }, //	 3

	{ "ld_02.16h",     0x10000, 0x91834d0c, BRF_ESS | BRF_PRG }, //  4	Z80 Program

	{ "ld_01.12f",     0x08000, 0xad3c6f87, BRF_GRA },	     	 //  5	Characters

	{ "ld-15.6p",      0x20000, 0xd977a175, BRF_GRA },	     	 //  6	BG Tiles
	{ "ld-13.6m",      0x20000, 0xbc25729f, BRF_GRA },	     	 //  7

	{ "ld-14.15n",     0x80000, 0xd0653739, BRF_GRA },	     	 //  8	FG Tiles

	{ "ld-09.12a",     0x20000, 0x6efadb74, BRF_GRA },	     	 //  9  Sprites
	{ "ld-10.17a",     0x20000, 0xb8d3b2e3, BRF_GRA },	     	 // 10
	{ "ld-11.12b",     0x20000, 0x49d4dbbd, BRF_GRA },	     	 // 11
	{ "ld-12.17b",     0x20000, 0x313e5338, BRF_GRA },	     	 // 12

	{ "ld.3d",         0x00100, 0x729a1ddc, BRF_GRA },  	     // 13	PROM (Priority)
};

STD_ROM_PICK(Lastduelo)
STD_ROM_FN(Lastduelo)

static struct BurnRomInfo LastdueljRomDesc[] = {
	{ "ld_06.13k",     0x20000, 0x58a9e12b, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "ld_05.12k",     0x20000, 0x14685d78, BRF_ESS | BRF_PRG }, //	 1
	{ "ld_04.11k",     0x10000, 0xaa4bf001, BRF_ESS | BRF_PRG }, //	 2
	{ "ld_03.9k",      0x10000, 0xbbaac8ab, BRF_ESS | BRF_PRG }, //	 3

	{ "ld_02.16h",     0x10000, 0x91834d0c, BRF_ESS | BRF_PRG }, //  4	Z80 Program

	{ "ld_01.12f",     0x08000, 0xad3c6f87, BRF_GRA },	     	 //  5	Characters

	{ "ld-15.6p",      0x20000, 0xd977a175, BRF_GRA },	     	 //  6	BG Tiles
	{ "ld-13.6m",      0x20000, 0xbc25729f, BRF_GRA },	     	 //  7

	{ "ld-14.15n",     0x80000, 0xd0653739, BRF_GRA },	     	 //  8	FG Tiles

	{ "ld-09.12a",     0x20000, 0x6efadb74, BRF_GRA },	     	 //  9  Sprites
	{ "ld-10.17a",     0x20000, 0xb8d3b2e3, BRF_GRA },	     	 // 10
	{ "ld-11.12b",     0x20000, 0x49d4dbbd, BRF_GRA },	     	 // 11
	{ "ld-12.17b",     0x20000, 0x313e5338, BRF_GRA },	     	 // 12

	{ "ld.3d",         0x00100, 0x729a1ddc, BRF_GRA },  	     // 13	PROM (Priority)
};

STD_ROM_PICK(Lastduelj)
STD_ROM_FN(Lastduelj)

static struct BurnRomInfo LastduelbRomDesc[] = {
	{ "ld_08.bin",     0x10000, 0x43811a96, BRF_ESS | BRF_PRG }, //  0	68000 Program Code
	{ "ld_07.bin",     0x10000, 0x63c30946, BRF_ESS | BRF_PRG }, //	 1
	{ "ld_04.bin",     0x10000, 0x46a4e0f8, BRF_ESS | BRF_PRG }, //	 2
	{ "ld_03.bin",     0x10000, 0x8d5f204a, BRF_ESS | BRF_PRG }, //	 3
	{ "ldu-04.rom",    0x10000, 0x429fb964, BRF_ESS | BRF_PRG }, //	 4
	{ "ldu-03.rom",    0x10000, 0x5aa4df72, BRF_ESS | BRF_PRG }, //	 5

	{ "ld_02.bin",     0x10000, 0x91834d0c, BRF_ESS | BRF_PRG }, //  6	Z80 Program

	{ "ld_01.bin",     0x08000, 0xad3c6f87, BRF_GRA },	     	 //  7	Characters

	{ "ld_17.bin",     0x10000, 0x7188bfdd, BRF_GRA },	     	 //  8	BG Tiles
	{ "ld_18.bin",     0x10000, 0xa62af66a, BRF_GRA },	     	 //  9
	{ "ld_19.bin",     0x10000, 0x4b762e50, BRF_GRA },	     	 // 10
	{ "ld_20.bin",     0x10000, 0xb140188e, BRF_GRA },	     	 // 11

	{ "ld_28.bin",     0x10000, 0x06778248, BRF_GRA },	     	 // 12	FG Tiles
	{ "ld_27.bin",     0x10000, 0x48c78675, BRF_GRA },	     	 // 13
	{ "ld_26.bin",     0x10000, 0xb0edac81, BRF_GRA },	     	 // 14
	{ "ld_25.bin",     0x10000, 0xc541ae9a, BRF_GRA },	     	 // 15
	{ "ld_24.bin",     0x10000, 0x66eac4df, BRF_GRA },	     	 // 16
	{ "ld_23.bin",     0x10000, 0xd817332c, BRF_GRA },	     	 // 17
	{ "ld_22.bin",     0x10000, 0xf80f8812, BRF_GRA },	     	 // 18
	{ "ld_21.bin",     0x10000, 0xb74f0c0e, BRF_GRA },	     	 // 19

	{ "ld_11.bin",     0x10000, 0x1a0d180e, BRF_GRA },	     	 // 20  Sprites
	{ "ld_12.bin",     0x10000, 0xb2745e26, BRF_GRA },	     	 // 21
	{ "ld_13.bin",     0x10000, 0xa1a598ac, BRF_GRA },	     	 // 22
	{ "ld_14.bin",     0x10000, 0xedf515cc, BRF_GRA },	     	 // 23
	{ "ld_09.bin",     0x10000, 0xf8fd5243, BRF_GRA },	     	 // 24
	{ "ld_10.bin",     0x10000, 0xb49ad746, BRF_GRA },	     	 // 25
	{ "ld_15.bin",     0x10000, 0x96b13bbc, BRF_GRA },	     	 // 26
	{ "ld_16.bin",     0x10000, 0x9d80f7e6, BRF_GRA },	     	 // 27

	{ "ld.3d",         0x00100, 0x729a1ddc, BRF_GRA },  	     // 28	PROM (Priority)
};

STD_ROM_PICK(Lastduelb)
STD_ROM_FN(Lastduelb)

static INT32 MemIndex()
{
	UINT8 *Next; Next = AllMem;

	Drv68KRom              = Next; Next += 0x80000;
	DrvZ80Rom              = Next; Next += 0x10000;
	MSM6295ROM             = Next; Next += 0x40000;

	RamStart               = Next;

	Drv68KRam              = Next; Next += 0x20000;
	DrvZ80Ram              = Next; Next += 0x00800;
	DrvSpriteRam           = Next; Next += 0x00800;
	DrvSpriteRamBuffer     = Next; Next += 0x00800;
	DrvVideoRam            = Next; Next += 0x02000;
	DrvPaletteRam          = Next; Next += 0x00800;
	DrvScroll1Ram          = Next; Next += 0x04000;
	DrvScroll2Ram          = Next; Next += 0x08000;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x0800 * 8 * 8;
	DrvBgTiles             = Next; Next += 0x0800 * 16 * 16;
	DrvFgTiles             = Next; Next += 0x1000 * 16 * 16;
	DrvSprites             = Next; Next += 0x1000 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x00800 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();

	ZetOpen(0);
	ZetReset();
	ZetClose();

	BurnYM2203Reset();

	DrvFgScrollX = 0;
	DrvFgScrollY = 0;
	DrvBgScrollX = 0;
	DrvBgScrollY = 0;
	DrvTmapPriority = 0;
	DrvZ80RomBank = 0;
	DrvSoundLatch = 0;

	nExtraCycles = 0;

	HiscoreReset();

	return 0;
}

static void MadgearZ80BankSwitch(INT32 nBank)
{
	DrvZ80RomBank = nBank & 1;
	ZetMapMemory(DrvZ80Rom + 0x8000 + DrvZ80RomBank * 0x4000, 0x8000, 0xbfff, MAP_ROM);
}

static INT32 MadgearDoReset()
{
	DrvDoReset();

	ZetOpen(0);
	MadgearZ80BankSwitch(0);
	ZetClose();

	MSM6295Reset(0);

	return 0;
}

static UINT8 __fastcall Madgear68KReadByte(UINT32 a)
{
	switch (a) {
		case 0xfc4000: {
			return DrvDip[0];
		}

		case 0xfc4001: {
			return DrvDip[1];
		}

		case 0xfc4002: {
			return DrvDip[2];
		}

		case 0xfc4004: {
			return 0xff - DrvInput[0];
		}

		case 0xfc4005: {
			return 0xff - DrvInput[1];
		}

		case 0xfc4006: {
			return 0xff - DrvInput[2];
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Madgear68KWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0xfc4000: {
			// flip
			return;
		}

		case 0xfc4002: {
			DrvSoundLatch = d & 0xff;
			return;
		}

		case 0xfd0000: {
			DrvFgScrollY = d & 0x1ff;
			return;
		}

		case 0xfd0002: {
			DrvFgScrollX = d & 0x3ff;
			return;
		}

		case 0xfd0004: {
			DrvBgScrollY = d & 0x1ff;
			return;
		}

		case 0xfd0006: {
			DrvBgScrollX = d & 0x3ff;
			return;
		}

		case 0xfd0008: {
			// ???
			return;
		}

		case 0xfd000e: {
			DrvTmapPriority = d;
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall MadgearZ80Read(UINT16 a)
{
	switch (a) {
		case 0xf000: {
			return BurnYM2203Read(0, 0);
		}

		case 0xf002: {
			return BurnYM2203Read(1, 0);
		}

		case 0xf006: {
			return DrvSoundLatch;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

static void __fastcall MadgearZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xf000: {
			BurnYM2203Write(0, 0, d);
			return;
		}

		case 0xf001: {
			BurnYM2203Write(0, 1, d);
			return;
		}

		case 0xf002: {
			BurnYM2203Write(1, 0, d);
			return;
		}

		case 0xf003: {
			BurnYM2203Write(1, 1, d);
			return;
		}

		case 0xf004: {
			MSM6295Write(0, d);
			return;
		}

		case 0xf00a: {
			MadgearZ80BankSwitch(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

static void __fastcall Lastduel68KWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0xfc4001: {
			// flip
			return;
		}

		case 0xfc4003: {
			DrvSoundLatch = d & 0xff;
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Lastduel68KReadWord(UINT32 a)
{
	switch (a) {
		case 0xfc4000: {
			return 0xffff - ((DrvInput[1] << 8) | DrvInput[0]);
		}

		case 0xfc4002: {
			return 0xffff - DrvInput[2];
		}

		case 0xfc4004: {
			return (DrvDip[1] << 8) | DrvDip[0];
		}

		case 0xfc4006: {
			return DrvDip[2];
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K Read Word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Lastduel68KWriteWord(UINT32 a, UINT16 d)
{
	switch (a) {
		case 0xfc0000:
		case 0xfc0002: {
			// NOP
			return;
		}

		case 0xfc8000: {
			DrvFgScrollY = d & 0x3ff;
			return;
		}

		case 0xfc8002: {
			DrvFgScrollX = d & 0x3ff;
			return;
		}

		case 0xfc8004: {
			DrvBgScrollY = d & 0x3ff;
			return;
		}

		case 0xfc8006: {
			DrvBgScrollX = d & 0x3ff;
			return;
		}

		case 0xfc8008:
		case 0xfc800e: {
			// ???
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall LastduelZ80Read(UINT16 a)
{
	switch (a) {
		case 0xe800: {
			return BurnYM2203Read(0, 0);
		}

		case 0xf000: {
			return BurnYM2203Read(1, 0);
		}

		case 0xf800: {
			return DrvSoundLatch;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

static void __fastcall LastduelZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xe800: {
			BurnYM2203Write(0, 0, d);
			return;
		}

		case 0xe801: {
			BurnYM2203Write(0, 1, d);
			return;
		}

		case 0xf000: {
			BurnYM2203Write(1, 0, d);
			return;
		}

		case 0xf001: {
			BurnYM2203Write(1, 1, d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

inline static void DrvYM2203IRQHandler(INT32, INT32 nStatus)
{
	ZetSetIRQLine(0, (nStatus) ? CPU_IRQSTATUS_ACK : CPU_IRQSTATUS_NONE);
}

static INT32 CharPlaneOffsets[2]     = { 4, 0 };
static INT32 CharXOffsets[8]         = { 0, 1, 2, 3, 8, 9, 10, 11 };
static INT32 CharYOffsets[8]         = { 0, 16, 32, 48, 64, 80, 96, 112 };
static INT32 BgTilePlaneOffsets[4]   = { 12, 8, 4, 0 };
static INT32 FgTilePlaneOffsets[4]   = { 4, 0, 12, 8 };
static INT32 TileXOffsets[16]        = { 0, 1, 2, 3, 16, 17, 18, 19, 512, 513, 514, 515, 528, 529, 530, 531 };
static INT32 TileYOffsets[16]        = { 0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 480 };
static INT32 SpritePlaneOffsets[4]   = { 16, 0, 24, 8 };
static INT32 SpriteXOffsets[16]      = { STEP8(0,1), STEP8(512,1) };
static INT32 SpriteYOffsets[16]      = { STEP16(0,32) };

static INT32 DrvInit() // Madgear, Ledstorm
{
	INT32 nRet = 0;

	BurnAllocMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x40001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x40000, 3, 2); if (nRet != 0) return 1;

	// Load Z80 Program Rom
	nRet = BurnLoadRom(DrvZ80Rom, 4, 1); if (nRet != 0) return 1;

	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 5, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);

	// Load and decode the bg tiles
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom, 6, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, BgTilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, DrvTempRom, DrvBgTiles);

	// Load and decode the fg tiles
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom, 7, 1); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 16, 16, FgTilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, DrvTempRom, DrvFgTiles);

	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00002,  8, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40002,  9, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 10, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000, 11, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00003, 12, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40003, 13, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00001, 14, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40001, 15, 4); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x400, DrvTempRom, DrvSprites);

	// Load the samples
	nRet = BurnLoadRom(MSM6295ROM + 0x00000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(MSM6295ROM + 0x20000, 17, 1); if (nRet != 0) return 1;

	BurnFree(DrvTempRom);

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x07ffff, MAP_ROM);
	SekMapMemory(DrvSpriteRam        , 0xfc1800, 0xfc1fff, MAP_RAM);
	SekMapMemory(DrvVideoRam         , 0xfc8000, 0xfc9fff, MAP_RAM);
	SekMapMemory(DrvPaletteRam       , 0xfcc000, 0xfcc7ff, MAP_RAM);
	SekMapMemory(DrvScroll1Ram       , 0xfd4000, 0xfd7fff, MAP_RAM);
	SekMapMemory(DrvScroll2Ram       , 0xfd8000, 0xfdffff, MAP_RAM);
	SekMapMemory(Drv68KRam           , 0xff0000, 0xffffff, MAP_RAM);
	SekSetWriteWordHandler(0, Madgear68KWriteWord);
	SekSetReadByteHandler(0, Madgear68KReadByte);
	SekClose();

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(MadgearZ80Read);
	ZetSetWriteHandler(MadgearZ80Write);
	ZetMapMemory(DrvZ80Rom          , 0x0000, 0x7fff, MAP_ROM);
	ZetMapMemory(DrvZ80Rom + 0x8000 , 0x8000, 0xbfff, MAP_ROM);
	ZetMapMemory(DrvZ80Ram          , 0xd000, 0xd7ff, MAP_RAM);
	ZetClose();

	GenericTilesInit();
	DrvSpritePriMask = 0x10;
	DrvSpriteFlipYMask = 0x80;

	BurnYM2203Init(2, 3579545, &DrvYM2203IRQHandler, 0);
	BurnTimerAttachZet(3579545);
	BurnYM2203SetAllRoutes(0, 0.40, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetAllRoutes(1, 0.40, BURN_SND_ROUTE_BOTH);

	MSM6295Init(0, 7575, 1);
	MSM6295SetRoute(0, 0.98, BURN_SND_ROUTE_BOTH);

	// Reset the driver
	MadgearDoReset();

	return 0;
}

static INT32 Leds2011Init()
{
	INT32 nRet = 0;

	BurnAllocMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x40001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x40000, 3, 2); if (nRet != 0) return 1;

	// Load Z80 Program Rom
	nRet = BurnLoadRom(DrvZ80Rom, 4, 1); if (nRet != 0) return 1;

	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 5, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);

	// Load and decode the bg tiles
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom, 6, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, BgTilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, DrvTempRom, DrvBgTiles);

	// Load and decode the fg tiles
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom, 7, 1); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 16, 16, FgTilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, DrvTempRom, DrvFgTiles);

	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00001,  8, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  9, 2); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x400, DrvTempRom, DrvSprites);

	// Load the samples
	nRet = BurnLoadRom(MSM6295ROM + 0x00000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(MSM6295ROM + 0x20000, 11, 1); if (nRet != 0) return 1;

	BurnFree(DrvTempRom);

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x07ffff, MAP_ROM);
	SekMapMemory(DrvSpriteRam        , 0xfc1800, 0xfc1fff, MAP_RAM);
	SekMapMemory(DrvVideoRam         , 0xfc8000, 0xfc9fff, MAP_RAM);
	SekMapMemory(DrvPaletteRam       , 0xfcc000, 0xfcc7ff, MAP_RAM);
	SekMapMemory(DrvScroll1Ram       , 0xfd4000, 0xfd7fff, MAP_RAM);
	SekMapMemory(DrvScroll2Ram       , 0xfd8000, 0xfdffff, MAP_RAM);
	SekMapMemory(Drv68KRam           , 0xff0000, 0xffffff, MAP_RAM);
	SekSetWriteWordHandler(0, Madgear68KWriteWord);
	SekSetReadByteHandler(0, Madgear68KReadByte);
	SekClose();

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(MadgearZ80Read);
	ZetSetWriteHandler(MadgearZ80Write);
	ZetMapMemory(DrvZ80Rom          , 0x0000, 0x7fff, MAP_ROM);
	ZetMapMemory(DrvZ80Rom + 0x8000 , 0x8000, 0xbfff, MAP_ROM);
	ZetMapMemory(DrvZ80Ram          , 0xd000, 0xd7ff, MAP_RAM);
	ZetClose();

	GenericTilesInit();
	DrvSpritePriMask = 0x10;
	DrvSpriteFlipYMask = 0x80;

	BurnYM2203Init(2, 3579545, &DrvYM2203IRQHandler, 0);
	BurnTimerAttachZet(3579545);
	BurnYM2203SetAllRoutes(0, 0.40, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetAllRoutes(1, 0.40, BURN_SND_ROUTE_BOTH);

	MSM6295Init(0, 7575, 1);
	MSM6295SetRoute(0, 0.98, BURN_SND_ROUTE_BOTH);

	// Reset the driver
	MadgearDoReset();

	return 0;
}

static INT32 LastduelInit()
{
	INT32 nRet = 0;

	BurnAllocMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x40001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x40000, 3, 2); if (nRet != 0) return 1;

	// Load Z80 Program Rom
	nRet = BurnLoadRom(DrvZ80Rom, 4, 1); if (nRet != 0) return 1;

	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 5, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);

	// Load and decode the bg tiles
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00001, 6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 7, 2); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, BgTilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, DrvTempRom, DrvBgTiles);

	// Load and decode the fg tiles
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 8, 1); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 16, 16, BgTilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, DrvTempRom, DrvFgTiles);

	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  9, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00001, 10, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00002, 11, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00003, 12, 4); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x400, DrvTempRom, DrvSprites);

	BurnFree(DrvTempRom);

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x05ffff, MAP_ROM);
	SekMapMemory(DrvSpriteRam        , 0xfc0800, 0xfc0fff, MAP_RAM);
	SekMapMemory(DrvVideoRam         , 0xfcc000, 0xfcdfff, MAP_RAM);
	SekMapMemory(DrvScroll1Ram       , 0xfd0000, 0xfd3fff, MAP_RAM);
	SekMapMemory(DrvScroll2Ram       , 0xfd4000, 0xfd7fff, MAP_RAM);
	SekMapMemory(DrvPaletteRam       , 0xfd8000, 0xfd87ff, MAP_RAM);
	SekMapMemory(Drv68KRam           , 0xfe0000, 0xffffff, MAP_RAM);
	SekSetReadWordHandler(0, Lastduel68KReadWord);
	SekSetWriteWordHandler(0, Lastduel68KWriteWord);
	SekSetWriteByteHandler(0, Lastduel68KWriteByte);
	SekClose();

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(LastduelZ80Read);
	ZetSetWriteHandler(LastduelZ80Write);
	ZetMapMemory(DrvZ80Rom           , 0x0000, 0xdfff, MAP_ROM);
	ZetMapMemory(DrvZ80Ram           , 0xe000, 0xe7ff, MAP_RAM);
	ZetClose();

	GenericTilesInit();
	DrvSpritePriMask = 0x00;
	DrvSpriteFlipYMask = 0x40;

	BurnYM2203Init(2, 3579545, &DrvYM2203IRQHandler, 0);
	BurnTimerAttachZet(3579545);
	BurnYM2203SetAllRoutes(0, 0.40, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetPSGVolume(0, 0.25)
	BurnYM2203SetAllRoutes(1, 0.40, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetPSGVolume(1, 0.25)

	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 LastduelbInit()
{
	INT32 nRet = 0;

	BurnAllocMemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x80000);

	// Load 68000 Program Roms
	nRet = BurnLoadRom(Drv68KRom + 0x00001, 0, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x00000, 1, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x20001, 2, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x20000, 3, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x40001, 4, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(Drv68KRom + 0x40000, 5, 2); if (nRet != 0) return 1;

	// Load Z80 Program Rom
	nRet = BurnLoadRom(DrvZ80Rom, 6, 1); if (nRet != 0) return 1;

	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 7, 1); if (nRet != 0) return 1;
	GfxDecode(0x0800, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);

	// Load and decode the bg tiles
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00001,  8, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20001,  9, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 10, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 11, 2); if (nRet != 0) return 1;
	GfxDecode(0x0800, 4, 16, 16, BgTilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, DrvTempRom, DrvBgTiles);

	// Load and decode the fg tiles
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00001, 12, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 13, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20001, 14, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 15, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40001, 16, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000, 17, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x60001, 18, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x60000, 19, 2); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 16, 16, BgTilePlaneOffsets, TileXOffsets, TileYOffsets, 0x400, DrvTempRom, DrvFgTiles);

	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x80000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 20, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40000, 21, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00001, 22, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40001, 23, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00002, 24, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40002, 25, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x00003, 26, 4); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x40003, 27, 4); if (nRet != 0) return 1;
	GfxDecode(0x1000, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x400, DrvTempRom, DrvSprites);

	BurnFree(DrvTempRom);

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Drv68KRom           , 0x000000, 0x05ffff, MAP_ROM);
	SekMapMemory(DrvSpriteRam        , 0xfc0800, 0xfc0fff, MAP_RAM);
	SekMapMemory(DrvVideoRam         , 0xfcc000, 0xfcdfff, MAP_RAM);
	SekMapMemory(DrvScroll1Ram       , 0xfd0000, 0xfd3fff, MAP_RAM);
	SekMapMemory(DrvScroll2Ram       , 0xfd4000, 0xfd7fff, MAP_RAM);
	SekMapMemory(DrvPaletteRam       , 0xfd8000, 0xfd87ff, MAP_RAM);
	SekMapMemory(Drv68KRam           , 0xfe0000, 0xffffff, MAP_RAM);
	SekSetReadWordHandler(0, Lastduel68KReadWord);
	SekSetWriteWordHandler(0, Lastduel68KWriteWord);
	SekSetWriteByteHandler(0, Lastduel68KWriteByte);
	SekClose();

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(LastduelZ80Read);
	ZetSetWriteHandler(LastduelZ80Write);
	ZetMapMemory(DrvZ80Rom           , 0x0000, 0xdfff, MAP_ROM);
	ZetMapMemory(DrvZ80Ram           , 0xe000, 0xe7ff, MAP_RAM);
	ZetClose();

	GenericTilesInit();
	DrvSpritePriMask = 0x00;
	DrvSpriteFlipYMask = 0x40;

	BurnYM2203Init(2, 3579545, &DrvYM2203IRQHandler, 0);
	BurnTimerAttachZet(3579545);
	BurnYM2203SetAllRoutes(0, 0.40, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetPSGVolume(0, 0.25)
	BurnYM2203SetAllRoutes(1, 0.40, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetPSGVolume(1, 0.25)

	// Reset the driver
	DrvDoReset();

	return 0;
}

static INT32 DrvExit()
{
	SekExit();
	ZetExit();

	BurnYM2203Exit();

	GenericTilesExit();

	DrvFgScrollX = 0;
	DrvFgScrollY = 0;
	DrvBgScrollX = 0;
	DrvBgScrollY = 0;
	DrvSpritePriMask = 0;
	DrvSpriteFlipYMask = 0;
	DrvZ80RomBank = 0;
	DrvSoundLatch = 0;

	BurnFreeMemIndex();

	return 0;
}

static INT32 MadgearExit()
{
	MSM6295Exit(0);
	return DrvExit();
}

inline static UINT32 CalcCol(UINT16 nColour)
{
	INT32 i, r, g, b;

	i = (nColour & 15) + 0x10;
	r = ((nColour >> 12) & 15) * i * 0x11 / 0x1f;
	g = ((nColour >>  8) & 15) * i * 0x11 / 0x1f;
	b = ((nColour >>  4) & 15) * i * 0x11 / 0x1f;

	return BurnHighCol(r, g, b, 0);
}

static void DrvCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)DrvPaletteRam, pd = DrvPalette; i < 0x800; i++, ps++, pd++) {
		*pd = CalcCol(BURN_ENDIAN_SWAP_INT16(*ps));
	}
}

static void DrvTileDraw(UINT8 *gfx, INT32 Code, INT32 Colour, INT32 x, INT32 y, INT32 xFlip, INT32 yFlip, INT32 transparent, INT32 tcolor)
{
	if (transparent)
	{
		Draw16x16MaskTile(pTransDraw, Code, x, y, xFlip, yFlip, Colour, 4, tcolor, 0, gfx);
	}
	else
	{
		Draw16x16Tile(pTransDraw, Code, x, y, xFlip, yFlip, Colour, 4, 0, gfx);
	}
}

static void DrvRenderBgLayer(INT32 nTransparent)
{
	INT32 mx, my, Code, Colour, x, y, TileIndex, Flip, xFlip, yFlip;

	UINT16 *VideoRam = (UINT16*)DrvScroll2Ram;

	for (mx = 0; mx < 32; mx++) {
		for (my = 0; my < 64; my++) {
			TileIndex = (my * 32) + mx;
			Code = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]) & 0x1fff;
			Colour = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex + 0x800]);
			Flip = (Colour & 0x60) >> 5;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			Colour &= 0x0f;

			y = 16 * mx;
			x = 16 * my;

			x -= DrvBgScrollX;
			y -= DrvBgScrollY;
			if (x < -16) x += 1024;
			if (y < -16) y += 512;

			DrvTileDraw(DrvBgTiles, Code, Colour, x - 64, y - 8, xFlip, yFlip, nTransparent, 0xf);
		}
	}
}

static void LastduelRenderBgLayer(INT32 nTransparent)
{
	INT32 mx, my, Code, Colour, x, y, TileIndex = 0, Flip, xFlip, yFlip;

	UINT16 *VideoRam = (UINT16*)DrvScroll2Ram;

	for (my = 0; my < 64; my++) {
		for (mx = 0; mx < 64; mx++) {
			Code = BURN_ENDIAN_SWAP_INT16(VideoRam[2 * TileIndex]) & 0x1fff;
			Colour = BURN_ENDIAN_SWAP_INT16(VideoRam[2 * TileIndex + 1]);
			Flip = (Colour & 0x60) >> 5;
			xFlip = (Flip >> 0) & 0x01;
			yFlip = (Flip >> 1) & 0x01;
			Colour &= 0x0f;

			x = 16 * mx;
			y = 16 * my;

			x -= DrvBgScrollX;
			y -= DrvBgScrollY;
			if (x < -16) x += 1024;
			if (y < -16) y += 1024;

			DrvTileDraw(DrvBgTiles, Code, Colour, x - 64, y - 8, xFlip, yFlip, nTransparent, 0);

			TileIndex++;
		}
	}
}

static void DrvRenderSprites(INT32 Priority)
{
	INT32 Offset;
	UINT16 *SpriteRam = (UINT16*)DrvSpriteRamBuffer;

	for (Offset = 0x400 - 4; Offset >= 0; Offset -= 4) {
		INT32 Attr, sy, sx, xFlip, yFlip, Code, Colour;

		Attr = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 1]);
		if (DrvSpritePriMask) {
			if (Priority == 1 && (Attr & DrvSpritePriMask)) continue;
			if (Priority == 0 && !(Attr & DrvSpritePriMask)) continue;
		}

		Code = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset]) & 0xfff;
		sx = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 3]) & 0x1ff;
		sy = BURN_ENDIAN_SWAP_INT16(SpriteRam[Offset + 2]) & 0x1ff;
		if (sy > 0x100) sy -= 0x200;

		xFlip = Attr & 0x20;
		yFlip = Attr & DrvSpriteFlipYMask;
		Colour = Attr & 0x0f;

		DrvTileDraw(DrvSprites, Code, Colour + 0x20, sx - 64, sy - 8, xFlip, yFlip, 1, 0xf);
	}
}

static void DrvRenderCharLayer()
{
	INT32 mx, my, Code, Colour, x, y, yFlip, xFlip, TileIndex = 0;

	UINT16 *VideoRam = (UINT16*)DrvVideoRam;

	for (my = 0; my < 32; my++) {
		for (mx = 0; mx < 64; mx++) {
			yFlip = 0;
			xFlip = 0;
			Code = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]);
			Colour = Code >> 12;
			if (Code & 0x800) yFlip = 1;
			Code &= 0x7ff;

			x = 8 * mx;
			y = 8 * my;

			x -= 64;
			y -= 8;

			Draw8x8MaskTile(pTransDraw, Code, x, y, xFlip, yFlip, Colour, 2, 3, 0x300, DrvChars);

			TileIndex++;
		}
	}
}

static void LastduelRenderFgLayer(INT32 BackLayer)
{
	INT32 mx, my, TileIndex = 0;

	INT32 masks[2][2] = { {0xffff, 0x0001}, {0xf07f, 0x0f81} };

	UINT16 *VideoRam = (UINT16*)DrvScroll1Ram;

	for (mx = 0; mx < 64; mx++) {
		for (my = 0; my < 64; my++) {
			INT32 code = BURN_ENDIAN_SWAP_INT16(VideoRam[2 * TileIndex]) & 0xfff;
			INT32 color = BURN_ENDIAN_SWAP_INT16(VideoRam[2 * TileIndex + 1]);
			INT32 flip = ((color & 0x40) ? 0xf0 : 0) + ((color & 0x20) ? 0x0f : 0);
			INT32 group = (color & 0x80) >> 7;
			color &= 0x0f;

			INT32 sy = 16 * mx;
			INT32 sx = 16 * my;

			sx -= DrvFgScrollX & 0x3ff;
			sy -= DrvFgScrollY & 0x3ff;
			if (sx < -16) sx += 1024;
			if (sy < -16) sy += 1024;

			sx -= 64; // layer offsets
			sy -= 8;

			TileIndex++;

			{
				color = (color << 4) + 0x100;
				UINT8 *src = DrvFgTiles + (code << 8);
				UINT16 *dst;

				for (INT32 y = 0; y < 16; y++, sy++) {
					if (sy < 0 || sy >= nScreenHeight) continue;

					dst = pTransDraw + sy * nScreenWidth;

					for (INT32 x = 0; x < 16; x++, sx++) {
						if (sx < 0 || sx >= nScreenWidth) continue;

						INT32 pxl = src[((y << 4) + x) ^ flip];

						if (masks[group][BackLayer] & (1 << pxl)) continue;

						dst[sx] = pxl + color;
					}

					sx -= 16;
				}
			}
		}
	}
}

static void DrvRenderFgLayer(INT32 BackLayer)
{
	INT32 mx, my, TileIndex;

	INT32 masks[2][2] = { {0xffff, 0x8000}, {0x80ff, 0xff00} };

	UINT16 *VideoRam = (UINT16*)DrvScroll1Ram;

	for (mx = 0; mx < 32; mx++) {
		for (my = 0; my < 64; my++) {
			TileIndex = (my * 32) + mx;
			INT32 code = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex]) & 0x1fff;
			INT32 color = BURN_ENDIAN_SWAP_INT16(VideoRam[TileIndex + 0x800]);
			INT32 flip = ((color & 0x40) ? 0xf0 : 0) + ((color & 0x20) ? 0x0f : 0);
			INT32 group = (color & 0x10) >> 4;
			color &= 0x0f;

			INT32 sy = 16 * mx;
			INT32 sx = 16 * my;

			sx -= DrvFgScrollX;
			sy -= DrvFgScrollY;
			if (sx < -16) sx += 1024;
			if (sy < -16) sy += 512;

			sx -= 64; // layer offsets
			sy -= 8;
			{
				color = (color << 4) + 0x100;
				UINT8 *src = DrvFgTiles + (code << 8);
				UINT16 *dst;

				for (INT32 y = 0; y < 16; y++, sy++) {
					if (sy < 0 || sy >= nScreenHeight) continue;

					dst = pTransDraw + sy * nScreenWidth;

					for (INT32 x = 0; x < 16; x++, sx++) {
						if (sx < 0 || sx >= nScreenWidth) continue;

						INT32 pxl = src[((y << 4) + x) ^ flip];

						if (masks[group][BackLayer] & (1 << pxl)) continue;

						dst[sx] = pxl + color;
					}

					sx -= 16;
				}
			}
		}
	}
}

static INT32 DrvDraw()  // madgear / ledstorm
{
	BurnTransferClear();
	DrvCalcPalette();

	if (DrvTmapPriority) {
		if (nBurnLayer & 2) DrvRenderFgLayer(1);
		if (nSpriteEnable & 1) DrvRenderSprites(0);
		if (nBurnLayer & 4) DrvRenderFgLayer(0);
		if (nBurnLayer & 1) DrvRenderBgLayer(1);
	} else {
		if (nBurnLayer & 1) DrvRenderBgLayer(0);
		if (nBurnLayer & 2) DrvRenderFgLayer(1);
		if (nSpriteEnable & 1) DrvRenderSprites(0);
		if (nBurnLayer & 4) DrvRenderFgLayer(0);
	}

	if (nSpriteEnable & 2) DrvRenderSprites(1);
	if (nBurnLayer & 8) DrvRenderCharLayer();
	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 LastduelDraw()
{
	BurnTransferClear();
	DrvCalcPalette();

	if (nBurnLayer & 1) LastduelRenderBgLayer(0);
	if (nBurnLayer & 2) LastduelRenderFgLayer(1);
	if (nSpriteEnable & 1) DrvRenderSprites(0);
	if (nBurnLayer & 4) LastduelRenderFgLayer(0);
	if (nBurnLayer & 8) DrvRenderCharLayer();

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	INT32 nInterleave = 100;

	if (DrvReset) MadgearDoReset();

	DrvMakeInputs();

	INT32 nCyclesTotal[2] = { 10000000 / 60, 3579545 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	SekNewFrame();
	ZetNewFrame();

	for (INT32 i = 0; i < nInterleave; i++) {
		SekOpen(0);
		CPU_RUN(0, Sek);
		if (i == 33 || i == 66) SekSetIRQLine(6, CPU_IRQSTATUS_AUTO);
		if (i == nInterleave - 1) SekSetIRQLine(5, CPU_IRQSTATUS_AUTO);
		SekClose();

		ZetOpen(0);
		CPU_RUN_TIMER(1);
		ZetClose();
	}

	nExtraCycles = nCyclesDone[0] - nCyclesTotal[0];

	if (pBurnSoundOut) {
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) DrvDraw();

	memcpy(DrvSpriteRamBuffer, DrvSpriteRam, 0x800);

	return 0;
}

static INT32 LastduelFrame()
{
	INT32 nInterleave = 100;

	if (DrvReset) DrvDoReset();

	DrvMakeInputs();

	INT32 nCyclesTotal[2] = { 10000000 / 60, 3579545 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	SekNewFrame();
	ZetNewFrame();

	for (INT32 i = 0; i < nInterleave; i++) {
		SekOpen(0);
		CPU_RUN(0, Sek);
		if (i == 33 || i == 66) SekSetIRQLine(4, CPU_IRQSTATUS_AUTO);
		if (i == nInterleave - 1) SekSetIRQLine(2, CPU_IRQSTATUS_AUTO);
		SekClose();

		ZetOpen(0);
		CPU_RUN_TIMER(1);
		ZetClose();
	}

	nExtraCycles = nCyclesDone[0] - nCyclesTotal[0];

	if (pBurnSoundOut) BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);

	if (pBurnDraw) LastduelDraw();

	memcpy(DrvSpriteRamBuffer, DrvSpriteRam, 0x800);

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029672;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = RamStart;
		ba.nLen	  = RamEnd-RamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		ZetScan(nAction);			// Scan Z80
		BurnYM2203Scan(nAction, pnMin);

		// Scan critical driver variables
		SCAN_VAR(DrvZ80RomBank);
		SCAN_VAR(DrvSoundLatch);
		SCAN_VAR(DrvFgScrollX);
		SCAN_VAR(DrvFgScrollY);
		SCAN_VAR(DrvBgScrollX);
		SCAN_VAR(DrvBgScrollY);
		SCAN_VAR(DrvTmapPriority);

		SCAN_VAR(nExtraCycles);
	}

	return 0;
}

static INT32 MadgearScan(INT32 nAction, INT32 *pnMin)
{
	INT32 i = DrvScan(nAction, pnMin);

	if (i == 0) {
		if (nAction & ACB_DRIVER_DATA) {
			MSM6295Scan(nAction, pnMin);
		}

		if (nAction & ACB_WRITE) {
			ZetOpen(0);
			MadgearZ80BankSwitch(DrvZ80RomBank);
			ZetClose();
		}
	}

	return i;
}

struct BurnDriver BurnDrvLastduel = {
	"lastduel", NULL, NULL, NULL, "1988",
	"Last Duel (US New Ver.)\0", NULL, "Capcom", "Miscellaneous",
	L"\u884C\u661F\u6218\u4E89 2012 (\u7F8E\u7248 \u65B0\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, LastduelRomInfo, LastduelRomName, NULL, NULL, NULL, NULL, LastduelInputInfo, LastduelDIPInfo,
	LastduelInit, DrvExit, LastduelFrame, LastduelDraw, DrvScan,
	NULL, 0x800, 240, 384, 3, 4
};

struct BurnDriver BurnDrvLastduelo = {
	"lastduelo", "lastduel", NULL, NULL, "1988",
	"Last Duel (US Old Ver.)\0", NULL, "Capcom", "Miscellaneous",
	L"\u884C\u661F\u6218\u4E89 2012 (\u7F8E\u7248 \u65E7\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, LastdueloRomInfo, LastdueloRomName, NULL, NULL, NULL, NULL, LastduelInputInfo, LastduelDIPInfo,
	LastduelInit, DrvExit, LastduelFrame, LastduelDraw, DrvScan,
	NULL, 0x800, 240, 384, 3, 4
};

struct BurnDriver BurnDrvLastduelj = {
	"lastduelj", "lastduel", NULL, NULL, "1988",
	"Last Duel (Japan)\0", NULL, "Capcom", "Miscellaneous",
	L"\u884C\u661F\u6218\u4E89 2012 (\u65E5\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, LastdueljRomInfo, LastdueljRomName, NULL, NULL, NULL, NULL, LastduelInputInfo, LastduelDIPInfo,
	LastduelInit, DrvExit, LastduelFrame, LastduelDraw, DrvScan,
	NULL, 0x800, 240, 384, 3, 4
};

struct BurnDriver BurnDrvLsstduelb = {
	"lastduelb", "lastduel", NULL, NULL, "1988",
	"Last Duel (bootleg)\0", NULL, "bootleg", "Miscellaneous",
	L"\u884C\u661F\u6218\u4E89 2012 (\u76D7\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, LastduelbRomInfo, LastduelbRomName, NULL, NULL, NULL, NULL, LastduelInputInfo, LastduelDIPInfo,
	LastduelbInit, DrvExit, LastduelFrame, LastduelDraw, DrvScan,
	NULL, 0x800, 240, 384, 3, 4
};

struct BurnDriver BurnDrvMadgear = {
	"madgear", NULL, NULL, NULL, "1989",
	"Mad Gear (US)\0", NULL, "Capcom", "Miscellaneous",
	L"\u75AF\u72C2\u673A\u8F66 (\u7F8E\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_RACING, 0,
	NULL, DrvRomInfo, DrvRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, MadgearExit, DrvFrame, DrvDraw, MadgearScan,
	NULL, 0x800, 240, 384, 3, 4
};

struct BurnDriver BurnDrvMadgearj = {
	"madgearj", "madgear", NULL, NULL, "1989",
	"Mad Gear (Japan)\0", NULL, "Capcom", "Miscellaneous",
	L"\u75AF\u72C2\u673A\u8F66 (\u65E5\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_RACING, 0,
	NULL, DrvjRomInfo, DrvjRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, MadgearExit, DrvFrame, DrvDraw, MadgearScan,
	NULL, 0x800, 240, 384, 3, 4
};

struct BurnDriver BurnDrvMadgearu = {
	"ledstorm", "madgear", NULL, NULL, "1988",
	"Led Storm (US)\0", NULL, "Capcom", "Miscellaneous",
	L"\u9053\u8DEF\u98CE\u66B4 (\u7F8E\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_RACING, 0,
	NULL, DrvuRomInfo, DrvuRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, MadgearExit, DrvFrame, DrvDraw, MadgearScan,
	NULL, 0x800, 240, 384, 3, 4
};

struct BurnDriver BurnDrvLeds2011 = {
	"leds2011", NULL, NULL, NULL, "1988",
	"Led Storm Rally 2011 (World)\0", NULL, "Capcom", "Miscellaneous",
	L"\u9053\u8DEF\u98CE\u66B4 2011 (\u4E16\u754C\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_RACING, 0,
	NULL, Leds2011RomInfo, Leds2011RomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	Leds2011Init, MadgearExit, DrvFrame, DrvDraw, MadgearScan,
	NULL, 0x800, 240, 384, 3, 4
};

struct BurnDriver BurnDrvLeds2011u = {
	"leds2011u", "leds2011", NULL, NULL, "1988",
	"Led Storm Rally 2011 (US)\0", NULL, "Capcom", "Miscellaneous",
	L"\u9053\u8DEF\u98CE\u66B4 2011 (\u7F8E\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_RACING, 0,
	NULL, Leds2011uRomInfo, Leds2011uRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	Leds2011Init, MadgearExit, DrvFrame, DrvDraw, MadgearScan,
	NULL, 0x800, 240, 384, 3, 4
};
