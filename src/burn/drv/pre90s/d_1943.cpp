// FB Alpha 1943 driver module
// Based on MAME driver by Paul Leaman

#include "tiles_generic.h"
#include "z80_intf.h"
#include "burn_ym2203.h"
#include "watchdog.h"

static UINT8 DrvInputPort0[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInputPort2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvDip[2]        = {0, 0};
static UINT8 DrvInput[3]      = {0x00, 0x00, 0x00};
static UINT8 DrvReset         = 0;

static UINT8 *Mem                  = NULL;
static UINT8 *MemEnd               = NULL;
static UINT8 *RamStart             = NULL;
static UINT8 *RamEnd               = NULL;
static UINT8 *DrvZ80Rom1           = NULL;
static UINT8 *DrvZ80Rom2           = NULL;
static UINT8 *DrvZ80Ram1           = NULL;
static UINT8 *DrvZ80Ram2           = NULL;
static UINT8 *DrvVideoRam          = NULL;
static UINT8 *DrvPaletteRam        = NULL;
static UINT8 *DrvSpriteRam         = NULL;
static UINT8 *DrvPromRed           = NULL;
static UINT8 *DrvPromGreen         = NULL;
static UINT8 *DrvPromBlue          = NULL;
static UINT8 *DrvPromCharLookup    = NULL;
static UINT8 *DrvPromBg2Lookup     = NULL;
static UINT8 *DrvPromBg2PalBank    = NULL;
static UINT8 *DrvPromBgLookup      = NULL;
static UINT8 *DrvPromBgPalBank     = NULL;
static UINT8 *DrvPromSpriteLookup  = NULL;
static UINT8 *DrvPromSpritePalBank = NULL;
static UINT8 *DrvBgTilemap         = NULL;
static UINT8 *DrvBg2Tilemap        = NULL;
static UINT8 *DrvChars             = NULL;
static UINT8 *DrvBg2Tiles          = NULL;
static UINT8 *DrvBgTiles           = NULL;
static UINT8 *DrvSprites           = NULL;
static UINT8 *DrvTempRom           = NULL;
static UINT32 *DrvPalette          = NULL;

static UINT8 DrvRecalc;

static UINT8 DrvSoundLatch;
static UINT8 DrvRomBank;
static UINT8 DrvBg2ScrollX[2];
static UINT8 DrvBgScrollX[2];
static UINT8 DrvBgScrollY;
static UINT8 DrvBg2On;
static UINT8 DrvBg1On;
static UINT8 DrvSpritesOn;
static UINT8 DrvCharsOn;
static UINT8 DrvProtValue;

static INT32 bootleg = 0;

static struct BurnInputInfo DrvInputList[] =
{
	{"Coin 1"            , BIT_DIGITAL  , DrvInputPort0 + 6, "p1 coin"   },
	{"Start 1"           , BIT_DIGITAL  , DrvInputPort0 + 0, "p1 start"  },
	{"Coin 2"            , BIT_DIGITAL  , DrvInputPort0 + 7, "p2 coin"   },
	{"Start 2"           , BIT_DIGITAL  , DrvInputPort0 + 1, "p2 start"  },

	{"P1 Up"             , BIT_DIGITAL  , DrvInputPort1 + 3, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL  , DrvInputPort1 + 2, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL  , DrvInputPort1 + 1, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL  , DrvInputPort1 + 0, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL  , DrvInputPort1 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL  , DrvInputPort1 + 5, "p1 fire 2" },
	
	{"P2 Up"             , BIT_DIGITAL  , DrvInputPort2 + 3, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL  , DrvInputPort2 + 2, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL  , DrvInputPort2 + 1, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL  , DrvInputPort2 + 0, "p2 right"  },
	{"Fire 1"            , BIT_DIGITAL  , DrvInputPort2 + 4, "p2 fire 1" },
	{"Fire 2"            , BIT_DIGITAL  , DrvInputPort2 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL  , &DrvReset        , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH, DrvDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH, DrvDip + 1       , "dip"       },
};

STDINPUTINFO(Drv)

static struct BurnDIPInfo DrvDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xf8, NULL                     },
	{0x12, 0xff, 0xff, 0xff, NULL                     },

	// Dip 1
	{0   , 0xfe, 0   , 16  , "Difficulty"             },
	{0x11, 0x01, 0x0f, 0x0f, "1 (Easy)"               },
	{0x11, 0x01, 0x0f, 0x0e, "2"                      },
	{0x11, 0x01, 0x0f, 0x0d, "3"                      },
	{0x11, 0x01, 0x0f, 0x0c, "4"                      },
	{0x11, 0x01, 0x0f, 0x0b, "5"                      },
	{0x11, 0x01, 0x0f, 0x0a, "6"                      },
	{0x11, 0x01, 0x0f, 0x09, "7"                      },
	{0x11, 0x01, 0x0f, 0x08, "8 (Normal)"             },
	{0x11, 0x01, 0x0f, 0x07, "9"                      },
	{0x11, 0x01, 0x0f, 0x06, "10"                     },
	{0x11, 0x01, 0x0f, 0x05, "11"                     },
	{0x11, 0x01, 0x0f, 0x04, "12"                     },
	{0x11, 0x01, 0x0f, 0x03, "13"                     },
	{0x11, 0x01, 0x0f, 0x02, "14"                     },
	{0x11, 0x01, 0x0f, 0x01, "15"                     },
	{0x11, 0x01, 0x0f, 0x00, "16 (Difficult)"         },

	{0   , 0xfe, 0   , 2   , "2 Player Game"          },
	{0x11, 0x01, 0x10, 0x00, "1 Credit / 2 Players"   },
	{0x11, 0x01, 0x10, 0x10, "2 Credits / 2 Players"  },

	{0   , 0xfe, 0   , 2   , "Flip Screen"            },
	{0x11, 0x01, 0x20, 0x20, "Off"                    },
	{0x11, 0x01, 0x20, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Freeze"                 },
	{0x11, 0x01, 0x40, 0x40, "Off"                    },
	{0x11, 0x01, 0x40, 0x00, "On"                     },

	{0   , 0xfe, 0   , 2   , "Service Mode"           },
	{0x11, 0x01, 0x80, 0x80, "Off"                    },
	{0x11, 0x01, 0x80, 0x00, "On"                     },

	// Dip 2
	{0   , 0xfe, 0   , 8   , "Coin A"                 },
	{0x12, 0x01, 0x07, 0x00, "4 Coins 1 Play"         },
	{0x12, 0x01, 0x07, 0x01, "3 Coins 1 Play"         },
	{0x12, 0x01, 0x07, 0x02, "2 Coins 1 Play"         },
	{0x12, 0x01, 0x07, 0x07, "1 Coin 1 Play"          },
	{0x12, 0x01, 0x07, 0x06, "1 Coin 2 Plays"         },
	{0x12, 0x01, 0x07, 0x05, "1 Coin 3 Plays"         },
	{0x12, 0x01, 0x07, 0x04, "1 Coin 4 Plays"         },
	{0x12, 0x01, 0x07, 0x03, "1 Coin 5 Plays"         },

	{0   , 0xfe, 0   , 8   , "Coin B"                 },
	{0x12, 0x01, 0x38, 0x00, "4 Coins 1 Play"         },
	{0x12, 0x01, 0x38, 0x08, "3 Coins 1 Play"         },
	{0x12, 0x01, 0x38, 0x10, "2 Coins 1 Play"         },
	{0x12, 0x01, 0x38, 0x38, "1 Coin 1 Play"          },
	{0x12, 0x01, 0x38, 0x30, "1 Coin 2 Plays"         },
	{0x12, 0x01, 0x38, 0x28, "1 Coin 3 Plays"         },
	{0x12, 0x01, 0x38, 0x20, "1 Coin 4 Plays"         },
	{0x12, 0x01, 0x38, 0x18, "1 Coin 5 Plays"         },

	{0   , 0xfe, 0   , 2   , "Allow Continue"         },
	{0x12, 0x01, 0x40, 0x00, "No"                     },
	{0x12, 0x01, 0x40, 0x40, "Yes"                    },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"            },
	{0x12, 0x01, 0x80, 0x00, "Off"                    },
	{0x12, 0x01, 0x80, 0x80, "On"                     },
};

STDDIPINFO(Drv)

static struct BurnRomInfo DrvRomDesc[] = {
	{ "bme01.12d",     0x08000, 0x55fd447e, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "bme02.13d",     0x10000, 0x073fc57c, BRF_ESS | BRF_PRG }, //	 1
	{ "bme03.14d",     0x10000, 0x835822c2, BRF_ESS | BRF_PRG }, //	 2
	
	{ "bm05.4k",       0x08000, 0xee2bd2d7, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 

	{ "bm04.5h",       0x08000, 0x46cb9d3d, BRF_GRA },	     	 //  4	Characters
	
	{ "bm15.10f",      0x08000, 0x6b1a0443, BRF_GRA },	     	 //  5	BG Tiles
	{ "bm16.11f",      0x08000, 0x23c908c2, BRF_GRA },	     	 //  6
	{ "bm17.12f",      0x08000, 0x46bcdd07, BRF_GRA },	     	 //  7
	{ "bm18.14f",      0x08000, 0xe6ae7ba0, BRF_GRA },	     	 //  8
	{ "bm19.10j",      0x08000, 0x868ababc, BRF_GRA },	     	 //  9
	{ "bm20.11j",      0x08000, 0x0917e5d4, BRF_GRA },	     	 //  10
	{ "bm21.12j",      0x08000, 0x9bfb0d89, BRF_GRA },	     	 //  11
	{ "bm22.14j",      0x08000, 0x04f3c274, BRF_GRA },	     	 //  12
	
	{ "bm24.14k",      0x08000, 0x11134036, BRF_GRA },	     	 //  13	BG2 Tiles
	{ "bm25.14l",      0x08000, 0x092cf9c1, BRF_GRA },	     	 //  14
	
	{ "bm06.10a",      0x08000, 0x97acc8af, BRF_GRA },	     	 //  15	Sprites
	{ "bm07.11a",      0x08000, 0xd78f7197, BRF_GRA },	     	 //  16
	{ "bm08.12a",      0x08000, 0x1a626608, BRF_GRA },	     	 //  17
	{ "bm09.14a",      0x08000, 0x92408400, BRF_GRA },	     	 //  18
	{ "bm10.10c",      0x08000, 0x8438a44a, BRF_GRA },	     	 //  19
	{ "bm11.11c",      0x08000, 0x6c69351d, BRF_GRA },	     	 //  20
	{ "bm12.12c",      0x08000, 0x5e7efdb7, BRF_GRA },	     	 //  21
	{ "bm13.14c",      0x08000, 0x1143829a, BRF_GRA },	     	 //  22
		
	{ "bm14.5f",       0x08000, 0x4d3c6401, BRF_GRA },	     	 //  23	Tilemaps
	{ "bm23.8k",       0x08000, 0xa52aecbd, BRF_GRA },	     	 //  24
	
	{ "bm1.12a",       0x00100, 0x74421f18, BRF_GRA },	     	 //  25	PROMs
	{ "bm2.13a",       0x00100, 0xac27541f, BRF_GRA },	     	 //  26
	{ "bm3.14a",       0x00100, 0x251fb6ff, BRF_GRA },	     	 //  27
	{ "bm5.7f",        0x00100, 0x206713d0, BRF_GRA },	     	 //  28
	{ "bm10.7l",       0x00100, 0x33c2491c, BRF_GRA },	     	 //  29
	{ "bm9.6l",        0x00100, 0xaeea4af7, BRF_GRA },	     	 //  30
	{ "bm12.12m",      0x00100, 0xc18aa136, BRF_GRA },	     	 //  31
	{ "bm11.12l",      0x00100, 0x405aae37, BRF_GRA },	     	 //  32
	{ "bm8.8c",        0x00100, 0xc2010a9e, BRF_GRA },	     	 //  33
	{ "bm7.7c",        0x00100, 0xb56f30c3, BRF_GRA },	     	 //  34
	{ "bm4.12c",       0x00100, 0x91a8a2e1, BRF_GRA },	     	 //  35
	{ "bm6.4b",        0x00100, 0x0eaf5158, BRF_GRA },	     	 //  36
	
	{ "bm.7k",         0x01000, 0xcf4781bf, BRF_OPT | BRF_PRG }, //  37	MCU
};

STD_ROM_PICK(Drv)
STD_ROM_FN(Drv)

static struct BurnRomInfo DrvuRomDesc[] = {
	{ "bmu01c.12d",    0x08000, 0xc686cc5c, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "bmu02c.13d",    0x10000, 0xd8880a41, BRF_ESS | BRF_PRG }, //	 1
	{ "bmu03c.14d",    0x10000, 0x3f0ee26c, BRF_ESS | BRF_PRG }, //	 2
	
