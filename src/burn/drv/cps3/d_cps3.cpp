// ------------------------------------------------------------------------------------
// Capcom Play System III Drivers for FB Alpha (2007 - 2008).
// ------------------------------------------------------------------------------------
//     Based on MAME driver by David Haywood, Andreas Naive, El Semi, Philip Bennett,
//	and Tomasz Slanina
//
//	v1	[ OopsWare ]
//     - Original drivers release.
//
//	v2  [ CaptainCPS-X ]
//     - Verified drivers.
//     - Updated DIPs.
//     - Updated some Inits.
//     - Added some Comments.
//
//  v3 [ BisonSAS ]
//     - Added default game regions DIPs.
//     - Added unicode titles for "jojo" and "jojoba".
//     - Changed the redeartn BIOS to "warzard_euro.29f400.u2".
//     - Added "HARDWARE_CAPCOM_CPS3_NO_CD" flag for NOCD sets.
//
//  v4 [ CaptainCPS-X ]
//     - Updated comments & organized structures of code.
//     - Revised code for compatibility with FB Alpha Enhanced.
//
//	More info: https://neo-source.com/
//
// ------------------------------------------------------------------------------------

#include "cps3.h"

#define LESS_ACCURATE 0

static struct BurnInputInfo cps3InputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	Cps3But2 +  8,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	Cps3But2 + 12,	"p1 start"	},

	{"P1 Up",			BIT_DIGITAL,	Cps3But1 +  0,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	Cps3But1 +  1,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	Cps3But1 +  2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	Cps3But1 +  3,	"p1 right"	},
	{"P1 Weak Punch",	BIT_DIGITAL,	Cps3But1 +  4,	"p1 fire 1"	},
	{"P1 Medium Punch",	BIT_DIGITAL,	Cps3But1 +  5,	"p1 fire 2"	},
	{"P1 Strong Punch",	BIT_DIGITAL,	Cps3But1 +  6,	"p1 fire 3"	},
	{"P1 Weak Kick",	BIT_DIGITAL,	Cps3But3 +  3,	"p1 fire 4"	},
	{"P1 Medium Kick",	BIT_DIGITAL,	Cps3But3 +  2,	"p1 fire 5"	},
	{"P1 Strong Kick",	BIT_DIGITAL,	Cps3But3 +  1,	"p1 fire 6"	},

	{"P2 Coin",			BIT_DIGITAL,	Cps3But2 +  9,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	Cps3But2 + 13,	"p2 start"	},

	{"P2 Up",			BIT_DIGITAL,	Cps3But1 +  8,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	Cps3But1 +  9,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	Cps3But1 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	Cps3But1 + 11,	"p2 right"	},
	{"P2 Weak Punch",	BIT_DIGITAL,	Cps3But1 + 12,	"p2 fire 1"	},
	{"P2 Medium Punch",	BIT_DIGITAL,	Cps3But1 + 13,	"p2 fire 2"	},
	{"P2 Strong Punch",	BIT_DIGITAL,	Cps3But1 + 14,	"p2 fire 3"	},
	{"P2 Weak Kick",	BIT_DIGITAL,	Cps3But3 +  4,	"p2 fire 4"	},
	{"P2 Medium Kick",	BIT_DIGITAL,	Cps3But3 +  5,	"p2 fire 5"	},
	{"P2 Strong Kick",	BIT_DIGITAL,	Cps3But2 + 10,	"p2 fire 6"	},

	{"Reset",			BIT_DIGITAL,	&cps3_reset,	"reset"		},
	{"Diagnostic",		BIT_DIGITAL,	Cps3But2 +  1,	"diag"		},
	{"Service",			BIT_DIGITAL,	Cps3But2 +  0,	"service"	},
	{"Region",			BIT_DIPSWITCH,	&cps3_dip,		"dip"		},
	{"Fake Dip",		BIT_DIPSWITCH,	&cps3_fake_dip,	"dip"		},
};

STDINPUTINFO(cps3)

static struct BurnInputInfo jojoInputList[] = {
	{"P1 Coin",			BIT_DIGITAL,	Cps3But2 +  8,	"p1 coin"	},
	{"P1 Start",		BIT_DIGITAL,	Cps3But2 + 12,	"p1 start"	},

	{"P1 Up",			BIT_DIGITAL,	Cps3But1 +  0,	"p1 up"		},
	{"P1 Down",			BIT_DIGITAL,	Cps3But1 +  1,	"p1 down"	},
	{"P1 Left",			BIT_DIGITAL,	Cps3But1 +  2,	"p1 left"	},
	{"P1 Right",		BIT_DIGITAL,	Cps3But1 +  3,	"p1 right"	},
	{"P1 Weak Attack",	BIT_DIGITAL,	Cps3But1 +  4,	"p1 fire 1"	},
	{"P1 Medium Attack",BIT_DIGITAL,	Cps3But1 +  5,	"p1 fire 2"	},
	{"P1 Strong Attack",BIT_DIGITAL,	Cps3But1 +  6,	"p1 fire 3"	},
	{"P1 Stand",		BIT_DIGITAL,	Cps3But3 +  3,	"p1 fire 4"	},
	{"P1 All Attacks (Fake)",	BIT_DIGITAL,	Cps3But3 +  2,	"p1 fire 5"	},
	{"P1 Not in use 2",	BIT_DIGITAL,	Cps3But3 +  1,	"p1 fire 6"	},

	{"P2 Coin",			BIT_DIGITAL,	Cps3But2 +  9,	"p2 coin"	},
	{"P2 Start",		BIT_DIGITAL,	Cps3But2 + 13,	"p2 start"	},

	{"P2 Up",			BIT_DIGITAL,	Cps3But1 +  8,	"p2 up"		},
	{"P2 Down",			BIT_DIGITAL,	Cps3But1 +  9,	"p2 down"	},
	{"P2 Left",			BIT_DIGITAL,	Cps3But1 + 10,	"p2 left"	},
	{"P2 Right",		BIT_DIGITAL,	Cps3But1 + 11,	"p2 right"	},
	{"P2 Weak Attack",	BIT_DIGITAL,	Cps3But1 + 12,	"p2 fire 1"	},
	{"P2 Medium Attack",BIT_DIGITAL,	Cps3But1 + 13,	"p2 fire 2"	},
	{"P2 Strong Attack",BIT_DIGITAL,	Cps3But1 + 14,	"p2 fire 3"	},
	{"P2 Stand",		BIT_DIGITAL,	Cps3But3 +  4,	"p2 fire 4"	},
	{"P2 All Attacks (Fake)",	BIT_DIGITAL,	Cps3But3 +  5,	"p2 fire 5"	},
	{"P2 Not in use 2",	BIT_DIGITAL,	Cps3But2 + 10,	"p2 fire 6"	},

	{"Reset",			BIT_DIGITAL,	&cps3_reset,	"reset"		},
	{"Diagnostic",		BIT_DIGITAL,	Cps3But2 +  1,	"diag"		},
	{"Service",			BIT_DIGITAL,	Cps3But2 +  0,	"service"	},
	{"Region",			BIT_DIPSWITCH,	&cps3_dip,		"dip"		},
	{"Fake Dip",		BIT_DIPSWITCH,	&cps3_fake_dip,	"dip"		},
};

STDINPUTINFO(jojo)

// ------------------------------------------------------------------------------------

static struct BurnDIPInfo regionDIPList[] = {

	// Region
	{0,		0xFD, 0,	9,	  "Region"},
	{0x1B,	0x01, 0xFF,	0x01, "Japan"},
	{0x1B,	0x01, 0xFF,	0x02, "Asia"},
	{0x1B,	0x01, 0xFF,	0x03, "Euro"},
	{0x1B,	0x01, 0xFF,	0x04, "USA"},
	{0x1B,	0x01, 0xFF,	0x05, "Hispanic"},
	{0x1B,	0x01, 0xFF,	0x06, "Brazil"},
	{0x1B,	0x01, 0xFF,	0x07, "Oceania"},
	{0x1B,	0x01, 0xFF,	0x08, "Asia"},
	{0x1B,	0x01, 0xFF,	0x00, "XXXXXX"},

	{0,		0xFE, 0,	2,	  "Less sprite lag"},
	{0x1C,	0x01, 0x01,	0x00, "No"},
	{0x1C,	0x01, 0x01,	0x01, "Yes"},

//	{0,		0xFE, 0,	2,		"NO CD"},
//	{0x1B,	0x01, 0x10, 0x00,	"No"},
//	{0x1B,	0x01, 0x10, 0x10,	"Yes"},
};

static struct BurnDIPInfo jojobaRegionDIPList[] = {

	// Region
	{0,		0xFD, 0,	9,	  "Region"},
	{0x1B,	0x01, 0xFF,	0x01, "Japan"},
	{0x1B,	0x01, 0xFF,	0x02, "Asia"},
	{0x1B,	0x01, 0xFF,	0x03, "Euro"},
	{0x1B,	0x01, 0xFF,	0x04, "USA"},
	{0x1B,	0x01, 0xFF,	0x05, "Hispanic"},
	{0x1B,	0x01, 0xFF,	0x06, "Brazil"},
	{0x1B,	0x01, 0xFF,	0x07, "Oceania"},
	{0x1B,	0x01, 0xFF,	0x08, "Korea"}, // fake region?
	{0x1B,	0x01, 0xFF,	0x00, "XXXXXX"},

	{0,		0xFE, 0,	2,	  "Less sprite lag"},
	{0x1C,	0x01, 0x01,	0x00, "No"},
	{0x1C,	0x01, 0x01,	0x01, "Yes"},
};

static struct BurnDIPInfo redeartnRegionDIPList[] = {

	// Region
	{0,		0xFD, 0,	9,	  "Region"},
	{0x1B,	0x01, 0xFF,	0x51, "Japan"},
	{0x1B,	0x01, 0xFF,	0x52, "Asia"},
	{0x1B,	0x01, 0xFF,	0x53, "Euro"},
	{0x1B,	0x01, 0xFF,	0x54, "USA"},
	{0x1B,	0x01, 0xFF,	0x55, "Hispanic"},
	{0x1B,	0x01, 0xFF,	0x56, "Brazil"},
	{0x1B,	0x01, 0xFF,	0x57, "Oceania"},
	{0x1B,	0x01, 0xFF,	0x58, "Asia"},
	{0x1B,	0x01, 0xFF,	0x50, "Japan"},

	{0,		0xFE, 0,	2,	  "Less sprite lag"},
	{0x1C,	0x01, 0x01,	0x00, "No"},
	{0x1C,	0x01, 0x01,	0x01, "Yes"},
};

static struct BurnDIPInfo sfiiiwideDIPList[] = {

	// Region
	{0,		0xFD, 0,	9,	  "Region"},
	{0x1B,	0x01, 0x0F,	0x01, "Japan"},
	{0x1B,	0x01, 0x0F,	0x02, "Asia"},
	{0x1B,	0x01, 0x0F,	0x03, "Euro"},
	{0x1B,	0x01, 0x0F,	0x04, "USA"},
	{0x1B,	0x01, 0x0F,	0x05, "Hispanic"},
	{0x1B,	0x01, 0x0F,	0x06, "Brazil"},
	{0x1B,	0x01, 0x0F,	0x07, "Oceania"},
	{0x1B,	0x01, 0x0F,	0x08, "Asia"},
	{0x1B,	0x01, 0x0F,	0x00, "XXXXXX"},
	
	{0,		0xFD, 0,	2,	  "Screen mode"},
	{0x1B,	0x01, 0x80,	0x80, "Wide"},
	{0x1B,	0x01, 0x80,	0x00, "Normal"},

	{0,		0xFE, 0,	2,	  "Less sprite lag"},
	{0x1C,	0x01, 0x01,	0x00, "No"},
	{0x1C,	0x01, 0x01,	0x01, "Yes"},

//	{0,		0xFE, 0,	2,		"NO CD"},
//	{0x1B,	0x01, 0x10, 0x00,	"No"},
//	{0x1B,	0x01, 0x10, 0x10,	"Yes"},
};

static struct BurnDIPInfo japanRegionDIPList[] = {

	// Defaults
	{0x1B,	0xFF, 0xFF,	0x01, NULL},
#if LESS_ACCURATE
	{0x1C,  0xFF, 0xFF, 0x01, NULL},
#else
	{0x1C,  0xFF, 0xFF, 0x00, NULL},
#endif
};

static struct BurnDIPInfo asiaRegionDIPList[] = {

	// Defaults
	{0x1B,	0xFF, 0xFF,	0x02, NULL},
#if LESS_ACCURATE
	{0x1C,  0xFF, 0xFF, 0x01, NULL},
#else
	{0x1C,  0xFF, 0xFF, 0x00, NULL},
#endif
};

static struct BurnDIPInfo euroRegionDIPList[] = {

	// Defaults
	{0x1B,	0xFF, 0xFF,	0x03, NULL},
#if LESS_ACCURATE
	{0x1C,  0xFF, 0xFF, 0x01, NULL},
#else
	{0x1C,  0xFF, 0xFF, 0x00, NULL},
#endif
};

static struct BurnDIPInfo usaRegionDIPList[] = {

	// Defaults
	{0x1B,	0xFF, 0xFF,	0x04, NULL},
#if LESS_ACCURATE
	{0x1C,  0xFF, 0xFF, 0x01, NULL},
#else
	{0x1C,  0xFF, 0xFF, 0x00, NULL},
#endif
};

static struct BurnDIPInfo hispanicRegionDIPList[] = {

	// Defaults
	{0x1B,	0xFF, 0xFF,	0x05, NULL},
#if LESS_ACCURATE
	{0x1C,  0xFF, 0xFF, 0x01, NULL},
#else
	{0x1C,  0xFF, 0xFF, 0x00, NULL},
#endif
};

static struct BurnDIPInfo euroRedeartnDIPList[] = {

	// Defaults
	{0x1B,	0xFF, 0xFF,	0x53, NULL},
#if LESS_ACCURATE
	{0x1C,  0xFF, 0xFF, 0x01, NULL},
#else
	{0x1C,  0xFF, 0xFF, 0x00, NULL},
#endif
};

static struct BurnDIPInfo asiaRedeartnDIPList[] = {

	// Defaults
	{0x1B,	0xFF, 0xFF,	0x58, NULL},
#if LESS_ACCURATE
	{0x1C,  0xFF, 0xFF, 0x01, NULL},
#else
	{0x1C,  0xFF, 0xFF, 0x00, NULL},
#endif
};

static struct BurnDIPInfo japanwarzardDIPList[] = {

	// Defaults
	{0x1B,	0xFF, 0xFF,	0x51, NULL},
#if LESS_ACCURATE
	{0x1C,  0xFF, 0xFF, 0x01, NULL},
#else
	{0x1C,  0xFF, 0xFF, 0x00, NULL},
#endif
};

STDDIPINFOEXT(japan, region, japanRegion)
STDDIPINFOEXT(asia, region, asiaRegion)
STDDIPINFOEXT(euro, region, euroRegion)
STDDIPINFOEXT(usa, region, usaRegion)
STDDIPINFOEXT(jojoba, jojobaRegion, japanRegion)
STDDIPINFOEXT(jojobane, jojobaRegion, euroRegion)
STDDIPINFOEXT(redearth, redeartnRegion, euroRedeartn)
STDDIPINFOEXT(redearthn, redeartnRegion, asiaRedeartn)
STDDIPINFOEXT(warzard, redeartnRegion, japanwarzard)
STDDIPINFOEXT(sfiiiwidehispanic, sfiiiwide, hispanicRegion)
STDDIPINFOEXT(sfiiiwidejapan, sfiiiwide, japanRegion)
STDDIPINFOEXT(sfiiiwideasia, sfiiiwide, asiaRegion)
STDDIPINFOEXT(sfiiiwideusa, sfiiiwide, usaRegion)
STDDIPINFOEXT(sfiiiwideeuro, sfiiiwide, euroRegion)

// -------------------------------------------
// Street Fighter III: New Generation (970204)
// -------------------------------------------

