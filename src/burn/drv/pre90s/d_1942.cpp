// FB Alpha 1942 driver module
// Based on MAME driver by Paul Leaman, Couriersud
#include "tiles_generic.h"
#include "z80_intf.h"
#include "ay8910.h"
#include "bitswap.h"

static UINT8 DrvInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvDip[2]        = {0, 0};
static UINT8 DrvInput[3]      = {0x00, 0x00, 0x00};
static UINT8 DrvReset         = 0;

static UINT8 *Mem                 = NULL;
static UINT8 *MemEnd              = NULL;
static UINT8 *RamStart            = NULL;
static UINT8 *RamEnd              = NULL;
static UINT8 *DrvZ80Rom1          = NULL;
static UINT8 *DrvZ80Rom2          = NULL;
static UINT8 *DrvZ80Ram1          = NULL;
static UINT8 *DrvZ80Ram2          = NULL;
static UINT8 *DrvFgVideoRam       = NULL;
static UINT8 *DrvBgVideoRam       = NULL;
static UINT8 *DrvSpriteRam        = NULL;
static UINT8 *DrvPromRed          = NULL;
static UINT8 *DrvPromGreen        = NULL;
static UINT8 *DrvPromBlue         = NULL;
static UINT8 *DrvPromCharLookup   = NULL;
static UINT8 *DrvPromTileLookup   = NULL;
static UINT8 *DrvPromSpriteLookup = NULL;
static UINT8 *DrvChars            = NULL;
static UINT8 *DrvTiles            = NULL;
static UINT8 *DrvSprites          = NULL;
static UINT8 *DrvTempRom          = NULL;
static UINT32 *DrvPalette          = NULL;
static UINT8 DrvRecalc;

static UINT8 DrvRomBank;
static UINT8 DrvPaletteBank;
static UINT8 DrvBgScroll[2];
static UINT8 DrvFlipScreen;
static UINT8 DrvSoundLatch;

static struct BurnInputInfo DrvInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 7, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort0 + 6, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p2 start"  },

	{"Up"                , BIT_DIGITAL  , DrvInputPort1 + 3, "p1 up"     },
	{"Down"              , BIT_DIGITAL  , DrvInputPort1 + 2, "p1 down"   },
	{"Left"              , BIT_DIGITAL  , DrvInputPort1 + 1, "p1 left"   },
	{"Right"             , BIT_DIGITAL  , DrvInputPort1 + 0, "p1 right"  },
	{"Fire 1"            , BIT_DIGITAL  , DrvInputPort1 + 4, "p1 fire 1" },
	{"Fire 2"            , BIT_DIGITAL  , DrvInputPort1 + 5, "p1 fire 2" },
	
	{"Up (Cocktail)"     , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 up"     },
	{"Down (Cocktail)"   , BIT_DIGITAL  , DrvInputPort2 + 2, "p2 down"   },
	{"Left (Cocktail)"   , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 left"   },
	{"Right (Cocktail)"  , BIT_DIGITAL  , DrvInputPort2 + 0, "p2 right"  },
	{"Fire 1 (Cocktail)" , BIT_DIGITAL  , DrvInputPort2 + 4, "p2 fire 1" },
	{"Fire 2 (Cocktail)" , BIT_DIGITAL  , DrvInputPort2 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Service"           , BIT_DIGITAL  , DrvInputPort0 + 4, "service"   },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};


STDINPUTINFO(Drv)

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
	DrvClearOpposites(&DrvInput[1]);
	DrvClearOpposites(&DrvInput[2]);
}

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x12, 0xff, 0xff, 0xf7, NULL                     },
	{0x13, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x12, 0x01, 0x07, 0x01, "4 Coins 1 Play"         },
	{0x12, 0x01, 0x07, 0x02, "3 Coins 1 Play"         },
	{0x12, 0x01, 0x07, 0x04, "2 Coins 1 Play"         },
	{0x12, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x12, 0x01, 0x07, 0x03, "2 Coins 3 Plays"        },
	{0x12, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x12, 0x01, 0x07, 0x05, "1 Coin  4 Plays"        },
	{0x12, 0x01, 0x07, 0x00, "Freeplay"               },
	
	{0   , 0xfe, 0   , 2   , "Cabinet"                },
	{0x12, 0x01, 0x08, 0x00, "Upright"                },
	{0x12, 0x01, 0x08, 0x08, "Cocktail"               },
	
	{0   , 0xfe, 0   , 4   , "Bonus Life"             },
	{0x12, 0x01, 0x30, 0x30, "20k  80k  80k+"         },
	{0x12, 0x01, 0x30, 0x20, "20k 100k 100k+"         },
	{0x12, 0x01, 0x30, 0x10, "30k  80k  80k+"         },
	{0x12, 0x01, 0x30, 0x00, "30k 100k 100k+"         },	
	
	{0   , 0xfe, 0   , 4   , "Lives"                  },
	{0x12, 0x01, 0xc0, 0x80, "1"                      },
	{0x12, 0x01, 0xc0, 0x40, "2"                      },
	{0x12, 0x01, 0xc0, 0xc0, "3"                      },
	{0x12, 0x01, 0xc0, 0x00, "5"                      },
	
	// Dip 2
	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x13, 0x01, 0x07, 0x01, "4 Coins 1 Play"         },
	{0x13, 0x01, 0x07, 0x02, "3 Coins 1 Play"         },
	{0x13, 0x01, 0x07, 0x04, "2 Coins 1 Play"         },
	{0x13, 0x01, 0x07, 0x07, "1 Coin  1 Play"         },
	{0x13, 0x01, 0x07, 0x03, "2 Coins 3 Plays"        },
	{0x13, 0x01, 0x07, 0x06, "1 Coin  2 Plays"        },
	{0x13, 0x01, 0x07, 0x05, "1 Coin  4 Plays"        },
	{0x13, 0x01, 0x07, 0x00, "Freeplay"               },
	
	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x13, 0x01, 0x08, 0x08, "Off"                    },
	{0x13, 0x01, 0x08, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x13, 0x01, 0x10, 0x10, "Off"                    },
	{0x13, 0x01, 0x10, 0x00, "On"                     },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"             },
	{0x13, 0x01, 0x60, 0x40, "Easy"                   },
	{0x13, 0x01, 0x60, 0x60, "Normal"                 },
	{0x13, 0x01, 0x60, 0x20, "Difficult"              },
	{0x13, 0x01, 0x60, 0x00, "Very Difficult"         },
	
	{0   , 0xfe, 0   , 2   , "Screen Stop"            },
	{0x13, 0x01, 0x80, 0x80, "Off"                    },
	{0x13, 0x01, 0x80, 0x00, "On"                     },
};