	{ "bm05.4k",       0x08000, 0xee2bd2d7, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 

	{ "bm04.5h",       0x08000, 0x46cb9d3d, BRF_GRA },	     	 //  4	Characters
	
	{ "bm15.10f",      0x08000, 0x6b1a0443, BRF_GRA },	     	 //  5	BG Tiles
	{ "bm16.11f",      0x08000, 0x23c908c2, BRF_GRA },	     	 //  6
	{ "bm17.12f",      0x08000, 0x46bcdd07, BRF_GRA },	     	 //  7
	{ "bm18.14f",      0x08000, 0xe6ae7ba0, BRF_GRA },	     	 //  8
	{ "bm19.10j",      0x08000, 0x868ababc, BRF_GRA },	     	 //  9
	{ "bm20.11j",      0x08000, 0x0917e5d4, BRF_GRA },	     	 //  10
	{ "bm21.12j",      0x08000, 0x9bfb0d89, BRF_GRA },	     	 //  11
	{ "bm22.14j",      0x08000, 0x04f3c274, BRF_GRA },	     	 //  12
	
	{ "bm24.14k",      0x08000, 0x11134036, BRF_GRA },	     	 //  13	BG2 Tiles
	{ "bm25.14l",      0x08000, 0x092cf9c1, BRF_GRA },	     	 //  14
	
	{ "bm06.10a",      0x08000, 0x97acc8af, BRF_GRA },	     	 //  15	Sprites
	{ "bm07.11a",      0x08000, 0xd78f7197, BRF_GRA },	     	 //  16
	{ "bm08.12a",      0x08000, 0x1a626608, BRF_GRA },	     	 //  17
	{ "bm09.14a",      0x08000, 0x92408400, BRF_GRA },	     	 //  18
	{ "bm10.10c",      0x08000, 0x8438a44a, BRF_GRA },	     	 //  19
	{ "bm11.11c",      0x08000, 0x6c69351d, BRF_GRA },	     	 //  20
	{ "bm12.12c",      0x08000, 0x5e7efdb7, BRF_GRA },	     	 //  21
	{ "bm13.14c",      0x08000, 0x1143829a, BRF_GRA },	     	 //  22
		
	{ "bm14.5f",       0x08000, 0x4d3c6401, BRF_GRA },	     	 //  23	Tilemaps
	{ "bm23.8k",       0x08000, 0xa52aecbd, BRF_GRA },	     	 //  24
	
	{ "bm1.12a",       0x00100, 0x74421f18, BRF_GRA },	     	 //  25	PROMs
	{ "bm2.13a",       0x00100, 0xac27541f, BRF_GRA },	     	 //  26
	{ "bm3.14a",       0x00100, 0x251fb6ff, BRF_GRA },	     	 //  27
	{ "bm5.7f",        0x00100, 0x206713d0, BRF_GRA },	     	 //  28
	{ "bm10.7l",       0x00100, 0x33c2491c, BRF_GRA },	     	 //  29
	{ "bm9.6l",        0x00100, 0xaeea4af7, BRF_GRA },	     	 //  30
	{ "bm12.12m",      0x00100, 0xc18aa136, BRF_GRA },	     	 //  31
	{ "bm11.12l",      0x00100, 0x405aae37, BRF_GRA },	     	 //  32
	{ "bm8.8c",        0x00100, 0xc2010a9e, BRF_GRA },	     	 //  33
	{ "bm7.7c",        0x00100, 0xb56f30c3, BRF_GRA },	     	 //  34
	{ "bm4.12c",       0x00100, 0x91a8a2e1, BRF_GRA },	     	 //  35
	{ "bm6.4b",        0x00100, 0x0eaf5158, BRF_GRA },	     	 //  36
	
	{ "bm.7k",         0x01000, 0xcf4781bf, BRF_OPT | BRF_PRG }, //  37	MCU
};

STD_ROM_PICK(Drvu)
STD_ROM_FN(Drvu)

static struct BurnRomInfo DrvuaRomDesc[] = {
	{ "bmu01.12d",     0x08000, 0x793cf15f, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "bmu02.13d",     0x10000, 0x6f1353d5, BRF_ESS | BRF_PRG }, //	 1
	{ "bmu03.14d",     0x10000, 0x9e7c07f7, BRF_ESS | BRF_PRG }, //	 2
	
	{ "bm05.4k",       0x08000, 0xee2bd2d7, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 

	{ "bm04.5h",       0x08000, 0x46cb9d3d, BRF_GRA },	     	 //  4	Characters
	
	{ "bm15.10f",      0x08000, 0x6b1a0443, BRF_GRA },	     	 //  5	BG Tiles
	{ "bm16.11f",      0x08000, 0x23c908c2, BRF_GRA },	     	 //  6
	{ "bm17.12f",      0x08000, 0x46bcdd07, BRF_GRA },	     	 //  7
	{ "bm18.14f",      0x08000, 0xe6ae7ba0, BRF_GRA },	     	 //  8
	{ "bm19.10j",      0x08000, 0x868ababc, BRF_GRA },	     	 //  9
	{ "bm20.11j",      0x08000, 0x0917e5d4, BRF_GRA },	     	 //  10
	{ "bm21.12j",      0x08000, 0x9bfb0d89, BRF_GRA },	     	 //  11
	{ "bm22.14j",      0x08000, 0x04f3c274, BRF_GRA },	     	 //  12
	
	{ "bm24.14k",      0x08000, 0x11134036, BRF_GRA },	     	 //  13	BG2 Tiles
	{ "bm25.14l",      0x08000, 0x092cf9c1, BRF_GRA },	     	 //  14
	
	{ "bm06.10a",      0x08000, 0x97acc8af, BRF_GRA },	     	 //  15	Sprites
	{ "bm07.11a",      0x08000, 0xd78f7197, BRF_GRA },	     	 //  16
	{ "bm08.12a",      0x08000, 0x1a626608, BRF_GRA },	     	 //  17
	{ "bm09.14a",      0x08000, 0x92408400, BRF_GRA },	     	 //  18
	{ "bm10.10c",      0x08000, 0x8438a44a, BRF_GRA },	     	 //  19
	{ "bm11.11c",      0x08000, 0x6c69351d, BRF_GRA },	     	 //  20
	{ "bm12.12c",      0x08000, 0x5e7efdb7, BRF_GRA },	     	 //  21
	{ "bm13.14c",      0x08000, 0x1143829a, BRF_GRA },	     	 //  22
		
	{ "bm14.5f",       0x08000, 0x4d3c6401, BRF_GRA },	     	 //  23	Tilemaps
	{ "bm23.8k",       0x08000, 0xa52aecbd, BRF_GRA },	     	 //  24
	
	{ "bm1.12a",       0x00100, 0x74421f18, BRF_GRA },	     	 //  25	PROMs
	{ "bm2.13a",       0x00100, 0xac27541f, BRF_GRA },	     	 //  26
	{ "bm3.14a",       0x00100, 0x251fb6ff, BRF_GRA },	     	 //  27
	{ "bm5.7f",        0x00100, 0x206713d0, BRF_GRA },	     	 //  28
	{ "bm10.7l",       0x00100, 0x33c2491c, BRF_GRA },	     	 //  29
	{ "bm9.6l",        0x00100, 0xaeea4af7, BRF_GRA },	     	 //  30
	{ "bm12.12m",      0x00100, 0xc18aa136, BRF_GRA },	     	 //  31
	{ "bm11.12l",      0x00100, 0x405aae37, BRF_GRA },	     	 //  32
	{ "bm8.8c",        0x00100, 0xc2010a9e, BRF_GRA },	     	 //  33
	{ "bm7.7c",        0x00100, 0xb56f30c3, BRF_GRA },	     	 //  34
	{ "bm4.12c",       0x00100, 0x91a8a2e1, BRF_GRA },	     	 //  35
	{ "bm6.4b",        0x00100, 0x0eaf5158, BRF_GRA },	     	 //  36
	
	{ "bm.7k",         0x01000, 0xcf4781bf, BRF_OPT | BRF_PRG }, //  37	MCU
};

STD_ROM_PICK(Drvua)
STD_ROM_FN(Drvua)

static struct BurnRomInfo DrvjRomDesc[] = {
	{ "bm01b.12d",     0x08000, 0x363f9f3d, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "bm02b.13d",     0x10000, 0x7f0d7edc, BRF_ESS | BRF_PRG }, //	 1
	{ "bm03b.14d",     0x10000, 0x7093da2a, BRF_ESS | BRF_PRG }, //	 2
	