#define SFIII_970204_FLASH \
	{ "sfiii-simm1.0",			0x200000, 0xcfc9e45a, BRF_ESS | BRF_PRG }, \
	{ "sfiii-simm1.1",			0x200000, 0x57920546, BRF_ESS | BRF_PRG }, \
	{ "sfiii-simm1.2",			0x200000, 0x0d8f2680, BRF_ESS | BRF_PRG }, \
	{ "sfiii-simm1.3",			0x200000, 0xea4ca054, BRF_ESS | BRF_PRG }, \
	{ "sfiii-simm3.0",			0x200000, 0x080b3bd3, BRF_GRA }, \
	{ "sfiii-simm3.1",			0x200000, 0x5c356f2f, BRF_GRA }, \
	{ "sfiii-simm3.2",			0x200000, 0xf9c97a45, BRF_GRA }, \
	{ "sfiii-simm3.3",			0x200000, 0x09de3ead, BRF_GRA }, \
	{ "sfiii-simm3.4",			0x200000, 0x7dd7e1f3, BRF_GRA }, \
	{ "sfiii-simm3.5",			0x200000, 0x47a03a3a, BRF_GRA }, \
	{ "sfiii-simm3.6",			0x200000, 0xe9eb7a26, BRF_GRA }, \
	{ "sfiii-simm3.7",			0x200000, 0x7f44395c, BRF_GRA }, \
	{ "sfiii-simm4.0",			0x200000, 0x9ac080fc, BRF_GRA }, \
	{ "sfiii-simm4.1",			0x200000, 0x6e2c4c94, BRF_GRA }, \
	{ "sfiii-simm4.2",			0x200000, 0x8afc22d4, BRF_GRA }, \
	{ "sfiii-simm4.3",			0x200000, 0x9f3873b8, BRF_GRA }, \
	{ "sfiii-simm4.4",			0x200000, 0x166b3c97, BRF_GRA }, \
	{ "sfiii-simm4.5",			0x200000, 0xe5ea2547, BRF_GRA }, \
	{ "sfiii-simm4.6",			0x200000, 0xe85b9fdd, BRF_GRA }, \
	{ "sfiii-simm4.7",			0x200000, 0x362c01b7, BRF_GRA }, \
	{ "sfiii-simm5.0",			0x200000, 0x9bc108b2, BRF_GRA }, \
	{ "sfiii-simm5.1",			0x200000, 0xc6f1c066, BRF_GRA },

// -----------------------------------------------
// Street Fighter III: New Generation (Europe 970204)
// -----------------------------------------------
static struct BurnRomInfo sfiiiRomDesc[] = {

	{ "sfiii_euro.29f400.u2",				0x080000, 0x27699ddc, BRF_ESS | BRF_BIOS },	// SH-2 Bios

#if !defined ROM_VERIFY
	SFIII_970204_FLASH
#endif
};

STD_ROM_PICK(sfiii)
STD_ROM_FN(sfiii)
	
// -----------------------------------------------
// Street Fighter III: New Generation (USA 970204)
// -----------------------------------------------
static struct BurnRomInfo sfiiiuRomDesc[] = {

	{ "sfiii_usa_region_b1.29f400.u2", 		0x080000, 0xfb172a8e, BRF_ESS | BRF_BIOS },	// SH-2 Bios

#if !defined ROM_VERIFY
	SFIII_970204_FLASH
#endif
};

STD_ROM_PICK(sfiiiu)
STD_ROM_FN(sfiiiu)

// -----------------------------------------------
// Street Fighter III: New Generation (Asia 970204)
// -----------------------------------------------
static struct BurnRomInfo sfiiiaRomDesc[] = {

	{ "sfiii_asia_region_bd.29f400.u2", 	0x080000, 0xcbd28de7, BRF_ESS | BRF_BIOS },	// SH-2 Bios

#if !defined ROM_VERIFY
	SFIII_970204_FLASH
#endif
};

STD_ROM_PICK(sfiiia)
STD_ROM_FN(sfiiia)

// -------------------------------------------------
// Street Fighter III: New Generation (Japan 970204)
// -------------------------------------------------
static struct BurnRomInfo sfiiijRomDesc[] = {

	{ "sfiii_japan.29f400.u2",				0x080000, 0x74205250, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	SFIII_970204_FLASH
#endif
};

STD_ROM_PICK(sfiiij)
STD_ROM_FN(sfiiij)

// ----------------------------------------------------
// Street Fighter III: New Generation (Hispanic 970204)
// ----------------------------------------------------
static struct BurnRomInfo sfiiihRomDesc[] = {

	{ "sfiii_hispanic.29f400.u2",			0x080000, 0xd2b3cd48, BRF_ESS | BRF_BIOS },	// SH-2 Bios

#if !defined ROM_VERIFY
	SFIII_970204_FLASH
#endif
};

STD_ROM_PICK(sfiiih)
STD_ROM_FN(sfiiih)

// -------------------------------------------------------------------
// Street Fighter III: New Generation (Asia 970204, NO CD, BIOS set 1)
// -------------------------------------------------------------------
static struct BurnRomInfo sfiiinRomDesc[] = {

	{ "sfiii_asia_nocd.29f400.u2",			0x080000, 0xca2b715f, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	SFIII_970204_FLASH
};

STD_ROM_PICK(sfiiin)
STD_ROM_FN(sfiiin)

// -------------------------------------------------------------------
// Street Fighter III: New Generation (Asia 970204, NO CD, BIOS set 2)
// -------------------------------------------------------------------
static struct BurnRomInfo sfiiinaRomDesc[] = {

	{ "sfiii_asia_nocd.29f400.u2",			0x080000, 0x73e32463, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	SFIII_970204_FLASH
};

STD_ROM_PICK(sfiiina)
STD_ROM_FN(sfiiina)

// ----------------------------------------------------
// Street Fighter III 2nd Impact: Giant Attack (970930)
// ----------------------------------------------------

#define SFIII2_970930_FLASH \
	{ "sfiii2-simm1.0",			0x200000, 0x2d666f0b, BRF_ESS | BRF_PRG }, \
	{ "sfiii2-simm1.1",			0x200000, 0x2a3a8ef6, BRF_ESS | BRF_PRG }, \
	{ "sfiii2-simm1.2",			0x200000, 0x161d2206, BRF_ESS | BRF_PRG }, \
	{ "sfiii2-simm1.3",			0x200000, 0x87ded8a3, BRF_ESS | BRF_PRG }, \
	{ "sfiii2-simm2.0",			0x200000, 0x94a4ce0f, BRF_ESS | BRF_PRG }, \
	{ "sfiii2-simm2.1",			0x200000, 0x67585033, BRF_ESS | BRF_PRG }, \
	{ "sfiii2-simm2.2",			0x200000, 0xfabffcd5, BRF_ESS | BRF_PRG }, \
	{ "sfiii2-simm2.3",			0x200000, 0x623c09ca, BRF_ESS | BRF_PRG }, \
	{ "sfiii2-simm3.0",			0x200000, 0xdab2d766, BRF_GRA }, \
	{ "sfiii2-simm3.1",			0x200000, 0x1f2aa34b, BRF_GRA }, \
	{ "sfiii2-simm3.2",			0x200000, 0x6f1a04eb, BRF_GRA }, \
	{ "sfiii2-simm3.3",			0x200000, 0xe05ef205, BRF_GRA }, \
	{ "sfiii2-simm3.4",			0x200000, 0xaffb074f, BRF_GRA }, \
	{ "sfiii2-simm3.5",			0x200000, 0x6962872e, BRF_GRA }, \
	{ "sfiii2-simm3.6",			0x200000, 0x6eed87de, BRF_GRA }, \
	{ "sfiii2-simm3.7",			0x200000, 0xe18f479e, BRF_GRA }, \
	{ "sfiii2-simm4.0",			0x200000, 0x764c2503, BRF_GRA }, \
	{ "sfiii2-simm4.1",			0x200000, 0x3e16af6e, BRF_GRA }, \
	{ "sfiii2-simm4.2",			0x200000, 0x215705e6, BRF_GRA }, \
	{ "sfiii2-simm4.3",			0x200000, 0xe30cbd9c, BRF_GRA }, \
	{ "sfiii2-simm4.4",			0x200000, 0x4185ded9, BRF_GRA }, \
	{ "sfiii2-simm4.5",			0x200000, 0x4e8db013, BRF_GRA }, \
	{ "sfiii2-simm4.6",			0x200000, 0x08df48ce, BRF_GRA }, \
	{ "sfiii2-simm4.7",			0x200000, 0xbb8f80a5, BRF_GRA }, \
	{ "sfiii2-simm5.0",			0x200000, 0xebdc4787, BRF_GRA }, \
	{ "sfiii2-simm5.1",			0x200000, 0x6b7c550e, BRF_GRA }, \
	{ "sfiii2-simm5.2",			0x200000, 0x56ff8c50, BRF_GRA }, \
	{ "sfiii2-simm5.3",			0x200000, 0x3f2ac3e9, BRF_GRA }, \
	{ "sfiii2-simm5.4",			0x200000, 0x48cda50e, BRF_GRA }, \
	{ "sfiii2-simm5.5",			0x200000, 0x520c0af6, BRF_GRA }, \
	{ "sfiii2-simm5.6",			0x200000, 0x2edc5986, BRF_GRA }, \
	{ "sfiii2-simm5.7",			0x200000, 0x93ffa199, BRF_GRA },

// --------------------------------------------------------
// Street Fighter III 2nd Impact: Giant Attack (USA 970930)
// --------------------------------------------------------
static struct BurnRomInfo sfiii2RomDesc[] = {

	{ "sfiii2_usa.29f400.u2",				0x080000, 0x75dd72e0, BRF_ESS | BRF_BIOS },	// SH-2 Bios
					  
#if !defined ROM_VERIFY
	SFIII2_970930_FLASH
#endif
};

STD_ROM_PICK(sfiii2)
STD_ROM_FN(sfiii2)

// ----------------------------------------------------------
// Street Fighter III 2nd Impact: Giant Attack (Japan 970930)
// ----------------------------------------------------------
static struct BurnRomInfo sfiii2jRomDesc[] = {

	{ "sfiii2_japan.29f400.u2",				0x080000, 0xfaea0a3e, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	SFIII2_970930_FLASH
#endif
};

STD_ROM_PICK(sfiii2j)
STD_ROM_FN(sfiii2j)

// ----------------------------------------------------------
// Street Fighter III 2nd Impact: Giant Attack (Hispanic 970930)
// ----------------------------------------------------------
static struct BurnRomInfo sfiii2hRomDesc[] = {

	{ "sfiii2_hispanic.29f400.u2",			0x080000, 0x5c799526, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	SFIII2_970930_FLASH
#endif
};

STD_ROM_PICK(sfiii2h)
STD_ROM_FN(sfiii2h)

// ----------------------------------------------------------------
// Street Fighter III 2nd Impact: Giant Attack (Asia 970930, NO CD)
// ----------------------------------------------------------------
static struct BurnRomInfo sfiii2nRomDesc[] = {

	{ "sfiii2_asia_nocd.29f400.u2",			0x080000, 0xfd297c0d, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	SFIII2_970930_FLASH
};

STD_ROM_PICK(sfiii2n)
STD_ROM_FN(sfiii2n)

// ------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future (990608)
// ------------------------------------------------------------

#define SFIII3_990608_FLASH \
	{ "sfiii3-simm1.0",			0x200000, 0x11dfd3cd, BRF_ESS | BRF_PRG }, \
	{ "sfiii3-simm1.1",			0x200000, 0xc50585e6, BRF_ESS | BRF_PRG }, \
	{ "sfiii3-simm1.2",			0x200000, 0x8e011d9b, BRF_ESS | BRF_PRG }, \
	{ "sfiii3-simm1.3",			0x200000, 0xdca8d92f, BRF_ESS | BRF_PRG }, \
	{ "sfiii3-simm2.0",			0x200000, 0x06eb969e, BRF_ESS | BRF_PRG }, \
	{ "sfiii3-simm2.1",			0x200000, 0xe7039f82, BRF_ESS | BRF_PRG }, \
	{ "sfiii3-simm2.2",			0x200000, 0x645c96f7, BRF_ESS | BRF_PRG }, \
	{ "sfiii3-simm2.3",			0x200000, 0x610efab1, BRF_ESS | BRF_PRG }, \
	{ "sfiii3-simm3.0",			0x200000, 0x7baa1f79, BRF_GRA }, \
	{ "sfiii3-simm3.1",			0x200000, 0x234bf8fe, BRF_GRA }, \
	{ "sfiii3-simm3.2",			0x200000, 0xd9ebc308, BRF_GRA }, \
	{ "sfiii3-simm3.3",			0x200000, 0x293cba77, BRF_GRA }, \
	{ "sfiii3-simm3.4",			0x200000, 0x6055e747, BRF_GRA }, \
	{ "sfiii3-simm3.5",			0x200000, 0x499aa6fc, BRF_GRA }, \
	{ "sfiii3-simm3.6",			0x200000, 0x6c13879e, BRF_GRA }, \
	{ "sfiii3-simm3.7",			0x200000, 0xcf4f8ede, BRF_GRA }, \
	{ "sfiii3-simm4.0",			0x200000, 0x091fd5ba, BRF_GRA }, \
	{ "sfiii3-simm4.1",			0x200000, 0x0bca8917, BRF_GRA }, \
	{ "sfiii3-simm4.2",			0x200000, 0xa0fd578b, BRF_GRA }, \
	{ "sfiii3-simm4.3",			0x200000, 0x4bf8c699, BRF_GRA }, \
	{ "sfiii3-simm4.4",			0x200000, 0x137b8785, BRF_GRA }, \
	{ "sfiii3-simm4.5",			0x200000, 0x4fb70671, BRF_GRA }, \
	{ "sfiii3-simm4.6",			0x200000, 0x832374a4, BRF_GRA }, \
	{ "sfiii3-simm4.7",			0x200000, 0x1c88576d, BRF_GRA }, \
	{ "sfiii3-simm5.0",			0x200000, 0xc67d9190, BRF_GRA }, \
	{ "sfiii3-simm5.1",			0x200000, 0x6cb79868, BRF_GRA }, \
	{ "sfiii3-simm5.2",			0x200000, 0xdf69930e, BRF_GRA }, \
	{ "sfiii3-simm5.3",			0x200000, 0x333754e0, BRF_GRA }, \
	{ "sfiii3-simm5.4",			0x200000, 0x78f6d417, BRF_GRA }, \
	{ "sfiii3-simm5.5",			0x200000, 0x8ccad9b1, BRF_GRA }, \
	{ "sfiii3-simm5.6",			0x200000, 0x85de59e5, BRF_GRA }, \
	{ "sfiii3-simm5.7",			0x200000, 0xee7e29b3, BRF_GRA }, \
	{ "sfiii3-simm6.0",			0x200000, 0x8da69042, BRF_GRA }, \
	{ "sfiii3-simm6.1",			0x200000, 0x1c8c7ac4, BRF_GRA }, \
	{ "sfiii3-simm6.2",			0x200000, 0xa671341d, BRF_GRA }, \
	{ "sfiii3-simm6.3",			0x200000, 0x1a990249, BRF_GRA }, \
	{ "sfiii3-simm6.4",			0x200000, 0x20cb39ac, BRF_GRA }, \
	{ "sfiii3-simm6.5",			0x200000, 0x5f844b2f, BRF_GRA }, \
	{ "sfiii3-simm6.6",			0x200000, 0x450e8d28, BRF_GRA }, \
	{ "sfiii3-simm6.7",			0x200000, 0xcc5f4187, BRF_GRA },

// -----------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future (Europe 990608)
// -----------------------------------------------------------------
static struct BurnRomInfo sfiii3RomDesc[] = {

	{ "sfiii3_euro.29f400.u2",				0x080000, 0x30bbf293, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	SFIII3_990608_FLASH
#endif
};

STD_ROM_PICK(sfiii3)
STD_ROM_FN(sfiii3)
	
// ----------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future (USA 990608)
// ----------------------------------------------------------------
static struct BurnRomInfo sfiii3uRomDesc[] = {

	{ "sfiii3_usa.29f400.u2",				0x080000, 0xecc545c1, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	SFIII3_990608_FLASH
#endif
};

STD_ROM_PICK(sfiii3u)
STD_ROM_FN(sfiii3u)

// -------------------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future (Japan 990608)
// -------------------------------------------------------------------------
static struct BurnRomInfo sfiii3jRomDesc[] = {

	{ "sfiii3_japan.29f400.u2",				0x080000, 0x63f23d1f, BRF_ESS | BRF_BIOS },	// SH-2 Bios

#if !defined ROM_VERIFY	
	SFIII3_990608_FLASH
#endif
};

STD_ROM_PICK(sfiii3j)
STD_ROM_FN(sfiii3j)

// -------------------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future (Japan 990608, NO CD)
// -------------------------------------------------------------------------
static struct BurnRomInfo sfiii3nRomDesc[] = {

	{ "sfiii3_japan_nocd.29f400.u2",		0x080000, 0x1edc6366, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	SFIII3_990608_FLASH
};

STD_ROM_PICK(sfiii3n)
STD_ROM_FN(sfiii3n)

// -------------------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future (Asia 990608, NO CD)
// -------------------------------------------------------------------------
static struct BurnRomInfo sfiii3naRomDesc[] = {