STDDIPINFO(Drv)

static struct BurnRomInfo DrvRomDesc[] = {
	{ "srb-03.m3",     0x04000, 0xd9dafcc3, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "srb-04.m4",     0x04000, 0xda0cf924, BRF_ESS | BRF_PRG }, //	 1
	{ "srb-05.m5",     0x04000, 0xd102911c, BRF_ESS | BRF_PRG }, //	 2
	{ "srb-06.m6",     0x02000, 0x466f8248, BRF_ESS | BRF_PRG }, //	 3
	{ "srb-07.m7",     0x04000, 0x0d31038c, BRF_ESS | BRF_PRG }, //	 4
	
	{ "sr-01.c11",     0x04000, 0xbd87f06b, BRF_ESS | BRF_PRG }, //  5	Z80 #2 Program 
	
	{ "sr-02.f2",      0x02000, 0x6ebca191, BRF_GRA },	     //  6	Characters
	
	{ "sr-08.a1",      0x02000, 0x3884d9eb, BRF_GRA },	     //  7	Tiles
	{ "sr-09.a2",      0x02000, 0x999cf6e0, BRF_GRA },	     //  8
	{ "sr-10.a3",      0x02000, 0x8edb273a, BRF_GRA },	     //  9
	{ "sr-11.a4",      0x02000, 0x3a2726c3, BRF_GRA },	     //  10
	{ "sr-12.a5",      0x02000, 0x1bd3d8bb, BRF_GRA },	     //  11
	{ "sr-13.a6",      0x02000, 0x658f02c4, BRF_GRA },	     //  12
	
	{ "sr-14.l1",      0x04000, 0x2528bec6, BRF_GRA },	     //  13	Sprites
	{ "sr-15.l2",      0x04000, 0xf89287aa, BRF_GRA },	     //  14
	{ "sr-16.n1",      0x04000, 0x024418f8, BRF_GRA },	     //  15
	{ "sr-17.n2",      0x04000, 0xe2c7e489, BRF_GRA },	     //  16
	
	{ "sb-5.e8",       0x00100, 0x93ab8153, BRF_GRA },	     //  17	PROMs
	{ "sb-6.e9",       0x00100, 0x8ab44f7d, BRF_GRA },	     //  18
	{ "sb-7.e10",      0x00100, 0xf4ade9a4, BRF_GRA },	     //  19
	{ "sb-0.f1",       0x00100, 0x6047d91b, BRF_GRA },	     //  20
	{ "sb-4.d6",       0x00100, 0x4858968d, BRF_GRA },	     //  21
	{ "sb-8.k3",       0x00100, 0xf6fad943, BRF_GRA },	     //  22
	{ "sb-2.d1",       0x00100, 0x8bb8b3df, BRF_GRA },	     //  23
	{ "sb-3.d2",       0x00100, 0x3b0c99af, BRF_GRA },	     //  24
	{ "sb-1.k6",       0x00100, 0x712ac508, BRF_GRA },	     //  25
	{ "sb-9.m11",      0x00100, 0x4921635c, BRF_GRA },	     //  26
};

STD_ROM_PICK(Drv)
STD_ROM_FN(Drv)

static struct BurnRomInfo DrvaRomDesc[] = {
	{ "sra-03.m3",     0x04000, 0x40201bab, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "sr-04.m4",      0x04000, 0xa60ac644, BRF_ESS | BRF_PRG }, //	 1
	{ "sr-05.m5",      0x04000, 0x835f7b24, BRF_ESS | BRF_PRG }, //	 2
	{ "sr-06.m6",      0x02000, 0x821c6481, BRF_ESS | BRF_PRG }, //	 3
	{ "sr-07.m7",      0x04000, 0x5df525e1, BRF_ESS | BRF_PRG }, //	 4
	
	{ "sr-01.c11",     0x04000, 0xbd87f06b, BRF_ESS | BRF_PRG }, //  5	Z80 #2 Program 
	
	{ "sr-02.f2",      0x02000, 0x6ebca191, BRF_GRA },	     //  6	Characters
	
	{ "sr-08.a1",      0x02000, 0x3884d9eb, BRF_GRA },	     //  7	Tiles
	{ "sr-09.a2",      0x02000, 0x999cf6e0, BRF_GRA },	     //  8
	{ "sr-10.a3",      0x02000, 0x8edb273a, BRF_GRA },	     //  9
	{ "sr-11.a4",      0x02000, 0x3a2726c3, BRF_GRA },	     //  10
	{ "sr-12.a5",      0x02000, 0x1bd3d8bb, BRF_GRA },	     //  11
	{ "sr-13.a6",      0x02000, 0x658f02c4, BRF_GRA },	     //  12
	
	{ "sr-14.l1",      0x04000, 0x2528bec6, BRF_GRA },	     //  13	Sprites
	{ "sr-15.l2",      0x04000, 0xf89287aa, BRF_GRA },	     //  14
	{ "sr-16.n1",      0x04000, 0x024418f8, BRF_GRA },	     //  15
	{ "sr-17.n2",      0x04000, 0xe2c7e489, BRF_GRA },	     //  16
	