	{ "bm05.4k",       0x08000, 0xee2bd2d7, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 

	{ "bm04.5h",       0x08000, 0x46cb9d3d, BRF_GRA },	     	 //  4	Characters
	
	{ "bm15.10f",      0x08000, 0x6b1a0443, BRF_GRA },	     	 //  5	BG Tiles
	{ "bm16.11f",      0x08000, 0x23c908c2, BRF_GRA },	     	 //  6
	{ "bm17.12f",      0x08000, 0x46bcdd07, BRF_GRA },	     	 //  7
	{ "bm18.14f",      0x08000, 0xe6ae7ba0, BRF_GRA },	     	 //  8
	{ "bm19.10j",      0x08000, 0x868ababc, BRF_GRA },	     	 //  9
	{ "bm20.11j",      0x08000, 0x0917e5d4, BRF_GRA },	     	 //  10
	{ "bm21.12j",      0x08000, 0x9bfb0d89, BRF_GRA },	     	 //  11
	{ "bm22.14j",      0x08000, 0x04f3c274, BRF_GRA },	     	 //  12
	
	{ "bm24.14k",      0x08000, 0x11134036, BRF_GRA },	     	 //  13	BG2 Tiles
	{ "bm25.14l",      0x08000, 0x092cf9c1, BRF_GRA },	     	 //  14
	
	{ "bm06.10a",      0x08000, 0x97acc8af, BRF_GRA },	     	 //  15	Sprites
	{ "bm07.11a",      0x08000, 0xd78f7197, BRF_GRA },	     	 //  16
	{ "bm08.12a",      0x08000, 0x1a626608, BRF_GRA },	     	 //  17
	{ "bm09.14a",      0x08000, 0x92408400, BRF_GRA },	     	 //  18
	{ "bm10.10c",      0x08000, 0x8438a44a, BRF_GRA },	     	 //  19
	{ "bm11.11c",      0x08000, 0x6c69351d, BRF_GRA },	     	 //  20
	{ "bm12.12c",      0x08000, 0x5e7efdb7, BRF_GRA },	     	 //  21
	{ "bm13.14c",      0x08000, 0x1143829a, BRF_GRA },	     	 //  22
		
	{ "bm14.5f",       0x08000, 0x4d3c6401, BRF_GRA },	     	 //  23	Tilemaps
	{ "bm23.8k",       0x08000, 0xa52aecbd, BRF_GRA },	     	 //  24
	
	{ "bm1.12a",       0x00100, 0x74421f18, BRF_GRA },	     	 //  25	PROMs
	{ "bm2.13a",       0x00100, 0xac27541f, BRF_GRA },	     	 //  26
	{ "bm3.14a",       0x00100, 0x251fb6ff, BRF_GRA },	     	 //  27
	{ "bm5.7f",        0x00100, 0x206713d0, BRF_GRA },	     	 //  28
	{ "bm10.7l",       0x00100, 0x33c2491c, BRF_GRA },	     	 //  29
	{ "bm9.6l",        0x00100, 0xaeea4af7, BRF_GRA },	     	 //  30
	{ "bm12.12m",      0x00100, 0xc18aa136, BRF_GRA },	     	 //  31
	{ "bm11.12l",      0x00100, 0x405aae37, BRF_GRA },	     	 //  32
	{ "bm8.8c",        0x00100, 0xc2010a9e, BRF_GRA },	     	 //  33
	{ "bm7.7c",        0x00100, 0xb56f30c3, BRF_GRA },	     	 //  34
	{ "bm4.12c",       0x00100, 0x91a8a2e1, BRF_GRA },	     	 //  35
	{ "bm6.4b",        0x00100, 0x0eaf5158, BRF_GRA },	     	 //  36
	
	{ "bm.7k",         0x01000, 0xcf4781bf, BRF_OPT | BRF_PRG }, //  37	MCU
};

STD_ROM_PICK(Drvj)
STD_ROM_FN(Drvj)

static struct BurnRomInfo DrvjaRomDesc[] = {
	{ "bm01.12d",      0x08000, 0x232df705, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "bm02.13d",      0x10000, 0xaf971575, BRF_ESS | BRF_PRG }, //	 1
	{ "bm03.14d",      0x10000, 0x300ec713, BRF_ESS | BRF_PRG }, //	 2
	
	{ "bm05.4k",       0x08000, 0xee2bd2d7, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 

	{ "bm04.5h",       0x08000, 0x46cb9d3d, BRF_GRA },	     	 //  4	Characters
	
	{ "bm15.10f",      0x08000, 0x6b1a0443, BRF_GRA },	     	 //  5	BG Tiles
	{ "bm16.11f",      0x08000, 0x23c908c2, BRF_GRA },	     	 //  6
	{ "bm17.12f",      0x08000, 0x46bcdd07, BRF_GRA },	     	 //  7
	{ "bm18.14f",      0x08000, 0xe6ae7ba0, BRF_GRA },	     	 //  8
	{ "bm19.10j",      0x08000, 0x868ababc, BRF_GRA },	     	 //  9
	{ "bm20.11j",      0x08000, 0x0917e5d4, BRF_GRA },	     	 //  10
	{ "bm21.12j",      0x08000, 0x9bfb0d89, BRF_GRA },	     	 //  11
	{ "bm22.14j",      0x08000, 0x04f3c274, BRF_GRA },	     	 //  12
	
	{ "bm24.14k",      0x08000, 0x11134036, BRF_GRA },	     	 //  13	BG2 Tiles
	{ "bm25.14l",      0x08000, 0x092cf9c1, BRF_GRA },	     	 //  14
	
	{ "bm06.10a",      0x08000, 0x97acc8af, BRF_GRA },	     	 //  15	Sprites
	{ "bm07.11a",      0x08000, 0xd78f7197, BRF_GRA },	     	 //  16
	{ "bm08.12a",      0x08000, 0x1a626608, BRF_GRA },	     	 //  17
	{ "bm09.14a",      0x08000, 0x92408400, BRF_GRA },	     	 //  18
	{ "bm10.10c",      0x08000, 0x8438a44a, BRF_GRA },	     	 //  19
	{ "bm11.11c",      0x08000, 0x6c69351d, BRF_GRA },	     	 //  20
	{ "bm12.12c",      0x08000, 0x5e7efdb7, BRF_GRA },	     	 //  21
	{ "bm13.14c",      0x08000, 0x1143829a, BRF_GRA },	     	 //  22
		
	{ "bm14.5f",       0x08000, 0x4d3c6401, BRF_GRA },	     	 //  23	Tilemaps
	{ "bm23.8k",       0x08000, 0xa52aecbd, BRF_GRA },	     	 //  24
	
	{ "bm1.12a",       0x00100, 0x74421f18, BRF_GRA },	     	 //  25	PROMs
	{ "bm2.13a",       0x00100, 0xac27541f, BRF_GRA },	     	 //  26
	{ "bm3.14a",       0x00100, 0x251fb6ff, BRF_GRA },	     	 //  27
	{ "bm5.7f",        0x00100, 0x206713d0, BRF_GRA },	     	 //  28
	{ "bm10.7l",       0x00100, 0x33c2491c, BRF_GRA },	     	 //  29
	{ "bm9.6l",        0x00100, 0xaeea4af7, BRF_GRA },	     	 //  30
	{ "bm12.12m",      0x00100, 0xc18aa136, BRF_GRA },	     	 //  31
	{ "bm11.12l",      0x00100, 0x405aae37, BRF_GRA },	     	 //  32
	{ "bm8.8c",        0x00100, 0xc2010a9e, BRF_GRA },	     	 //  33
	{ "bm7.7c",        0x00100, 0xb56f30c3, BRF_GRA },	     	 //  34
	{ "bm4.12c",       0x00100, 0x91a8a2e1, BRF_GRA },	     	 //  35
	{ "bm6.4b",        0x00100, 0x0eaf5158, BRF_GRA },	     	 //  36
	
	{ "bm.7k",         0x01000, 0xcf4781bf, BRF_OPT | BRF_PRG }, //  37	MCU
};

STD_ROM_PICK(Drvja)
STD_ROM_FN(Drvja)

static struct BurnRomInfo DrvjahRomDesc[] = {
	{ "bm01_hack.12d", 0x08000, 0xf6935937, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "bm02.13d",      0x10000, 0xaf971575, BRF_ESS | BRF_PRG }, //	 1
	{ "bm03.14d",      0x10000, 0x300ec713, BRF_ESS | BRF_PRG }, //	 2
	
	{ "bm05.4k",       0x08000, 0xee2bd2d7, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program

	{ "bm04.5h",       0x08000, 0x46cb9d3d, BRF_GRA },	     	 //  4	Characters
	
	{ "bm15.10f",      0x08000, 0x6b1a0443, BRF_GRA },	     	 //  5	BG Tiles
	{ "bm16.11f",      0x08000, 0x23c908c2, BRF_GRA },	     	 //  6
	{ "bm17.12f",      0x08000, 0x46bcdd07, BRF_GRA },	     	 //  7
	{ "bm18.14f",      0x08000, 0xe6ae7ba0, BRF_GRA },	     	 //  8
	{ "bm19.10j",      0x08000, 0x868ababc, BRF_GRA },	     	 //  9
	{ "bm20.11j",      0x08000, 0x0917e5d4, BRF_GRA },	     	 //  10
	{ "bm21.12j",      0x08000, 0x9bfb0d89, BRF_GRA },	     	 //  11
	{ "bm22.14j",      0x08000, 0x04f3c274, BRF_GRA },	     	 //  12
	
	{ "bm24.14k",      0x08000, 0x11134036, BRF_GRA },	     	 //  13	BG2 Tiles
	{ "bm25.14l",      0x08000, 0x092cf9c1, BRF_GRA },	     	 //  14
	
	{ "bm06.10a",      0x08000, 0x97acc8af, BRF_GRA },	     	 //  15	Sprites
	{ "bm07.11a",      0x08000, 0xd78f7197, BRF_GRA },	     	 //  16
	{ "bm08.12a",      0x08000, 0x1a626608, BRF_GRA },	     	 //  17
	{ "bm09.14a",      0x08000, 0x92408400, BRF_GRA },	     	 //  18
	{ "bm10.10c",      0x08000, 0x8438a44a, BRF_GRA },	     	 //  19
	{ "bm11.11c",      0x08000, 0x6c69351d, BRF_GRA },	     	 //  20
	{ "bm12.12c",      0x08000, 0x5e7efdb7, BRF_GRA },	     	 //  21
	{ "bm13.14c",      0x08000, 0x1143829a, BRF_GRA },	     	 //  22
		
	{ "bm14.5f",       0x08000, 0x4d3c6401, BRF_GRA },	     	 //  23	Tilemaps
	{ "bm23.8k",       0x08000, 0xa52aecbd, BRF_GRA },	     	 //  24
	
	{ "bm1.12a",       0x00100, 0x74421f18, BRF_GRA },	     	 //  25	PROMs
	{ "bm2.13a",       0x00100, 0xac27541f, BRF_GRA },	     	 //  26
	{ "bm3.14a",       0x00100, 0x251fb6ff, BRF_GRA },	     	 //  27
	{ "bm5.7f",        0x00100, 0x206713d0, BRF_GRA },	     	 //  28
	{ "bm10.7l",       0x00100, 0x33c2491c, BRF_GRA },	     	 //  29
	{ "bm9.6l",        0x00100, 0xaeea4af7, BRF_GRA },	     	 //  30
	{ "bm12.12m",      0x00100, 0xc18aa136, BRF_GRA },	     	 //  31
	{ "bm11.12l",      0x00100, 0x405aae37, BRF_GRA },	     	 //  32
	{ "bm8.8c",        0x00100, 0xc2010a9e, BRF_GRA },	     	 //  33
	{ "bm7.7c",        0x00100, 0xb56f30c3, BRF_GRA },	     	 //  34
	{ "bm4.12c",       0x00100, 0x91a8a2e1, BRF_GRA },	     	 //  35
	{ "bm6.4b",        0x00100, 0x0eaf5158, BRF_GRA },	     	 //  36
	
	{ "bm.7k",         0x01000, 0xcf4781bf, BRF_OPT | BRF_PRG }, //  37	MCU
};

STD_ROM_PICK(Drvjah)
STD_ROM_FN(Drvjah)

static struct BurnRomInfo DrvjahaRomDesc[] = {
	{ "bm01_hack.12d", 0x08000, 0xf6935937, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "bm02.13d",      0x10000, 0xaf971575, BRF_ESS | BRF_PRG }, //	 1
	{ "bm03.14d",      0x10000, 0x300ec713, BRF_ESS | BRF_PRG }, //	 2
	