	{ "sfiii3_asia_nocd.29f400.u2",		0x080000, 0xa12ebcd1, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	SFIII3_990608_FLASH
};

STD_ROM_PICK(sfiii3na)
STD_ROM_FN(sfiii3na)

// ------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future (990512)
// ------------------------------------------------------------

#define SFIII3_990512_FLASH \
	{ "sfiii3-simm1.0",			0x200000, 0x66e66235, BRF_ESS | BRF_PRG }, \
	{ "sfiii3-simm1.1",			0x200000, 0x186e8c5f, BRF_ESS | BRF_PRG }, \
	{ "sfiii3-simm1.2",			0x200000, 0xbce18cab, BRF_ESS | BRF_PRG }, \
	{ "sfiii3-simm1.3",			0x200000, 0x129dc2c9, BRF_ESS | BRF_PRG }, \
	{ "sfiii3-simm2.0",			0x200000, 0x06eb969e, BRF_ESS | BRF_PRG }, \
	{ "sfiii3-simm2.1",			0x200000, 0xe7039f82, BRF_ESS | BRF_PRG }, \
	{ "sfiii3-simm2.2",			0x200000, 0x645c96f7, BRF_ESS | BRF_PRG }, \
	{ "sfiii3-simm2.3",			0x200000, 0x610efab1, BRF_ESS | BRF_PRG }, \
	{ "sfiii3-simm3.0",			0x200000, 0x7baa1f79, BRF_GRA }, \
	{ "sfiii3-simm3.1",			0x200000, 0x234bf8fe, BRF_GRA }, \
	{ "sfiii3-simm3.2",			0x200000, 0xd9ebc308, BRF_GRA }, \
	{ "sfiii3-simm3.3",			0x200000, 0x293cba77, BRF_GRA }, \
	{ "sfiii3-simm3.4",			0x200000, 0x6055e747, BRF_GRA }, \
	{ "sfiii3-simm3.5",			0x200000, 0x499aa6fc, BRF_GRA }, \
	{ "sfiii3-simm3.6",			0x200000, 0x6c13879e, BRF_GRA }, \
	{ "sfiii3-simm3.7",			0x200000, 0xcf4f8ede, BRF_GRA }, \
	{ "sfiii3-simm4.0",			0x200000, 0x091fd5ba, BRF_GRA }, \
	{ "sfiii3-simm4.1",			0x200000, 0x0bca8917, BRF_GRA }, \
	{ "sfiii3-simm4.2",			0x200000, 0xa0fd578b, BRF_GRA }, \
	{ "sfiii3-simm4.3",			0x200000, 0x4bf8c699, BRF_GRA }, \
	{ "sfiii3-simm4.4",			0x200000, 0x137b8785, BRF_GRA }, \
	{ "sfiii3-simm4.5",			0x200000, 0x4fb70671, BRF_GRA }, \
	{ "sfiii3-simm4.6",			0x200000, 0x832374a4, BRF_GRA }, \
	{ "sfiii3-simm4.7",			0x200000, 0x1c88576d, BRF_GRA }, \
	{ "sfiii3-simm5.0",			0x200000, 0xc67d9190, BRF_GRA }, \
	{ "sfiii3-simm5.1",			0x200000, 0x6cb79868, BRF_GRA }, \
	{ "sfiii3-simm5.2",			0x200000, 0xdf69930e, BRF_GRA }, \
	{ "sfiii3-simm5.3",			0x200000, 0x333754e0, BRF_GRA }, \
	{ "sfiii3-simm5.4",			0x200000, 0x78f6d417, BRF_GRA }, \
	{ "sfiii3-simm5.5",			0x200000, 0x8ccad9b1, BRF_GRA }, \
	{ "sfiii3-simm5.6",			0x200000, 0x85de59e5, BRF_GRA }, \
	{ "sfiii3-simm5.7",			0x200000, 0xee7e29b3, BRF_GRA }, \
	{ "sfiii3-simm6.0",			0x200000, 0x8da69042, BRF_GRA }, \
	{ "sfiii3-simm6.1",			0x200000, 0x1c8c7ac4, BRF_GRA }, \
	{ "sfiii3-simm6.2",			0x200000, 0xa671341d, BRF_GRA }, \
	{ "sfiii3-simm6.3",			0x200000, 0x1a990249, BRF_GRA }, \
	{ "sfiii3-simm6.4",			0x200000, 0x20cb39ac, BRF_GRA }, \
	{ "sfiii3-simm6.5",			0x200000, 0x5f844b2f, BRF_GRA }, \
	{ "sfiii3-simm6.6",			0x200000, 0x450e8d28, BRF_GRA }, \
	{ "sfiii3-simm6.7",			0x200000, 0xcc5f4187, BRF_GRA },

// -----------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future (Europe 990512)
// -----------------------------------------------------------------
static struct BurnRomInfo sfiii3r1RomDesc[] = {

	{ "sfiii3_euro.29f400.u2",				0x080000, 0x30bbf293, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	SFIII3_990512_FLASH
#endif
};

STD_ROM_PICK(sfiii3r1)
STD_ROM_FN(sfiii3r1)

// ----------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future (USA 990512)
// ----------------------------------------------------------------
static struct BurnRomInfo sfiii3ur1RomDesc[] = {

	{ "sfiii3_usa.29f400.u2",				0x080000, 0xecc545c1, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	SFIII3_990512_FLASH
#endif
};

STD_ROM_PICK(sfiii3ur1)
STD_ROM_FN(sfiii3ur1)

// -------------------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future (Japan 990512)
// -------------------------------------------------------------------------
static struct BurnRomInfo sfiii3jr1RomDesc[] = {

	{ "sfiii3_japan.29f400.u2",				0x080000, 0x63f23d1f, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	SFIII3_990512_FLASH
#endif
};

STD_ROM_PICK(sfiii3jr1)
STD_ROM_FN(sfiii3jr1)

// -------------------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future (Japan 990512, NO CD)
// -------------------------------------------------------------------------
static struct BurnRomInfo sfiii3nr1RomDesc[] = {

	{ "sfiii3_japan_nocd.29f400.u2",		0x080000, 0x1edc6366, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	SFIII3_990512_FLASH
};

STD_ROM_PICK(sfiii3nr1)
STD_ROM_FN(sfiii3nr1)

// -------------------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future (Asia 990512, NO CD)
// -------------------------------------------------------------------------
static struct BurnRomInfo sfiii3nar1RomDesc[] = {

	{ "sfiii3_asia_nocd.29f400.u2",		0x080000, 0xa12ebcd1, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	SFIII3_990512_FLASH
};

STD_ROM_PICK(sfiii3nar1)
STD_ROM_FN(sfiii3nar1)

// --------------------------------------------------
// JoJo no Kimyou na Bouken / JoJo's Venture (990128)
// --------------------------------------------------

#define JOJO_990128_FLASH \
	{ "jojo-simm1.0",			0x200000, 0x9516948b, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm1.1",			0x200000, 0xa847848d, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm1.2",			0x200000, 0x853e8846, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm1.3",			0x200000, 0xc04fe00e, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm2.0",			0x200000, 0xe1a4b3c8, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm2.1",			0x200000, 0x189cef95, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm2.2",			0x200000, 0x47db5ec6, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm2.3",			0x200000, 0xe3d3a155, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm3.0",			0x200000, 0xde7fc9c1, BRF_GRA }, \
	{ "jojo-simm3.1",			0x200000, 0x43d053d3, BRF_GRA }, \
	{ "jojo-simm3.2",			0x200000, 0x2ffd7fa5, BRF_GRA }, \
	{ "jojo-simm3.3",			0x200000, 0x4da4985b, BRF_GRA }, \
	{ "jojo-simm3.4",			0x200000, 0xfde98d72, BRF_GRA }, \
	{ "jojo-simm3.5",			0x200000, 0xedb2a266, BRF_GRA }, \
	{ "jojo-simm3.6",			0x200000, 0xbe7cf319, BRF_GRA }, \
	{ "jojo-simm3.7",			0x200000, 0x56fe1a9f, BRF_GRA }, \
	{ "jojo-simm4.0",			0x200000, 0xc4e7bf68, BRF_GRA }, \
	{ "jojo-simm4.1",			0x200000, 0xb62b2719, BRF_GRA }, \
	{ "jojo-simm4.2",			0x200000, 0x18d15809, BRF_GRA }, \
	{ "jojo-simm4.3",			0x200000, 0x9af0ad79, BRF_GRA }, \
	{ "jojo-simm4.4",			0x200000, 0x4124c1f0, BRF_GRA }, \
	{ "jojo-simm4.5",			0x200000, 0x5e001fd1, BRF_GRA }, \
	{ "jojo-simm4.6",			0x200000, 0x9affa23b, BRF_GRA }, \
	{ "jojo-simm4.7",			0x200000, 0x2511572a, BRF_GRA }, \
	{ "jojo-simm5.0",			0x200000, 0x797615fc, BRF_GRA }, \
	{ "jojo-simm5.1",			0x200000, 0x734fd162, BRF_GRA },
	
// ----------------------------
// JoJo's Venture (Europe 990128)
// ----------------------------
static struct BurnRomInfo jojoRomDesc[] = {

	{ "jojo_euro.29f400.u2",				0x080000, 0x513e40ec, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	JOJO_990128_FLASH
#endif
};

STD_ROM_PICK(jojo)
STD_ROM_FN(jojo)

// ---------------------------
// JoJo's Venture (USA 990128)
// ---------------------------
static struct BurnRomInfo jojouRomDesc[] = {

	{ "jojo_usa.29f400.u2",					0x080000, 0x8d40f7be, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	JOJO_990128_FLASH
#endif
};

STD_ROM_PICK(jojou)
STD_ROM_FN(jojou)

// ----------------------------
// JoJo's Venture (Asia 990128)
// ----------------------------
static struct BurnRomInfo jojoaRomDesc[] = {

	{ "jojo_asia.29f400.u2",				0x080000, 0x789aa72a, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	JOJO_990128_FLASH
#endif
};

STD_ROM_PICK(jojoa)
STD_ROM_FN(jojoa)

// ---------------------------------------
// JoJo no Kimyou na Bouken (Japan 990128)
// ---------------------------------------
static struct BurnRomInfo jojojRomDesc[] = {

	{ "jojo_japan.29f400.u2",				0x080000, 0x02778f60, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	JOJO_990128_FLASH
#endif
};

STD_ROM_PICK(jojoj)
STD_ROM_FN(jojoj)

// -----------------------------------
// JoJo's Venture (Asia 990128, NO CD)
// -----------------------------------
static struct BurnRomInfo jojonRomDesc[] = {

	{ "jojo_asia_nocd.29f400.u2",			0x080000, 0x05b4f953, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	JOJO_990128_FLASH
};

STD_ROM_PICK(jojon)
STD_ROM_FN(jojon)

// --------------------------------------------------
// JoJo no Kimyou na Bouken / JoJo's Venture (990108)
// --------------------------------------------------

#define JOJO_990108_FLASH \
	{ "jojo-simm1.0",			0x200000, 0xcfbc38d6, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm1.1",			0x200000, 0x42578d94, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm1.2",			0x200000, 0x1b40c566, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm1.3",			0x200000, 0xbba709b4, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm2.0",			0x200000, 0x417e5dc1, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm2.1",			0x200000, 0xd3b3267d, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm2.2",			0x200000, 0xc66d96b1, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm2.3",			0x200000, 0xaa34cc85, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm3.0",			0x200000, 0xde7fc9c1, BRF_GRA }, \
	{ "jojo-simm3.1",			0x200000, 0x43d053d3, BRF_GRA }, \
	{ "jojo-simm3.2",			0x200000, 0x2ffd7fa5, BRF_GRA }, \
	{ "jojo-simm3.3",			0x200000, 0x4da4985b, BRF_GRA }, \
	{ "jojo-simm3.4",			0x200000, 0xfde98d72, BRF_GRA }, \
	{ "jojo-simm3.5",			0x200000, 0xedb2a266, BRF_GRA }, \
	{ "jojo-simm3.6",			0x200000, 0xbe7cf319, BRF_GRA }, \
	{ "jojo-simm3.7",			0x200000, 0x56fe1a9f, BRF_GRA }, \
	{ "jojo-simm4.0",			0x200000, 0xc4e7bf68, BRF_GRA }, \
	{ "jojo-simm4.1",			0x200000, 0xb62b2719, BRF_GRA }, \
	{ "jojo-simm4.2",			0x200000, 0x18d15809, BRF_GRA }, \
	{ "jojo-simm4.3",			0x200000, 0x9af0ad79, BRF_GRA }, \
	{ "jojo-simm4.4",			0x200000, 0x4124c1f0, BRF_GRA }, \
	{ "jojo-simm4.5",			0x200000, 0x5e001fd1, BRF_GRA }, \
	{ "jojo-simm4.6",			0x200000, 0x9affa23b, BRF_GRA }, \
	{ "jojo-simm4.7",			0x200000, 0x2511572a, BRF_GRA }, \
	{ "jojo-simm5.0",			0x200000, 0x797615fc, BRF_GRA }, \
	{ "jojo-simm5.1",			0x200000, 0x734fd162, BRF_GRA },

// ----------------------------
// JoJo's Venture (Europe 990108)
// ----------------------------
static struct BurnRomInfo jojor1RomDesc[] = {

	{ "jojo_euro.29f400.u2",				0x080000, 0x513e40ec, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	JOJO_990108_FLASH
#endif
};

STD_ROM_PICK(jojor1)
STD_ROM_FN(jojor1)

// ---------------------------
// JoJo's Venture (USA 990108)
// ---------------------------
static struct BurnRomInfo jojour1RomDesc[] = {

	{ "jojo_usa.29f400.u2",					0x080000, 0x8d40f7be, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	JOJO_990108_FLASH
#endif
};

STD_ROM_PICK(jojour1)
STD_ROM_FN(jojour1)

// ----------------------------
// JoJo's Venture (Asia 990108)
// ----------------------------
static struct BurnRomInfo jojoar1RomDesc[] = {

	{ "jojo_asia.29f400.u2",				0x080000, 0x789aa72a, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	JOJO_990108_FLASH
#endif
};

STD_ROM_PICK(jojoar1)
STD_ROM_FN(jojoar1)

// ---------------------------------------
// JoJo no Kimyou na Bouken (Japan 990108)
// ---------------------------------------
static struct BurnRomInfo jojojr1RomDesc[] = {

	{ "jojo_japan.29f400.u2",				0x080000, 0x02778f60, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	JOJO_990108_FLASH
#endif
};

STD_ROM_PICK(jojojr1)
STD_ROM_FN(jojojr1)

// -----------------------------------
// JoJo's Venture (Asia 990108, NO CD)
// -----------------------------------
static struct BurnRomInfo jojonr1RomDesc[] = {