	{ "sb-5.e8",       0x00100, 0x93ab8153, BRF_GRA },	     //  17	PROMs
	{ "sb-6.e9",       0x00100, 0x8ab44f7d, BRF_GRA },	     //  18
	{ "sb-7.e10",      0x00100, 0xf4ade9a4, BRF_GRA },	     //  19
	{ "sb-0.f1",       0x00100, 0x6047d91b, BRF_GRA },	     //  20
	{ "sb-4.d6",       0x00100, 0x4858968d, BRF_GRA },	     //  21
	{ "sb-8.k3",       0x00100, 0xf6fad943, BRF_GRA },	     //  22
	{ "sb-2.d1",       0x00100, 0x8bb8b3df, BRF_GRA },	     //  23
	{ "sb-3.d2",       0x00100, 0x3b0c99af, BRF_GRA },	     //  24
	{ "sb-1.k6",       0x00100, 0x712ac508, BRF_GRA },	     //  25
	{ "sb-9.m11",      0x00100, 0x4921635c, BRF_GRA },	     //  26
};

STD_ROM_PICK(Drva)
STD_ROM_FN(Drva)

static struct BurnRomInfo DrvablRomDesc[] = {
	{ "3.bin",         0x08000, 0xf3184f5a, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "5.bin",         0x04000, 0x835f7b24, BRF_ESS | BRF_PRG }, //	 1
	{ "7.bin",         0x08000, 0x2f456c6e, BRF_ESS | BRF_PRG }, //	 2
	
	{ "1.bin",         0x04000, 0xbd87f06b, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 
	
	{ "2.bin",         0x02000, 0x6ebca191, BRF_GRA },	     //  4	Characters
	
	{ "9.bin",         0x04000, 0x60329fa4, BRF_GRA },	     //  5	Tiles
	{ "11.bin",        0x04000, 0x66bac116, BRF_GRA },	     //  6
	{ "13.bin",        0x04000, 0x623fcec1, BRF_GRA },	     //  7
	
	{ "14.bin",        0x08000, 0xdf2345ef, BRF_GRA },	     //  8	Sprites
	{ "16.bin",        0x08000, 0xc106b1ed, BRF_GRA },	     //  9
	
	{ "sb-5.e8",       0x00100, 0x93ab8153, BRF_GRA },	     //  10	PROMs
	{ "sb-6.e9",       0x00100, 0x8ab44f7d, BRF_GRA },	     //  11
	{ "sb-7.e10",      0x00100, 0xf4ade9a4, BRF_GRA },	     //  12
	{ "sb-0.f1",       0x00100, 0x6047d91b, BRF_GRA },	     //  13
	{ "sb-4.d6",       0x00100, 0x4858968d, BRF_GRA },	     //  14
	{ "sb-8.k3",       0x00100, 0xf6fad943, BRF_GRA },	     //  15
	{ "sb-2.d1",       0x00100, 0x8bb8b3df, BRF_GRA },	     //  16
	{ "sb-3.d2",       0x00100, 0x3b0c99af, BRF_GRA },	     //  17
	{ "sb-1.k6",       0x00100, 0x712ac508, BRF_GRA },	     //  18
	{ "sb-9.m11",      0x00100, 0x4921635c, BRF_GRA },	     //  19
};

STD_ROM_PICK(Drvabl)
STD_ROM_FN(Drvabl)

static struct BurnRomInfo DrvbRomDesc[] = {
	{ "sr-03.m3",      0x04000, 0x612975f2, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "sr-04.m4",      0x04000, 0xa60ac644, BRF_ESS | BRF_PRG }, //	 1
	{ "sr-05.m5",      0x04000, 0x835f7b24, BRF_ESS | BRF_PRG }, //	 2
	{ "sr-06.m6",      0x02000, 0x821c6481, BRF_ESS | BRF_PRG }, //	 3
	{ "sr-07.m7",      0x04000, 0x5df525e1, BRF_ESS | BRF_PRG }, //	 4
	
	{ "sr-01.c11",     0x04000, 0xbd87f06b, BRF_ESS | BRF_PRG }, //  5	Z80 #2 Program 
	
	{ "sr-02.f2",      0x02000, 0x6ebca191, BRF_GRA },	     //  6	Characters
	
	{ "sr-08.a1",      0x02000, 0x3884d9eb, BRF_GRA },	     //  7	Tiles
	{ "sr-09.a2",      0x02000, 0x999cf6e0, BRF_GRA },	     //  8
	{ "sr-10.a3",      0x02000, 0x8edb273a, BRF_GRA },	     //  9
	{ "sr-11.a4",      0x02000, 0x3a2726c3, BRF_GRA },	     //  10
	{ "sr-12.a5",      0x02000, 0x1bd3d8bb, BRF_GRA },	     //  11
	{ "sr-13.a6",      0x02000, 0x658f02c4, BRF_GRA },	     //  12
	
	{ "sr-14.l1",      0x04000, 0x2528bec6, BRF_GRA },	     //  13	Sprites
	{ "sr-15.l2",      0x04000, 0xf89287aa, BRF_GRA },	     //  14
	{ "sr-16.n1",      0x04000, 0x024418f8, BRF_GRA },	     //  15
	{ "sr-17.n2",      0x04000, 0xe2c7e489, BRF_GRA },	     //  16
	