	{ "005.bin",       0x08000, 0x06ea3878, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program

	{ "bm04.5h",       0x08000, 0x46cb9d3d, BRF_GRA },	     	 //  4	Characters
	
	{ "bm15.10f",      0x08000, 0x6b1a0443, BRF_GRA },	     	 //  5	BG Tiles
	{ "bm16.11f",      0x08000, 0x23c908c2, BRF_GRA },	     	 //  6
	{ "bm17.12f",      0x08000, 0x46bcdd07, BRF_GRA },	     	 //  7
	{ "bm18.14f",      0x08000, 0xe6ae7ba0, BRF_GRA },	     	 //  8
	{ "bm19.10j",      0x08000, 0x868ababc, BRF_GRA },	     	 //  9
	{ "bm20.11j",      0x08000, 0x0917e5d4, BRF_GRA },	     	 //  10
	{ "bm21.12j",      0x08000, 0x9bfb0d89, BRF_GRA },	     	 //  11
	{ "bm22.14j",      0x08000, 0x04f3c274, BRF_GRA },	     	 //  12
	
	{ "bm24.14k",      0x08000, 0x11134036, BRF_GRA },	     	 //  13	BG2 Tiles
	{ "bm25.14l",      0x08000, 0x092cf9c1, BRF_GRA },	     	 //  14
	
	{ "bm06.10a",      0x08000, 0x97acc8af, BRF_GRA },	     	 //  15	Sprites
	{ "bm07.11a",      0x08000, 0xd78f7197, BRF_GRA },	     	 //  16
	{ "bm08.12a",      0x08000, 0x1a626608, BRF_GRA },	     	 //  17
	{ "bm09.14a",      0x08000, 0x92408400, BRF_GRA },	     	 //  18
	{ "013.bin",       0x08000, 0x1b072548, BRF_GRA },	     	 //  19
	{ "bm11.11c",      0x08000, 0x6c69351d, BRF_GRA },	     	 //  20
	{ "bm12.12c",      0x08000, 0x5e7efdb7, BRF_GRA },	     	 //  21
	{ "bm13.14c",      0x08000, 0x1143829a, BRF_GRA },	     	 //  22
		
	{ "bm14.5f",       0x08000, 0x4d3c6401, BRF_GRA },	     	 //  23	Tilemaps
	{ "bm23.8k",       0x08000, 0xa52aecbd, BRF_GRA },	     	 //  24
	
	{ "bm1.12a",       0x00100, 0x74421f18, BRF_GRA },	     	 //  25	PROMs
	{ "bm2.13a",       0x00100, 0xac27541f, BRF_GRA },	     	 //  26
	{ "bm3.14a",       0x00100, 0x251fb6ff, BRF_GRA },	     	 //  27
	{ "bm5.7f",        0x00100, 0x206713d0, BRF_GRA },	     	 //  28
	{ "bm10.7l",       0x00100, 0x33c2491c, BRF_GRA },	     	 //  29
	{ "bm9.6l",        0x00100, 0xaeea4af7, BRF_GRA },	     	 //  30
	{ "bm12.12m",      0x00100, 0xc18aa136, BRF_GRA },	     	 //  31
	{ "bm11.12l",      0x00100, 0x405aae37, BRF_GRA },	     	 //  32
	{ "bm8.8c",        0x00100, 0xc2010a9e, BRF_GRA },	     	 //  33
	{ "bm7.7c",        0x00100, 0xb56f30c3, BRF_GRA },	     	 //  34
	{ "bm4.12c",       0x00100, 0x91a8a2e1, BRF_GRA },	     	 //  35
	{ "bm6.4b",        0x00100, 0x0eaf5158, BRF_GRA },	     	 //  36
	
	{ "bm.7k",         0x01000, 0xcf4781bf, BRF_OPT | BRF_PRG }, //  37	MCU
};

STD_ROM_PICK(Drvjaha)
STD_ROM_FN(Drvjaha)

static struct BurnRomInfo DrvbRomDesc[] = {
	{ "1.12d",         0x08000, 0x9a2d70ab, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "bm02.13d",      0x10000, 0xaf971575, BRF_ESS | BRF_PRG }, //	 1
	{ "bm03.14d",      0x10000, 0x300ec713, BRF_ESS | BRF_PRG }, //	 2
	
	{ "bm05.4k",       0x08000, 0xee2bd2d7, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 

	{ "4.5h",          0x08000, 0x0aba2096, BRF_GRA },	     	 //  4	Characters
	
	{ "15.12f",        0x10000, 0x622b4fba, BRF_GRA },	     	 //  5	BG Tiles
	{ "16.14f",        0x10000, 0x25471a8d, BRF_GRA },	     	 //  6
	{ "17.12j",        0x10000, 0x9da79653, BRF_GRA },	     	 //  7
	{ "18.14j",        0x10000, 0x1f3aced8, BRF_GRA },	     	 //  8
	
	{ "bm24.14k",      0x08000, 0x11134036, BRF_GRA },	     	 //  9	BG2 Tiles
	{ "bm25.14l",      0x08000, 0x092cf9c1, BRF_GRA },	     	 //  10
	
	{ "bm06.10a",      0x08000, 0x97acc8af, BRF_GRA },	     	 //  11	Sprites
	{ "bm07.11a",      0x08000, 0xd78f7197, BRF_GRA },	     	 //  12
	{ "bm08.12a",      0x08000, 0x1a626608, BRF_GRA },	     	 //  13
	{ "bm09.14a",      0x08000, 0x92408400, BRF_GRA },	     	 //  14
	{ "bm10.10c",      0x08000, 0x8438a44a, BRF_GRA },	     	 //  15
	{ "bm11.11c",      0x08000, 0x6c69351d, BRF_GRA },	     	 //  16
	{ "bm12.12c",      0x08000, 0x5e7efdb7, BRF_GRA },	     	 //  17
	{ "bm13.14c",      0x08000, 0x1143829a, BRF_GRA },	     	 //  18
		
	{ "bm14.5f",       0x08000, 0x4d3c6401, BRF_GRA },	     	 //  19	Tilemaps
	{ "bm23.8k",       0x08000, 0xa52aecbd, BRF_GRA },	     	 //  20
	
	{ "bm1.12a",       0x00100, 0x74421f18, BRF_GRA },	     	 //  21	PROMs
	{ "bm2.13a",       0x00100, 0xac27541f, BRF_GRA },	     	 //  22
	{ "bm3.14a",       0x00100, 0x251fb6ff, BRF_GRA },	     	 //  23
	{ "bm5.7f",        0x00100, 0x206713d0, BRF_GRA },	     	 //  24
	{ "bm10.7l",       0x00100, 0x33c2491c, BRF_GRA },	     	 //  25
	{ "bm9.6l",        0x00100, 0xaeea4af7, BRF_GRA },	     	 //  26
	{ "bm12.12m",      0x00100, 0xc18aa136, BRF_GRA },	     	 //  27
	{ "bm11.12l",      0x00100, 0x405aae37, BRF_GRA },	     	 //  28
	{ "bm8.8c",        0x00100, 0xc2010a9e, BRF_GRA },	     	 //  29
	{ "bm7.7c",        0x00100, 0xb56f30c3, BRF_GRA },	     	 //  30
	{ "bm4.12c",       0x00100, 0x91a8a2e1, BRF_GRA },	     	 //  31
	{ "bm6.4b",        0x00100, 0x0eaf5158, BRF_GRA },	     	 //  32
};

STD_ROM_PICK(Drvb)
STD_ROM_FN(Drvb)

static struct BurnRomInfo Drvb2RomDesc[] = {
	{ "u28.bin",       0x08000, 0xb3b7c7cd, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "u27.bin",       0x10000, 0xaf971575, BRF_ESS | BRF_PRG }, //	 1
	{ "u26.bin",       0x10000, 0x300ec713, BRF_ESS | BRF_PRG }, //	 2
	
	{ "u88.bin",       0x08000, 0xee2bd2d7, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 

	{ "u62.bin",       0x08000, 0x0aba2096, BRF_GRA },	     	 //  4	Characters
	
	{ "u66.bin",       0x10000, 0x075e9a7f, BRF_GRA },	     	 //  5	BG Tiles
	{ "u65.bin",       0x10000, 0x05aca09a, BRF_GRA },	     	 //  6
	{ "u79.bin",       0x10000, 0x0f4b7e0e, BRF_GRA },	     	 //  7
	{ "u70.bin",       0x10000, 0x61a90c0a, BRF_GRA },	     	 //  8
	
	{ "u102.bin",      0x08000, 0x11134036, BRF_GRA },	     	 //  9	BG2 Tiles
	{ "u116.bin",      0x08000, 0x092cf9c1, BRF_GRA },	     	 //  10
	
	{ "u9.bin",        0x08000, 0x97acc8af, BRF_GRA },	     	 //  11	Sprites
	{ "u10.bin",       0x08000, 0xd78f7197, BRF_GRA },	     	 //  12
	{ "u8.bin",        0x10000, 0xed5c788a, BRF_GRA },	     	 //  13
	{ "u18.bin",       0x08000, 0x8438a44a, BRF_GRA },	     	 //  14
	{ "u19.bin",       0x08000, 0x6c69351d, BRF_GRA },	     	 //  15
	{ "u17.bin",       0x10000, 0x4b42445e, BRF_GRA },	     	 //  16
		
	{ "u58.bin",       0x08000, 0x4d3c6401, BRF_GRA },	     	 //  17	Tilemaps
	{ "u96.bin",       0x08000, 0x254c881d, BRF_GRA },	     	 //  18
	
	{ "bm1.12a",       0x00100, 0x74421f18, BRF_GRA },	     	 //  19	PROMs
	{ "bm2.13a",       0x00100, 0xac27541f, BRF_GRA },	     	 //  20
	{ "bm3.14a",       0x00100, 0x251fb6ff, BRF_GRA },	     	 //  21
	{ "bm5.7f",        0x00100, 0x206713d0, BRF_GRA },	     	 //  22
	{ "bm10.7l",       0x00100, 0x33c2491c, BRF_GRA },	     	 //  23
	{ "bm9.6l",        0x00100, 0xaeea4af7, BRF_GRA },	     	 //  24
	{ "bm12.12m",      0x00100, 0xc18aa136, BRF_GRA },	     	 //  25
	{ "bm11.12l",      0x00100, 0x405aae37, BRF_GRA },	     	 //  26
	{ "bm8.8c",        0x00100, 0xc2010a9e, BRF_GRA },	     	 //  27
	{ "bm7.7c",        0x00100, 0xb56f30c3, BRF_GRA },	     	 //  28
	{ "bm4.12c",       0x00100, 0x91a8a2e1, BRF_GRA },	     	 //  29
	{ "bm6.4b",        0x00100, 0x0eaf5158, BRF_GRA },	     	 //  30
};

STD_ROM_PICK(Drvb2)
STD_ROM_FN(Drvb2)

static struct BurnRomInfo DrvbjRomDesc[] = {
	{ "mkb03.12d",     0x08000, 0xb3b7c7cd, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "bm02.13d",      0x10000, 0xaf971575, BRF_ESS | BRF_PRG }, //	 1
	{ "bm03.14d",      0x10000, 0x300ec713, BRF_ESS | BRF_PRG }, //	 2
	