	{ "jojo_asia_nocd.29f400.u2",			0x080000, 0x05b4f953, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	JOJO_990108_FLASH
};

STD_ROM_PICK(jojonr1)
STD_ROM_FN(jojonr1)

// --------------------------------------------------
// JoJo no Kimyou na Bouken / JoJo's Venture (981202)
// --------------------------------------------------

#define JOJO_981202_FLASH \
	{ "jojo-simm1.0",			0x200000, 0xe06ba886, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm1.1",			0x200000, 0x6dd177c8, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm1.2",			0x200000, 0xd35a15e0, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm1.3",			0x200000, 0x66d865ac, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm2.0",			0x200000, 0x417e5dc1, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm2.1",			0x200000, 0xc891c887, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm2.2",			0x200000, 0x1e101f30, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm2.3",			0x200000, 0x1fd1d3e4, BRF_ESS | BRF_PRG }, \
	{ "jojo-simm3.0",			0x200000, 0xde7fc9c1, BRF_GRA }, \
	{ "jojo-simm3.1",			0x200000, 0x43d053d3, BRF_GRA }, \
	{ "jojo-simm3.2",			0x200000, 0x2ffd7fa5, BRF_GRA }, \
	{ "jojo-simm3.3",			0x200000, 0x4da4985b, BRF_GRA }, \
	{ "jojo-simm3.4",			0x200000, 0xfde98d72, BRF_GRA }, \
	{ "jojo-simm3.5",			0x200000, 0xedb2a266, BRF_GRA }, \
	{ "jojo-simm3.6",			0x200000, 0xbe7cf319, BRF_GRA }, \
	{ "jojo-simm3.7",			0x200000, 0x56fe1a9f, BRF_GRA }, \
	{ "jojo-simm4.0",			0x200000, 0xc4e7bf68, BRF_GRA }, \
	{ "jojo-simm4.1",			0x200000, 0xb62b2719, BRF_GRA }, \
	{ "jojo-simm4.2",			0x200000, 0x18d15809, BRF_GRA }, \
	{ "jojo-simm4.3",			0x200000, 0x9af0ad79, BRF_GRA }, \
	{ "jojo-simm4.4",			0x200000, 0x4124c1f0, BRF_GRA }, \
	{ "jojo-simm4.5",			0x200000, 0x5e001fd1, BRF_GRA }, \
	{ "jojo-simm4.6",			0x200000, 0x9affa23b, BRF_GRA }, \
	{ "jojo-simm4.7",			0x200000, 0x2511572a, BRF_GRA }, \
	{ "jojo-simm5.0",			0x200000, 0x797615fc, BRF_GRA }, \
	{ "jojo-simm5.1",			0x200000, 0x734fd162, BRF_GRA },

// ----------------------------
// JoJo's Venture (Europe 981202)
// ----------------------------
static struct BurnRomInfo jojor2RomDesc[] = {

	{ "jojo_euro.29f400.u2",				0x080000, 0x513e40ec, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	JOJO_981202_FLASH
#endif
};

STD_ROM_PICK(jojor2)
STD_ROM_FN(jojor2)

// ---------------------------
// JoJo's Venture (USA 981202)
// ---------------------------
static struct BurnRomInfo jojour2RomDesc[] = {

	{ "jojo_usa.29f400.u2",					0x080000, 0x8d40f7be, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	JOJO_981202_FLASH
#endif
};

STD_ROM_PICK(jojour2)
STD_ROM_FN(jojour2)

// ----------------------------
// JoJo's Venture (Asia 981202)
// ----------------------------
static struct BurnRomInfo jojoar2RomDesc[] = {

	{ "jojo_asia.29f400.u2",				0x080000, 0x789aa72a, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	JOJO_981202_FLASH
#endif
};

STD_ROM_PICK(jojoar2)
STD_ROM_FN(jojoar2)

// ---------------------------------------
// JoJo no Kimyou na Bouken (Japan 981202)
// ---------------------------------------
static struct BurnRomInfo jojojr2RomDesc[] = {

	{ "jojo_japan.29f400.u2",				0x080000, 0x02778f60, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	JOJO_981202_FLASH
#endif
};

STD_ROM_PICK(jojojr2)
STD_ROM_FN(jojojr2)

// -----------------------------------
// JoJo's Venture (Asia 981202, NO CD)
// -----------------------------------
static struct BurnRomInfo jojonr2RomDesc[] = {

	{ "jojo_asia_nocd.29f400.u2",			0x080000, 0x05b4f953, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	JOJO_981202_FLASH
};

STD_ROM_PICK(jojonr2)
STD_ROM_FN(jojonr2)

// -----------------------------------------------------------------------------
// JoJo no Kimyou na Bouken: Mirai e no Isan / JoJo's Bizarre Adventure (991015)
// -----------------------------------------------------------------------------

#define JOJOBA_991015_FLASH \
	{ "jojoba-simm1.0",			0x200000, 0xb3cc516d, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm1.1",			0x200000, 0xdab4bdc7, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm1.2",			0x200000, 0xa6a4bf48, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm1.3",			0x200000, 0x731229ee, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm2.0",			0x200000, 0x535f2eba, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm2.1",			0x200000, 0x01dd3a01, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm2.2",			0x200000, 0x61432672, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm2.3",			0x200000, 0xacdc9aca, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm3.0",			0x200000, 0x4d16e111, BRF_GRA }, \
	{ "jojoba-simm3.1",			0x200000, 0x9b3406d3, BRF_GRA }, \
	{ "jojoba-simm3.2",			0x200000, 0xf2414997, BRF_GRA }, \
	{ "jojoba-simm3.3",			0x200000, 0x954b9c7d, BRF_GRA }, \
	{ "jojoba-simm3.4",			0x200000, 0x625adc1d, BRF_GRA }, \
	{ "jojoba-simm3.5",			0x200000, 0x20a70bb4, BRF_GRA }, \
	{ "jojoba-simm3.6",			0x200000, 0xa10ec5af, BRF_GRA }, \
	{ "jojoba-simm3.7",			0x200000, 0x0bd0de7a, BRF_GRA }, \
	{ "jojoba-simm4.0",			0x200000, 0x6ea14adc, BRF_GRA }, \
	{ "jojoba-simm4.1",			0x200000, 0x8f4c42fb, BRF_GRA }, \
	{ "jojoba-simm4.2",			0x200000, 0xef0586d1, BRF_GRA }, \
	{ "jojoba-simm4.3",			0x200000, 0x93ccc470, BRF_GRA }, \
	{ "jojoba-simm4.4",			0x200000, 0x3d9ec7d2, BRF_GRA }, \
	{ "jojoba-simm4.5",			0x200000, 0x03e66850, BRF_GRA }, \
	{ "jojoba-simm4.6",			0x200000, 0x01606ac3, BRF_GRA }, \
	{ "jojoba-simm4.7",			0x200000, 0x36392b87, BRF_GRA }, \
	{ "jojoba-simm5.0",			0x200000, 0x2ef8c60c, BRF_GRA }, \
	{ "jojoba-simm5.1",			0x200000, 0xcf7d7ca6, BRF_GRA }, \
	{ "jojoba-simm5.2",			0x200000, 0xb7815bfa, BRF_GRA }, \
	{ "jojoba-simm5.3",			0x200000, 0x9bfec049, BRF_GRA }, \
	{ "jojoba-simm5.4",			0x200000, 0xd167536b, BRF_GRA }, \
	{ "jojoba-simm5.5",			0x200000, 0x55e7a042, BRF_GRA }, \
	{ "jojoba-simm5.6",			0x200000, 0x4fb32906, BRF_GRA }, \
	{ "jojoba-simm5.7",			0x200000, 0x8c8be520, BRF_GRA },

// ---------------------------------------------
// JoJo's Bizarre Adventure (Europe 991015, NO CD)
// ---------------------------------------------
static struct BurnRomInfo jojobaRomDesc[] = {

	{ "jojoba_euro_nocd.29f400.u2",			0x080000, 0x1ee2d679, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	JOJOBA_991015_FLASH
};

STD_ROM_PICK(jojoba)
STD_ROM_FN(jojoba)

// ---------------------------------------------------------------
// JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 991015, NO CD)
// ---------------------------------------------------------------
static struct BurnRomInfo jojobanRomDesc[] = {

	{ "jojoba_japan_nocd.29f400.u2",		0x080000, 0x4dab19f5, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	JOJOBA_991015_FLASH
};

STD_ROM_PICK(jojoban)
STD_ROM_FN(jojoban)

// -----------------------------------------------------------------------------
// JoJo no Kimyou na Bouken: Mirai e no Isan / JoJo's Bizarre Adventure (990927)
// -----------------------------------------------------------------------------

#define JOJOBA_990927_FLASH \
	{ "jojoba-simm1.0",			0x200000, 0xadcd8377, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm1.1",			0x200000, 0xd7590b59, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm1.2",			0x200000, 0xe62e240b, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm1.3",			0x200000, 0xc95450c3, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm2.0",			0x200000, 0x535f2eba, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm2.1",			0x200000, 0x01dd3a01, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm2.2",			0x200000, 0x61432672, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm2.3",			0x200000, 0xacdc9aca, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm3.0",			0x200000, 0x4d16e111, BRF_GRA }, \
	{ "jojoba-simm3.1",			0x200000, 0x9b3406d3, BRF_GRA }, \
	{ "jojoba-simm3.2",			0x200000, 0xf2414997, BRF_GRA }, \
	{ "jojoba-simm3.3",			0x200000, 0x954b9c7d, BRF_GRA }, \
	{ "jojoba-simm3.4",			0x200000, 0x625adc1d, BRF_GRA }, \
	{ "jojoba-simm3.5",			0x200000, 0x20a70bb4, BRF_GRA }, \
	{ "jojoba-simm3.6",			0x200000, 0xa10ec5af, BRF_GRA }, \
	{ "jojoba-simm3.7",			0x200000, 0x0bd0de7a, BRF_GRA }, \
	{ "jojoba-simm4.0",			0x200000, 0x6ea14adc, BRF_GRA }, \
	{ "jojoba-simm4.1",			0x200000, 0x8f4c42fb, BRF_GRA }, \
	{ "jojoba-simm4.2",			0x200000, 0xef0586d1, BRF_GRA }, \
	{ "jojoba-simm4.3",			0x200000, 0x93ccc470, BRF_GRA }, \
	{ "jojoba-simm4.4",			0x200000, 0x3d9ec7d2, BRF_GRA }, \
	{ "jojoba-simm4.5",			0x200000, 0x03e66850, BRF_GRA }, \
	{ "jojoba-simm4.6",			0x200000, 0x01606ac3, BRF_GRA }, \
	{ "jojoba-simm4.7",			0x200000, 0x36392b87, BRF_GRA }, \
	{ "jojoba-simm5.0",			0x200000, 0x2ef8c60c, BRF_GRA }, \
	{ "jojoba-simm5.1",			0x200000, 0xcf7d7ca6, BRF_GRA }, \
	{ "jojoba-simm5.2",			0x200000, 0xb7815bfa, BRF_GRA }, \
	{ "jojoba-simm5.3",			0x200000, 0x9bfec049, BRF_GRA }, \
	{ "jojoba-simm5.4",			0x200000, 0xd167536b, BRF_GRA }, \
	{ "jojoba-simm5.5",			0x200000, 0x55e7a042, BRF_GRA }, \
	{ "jojoba-simm5.6",			0x200000, 0x4fb32906, BRF_GRA }, \
	{ "jojoba-simm5.7",			0x200000, 0x8c8be520, BRF_GRA },

// --------------------------------------
// JoJo's Bizarre Adventure (Europe 990927)
// --------------------------------------
static struct BurnRomInfo jojobar1RomDesc[] = {

	{ "jojoba_euro.29f400.u2",				0x080000, 0x63cc8800, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	JOJOBA_990927_FLASH
#endif
};

STD_ROM_PICK(jojobar1)
STD_ROM_FN(jojobar1)

// --------------------------------------------------------
// JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 990927)
// --------------------------------------------------------
static struct BurnRomInfo jojobajr1RomDesc[] = {

	{ "jojoba_japan.29f400.u2",				0x080000, 0x3085478c, BRF_ESS | BRF_BIOS },	// SH-2 Bios
					  
#if !defined ROM_VERIFY
	JOJOBA_990927_FLASH
#endif
};

STD_ROM_PICK(jojobajr1)
STD_ROM_FN(jojobajr1)

// ---------------------------------------------------------------
// JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 990927, NO CD)
// ---------------------------------------------------------------
static struct BurnRomInfo jojobanr1RomDesc[] = {

	{ "jojoba_japan_nocd.29f400.u2",		0x080000, 0x4dab19f5, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	JOJOBA_990927_FLASH
};

STD_ROM_PICK(jojobanr1)
STD_ROM_FN(jojobanr1)

// ---------------------------------------------
// JoJo's Bizarre Adventure (Europe 990927, NO CD)
// ---------------------------------------------
static struct BurnRomInfo jojobaner1RomDesc[] = {

	{ "jojoba_euro_nocd.29f400.u2",			0x080000, 0x1ee2d679, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	JOJOBA_990927_FLASH
};

STD_ROM_PICK(jojobaner1)
STD_ROM_FN(jojobaner1)
	
// -----------------------------------------------------------------------------
// JoJo no Kimyou na Bouken: Mirai e no Isan / JoJo's Bizarre Adventure (990913)
// -----------------------------------------------------------------------------

#define JOJOBA_990913_FLASH \
	{ "jojoba-simm1.0",			0x200000, 0x76976231, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm1.1",			0x200000, 0xcedd78e7, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm1.2",			0x200000, 0x2955b77f, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm1.3",			0x200000, 0x280139d7, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm2.0",			0x200000, 0x305c4914, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm2.1",			0x200000, 0x18af4f3b, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm2.2",			0x200000, 0x397e5c9e, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm2.3",			0x200000, 0xa9d0a7d7, BRF_ESS | BRF_PRG }, \
	{ "jojoba-simm3.0",			0x200000, 0x4d16e111, BRF_GRA }, \
	{ "jojoba-simm3.1",			0x200000, 0x9b3406d3, BRF_GRA }, \
	{ "jojoba-simm3.2",			0x200000, 0xf2414997, BRF_GRA }, \
	{ "jojoba-simm3.3",			0x200000, 0x954b9c7d, BRF_GRA }, \
	{ "jojoba-simm3.4",			0x200000, 0x625adc1d, BRF_GRA }, \
	{ "jojoba-simm3.5",			0x200000, 0x20a70bb4, BRF_GRA }, \
	{ "jojoba-simm3.6",			0x200000, 0xa10ec5af, BRF_GRA }, \
	{ "jojoba-simm3.7",			0x200000, 0x0bd0de7a, BRF_GRA }, \
	{ "jojoba-simm4.0",			0x200000, 0x6ea14adc, BRF_GRA }, \
	{ "jojoba-simm4.1",			0x200000, 0x8f4c42fb, BRF_GRA }, \
	{ "jojoba-simm4.2",			0x200000, 0xef0586d1, BRF_GRA }, \
	{ "jojoba-simm4.3",			0x200000, 0x93ccc470, BRF_GRA }, \
	{ "jojoba-simm4.4",			0x200000, 0x3d9ec7d2, BRF_GRA }, \
	{ "jojoba-simm4.5",			0x200000, 0x03e66850, BRF_GRA }, \
	{ "jojoba-simm4.6",			0x200000, 0x01606ac3, BRF_GRA }, \
	{ "jojoba-simm4.7",			0x200000, 0x36392b87, BRF_GRA }, \
	{ "jojoba-simm5.0",			0x200000, 0x2ef8c60c, BRF_GRA }, \
	{ "jojoba-simm5.1",			0x200000, 0xcf7d7ca6, BRF_GRA }, \
	{ "jojoba-simm5.2",			0x200000, 0xb7815bfa, BRF_GRA }, \
	{ "jojoba-simm5.3",			0x200000, 0x9bfec049, BRF_GRA }, \
	{ "jojoba-simm5.4",			0x200000, 0xd167536b, BRF_GRA }, \
	{ "jojoba-simm5.5",			0x200000, 0x55e7a042, BRF_GRA }, \
	{ "jojoba-simm5.6",			0x200000, 0x4fb32906, BRF_GRA }, \
	{ "jojoba-simm5.7",			0x200000, 0x8c8be520, BRF_GRA },

// --------------------------------------
// JoJo's Bizarre Adventure (Europe 990913)
// --------------------------------------
static struct BurnRomInfo jojobar2RomDesc[] = {

	{ "jojoba_euro.29f400.u2",				0x080000, 0x63cc8800, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	JOJOBA_990913_FLASH
#endif
};

STD_ROM_PICK(jojobar2)
STD_ROM_FN(jojobar2)

// --------------------------------------------------------
// JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 990913)
// --------------------------------------------------------
static struct BurnRomInfo jojobajr2RomDesc[] = {

	{ "jojoba_japan.29f400.u2",				0x080000, 0x3085478c, BRF_ESS | BRF_BIOS },	// SH-2 Bios
					  
#if !defined ROM_VERIFY
	JOJOBA_990913_FLASH
#endif
};

STD_ROM_PICK(jojobajr2)
STD_ROM_FN(jojobajr2)

// ---------------------------------------------------------------
// JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 990913, NO CD)
// ---------------------------------------------------------------
static struct BurnRomInfo jojobanr2RomDesc[] = {

	{ "jojoba_japan_nocd.29f400.u2",		0x080000, 0x4dab19f5, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	JOJOBA_990913_FLASH
};

STD_ROM_PICK(jojobanr2)
STD_ROM_FN(jojobanr2)

// ---------------------------------------------
// JoJo's Bizarre Adventure (Europe 990913, NO CD)
// ---------------------------------------------
static struct BurnRomInfo jojobaner2RomDesc[] = {

	{ "jojoba_euro_nocd.29f400.u2",			0x080000, 0x1ee2d679, BRF_ESS | BRF_BIOS },	// SH-2 Bios

	JOJOBA_990913_FLASH
};

STD_ROM_PICK(jojobaner2)
STD_ROM_FN(jojobaner2)

// ----------------------------
// Red Earth / Warzard (961121)
// ----------------------------

#define REDEARTH_961121_FLASH \
	{ "redearth-simm1.0",		0x200000, 0xcad468f8, BRF_ESS | BRF_PRG }, \
	{ "redearth-simm1.1",		0x200000, 0xe9721d89, BRF_ESS | BRF_PRG }, \
	{ "redearth-simm1.2",		0x200000, 0x2889ec98, BRF_ESS | BRF_PRG }, \
	{ "redearth-simm1.3",		0x200000, 0x5a6cd148, BRF_ESS | BRF_PRG }, \
	{ "redearth-simm3.0",		0x200000, 0x83350cc5, BRF_GRA }, \
	{ "redearth-simm3.1",		0x200000, 0x56734de6, BRF_GRA }, \
	{ "redearth-simm3.2",		0x200000, 0x800ea0f1, BRF_GRA }, \
	{ "redearth-simm3.3",		0x200000, 0x97e9146c, BRF_GRA }, \
	{ "redearth-simm3.4",		0x200000, 0x0cb1d648, BRF_GRA }, \
	{ "redearth-simm3.5",		0x200000, 0x7a1099f0, BRF_GRA }, \
	{ "redearth-simm3.6",		0x200000, 0xaeff8f54, BRF_GRA }, \
	{ "redearth-simm3.7",		0x200000, 0xf770acd0, BRF_GRA }, \
	{ "redearth-simm4.0",		0x200000, 0x301e56f2, BRF_GRA }, \
	{ "redearth-simm4.1",		0x200000, 0x2048e103, BRF_GRA }, \
	{ "redearth-simm4.2",		0x200000, 0xc9433455, BRF_GRA }, \
	{ "redearth-simm4.3",		0x200000, 0xc02171a8, BRF_GRA }, \
	{ "redearth-simm4.4",		0x200000, 0x2ddbf276, BRF_GRA }, \
	{ "redearth-simm4.5",		0x200000, 0xfea820a6, BRF_GRA }, \
	{ "redearth-simm4.6",		0x200000, 0xc7528df1, BRF_GRA }, \
	{ "redearth-simm4.7",		0x200000, 0x2449cf3b, BRF_GRA }, \
	{ "redearth-simm5.0",		0x200000, 0x424451b9, BRF_GRA }, \
	{ "redearth-simm5.1",		0x200000, 0x9b8cb56b, BRF_GRA },

// -----------------------
// Red Earth (Europe 961121)
// -----------------------
static struct BurnRomInfo redearthRomDesc[] = {

	{ "redearth_euro.29f400.u2",			0x080000, 0x02e0f336, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	REDEARTH_961121_FLASH
#endif
};

STD_ROM_PICK(redearth)
STD_ROM_FN(redearth)

// ----------------------
// Warzard (Japan 961121)
// ----------------------
static struct BurnRomInfo warzardRomDesc[] = {

	{ "warzard_japan.29f400.u2",			0x080000, 0xf8e2f0c6, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	REDEARTH_961121_FLASH
#endif
};

STD_ROM_PICK(warzard)
STD_ROM_FN(warzard)

// ------------------------------
// Red Earth (Asia 961121, NO CD)
// ------------------------------
static struct BurnRomInfo redearthnRomDesc[] = {

	{ "redearth_asia_nocd.29f400.u2",		0x080000, 0x7a4f0851, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	REDEARTH_961121_FLASH
};

STD_ROM_PICK(redearthn)
STD_ROM_FN(redearthn)

// ----------------------------
// Red Earth / Warzard (961023)
// ----------------------------

#define REDEARTH_961023_FLASH \
	{ "redearth-simm1.0",		0x200000, 0x65bac346, BRF_ESS | BRF_PRG }, \
	{ "redearth-simm1.1",		0x200000, 0xa8ec4aae, BRF_ESS | BRF_PRG }, \
	{ "redearth-simm1.2",		0x200000, 0x2caf8995, BRF_ESS | BRF_PRG }, \
	{ "redearth-simm1.3",		0x200000, 0x13ebc21d, BRF_ESS | BRF_PRG }, \
	{ "redearth-simm3.0",		0x200000, 0x83350cc5, BRF_GRA }, \
	{ "redearth-simm3.1",		0x200000, 0x56734de6, BRF_GRA }, \
	{ "redearth-simm3.2",		0x200000, 0x800ea0f1, BRF_GRA }, \
	{ "redearth-simm3.3",		0x200000, 0x97e9146c, BRF_GRA }, \
	{ "redearth-simm3.4",		0x200000, 0x0cb1d648, BRF_GRA }, \
	{ "redearth-simm3.5",		0x200000, 0x7a1099f0, BRF_GRA }, \
	{ "redearth-simm3.6",		0x200000, 0xaeff8f54, BRF_GRA }, \
	{ "redearth-simm3.7",		0x200000, 0xf770acd0, BRF_GRA }, \
	{ "redearth-simm4.0",		0x200000, 0x301e56f2, BRF_GRA }, \
	{ "redearth-simm4.1",		0x200000, 0x2048e103, BRF_GRA }, \
	{ "redearth-simm4.2",		0x200000, 0xc9433455, BRF_GRA }, \
	{ "redearth-simm4.3",		0x200000, 0xc02171a8, BRF_GRA }, \
	{ "redearth-simm4.4",		0x200000, 0x2ddbf276, BRF_GRA }, \
	{ "redearth-simm4.5",		0x200000, 0xfea820a6, BRF_GRA }, \
	{ "redearth-simm4.6",		0x200000, 0xc7528df1, BRF_GRA }, \
	{ "redearth-simm4.7",		0x200000, 0x2449cf3b, BRF_GRA }, \
	{ "redearth-simm5.0",		0x200000, 0x424451b9, BRF_GRA }, \
	{ "redearth-simm5.1",		0x200000, 0x9b8cb56b, BRF_GRA },

// -----------------------
// Red Earth (Europe 961023)
// -----------------------
static struct BurnRomInfo redearthr1RomDesc[] = {

	{ "redearth_euro.29f400.u2",			0x080000, 0x02e0f336, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	REDEARTH_961023_FLASH
#endif
};

STD_ROM_PICK(redearthr1)
STD_ROM_FN(redearthr1)

// ----------------------
// Warzard (Japan 961023)
// ----------------------
static struct BurnRomInfo warzardr1RomDesc[] = {

	{ "warzard_japan.29f400.u2",		 	0x080000, 0xf8e2f0c6, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
#if !defined ROM_VERIFY
	REDEARTH_961023_FLASH
#endif
};

STD_ROM_PICK(warzardr1)
STD_ROM_FN(warzardr1)

// ------------------------------
// Red Earth (Asia 961023, NO CD)
// ------------------------------
static struct BurnRomInfo redearthnr1RomDesc[] = {

	{ "redearth_asia_nocd.29f400.u2",	0x080000, 0x7a4f0851, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	{ "redearthr1-simm1.0",				0x200000, 0x65bac346, BRF_ESS | BRF_PRG }, \
	{ "redearthr1-simm1.1",				0x200000, 0xa8ec4aae, BRF_ESS | BRF_PRG }, \
	{ "redearthr1-simm1.2",				0x200000, 0x2caf8995, BRF_ESS | BRF_PRG }, \
	{ "redearthr1-simm1.3",				0x200000, 0x13ebc21d, BRF_ESS | BRF_PRG }, \
	{ "redearth-simm3.0",				0x200000, 0x83350cc5, BRF_GRA }, \
	{ "redearth-simm3.1",				0x200000, 0x56734de6, BRF_GRA }, \
	{ "redearth-simm3.2",				0x200000, 0x800ea0f1, BRF_GRA }, \
	{ "redearth-simm3.3",				0x200000, 0x97e9146c, BRF_GRA }, \
	{ "redearth-simm3.4",				0x200000, 0x0cb1d648, BRF_GRA }, \
	{ "redearth-simm3.5",				0x200000, 0x7a1099f0, BRF_GRA }, \
	{ "redearth-simm3.6",				0x200000, 0xaeff8f54, BRF_GRA }, \
	{ "redearth-simm3.7",				0x200000, 0xf770acd0, BRF_GRA }, \
	{ "redearth-simm4.0",				0x200000, 0x301e56f2, BRF_GRA }, \
	{ "redearth-simm4.1",				0x200000, 0x2048e103, BRF_GRA }, \
	{ "redearth-simm4.2",				0x200000, 0xc9433455, BRF_GRA }, \
	{ "redearth-simm4.3",				0x200000, 0xc02171a8, BRF_GRA }, \
	{ "redearth-simm4.4",				0x200000, 0x2ddbf276, BRF_GRA }, \
	{ "redearth-simm4.5",				0x200000, 0xfea820a6, BRF_GRA }, \
	{ "redearth-simm4.6",				0x200000, 0xc7528df1, BRF_GRA }, \
	{ "redearth-simm4.7",				0x200000, 0x2449cf3b, BRF_GRA }, \
	{ "redearth-simm5.0",				0x200000, 0x424451b9, BRF_GRA }, \
	{ "redearth-simm5.1",				0x200000, 0x9b8cb56b, BRF_GRA },
};

STD_ROM_PICK(redearthnr1)
STD_ROM_FN(redearthnr1)

// ------------------------------------------------------------------------------------

static INT32 sfiiiInit()
{
	cps3_key1 = 0xb5fe053e;
	cps3_key2 = 0xfc03925a;
	cps3_isSpecial = 0;

	cps3_bios_test_hack = 0x000166b4;
	cps3_game_test_hack = 0x063cdff4;

	cps3_speedup_ram_address  = 0x0200cc6c;
	cps3_speedup_code_address = 0x06000884;

	cps3_region_address = 0x0001fec8;
	cps3_ncd_address    = 0x0001fecf;

	return cps3Init();
}

static INT32 sfiii2Init()
{
	cps3_key1 = 0x00000000;
	cps3_key2 = 0x00000000;
	cps3_isSpecial = 1;

	cps3_bios_test_hack = 0x00000000;
	cps3_game_test_hack = 0x00000000;

	cps3_speedup_ram_address  = 0x0200dfe4;
	cps3_speedup_code_address = 0x06000884;

	cps3_region_address = 0x0001fec8;
	cps3_ncd_address    = 0x0001fecf;

	return cps3Init();
}

static INT32 sfiii3Init()
{
	cps3_key1 = 0xa55432b4;
	cps3_key2 = 0x0c129981;
	cps3_isSpecial = 0;

	cps3_bios_test_hack = 0x00011c44;
	cps3_game_test_hack = 0x0613ab48;

	cps3_speedup_ram_address  = 0x0200d794;
	cps3_speedup_code_address = 0x06000884;

	cps3_region_address = 0x0001fec8;
	cps3_ncd_address    = 0x0001fecf;

	return cps3Init();
}

static INT32 jojor1Init()
{
	cps3_key1 = 0x02203ee3;
	cps3_key2 = 0x01301972;
	cps3_isSpecial = 0;

	cps3_bios_test_hack = 0x00011c2c;
	cps3_game_test_hack = 0x06172568;

	cps3_speedup_ram_address  = 0x020223d8;
	cps3_speedup_code_address = 0x0600065c;

	cps3_region_address = 0x0001fec8;
	cps3_ncd_address    = 0x0001fecf;

	return cps3Init();
}

static INT32 jojor2Init()
{
	cps3_key1 = 0x02203ee3;
	cps3_key2 = 0x01301972;
	cps3_isSpecial = 0;

	cps3_bios_test_hack = 0x00011c2c;
	cps3_game_test_hack = 0x06172568;

	cps3_speedup_ram_address  = 0x020223c0;
	cps3_speedup_code_address = 0x0600065c;

	cps3_region_address = 0x0001fec8;
	cps3_ncd_address    = 0x0001fecf;

	return cps3Init();
}

static INT32 jojobaInit()
{
	cps3_key1 = 0x23323ee3;
	cps3_key2 = 0x03021972;
	cps3_isSpecial = 0;

	cps3_bios_test_hack = 0x00011c90;
	cps3_game_test_hack = 0x061c45bc;

	cps3_speedup_ram_address  = 0x020267dc;
	cps3_speedup_code_address = 0x0600065c;

	cps3_region_address = 0x0001fec8;
	cps3_ncd_address    = 0x0001fecf;

	return cps3Init();
}

static INT32 redearthInit()
{
	cps3_key1 = 0x9e300ab1;
	cps3_key2 = 0xa175b82c;
	cps3_isSpecial = 0;

	cps3_bios_test_hack = 0x00016530;
	cps3_game_test_hack = 0x060105f0;

	cps3_speedup_ram_address  = 0x0202136c;
	cps3_speedup_code_address = 0x0600194e;

	cps3_region_address = 0x0001fed8;
	cps3_ncd_address    = 0x00000000;

	return cps3Init();
}


struct BurnDriver BurnDrvSfiii = {
	"sfiii", NULL, NULL, NULL, "1997",
	"Street Fighter III: New Generation (Europe 970204)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u65B0\u4E16\u4EE3 (\u6B27\u7248 970204)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, FBF_SF,
	NULL, sfiiiRomInfo, sfiiiRomName, NULL, NULL, NULL, NULL, cps3InputInfo, sfiiiwideeuroDIPInfo,
	sfiiiInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiiiu = {
	"sfiiiu", "sfiii", NULL, NULL, "1997",
	"Street Fighter III: New Generation (USA 970204)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u65B0\u4E16\u4EE3 (\u7F8E\u7248 970204)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, FBF_SF,
	NULL, sfiiiuRomInfo, sfiiiuRomName, NULL, NULL, NULL, NULL, cps3InputInfo, sfiiiwideusaDIPInfo,
	sfiiiInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiiia = {
	"sfiiia", "sfiii", NULL, NULL, "1997",
	"Street Fighter III: New Generation (Asia 970204)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u65B0\u4E16\u4EE3 (\u4E9A\u7248 970204)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, FBF_SF,
	NULL, sfiiiaRomInfo, sfiiiaRomName, NULL, NULL, NULL, NULL, cps3InputInfo, sfiiiwideasiaDIPInfo,
	sfiiiInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiiij = {
	"sfiiij", "sfiii", NULL, NULL, "1997",
	"Street Fighter III: New Generation (Japan 970204)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u65B0\u4E16\u4EE3 (\u65E5\u7248 970204)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, FBF_SF,
	NULL, sfiiijRomInfo, sfiiijRomName, NULL, NULL, NULL, NULL, cps3InputInfo, sfiiiwidejapanDIPInfo,
	sfiiiInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiiih = {
	"sfiiih", "sfiii", NULL, NULL, "1997",
	"Street Fighter III: New Generation (Hispanic 970204)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u65B0\u4E16\u4EE3 (\u897F\u7248 970204)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, FBF_SF,
	NULL, sfiiihRomInfo, sfiiihRomName, NULL, NULL, NULL, NULL, cps3InputInfo, sfiiiwidehispanicDIPInfo,
	sfiiiInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiiin = {
	"sfiiin", "sfiii", NULL, NULL, "1997",
	"Street Fighter III: New Generation (Asia 970204, NO CD, BIOS set 1)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u65B0\u4E16\u4EE3 (\u4E9A\u7248 970204, \u514D\u5149\u76D8, bios \u7B2C\u4E00\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, FBF_SF,
	NULL, sfiiinRomInfo, sfiiinRomName, NULL, NULL, NULL, NULL, cps3InputInfo, sfiiiwideasiaDIPInfo,
	sfiiiInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiiina = {
	"sfiiina", "sfiii", NULL, NULL, "1997",
	"Street Fighter III: New Generation (Asia 970204, NO CD, BIOS set 2)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u65B0\u4E16\u4EE3 (\u4E9A\u7248 970204, \u514D\u5149\u76D8, bios \u7B2C\u4E09\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, FBF_SF,
	NULL, sfiiinaRomInfo, sfiiinaRomName, NULL, NULL, NULL, NULL, cps3InputInfo, sfiiiwideasiaDIPInfo,
	sfiiiInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiii2 = {
	"sfiii2", NULL, NULL, NULL, "1997",
	"Street Fighter III 2nd Impact: Giant Attack (USA 970930)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u4E8C\u5EA6\u51B2\u51FB \u5DE8\u578B\u653B\u51FB (\u7F8E\u7248 970930)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii2RomInfo, sfiii2RomName, NULL, NULL, NULL, NULL, cps3InputInfo, sfiiiwideusaDIPInfo,
	sfiii2Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiii2j = {
	"sfiii2j", "sfiii2", NULL, NULL, "1997",
	"Street Fighter III 2nd Impact: Giant Attack (Japan 970930)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u4E8C\u5EA6\u51B2\u51FB \u5DE8\u578B\u653B\u51FB (\u65E5\u7248 970930)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii2jRomInfo, sfiii2jRomName, NULL, NULL, NULL, NULL, cps3InputInfo, sfiiiwidejapanDIPInfo,
	sfiii2Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiii2h = {
	"sfiii2h", "sfiii2", NULL, NULL, "1997",
	"Street Fighter III 2nd Impact: Giant Attack (Hispanic 970930)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u4E8C\u5EA6\u51B2\u51FB \u5DE8\u578B\u653B\u51FB (\u897F\u7248 970930)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii2hRomInfo, sfiii2hRomName, NULL, NULL, NULL, NULL, cps3InputInfo, sfiiiwidehispanicDIPInfo,
	sfiii2Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiii2n = {
	"sfiii2n", "sfiii2", NULL, NULL, "1997",
	"Street Fighter III 2nd Impact: Giant Attack (Asia 970930, NO CD)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u4E8C\u5EA6\u51B2\u51FB \u5DE8\u578B\u653B\u51FB (\u4E9A\u7248 970930, \u514D\u5149\u76D8)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii2nRomInfo, sfiii2nRomName, NULL, NULL, NULL, NULL, cps3InputInfo, sfiiiwideasiaDIPInfo,
	sfiii2Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiii3 = {
	"sfiii3", NULL, NULL, NULL, "1999",
	"Street Fighter III 3rd Strike: Fight for the Future (Europe 990608)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u4E09\u5EA6\u51B2\u51FB \u672A\u6765\u6218\u6597 (\u6B27\u7248 990608)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii3RomInfo, sfiii3RomName, NULL, NULL, NULL, NULL, cps3InputInfo, euroDIPInfo,
	sfiii3Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiii3u = {
	"sfiii3u", "sfiii3", NULL, NULL, "1999",
	"Street Fighter III 3rd Strike: Fight for the Future (USA 990608)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u4E09\u5EA6\u51B2\u51FB \u672A\u6765\u6218\u6597 (\u7F8E\u7248 990608)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii3uRomInfo, sfiii3uRomName, NULL, NULL, NULL, NULL, cps3InputInfo, usaDIPInfo,
	sfiii3Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiii3j = {
	"sfiii3j", "sfiii3", NULL, NULL, "1999",
	"Street Fighter III 3rd Strike: Fight for the Future (Japan 990608)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u4E09\u5EA6\u51B2\u51FB \u672A\u6765\u6218\u6597 (\u65E5\u7248 990608)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii3jRomInfo, sfiii3jRomName, NULL, NULL, NULL, NULL, cps3InputInfo, japanDIPInfo,
	sfiii3Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiii3n = {
	"sfiii3n", "sfiii3", NULL, NULL, "1999",
	"Street Fighter III 3rd Strike: Fight for the Future (Japan 990608, NO CD)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u4E09\u5EA6\u51B2\u51FB \u672A\u6765\u6218\u6597 (\u65E5\u7248 990608, \u514D\u5149\u76D8)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii3nRomInfo, sfiii3nRomName, NULL, NULL, NULL, NULL, cps3InputInfo, japanDIPInfo,
	sfiii3Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiii3na = {
	"sfiii3na", "sfiii3", NULL, NULL, "1999",
	"Street Fighter III 3rd Strike: Fight for the Future (Asia 990608, NO CD)\0", NULL, "Capcom", "CPS-3",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii3naRomInfo, sfiii3naRomName, NULL, NULL, NULL, NULL, cps3InputInfo, asiaDIPInfo,
	sfiii3Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiii3r1 = {
	"sfiii3r1", "sfiii3", NULL, NULL, "1999",
	"Street Fighter III 3rd Strike: Fight for the Future (Europe 990512)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u4E09\u5EA6\u51B2\u51FB \u672A\u6765\u6218\u6597 (\u6B27\u7248 990512)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii3r1RomInfo, sfiii3r1RomName, NULL, NULL, NULL, NULL, cps3InputInfo, euroDIPInfo,
	sfiii3Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiii3ur1 = {
	"sfiii3ur1", "sfiii3", NULL, NULL, "1999",
	"Street Fighter III 3rd Strike: Fight for the Future (USA 990512)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u4E09\u5EA6\u51B2\u51FB \u672A\u6765\u6218\u6597 (\u7F8E\u7248 990512)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii3ur1RomInfo, sfiii3ur1RomName, NULL, NULL, NULL, NULL, cps3InputInfo, usaDIPInfo,
	sfiii3Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiii3jr1 = {
	"sfiii3jr1", "sfiii3", NULL, NULL, "1999",
	"Street Fighter III 3rd Strike: Fight for the Future (Japan 990512)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u4E09\u5EA6\u51B2\u51FB \u672A\u6765\u6218\u6597 (\u65E5\u7248 990512)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii3jr1RomInfo, sfiii3jr1RomName, NULL, NULL, NULL, NULL, cps3InputInfo, japanDIPInfo,
	sfiii3Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiii3nr1 = {
	"sfiii3nr1", "sfiii3", NULL, NULL, "1999",
	"Street Fighter III 3rd Strike: Fight for the Future (Japan 990512, NO CD)\0", NULL, "Capcom", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u4E09\u5EA6\u51B2\u51FB \u672A\u6765\u6218\u6597 (\u65E5\u7248 990512, \u514D\u5149\u76D8)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii3nr1RomInfo, sfiii3nr1RomName, NULL, NULL, NULL, NULL, cps3InputInfo, japanDIPInfo,
	sfiii3Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvSfiii3nar1 = {
	"sfiii3nar1", "sfiii3", NULL, NULL, "1999",
	"Street Fighter III 3rd Strike: Fight for the Future (Asia 990512, NO CD)\0", NULL, "Capcom", "CPS-3",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii3nar1RomInfo, sfiii3nar1RomName, NULL, NULL, NULL, NULL, cps3InputInfo, asiaDIPInfo,
	sfiii3Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojo = {
	"jojo", NULL, NULL, NULL, "1998",
	"JoJo's Venture (Europe 990128)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669 (\u6B27\u7248 990128)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, jojoRomInfo, jojoRomName, NULL, NULL, NULL, NULL, jojoInputInfo, euroDIPInfo,
	jojor1Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojou = {
	"jojou", "jojo", NULL, NULL, "1998",
	"JoJo's Venture (USA 990128)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669 (\u7F8E\u7248 990128)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, jojouRomInfo, jojouRomName, NULL, NULL, NULL, NULL, jojoInputInfo, usaDIPInfo,
	jojor1Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojoa = {
	"jojoa", "jojo", NULL, NULL, "1998",
	"JoJo's Venture (Asia 990128)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669 (\u4E9A\u7248 990128)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, jojoaRomInfo, jojoaRomName, NULL, NULL, NULL, NULL, jojoInputInfo, asiaDIPInfo,
	jojor1Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojoj = {
	"jojoj", "jojo", NULL, NULL, "1998",
	"JoJo no Kimyou na Bouken (Japan 990128)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669 (\u65E5\u7248 990128)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, jojojRomInfo, jojojRomName, NULL, NULL, NULL, NULL, jojoInputInfo, japanDIPInfo,
	jojor1Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojon = {
	"jojon", "jojo", NULL, NULL, "1998",
	"JoJo's Venture (Asia 990128, NO CD)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669 (\u4E9A\u7248 990128, \u514D\u5149\u76D8)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, 0,
	NULL, jojonRomInfo, jojonRomName, NULL, NULL, NULL, NULL, jojoInputInfo, asiaDIPInfo,
	jojor1Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojor1 = {
	"jojor1", "jojo", NULL, NULL, "1998",
	"JoJo's Venture (Europe 990108)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669 (\u6B27\u7248 990108)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, jojor1RomInfo, jojor1RomName, NULL, NULL, NULL, NULL, jojoInputInfo, euroDIPInfo,
	jojor1Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojour1 = {
	"jojour1", "jojo", NULL, NULL, "1998",
	"JoJo's Venture (USA 990108)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669 (\u7F8E\u7248 990108)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, jojour1RomInfo, jojour1RomName, NULL, NULL, NULL, NULL, jojoInputInfo, usaDIPInfo,
	jojor1Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojoar1 = {
	"jojoar1", "jojo", NULL, NULL, "1998",
	"JoJo's Venture (Asia 990108)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669 (\u4E9A\u7248 990108)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, jojoar1RomInfo, jojoar1RomName, NULL, NULL, NULL, NULL, jojoInputInfo, asiaDIPInfo,
	jojor1Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojojr1 = {
	"jojojr1", "jojo", NULL, NULL, "1998",
	"JoJo no Kimyou na Bouken (Japan 990108)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669 (\u65E5\u7248 990108)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, jojojr1RomInfo, jojojr1RomName, NULL, NULL, NULL, NULL, jojoInputInfo, japanDIPInfo,
	jojor1Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojonr1 = {
	"jojonr1", "jojo", NULL, NULL, "1998",
	"JoJo's Venture (Asia 990108, NO CD)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669 (\u4E9A\u7248 990108, \u514D\u5149\u76D8)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, 0,
	NULL, jojonr1RomInfo, jojonr1RomName, NULL, NULL, NULL, NULL, jojoInputInfo, asiaDIPInfo,
	jojor1Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojor2 = {
	"jojor2", "jojo", NULL, NULL, "1998",
	"JoJo's Venture (Europe 981202)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669 (\u6B27\u7248 981202)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, jojor2RomInfo, jojor2RomName, NULL, NULL, NULL, NULL, jojoInputInfo, euroDIPInfo,
	jojor2Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojour2 = {
	"jojour2", "jojo", NULL, NULL, "1998",
	"JoJo's Venture (USA 981202)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669 (\u7F8E\u7248 981202)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, jojour2RomInfo, jojour2RomName, NULL, NULL, NULL, NULL, jojoInputInfo, usaDIPInfo,
	jojor2Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojoar2 = {
	"jojoar2", "jojo", NULL, NULL, "1998",
	"JoJo's Venture (Asia 981202)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669 (\u4E9A\u7248 981202)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, jojoar2RomInfo, jojoar2RomName, NULL, NULL, NULL, NULL, jojoInputInfo, asiaDIPInfo,
	jojor2Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojojr2 = {
	"jojojr2", "jojo", NULL, NULL, "1998",
	"JoJo no Kimyou na Bouken (Japan 981202)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669 (\u65E5\u7248 981202)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, jojojr2RomInfo, jojojr2RomName, NULL, NULL, NULL, NULL, jojoInputInfo, japanDIPInfo,
	jojor2Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojonr2 = {
	"jojonr2", "jojo", NULL, NULL, "1998",
	"JoJo's Venture (Asia 981202, NO CD)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669 (\u4E9A\u7248 981202, \u514D\u5149\u76D8)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, 0,
	NULL, jojonr2RomInfo, jojonr2RomName, NULL, NULL, NULL, NULL, jojoInputInfo, asiaDIPInfo,
	jojor2Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojoba = {
	"jojoba", NULL, NULL, NULL, "1999",
	"JoJo's Bizarre Adventure (Europe 991015, NO CD)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669: \u672A\u6765\u9057\u4EA7 (\u6B27\u7248 991015)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, 0,
	NULL, jojobaRomInfo, jojobaRomName, NULL, NULL, NULL, NULL, jojoInputInfo, jojobaneDIPInfo,
	jojobaInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojoban = {
	"jojoban", "jojoba", NULL, NULL, "1999",
	"JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 991015, NO CD)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669: \u672A\u6765\u9057\u4EA7 (\u65E5\u7248 991015, \u514D\u5149\u76D8)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, 0,
	NULL, jojobanRomInfo, jojobanRomName, NULL, NULL, NULL, NULL, jojoInputInfo, jojobaDIPInfo,
	jojobaInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojobar1 = {
	"jojobar1", "jojoba", NULL, NULL, "1999",
	"JoJo's Bizarre Adventure (Europe 990927)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669: \u672A\u6765\u9057\u4EA7 (\u6B27\u7248 990927)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, jojobar1RomInfo, jojobar1RomName, NULL, NULL, NULL, NULL, jojoInputInfo, jojobaneDIPInfo,
	jojobaInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojobajr1 = {
	"jojobajr1", "jojoba", NULL, NULL, "1999",
	"JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 990927)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669: \u672A\u6765\u9057\u4EA7 (\u65E5\u7248 990927)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, jojobajr1RomInfo, jojobajr1RomName, NULL, NULL, NULL, NULL, jojoInputInfo, jojobaDIPInfo,
	jojobaInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojobanr1 = {
	"jojobanr1", "jojoba", NULL, NULL, "1999",
	"JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 990927, NO CD)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669: \u672A\u6765\u9057\u4EA7 (\u65E5\u7248 990927, \u514D\u5149\u76D8)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, 0,
	NULL, jojobanr1RomInfo, jojobanr1RomName, NULL, NULL, NULL, NULL, jojoInputInfo, jojobaDIPInfo,
	jojobaInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojobaner1 = {
	"jojobaner1", "jojoba", NULL, NULL, "1999",
	"JoJo's Bizarre Adventure (Europe 990927, NO CD)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669: \u672A\u6765\u9057\u4EA7 (\u6B27\u7248 990927, \u514D\u5149\u76D8)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, 0,
	NULL, jojobaner1RomInfo, jojobaner1RomName, NULL, NULL, NULL, NULL, jojoInputInfo, jojobaneDIPInfo,
	jojobaInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojobar2 = {
	"jojobar2", "jojoba", NULL, NULL, "1999",
	"JoJo's Bizarre Adventure (Europe 990913)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669: \u672A\u6765\u9057\u4EA7 (\u6B27\u7248 990913)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, jojobar2RomInfo, jojobar2RomName, NULL, NULL, NULL, NULL, jojoInputInfo, jojobaneDIPInfo,
	jojobaInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojobajr2 = {
	"jojobajr2", "jojoba", NULL, NULL, "1999",
	"JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 990913)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669: \u672A\u6765\u9057\u4EA7 (\u65E5\u7248 990913)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, jojobajr2RomInfo, jojobajr2RomName, NULL, NULL, NULL, NULL, jojoInputInfo, jojobaDIPInfo,
	jojobaInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojobanr2 = {
	"jojobanr2", "jojoba", NULL, NULL, "1999",
	"JoJo no Kimyou na Bouken: Mirai e no Isan (Japan 990913, NO CD)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669: \u672A\u6765\u9057\u4EA7 (\u65E5\u7248 990913, \u514D\u5149\u76D8)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, 0,
	NULL, jojobanr2RomInfo, jojobanr2RomName, NULL, NULL, NULL, NULL, jojoInputInfo, jojobaDIPInfo,
	jojobaInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvJojobaner2 = {
	"jojobaner2", "jojoba", NULL, NULL, "1999",
	"JoJo's Bizarre Adventure (Europe 990913, NO CD)\0", NULL, "Capcom", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669: \u672A\u6765\u9057\u4EA7 (\u6B27\u7248 990913, \u514D\u5149\u76D8)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, 0,
	NULL, jojobaner2RomInfo, jojobaner2RomName, NULL, NULL, NULL, NULL, jojoInputInfo, jojobaneDIPInfo,
	jojobaInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvRedearth = {
	"redearth", NULL, NULL, NULL, "1996",
	"Red Earth (Europe 961121)\0", NULL, "Capcom", "CPS-3",
	L"\u8D64\u8272\u5927\u5730 (\u6B27\u7248 961121)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, redearthRomInfo, redearthRomName, NULL, NULL, NULL, NULL, cps3InputInfo, redearthDIPInfo,
	redearthInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvWarzard = {
	"warzard", "redearth", NULL, NULL, "1996",
	"Warzard (Japan 961121)\0", NULL, "Capcom", "CPS-3",
	L"\u8D64\u8272\u5927\u5730 (\u65E5\u7248 961121)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, warzardRomInfo, warzardRomName, NULL, NULL, NULL, NULL, cps3InputInfo, warzardDIPInfo,
	redearthInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvRedearthn = {
	"redearthn", "redearth", NULL, NULL, "1996",
	"Red Earth (Asia 961121, NO CD)\0", NULL, "Capcom", "CPS-3",
	L"\u8D64\u8272\u5927\u5730 (\u4E9A\u7248 961121, \u514D\u5149\u76D8)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, redearthnRomInfo, redearthnRomName, NULL, NULL, NULL, NULL, cps3InputInfo, redearthnDIPInfo,
	redearthInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvRedearthr1 = {
	"redearthr1", "redearth", NULL, NULL, "1996",
	"Red Earth (Europe 961023)\0", NULL, "Capcom", "CPS-3",
	L"\u8D64\u8272\u5927\u5730 (\u6B27\u7248 961023)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, redearthr1RomInfo, redearthr1RomName, NULL, NULL, NULL, NULL, cps3InputInfo, redearthDIPInfo,
	redearthInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvWarzardr1 = {
	"warzardr1", "redearth", NULL, NULL, "1996",
	"Warzard (Japan 961023)\0", NULL, "Capcom", "CPS-3",
	L"\u8D64\u8272\u5927\u5730 (\u65E5\u7248 961023)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, warzardr1RomInfo, warzardr1RomName, NULL, NULL, NULL, NULL, cps3InputInfo, warzardDIPInfo,
	redearthInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

struct BurnDriver BurnDrvRedearthnr1 = {
	"redearthnr1", "redearth", NULL, NULL, "1996",
	"Red Earth (Asia 961023, NO CD)\0", NULL, "Capcom", "CPS-3",
	L"\u8D64\u8272\u5927\u5730 (\u4E9A\u7248 961023, \u514D\u5149\u76D8)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3, GBF_VSFIGHT, 0,
	NULL, redearthnr1RomInfo, redearthnr1RomName, NULL, NULL, NULL, NULL, cps3InputInfo, redearthnDIPInfo,
	redearthInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};


// CPS-3 Hacks

// ----------------------------------------------
// Street Fighter III: New Generation (Boss Hack)
// ----------------------------------------------
static struct BurnRomInfo sfiiibhRomDesc[] = {

	{ "sfiii_asia_nocd.29f400.u2",			0x080000, 0xca2b715f, BRF_ESS | BRF_BIOS },	// SH-2 Bios

	{ "sfiiibh-simm1.0",		0x200000, 0x0beaed86, BRF_ESS | BRF_PRG },
	{ "sfiiibh-simm1.1",		0x200000, 0xe5c19320, BRF_ESS | BRF_PRG },
	{ "sfiiibh-simm1.2",		0x200000, 0x316b87c2, BRF_ESS | BRF_PRG },
	{ "sfiiibh-simm1.3",		0x200000, 0x46d63ec8, BRF_ESS | BRF_PRG },
	{ "sfiii-simm3.0",			0x200000, 0x080b3bd3, BRF_GRA },
	{ "sfiii-simm3.1",			0x200000, 0x5c356f2f, BRF_GRA },
	{ "sfiii-simm3.2",			0x200000, 0xf9c97a45, BRF_GRA },
	{ "sfiii-simm3.3",			0x200000, 0x09de3ead, BRF_GRA },
	{ "sfiii-simm3.4",			0x200000, 0x7dd7e1f3, BRF_GRA },
	{ "sfiii-simm3.5",			0x200000, 0x47a03a3a, BRF_GRA },
	{ "sfiii-simm3.6",			0x200000, 0xe9eb7a26, BRF_GRA },
	{ "sfiii-simm3.7",			0x200000, 0x7f44395c, BRF_GRA },
	{ "sfiii-simm4.0",			0x200000, 0x9ac080fc, BRF_GRA },
	{ "sfiii-simm4.1",			0x200000, 0x6e2c4c94, BRF_GRA },
	{ "sfiii-simm4.2",			0x200000, 0x8afc22d4, BRF_GRA },
	{ "sfiii-simm4.3",			0x200000, 0x9f3873b8, BRF_GRA },
	{ "sfiii-simm4.4",			0x200000, 0x166b3c97, BRF_GRA },
	{ "sfiii-simm4.5",			0x200000, 0xe5ea2547, BRF_GRA },
	{ "sfiii-simm4.6",			0x200000, 0xe85b9fdd, BRF_GRA },
	{ "sfiii-simm4.7",			0x200000, 0x362c01b7, BRF_GRA },
	{ "sfiii-simm5.0",			0x200000, 0x9bc108b2, BRF_GRA },
	{ "sfiii-simm5.1",			0x200000, 0xc6f1c066, BRF_GRA },
};

STD_ROM_PICK(sfiiibh)
STD_ROM_FN(sfiiibh)

struct BurnDriver BurnDrvSfiiibh = {
	"sfiiibh", "sfiii", NULL, NULL, "201?",
	"Street Fighter III: New Generation (Boss Hack)\0", NULL, "Yumeji", "CPS-3",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HACK | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, FBF_SF,
	NULL, sfiiibhRomInfo, sfiiibhRomName, NULL, NULL, NULL, NULL, cps3InputInfo, sfiiiwideasiaDIPInfo,
	sfiiiInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};


// -------------------------------------------------------
// Street Fighter III 2nd Impact: Giant Attack (Boss Hack)
// -------------------------------------------------------
static struct BurnRomInfo sfiii2bhRomDesc[] = {

	{ "sfiii2_asia_nocd.29f400.u2",			0x080000, 0xfd297c0d, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	{ "sfiii2bhv2-simm1.0",		0x200000, 0x1c74e097, BRF_ESS | BRF_PRG },
	{ "sfiii2bhv2-simm1.1",		0x200000, 0x3aa0f060, BRF_ESS | BRF_PRG },
	{ "sfiii2bhv2-simm1.2",		0x200000, 0x5a7adc60, BRF_ESS | BRF_PRG },
	{ "sfiii2bhv2-simm1.3",		0x200000, 0xe6cad3d5, BRF_ESS | BRF_PRG },
	{ "sfiii2-simm2.0",			0x200000, 0x94a4ce0f, BRF_ESS | BRF_PRG },
	{ "sfiii2-simm2.1",			0x200000, 0x67585033, BRF_ESS | BRF_PRG },
	{ "sfiii2-simm2.2",			0x200000, 0xfabffcd5, BRF_ESS | BRF_PRG },
	{ "sfiii2-simm2.3",			0x200000, 0x623c09ca, BRF_ESS | BRF_PRG },
	{ "sfiii2-simm3.0",			0x200000, 0xdab2d766, BRF_GRA },
	{ "sfiii2-simm3.1",			0x200000, 0x1f2aa34b, BRF_GRA },
	{ "sfiii2-simm3.2",			0x200000, 0x6f1a04eb, BRF_GRA },
	{ "sfiii2-simm3.3",			0x200000, 0xe05ef205, BRF_GRA },
	{ "sfiii2-simm3.4",			0x200000, 0xaffb074f, BRF_GRA },
	{ "sfiii2-simm3.5",			0x200000, 0x6962872e, BRF_GRA },
	{ "sfiii2-simm3.6",			0x200000, 0x6eed87de, BRF_GRA },
	{ "sfiii2-simm3.7",			0x200000, 0xe18f479e, BRF_GRA },
	{ "sfiii2-simm4.0",			0x200000, 0x764c2503, BRF_GRA },
	{ "sfiii2-simm4.1",			0x200000, 0x3e16af6e, BRF_GRA },
	{ "sfiii2-simm4.2",			0x200000, 0x215705e6, BRF_GRA },
	{ "sfiii2-simm4.3",			0x200000, 0xe30cbd9c, BRF_GRA },
	{ "sfiii2-simm4.4",			0x200000, 0x4185ded9, BRF_GRA },
	{ "sfiii2-simm4.5",			0x200000, 0x4e8db013, BRF_GRA },
	{ "sfiii2-simm4.6",			0x200000, 0x08df48ce, BRF_GRA },
	{ "sfiii2-simm4.7",			0x200000, 0xbb8f80a5, BRF_GRA },
	{ "sfiii2-simm5.0",			0x200000, 0xebdc4787, BRF_GRA },
	{ "sfiii2-simm5.1",			0x200000, 0x6b7c550e, BRF_GRA },
	{ "sfiii2-simm5.2",			0x200000, 0x56ff8c50, BRF_GRA },
	{ "sfiii2-simm5.3",			0x200000, 0x3f2ac3e9, BRF_GRA },
	{ "sfiii2-simm5.4",			0x200000, 0x48cda50e, BRF_GRA },
	{ "sfiii2-simm5.5",			0x200000, 0x520c0af6, BRF_GRA },
	{ "sfiii2-simm5.6",			0x200000, 0x2edc5986, BRF_GRA },
	{ "sfiii2-simm5.7",			0x200000, 0x93ffa199, BRF_GRA },
};

STD_ROM_PICK(sfiii2bh)
STD_ROM_FN(sfiii2bh)

struct BurnDriver BurnDrvSfiii2bh = {
	"sfiii2bh", "sfiii2", NULL, NULL, "201?",
	"Street Fighter III 2nd Impact: Giant Attack (Boss Hack)\0", NULL, "Yumeji", "CPS-3",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HACK | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii2bhRomInfo, sfiii2bhRomName, NULL, NULL, NULL, NULL, cps3InputInfo, sfiiiwideasiaDIPInfo,
	sfiii2Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};


// ---------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future (Boss Hack)
// ---------------------------------------------------------------
static struct BurnRomInfo sfiii3bhRomDesc[] = {

	{ "sfiii3_japan_nocd.29f400.u2",		0x080000, 0x1edc6366, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	{ "sfiii3bh-simm1.0",		0x200000, 0x2753fad5, BRF_ESS | BRF_PRG },
	{ "sfiii3bh-simm1.1",		0x200000, 0xfca5f8e6, BRF_ESS | BRF_PRG },
	{ "sfiii3bh-simm1.2",		0x200000, 0xcd6367b3, BRF_ESS | BRF_PRG },
	{ "sfiii3bh-simm1.3",		0x200000, 0xe5c4245a, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.0",			0x200000, 0x06eb969e, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.1",			0x200000, 0xe7039f82, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.2",			0x200000, 0x645c96f7, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.3",			0x200000, 0x610efab1, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm3.0",			0x200000, 0x7baa1f79, BRF_GRA },
	{ "sfiii3-simm3.1",			0x200000, 0x234bf8fe, BRF_GRA },
	{ "sfiii3-simm3.2",			0x200000, 0xd9ebc308, BRF_GRA },
	{ "sfiii3-simm3.3",			0x200000, 0x293cba77, BRF_GRA },
	{ "sfiii3-simm3.4",			0x200000, 0x6055e747, BRF_GRA },
	{ "sfiii3-simm3.5",			0x200000, 0x499aa6fc, BRF_GRA },
	{ "sfiii3-simm3.6",			0x200000, 0x6c13879e, BRF_GRA },
	{ "sfiii3-simm3.7",			0x200000, 0xcf4f8ede, BRF_GRA },
	{ "sfiii3-simm4.0",			0x200000, 0x091fd5ba, BRF_GRA },
	{ "sfiii3-simm4.1",			0x200000, 0x0bca8917, BRF_GRA },
	{ "sfiii3-simm4.2",			0x200000, 0xa0fd578b, BRF_GRA },
	{ "sfiii3-simm4.3",			0x200000, 0x4bf8c699, BRF_GRA },
	{ "sfiii3-simm4.4",			0x200000, 0x137b8785, BRF_GRA },
	{ "sfiii3-simm4.5",			0x200000, 0x4fb70671, BRF_GRA },
	{ "sfiii3-simm4.6",			0x200000, 0x832374a4, BRF_GRA },
	{ "sfiii3-simm4.7",			0x200000, 0x1c88576d, BRF_GRA },
	{ "sfiii3-simm5.0",			0x200000, 0xc67d9190, BRF_GRA },
	{ "sfiii3-simm5.1",			0x200000, 0x6cb79868, BRF_GRA },
	{ "sfiii3-simm5.2",			0x200000, 0xdf69930e, BRF_GRA },
	{ "sfiii3-simm5.3",			0x200000, 0x333754e0, BRF_GRA },
	{ "sfiii3-simm5.4",			0x200000, 0x78f6d417, BRF_GRA },
	{ "sfiii3-simm5.5",			0x200000, 0x8ccad9b1, BRF_GRA },
	{ "sfiii3-simm5.6",			0x200000, 0x85de59e5, BRF_GRA },
	{ "sfiii3-simm5.7",			0x200000, 0xee7e29b3, BRF_GRA },
	{ "sfiii3-simm6.0",			0x200000, 0x8da69042, BRF_GRA },
	{ "sfiii3-simm6.1",			0x200000, 0x1c8c7ac4, BRF_GRA },
	{ "sfiii3-simm6.2",			0x200000, 0xa671341d, BRF_GRA },
	{ "sfiii3-simm6.3",			0x200000, 0x1a990249, BRF_GRA },
	{ "sfiii3-simm6.4",			0x200000, 0x20cb39ac, BRF_GRA },
	{ "sfiii3-simm6.5",			0x200000, 0x5f844b2f, BRF_GRA },
	{ "sfiii3-simm6.6",			0x200000, 0x450e8d28, BRF_GRA },
	{ "sfiii3-simm6.7",			0x200000, 0xcc5f4187, BRF_GRA },
};

STD_ROM_PICK(sfiii3bh)
STD_ROM_FN(sfiii3bh)


struct BurnDriver BurnDrvSfiii3bh = {
	"sfiii3bh", "sfiii3", NULL, NULL, "201?",
	"Street Fighter III 3rd Strike: Fight for the Future (Boss Hack)\0", NULL, "Yumeji", "CPS-3",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HACK | BDF_HISCORE_SUPPORTED, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii3bhRomInfo, sfiii3bhRomName, NULL, NULL, NULL, NULL, cps3InputInfo, japanDIPInfo,
	sfiii3Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};


// ------------------------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future (4rd Arrange Edition 2013)
// ------------------------------------------------------------------------------

static struct BurnRomInfo sfiii4nRomDesc[] = {

	{ "sfiii3_japan_nocd.29f400.u2",	0x080000, 0x1edc6366, BRF_ESS | BRF_BIOS },	// SH-2 Bios

	{ "4rd-simm1.0",					0x200000, 0xca97f95e, BRF_ESS | BRF_PRG },
	{ "4rd-simm1.1",					0x200000, 0x5bc7faa6, BRF_ESS | BRF_PRG },
	{ "4rd-simm1.2",					0x200000, 0xec2eaa29, BRF_ESS | BRF_PRG },
	{ "4rd-simm1.3",					0x200000, 0x41c4ce7c, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.0",					0x200000, 0x06eb969e, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.1",					0x200000, 0xe7039f82, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.2",					0x200000, 0x645c96f7, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.3",					0x200000, 0x610efab1, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm3.0",					0x200000, 0x7baa1f79, BRF_GRA },
	{ "sfiii3-simm3.1",					0x200000, 0x234bf8fe, BRF_GRA },
	{ "sfiii3-simm3.2",					0x200000, 0xd9ebc308, BRF_GRA },
	{ "sfiii3-simm3.3",					0x200000, 0x293cba77, BRF_GRA },
	{ "sfiii3-simm3.4",					0x200000, 0x6055e747, BRF_GRA },
	{ "sfiii3-simm3.5",					0x200000, 0x499aa6fc, BRF_GRA },
	{ "sfiii3-simm3.6",					0x200000, 0x6c13879e, BRF_GRA },
	{ "sfiii3-simm3.7",					0x200000, 0xcf4f8ede, BRF_GRA },
	{ "sfiii3-simm4.0",					0x200000, 0x091fd5ba, BRF_GRA },
	{ "sfiii3-simm4.1",					0x200000, 0x0bca8917, BRF_GRA },
	{ "sfiii3-simm4.2",					0x200000, 0xa0fd578b, BRF_GRA },
	{ "sfiii3-simm4.3",					0x200000, 0x4bf8c699, BRF_GRA },
	{ "sfiii3-simm4.4",					0x200000, 0x137b8785, BRF_GRA },
	{ "sfiii3-simm4.5",					0x200000, 0x4fb70671, BRF_GRA },
	{ "sfiii3-simm4.6",					0x200000, 0x832374a4, BRF_GRA },
	{ "sfiii3-simm4.7",					0x200000, 0x1c88576d, BRF_GRA },
	{ "sfiii3-simm5.0",					0x200000, 0xc67d9190, BRF_GRA },
	{ "sfiii3-simm5.1",					0x200000, 0x6cb79868, BRF_GRA },
	{ "sfiii3-simm5.2",					0x200000, 0xdf69930e, BRF_GRA },
	{ "sfiii3-simm5.3",					0x200000, 0x333754e0, BRF_GRA },
	{ "sfiii3-simm5.4",					0x200000, 0x78f6d417, BRF_GRA },
	{ "sfiii3-simm5.5",					0x200000, 0x8ccad9b1, BRF_GRA },
	{ "4rd-simm5.6",					0x200000, 0xe9b61a56, BRF_GRA },
	{ "4rd-simm5.7",					0x200000, 0x8db3a249, BRF_GRA },
	{ "sfiii3-simm6.0",					0x200000, 0x8da69042, BRF_GRA },
	{ "sfiii3-simm6.1",					0x200000, 0x1c8c7ac4, BRF_GRA },
	{ "sfiii3-simm6.2",					0x200000, 0xa671341d, BRF_GRA },
	{ "sfiii3-simm6.3",					0x200000, 0x1a990249, BRF_GRA },
	{ "sfiii3-simm6.4",					0x200000, 0x20cb39ac, BRF_GRA },
	{ "sfiii3-simm6.5",					0x200000, 0x5f844b2f, BRF_GRA },
	{ "sfiii3-simm6.6",					0x200000, 0x450e8d28, BRF_GRA },
	{ "sfiii3-simm6.7",					0x200000, 0xcc5f4187, BRF_GRA }
};

STD_ROM_PICK(sfiii4n)
STD_ROM_FN(sfiii4n)

struct BurnDriver BurnDrvSfiii4n = {
	"sfiii4n", "sfiii3", NULL, NULL, "2013",
	"Street Fighter III 3rd Strike: Fight for the Future (4rd Arrange Edition 2013)\0", NULL, "hack", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u4E09\u5EA6\u51B2\u51FB \u672A\u6765\u6218\u6597 (\u7B2C\u56DB\u6B21\u66F4\u65B0 2013)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HACK, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii4nRomInfo, sfiii4nRomName, NULL, NULL, NULL, NULL, cps3InputInfo, japanDIPInfo,
	sfiii3Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};


// -------------------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future | re:THIRD update
// -------------------------------------------------------------------------
// PalMod by riiya [20210419]
// Patches applied
// 1 - Original VS Screen
// 1 - Default HUD
// -------------------------------------------------------------------------

static struct BurnRomInfo sfiii3thRomDesc[] = {

	{ "sfiii3_japan_nocd.29f400.u2",	0x080000, 0x1edc6366, BRF_ESS | BRF_BIOS },	// SH-2 Bios

	{ "sfiii3-simm1.0",					0x200000, 0x11dfd3cd, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm1.1",					0x200000, 0xc50585e6, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm1.2",					0x200000, 0x8e011d9b, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm1.3",					0x200000, 0xdca8d92f, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.0",					0x200000, 0x06eb969e, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.1",					0x200000, 0xe7039f82, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.2",					0x200000, 0x645c96f7, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.3",					0x200000, 0x610efab1, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm3.0",					0x200000, 0x7baa1f79, BRF_GRA },
	{ "sfiii3-simm3.1",					0x200000, 0x234bf8fe, BRF_GRA },
	{ "sfiii3-simm3.2",					0x200000, 0xd9ebc308, BRF_GRA },
	{ "sfiii3-simm3.3",					0x200000, 0x293cba77, BRF_GRA },
	{ "sfiii3-simm3.4",					0x200000, 0x6055e747, BRF_GRA },
	{ "sfiii3-simm3.5",					0x200000, 0x499aa6fc, BRF_GRA },
	{ "sfiii3-simm3.6",					0x200000, 0x6c13879e, BRF_GRA },
	{ "sfiii3-simm3.7",					0x200000, 0xcf4f8ede, BRF_GRA },
	{ "sfiii3-simm4.0",					0x200000, 0x091fd5ba, BRF_GRA },
	{ "sfiii3-simm4.1",					0x200000, 0x0bca8917, BRF_GRA },
	{ "sfiii3-simm4.2",					0x200000, 0xa0fd578b, BRF_GRA },
	{ "sfiii3-simm4.3",					0x200000, 0x4bf8c699, BRF_GRA },
	{ "sfiii3-simm4.4",					0x200000, 0x137b8785, BRF_GRA },
	{ "sfiii3-simm4.5",					0x200000, 0x4fb70671, BRF_GRA },
	{ "sfiii3-simm4.6",					0x200000, 0x832374a4, BRF_GRA },
	{ "sfiii3-simm4.7",					0x200000, 0x1c88576d, BRF_GRA },
	{ "sfiii3-simm5.0",					0x200000, 0xc67d9190, BRF_GRA },
	{ "sfiii3-simm5.1",					0x200000, 0x6cb79868, BRF_GRA },
	{ "sfiii3-simm5.2",					0x200000, 0xdf69930e, BRF_GRA },
	{ "sfiii3-simm5.3",					0x200000, 0x333754e0, BRF_GRA },
	{ "sfiii3-simm5.4",					0x200000, 0x78f6d417, BRF_GRA },
	{ "sfiii3-simm5.5",					0x200000, 0x8ccad9b1, BRF_GRA },
	{ "3rd-simm5.6",					0x200000, 0x7fffb6d4, BRF_GRA },
	{ "3rd-simm5.7",					0x200000, 0x2dfcd4a6, BRF_GRA },
	{ "sfiii3-simm6.0",					0x200000, 0x8da69042, BRF_GRA },
	{ "sfiii3-simm6.1",					0x200000, 0x1c8c7ac4, BRF_GRA },
	{ "sfiii3-simm6.2",					0x200000, 0xa671341d, BRF_GRA },
	{ "sfiii3-simm6.3",					0x200000, 0x1a990249, BRF_GRA },
	{ "sfiii3-simm6.4",					0x200000, 0x20cb39ac, BRF_GRA },
	{ "sfiii3-simm6.5",					0x200000, 0x5f844b2f, BRF_GRA },
	{ "sfiii3-simm6.6",					0x200000, 0x450e8d28, BRF_GRA },
	{ "sfiii3-simm6.7",					0x200000, 0xcc5f4187, BRF_GRA }
};

STD_ROM_PICK(sfiii3th)
STD_ROM_FN(sfiii3th)

struct BurnDriver BurnDrvSfiii3th = {
	"sfiii3th", "sfiii3", NULL, NULL, "2021",
	"Street Fighter III 3rd Strike: Fight for the Future | re:THIRD update\0", NULL, "hack", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u4E09\u5EA6\u51B2\u51FB \u672A\u6765\u6218\u6597 \u007C re: \u7B2C\u4E09\u6B21\u66F4\u65B0\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HACK, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii3thRomInfo, sfiii3thRomName, NULL, NULL, NULL, NULL, cps3InputInfo, japanDIPInfo,
	sfiii3Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

// -------------------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future (Widescreen 20220908)
// -------------------------------------------------------------------------

static struct BurnRomInfo sfiii3wsRomDesc[] = {

	{ "sfiii3_japan_nocd.29f400.u2",	0x080000, 0x1edc6366, BRF_ESS | BRF_BIOS },	// SH-2 Bios

	{ "sfiii3ws-simm1.0",				0x200000, 0x16843828, BRF_ESS | BRF_PRG },
	{ "sfiii3ws-simm1.1",				0x200000, 0x0d084fbc, BRF_ESS | BRF_PRG },
	{ "sfiii3ws-simm1.2",				0x200000, 0x3f6d1f52, BRF_ESS | BRF_PRG },
	{ "sfiii3ws-simm1.3",				0x200000, 0x3ff7516b, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.0",					0x200000, 0x06eb969e, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.1",					0x200000, 0xe7039f82, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.2",					0x200000, 0x645c96f7, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.3",					0x200000, 0x610efab1, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm3.0",					0x200000, 0x7baa1f79, BRF_GRA },
	{ "sfiii3-simm3.1",					0x200000, 0x234bf8fe, BRF_GRA },
	{ "sfiii3-simm3.2",					0x200000, 0xd9ebc308, BRF_GRA },
	{ "sfiii3-simm3.3",					0x200000, 0x293cba77, BRF_GRA },
	{ "sfiii3-simm3.4",					0x200000, 0x6055e747, BRF_GRA },
	{ "sfiii3-simm3.5",					0x200000, 0x499aa6fc, BRF_GRA },
	{ "sfiii3-simm3.6",					0x200000, 0x6c13879e, BRF_GRA },
	{ "sfiii3-simm3.7",					0x200000, 0xcf4f8ede, BRF_GRA },
	{ "sfiii3-simm4.0",					0x200000, 0x091fd5ba, BRF_GRA },
	{ "sfiii3-simm4.1",					0x200000, 0x0bca8917, BRF_GRA },
	{ "sfiii3-simm4.2",					0x200000, 0xa0fd578b, BRF_GRA },
	{ "sfiii3-simm4.3",					0x200000, 0x4bf8c699, BRF_GRA },
	{ "sfiii3-simm4.4",					0x200000, 0x137b8785, BRF_GRA },
	{ "sfiii3-simm4.5",					0x200000, 0x4fb70671, BRF_GRA },
	{ "sfiii3-simm4.6",					0x200000, 0x832374a4, BRF_GRA },
	{ "sfiii3-simm4.7",					0x200000, 0x1c88576d, BRF_GRA },
	{ "sfiii3-simm5.0",					0x200000, 0xc67d9190, BRF_GRA },
	{ "sfiii3-simm5.1",					0x200000, 0x6cb79868, BRF_GRA },
	{ "sfiii3-simm5.2",					0x200000, 0xdf69930e, BRF_GRA },
	{ "sfiii3-simm5.3",					0x200000, 0x333754e0, BRF_GRA },
	{ "sfiii3-simm5.4",					0x200000, 0x78f6d417, BRF_GRA },
	{ "sfiii3-simm5.5",					0x200000, 0x8ccad9b1, BRF_GRA },
	{ "sfiii3-simm5.6",					0x200000, 0x85de59e5, BRF_GRA },
	{ "sfiii3-simm5.7",					0x200000, 0xee7e29b3, BRF_GRA },
	{ "sfiii3-simm6.0",					0x200000, 0x8da69042, BRF_GRA },
	{ "sfiii3-simm6.1",					0x200000, 0x1c8c7ac4, BRF_GRA },
	{ "sfiii3-simm6.2",					0x200000, 0xa671341d, BRF_GRA },
	{ "sfiii3-simm6.3",					0x200000, 0x1a990249, BRF_GRA },
	{ "sfiii3-simm6.4",					0x200000, 0x20cb39ac, BRF_GRA },
	{ "sfiii3-simm6.5",					0x200000, 0x5f844b2f, BRF_GRA },
	{ "sfiii3-simm6.6",					0x200000, 0x450e8d28, BRF_GRA },
	{ "sfiii3-simm6.7",					0x200000, 0xcc5f4187, BRF_GRA }
};

STD_ROM_PICK(sfiii3ws)
STD_ROM_FN(sfiii3ws)

struct BurnDriver BurnDrvSfiii3ws = {
	"sfiii3ws", "sfiii3", NULL, NULL, "2022",
	"Street Fighter III 3rd Strike: Fight for the Future (Widescreen 20220908)\0", NULL, "Bank Bank", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u4E09\u5EA6\u51B2\u51FB \u672A\u6765\u6218\u6597 (\u5BBD\u5C4F 20220908)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HACK, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii3wsRomInfo, sfiii3wsRomName, NULL, NULL, NULL, NULL, cps3InputInfo, japanDIPInfo,
	sfiii3Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	496, 224, 16, 9
};

// ---------------------------------------------------------------------------
// Street Fighter III 3rd Strike: Fight for the Future (4rd Strike 2011-07-17)
// ---------------------------------------------------------------------------

static struct BurnRomInfo sfiii4fsRomDesc[] = {

	{ "sfiii3_japan_nocd.29f400.u2",	0x080000, 0x1edc6366, BRF_ESS | BRF_BIOS },	// SH-2 Bios

	{ "sfiii4fs-simm1.0",				0x200000, 0xcd044113, BRF_ESS | BRF_PRG },
	{ "sfiii4fs-simm1.1",				0x200000, 0xb6334750, BRF_ESS | BRF_PRG },
	{ "sfiii4fs-simm1.2",				0x200000, 0x48fed039, BRF_ESS | BRF_PRG },
	{ "sfiii4fs-simm1.3",				0x200000, 0x130a29b5, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.0",					0x200000, 0x06eb969e, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.1",					0x200000, 0xe7039f82, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.2",					0x200000, 0x645c96f7, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm2.3",					0x200000, 0x610efab1, BRF_ESS | BRF_PRG },
	{ "sfiii3-simm3.0",					0x200000, 0x7baa1f79, BRF_GRA },
	{ "sfiii3-simm3.1",					0x200000, 0x234bf8fe, BRF_GRA },
	{ "sfiii3-simm3.2",					0x200000, 0xd9ebc308, BRF_GRA },
	{ "sfiii3-simm3.3",					0x200000, 0x293cba77, BRF_GRA },
	{ "sfiii3-simm3.4",					0x200000, 0x6055e747, BRF_GRA },
	{ "sfiii3-simm3.5",					0x200000, 0x499aa6fc, BRF_GRA },
	{ "sfiii3-simm3.6",					0x200000, 0x6c13879e, BRF_GRA },
	{ "sfiii3-simm3.7",					0x200000, 0xcf4f8ede, BRF_GRA },
	{ "sfiii3-simm4.0",					0x200000, 0x091fd5ba, BRF_GRA },
	{ "sfiii3-simm4.1",					0x200000, 0x0bca8917, BRF_GRA },
	{ "sfiii3-simm4.2",					0x200000, 0xa0fd578b, BRF_GRA },
	{ "sfiii3-simm4.3",					0x200000, 0x4bf8c699, BRF_GRA },
	{ "sfiii3-simm4.4",					0x200000, 0x137b8785, BRF_GRA },
	{ "sfiii3-simm4.5",					0x200000, 0x4fb70671, BRF_GRA },
	{ "sfiii3-simm4.6",					0x200000, 0x832374a4, BRF_GRA },
	{ "sfiii3-simm4.7",					0x200000, 0x1c88576d, BRF_GRA },
	{ "sfiii3-simm5.0",					0x200000, 0xc67d9190, BRF_GRA },
	{ "sfiii3-simm5.1",					0x200000, 0x6cb79868, BRF_GRA },
	{ "sfiii3-simm5.2",					0x200000, 0xdf69930e, BRF_GRA },
	{ "sfiii3-simm5.3",					0x200000, 0x333754e0, BRF_GRA },
	{ "sfiii3-simm5.4",					0x200000, 0x78f6d417, BRF_GRA },
	{ "sfiii3-simm5.5",					0x200000, 0x8ccad9b1, BRF_GRA },
	{ "sfiii3-simm5.6",					0x200000, 0x85de59e5, BRF_GRA },
	{ "sfiii3-simm5.7",					0x200000, 0xee7e29b3, BRF_GRA },
	{ "sfiii3-simm6.0",					0x200000, 0x8da69042, BRF_GRA },
	{ "sfiii3-simm6.1",					0x200000, 0x1c8c7ac4, BRF_GRA },
	{ "sfiii3-simm6.2",					0x200000, 0xa671341d, BRF_GRA },
	{ "sfiii3-simm6.3",					0x200000, 0x1a990249, BRF_GRA },
	{ "sfiii3-simm6.4",					0x200000, 0x20cb39ac, BRF_GRA },
	{ "sfiii3-simm6.5",					0x200000, 0x5f844b2f, BRF_GRA },
	{ "sfiii3-simm6.6",					0x200000, 0x450e8d28, BRF_GRA },
	{ "sfiii3-simm6.7",					0x200000, 0xcc5f4187, BRF_GRA }
};

STD_ROM_PICK(sfiii4fs)
STD_ROM_FN(sfiii4fs)

struct BurnDriver BurnDrvsfiii4fs = {
	"sfiii4fs", "sfiii3", NULL, NULL, "2011",
	"Street Fighter III 3rd Strike: Fight for the Future (4rd Strike 2011-07-17)\0", NULL, "hack", "CPS-3",
	L"\u8857\u5934\u9738\u738B III \u4E09\u5EA6\u51B2\u51FB \u672A\u6765\u6218\u6597 (\u56DB\u5EA6\u51B2\u51FB 2011-07-17)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HACK, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, FBF_SF,
	NULL, sfiii4fsRomInfo, sfiii4fsRomName, NULL, NULL, NULL, NULL, cps3InputInfo, japanDIPInfo,
	sfiii3Init, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};

// ----------------------------------------------------
// JoJo's Bizarre Adventure Rainbow Edition v1.0.1 2021
// ----------------------------------------------------

static struct BurnRomInfo jojobanrbRomDesc[] = {
	{ "jojoba_japan_nocd.29f400.u2",	0x080000, 0x4dab19f5, BRF_ESS | BRF_BIOS },	// SH-2 Bios
	
	{ "jojobanrb-simm1.0",				0x200000, 0x3b75c72e, BRF_ESS | BRF_PRG }, 
	{ "jojobanrb-simm1.1",				0x200000, 0x9f0d57f6, BRF_ESS | BRF_PRG }, 
	{ "jojobanrb-simm1.2",				0x200000, 0x480184c2, BRF_ESS | BRF_PRG }, 
	{ "jojobanrb-simm1.3",				0x200000, 0x2ddb747c, BRF_ESS | BRF_PRG }, 
	{ "jojobanrb-simm2.0",				0x200000, 0x7d618904, BRF_ESS | BRF_PRG }, 
	{ "jojobanrb-simm2.1",				0x200000, 0x882f76a0, BRF_ESS | BRF_PRG }, 
	{ "jojobanrb-simm2.2",				0x200000, 0x58ed6139, BRF_ESS | BRF_PRG }, 
	{ "jojobanrb-simm2.3",				0x200000, 0x64c19159, BRF_ESS | BRF_PRG }, 
	{ "jojobanrb-simm3.0",				0x200000, 0xc579314a, BRF_GRA }, 
	{ "jojobanrb-simm3.1",				0x200000, 0x2dc0682d, BRF_GRA }, 
	{ "jojobanrb-simm3.2",				0x200000, 0xeb3470ff, BRF_GRA }, 
	{ "jojobanrb-simm3.3",				0x200000, 0xb9d1fb4f, BRF_GRA }, 
	{ "jojobanrb-simm3.4",				0x200000, 0xbb8a7b4d, BRF_GRA }, 
	{ "jojobanrb-simm3.5",				0x200000, 0xb5e2a429, BRF_GRA }, 
	{ "jojoba-simm3.6",					0x200000, 0xa10ec5af, BRF_GRA }, 
	{ "jojoba-simm3.7",					0x200000, 0x0bd0de7a, BRF_GRA }, 
	{ "jojoba-simm4.0",					0x200000, 0x6ea14adc, BRF_GRA }, 
	{ "jojoba-simm4.1",					0x200000, 0x8f4c42fb, BRF_GRA }, 
	{ "jojoba-simm4.2",					0x200000, 0xef0586d1, BRF_GRA }, 
	{ "jojoba-simm4.3",					0x200000, 0x93ccc470, BRF_GRA }, 
	{ "jojoba-simm4.4",					0x200000, 0x3d9ec7d2, BRF_GRA }, 
	{ "jojoba-simm4.5",					0x200000, 0x03e66850, BRF_GRA }, 
	{ "jojoba-simm4.6",					0x200000, 0x01606ac3, BRF_GRA }, 
	{ "jojoba-simm4.7",					0x200000, 0x36392b87, BRF_GRA }, 
	{ "jojoba-simm5.0",					0x200000, 0x2ef8c60c, BRF_GRA }, 
	{ "jojoba-simm5.1",					0x200000, 0xcf7d7ca6, BRF_GRA }, 
	{ "jojoba-simm5.2",					0x200000, 0xb7815bfa, BRF_GRA }, 
	{ "jojoba-simm5.3",					0x200000, 0x9bfec049, BRF_GRA }, 
	{ "jojobanrb-simm5.4",				0x200000, 0xf475f126, BRF_GRA }, 
	{ "jojobanrb-simm5.5",				0x200000, 0xb028b441, BRF_GRA }, 
	{ "jojoba-simm5.6",					0x200000, 0x4fb32906, BRF_GRA }, 
	{ "jojoba-simm5.7",					0x200000, 0x8c8be520, BRF_GRA }
};

STD_ROM_PICK(jojobanrb)
STD_ROM_FN(jojobanrb)

struct BurnDriver BurnDrvJojobanrb = {
	"jojobanrb", "jojoba", NULL, NULL, "2021",
	"JoJo's Bizarre Adventure (Rainbow Edition v1.0.1 2021)\0", NULL, "hack", "CPS-3",
	L"\u4E54\u4E54\u7684\u5947\u5999\u5192\u9669: \u672A\u6765\u9057\u4EA7 (\u5F69\u8679\u7248 v1.0.1 2021)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_16BIT_ONLY | BDF_CLONE | BDF_HACK, 2, HARDWARE_CAPCOM_CPS3 | HARDWARE_CAPCOM_CPS3_NO_CD, GBF_VSFIGHT, 0,
	NULL, jojobanrbRomInfo, jojobanrbRomName, NULL, NULL, NULL, NULL, jojoInputInfo, jojobaDIPInfo,
	jojobaInit, cps3Exit, cps3Frame, DrvCps3Draw, cps3Scan, &cps3_palette_change, 0x40000,
	384, 224, 4, 3
};