	{ "sb-5.e8",       0x00100, 0x93ab8153, BRF_GRA },	     //  17	PROMs
	{ "sb-6.e9",       0x00100, 0x8ab44f7d, BRF_GRA },	     //  18
	{ "sb-7.e10",      0x00100, 0xf4ade9a4, BRF_GRA },	     //  19
	{ "sb-0.f1",       0x00100, 0x6047d91b, BRF_GRA },	     //  20
	{ "sb-4.d6",       0x00100, 0x4858968d, BRF_GRA },	     //  21
	{ "sb-8.k3",       0x00100, 0xf6fad943, BRF_GRA },	     //  22
	{ "sb-2.d1",       0x00100, 0x8bb8b3df, BRF_GRA },	     //  23
	{ "sb-3.d2",       0x00100, 0x3b0c99af, BRF_GRA },	     //  24
	{ "sb-1.k6",       0x00100, 0x712ac508, BRF_GRA },	     //  25
	{ "sb-9.m11",      0x00100, 0x4921635c, BRF_GRA },	     //  26
};

STD_ROM_PICK(Drvb)
STD_ROM_FN(Drvb)

static struct BurnRomInfo DrvwRomDesc[] = {
	{ "sw-03.m3",      0x04000, 0xafd79770, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "sw-04.m4",      0x04000, 0x933d9910, BRF_ESS | BRF_PRG }, //	 1
	{ "sw-05.m5",      0x04000, 0xe9a71bb6, BRF_ESS | BRF_PRG }, //	 2
	{ "sw-06.m6",      0x02000, 0x466f8248, BRF_ESS | BRF_PRG }, //	 3
	{ "sw-07.m7",      0x04000, 0xec41655e, BRF_ESS | BRF_PRG }, //	 4
	
	{ "sr-01.c11",     0x04000, 0xbd87f06b, BRF_ESS | BRF_PRG }, //  5	Z80 #2 Program 
	
	{ "sw-02.f2",      0x02000, 0xf8e9ada2, BRF_GRA },	     //  6	Characters
	
	{ "sr-08.a1",      0x02000, 0x3884d9eb, BRF_GRA },	     //  7	Tiles
	{ "sr-09.a2",      0x02000, 0x999cf6e0, BRF_GRA },	     //  8
	{ "sr-10.a3",      0x02000, 0x8edb273a, BRF_GRA },	     //  9
	{ "sr-11.a4",      0x02000, 0x3a2726c3, BRF_GRA },	     //  10
	{ "sr-12.a5",      0x02000, 0x1bd3d8bb, BRF_GRA },	     //  11
	{ "sr-13.a6",      0x02000, 0x658f02c4, BRF_GRA },	     //  12
	
	{ "sr-14.l1",      0x04000, 0x2528bec6, BRF_GRA },	     //  13	Sprites
	{ "sr-15.l2",      0x04000, 0xf89287aa, BRF_GRA },	     //  14
	{ "sr-16.n1",      0x04000, 0x024418f8, BRF_GRA },	     //  15
	{ "sr-17.n2",      0x04000, 0xe2c7e489, BRF_GRA },	     //  16
	
	{ "sb-5.e8",       0x00100, 0x93ab8153, BRF_GRA },	     //  17	PROMs
	{ "sb-6.e9",       0x00100, 0x8ab44f7d, BRF_GRA },	     //  18
	{ "sb-7.e10",      0x00100, 0xf4ade9a4, BRF_GRA },	     //  19
	{ "sb-0.f1",       0x00100, 0x6047d91b, BRF_GRA },	     //  20
	{ "sb-4.d6",       0x00100, 0x4858968d, BRF_GRA },	     //  21
	{ "sb-8.k3",       0x00100, 0xf6fad943, BRF_GRA },	     //  22
	{ "sb-2.d1",       0x00100, 0x8bb8b3df, BRF_GRA },	     //  23
	{ "sb-3.d2",       0x00100, 0x3b0c99af, BRF_GRA },	     //  24
	{ "sb-1.k6",       0x00100, 0x712ac508, BRF_GRA },	     //  25
	{ "sb-9.m11",      0x00100, 0x4921635c, BRF_GRA },	     //  26
};

STD_ROM_PICK(Drvw)
STD_ROM_FN(Drvw)

static struct BurnRomInfo DrvhRomDesc[] = {
	{ "supercharger_1942_@3.m3", 0x04000, 0xec70785f, BRF_ESS | BRF_PRG }, //  0  Z80 #1 Program Code
	{ "supercharger_1942_@4.m4", 0x04000, 0xcc11355f, BRF_ESS | BRF_PRG }, //  1
	{ "supercharger_1942_@5.m5", 0x04000, 0x42746d75, BRF_ESS | BRF_PRG }, //  2
	{ "srb-06.m6",     			 0x02000, 0x466f8248, BRF_ESS | BRF_PRG }, //  3
	{ "srb-07.m7",     			 0x04000, 0x0d31038c, BRF_ESS | BRF_PRG }, //  4
	
	{ "sr-01.c11",     0x04000, 0xbd87f06b, BRF_ESS | BRF_PRG }, //  5	Z80 #2 Program 
	
	{ "sr-02.f2",      0x02000, 0x6ebca191, BRF_GRA },	     //  6	Characters
	
	{ "sr-08.a1",      0x02000, 0x3884d9eb, BRF_GRA },	     //  7	Tiles
	{ "sr-09.a2",      0x02000, 0x999cf6e0, BRF_GRA },	     //  8
	{ "sr-10.a3",      0x02000, 0x8edb273a, BRF_GRA },	     //  9
	{ "sr-11.a4",      0x02000, 0x3a2726c3, BRF_GRA },	     //  10
	{ "sr-12.a5",      0x02000, 0x1bd3d8bb, BRF_GRA },	     //  11
	{ "sr-13.a6",      0x02000, 0x658f02c4, BRF_GRA },	     //  12
	
	{ "sr-14.l1",      0x04000, 0x2528bec6, BRF_GRA },	     //  13	Sprites
	{ "sr-15.l2",      0x04000, 0xf89287aa, BRF_GRA },	     //  14
	{ "sr-16.n1",      0x04000, 0x024418f8, BRF_GRA },	     //  15
	{ "sr-17.n2",      0x04000, 0xe2c7e489, BRF_GRA },	     //  16
	