	{ "bm05.4k",       0x08000, 0xee2bd2d7, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 

	{ "bm04.5h",       0x08000, 0x46cb9d3d, BRF_GRA },	     	 //  4	Characters
	
	{ "mkb12.12f",     0x10000, 0x075e9a7f, BRF_GRA },	     	 //  5	BG Tiles
	{ "mkb11.14f",     0x10000, 0x05aca09a, BRF_GRA },	     	 //  6
	{ "mkb09.12j",     0x10000, 0x0f4b7e0e, BRF_GRA },	     	 //  7
	{ "mkb10.14j",     0x10000, 0x61a90c0a, BRF_GRA },	     	 //  8
	
	{ "mkb08.14k",     0x08000, 0x798215e3, BRF_GRA },	     	 //  9	BG2 Tiles
	{ "bm25.14l",      0x08000, 0x092cf9c1, BRF_GRA },	     	 //  10
	
	{ "bm06.10a",      0x08000, 0x97acc8af, BRF_GRA },	     	 //  11	Sprites
	{ "bm07.11a",      0x08000, 0xd78f7197, BRF_GRA },	     	 //  12
	{ "bm08.12a",      0x08000, 0x1a626608, BRF_GRA },	     	 //  13
	{ "bm09.14a",      0x08000, 0x92408400, BRF_GRA },	     	 //  14
	{ "bm10.10c",      0x08000, 0x8438a44a, BRF_GRA },	     	 //  15
	{ "bm11.11c",      0x08000, 0x6c69351d, BRF_GRA },	     	 //  16
	{ "bm12.12c",      0x08000, 0x5e7efdb7, BRF_GRA },	     	 //  17
	{ "bm13.14c",      0x08000, 0x1143829a, BRF_GRA },	     	 //  18
		
	{ "bm14.5f",       0x08000, 0x4d3c6401, BRF_GRA },	     	 //  19	Tilemaps
	{ "mkb07.8k",      0x08000, 0xae1b317f, BRF_GRA },	     	 //  20
	
	{ "bm1.12a",       0x00100, 0x74421f18, BRF_GRA },	     	 //  21	PROMs
	{ "bm2.13a",       0x00100, 0xac27541f, BRF_GRA },	     	 //  22
	{ "bm3.14a",       0x00100, 0x251fb6ff, BRF_GRA },	     	 //  23
	{ "bm5.7f",        0x00100, 0x206713d0, BRF_GRA },	     	 //  24
	{ "bm10.7l",       0x00100, 0x33c2491c, BRF_GRA },	     	 //  25
	{ "bm9.6l",        0x00100, 0xaeea4af7, BRF_GRA },	     	 //  26
	{ "bm12.12m",      0x00100, 0xc18aa136, BRF_GRA },	     	 //  27
	{ "bm11.12l",      0x00100, 0x405aae37, BRF_GRA },	     	 //  28
	{ "bm8.8c",        0x00100, 0xc2010a9e, BRF_GRA },	     	 //  29
	{ "bm7.7c",        0x00100, 0xb56f30c3, BRF_GRA },	     	 //  30
	{ "bm4.12c",       0x00100, 0x91a8a2e1, BRF_GRA },	     	 //  31
	{ "bm6.4b",        0x00100, 0x0eaf5158, BRF_GRA },	     	 //  32
};

STD_ROM_PICK(Drvbj)
STD_ROM_FN(Drvbj)

static struct BurnRomInfo DrvkaiRomDesc[] = {
	{ "bmk01.12d",     0x08000, 0x7d2211db, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "bmk02.13d",     0x10000, 0x2ebbc8c5, BRF_ESS | BRF_PRG }, //	 1
	{ "bmk03.14d",     0x10000, 0x475a6ac5, BRF_ESS | BRF_PRG }, //	 2
	
	{ "bmk05.4k",      0x08000, 0x25f37957, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 

	{ "bmk04.5h",      0x08000, 0x884a8692, BRF_GRA },	     	 //  4	Characters
	
	{ "bm15.10f",      0x08000, 0x6b1a0443, BRF_GRA },	     	 //  5	BG Tiles
	{ "bmk16.11f",     0x08000, 0x9416fe0d, BRF_GRA },	     	 //  6
	{ "bmk17.12f",     0x08000, 0x3d5acab9, BRF_GRA },	     	 //  7
	{ "bmk18.14f",     0x08000, 0x7b62da1d, BRF_GRA },	     	 //  8
	{ "bm19.10j",      0x08000, 0x868ababc, BRF_GRA },	     	 //  9
	{ "bmk20.11j",     0x08000, 0xb90364c1, BRF_GRA },	     	 //  10
	{ "bmk21.12j",     0x08000, 0x8c7fe74a, BRF_GRA },	     	 //  11
	{ "bmk22.14j",     0x08000, 0xd5ef8a0e, BRF_GRA },	     	 //  12
	
	{ "bmk24.14k",     0x08000, 0xbf186ef2, BRF_GRA },	     	 //  13	BG2 Tiles
	{ "bmk25.14l",     0x08000, 0xa755faf1, BRF_GRA },	     	 //  14
	
	{ "bmk06.10a",     0x08000, 0x5f7e38b3, BRF_GRA },	     	 //  15	Sprites
	{ "bmk07.11a",     0x08000, 0xff3751fd, BRF_GRA },	     	 //  16
	{ "bmk08.12a",     0x08000, 0x159d51bd, BRF_GRA },	     	 //  17
	{ "bmk09.14a",     0x08000, 0x8683e3d2, BRF_GRA },	     	 //  18
	{ "bmk10.10c",     0x08000, 0x1e0d9571, BRF_GRA },	     	 //  19
	{ "bmk11.11c",     0x08000, 0xf1fc5ee1, BRF_GRA },	     	 //  20
	{ "bmk12.12c",     0x08000, 0x0f50c001, BRF_GRA },	     	 //  21
	{ "bmk13.14c",     0x08000, 0xfd1acf8e, BRF_GRA },	     	 //  22
		
	{ "bmk14.5f",      0x08000, 0xcf0f5a53, BRF_GRA },	     	 //  23	Tilemaps
	{ "bmk23.8k",      0x08000, 0x17f77ef9, BRF_GRA },	     	 //  24
	
	{ "bmk1.12a",      0x00100, 0xe001ea33, BRF_GRA },	     	 //  25	PROMs
	{ "bmk2.13a",      0x00100, 0xaf34d91a, BRF_GRA },	     	 //  26
	{ "bmk3.14a",      0x00100, 0x43e9f6ef, BRF_GRA },	     	 //  27
	{ "bmk5.7f",       0x00100, 0x41878934, BRF_GRA },	     	 //  28
	{ "bmk10.7l",      0x00100, 0xde44b748, BRF_GRA },	     	 //  29
	{ "bmk9.6l",       0x00100, 0x59ea57c0, BRF_GRA },	     	 //  30
	{ "bmk12.12m",     0x00100, 0x8765f8b0, BRF_GRA },	     	 //  31
	{ "bmk11.12l",     0x00100, 0x87a8854e, BRF_GRA },	     	 //  32
	{ "bmk8.8c",       0x00100, 0xdad17e2d, BRF_GRA },	     	 //  33
	{ "bmk7.7c",       0x00100, 0x76307f8d, BRF_GRA },	     	 //  34
	{ "bm4.12c",       0x00100, 0x91a8a2e1, BRF_GRA },	     	 //  35
	{ "bm6.4b",        0x00100, 0x0eaf5158, BRF_GRA },	     	 //  36
	
	{ "bm.7k",         0x01000, 0xcf4781bf, BRF_OPT | BRF_PRG }, //  37	MCU
};

STD_ROM_PICK(Drvkai)
STD_ROM_FN(Drvkai)

static struct BurnRomInfo DrvmiiRomDesc[] = {
	/* Prototype, location test or limited release? - PCB had genuine CAPCOM labels on the roms */
	{ "01.12d",        0x08000, 0x8ba22485, BRF_ESS | BRF_PRG }, //  0	Z80 #1 Program Code
	{ "02.13d",        0x10000, 0x659a5455, BRF_ESS | BRF_PRG }, //	 1
	{ "03.14d",        0x10000, 0x159ea771, BRF_ESS | BRF_PRG }, //	 2
	
	{ "05.4k",         0x08000, 0x25f37957, BRF_ESS | BRF_PRG }, //  3	Z80 #2 Program 

	{ "04.5h",         0x08000, 0x8190e092, BRF_GRA },	     	 //  4	Characters
	
	{ "bm15.10f",      0x08000, 0x6b1a0443, BRF_GRA },	     	 //  5	BG Tiles
	{ "bm16.11f",      0x08000, 0x23c908c2, BRF_GRA },	     	 //  6
	{ "17.12f",        0x08000, 0x3d5acab9, BRF_GRA },	     	 //  7
	{ "18.14f",        0x08000, 0x7b62da1d, BRF_GRA },	     	 //  8
	{ "bm19.10j",      0x08000, 0x868ababc, BRF_GRA },	     	 //  9
	{ "bm20.11j",      0x08000, 0x0917e5d4, BRF_GRA },	     	 //  10
	{ "21.12j",        0x08000, 0x8c7fe74a, BRF_GRA },	     	 //  11
	{ "22.14j",        0x08000, 0xd5ef8a0e, BRF_GRA },	     	 //  12
	
	{ "24.14k",        0x08000, 0xa0074c68, BRF_GRA },	     	 //  13	BG2 Tiles
	{ "25.14l",        0x08000, 0xf979b2f2, BRF_GRA },	     	 //  14
	
	{ "06.10a",        0x08000, 0xb261d5d7, BRF_GRA },	     	 //  15	Sprites
	{ "07.11a",        0x08000, 0x2af8a6f2, BRF_GRA },	     	 //  16
	{ "08.12a",        0x08000, 0x159d51bd, BRF_GRA },	     	 //  17
	{ "09.14a",        0x08000, 0x70d9f9a7, BRF_GRA },	     	 //  18
	{ "10.10c",        0x08000, 0xde539920, BRF_GRA },	     	 //  19
	{ "11.11c",        0x08000, 0xa6abf183, BRF_GRA },	     	 //  20
	{ "12.12c",        0x08000, 0x0f50c001, BRF_GRA },	     	 //  21
	{ "13.14c",        0x08000, 0xf065f619, BRF_GRA },	     	 //  22
		
	{ "14.5f",         0x08000, 0x02a899f1, BRF_GRA },	     	 //  23	Tilemaps
	{ "23.8k",         0x08000, 0xb6dfdf85, BRF_GRA },	     	 //  24
	
	//  PCB had standard BM0x for bproms 1 through 3, but clearly these should use the Kai BPROMs for correct colors
    //  BPROMs 4 through 8 macth the Kai set - labels were a non descript yellow dot with prom number
    //  BPROMs 9 through 12 are unique - labels were a non descript yellow dot with prom number
	{ "bmk1.12a",      0x00100, 0xe001ea33, BRF_GRA },	     	 //  25	PROMs
	{ "bmk2.13a",      0x00100, 0xaf34d91a, BRF_GRA },	     	 //  26
	{ "bmk3.14a",      0x00100, 0x43e9f6ef, BRF_GRA },	     	 //  27
	{ "5.7f",          0x00100, 0x41878934, BRF_GRA },	     	 //  28
	{ "10.7l",         0x00100, 0xdb53adf0, BRF_GRA },	     	 //  29
	{ "9.6l",          0x00100, 0x75d5cc90, BRF_GRA },	     	 //  30
	{ "12.12m",        0x00100, 0x784bdf33, BRF_GRA },	     	 //  31
	{ "11.12l",        0x00100, 0x6fb2e170, BRF_GRA },	     	 //  32
	{ "8.8c",          0x00100, 0xdad17e2d, BRF_GRA },	     	 //  33
	{ "7.7c",          0x00100, 0x76307f8d, BRF_GRA },	     	 //  34
	{ "4.12c",         0x00100, 0x91a8a2e1, BRF_GRA },	     	 //  35
	{ "6.4b",          0x00100, 0x0eaf5158, BRF_GRA },	     	 //  36
	