	{ "sb-5.e8",       0x00100, 0x93ab8153, BRF_GRA },	     //  17	PROMs
	{ "sb-6.e9",       0x00100, 0x8ab44f7d, BRF_GRA },	     //  18
	{ "sb-7.e10",      0x00100, 0xf4ade9a4, BRF_GRA },	     //  19
	{ "sb-0.f1",       0x00100, 0x6047d91b, BRF_GRA },	     //  20
	{ "sb-4.d6",       0x00100, 0x4858968d, BRF_GRA },	     //  21
	{ "sb-8.k3",       0x00100, 0xf6fad943, BRF_GRA },	     //  22
	{ "sb-2.d1",       0x00100, 0x8bb8b3df, BRF_GRA },	     //  23
	{ "sb-3.d2",       0x00100, 0x3b0c99af, BRF_GRA },	     //  24
	{ "sb-1.k6",       0x00100, 0x712ac508, BRF_GRA },	     //  25
	{ "sb-9.m11",      0x00100, 0x4921635c, BRF_GRA },	     //  26
};

STD_ROM_PICK(Drvh)
STD_ROM_FN(Drvh)

// 1942 (C64 Music)

static struct BurnRomInfo Drvc64RomDesc[] = {
	{ "srb-03.m3",	0x4000, 0xd9dafcc3, 1 | BRF_PRG | BRF_ESS }, //  0 maincpu
	{ "srb-04.m4",	0x4000, 0xda0cf924, 1 | BRF_PRG | BRF_ESS }, //  1
	{ "srb-05.m5",	0x4000, 0xd102911c, 1 | BRF_PRG | BRF_ESS }, //  2
	{ "srb-06.m6",	0x2000, 0x466f8248, 1 | BRF_PRG | BRF_ESS }, //  3
	{ "srb-07.m7",	0x4000, 0x0d31038c, 1 | BRF_PRG | BRF_ESS }, //  4

	{ "1942c64.c11",0x4000, 0x4ee7ab6b, 2 | BRF_PRG | BRF_ESS }, //  5 audiocpu

	{ "sr-02.f2",	0x2000, 0x6ebca191, 3 | BRF_GRA },           //  6 gfx1

	{ "sr-08.a1",	0x2000, 0x3884d9eb, 4 | BRF_GRA },           //  7 gfx2
	{ "sr-09.a2",	0x2000, 0x999cf6e0, 4 | BRF_GRA },           //  8
	{ "sr-10.a3",	0x2000, 0x8edb273a, 4 | BRF_GRA },           //  9
	{ "sr-11.a4",	0x2000, 0x3a2726c3, 4 | BRF_GRA },           // 10
	{ "sr-12.a5",	0x2000, 0x1bd3d8bb, 4 | BRF_GRA },           // 11
	{ "sr-13.a6",	0x2000, 0x658f02c4, 4 | BRF_GRA },           // 12

	{ "sr-14.l1",	0x4000, 0x2528bec6, 5 | BRF_GRA },           // 13 gfx3
	{ "sr-15.l2",	0x4000, 0xf89287aa, 5 | BRF_GRA },           // 14
	{ "sr-16.n1",	0x4000, 0x024418f8, 5 | BRF_GRA },           // 15
	{ "sr-17.n2",	0x4000, 0xe2c7e489, 5 | BRF_GRA },           // 16

	{ "sb-5.e8",	0x0100, 0x93ab8153, 6 | BRF_GRA },           // 17 proms
	{ "sb-6.e9",	0x0100, 0x8ab44f7d, 6 | BRF_GRA },           // 18
	{ "sb-7.e10",	0x0100, 0xf4ade9a4, 6 | BRF_GRA },           // 19
	{ "sb-0.f1",	0x0100, 0x6047d91b, 6 | BRF_GRA },           // 20
	{ "sb-4.d6",	0x0100, 0x4858968d, 6 | BRF_GRA },           // 21
	{ "sb-8.k3",	0x0100, 0xf6fad943, 6 | BRF_GRA },           // 22
	{ "sb-2.d1",	0x0100, 0x8bb8b3df, 6 | BRF_GRA },           // 23
	{ "sb-3.d2",	0x0100, 0x3b0c99af, 6 | BRF_GRA },           // 24
	{ "sb-1.k6",	0x0100, 0x712ac508, 6 | BRF_GRA },           // 25
	{ "sb-9.m11",	0x0100, 0x4921635c, 6 | BRF_GRA },           // 26
};

STD_ROM_PICK(Drvc64)
STD_ROM_FN(Drvc64)

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvZ80Rom1             = Next; Next += 0x1c000;
	DrvZ80Rom2             = Next; Next += 0x04000;
	DrvPromRed             = Next; Next += 0x00100;
	DrvPromGreen           = Next; Next += 0x00100;
	DrvPromBlue            = Next; Next += 0x00100;
	DrvPromCharLookup      = Next; Next += 0x00100;
	DrvPromTileLookup      = Next; Next += 0x00100;
	DrvPromSpriteLookup    = Next; Next += 0x00100;

	RamStart               = Next;

	DrvZ80Ram1             = Next; Next += 0x01000;
	DrvZ80Ram2             = Next; Next += 0x00800;
	DrvSpriteRam           = Next; Next += 0x00080;
	DrvFgVideoRam          = Next; Next += 0x00800;
	DrvBgVideoRam          = Next; Next += 0x00400;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 0x200 * 8 * 8;
	DrvTiles               = Next; Next += 0x200 * 16 * 16;
	DrvSprites             = Next; Next += 0x200 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x00600 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 DrvDoReset()
{
	ZetReset(0);
	ZetReset(1);

	AY8910Reset(0);
	AY8910Reset(1);

	DrvPaletteBank = 0;
	DrvBgScroll[0] = 0;
	DrvBgScroll[1] = 0;
	DrvFlipScreen = 0;
	DrvSoundLatch = 0;
	DrvRomBank = 0;

	HiscoreReset();

	return 0;
}

static UINT8 __fastcall Drv1942Read1(UINT16 a)
{
	switch (a) {
		case 0xc000: {
			return 0xff - DrvInput[0];
		}
		
		case 0xc001: {
			return 0xff - DrvInput[1];
		}
		
		case 0xc002: {
			return 0xff - DrvInput[2];
		}
		
		case 0xc003: {
			return DrvDip[0];
		}
		
		case 0xc004: {
			return DrvDip[1];
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Drv1942Write1(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xc800: {
			DrvSoundLatch = d;
			return;
		}
		
		case 0xc802: {
			DrvBgScroll[0] = d;
			return;
		}
		
		case 0xc803: {
			DrvBgScroll[1] = d;
			return;
		}
		
		case 0xc804: {
			DrvFlipScreen = d & 0x80;
			if (d & 0x10) {
				ZetReset(1);
			}
			return;
		}
		
		case 0xc805: {
			DrvPaletteBank = d;
			return;
		}
		
		case 0xc806: {
			d &= 0x03;
			DrvRomBank = d;
			ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000 + DrvRomBank * 0x4000 );
			ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000 + DrvRomBank * 0x4000 );
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Drv1942Read2(UINT16 a)
{
	switch (a) {
		case 0x6000: {
			return DrvSoundLatch;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Read => %04X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Drv1942Write2(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0x8000: {
			AY8910Write(0, 0, d);
			return;
		}
		
		case 0x8001: {
			AY8910Write(0, 1, d);
			return;
		}
		
		case 0xc000: {
			AY8910Write(1, 0, d);
			return;
		}
		
		case 0xc001: {
			AY8910Write(1, 1, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Write => %04X, %02X\n"), a, d);
		}
	}
}

static tilemap_callback( bg )
{
	offs = (offs & 0x0f) | ((offs & 0x01f0) << 1);

	INT32 color = DrvBgVideoRam[offs + 0x10];
	INT32 code  = DrvBgVideoRam[offs] + ((color & 0x80) << 1);

	TILE_SET_INFO(0, code, (color & 0x1f) + (0x20 * DrvPaletteBank), TILE_FLIPYX(color >> 5));
}

static tilemap_callback( fg )
{
	INT32 color = DrvFgVideoRam[offs + 0x400];
	INT32 code  = DrvFgVideoRam[offs] + ((color & 0x80) << 1);

	TILE_SET_INFO(1, code, color, 0);
}

static INT32 CharPlaneOffsets[2]   = { 4, 0 };
static INT32 CharXOffsets[8]       = { 0, 1, 2, 3, 8, 9, 10, 11 };
static INT32 CharYOffsets[8]       = { 0, 16, 32, 48, 64, 80, 96, 112 };
static INT32 TilePlaneOffsets[3]   = { 0, 0x20000, 0x40000 };
static INT32 TileXOffsets[16]      = { 0, 1, 2, 3, 4, 5, 6, 7, 128, 129, 130, 131, 132, 133, 134, 135 };
static INT32 TileYOffsets[16]      = { 0, 8, 16, 24, 32, 40, 48, 56, 64, 72, 80, 88, 96, 104, 112, 120 };
static INT32 SpritePlaneOffsets[4] = { 0x40004, 0x40000, 4, 0 };
static INT32 SpriteXOffsets[16]    = { 0, 1, 2, 3, 8, 9, 10, 11, 256, 257, 258, 259, 264, 265, 266, 267 };
static INT32 SpriteYOffsets[16]    = { 0, 16, 32, 48, 64, 80, 96, 112, 128, 144, 160, 176, 192, 208, 224, 240 };

static void MachineInit()
{
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(Drv1942Read1);
	ZetSetWriteHandler(Drv1942Write1);
	ZetMapArea(0x0000, 0x7fff, 0, DrvZ80Rom1             );
	ZetMapArea(0x0000, 0x7fff, 2, DrvZ80Rom1             );
	ZetMapArea(0x8000, 0xbfff, 0, DrvZ80Rom1 + 0x10000   );
	ZetMapArea(0x8000, 0xbfff, 2, DrvZ80Rom1 + 0x10000   );
	ZetMapArea(0xcc00, 0xcc7f, 0, DrvSpriteRam           );
	ZetMapArea(0xcc00, 0xcc7f, 1, DrvSpriteRam           );
	ZetMapArea(0xcc00, 0xcc7f, 2, DrvSpriteRam           );
	ZetMapArea(0xd000, 0xd7ff, 0, DrvFgVideoRam          );
	ZetMapArea(0xd000, 0xd7ff, 1, DrvFgVideoRam          );
	ZetMapArea(0xd000, 0xd7ff, 2, DrvFgVideoRam          );
	ZetMapArea(0xd800, 0xdbff, 0, DrvBgVideoRam          );
	ZetMapArea(0xd800, 0xdbff, 1, DrvBgVideoRam          );
	ZetMapArea(0xd800, 0xdbff, 2, DrvBgVideoRam          );
	ZetMapArea(0xe000, 0xefff, 0, DrvZ80Ram1             );
	ZetMapArea(0xe000, 0xefff, 1, DrvZ80Ram1             );
	ZetMapArea(0xe000, 0xefff, 2, DrvZ80Ram1             );
	ZetClose();
	
	ZetInit(1);
	ZetOpen(1);
	ZetSetReadHandler(Drv1942Read2);
	ZetSetWriteHandler(Drv1942Write2);
	ZetMapArea(0x0000, 0x3fff, 0, DrvZ80Rom2             );
	ZetMapArea(0x0000, 0x3fff, 2, DrvZ80Rom2             );
	ZetMapArea(0x4000, 0x47ff, 0, DrvZ80Ram2             );
	ZetMapArea(0x4000, 0x47ff, 1, DrvZ80Ram2             );
	ZetMapArea(0x4000, 0x47ff, 2, DrvZ80Ram2             );
	ZetClose();
	
	AY8910Init(0, 1500000, 0);
	AY8910Init(1, 1500000, 1);
	AY8910SetAllRoutes(0, 0.25, BURN_SND_ROUTE_BOTH); // Plane Noise/Bass/Shot
	AY8910SetAllRoutes(1, 0.25, BURN_SND_ROUTE_BOTH); // Whistle/Snare
	AY8910SetBuffered(ZetTotalCycles, 3000000);

	GenericTilesInit();
	GenericTilemapInit(0, TILEMAP_SCAN_COLS, bg_map_callback, 16, 16, 32, 16);
	GenericTilemapInit(1, TILEMAP_SCAN_ROWS, fg_map_callback,  8,  8, 32, 32);
	GenericTilemapSetGfx(0, DrvTiles, 3, 16, 16, 0x20000, 0x100, 0x7f);
	GenericTilemapSetGfx(1, DrvChars, 2,  8,  8, 0x08000, 0x000, 0x3f);
	GenericTilemapSetTransparent(1, 0);
	GenericTilemapSetOffsets(TMAP_GLOBAL, 0, -16);
	
	// Reset the driver
	DrvDoReset();
}

static INT32 DrvInit()
{
	INT32 nRet = 0, nLen;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x10000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x04000, 1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000, 2, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x14000, 3, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x18000, 4, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000, 5, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 6, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the tiles
	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x02000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x04000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x06000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0a000, 12, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 3, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, DrvTempRom, DrvTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x04000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x0c000, 16, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);

	// Load the PROMs
	nRet = BurnLoadRom(DrvPromRed,          17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromGreen,        18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBlue,         19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromCharLookup,   20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromTileLookup,   21, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromSpriteLookup, 22, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	MachineInit();
	
	return 0;
}

static INT32 DrvablInit()
{
	INT32 nRet = 0, nLen;
	
	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	DrvTempRom = (UINT8 *)BurnMalloc(0x18000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000, 1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x14000, 2, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000, 3, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 4, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the tiles
	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x04000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000,  7, 1); if (nRet != 0) return 1;
	GfxDecode(0x200, 3, 16, 16, TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x100, DrvTempRom, DrvTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x10000);
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  8, 1); if (nRet != 0) return 1;
	memcpy(DrvTempRom + 0x04000, DrvTempRom + 0x10000, 0x4000);
	memcpy(DrvTempRom + 0x00000, DrvTempRom + 0x14000, 0x4000);
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  9, 1); if (nRet != 0) return 1;
	memcpy(DrvTempRom + 0x0c000, DrvTempRom + 0x10000, 0x4000);
	memcpy(DrvTempRom + 0x08000, DrvTempRom + 0x14000, 0x4000);
	GfxDecode(0x200, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load the PROMs
	nRet = BurnLoadRom(DrvPromRed,          10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromGreen,        11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBlue,         12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromCharLookup,   13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromTileLookup,   14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromSpriteLookup, 15, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	MachineInit();
	
	return 0;
}

static INT32 DrvExit()
{
	ZetExit();
	
	AY8910Exit(0);
	AY8910Exit(1);

	GenericTilesExit();
	
	DrvPaletteBank = 0;
	DrvBgScroll[0] = 0;
	DrvBgScroll[1] = 0;
	DrvFlipScreen = 0;
	DrvSoundLatch = 0;
	DrvRomBank = 0;

	BurnFree(Mem);

	return 0;
}

static void DrvCalcPalette()
{
	INT32 i;
	UINT32 Palette[256];
	
	for (i = 0; i < 256; i++) {
		INT32 bit0, bit1, bit2, bit3, r, g, b;
		
		bit0 = BIT(DrvPromRed[i], 0);
		bit1 = BIT(DrvPromRed[i], 1);
		bit2 = BIT(DrvPromRed[i], 2);
		bit3 = BIT(DrvPromRed[i], 3);
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		
		bit0 = BIT(DrvPromGreen[i], 0);
		bit1 = BIT(DrvPromGreen[i], 1);
		bit2 = BIT(DrvPromGreen[i], 2);
		bit3 = BIT(DrvPromGreen[i], 3);
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		
		bit0 = BIT(DrvPromBlue[i], 0);
		bit1 = BIT(DrvPromBlue[i], 1);
		bit2 = BIT(DrvPromBlue[i], 2);
		bit3 = BIT(DrvPromBlue[i], 3);
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		
		Palette[i] = BurnHighCol(r, g, b, 0);
	}
	
	for (i = 0; i < 256; i++) {
		DrvPalette[i] = Palette[0x80 | DrvPromCharLookup[i]];
	}
	
	for (i = 0; i < 256; i++) {
		DrvPalette[256 + i +   0] = Palette[0x00 | DrvPromTileLookup[i]];
		DrvPalette[256 + i + 256] = Palette[0x10 | DrvPromTileLookup[i]];
		DrvPalette[256 + i + 512] = Palette[0x20 | DrvPromTileLookup[i]];
		DrvPalette[256 + i + 768] = Palette[0x30 | DrvPromTileLookup[i]];
	}
	
	for (i = 0; i < 256; i++) {
		DrvPalette[1280 + i] = Palette[0x40 | DrvPromSpriteLookup[i]];
	}
}

static void DrvRenderSpriteLayer()
{
	for (int y = 16; y <= nScreenHeight + 16; y++) {
		// the y-16 and y-15 values are guessed and might be wrong, MAME is using y and y, it won't work here though -barbudreadmon
		GenericTilesSetClip(-1, -1, y-16, y-15);
		UINT8 objdata[4];
		UINT8 v = y - 1;
		for (int h = 496; h >= 128; h -= 16)
		{
			const bool objcnt4 = BIT(h, 8) != BIT(~h, 7);
			const bool objcnt3 = (BIT(v, 7) && objcnt4) != BIT(~h, 7);
			UINT8 obj_idx = (h >> 4) & 7;
			obj_idx |= objcnt3 ? 0x08 : 0x00;
			obj_idx |= objcnt4 ? 0x10 : 0x00;
			obj_idx <<= 2;
			for (INT32 i = 0; i < 4; i++)
				objdata[i] = DrvSpriteRam[obj_idx | i];

			INT32 code = (objdata[0] & 0x7f) + ((objdata[1] & 0x20) << 2) + ((objdata[0] & 0x80) << 1);
			INT32 col = objdata[1] & 0x0f;
			INT32 sx = objdata[3] - 0x10 * (objdata[1] & 0x10);
			INT32 sy = objdata[2];
			INT32 dir = 1;

			UINT8 valpha = (UINT8)sy;
			UINT8 v2c = (UINT8)(~v) + 0xff;
			UINT8 lvbeta = v2c + valpha;
			UINT8 vbeta = ~lvbeta;
			bool vleq = (vbeta <= ((~valpha) & 0xff));
			bool vinlen = 1;
			UINT8 vlen = objdata[1] >> 6;
			switch (vlen & 3)
			{
			case 0:
				vinlen = BIT(lvbeta, 7) && BIT(lvbeta, 6) && BIT(lvbeta, 5) && BIT(lvbeta, 4);
				break;
			case 1:
				vinlen = BIT(lvbeta, 7) && BIT(lvbeta, 6) && BIT(lvbeta, 5);
				break;
			case 2:
				vinlen = BIT(lvbeta, 7) && BIT(lvbeta, 6);
				break;
			case 3:
				vinlen = true;
				break;
			}
			bool vinzone = !(vleq && vinlen);

			/* handle double / quadruple height */
			INT32 i = (objdata[1] & 0xc0) >> 6;
			if (i == 2) i = 3;

			if (!vinzone)
			{
				do {
					Render16x16Tile_Mask_Clip(pTransDraw, code + i, sx, (sy + (16 * i) * dir) - 16, col, 4, 15, 1280, DrvSprites);
				} while (i-- > 0);
			}
		}
		GenericTilesClearClip();
	}
}

static INT32 DrvDraw()
{
	BurnTransferClear();

	if (DrvRecalc) {
		DrvCalcPalette();
		DrvRecalc = 0;
	}
	
	GenericTilemapSetScrollX(0, DrvBgScroll[0] | (DrvBgScroll[1] << 8));

	if (nBurnLayer & 1) GenericTilemapDraw(0, pTransDraw, 0);

	if (nSpriteEnable & 1) DrvRenderSpriteLayer();

	if (nBurnLayer & 2) GenericTilemapDraw(1, pTransDraw, 0);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static INT32 DrvFrame()
{
	if (DrvReset) DrvDoReset();

	DrvMakeInputs();

	ZetNewFrame();

	INT32 nInterleave = 8;
	INT32 nCyclesTotal[2] = { 4000000 / 60, 3000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };

	for (INT32 i = 0; i < nInterleave; i++) {
		ZetOpen(0);
		CPU_RUN(0, Zet);
		if (i == 0 || i == 7) {
			ZetSetVector((i == 0) ? 0xcf : 0xd7);
			ZetSetIRQLine(0, CPU_IRQSTATUS_HOLD);
		}
		ZetClose();

		ZetOpen(1);
		CPU_RUN(1, Zet);
		if (i & 1) { // 4 times per frame
			ZetSetIRQLine(0, CPU_IRQSTATUS_HOLD);
		}
		ZetClose();
	}

	if (pBurnSoundOut) {
		AY8910Render(pBurnSoundOut, nBurnSoundLen);
	}

	if (pBurnDraw) {
		BurnDrvRedraw();
	}

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
		ZetScan(nAction);			// Scan Z80
		AY8910Scan(nAction, pnMin);

		// Scan critical driver variables
		SCAN_VAR(DrvRomBank);
		SCAN_VAR(DrvPaletteBank);
		SCAN_VAR(DrvSoundLatch);
		SCAN_VAR(DrvBgScroll);
		SCAN_VAR(DrvFlipScreen);
	}
	
	if (nAction & ACB_WRITE) {
		ZetOpen(0);
		ZetMapMemory(DrvZ80Rom1 + 0x10000 + DrvRomBank * 0x4000, 0x8000, 0xbfff, MAP_ROM);
		ZetClose();
	}

	return 0;
}

struct BurnDriver BurnDrvNineteen42 = {
	"1942", NULL, NULL, NULL, "1984",
	"1942 (Revision B)\0", NULL, "Capcom", "Miscellaneous",
	L"1942 (\u4FEE\u8BA2\u7248 B)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvRomInfo, DrvRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x600, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen42a = {
	"1942a", "1942", NULL, NULL, "1984",
	"1942 (Revision A)\0", NULL, "Capcom", "Miscellaneous",
	L"1942 (\u4FEE\u8BA2\u7248 A)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvaRomInfo, DrvaRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x600, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen42abl = {
	"1942abl", "1942", NULL, NULL, "1984",
	"1942 (Revision A, bootleg)\0", NULL, "bootleg", "Miscellaneous",
	L"1942 (\u4FEE\u8BA2\u7248 A, \u76D7\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvablRomInfo, DrvablRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvablInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x600, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen42b = {
	"1942b", "1942", NULL, NULL, "1984",
	"1942 (First Version)\0", NULL, "Capcom", "Miscellaneous",
	L"1942 - \u7A7A\u4E2D\u5927\u6218 (\u521D\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvbRomInfo, DrvbRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x600, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen42w = {
	"1942w", "1942", NULL, NULL, "1985",
	"1942 (Williams Electronics license)\0", NULL, "Capcom (Williams Electronics license)", "Miscellaneous",
	L"1942 (Williams Electronics \u6388\u6743)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvwRomInfo, DrvwRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x600, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen42h = {
	"1942h", "1942", NULL, NULL, "1991",
	"Supercharger 1942\0", NULL, "hack (Two Bit Score)", "Miscellaneous",
	L"1942 - \u7A7A\u4E2D\u5927\u6218 (\u76D7\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HACK | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvhRomInfo, DrvhRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x600, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen42c64 = {
	"1942c64", "1942", NULL, NULL, "2015",
	"1942 (C64 Music)\0", NULL, "hack by Minwah", "Miscellaneous",
	L"1942 - \u7A7A\u4E2D\u5927\u6218 (C64 \u97F3\u6E90)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, Drvc64RomInfo, Drvc64RomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x600, 224, 256, 3, 4
};