	{ "bm.7k",         0x01000, 0xcf4781bf, BRF_OPT | BRF_PRG }, //  37	MCU
};

STD_ROM_PICK(Drvmii)
STD_ROM_FN(Drvmii)

static UINT8 Drv1943ProtRead()
{
	if (bootleg) return 0;

	// This data comes from a table at $21a containing 64 entries, even is "case", odd is return value.
	switch (DrvProtValue) {
		case 0x24: return 0x1d;
		case 0x60: return 0xf7;
		case 0x01: return 0xac;
		case 0x55: return 0x50;
		case 0x56: return 0xe2;
		case 0x2a: return 0x58;
		case 0xa8: return 0x13;
		case 0x22: return 0x3e;
		case 0x3b: return 0x5a;
		case 0x1e: return 0x1b;
		case 0xe9: return 0x41;
		case 0x7d: return 0xd5;
		case 0x43: return 0x54;
		case 0x37: return 0x6f;
		case 0x4c: return 0x59;
		case 0x5f: return 0x56;
		case 0x3f: return 0x2f;
		case 0x3e: return 0x3d;
		case 0xfb: return 0x36;
		case 0x1d: return 0x3b;
		case 0x27: return 0xae;
		case 0x26: return 0x39;
		case 0x58: return 0x3c;
		case 0x32: return 0x51;
		case 0x1a: return 0xa8;
		case 0xbc: return 0x33;
		case 0x30: return 0x4a;
		case 0x64: return 0x12;
		case 0x11: return 0x40;
		case 0x33: return 0x35;
		case 0x09: return 0x17;
		case 0x25: return 0x04;
	}

	return 0;
}

static UINT8 __fastcall Drv1943Read1(UINT16 a)
{
	switch (a) {
		case 0xc000: {
			return DrvInput[0];
		}
		
		case 0xc001: {
			return DrvInput[1];
		}
		
		case 0xc002: {
			return DrvInput[2];
		}
		
		case 0xc003: {
			return DrvDip[0];
		}
		
		case 0xc004: {
			return DrvDip[1];
		}
		
		case 0xc007: {
			return Drv1943ProtRead();
		}
	
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Read => %04X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Drv1943Write1(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xc800: {
			DrvSoundLatch = d;
			return;
		}
		
		case 0xc804: {
			DrvRomBank = d & 0x1c;

			ZetMapMemory(DrvZ80Rom1 + 0x10000 + (DrvRomBank * 0x1000), 0x8000, 0xbfff, MAP_ROM);
			
			DrvCharsOn = d & 0x80;
			return;
		}
		
		case 0xc806: {
			BurnWatchdogWrite();
			return;
		}
		
		case 0xc807: {
			DrvProtValue = d;
			return;
		}
		
		case 0xd800: {
			DrvBgScrollX[0] = d;
			return;
		}
		
		case 0xd801: {
			DrvBgScrollX[1] = d;
			return;
		}
		
		case 0xd802: {
			DrvBgScrollY = d;
			return;
		}
		
		case 0xd803: {
			DrvBg2ScrollX[0] = d;
			return;
		}
		
		case 0xd804: {
			DrvBg2ScrollX[1] = d;
			return;
		}
		
		case 0xd806: {
			DrvBg1On = d & 0x10;
			DrvBg2On = d & 0x20;
			DrvSpritesOn = d & 0x40;
			return;
		}

		case 0xD808:
		case 0xD868:
		case 0xD888:
		case 0xD8A8: {
			return; // NOP
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #1 Write => %04X, %02X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Drv1943Read2(UINT16 a)
{
	switch (a) {
		case 0xc800: {
			return DrvSoundLatch;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Read => %04X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Drv1943Write2(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xe000:
		case 0xe001: {
			BurnYM2203Write(0, a & 1, d);
			return;
		}
		
		case 0xe002:
		case 0xe003: {
			BurnYM2203Write(1, a & 1, d);
			return;
		}
		
		default: {
			bprintf(PRINT_NORMAL, _T("Z80 #2 Write => %04X, %02X\n"), a, d);
		}
	}
}

static INT32 CharPlaneOffsets[2]    = { 4, 0 };
static INT32 CharXOffsets[8]        = { STEP4(0,1), STEP4(8,1) };
static INT32 CharYOffsets[8]        = { STEP8(0,16) };
static INT32 Bg2TilePlaneOffsets[4] = { 0x40004, 0x40000, 4, 0 };
static INT32 BgTilePlaneOffsets[4]  = { 0x100004, 0x100000, 4, 0 };
static INT32 TileXOffsets[32]       = { STEP4(0,1), STEP4(8,1), STEP4(512,1), STEP4(520,1), STEP4(1024,1), STEP4(1032,1), STEP4(1536,1), STEP4(1544,1) };
static INT32 TileYOffsets[32]       = { STEP32(0,16) };
static INT32 SpritePlaneOffsets[4]  = { 0x100004, 0x100000, 4, 0 };
static INT32 SpriteXOffsets[16]     = { STEP4(0,1), STEP4(8,1), STEP4(256,1), STEP4(264,1) };
static INT32 SpriteYOffsets[16]     = { STEP16(0,16) };

static tilemap_callback( bg2 )
{
	INT32 Attr = DrvBg2Tilemap[offs * 2 + 1];
	INT32 Code = DrvBg2Tilemap[offs * 2 + 0];

	TILE_SET_INFO(0, Code, ((Attr >> 2) & 0xf), TILE_FLIPYX(Attr >> 6));
}

static tilemap_callback( bg1 )
{
	INT32 Attr = DrvBgTilemap[offs * 2 + 1];
	INT32 Code = DrvBgTilemap[offs * 2 + 0] + ((Attr & 0x01) << 8);

	sTile->category = (Attr >> 2) & 0xf;

	TILE_SET_INFO(1, Code, ((Attr >> 2) & 0xf), TILE_FLIPYX(Attr >> 6));
}

static tilemap_callback( fg )
{
	INT32 Attr = DrvPaletteRam[offs];
	INT32 Code = DrvVideoRam[offs] + ((Attr & 0xe0) << 3);

	TILE_SET_INFO(2, Code, Attr, 0);
}

static INT32 DrvDoReset(INT32 clear_mem)
{
	ZetOpen(0);
	ZetReset();
	ZetClose();

	ZetOpen(1);
	ZetReset();
	BurnYM2203Reset();
	ZetClose();

	BurnWatchdogReset();

	if (clear_mem) {
		DrvRomBank = 0;
		DrvSoundLatch = 0;
		DrvBg2ScrollX[0] = 0;
		DrvBg2ScrollX[1] = 0;
		DrvBgScrollX[0] = 0;
		DrvBgScrollX[1] = 0;
		DrvBgScrollY = 0;
		DrvBg2On = 0;
		DrvBg1On = 0;
		DrvSpritesOn = 0;
		DrvCharsOn = 0;
		DrvProtValue = 0;

		HiscoreReset();
	}

	return 0;
}

static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;

	DrvZ80Rom1             = Next; Next += 0x30000;
	DrvZ80Rom2             = Next; Next += 0x08000;
	DrvPromRed             = Next; Next += 0x00100;
	DrvPromGreen           = Next; Next += 0x00100;
	DrvPromBlue            = Next; Next += 0x00100;
	DrvPromCharLookup      = Next; Next += 0x00100;
	DrvPromBg2Lookup       = Next; Next += 0x00100;
	DrvPromBg2PalBank      = Next; Next += 0x00100;
	DrvPromBgLookup        = Next; Next += 0x00100;
	DrvPromBgPalBank       = Next; Next += 0x00100;
	DrvPromSpriteLookup    = Next; Next += 0x00100;
	DrvPromSpritePalBank   = Next; Next += 0x00100;
	DrvBgTilemap           = Next; Next += 0x08000;
	DrvBg2Tilemap          = Next; Next += 0x08000;

	RamStart               = Next;

	DrvZ80Ram1             = Next; Next += 0x01000;
	DrvZ80Ram2             = Next; Next += 0x00800;
	DrvVideoRam            = Next; Next += 0x00400;
	DrvPaletteRam          = Next; Next += 0x00400;
	DrvSpriteRam           = Next; Next += 0x01000;

	RamEnd                 = Next;

	DrvChars               = Next; Next += 2048 * 8 * 8;
	DrvBg2Tiles            = Next; Next += 128 * 32 * 32;
	DrvBgTiles             = Next; Next += 512 * 32 * 32;
	DrvSprites             = Next; Next += 2048 * 16 * 16;
	DrvPalette             = (UINT32*)Next; Next += 0x00380 * sizeof(UINT32);

	MemEnd                 = Next;

	return 0;
}

static INT32 CommonInit(INT32 (*load)())
{
	// Allocate and Blank all required memory
	Mem = NULL;
	MemIndex();
	INT32 nLen = MemEnd - (UINT8 *)0;
	if ((Mem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(Mem, 0, nLen);
	MemIndex();

	if (load()) return 1;

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetMapMemory(DrvZ80Rom1,		0x0000, 0x7fff, MAP_ROM);
	ZetMapMemory(DrvZ80Rom1 + 0x10000,	0x8000, 0xbfff, MAP_ROM);
	ZetMapMemory(DrvVideoRam,		0xd000, 0xd3ff, MAP_RAM);
	ZetMapMemory(DrvPaletteRam,		0xd400, 0xd7ff, MAP_RAM);
	ZetMapMemory(DrvZ80Ram1,		0xe000, 0xefff, MAP_RAM);
	ZetMapMemory(DrvSpriteRam,		0xf000, 0xffff, MAP_RAM);
	ZetSetReadHandler(Drv1943Read1);
	ZetSetWriteHandler(Drv1943Write1);
	ZetClose();
	
	ZetInit(1);
	ZetOpen(1);
	ZetMapMemory(DrvZ80Rom2,		0x0000, 0x7fff, MAP_ROM);
	ZetMapMemory(DrvZ80Ram2,		0xc000, 0xc7ff, MAP_RAM);
	ZetSetReadHandler(Drv1943Read2);
	ZetSetWriteHandler(Drv1943Write2);
	ZetClose();

	BurnWatchdogInit(DrvDoReset, 180);

	BurnYM2203Init(2, 1500000, NULL, 0);
	BurnTimerAttachZet(3000000);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_YM2203_ROUTE, 0.10, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_1, 0.15, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_2, 0.15, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_3, 0.15, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_YM2203_ROUTE, 0.10, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_1, 0.15, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_2, 0.15, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(1, BURN_SND_YM2203_AY8910_ROUTE_3, 0.15, BURN_SND_ROUTE_BOTH);

	GenericTilesInit();
	GenericTilemapInit(0, TILEMAP_SCAN_COLS, bg2_map_callback, 32, 32, 2048,  8);
	GenericTilemapInit(1, TILEMAP_SCAN_COLS, bg1_map_callback, 32, 32, 2048,  8);
	GenericTilemapInit(2, TILEMAP_SCAN_ROWS, fg_map_callback,   8,  8,   32, 32);
	GenericTilemapSetGfx(0, DrvBg2Tiles, 4, 32, 32, 0x20000, 0x180, 0x0f);
	GenericTilemapSetGfx(1, DrvBgTiles,  4, 32, 32, 0x80000, 0x080, 0x0f);
	GenericTilemapSetGfx(2, DrvChars,    2,  8,  8, 0x20000, 0x000, 0x1f);
	GenericTilemapSetOffsets(TMAP_GLOBAL, 0, -16);
	GenericTilemapSetTransparent(2, 0);

	GenericTilemapCategoryConfig(1, 0x100);
	for (INT32 i = 0; i < 0x100; i++) {
		UINT8 color = ((DrvPromBgPalBank[i] & 0x03) << 4) | (DrvPromBgLookup[i] & 0x0f);
		GenericTilemapSetCategoryEntry(1, i / 16, i % 16, (color == 0x0f) ? 1 : 0);
	}

	// Reset the driver
	DrvDoReset(1);

	return 0;
}

static INT32 DrvLoad()
{
	INT32 nRet = 0;

	DrvTempRom = (UINT8 *)BurnMalloc(0x40000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000, 1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x20000, 2, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000, 3, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 4, 1); if (nRet != 0) return 1;
	GfxDecode(2048, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the bg2 tiles
	memset(DrvTempRom, 0, 0x40000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 14, 1); if (nRet != 0) return 1;
	GfxDecode(128, 4, 32, 32, Bg2TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x800, DrvTempRom, DrvBg2Tiles);
	
	// Load and decode the bg tiles
	memset(DrvTempRom, 0, 0x40000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x18000,  8, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x28000, 10, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x38000, 12, 1); if (nRet != 0) return 1;
	GfxDecode(512, 4, 32, 32, BgTilePlaneOffsets, TileXOffsets, TileYOffsets, 0x800, DrvTempRom, DrvBgTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x40000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x18000, 18, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x28000, 20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 21, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x38000, 22, 1); if (nRet != 0) return 1;
	GfxDecode(2048, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load the Tilemaps
	nRet = BurnLoadRom(DrvBgTilemap,         23, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvBg2Tilemap,        24, 1); if (nRet != 0) return 1;
	
	// Load the PROMs
	nRet = BurnLoadRom(DrvPromRed,           25, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromGreen,         26, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBlue,          27, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromCharLookup,    28, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBgLookup,      29, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBgPalBank,     30, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBg2Lookup,     31, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBg2PalBank,    32, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromSpriteLookup,  33, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromSpritePalBank, 34, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);

	return nRet;
}

static INT32 DrvInit()
{
	return CommonInit(DrvLoad);
}

static INT32 DrvbLoad()
{
	INT32 nRet = 0;

	DrvTempRom = (UINT8 *)BurnMalloc(0x40000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000, 1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x20000, 2, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000, 3, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 4, 1); if (nRet != 0) return 1;
	GfxDecode(2048, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the bg2 tiles
	memset(DrvTempRom, 0, 0x40000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 10, 1); if (nRet != 0) return 1;
	GfxDecode(128, 4, 32, 32, Bg2TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x800, DrvTempRom, DrvBg2Tiles);
	
	// Load and decode the bg tiles
	memset(DrvTempRom, 0, 0x40000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000,  8, 1); if (nRet != 0) return 1;
	GfxDecode(512, 4, 32, 32, BgTilePlaneOffsets, TileXOffsets, TileYOffsets, 0x800, DrvTempRom, DrvBgTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x40000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x18000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x28000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x38000, 18, 1); if (nRet != 0) return 1;
	GfxDecode(2048, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load the Tilemaps
	nRet = BurnLoadRom(DrvBgTilemap,         19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvBg2Tilemap,        20, 1); if (nRet != 0) return 1;
	
	// Load the PROMs
	nRet = BurnLoadRom(DrvPromRed,           21, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromGreen,         22, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBlue,          23, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromCharLookup,    24, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBgLookup,      25, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBgPalBank,     26, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBg2Lookup,     27, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBg2PalBank,    28, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromSpriteLookup,  29, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromSpritePalBank, 30, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);
	
	return nRet;
}

static INT32 DrvbInit()
{
	bootleg = 1;
	return CommonInit(DrvbLoad);
}

static INT32 DrvbjLoad()
{
	INT32 nRet = 0;

	DrvTempRom = (UINT8 *)BurnMalloc(0x40000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000, 1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x20000, 2, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000, 3, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 4, 1); if (nRet != 0) return 1;
	GfxDecode(2048, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the bg2 tiles
	memset(DrvTempRom, 0, 0x40000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 10, 1); if (nRet != 0) return 1;
	GfxDecode(128, 4, 32, 32, Bg2TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x800, DrvTempRom, DrvBg2Tiles);

	memset(DrvTempRom, 0, 0x40000);
	UINT8 *pTemp = (UINT8*)BurnMalloc(0x40000);
	nRet = BurnLoadRom(pTemp + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(pTemp + 0x10000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(pTemp + 0x20000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(pTemp + 0x30000,  8, 1); if (nRet != 0) return 1;
	memcpy(DrvTempRom + 0x00000, pTemp + 0x00000, 0x8000);
	memcpy(DrvTempRom + 0x10000, pTemp + 0x08000, 0x8000);
	memcpy(DrvTempRom + 0x08000, pTemp + 0x10000, 0x8000);
	memcpy(DrvTempRom + 0x18000, pTemp + 0x18000, 0x8000);
	memcpy(DrvTempRom + 0x20000, pTemp + 0x20000, 0x8000);
	memcpy(DrvTempRom + 0x30000, pTemp + 0x28000, 0x8000);
	memcpy(DrvTempRom + 0x28000, pTemp + 0x30000, 0x8000);
	memcpy(DrvTempRom + 0x38000, pTemp + 0x38000, 0x8000);
	BurnFree(pTemp);
	GfxDecode(512, 4, 32, 32, BgTilePlaneOffsets, TileXOffsets, TileYOffsets, 0x800, DrvTempRom, DrvBgTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x40000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x18000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x28000, 16, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x38000, 18, 1); if (nRet != 0) return 1;
	GfxDecode(2048, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load the Tilemaps
	nRet = BurnLoadRom(DrvBgTilemap,         19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom,           20, 1); if (nRet != 0) return 1;
	memcpy(DrvBg2Tilemap + 0x6000, DrvTempRom + 0x0000, 0x2000);
	memcpy(DrvBg2Tilemap + 0x4000, DrvTempRom + 0x2000, 0x2000);
	memcpy(DrvBg2Tilemap + 0x2000, DrvTempRom + 0x4000, 0x2000);
	memcpy(DrvBg2Tilemap + 0x0000, DrvTempRom + 0x6000, 0x2000);
	
	// Load the PROMs
	nRet = BurnLoadRom(DrvPromRed,           21, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromGreen,         22, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBlue,          23, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromCharLookup,    24, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBgLookup,      25, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBgPalBank,     26, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBg2Lookup,     27, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBg2PalBank,    28, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromSpriteLookup,  29, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromSpritePalBank, 30, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);

	return nRet;
}

static INT32 DrvbjInit()
{
	bootleg = 1;

	return CommonInit(DrvbjLoad);
}

static INT32 Drvb2Load()
{
	INT32 nRet = 0;

	DrvTempRom = (UINT8 *)BurnMalloc(0x40000);

	// Load Z80 #1 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x00000, 0, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x10000, 1, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvZ80Rom1 + 0x20000, 2, 1); if (nRet != 0) return 1;
	
	// Load Z80 #2 Program Roms
	nRet = BurnLoadRom(DrvZ80Rom2 + 0x00000, 3, 1); if (nRet != 0) return 1;
	
	// Load and decode the chars
	nRet = BurnLoadRom(DrvTempRom, 4, 1); if (nRet != 0) return 1;
	GfxDecode(2048, 2, 8, 8, CharPlaneOffsets, CharXOffsets, CharYOffsets, 0x80, DrvTempRom, DrvChars);
	
	// Load and decode the bg2 tiles
	memset(DrvTempRom, 0, 0x40000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000,  9, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 10, 1); if (nRet != 0) return 1;
	GfxDecode(128, 4, 32, 32, Bg2TilePlaneOffsets, TileXOffsets, TileYOffsets, 0x800, DrvTempRom, DrvBg2Tiles);
	
	memset(DrvTempRom, 0, 0x40000);
	UINT8 *pTemp = (UINT8*)BurnMalloc(0x40000);
	nRet = BurnLoadRom(pTemp + 0x00000,  5, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(pTemp + 0x10000,  6, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(pTemp + 0x20000,  7, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(pTemp + 0x30000,  8, 1); if (nRet != 0) return 1;
	memcpy(DrvTempRom + 0x00000, pTemp + 0x00000, 0x8000);
	memcpy(DrvTempRom + 0x10000, pTemp + 0x08000, 0x8000);
	memcpy(DrvTempRom + 0x08000, pTemp + 0x10000, 0x8000);
	memcpy(DrvTempRom + 0x18000, pTemp + 0x18000, 0x8000);
	memcpy(DrvTempRom + 0x20000, pTemp + 0x20000, 0x8000);
	memcpy(DrvTempRom + 0x30000, pTemp + 0x28000, 0x8000);
	memcpy(DrvTempRom + 0x28000, pTemp + 0x30000, 0x8000);
	memcpy(DrvTempRom + 0x38000, pTemp + 0x38000, 0x8000);
	BurnFree(pTemp);
	GfxDecode(512, 4, 32, 32, BgTilePlaneOffsets, TileXOffsets, TileYOffsets, 0x800, DrvTempRom, DrvBgTiles);
	
	// Load and decode the sprites
	memset(DrvTempRom, 0, 0x40000);
	nRet = BurnLoadRom(DrvTempRom + 0x00000, 11, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x08000, 12, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x10000, 13, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x20000, 14, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x28000, 15, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom + 0x30000, 16, 1); if (nRet != 0) return 1;
	GfxDecode(2048, 4, 16, 16, SpritePlaneOffsets, SpriteXOffsets, SpriteYOffsets, 0x200, DrvTempRom, DrvSprites);
	
	// Load the Tilemaps
	nRet = BurnLoadRom(DrvBgTilemap,         17, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvTempRom,           18, 1); if (nRet != 0) return 1;
	memcpy(DrvBg2Tilemap + 0x6000, DrvTempRom + 0x0000, 0x2000);
	memcpy(DrvBg2Tilemap + 0x4000, DrvTempRom + 0x2000, 0x2000);
	memcpy(DrvBg2Tilemap + 0x2000, DrvTempRom + 0x4000, 0x2000);
	memcpy(DrvBg2Tilemap + 0x0000, DrvTempRom + 0x6000, 0x2000);
	
	// Load the PROMs
	nRet = BurnLoadRom(DrvPromRed,           19, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromGreen,         20, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBlue,          21, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromCharLookup,    22, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBgLookup,      23, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBgPalBank,     24, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBg2Lookup,     25, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromBg2PalBank,    26, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromSpriteLookup,  27, 1); if (nRet != 0) return 1;
	nRet = BurnLoadRom(DrvPromSpritePalBank, 28, 1); if (nRet != 0) return 1;
	
	BurnFree(DrvTempRom);

	return nRet;
}

static INT32 Drvb2Init()
{
	bootleg = 1;

	return CommonInit(Drvb2Load);
}

static INT32 DrvExit()
{
	ZetExit();
	BurnYM2203Exit();
	
	GenericTilesExit();
	
	DrvRomBank = 0;
	DrvSoundLatch = 0;
	DrvBg2ScrollX[0] = 0;
	DrvBg2ScrollX[1] = 0;
	DrvBgScrollX[0] = 0;
	DrvBgScrollX[1] = 0;
	DrvBgScrollY = 0;
	DrvBg2On = 0;
	DrvBg1On = 0;
	DrvSpritesOn = 0;
	DrvCharsOn = 0;
	DrvProtValue = 0;
	bootleg = 0;

	BurnFree(Mem);

	return 0;
}

static void DrvCalcPalette()
{
	INT32 i;
	UINT32 Palette[256];
	
	for (i = 0; i < 256; i++) {
		INT32 bit0, bit1, bit2, bit3, r, g, b;
		
		bit0 = (DrvPromRed[i] >> 0) & 0x01;
		bit1 = (DrvPromRed[i] >> 1) & 0x01;
		bit2 = (DrvPromRed[i] >> 2) & 0x01;
		bit3 = (DrvPromRed[i] >> 3) & 0x01;
		r = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		
		bit0 = (DrvPromGreen[i] >> 0) & 0x01;
		bit1 = (DrvPromGreen[i] >> 1) & 0x01;
		bit2 = (DrvPromGreen[i] >> 2) & 0x01;
		bit3 = (DrvPromGreen[i] >> 3) & 0x01;
		g = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		
		bit0 = (DrvPromBlue[i] >> 0) & 0x01;
		bit1 = (DrvPromBlue[i] >> 1) & 0x01;
		bit2 = (DrvPromBlue[i] >> 2) & 0x01;
		bit3 = (DrvPromBlue[i] >> 3) & 0x01;
		b = 0x0e * bit0 + 0x1f * bit1 + 0x43 * bit2 + 0x8f * bit3;
		
		Palette[i] = BurnHighCol(r, g, b, 0);
	}
	
	for (i = 0; i < 0x80; i++) {
		DrvPalette[i] = Palette[(DrvPromCharLookup[i] & 0x0f) | 0x40];
	}
	
	for (i = 0x80; i < 0x180; i++) {
		DrvPalette[i] = Palette[((DrvPromBgPalBank[i - 0x80] & 0x03) << 4) | (DrvPromBgLookup[i - 0x80] & 0x0f)];
	}
	
	for (i = 0x180; i < 0x280; i++) {
		DrvPalette[i] = Palette[((DrvPromBg2PalBank[i - 0x180] & 0x03) << 4) | (DrvPromBg2Lookup[i - 0x180] & 0x0f)];
	}
	
	for (i = 0x280; i < 0x380; i++) {
		DrvPalette[i] = Palette[((DrvPromSpritePalBank[i - 0x280] & 0x07) << 4) | (DrvPromSpriteLookup[i - 0x280] & 0x0f) | 0x80];
	}
}

static void DrvRenderSprites(INT32 Priority)
{	
	for (INT32 Offs = 0x1000 - 32; Offs >= 0; Offs -= 32) {
		INT32 Attr = DrvSpriteRam[Offs + 1];
		INT32 Code = DrvSpriteRam[Offs + 0] + ((Attr & 0xe0) << 3);
		INT32 Colour = Attr & 0x0f;
		INT32 sx = DrvSpriteRam[Offs + 3] - ((Attr & 0x10) << 4);
		INT32 sy = DrvSpriteRam[Offs + 2];
		
		sy -= 16;
		
		if (Priority) {
			if (Colour != 0x0a && Colour != 0x0b) {
				if (sx > 0 && sx < 240 && sy > 0 && sy < 208) {
					Render16x16Tile_Mask(pTransDraw, Code, sx, sy, Colour + 0x28, 4, 0, 0, DrvSprites);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, Code, sx, sy, Colour + 0x28, 4, 0, 0, DrvSprites);
				}
			}
		} else {
			if (Colour == 0x0a || Colour == 0x0b) {
				if (sx > 0 && sx < 240 && sy > 0 && sy < 208) {
					Render16x16Tile_Mask(pTransDraw, Code, sx, sy, Colour + 0x28, 4, 0, 0, DrvSprites);
				} else {
					Render16x16Tile_Mask_Clip(pTransDraw, Code, sx, sy, Colour + 0x28, 4, 0, 0, DrvSprites);
				}
			}
		
		}
	}
}

static INT32 DrvDraw()
{
	if (DrvRecalc) {
		DrvCalcPalette();
		DrvRecalc = 0;
	}

	GenericTilemapSetScrollX(0, DrvBg2ScrollX[0] + (256 * DrvBg2ScrollX[1]));
	GenericTilemapSetScrollX(1, DrvBgScrollX[0] + (256 * DrvBgScrollX[1]));
	GenericTilemapSetScrollY(1, DrvBgScrollY);

	if (DrvBg2On && nBurnLayer & 1) GenericTilemapDraw(0, pTransDraw, 0);
	else BurnTransferClear();

	if (DrvSpritesOn) DrvRenderSprites(0);

	if (DrvBg1On && nBurnLayer & 2) GenericTilemapDraw(1, pTransDraw, 0);

	if (DrvSpritesOn) DrvRenderSprites(1);

	if (DrvCharsOn && nBurnLayer & 4) GenericTilemapDraw(2, pTransDraw, 0);

	BurnTransferCopy(DrvPalette);

	return 0;
}

static inline void DrvClearOpposites(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x00) {
		*nJoystickInputs |= 0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x00) {
		*nJoystickInputs |= 0x0c;
	}
}

static inline void DrvMakeInputs()
{
	// Reset Inputs
	DrvInput[0] = DrvInput[1] = DrvInput[2] = 0xff;

	// Compile Digital Inputs
	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] ^= (DrvInputPort0[i] & 1) << i;
		DrvInput[1] ^= (DrvInputPort1[i] & 1) << i;
		DrvInput[2] ^= (DrvInputPort2[i] & 1) << i;
	}

	// Clear Opposites
	DrvClearOpposites(&DrvInput[1]);
	DrvClearOpposites(&DrvInput[2]);
}

static INT32 DrvFrame()
{
	BurnWatchdogUpdate();

	if (DrvReset) {
		DrvDoReset(1);
	}

	DrvMakeInputs();

	INT32 nInterleave = 256;
	INT32 nCyclesTotal[2] = { 6000000 / 60, 3000000 / 60 };
	INT32 nCyclesDone[2] = { 0, 0 };
	INT32 nCyclesSegment;

	ZetNewFrame();

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run Z80 #1
		nCurrentCPU = 0;
		ZetOpen(nCurrentCPU);
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;
		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += ZetRun(nCyclesSegment);
		if (i == 240) ZetSetIRQLine(0, CPU_IRQSTATUS_HOLD);
		ZetClose();

		// Run Z80 #2
		nCurrentCPU = 1;
		ZetOpen(nCurrentCPU);
		BurnTimerUpdate((i + 1) * (nCyclesTotal[1] / nInterleave));
		// execute IRQ quarterly
		if (i%(nInterleave/4) == (nInterleave/4)-1) ZetSetIRQLine(0, CPU_IRQSTATUS_HOLD);
		ZetClose();
	}
	
	ZetOpen(1);
	BurnTimerEndFrame(nCyclesTotal[1]);
	ZetClose();
	
	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		ZetOpen(1);
		BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		ZetClose();
	}
	
	if (pBurnDraw) {
		DrvDraw();
	}

	return 0;
}

static INT32 DrvScan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;
	
	if (pnMin != NULL) {
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
		ZetScan(nAction);

		BurnWatchdogScan(nAction);

		BurnYM2203Scan(nAction, pnMin);

		SCAN_VAR(DrvSoundLatch);
		SCAN_VAR(DrvRomBank);
		SCAN_VAR(DrvBg2ScrollX[0]);
		SCAN_VAR(DrvBg2ScrollX[1]);
		SCAN_VAR(DrvBgScrollX[0]);
		SCAN_VAR(DrvBgScrollX[1]);
		SCAN_VAR(DrvBgScrollY);
		SCAN_VAR(DrvBg2On);
		SCAN_VAR(DrvBg1On);
		SCAN_VAR(DrvSpritesOn);
		SCAN_VAR(DrvCharsOn);
		SCAN_VAR(DrvProtValue);
	}

	if (nAction & ACB_WRITE) {
		ZetOpen(0);
		ZetMapMemory(DrvZ80Rom1 + 0x10000 + (DrvRomBank * 0x1000), 0x8000, 0xbfff, MAP_ROM);
		ZetClose();
	}

	return 0;
}

struct BurnDriver BurnDrvNineteen43 = {
	"1943", NULL, NULL, NULL, "1987",
	"1943: The Battle of Midway (Euro)\0", NULL, "Capcom", "Miscellaneous",
	L"1943 - \u4E2D\u9014\u5C9B\u6D77\u6218 (\u6B27\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvRomInfo, DrvRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x380, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen43u = {
	"1943u", "1943", NULL, NULL, "1987",
	"1943: The Battle of Midway (US, Rev C)\0", NULL, "Capcom", "Miscellaneous",
	L"1943 - \u4E2D\u9014\u5C9B\u6D77\u6218 (\u7F8E\u7248, \u4FEE\u8BA2\u7248 C)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvuRomInfo, DrvuRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x380, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen43ua = {
	"1943ua", "1943", NULL, NULL, "1987",
	"1943: The Battle of Midway (US)\0", NULL, "Capcom", "Miscellaneous",
	L"1943 - \u4E2D\u9014\u5C9B\u6D77\u6218 (\u7F8E\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvuaRomInfo, DrvuaRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x380, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen43j = {
	"1943j", "1943", NULL, NULL, "1987",
	"1943: Midway Kaisen (Japan, Rev B)\0", NULL, "Capcom", "Miscellaneous",
	L"1943 - \u4E2D\u9014\u5C9B\u6D77\u6218 (\u65E5\u7248, \u4FEE\u8BA2\u7248 B)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvjRomInfo, DrvjRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x380, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen43ja = {
	"1943ja", "1943", NULL, NULL, "1987",
	"1943: Midway Kaisen (Japan)\0", NULL, "Capcom", "Miscellaneous",
	L"1943 - \u4E2D\u9014\u5C9B\u6D77\u6218 (\u65E5\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvjaRomInfo, DrvjaRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x380, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen43jah = {
	"1943jah", "1943", NULL, NULL, "1987",
	"1943: Midway Kaisen (Japan, no protection hack)\0", NULL, "Capcom", "Miscellaneous",
	L"1943 - \u4E2D\u9014\u5C9B\u6D77\u6218 (\u65E5\u7248, \u672A\u4FDD\u62A4 Hack)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvjahRomInfo, DrvjahRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x380, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen43jaha = {
	"1943jaha", "1943", NULL, NULL, "1987",
	"1943: Midway Kaisen (Japan, no protection hack, alt)\0", NULL, "Capcom", "Miscellaneous",
	L"1943 - \u4E2D\u9014\u5C9B\u6D77\u6218 (\u65E5\u7248, \u672A\u4FDD\u62A4 \u66FF\u6362\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvjahaRomInfo, DrvjahaRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x380, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen43b = {
	"1943b", "1943", NULL, NULL, "1987",
	"1943: Battle of Midway (bootleg, hack of Japan set, set 1)\0", NULL, "bootleg", "Miscellaneous",
	L"1943 - \u4E2D\u9014\u5C9B\u6D77\u6218 (\u76D7\u7248 \u7B2C\u4E00\u5957, \u65E5\u7248\u7684Hack)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvbRomInfo, DrvbRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvbInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x380, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen43b2 = {
	"1943b2", "1943", NULL, NULL, "1987",
	"1943: Battle of Midway (bootleg, hack of Japan set, set 2)\0", NULL, "bootleg", "Miscellaneous",
	L"1943 - \u4E2D\u9014\u5C9B\u6D77\u6218 (\u76D7\u7248 \u7B2C\u4E8C\u5957, \u65E5\u7248\u7684Hack)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, Drvb2RomInfo, Drvb2RomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	Drvb2Init, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x380, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen43bj = {
	"1943bj", "1943", NULL, NULL, "1987",
	"1943: Midway Kaisen (bootleg)\0", NULL, "bootleg", "Miscellaneous",
	L"1943 - \u4E2D\u9014\u5C9B\u6D77\u6218 (\u76D7\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvbjRomInfo, DrvbjRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvbjInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x380, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen43kai = {
	"1943kai", NULL, NULL, NULL, "1987",
	"1943 Kai: Midway Kaisen (Japan)\0", NULL, "Capcom", "Miscellaneous",
	L"1943 \u6539 - \u4E2D\u9014\u5C9B\u6D77\u6218 (\u65E5\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvkaiRomInfo, DrvkaiRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x380, 224, 256, 3, 4
};

struct BurnDriver BurnDrvNineteen43mii = {
	"1943mii", NULL, NULL, NULL, "1987",
	"1943: The Battle of Midway Mark II (US)\0", NULL, "Capcom", "Miscellaneous",
	L"1943 - \u4E2D\u9014\u5C9B\u6D77\u6218 Mark II (\u7F8E\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARWARE_CAPCOM_MISC, GBF_VERSHOOT, 0,
	NULL, DrvmiiRomInfo, DrvmiiRomName, NULL, NULL, NULL, NULL, DrvInputInfo, DrvDIPInfo,
	DrvInit, DrvExit, DrvFrame, DrvDraw, DrvScan,
	&DrvRecalc, 0x380, 224, 256, 3, 4
};
