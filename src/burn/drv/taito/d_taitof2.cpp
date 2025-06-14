// Based on MAME driver by David Graves, Bryan McPhail, Brad Oliver, Andrew Prime, Brian Troha, Nicola Salmoria

// Todo:
//  what next?

#include "tiles_generic.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "taito.h"
#include "taito_ic.h"
#include "burn_ym2610.h"
#include "burn_ym2203.h"
#include "msm6295.h"

static INT32 Footchmp = 0;
static INT32 YesnoDip;
static INT32 MjnquestInput;
static INT32 DriveoutSoundNibble;
static INT32 DriveoutOkiBank;
static INT32 Driftout = 0;
static INT32 bNoClearOpposites = 0;

INT32 TaitoF2SpriteType;

INT32 TaitoF2SpritesFlipScreen;
INT32 TaitoF2PrepareSprites;
INT32 TaitoF2SpritesDisabled, TaitoF2SpritesActiveArea, TaitoF2SpritesMasterScrollX, TaitoF2SpritesMasterScrollY;
INT32 TaitoF2SpriteBlendMode;
UINT16 TaitoF2SpriteBank[8];
UINT16 TaitoF2SpriteBankBuffered[8];
UINT8 TaitoF2TilePriority[5];
UINT8 TaitoF2SpritePriority[4];
struct TaitoF2SpriteEntry *TaitoF2SpriteList;
static TaitoF2SpriteBufferUpdate TaitoF2SpriteBufferFunction;

static INT32 PaletteType = 0;
enum { TAITOF2Palette = 0, METALBPalette = 1, QZQUESTPalette = 2 };

static INT32 SpritePriWritebackMode = 0; // no priority writeback for sprite blender

#ifdef BUILD_A68K
static bool bUseAsm68KCoreOldValue = false;
#endif

#define A(a, b, c, d) {a, b, (UINT8*)(c), d}

static struct BurnInputInfo CameltryInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0220IOCInputPort0 + 7, "p1 start"  },
	A("P1 Paddle"        , BIT_ANALOG_REL, &TaitoAnalogPort0      , "p1 x-axis" ),
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "p1 fire 1" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 7, "p2 start"  },
	A("P2 Paddle"        , BIT_ANALOG_REL, &TaitoAnalogPort1      , "p2 x-axis" ),
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Cameltry)

static struct BurnInputInfo DeadconxInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TaitoInputPort2 + 0, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TaitoInputPort0 + 7, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TaitoInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TaitoInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TaitoInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TaitoInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TaitoInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TaitoInputPort0 + 5, "p1 fire 2" },

	{"P2 Coin"           , BIT_DIGITAL   , TaitoInputPort2 + 1, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TaitoInputPort1 + 7, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TaitoInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TaitoInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TaitoInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TaitoInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TaitoInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TaitoInputPort1 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , TaitoInputPort2 + 4, "service"   },
	{"Service 2"         , BIT_DIGITAL   , TaitoInputPort2 + 5, "service 2" },
	{"Service 3"         , BIT_DIGITAL   , TaitoInputPort2 + 6, "service 3" },
	{"Tilt"              , BIT_DIGITAL   , TaitoInputPort2 + 7, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TaitoDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TaitoDip + 1       , "dip"       },
};

STDINPUTINFO(Deadconx)

static struct BurnInputInfo DinorexInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 7, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TC0510NIOInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 6, "p1 fire 3" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 7, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TC0510NIOInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 6, "p2 fire 3" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0510NIOInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0510NIODip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0510NIODip + 1       , "dip"       },
};

STDINPUTINFO(Dinorex)

static struct BurnInputInfo DondokodInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0220IOCInputPort0 + 7, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TC0220IOCInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 5, "p1 fire 2" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 7, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Dondokod)

static struct BurnInputInfo DriftoutInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 7, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TC0510NIOInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 5, "p1 fire 2" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 3, "p2 coin"   },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0510NIOInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0510NIODip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0510NIODip + 1       , "dip"       },
};

STDINPUTINFO(Driftout)

static struct BurnInputInfo FinalbInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0220IOCInputPort0 + 7, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TC0220IOCInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 6, "p1 fire 3" },
	{"P1 Fire 4"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 4, "p1 fire 4" },
	{"P1 Fire 5"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 5, "p1 fire 5" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 7, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 6, "p2 fire 3" },
	{"P2 Fire 4"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 6, "p2 fire 4" },
	{"P2 Fire 5"         , BIT_DIGITAL   , TC0220IOCInputPort2 + 7, "p2 fire 5" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Finalb)

static struct BurnInputInfo FootchmpInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TaitoInputPort2 + 0, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TaitoInputPort0 + 7, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TaitoInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TaitoInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TaitoInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TaitoInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TaitoInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TaitoInputPort0 + 5, "p1 fire 2" },

	{"P2 Coin"           , BIT_DIGITAL   , TaitoInputPort2 + 1, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TaitoInputPort1 + 7, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TaitoInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TaitoInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TaitoInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TaitoInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TaitoInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TaitoInputPort1 + 5, "p2 fire 2" },

	{"P3 Coin"           , BIT_DIGITAL   , TaitoInputPort2 + 2, "p3 coin"   },
	{"P3 Start"          , BIT_DIGITAL   , TaitoInputPort3 + 7, "p3 start"  },
	{"P3 Up"             , BIT_DIGITAL   , TaitoInputPort3 + 0, "p3 up"     },
	{"P3 Down"           , BIT_DIGITAL   , TaitoInputPort3 + 1, "p3 down"   },
	{"P3 Left"           , BIT_DIGITAL   , TaitoInputPort3 + 2, "p3 left"   },
	{"P3 Right"          , BIT_DIGITAL   , TaitoInputPort3 + 3, "p3 right"  },
	{"P3 Fire 1"         , BIT_DIGITAL   , TaitoInputPort3 + 4, "p3 fire 1" },
	{"P3 Fire 2"         , BIT_DIGITAL   , TaitoInputPort3 + 5, "p3 fire 2" },

	{"P4 Coin"           , BIT_DIGITAL   , TaitoInputPort2 + 3, "p4 coin"   },
	{"P4 Start"          , BIT_DIGITAL   , TaitoInputPort4 + 7, "p4 start"  },
	{"P4 Up"             , BIT_DIGITAL   , TaitoInputPort4 + 0, "p4 up"     },
	{"P4 Down"           , BIT_DIGITAL   , TaitoInputPort4 + 1, "p4 down"   },
	{"P4 Left"           , BIT_DIGITAL   , TaitoInputPort4 + 2, "p4 left"   },
	{"P4 Right"          , BIT_DIGITAL   , TaitoInputPort4 + 3, "p4 right"  },
	{"P4 Fire 1"         , BIT_DIGITAL   , TaitoInputPort4 + 4, "p4 fire 1" },
	{"P4 Fire 2"         , BIT_DIGITAL   , TaitoInputPort4 + 5, "p4 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , TaitoInputPort2 + 4, "service"   },
	{"Service 2"         , BIT_DIGITAL   , TaitoInputPort2 + 5, "service 2" },
	{"Service 3"         , BIT_DIGITAL   , TaitoInputPort2 + 6, "service 3" },
	{"Tilt"              , BIT_DIGITAL   , TaitoInputPort2 + 7, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TaitoDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TaitoDip + 1       , "dip"       },
};

STDINPUTINFO(Footchmp)

static struct BurnInputInfo GrowlInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TaitoInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TaitoInputPort0 + 7, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TaitoInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TaitoInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TaitoInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TaitoInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TaitoInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TaitoInputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TaitoInputPort0 + 6, "p1 fire 3" },

	{"P2 Coin"           , BIT_DIGITAL   , TaitoInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TaitoInputPort1 + 7, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TaitoInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TaitoInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TaitoInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TaitoInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TaitoInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TaitoInputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TaitoInputPort1 + 6, "p2 fire 3" },

	{"P3 Coin"           , BIT_DIGITAL   , TaitoInputPort5 + 0, "p3 coin"   },
	{"P3 Start"          , BIT_DIGITAL   , TaitoInputPort3 + 7, "p3 start"  },
	{"P3 Up"             , BIT_DIGITAL   , TaitoInputPort3 + 0, "p3 up"     },
	{"P3 Down"           , BIT_DIGITAL   , TaitoInputPort3 + 1, "p3 down"   },
	{"P3 Left"           , BIT_DIGITAL   , TaitoInputPort3 + 2, "p3 left"   },
	{"P3 Right"          , BIT_DIGITAL   , TaitoInputPort3 + 3, "p3 right"  },
	{"P3 Fire 1"         , BIT_DIGITAL   , TaitoInputPort3 + 4, "p3 fire 1" },
	{"P3 Fire 2"         , BIT_DIGITAL   , TaitoInputPort3 + 5, "p3 fire 2" },
	{"P3 Fire 3"         , BIT_DIGITAL   , TaitoInputPort3 + 6, "p3 fire 3" },

	{"P4 Coin"           , BIT_DIGITAL   , TaitoInputPort5 + 1, "p4 coin"   },
	{"P4 Start"          , BIT_DIGITAL   , TaitoInputPort4 + 7, "p4 start"  },
	{"P4 Up"             , BIT_DIGITAL   , TaitoInputPort4 + 0, "p4 up"     },
	{"P4 Down"           , BIT_DIGITAL   , TaitoInputPort4 + 1, "p4 down"   },
	{"P4 Left"           , BIT_DIGITAL   , TaitoInputPort4 + 2, "p4 left"   },
	{"P4 Right"          , BIT_DIGITAL   , TaitoInputPort4 + 3, "p4 right"  },
	{"P4 Fire 1"         , BIT_DIGITAL   , TaitoInputPort4 + 4, "p4 fire 1" },
	{"P4 Fire 2"         , BIT_DIGITAL   , TaitoInputPort4 + 5, "p4 fire 2" },
	{"P4 Fire 3"         , BIT_DIGITAL   , TaitoInputPort4 + 6, "p4 fire 3" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , TaitoInputPort2 + 1, "service"   },
	{"Service 2"         , BIT_DIGITAL   , TaitoInputPort5 + 2, "service 2" },
	{"Tilt"              , BIT_DIGITAL   , TaitoInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TaitoDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TaitoDip + 1       , "dip"       },
};

STDINPUTINFO(Growl)

static struct BurnInputInfo GunfrontInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 7, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TC0510NIOInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 5, "p1 fire 2" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 7, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TC0510NIOInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0510NIOInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0510NIODip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0510NIODip + 1       , "dip"       },
};

STDINPUTINFO(Gunfront)

static struct BurnInputInfo KoshienInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 3, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TC0510NIOInputPort2 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0510NIOInputPort2 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 0, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 1, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 2, "p1 fire 3" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 7, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TC0510NIOInputPort2 + 4, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 5, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 6, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TC0510NIOInputPort2 + 7, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 6, "p2 fire 3" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0510NIOInputPort1 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0510NIODip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0510NIODip + 1       , "dip"       },
};

STDINPUTINFO(Koshien)

static struct BurnInputInfo LiquidkInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0220IOCInputPort0 + 7, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TC0220IOCInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 5, "p1 fire 2" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 7, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Liquidk)

static struct BurnInputInfo MegablstInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0220IOCInputPort2 + 4, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TC0220IOCInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 5, "p1 fire 2" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0220IOCInputPort2 + 5, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Megablst)

static struct BurnInputInfo MetalbInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 7, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TC0510NIOInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 6, "p1 fire 3" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 7, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TC0510NIOInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 6, "p2 fire 3" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0510NIOInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0510NIODip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0510NIODip + 1       , "dip"       },
};

STDINPUTINFO(Metalb)

static struct BurnInputInfo MjnquestInputList[] =
{
	{"P1 Coin"            , BIT_DIGITAL   , TaitoInputPort5 + 1, "p1 coin"   },
	{"P1 Start"           , BIT_DIGITAL   , TaitoInputPort0 + 5, "p1 start"  },
	{"P1 A"               , BIT_DIGITAL   , TaitoInputPort0 + 0, "mah a"     },
	{"P1 B"               , BIT_DIGITAL   , TaitoInputPort1 + 0, "mah b"     },
	{"P1 C"               , BIT_DIGITAL   , TaitoInputPort2 + 0, "mah c"     },
	{"P1 D"               , BIT_DIGITAL   , TaitoInputPort3 + 0, "mah d"     },
	{"P1 E"               , BIT_DIGITAL   , TaitoInputPort0 + 1, "mah e"     },
	{"P1 F"               , BIT_DIGITAL   , TaitoInputPort1 + 1, "mah f"     },
	{"P1 G"               , BIT_DIGITAL   , TaitoInputPort2 + 1, "mah g"     },
	{"P1 H"               , BIT_DIGITAL   , TaitoInputPort3 + 1, "mah h"     },
	{"P1 I"               , BIT_DIGITAL   , TaitoInputPort0 + 2, "mah i"     },
	{"P1 J"               , BIT_DIGITAL   , TaitoInputPort1 + 2, "mah j"     },
	{"P1 K"               , BIT_DIGITAL   , TaitoInputPort2 + 2, "mah k"     },
	{"P1 L"               , BIT_DIGITAL   , TaitoInputPort3 + 2, "mah l"     },
	{"P1 M"               , BIT_DIGITAL   , TaitoInputPort0 + 3, "mah m"     },
	{"P1 N"               , BIT_DIGITAL   , TaitoInputPort1 + 3, "mah n"     },
	{"P1 Kan"             , BIT_DIGITAL   , TaitoInputPort0 + 4, "mah kan"   },
	{"P1 Pon"             , BIT_DIGITAL   , TaitoInputPort3 + 3, "mah pon"   },
	{"P1 Chi"             , BIT_DIGITAL   , TaitoInputPort2 + 3, "mah chi"   },
	{"P1 Reach"           , BIT_DIGITAL   , TaitoInputPort1 + 4, "mah reach" },
	{"P1 Ron"             , BIT_DIGITAL   , TaitoInputPort2 + 4, "mah ron"   },

	{"P2 Coin"            , BIT_DIGITAL   , TaitoInputPort4 + 1, "p2 coin"   },

	{"Reset"              , BIT_DIGITAL   , &TaitoReset        , "reset"     },
	{"Service"            , BIT_DIGITAL   , TaitoInputPort5 + 0, "service"   },
	{"Tilt"               , BIT_DIGITAL   , TaitoInputPort4 + 0, "tilt"      },
	{"Dip 1"              , BIT_DIPSWITCH , TaitoDip + 0       , "dip"       },
	{"Dip 2"              , BIT_DIPSWITCH , TaitoDip + 1       , "dip"       },
};

STDINPUTINFO(Mjnquest)

static struct BurnInputInfo NinjakInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TaitoInputPort2 + 4, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TaitoInputPort0 + 7, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TaitoInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TaitoInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TaitoInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TaitoInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TaitoInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TaitoInputPort0 + 5, "p1 fire 2" },

	{"P2 Coin"           , BIT_DIGITAL   , TaitoInputPort2 + 5, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TaitoInputPort1 + 7, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TaitoInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TaitoInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TaitoInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TaitoInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TaitoInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TaitoInputPort1 + 5, "p2 fire 2" },

	{"P3 Coin"           , BIT_DIGITAL   , TaitoInputPort2 + 6, "p3 coin"   },
	{"P3 Start"          , BIT_DIGITAL   , TaitoInputPort3 + 7, "p3 start"  },
	{"P3 Up"             , BIT_DIGITAL   , TaitoInputPort3 + 0, "p3 up"     },
	{"P3 Down"           , BIT_DIGITAL   , TaitoInputPort3 + 1, "p3 down"   },
	{"P3 Left"           , BIT_DIGITAL   , TaitoInputPort3 + 2, "p3 left"   },
	{"P3 Right"          , BIT_DIGITAL   , TaitoInputPort3 + 3, "p3 right"  },
	{"P3 Fire 1"         , BIT_DIGITAL   , TaitoInputPort3 + 4, "p3 fire 1" },
	{"P3 Fire 2"         , BIT_DIGITAL   , TaitoInputPort3 + 5, "p3 fire 2" },

	{"P4 Coin"           , BIT_DIGITAL   , TaitoInputPort2 + 7, "p4 coin"   },
	{"P4 Start"          , BIT_DIGITAL   , TaitoInputPort4 + 7, "p4 start"  },
	{"P4 Up"             , BIT_DIGITAL   , TaitoInputPort4 + 0, "p4 up"     },
	{"P4 Down"           , BIT_DIGITAL   , TaitoInputPort4 + 1, "p4 down"   },
	{"P4 Left"           , BIT_DIGITAL   , TaitoInputPort4 + 2, "p4 left"   },
	{"P4 Right"          , BIT_DIGITAL   , TaitoInputPort4 + 3, "p4 right"  },
	{"P4 Fire 1"         , BIT_DIGITAL   , TaitoInputPort4 + 4, "p4 fire 1" },
	{"P4 Fire 2"         , BIT_DIGITAL   , TaitoInputPort4 + 5, "p4 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , TaitoInputPort2 + 1, "service"   },
	{"Service 2"         , BIT_DIGITAL   , TaitoInputPort2 + 2, "service 2" },
	{"Tilt"              , BIT_DIGITAL   , TaitoInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TaitoDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TaitoDip + 1       , "dip"       },
};

STDINPUTINFO(Ninjak)

static struct BurnInputInfo PulirulaInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 7, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TC0510NIOInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 6, "p1 fire 3" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 7, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TC0510NIOInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 6, "p2 fire 3" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0510NIOInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0510NIODip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0510NIODip + 1       , "dip"       },
};

STDINPUTINFO(Pulirula)

static struct BurnInputInfo QcrayonInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 7, "p1 start"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 0, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 1, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 3, "p1 fire 3" },
	{"P1 Fire 4"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 2, "p1 fire 4" },
	{"P1 Fire 5"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 4, "p1 fire 5" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 7, "p2 start"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 0, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 1, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 3, "p2 fire 3" },
	{"P2 Fire 4"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 2, "p2 fire 4" },
	{"P2 Fire 5"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 4, "p2 fire 5" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0510NIOInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0510NIODip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0510NIODip + 1       , "dip"       },
};

STDINPUTINFO(Qcrayon)

static struct BurnInputInfo Qcrayon2InputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 7, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TC0510NIOInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 4, "p1 fire 1" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 7, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TC0510NIOInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 4, "p2 fire 1" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0510NIOInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0510NIODip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0510NIODip + 1       , "dip"       },
};

STDINPUTINFO(Qcrayon2)

static struct BurnInputInfo QjinseiInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 7, "p1 start"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 0, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 1, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 3, "p1 fire 3" },
	{"P1 Fire 4"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 2, "p1 fire 4" },
	{"P1 Fire 5"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 4, "p1 fire 5" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 7, "p2 start"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 0, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 1, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 3, "p2 fire 3" },
	{"P2 Fire 4"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 2, "p2 fire 4" },
	{"P2 Fire 5"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 4, "p2 fire 5" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0510NIOInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0510NIODip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0510NIODip + 1       , "dip"       },
};

STDINPUTINFO(Qjinsei)

static struct BurnInputInfo QtorimonInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0220IOCInputPort0 + 7, "p1 start"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 0, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 1, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "p1 fire 3" },
	{"P1 Fire 4"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "p1 fire 4" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 7, "p2 start"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 0, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 1, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 3, "p2 fire 3" },
	{"P2 Fire 4"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 2, "p2 fire 4" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Qtorimon)

static struct BurnInputInfo QuizhqInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TaitoInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TaitoInputPort0 + 7, "p1 start"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TaitoInputPort0 + 0, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TaitoInputPort0 + 1, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TaitoInputPort0 + 3, "p1 fire 3" },
	{"P1 Fire 4"         , BIT_DIGITAL   , TaitoInputPort0 + 2, "p1 fire 4" },
	{"P1 Fire 5"         , BIT_DIGITAL   , TaitoInputPort0 + 4, "p1 fire 5" },

	{"P2 Coin"           , BIT_DIGITAL   , TaitoInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TaitoInputPort1 + 7, "p2 start"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TaitoInputPort1 + 0, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TaitoInputPort1 + 1, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TaitoInputPort1 + 3, "p2 fire 3" },
	{"P2 Fire 4"         , BIT_DIGITAL   , TaitoInputPort1 + 2, "p2 fire 4" },
	{"P2 Fire 5"         , BIT_DIGITAL   , TaitoInputPort1 + 4, "p2 fire 5" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , TaitoInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TaitoInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TaitoDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TaitoDip + 1       , "dip"       },
};

STDINPUTINFO(Quizhq)

static struct BurnInputInfo QzchikyuInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 7, "p1 start"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 0, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 1, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 3, "p1 fire 3" },
	{"P1 Fire 4"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 2, "p1 fire 4" },
	{"P1 Fire 5"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 4, "p1 fire 5" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 7, "p2 start"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 0, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 1, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 3, "p2 fire 3" },
	{"P2 Fire 4"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 2, "p2 fire 4" },
	{"P2 Fire 5"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 5, "p2 fire 5" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0510NIOInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0510NIODip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0510NIODip + 1       , "dip"       },
};

STDINPUTINFO(Qzchikyu)

static struct BurnInputInfo QzquestInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 7, "p1 start"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 0, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 1, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 3, "p1 fire 3" },
	{"P1 Fire 4"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 2, "p1 fire 4" },
	{"P1 Fire 5"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 4, "p1 fire 5" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 7, "p2 start"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 0, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 1, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 3, "p2 fire 3" },
	{"P2 Fire 4"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 2, "p2 fire 4" },
	{"P2 Fire 5"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 5, "p2 fire 5" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0510NIOInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0510NIODip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0510NIODip + 1       , "dip"       },
};

STDINPUTINFO(Qzquest)

static struct BurnInputInfo SolfigtrInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TaitoInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TaitoInputPort0 + 7, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TaitoInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TaitoInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TaitoInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TaitoInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TaitoInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TaitoInputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TaitoInputPort0 + 6, "p1 fire 3" },

	{"P2 Coin"           , BIT_DIGITAL   , TaitoInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TaitoInputPort1 + 7, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TaitoInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TaitoInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TaitoInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TaitoInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TaitoInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TaitoInputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TaitoInputPort1 + 6, "p2 fire 3" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , TaitoInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TaitoInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TaitoDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TaitoDip + 1       , "dip"       },
};

STDINPUTINFO(Solfigtr)

static struct BurnInputInfo SsiInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 7, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TC0510NIOInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0510NIOInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 5, "p1 fire 2" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 7, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TC0510NIOInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TC0510NIOInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 5, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0510NIOInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0510NIODip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0510NIODip + 1       , "dip"       },
};

STDINPUTINFO(Ssi)

static struct BurnInputInfo ThundfoxInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0220IOCInputPort0 + 7, "p1 start"  },
	{"P1 Up"             , BIT_DIGITAL   , TC0220IOCInputPort0 + 0, "p1 up"     },
	{"P1 Down"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 1, "p1 down"   },
	{"P1 Left"           , BIT_DIGITAL   , TC0220IOCInputPort0 + 2, "p1 left"   },
	{"P1 Right"          , BIT_DIGITAL   , TC0220IOCInputPort0 + 3, "p1 right"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 4, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 5, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TC0220IOCInputPort0 + 6, "p1 fire 3" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 7, "p2 start"  },
	{"P2 Up"             , BIT_DIGITAL   , TC0220IOCInputPort1 + 0, "p2 up"     },
	{"P2 Down"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 1, "p2 down"   },
	{"P2 Left"           , BIT_DIGITAL   , TC0220IOCInputPort1 + 2, "p2 left"   },
	{"P2 Right"          , BIT_DIGITAL   , TC0220IOCInputPort1 + 3, "p2 right"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 4, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 5, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TC0220IOCInputPort1 + 6, "p2 fire 3" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0220IOCInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0220IOCInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0220IOCDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0220IOCDip + 1       , "dip"       },
};

STDINPUTINFO(Thundfox)

static struct BurnInputInfo YesnojInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TaitoInputPort1 + 2, "p1 coin"   },
	{"P1 Fire 1"         , BIT_DIGITAL   , TaitoInputPort0 + 0, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TaitoInputPort0 + 1, "p1 fire 2" },

	{"P2 Coin"           , BIT_DIGITAL   , TaitoInputPort1 + 3, "p2 coin"   },
	{"P2 Fire 1"         , BIT_DIGITAL   , TaitoInputPort0 + 2, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TaitoInputPort0 + 3, "p2 fire 2" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset        , "reset"     },
	{"Service"           , BIT_DIGITAL   , TaitoInputPort1 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TaitoInputPort1 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TaitoDip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TaitoDip + 1       , "dip"       },
};

STDINPUTINFO(Yesnoj)

static struct BurnInputInfo YuyugogoInputList[] =
{
	{"P1 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 2, "p1 coin"   },
	{"P1 Start"          , BIT_DIGITAL   , TC0510NIOInputPort0 + 7, "p1 start"  },
	{"P1 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 0, "p1 fire 1" },
	{"P1 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 1, "p1 fire 2" },
	{"P1 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 3, "p1 fire 3" },
	{"P1 Fire 4"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 2, "p1 fire 4" },
	{"P1 Fire 5"         , BIT_DIGITAL   , TC0510NIOInputPort0 + 4, "p1 fire 5" },

	{"P2 Coin"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 3, "p2 coin"   },
	{"P2 Start"          , BIT_DIGITAL   , TC0510NIOInputPort1 + 7, "p2 start"  },
	{"P2 Fire 1"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 0, "p2 fire 1" },
	{"P2 Fire 2"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 1, "p2 fire 2" },
	{"P2 Fire 3"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 3, "p2 fire 3" },
	{"P2 Fire 4"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 2, "p2 fire 4" },
	{"P2 Fire 5"         , BIT_DIGITAL   , TC0510NIOInputPort1 + 4, "p2 fire 5" },

	{"Reset"             , BIT_DIGITAL   , &TaitoReset            , "reset"     },
	{"Service"           , BIT_DIGITAL   , TC0510NIOInputPort2 + 1, "service"   },
	{"Tilt"              , BIT_DIGITAL   , TC0510NIOInputPort2 + 0, "tilt"      },
	{"Dip 1"             , BIT_DIPSWITCH , TC0510NIODip + 0       , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , TC0510NIODip + 1       , "dip"       },
};

STDINPUTINFO(Yuyugogo)

#undef A

static inline void DrvClearOppositesCommon(UINT8* nJoystickInputs)
{
	if ((*nJoystickInputs & 0x03) == 0x00) {
		*nJoystickInputs |= 0x03;
	}
	if ((*nJoystickInputs & 0x0c) == 0x00) {
		*nJoystickInputs |= 0x0c;
	}
}

static void TC0220IOCMakeInputs()
{
	// Reset Inputs
	TC0220IOCInput[0] = 0xff;
	TC0220IOCInput[1] = 0xff;
	TC0220IOCInput[2] = 0xff;

	for (INT32 i = 0; i < 8; i++) {
		TC0220IOCInput[0] -= (TC0220IOCInputPort0[i] & 1) << i;
		TC0220IOCInput[1] -= (TC0220IOCInputPort1[i] & 1) << i;
		TC0220IOCInput[2] -= (TC0220IOCInputPort2[i] & 1) << i;
	}

	if (bNoClearOpposites == 0) {
		DrvClearOppositesCommon(&TC0220IOCInput[0]);
		DrvClearOppositesCommon(&TC0220IOCInput[1]);
	}
}

static void TC0510NIOMakeInputs()
{
	// Reset Inputs
	TC0510NIOInput[0] = 0xff;
	TC0510NIOInput[1] = 0xff;
	TC0510NIOInput[2] = 0xff;

	for (INT32 i = 0; i < 8; i++) {
		TC0510NIOInput[0] -= (TC0510NIOInputPort0[i] & 1) << i;
		TC0510NIOInput[1] -= (TC0510NIOInputPort1[i] & 1) << i;
		TC0510NIOInput[2] -= (TC0510NIOInputPort2[i] & 1) << i;
	}

	if (bNoClearOpposites == 0) {
		DrvClearOppositesCommon(&TC0510NIOInput[0]);
		DrvClearOppositesCommon(&TC0510NIOInput[1]);
	}
}

static void TaitoF2MakeInputs()
{
	// Reset Inputs
	TaitoInput[0] = 0xff;
	TaitoInput[1] = 0xff;
	TaitoInput[2] = 0xff;
	TaitoInput[3] = 0xff;
	TaitoInput[4] = 0xff;
	TaitoInput[5] = 0xff;

	for (INT32 i = 0; i < 8; i++) {
		TaitoInput[0] -= (TaitoInputPort0[i] & 1) << i;
		TaitoInput[1] -= (TaitoInputPort1[i] & 1) << i;
		TaitoInput[2] -= (TaitoInputPort2[i] & 1) << i;
		TaitoInput[3] -= (TaitoInputPort3[i] & 1) << i;
		TaitoInput[4] -= (TaitoInputPort4[i] & 1) << i;
		TaitoInput[5] -= (TaitoInputPort5[i] & 1) << i;
	}

	if (bNoClearOpposites == 0) {
		DrvClearOppositesCommon(&TaitoInput[0]);
		DrvClearOppositesCommon(&TaitoInput[1]);
		DrvClearOppositesCommon(&TaitoInput[3]);
		DrvClearOppositesCommon(&TaitoInput[4]);
	}
}

static struct BurnDIPInfo CameltryDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0xfe, NULL                             },
	{0x0c, 0xff, 0xff, 0x7f, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x0b, 0x01, 0x01, 0x00, "Upright"                        },
	{0x0b, 0x01, 0x01, 0x01, "Cocktail"                       },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x0b, 0x01, 0x02, 0x02, "Off"                            },
	{0x0b, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0b, 0x01, 0x04, 0x04, "Off"                            },
	{0x0b, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0b, 0x01, 0x08, 0x00, "Off"                            },
	{0x0b, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coinage"                        },
	{0x0b, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Price to Continue"              },
	{0x0b, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x0b, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0xc0, 0x80, "1 Coin 1 Credit"                },
	{0x0b, 0x01, 0xc0, 0xc0, "Same as start"                  },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0c, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0c, 0x01, 0x03, 0x03, "Medium"                         },
	{0x0c, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0c, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Start remain time"              },
	{0x0c, 0x01, 0x0c, 0x00, "35"                             },
	{0x0c, 0x01, 0x0c, 0x04, "40"                             },
	{0x0c, 0x01, 0x0c, 0x0c, "50"                             },
	{0x0c, 0x01, 0x0c, 0x08, "60"                             },

	{0   , 0xfe, 0   , 4   , "Continue play time"             },
	{0x0c, 0x01, 0x30, 0x00, "+20"                            },
	{0x0c, 0x01, 0x30, 0x10, "+25"                            },
	{0x0c, 0x01, 0x30, 0x30, "+30"                            },
	{0x0c, 0x01, 0x30, 0x20, "+40"                            },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x0c, 0x01, 0x40, 0x00, "Off"                            },
	{0x0c, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x0c, 0x01, 0x80, 0x80, "Single"                         },
	{0x0c, 0x01, 0x80, 0x00, "Dual"                           },
};

STDDIPINFO(Cameltry)

static struct BurnDIPInfo CameltrjDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0xfe, NULL                             },
	{0x0c, 0xff, 0xff, 0x7f, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x0b, 0x01, 0x01, 0x00, "Upright"                        },
	{0x0b, 0x01, 0x01, 0x01, "Cocktail"                       },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x0b, 0x01, 0x02, 0x02, "Off"                            },
	{0x0b, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0b, 0x01, 0x04, 0x04, "Off"                            },
	{0x0b, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0b, 0x01, 0x08, 0x00, "Off"                            },
	{0x0b, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0b, 0x01, 0x30, 0x00, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x0b, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x0b, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0b, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0b, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x0b, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x0b, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0c, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0c, 0x01, 0x03, 0x03, "Medium"                         },
	{0x0c, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0c, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Start remain time"              },
	{0x0c, 0x01, 0x0c, 0x00, "35"                             },
	{0x0c, 0x01, 0x0c, 0x04, "40"                             },
	{0x0c, 0x01, 0x0c, 0x0c, "50"                             },
	{0x0c, 0x01, 0x0c, 0x08, "60"                             },

	{0   , 0xfe, 0   , 4   , "Continue play time"             },
	{0x0c, 0x01, 0x30, 0x00, "+20"                            },
	{0x0c, 0x01, 0x30, 0x10, "+25"                            },
	{0x0c, 0x01, 0x30, 0x30, "+30"                            },
	{0x0c, 0x01, 0x30, 0x20, "+40"                            },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x0c, 0x01, 0x40, 0x00, "Off"                            },
	{0x0c, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x0c, 0x01, 0x80, 0x80, "Single"                         },
	{0x0c, 0x01, 0x80, 0x00, "Dual"                           },
};

STDDIPINFO(Cameltrj)

static struct BurnDIPInfo DeadconxDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xfe, NULL                             },
	{0x16, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x15, 0x01, 0x01, 0x00, "Upright"                        },
	{0x15, 0x01, 0x01, 0x01, "Cocktail"                       },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x15, 0x01, 0x02, 0x02, "Off"                            },
	{0x15, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x15, 0x01, 0x04, 0x04, "Off"                            },
	{0x15, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x15, 0x01, 0x08, 0x00, "Off"                            },
	{0x15, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x15, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x15, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x15, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x15, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Number of Enemies 1/2 Player"	  },
	{0x16, 0x01, 0x03, 0x01, "25/45 (Easy)"					  },
	{0x16, 0x01, 0x03, 0x03, "30/50 (Medium)"				  },
	{0x16, 0x01, 0x03, 0x02, "40/60 (Hard)"					  },
	{0x16, 0x01, 0x03, 0x00, "50/70 (Hardest)"				  },

	{0   , 0xfe, 0   , 4   , "Life Meter"					  },
	{0x16, 0x01, 0x18, 0x00, "5  (Small)"					  },
	{0x16, 0x01, 0x18, 0x10, "8  (Normal)"					  },
	{0x16, 0x01, 0x18, 0x18, "10 (Big)"						  },
	{0x16, 0x01, 0x18, 0x08, "12 (Biggest)"					  },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x16, 0x01, 0x80, 0x00, "1 Player Only"                  },
	{0x16, 0x01, 0x80, 0x80, "Multiplayer"                    },
};

STDDIPINFO(Deadconx)

static struct BurnDIPInfo DeadconxjDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xfe, NULL                             },
	{0x16, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x15, 0x01, 0x01, 0x00, "Upright"                        },
	{0x15, 0x01, 0x01, 0x01, "Cocktail"                       },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x15, 0x01, 0x02, 0x02, "Off"                            },
	{0x15, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x15, 0x01, 0x04, 0x04, "Off"                            },
	{0x15, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x15, 0x01, 0x08, 0x00, "Off"                            },
	{0x15, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x15, 0x01, 0x0c, 0x00, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0x0c, 0x08, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0x0c, 0x0c, "1 Coin  1 Credit"               },
	{0x15, 0x01, 0x0c, 0x04, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x15, 0x01, 0x03, 0x00, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0x03, 0x02, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0x03, 0x03, "1 Coin  1 Credit"               },
	{0x15, 0x01, 0x03, 0x01, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Number of Enemies 1/2 Player"	  },
	{0x16, 0x01, 0x03, 0x01, "25/45 (Easy)"					  },
	{0x16, 0x01, 0x03, 0x03, "30/50 (Medium)"				  },
	{0x16, 0x01, 0x03, 0x02, "40/60 (Hard)"					  },
	{0x16, 0x01, 0x03, 0x00, "50/70 (Hardest)"				  },

	{0   , 0xfe, 0   , 4   , "Life Meter"					  },
	{0x16, 0x01, 0x18, 0x00, "5  (Small)"					  },
	{0x16, 0x01, 0x18, 0x10, "8  (Normal)"					  },
	{0x16, 0x01, 0x18, 0x18, "10 (Big)"						  },
	{0x16, 0x01, 0x18, 0x08, "12 (Biggest)"					  },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x16, 0x01, 0x80, 0x00, "1 Player Only"                  },
	{0x16, 0x01, 0x80, 0x80, "Multiplayer"                    },
};

STDDIPINFO(Deadconxj)

static struct BurnDIPInfo DinorexDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xff, NULL                             },
	{0x16, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x15, 0x01, 0x02, 0x02, "Off"                            },
	{0x15, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x15, 0x01, 0x04, 0x04, "Off"                            },
	{0x15, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x15, 0x01, 0x08, 0x00, "Off"                            },
	{0x15, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x15, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x15, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x15, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x15, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x16, 0x01, 0x03, 0x02, "Easy"                           },
	{0x16, 0x01, 0x03, 0x03, "Medium"                         },
	{0x16, 0x01, 0x03, 0x01, "Hard"                           },
	{0x16, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Damage"                         },
	{0x16, 0x01, 0x0c, 0x08, "Small"                          },
	{0x16, 0x01, 0x0c, 0x0c, "Normal"                         },
	{0x16, 0x01, 0x0c, 0x04, "Big"                            },
	{0x16, 0x01, 0x0c, 0x00, "Biggest"                        },

	{0   , 0xfe, 0   , 2   , "Timer Speed"                    },
	{0x16, 0x01, 0x10, 0x10, "Normal"                         },
	{0x16, 0x01, 0x10, 0x00, "Fast"                           },

	{0   , 0xfe, 0   , 2   , "Match Type"                     },
	{0x16, 0x01, 0x20, 0x20, "Best of 3"                      },
	{0x16, 0x01, 0x20, 0x00, "Single"                         },

	{0   , 0xfe, 0   , 2   , "2 Player Mode"                  },
	{0x16, 0x01, 0x40, 0x40, "Upright"                        },
	{0x16, 0x01, 0x40, 0x00, "Cocktail"                       },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x16, 0x01, 0x80, 0x00, "Single"                         },
	{0x16, 0x01, 0x80, 0x80, "Dual"                           },
};

STDDIPINFO(Dinorex)

static struct BurnDIPInfo DinorexjDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xff, NULL                             },
	{0x16, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x15, 0x01, 0x02, 0x02, "Off"                            },
	{0x15, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x15, 0x01, 0x04, 0x04, "Off"                            },
	{0x15, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x15, 0x01, 0x08, 0x00, "Off"                            },
	{0x15, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x15, 0x01, 0x0c, 0x00, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0x0c, 0x08, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0x0c, 0x0c, "1 Coin  1 Credit"               },
	{0x15, 0x01, 0x0c, 0x04, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x15, 0x01, 0x03, 0x00, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0x03, 0x02, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0x03, 0x03, "1 Coin  1 Credit"               },
	{0x15, 0x01, 0x03, 0x01, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x16, 0x01, 0x03, 0x02, "Easy"                           },
	{0x16, 0x01, 0x03, 0x03, "Medium"                         },
	{0x16, 0x01, 0x03, 0x01, "Hard"                           },
	{0x16, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Damage"                         },
	{0x16, 0x01, 0x0c, 0x08, "Small"                          },
	{0x16, 0x01, 0x0c, 0x0c, "Normal"                         },
	{0x16, 0x01, 0x0c, 0x04, "Big"                            },
	{0x16, 0x01, 0x0c, 0x00, "Biggest"                        },

	{0   , 0xfe, 0   , 2   , "Timer Speed"                    },
	{0x16, 0x01, 0x10, 0x10, "Normal"                         },
	{0x16, 0x01, 0x10, 0x00, "Fast"                           },

	{0   , 0xfe, 0   , 2   , "Match Type"                     },
	{0x16, 0x01, 0x20, 0x20, "Best of 3"                      },
	{0x16, 0x01, 0x20, 0x00, "Single"                         },

	{0   , 0xfe, 0   , 2   , "2 Player Mode"                  },
	{0x16, 0x01, 0x40, 0x40, "Upright"                        },
	{0x16, 0x01, 0x40, 0x00, "Cocktail"                       },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x16, 0x01, 0x80, 0x00, "Single"                         },
	{0x16, 0x01, 0x80, 0x80, "Dual"                           },
};

STDDIPINFO(Dinorexj)

static struct BurnDIPInfo DondokodDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                             },
	{0x14, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x13, 0x01, 0x02, 0x02, "Off"                            },
	{0x13, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x13, 0x01, 0x04, 0x04, "Off"                            },
	{0x13, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x13, 0x01, 0x08, 0x00, "Off"                            },
	{0x13, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x13, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x13, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x13, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x14, 0x01, 0x03, 0x02, "Easy"                           },
	{0x14, 0x01, 0x03, 0x03, "Medium"                         },
	{0x14, 0x01, 0x03, 0x01, "Hard"                           },
	{0x14, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x14, 0x01, 0x0c, 0x0c, "10k and 100k"                   },
	{0x14, 0x01, 0x0c, 0x08, "10k and 150k"                   },
	{0x14, 0x01, 0x0c, 0x04, "10k and 250k"                   },
	{0x14, 0x01, 0x0c, 0x00, "10k and 350k"                   },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x14, 0x01, 0x30, 0x20, "2"                              },
	{0x14, 0x01, 0x30, 0x30, "3"                              },
	{0x14, 0x01, 0x30, 0x00, "4"                              },
	{0x14, 0x01, 0x30, 0x10, "5"                              },
};

STDDIPINFO(Dondokod)

static struct BurnDIPInfo DondokodjDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                             },
	{0x14, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x13, 0x01, 0x02, 0x02, "Off"                            },
	{0x13, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x13, 0x01, 0x04, 0x04, "Off"                            },
	{0x13, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x13, 0x01, 0x08, 0x00, "Off"                            },
	{0x13, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x13, 0x01, 0x30, 0x00, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x13, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x13, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x13, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x14, 0x01, 0x03, 0x02, "Easy"                           },
	{0x14, 0x01, 0x03, 0x03, "Medium"                         },
	{0x14, 0x01, 0x03, 0x01, "Hard"                           },
	{0x14, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x14, 0x01, 0x0c, 0x0c, "10k and 100k"                   },
	{0x14, 0x01, 0x0c, 0x08, "10k and 150k"                   },
	{0x14, 0x01, 0x0c, 0x04, "10k and 250k"                   },
	{0x14, 0x01, 0x0c, 0x00, "10k and 350k"                   },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x14, 0x01, 0x30, 0x20, "2"                              },
	{0x14, 0x01, 0x30, 0x30, "3"                              },
	{0x14, 0x01, 0x30, 0x00, "4"                              },
	{0x14, 0x01, 0x30, 0x10, "5"                              },
};

STDDIPINFO(Dondokodj)

static struct BurnDIPInfo DondokoduDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                             },
	{0x14, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x13, 0x01, 0x02, 0x02, "Off"                            },
	{0x13, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x13, 0x01, 0x04, 0x04, "Off"                            },
	{0x13, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x13, 0x01, 0x08, 0x00, "Off"                            },
	{0x13, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coinage"                        },
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Price to Continue"              },
	{0x13, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0xc0, 0x80, "1 Coin 1 Credit"                },
	{0x13, 0x01, 0xc0, 0xc0, "Same as start"                  },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x14, 0x01, 0x03, 0x02, "Easy"                           },
	{0x14, 0x01, 0x03, 0x03, "Medium"                         },
	{0x14, 0x01, 0x03, 0x01, "Hard"                           },
	{0x14, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x14, 0x01, 0x0c, 0x0c, "10k and 100k"                   },
	{0x14, 0x01, 0x0c, 0x08, "10k and 150k"                   },
	{0x14, 0x01, 0x0c, 0x04, "10k and 250k"                   },
	{0x14, 0x01, 0x0c, 0x00, "10k and 350k"                   },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x14, 0x01, 0x30, 0x20, "2"                              },
	{0x14, 0x01, 0x30, 0x30, "3"                              },
	{0x14, 0x01, 0x30, 0x00, "4"                              },
	{0x14, 0x01, 0x30, 0x10, "5"                              },
};

STDDIPINFO(Dondokodu)

static struct BurnDIPInfo DriftoutDIPList[]=
{
	// Default Values
	{0x0c, 0xff, 0xff, 0xff, NULL                             },
	{0x0d, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x0c, 0x01, 0x02, 0x02, "Off"                            },
	{0x0c, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0c, 0x01, 0x04, 0x04, "Off"                            },
	{0x0c, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0c, 0x01, 0x08, 0x00, "Off"                            },
	{0x0c, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0c, 0x01, 0x30, 0x00, "2 Coins 1 Credit"               },
	{0x0c, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x0c, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x0c, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0c, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0c, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x0c, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x0c, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x0d, 0x01, 0x03, 0x02, "Easy"                           },
	{0x0d, 0x01, 0x03, 0x03, "Medium"                         },
	{0x0d, 0x01, 0x03, 0x01, "Hard"                           },
	{0x0d, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Control"                        },
	{0x0d, 0x01, 0x0c, 0x0c, "Joystick"                       },
	{0x0d, 0x01, 0x0c, 0x08, "Paddle"                         },
	{0x0d, 0x01, 0x0c, 0x04, "Joystick"                       },
	{0x0d, 0x01, 0x0c, 0x00, "Steering Wheel"                 },
};

STDDIPINFO(Driftout)

static struct BurnDIPInfo FinalbDIPList[]=
{
	// Default Values
	{0x19, 0xff, 0xff, 0xff, NULL                             },
	{0x1a, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Alternate Controls"             },
	{0x19, 0x01, 0x01, 0x01, "Off"                            },
	{0x19, 0x01, 0x01, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x19, 0x01, 0x02, 0x02, "Off"                            },
	{0x19, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x19, 0x01, 0x04, 0x04, "Off"                            },
	{0x19, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x19, 0x01, 0x08, 0x00, "Off"                            },
	{0x19, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x19, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x19, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x19, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x19, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x19, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x19, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x19, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x19, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x1a, 0x01, 0x03, 0x02, "Easy"                           },
	{0x1a, 0x01, 0x03, 0x03, "Medium"                         },
	{0x1a, 0x01, 0x03, 0x01, "Hard"                           },
	{0x1a, 0x01, 0x03, 0x00, "Hardest"                        },
};

STDDIPINFO(Finalb)

static struct BurnDIPInfo FinalbjDIPList[]=
{
	// Default Values
	{0x19, 0xff, 0xff, 0xff, NULL                             },
	{0x1a, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Alternate Controls"             },
	{0x19, 0x01, 0x01, 0x01, "Off"                            },
	{0x19, 0x01, 0x01, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x19, 0x01, 0x02, 0x02, "Off"                            },
	{0x19, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x19, 0x01, 0x04, 0x04, "Off"                            },
	{0x19, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x19, 0x01, 0x08, 0x00, "Off"                            },
	{0x19, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x19, 0x01, 0x30, 0x00, "2 Coins 1 Credit"               },
	{0x19, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x19, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x19, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x19, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x19, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x19, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x19, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x1a, 0x01, 0x03, 0x02, "Easy"                           },
	{0x1a, 0x01, 0x03, 0x03, "Medium"                         },
	{0x1a, 0x01, 0x03, 0x01, "Hard"                           },
	{0x1a, 0x01, 0x03, 0x00, "Hardest"                        },
};

STDDIPINFO(Finalbj)

static struct BurnDIPInfo FootchmpDIPList[]=
{
	// Default Values
	{0x25, 0xff, 0xff, 0xfe, NULL                             },
	{0x26, 0xff, 0xff, 0x7f, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Game Over Type"                 },
	{0x25, 0x01, 0x01, 0x01, "Both Teams' Games Over"         },
	{0x25, 0x01, 0x01, 0x00, "Losing Team's Game is Over"     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x25, 0x01, 0x02, 0x02, "Off"                            },
	{0x25, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x25, 0x01, 0x04, 0x04, "Off"                            },
	{0x25, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x25, 0x01, 0x08, 0x00, "Off"                            },
	{0x25, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x25, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x25, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x25, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x25, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x25, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x25, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x25, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x25, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x26, 0x01, 0x03, 0x02, "Easy"                           },
	{0x26, 0x01, 0x03, 0x03, "Normal"                         },
	{0x26, 0x01, 0x03, 0x01, "Hard"                           },
	{0x26, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Game Time"                      },
	{0x26, 0x01, 0x0c, 0x00, "1 minute  30 seconds"           },
	{0x26, 0x01, 0x0c, 0x0c, "2 minutes"                      },
	{0x26, 0x01, 0x0c, 0x04, "2 minutes 30 seconds"           },
	{0x26, 0x01, 0x0c, 0x08, "3 minutes"                      },

	{0   , 0xfe, 0   , 4   , "Cabinet"                        },
	{0x26, 0x01, 0x30, 0x30, "2 Players"                      },
	{0x26, 0x01, 0x30, 0x20, "4 Players / 4 Coin Slots"       },
	{0x26, 0x01, 0x30, 0x10, "4 Players / 2 Cabinets Combined"},
	{0x26, 0x01, 0x30, 0x00, "4 Players / 2 Coin Slots"       },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x26, 0x01, 0x40, 0x00, "Off"                            },
	{0x26, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Game Version"                   },
	{0x26, 0x01, 0x80, 0x00, "Normal"                         },
	{0x26, 0x01, 0x80, 0x80, "European"                       },

};

STDDIPINFO(Footchmp)

static struct BurnDIPInfo HtherouDIPList[]=
{
	// Default Values
	{0x25, 0xff, 0xff, 0xfe, NULL                             },
	{0x26, 0xff, 0xff, 0x7f, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Game Over Type"                 },
	{0x25, 0x01, 0x01, 0x01, "Both Teams' Games Over"         },
	{0x25, 0x01, 0x01, 0x00, "Losing Team's Game is Over"     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x25, 0x01, 0x02, 0x02, "Off"                            },
	{0x25, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x25, 0x01, 0x04, 0x04, "Off"                            },
	{0x25, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x25, 0x01, 0x08, 0x00, "Off"                            },
	{0x25, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x25, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x25, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x25, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x25, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x25, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x25, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x25, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x25, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x26, 0x01, 0x03, 0x02, "Easy"                           },
	{0x26, 0x01, 0x03, 0x03, "Normal"                         },
	{0x26, 0x01, 0x03, 0x01, "Hard"                           },
	{0x26, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Game Time"                      },
	{0x26, 0x01, 0x0c, 0x00, "1 minute  30 seconds"           },
	{0x26, 0x01, 0x0c, 0x0c, "2 minutes"                      },
	{0x26, 0x01, 0x0c, 0x04, "2 minutes 30 seconds"           },
	{0x26, 0x01, 0x0c, 0x08, "3 minutes"                      },

	{0   , 0xfe, 0   , 4   , "Cabinet"                        },
	{0x26, 0x01, 0x30, 0x30, "2 Players"                      },
	{0x26, 0x01, 0x30, 0x20, "4 Players / 4 Coin Slots"       },
	{0x26, 0x01, 0x30, 0x10, "4 Players / 2 Cabinets Combined"},
	{0x26, 0x01, 0x30, 0x00, "4 Players / 2 Coin Slots"       },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x26, 0x01, 0x40, 0x00, "Off"                            },
	{0x26, 0x01, 0x40, 0x40, "On"                             },
};

STDDIPINFO(Htherou)

static struct BurnDIPInfo HtherojDIPList[]=
{
	// Default Values
	{0x25, 0xff, 0xff, 0x7f, NULL                             },
	{0x26, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Game Over Type"                 },
	{0x25, 0x01, 0x80, 0x80, "Both Teams' Games Over"         },
	{0x25, 0x01, 0x80, 0x00, "Losing Team's Game is Over"     },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x25, 0x01, 0x40, 0x40, "Off"                            },
	{0x25, 0x01, 0x40, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x25, 0x01, 0x20, 0x20, "Off"                            },
	{0x25, 0x01, 0x20, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x25, 0x01, 0x10, 0x00, "Off"                            },
	{0x25, 0x01, 0x10, 0x10, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x25, 0x01, 0x0c, 0x00, "3 Coins 1 Credit"               },
	{0x25, 0x01, 0x0c, 0x08, "2 Coins 1 Credit"               },
	{0x25, 0x01, 0x0c, 0x0c, "1 Coin  1 Credit"               },
	{0x25, 0x01, 0x0c, 0x04, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x25, 0x01, 0x03, 0x00, "3 Coins 1 Credit"               },
	{0x25, 0x01, 0x03, 0x02, "2 Coins 1 Credit"               },
	{0x25, 0x01, 0x03, 0x03, "1 Coin  1 Credit"               },
	{0x25, 0x01, 0x03, 0x01, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x26, 0x01, 0xc0, 0x40, "Easy"                           },
	{0x26, 0x01, 0xc0, 0xc0, "Normal"                         },
	{0x26, 0x01, 0xc0, 0x80, "Hard"                           },
	{0x26, 0x01, 0xc0, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Game Time"                      },
	{0x26, 0x01, 0x30, 0x00, "1 minute  30 seconds"           },
	{0x26, 0x01, 0x30, 0x30, "2 minutes"                      },
	{0x26, 0x01, 0x30, 0x20, "2 minutes 30 seconds"           },
	{0x26, 0x01, 0x30, 0x10, "3 minutes"                      },

	{0   , 0xfe, 0   , 4   , "Cabinet"                        },
	{0x26, 0x01, 0x0c, 0x0c, "2 Players"                      },
	{0x26, 0x01, 0x0c, 0x04, "4 Players / 4 Coin Slots"       },
	{0x26, 0x01, 0x0c, 0x08, "4 Players / 2 Cabinets Combined"},
	{0x26, 0x01, 0x0c, 0x00, "4 Players / 2 Coin Slots"       },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x26, 0x01, 0x02, 0x00, "Off"                            },
	{0x26, 0x01, 0x02, 0x02, "On"                             },
};

STDDIPINFO(Htheroj)

static struct BurnDIPInfo GrowlDIPList[]=
{
	// Default Values
	{0x28, 0xff, 0xff, 0xff, NULL                             },
	{0x29, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x28, 0x01, 0x02, 0x02, "Off"                            },
	{0x28, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x28, 0x01, 0x04, 0x04, "Off"                            },
	{0x28, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x28, 0x01, 0x08, 0x00, "Off"                            },
	{0x28, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x28, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x28, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x28, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x28, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x28, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x28, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x28, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x28, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x29, 0x01, 0x03, 0x02, "Easy"                           },
	{0x29, 0x01, 0x03, 0x03, "Normal"                         },
	{0x29, 0x01, 0x03, 0x01, "Hard"                           },
	{0x29, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Cabinet Type"                   },
	{0x29, 0x01, 0x30, 0x30, "2 Players"                      },
	{0x29, 0x01, 0x30, 0x20, "4 Players / 4 Coin Slots"       },
	{0x29, 0x01, 0x30, 0x10, "4 Players / 2 Cabinets Combined"},
	{0x29, 0x01, 0x30, 0x00, "4 Players / 2 Coin Slots"       },

	{0   , 0xfe, 0   , 2   , "Final Boss Continue"            },
	{0x29, 0x01, 0x40, 0x00, "Off"                            },
	{0x29, 0x01, 0x40, 0x40, "On"                             },
};

STDDIPINFO(Growl)

static struct BurnDIPInfo GrowluDIPList[]=
{
	// Default Values
	{0x28, 0xff, 0xff, 0xff, NULL                             },
	{0x29, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x28, 0x01, 0x02, 0x02, "Off"                            },
	{0x28, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x28, 0x01, 0x04, 0x04, "Off"                            },
	{0x28, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x28, 0x01, 0x08, 0x00, "Off"                            },
	{0x28, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coinage"                        },
	{0x28, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x28, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x28, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x28, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Price to Continue"              },
	{0x28, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x28, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x28, 0x01, 0xc0, 0x80, "1 Coin 1 Credit"                },
	{0x28, 0x01, 0xc0, 0xc0, "Same as start"                  },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x29, 0x01, 0x03, 0x02, "Easy"                           },
	{0x29, 0x01, 0x03, 0x03, "Normal"                         },
	{0x29, 0x01, 0x03, 0x01, "Hard"                           },
	{0x29, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Cabinet Type"                   },
	{0x29, 0x01, 0x30, 0x30, "2 Players"                      },
	{0x29, 0x01, 0x30, 0x20, "4 Players / 4 Coin Slots"       },
	{0x29, 0x01, 0x30, 0x10, "4 Players / 2 Cabinets Combined"},
	{0x29, 0x01, 0x30, 0x00, "4 Players / 2 Coin Slots"       },

	{0   , 0xfe, 0   , 2   , "Final Boss Continue"            },
	{0x29, 0x01, 0x40, 0x00, "Off"                            },
	{0x29, 0x01, 0x40, 0x40, "On"                             },
};

STDDIPINFO(Growlu)

static struct BurnDIPInfo RunarkDIPList[]=
{
	// Default Values
	{0x28, 0xff, 0xff, 0xff, NULL                             },
	{0x29, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x28, 0x01, 0x02, 0x02, "Off"                            },
	{0x28, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x28, 0x01, 0x04, 0x04, "Off"                            },
	{0x28, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x28, 0x01, 0x08, 0x00, "Off"                            },
	{0x28, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x28, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x28, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x28, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x28, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x28, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x28, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x28, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x28, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x29, 0x01, 0x03, 0x02, "Easy"                           },
	{0x29, 0x01, 0x03, 0x03, "Normal"                         },
	{0x29, 0x01, 0x03, 0x01, "Hard"                           },
	{0x29, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Cabinet Type"                   },
	{0x29, 0x01, 0x30, 0x30, "2 Players"                      },
	{0x29, 0x01, 0x30, 0x20, "4 Players / 4 Coin Slots"       },
	{0x29, 0x01, 0x30, 0x10, "4 Players / 2 Cabinets Combined"},
	{0x29, 0x01, 0x30, 0x00, "4 Players / 2 Coin Slots"       },

	{0   , 0xfe, 0   , 2   , "Final Boss Continue"            },
	{0x29, 0x01, 0x40, 0x00, "Off"                            },
	{0x29, 0x01, 0x40, 0x40, "On"                             },
};

STDDIPINFO(Runark)

static struct BurnDIPInfo GunfrontDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                             },
	{0x14, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x13, 0x01, 0x02, 0x02, "Off"                            },
	{0x13, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x13, 0x01, 0x04, 0x04, "Off"                            },
	{0x13, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x13, 0x01, 0x08, 0x00, "Off"                            },
	{0x13, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x13, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x13, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x13, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x14, 0x01, 0x03, 0x02, "Easy"                           },
	{0x14, 0x01, 0x03, 0x03, "Medium"                         },
	{0x14, 0x01, 0x03, 0x01, "Hard"                           },
	{0x14, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x14, 0x01, 0x0c, 0x08, "10k and every 80k"              },
	{0x14, 0x01, 0x0c, 0x0c, "20k and every 80k"              },
	{0x14, 0x01, 0x0c, 0x04, "30k and every 80k"              },
	{0x14, 0x01, 0x0c, 0x00, "60k and every 80k"              },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x14, 0x01, 0x30, 0x20, "1"                              },
	{0x14, 0x01, 0x30, 0x10, "2"                              },
	{0x14, 0x01, 0x30, 0x30, "3"                              },
	{0x14, 0x01, 0x30, 0x00, "5"                              },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x14, 0x01, 0x40, 0x00, "Off"                            },
	{0x14, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x14, 0x01, 0x80, 0x00, "Single"                         },
	{0x14, 0x01, 0x80, 0x80, "Dual"                           },
};

STDDIPINFO(Gunfront)

static struct BurnDIPInfo GunfrontjDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xff, NULL                             },
	{0x14, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x13, 0x01, 0x02, 0x02, "Off"                            },
	{0x13, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x13, 0x01, 0x04, 0x04, "Off"                            },
	{0x13, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x13, 0x01, 0x08, 0x00, "Off"                            },
	{0x13, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x13, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x13, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x13, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x14, 0x01, 0x03, 0x02, "Easy"                           },
	{0x14, 0x01, 0x03, 0x03, "Medium"                         },
	{0x14, 0x01, 0x03, 0x01, "Hard"                           },
	{0x14, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x14, 0x01, 0x0c, 0x08, "10k and every 80k"              },
	{0x14, 0x01, 0x0c, 0x0c, "20k and every 80k"              },
	{0x14, 0x01, 0x0c, 0x04, "30k and every 80k"              },
	{0x14, 0x01, 0x0c, 0x00, "60k and every 80k"              },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x14, 0x01, 0x30, 0x20, "1"                              },
	{0x14, 0x01, 0x30, 0x10, "2"                              },
	{0x14, 0x01, 0x30, 0x30, "3"                              },
	{0x14, 0x01, 0x30, 0x00, "5"                              },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x14, 0x01, 0x40, 0x00, "Off"                            },
	{0x14, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x14, 0x01, 0x80, 0x00, "Single"                         },
	{0x14, 0x01, 0x80, 0x80, "Dual"                           },
};

STDDIPINFO(Gunfrontj)

static struct BurnDIPInfo KoshienDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xff, NULL                             },
	{0x16, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x15, 0x01, 0x02, 0x02, "Off"                            },
	{0x15, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x15, 0x01, 0x04, 0x04, "Off"                            },
	{0x15, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x15, 0x01, 0x08, 0x00, "Off"                            },
	{0x15, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x15, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x15, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x15, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x15, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x16, 0x01, 0x03, 0x02, "Easy"                           },
	{0x16, 0x01, 0x03, 0x03, "Medium"                         },
	{0x16, 0x01, 0x03, 0x01, "Hard"                           },
	{0x16, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 2   , "Timer"                          },
	{0x16, 0x01, 0x04, 0x00, "Off"                            },
	{0x16, 0x01, 0x04, 0x04, "On"                             },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x16, 0x01, 0x30, 0x20, "2"                              },
	{0x16, 0x01, 0x30, 0x30, "3"                              },
	{0x16, 0x01, 0x30, 0x10, "4"                              },
	{0x16, 0x01, 0x30, 0x00, "5"                              },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x16, 0x01, 0x40, 0x00, "Off"                            },
	{0x16, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x16, 0x01, 0x80, 0x00, "Single"                         },
	{0x16, 0x01, 0x80, 0x80, "Dual"                           },
};

STDDIPINFO(Koshien)

static struct BurnDIPInfo LiquidkDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfe, NULL                             },
	{0x14, 0xff, 0xff, 0x7f, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x13, 0x01, 0x01, 0x00, "Upright"                        },
	{0x13, 0x01, 0x01, 0x01, "Cocktail"                       },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x13, 0x01, 0x02, 0x02, "Off"                            },
	{0x13, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x13, 0x01, 0x04, 0x04, "Off"                            },
	{0x13, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x13, 0x01, 0x08, 0x00, "Off"                            },
	{0x13, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x13, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x13, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x13, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x14, 0x01, 0x03, 0x02, "Easy"                           },
	{0x14, 0x01, 0x03, 0x03, "Medium"                         },
	{0x14, 0x01, 0x03, 0x01, "Hard"                           },
	{0x14, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x14, 0x01, 0x0c, 0x0c, "30k and 100k"                   },
	{0x14, 0x01, 0x0c, 0x08, "30k and 150k"                   },
	{0x14, 0x01, 0x0c, 0x04, "50k and 250k"                   },
	{0x14, 0x01, 0x0c, 0x00, "50k and 350k"                   },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x14, 0x01, 0x30, 0x20, "2"                              },
	{0x14, 0x01, 0x30, 0x30, "3"                              },
	{0x14, 0x01, 0x30, 0x00, "4"                              },
	{0x14, 0x01, 0x30, 0x10, "5"                              },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x14, 0x01, 0x40, 0x00, "Off"                            },
	{0x14, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x14, 0x01, 0x80, 0x80, "Single"                         },
	{0x14, 0x01, 0x80, 0x00, "Dual"                           },
};

STDDIPINFO(Liquidk)

static struct BurnDIPInfo LiquidkuDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfe, NULL                             },
	{0x14, 0xff, 0xff, 0x7f, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x13, 0x01, 0x01, 0x00, "Upright"                        },
	{0x13, 0x01, 0x01, 0x01, "Cocktail"                       },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x13, 0x01, 0x02, 0x02, "Off"                            },
	{0x13, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x13, 0x01, 0x04, 0x04, "Off"                            },
	{0x13, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x13, 0x01, 0x08, 0x00, "Off"                            },
	{0x13, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coinage"                        },
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Price to Continue"              },
	{0x13, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0xc0, 0x80, "1 Coin 1 Credit"                },
	{0x13, 0x01, 0xc0, 0xc0, "Same as start"                  },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x14, 0x01, 0x03, 0x02, "Easy"                           },
	{0x14, 0x01, 0x03, 0x03, "Medium"                         },
	{0x14, 0x01, 0x03, 0x01, "Hard"                           },
	{0x14, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x14, 0x01, 0x0c, 0x0c, "30k and 100k"                   },
	{0x14, 0x01, 0x0c, 0x08, "30k and 150k"                   },
	{0x14, 0x01, 0x0c, 0x04, "50k and 250k"                   },
	{0x14, 0x01, 0x0c, 0x00, "50k and 350k"                   },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x14, 0x01, 0x30, 0x20, "2"                              },
	{0x14, 0x01, 0x30, 0x30, "3"                              },
	{0x14, 0x01, 0x30, 0x00, "4"                              },
	{0x14, 0x01, 0x30, 0x10, "5"                              },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x14, 0x01, 0x40, 0x00, "Off"                            },
	{0x14, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x14, 0x01, 0x80, 0x80, "Single"                         },
	{0x14, 0x01, 0x80, 0x00, "Dual"                           },
};

STDDIPINFO(Liquidku)

static struct BurnDIPInfo MegablstDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfe, NULL                             },
	{0x14, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x13, 0x01, 0x01, 0x00, "Upright"                        },
	{0x13, 0x01, 0x01, 0x01, "Cocktail"                       },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x13, 0x01, 0x02, 0x02, "Off"                            },
	{0x13, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x13, 0x01, 0x04, 0x04, "Off"                            },
	{0x13, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x13, 0x01, 0x08, 0x00, "Off"                            },
	{0x13, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x13, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x13, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x13, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x14, 0x01, 0x03, 0x02, "Easy"                           },
	{0x14, 0x01, 0x03, 0x03, "Medium"                         },
	{0x14, 0x01, 0x03, 0x01, "Hard"                           },
	{0x14, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x14, 0x01, 0x0c, 0x0c, "100k only"                      },
	{0x14, 0x01, 0x0c, 0x04, "150k only"                      },
	{0x14, 0x01, 0x0c, 0x08, "200k only"                      },
	{0x14, 0x01, 0x0c, 0x00, "None"                           },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x14, 0x01, 0x30, 0x10, "1"                              },
	{0x14, 0x01, 0x30, 0x00, "2"                              },
	{0x14, 0x01, 0x30, 0x30, "3"                              },
	{0x14, 0x01, 0x30, 0x20, "4"                              },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x14, 0x01, 0x40, 0x00, "Single"                         },
	{0x14, 0x01, 0x40, 0x40, "Dual"                           },
};

STDDIPINFO(Megablst)

static struct BurnDIPInfo MegablstjDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfe, NULL                             },
	{0x14, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x13, 0x01, 0x01, 0x00, "Upright"                        },
	{0x13, 0x01, 0x01, 0x01, "Cocktail"                       },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x13, 0x01, 0x02, 0x02, "Off"                            },
	{0x13, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x13, 0x01, 0x04, 0x04, "Off"                            },
	{0x13, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x13, 0x01, 0x08, 0x00, "Off"                            },
	{0x13, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x13, 0x01, 0x30, 0x00, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x13, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x13, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x13, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x14, 0x01, 0x03, 0x02, "Easy"                           },
	{0x14, 0x01, 0x03, 0x03, "Medium"                         },
	{0x14, 0x01, 0x03, 0x01, "Hard"                           },
	{0x14, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x14, 0x01, 0x0c, 0x0c, "100k only"                      },
	{0x14, 0x01, 0x0c, 0x04, "150k only"                      },
	{0x14, 0x01, 0x0c, 0x08, "200k only"                      },
	{0x14, 0x01, 0x0c, 0x00, "None"                           },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x14, 0x01, 0x30, 0x10, "1"                              },
	{0x14, 0x01, 0x30, 0x00, "2"                              },
	{0x14, 0x01, 0x30, 0x30, "3"                              },
	{0x14, 0x01, 0x30, 0x20, "4"                              },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x14, 0x01, 0x40, 0x00, "Single"                         },
	{0x14, 0x01, 0x40, 0x40, "Dual"                           },
};

STDDIPINFO(Megablstj)

static struct BurnDIPInfo MegablstuDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfe, NULL                             },
	{0x14, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x13, 0x01, 0x01, 0x00, "Upright"                        },
	{0x13, 0x01, 0x01, 0x01, "Cocktail"                       },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x13, 0x01, 0x02, 0x02, "Off"                            },
	{0x13, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x13, 0x01, 0x04, 0x04, "Off"                            },
	{0x13, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x13, 0x01, 0x08, 0x00, "Off"                            },
	{0x13, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coinage"                        },
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Price to Continue"              },
	{0x13, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0xc0, 0x80, "1 Coin 1 Credit"                },
	{0x13, 0x01, 0xc0, 0xc0, "Same as start"                  },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x14, 0x01, 0x03, 0x02, "Easy"                           },
	{0x14, 0x01, 0x03, 0x03, "Medium"                         },
	{0x14, 0x01, 0x03, 0x01, "Hard"                           },
	{0x14, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x14, 0x01, 0x0c, 0x0c, "100k only"                      },
	{0x14, 0x01, 0x0c, 0x04, "150k only"                      },
	{0x14, 0x01, 0x0c, 0x08, "200k only"                      },
	{0x14, 0x01, 0x0c, 0x00, "None"                           },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x14, 0x01, 0x30, 0x10, "1"                              },
	{0x14, 0x01, 0x30, 0x00, "2"                              },
	{0x14, 0x01, 0x30, 0x30, "3"                              },
	{0x14, 0x01, 0x30, 0x20, "4"                              },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x14, 0x01, 0x40, 0x00, "Single"                         },
	{0x14, 0x01, 0x40, 0x40, "Dual"                           },
};

STDDIPINFO(Megablstu)

static struct BurnDIPInfo MetalbDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xff, NULL                             },
	{0x16, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x15, 0x01, 0x02, 0x02, "Off"                            },
	{0x15, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x15, 0x01, 0x04, 0x04, "Off"                            },
	{0x15, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x15, 0x01, 0x08, 0x00, "Off"                            },
	{0x15, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x15, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x15, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x15, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x15, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x16, 0x01, 0x03, 0x02, "Easy"                           },
	{0x16, 0x01, 0x03, 0x03, "Medium"                         },
	{0x16, 0x01, 0x03, 0x01, "Hard"                           },
	{0x16, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x16, 0x01, 0x0c, 0x08, "50k and every 120k"             },
	{0x16, 0x01, 0x0c, 0x0c, "70k and every 150k"             },
	{0x16, 0x01, 0x0c, 0x04, "80k and every 160k"             },
	{0x16, 0x01, 0x0c, 0x00, "100k and every 200k"            },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x16, 0x01, 0x30, 0x00, "1"                              },
	{0x16, 0x01, 0x30, 0x10, "2"                              },
	{0x16, 0x01, 0x30, 0x30, "3"                              },
	{0x16, 0x01, 0x30, 0x20, "5"                              },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x16, 0x01, 0x40, 0x00, "Off"                            },
	{0x16, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x16, 0x01, 0x80, 0x00, "Single"                         },
	{0x16, 0x01, 0x80, 0x80, "Dual"                           },
};

STDDIPINFO(Metalb)

static struct BurnDIPInfo MetalbjDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xff, NULL                             },
	{0x16, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x15, 0x01, 0x02, 0x02, "Off"                            },
	{0x15, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x15, 0x01, 0x04, 0x04, "Off"                            },
	{0x15, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x15, 0x01, 0x08, 0x00, "Off"                            },
	{0x15, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x15, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x15, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x15, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x15, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x16, 0x01, 0x03, 0x02, "Easy"                           },
	{0x16, 0x01, 0x03, 0x03, "Medium"                         },
	{0x16, 0x01, 0x03, 0x01, "Hard"                           },
	{0x16, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Bonus Life"                     },
	{0x16, 0x01, 0x0c, 0x08, "50k and every 120k"             },
	{0x16, 0x01, 0x0c, 0x0c, "70k and every 150k"             },
	{0x16, 0x01, 0x0c, 0x04, "80k and every 160k"             },
	{0x16, 0x01, 0x0c, 0x00, "100k and every 200k"            },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x16, 0x01, 0x30, 0x00, "1"                              },
	{0x16, 0x01, 0x30, 0x10, "2"                              },
	{0x16, 0x01, 0x30, 0x30, "3"                              },
	{0x16, 0x01, 0x30, 0x20, "5"                              },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x16, 0x01, 0x40, 0x00, "Off"                            },
	{0x16, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x16, 0x01, 0x80, 0x00, "Single"                         },
	{0x16, 0x01, 0x80, 0x80, "Dual"                           },
};

STDDIPINFO(Metalbj)

static struct BurnDIPInfo MjnquestDIPList[]=
{
	// Default Values
	{0x19, 0xff, 0xff, 0xff, NULL                             },
	{0x1a, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x19, 0x01, 0x02, 0x02, "Off"                            },
	{0x19, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x19, 0x01, 0x04, 0x04, "Off"                            },
	{0x19, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x19, 0x01, 0x08, 0x00, "Off"                            },
	{0x19, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x19, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x19, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x19, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x19, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x19, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x19, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x19, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x19, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x1a, 0x01, 0x03, 0x02, "Easy"                           },
	{0x1a, 0x01, 0x03, 0x03, "Medium"                         },
	{0x1a, 0x01, 0x03, 0x01, "Hard"                           },
	{0x1a, 0x01, 0x03, 0x00, "Hardest"                        },
};

STDDIPINFO(Mjnquest)

static struct BurnDIPInfo NinjakDIPList[]=
{
	// Default Values
	{0x24, 0xff, 0xff, 0xff, NULL                             },
	{0x25, 0xff, 0xff, 0xdf, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x24, 0x01, 0x02, 0x02, "Off"                            },
	{0x24, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x24, 0x01, 0x04, 0x04, "Off"                            },
	{0x24, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x24, 0x01, 0x08, 0x00, "Off"                            },
	{0x24, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x24, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x24, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x24, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x24, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x24, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x24, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x24, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x24, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x25, 0x01, 0x03, 0x02, "Easy"                           },
	{0x25, 0x01, 0x03, 0x03, "Medium"                         },
	{0x25, 0x01, 0x03, 0x01, "Hard"                           },
	{0x25, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Cabinet Type"                   },
	{0x25, 0x01, 0x0c, 0x0c, "2 Players"                      },
	{0x25, 0x01, 0x0c, 0x08, "TROG (4 Players / 2 Coin Slots" },
	{0x25, 0x01, 0x0c, 0x04, "MTX2 (4 Players / 2 Cabinets"   },
	{0x25, 0x01, 0x0c, 0x00, "YMNT (4 Players / 4 Coin Slots" },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x25, 0x01, 0x30, 0x20, "2"                              },
	{0x25, 0x01, 0x30, 0x30, "3"                              },
	{0x25, 0x01, 0x30, 0x10, "4"                              },
	{0x25, 0x01, 0x30, 0x00, "5"                              },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x25, 0x01, 0x40, 0x00, "Off"                            },
	{0x25, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Game Type"                      },
	{0x25, 0x01, 0x80, 0x00, "1 Player Only"                  },
	{0x25, 0x01, 0x80, 0x80, "Multiplayer"                    },
};

STDDIPINFO(Ninjak)

static struct BurnDIPInfo NinjakjDIPList[]=
{
	// Default Values
	{0x24, 0xff, 0xff, 0xff, NULL                             },
	{0x25, 0xff, 0xff, 0xdf, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x24, 0x01, 0x02, 0x02, "Off"                            },
	{0x24, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x24, 0x01, 0x04, 0x04, "Off"                            },
	{0x24, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x24, 0x01, 0x08, 0x00, "Off"                            },
	{0x24, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x24, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x24, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x24, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x24, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x24, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x24, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x24, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x24, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x25, 0x01, 0x03, 0x02, "Easy"                           },
	{0x25, 0x01, 0x03, 0x03, "Medium"                         },
	{0x25, 0x01, 0x03, 0x01, "Hard"                           },
	{0x25, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Cabinet Type"                   },
	{0x25, 0x01, 0x0c, 0x0c, "2 Players"                      },
	{0x25, 0x01, 0x0c, 0x08, "TROG (4 Players / 2 Coin Slots" },
	{0x25, 0x01, 0x0c, 0x04, "MTX2 (4 Players / 2 Cabinets"   },
	{0x25, 0x01, 0x0c, 0x00, "YMNT (4 Players / 4 Coin Slots" },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x25, 0x01, 0x30, 0x20, "2"                              },
	{0x25, 0x01, 0x30, 0x30, "3"                              },
	{0x25, 0x01, 0x30, 0x10, "4"                              },
	{0x25, 0x01, 0x30, 0x00, "5"                              },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x25, 0x01, 0x40, 0x00, "Off"                            },
	{0x25, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Game Type"                      },
	{0x25, 0x01, 0x80, 0x00, "1 Player Only"                  },
	{0x25, 0x01, 0x80, 0x80, "Multiplayer"                    },
};

STDDIPINFO(Ninjakj)

static struct BurnDIPInfo NinjakuDIPList[]=
{
	// Default Values
	{0x24, 0xff, 0xff, 0xff, NULL                             },
	{0x25, 0xff, 0xff, 0xdf, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x24, 0x01, 0x02, 0x02, "Off"                            },
	{0x24, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x24, 0x01, 0x04, 0x04, "Off"                            },
	{0x24, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x24, 0x01, 0x08, 0x00, "Off"                            },
	{0x24, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coinage"                        },
	{0x24, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x24, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x24, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x24, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Price to Continue"              },
	{0x24, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x24, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x24, 0x01, 0xc0, 0x80, "1 Coin 1 Credit"                },
	{0x24, 0x01, 0xc0, 0xc0, "Same as start"                  },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x25, 0x01, 0x03, 0x02, "Easy"                           },
	{0x25, 0x01, 0x03, 0x03, "Medium"                         },
	{0x25, 0x01, 0x03, 0x01, "Hard"                           },
	{0x25, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Cabinet Type"                   },
	{0x25, 0x01, 0x0c, 0x0c, "2 Players"                      },
	{0x25, 0x01, 0x0c, 0x08, "TROG (4 Players / 2 Coin Slots" },
	{0x25, 0x01, 0x0c, 0x04, "MTX2 (4 Players / 2 Cabinets"   },
	{0x25, 0x01, 0x0c, 0x00, "YMNT (4 Players / 4 Coin Slots" },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x25, 0x01, 0x30, 0x20, "2"                              },
	{0x25, 0x01, 0x30, 0x30, "3"                              },
	{0x25, 0x01, 0x30, 0x10, "4"                              },
	{0x25, 0x01, 0x30, 0x00, "5"                              },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x25, 0x01, 0x40, 0x00, "Off"                            },
	{0x25, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Game Type"                      },
	{0x25, 0x01, 0x80, 0x00, "1 Player Only"                  },
	{0x25, 0x01, 0x80, 0x80, "Multiplayer"                    },
};

STDDIPINFO(Ninjaku)

static struct BurnDIPInfo PulirulaDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xff, NULL                             },
	{0x16, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x15, 0x01, 0x02, 0x02, "Off"                            },
	{0x15, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x15, 0x01, 0x04, 0x04, "Off"                            },
	{0x15, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x15, 0x01, 0x08, 0x00, "Off"                            },
	{0x15, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x15, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x15, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x15, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x15, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x16, 0x01, 0x03, 0x02, "Easy"                           },
	{0x16, 0x01, 0x03, 0x03, "Medium"                         },
	{0x16, 0x01, 0x03, 0x01, "Hard"                           },
	{0x16, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 3   , "Magic"                          },
	{0x16, 0x01, 0x0c, 0x0c, "3"                              },
	{0x16, 0x01, 0x0c, 0x08, "4"                              },
	{0x16, 0x01, 0x0c, 0x04, "5"                              },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x16, 0x01, 0x30, 0x20, "2"                              },
	{0x16, 0x01, 0x30, 0x30, "3"                              },
	{0x16, 0x01, 0x30, 0x10, "4"                              },
	{0x16, 0x01, 0x30, 0x00, "5"                              },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x16, 0x01, 0x80, 0x00, "Single"                         },
	{0x16, 0x01, 0x80, 0x80, "Dual"                           },
};

STDDIPINFO(Pulirula)

static struct BurnDIPInfo PulirulajDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xff, NULL                             },
	{0x16, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x15, 0x01, 0x02, 0x02, "Off"                            },
	{0x15, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x15, 0x01, 0x04, 0x04, "Off"                            },
	{0x15, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x15, 0x01, 0x08, 0x00, "Off"                            },
	{0x15, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x15, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x15, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x15, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x15, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x16, 0x01, 0x03, 0x02, "Easy"                           },
	{0x16, 0x01, 0x03, 0x03, "Medium"                         },
	{0x16, 0x01, 0x03, 0x01, "Hard"                           },
	{0x16, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 3   , "Magic"                          },
	{0x16, 0x01, 0x0c, 0x0c, "3"                              },
	{0x16, 0x01, 0x0c, 0x08, "4"                              },
	{0x16, 0x01, 0x0c, 0x04, "5"                              },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x16, 0x01, 0x30, 0x20, "2"                              },
	{0x16, 0x01, 0x30, 0x30, "3"                              },
	{0x16, 0x01, 0x30, 0x10, "4"                              },
	{0x16, 0x01, 0x30, 0x00, "5"                              },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x16, 0x01, 0x80, 0x00, "Single"                         },
	{0x16, 0x01, 0x80, 0x80, "Dual"                           },
};

STDDIPINFO(Pulirulaj)

static struct BurnDIPInfo QcrayonDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                             },
	{0x12, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x11, 0x01, 0x02, 0x02, "Off"                            },
	{0x11, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x11, 0x01, 0x04, 0x04, "Off"                            },
	{0x11, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x11, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x11, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x11, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x11, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x11, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x11, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x11, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x11, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x12, 0x01, 0x03, 0x02, "Easy"                           },
	{0x12, 0x01, 0x03, 0x03, "Medium"                         },
	{0x12, 0x01, 0x03, 0x01, "Hard"                           },
	{0x12, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Default Time"                   },
	{0x12, 0x01, 0x0c, 0x00, "6 seconds"                      },
	{0x12, 0x01, 0x0c, 0x04, "7 seconds"                      },
	{0x12, 0x01, 0x0c, 0x08, "8 seconds"                      },
	{0x12, 0x01, 0x0c, 0x0c, "10 seconds"                     },
};

STDDIPINFO(Qcrayon)

static struct BurnDIPInfo Qcrayon2DIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                             },
	{0x12, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x11, 0x01, 0x02, 0x02, "Off"                            },
	{0x11, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x11, 0x01, 0x04, 0x04, "Off"                            },
	{0x11, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x11, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x11, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x11, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x11, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x11, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x11, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x11, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x11, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x12, 0x01, 0x03, 0x02, "Easy"                           },
	{0x12, 0x01, 0x03, 0x03, "Medium"                         },
	{0x12, 0x01, 0x03, 0x01, "Hard"                           },
	{0x12, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 2   , "Game Control"                   },
	{0x12, 0x01, 0x80, 0x80, "Joystick"                       },
	{0x12, 0x01, 0x80, 0x00, "4 Buttons"                      },
};

STDDIPINFO(Qcrayon2)

static struct BurnDIPInfo QjinseiDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                             },
	{0x12, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x11, 0x01, 0x02, 0x02, "Off"                            },
	{0x11, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x11, 0x01, 0x04, 0x04, "Off"                            },
	{0x11, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x11, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x11, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x11, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x11, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x11, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x11, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x11, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x11, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x12, 0x01, 0x03, 0x02, "Easy"                           },
	{0x12, 0x01, 0x03, 0x03, "Medium"                         },
	{0x12, 0x01, 0x03, 0x01, "Hard"                           },
	{0x12, 0x01, 0x03, 0x00, "Hardest"                        },
};

STDDIPINFO(Qjinsei)

static struct BurnDIPInfo QtorimonDIPList[]=
{
	// Default Values
	{0x0f, 0xff, 0xff, 0xff, NULL                             },
	{0x10, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x0f, 0x01, 0x02, 0x02, "Off"                            },
	{0x0f, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x0f, 0x01, 0x04, 0x04, "Off"                            },
	{0x0f, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x0f, 0x01, 0x30, 0x00, "2 Coins 1 Credit"               },
	{0x0f, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x0f, 0x01, 0x30, 0x00, "2 Coins 3 Credits"              },
	{0x0f, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x0f, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x0f, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x0f, 0x01, 0xc0, 0x00, "2 Coins 3 Credits"              },
	{0x0f, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x10, 0x01, 0x03, 0x02, "Easy"                           },
	{0x10, 0x01, 0x03, 0x03, "Medium"                         },
	{0x10, 0x01, 0x03, 0x01, "Hard"                           },
	{0x10, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x10, 0x01, 0x30, 0x10, "1"                              },
	{0x10, 0x01, 0x30, 0x20, "2"                              },
	{0x10, 0x01, 0x30, 0x30, "3"                              },
	{0x10, 0x01, 0x30, 0x00, "4"                              },

	{0   , 0xfe, 0   , 2   , "Show Correct Answer"            },
	{0x10, 0x01, 0x40, 0x40, "Off"                            },
	{0x10, 0x01, 0x40, 0x00, "On"                             },
};

STDDIPINFO(Qtorimon)

static struct BurnDIPInfo QuizhqDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                             },
	{0x12, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x11, 0x01, 0x02, 0x02, "Off"                            },
	{0x11, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x11, 0x01, 0x04, 0x04, "Off"                            },
	{0x11, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x11, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x11, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x11, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x11, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x11, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x11, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x11, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x11, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x12, 0x01, 0x03, 0x02, "Easy"                           },
	{0x12, 0x01, 0x03, 0x03, "Medium"                         },
	{0x12, 0x01, 0x03, 0x01, "Hard"                           },
	{0x12, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Time"                           },
	{0x12, 0x01, 0x0c, 0x0c, "5 seconds"                      },
	{0x12, 0x01, 0x0c, 0x08, "10 seconds"                     },
	{0x12, 0x01, 0x0c, 0x04, "15 seconds"                     },
	{0x12, 0x01, 0x0c, 0x00, "20 seconds"                     },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x12, 0x01, 0x30, 0x20, "1"                              },
	{0x12, 0x01, 0x30, 0x30, "2"                              },
	{0x12, 0x01, 0x30, 0x10, "3"                              },
	{0x12, 0x01, 0x30, 0x00, "4"                              },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x12, 0x01, 0x40, 0x00, "Off"                            },
	{0x12, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x12, 0x01, 0x80, 0x00, "Single"                         },
	{0x12, 0x01, 0x80, 0x80, "Dual"                           },
};

STDDIPINFO(Quizhq)

static struct BurnDIPInfo QzchikyuDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xfe, NULL                             },
	{0x12, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x11, 0x01, 0x01, 0x00, "Upright"                        },
	{0x11, 0x01, 0x01, 0x01, "Cocktail"                       },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x11, 0x01, 0x02, 0x02, "Off"                            },
	{0x11, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x11, 0x01, 0x04, 0x04, "Off"                            },
	{0x11, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x11, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x11, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x11, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x11, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x11, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x11, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x11, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x11, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x12, 0x01, 0x03, 0x02, "Easy"                           },
	{0x12, 0x01, 0x03, 0x03, "Medium"                         },
	{0x12, 0x01, 0x03, 0x01, "Hard"                           },
	{0x12, 0x01, 0x03, 0x00, "Hardest"                        },
};

STDDIPINFO(Qzchikyu)

static struct BurnDIPInfo QzquestDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xfe, NULL                             },
	{0x12, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x11, 0x01, 0x01, 0x00, "Upright"                        },
	{0x11, 0x01, 0x01, 0x01, "Cocktail"                       },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x11, 0x01, 0x02, 0x02, "Off"                            },
	{0x11, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x11, 0x01, 0x04, 0x04, "Off"                            },
	{0x11, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x11, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x11, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x11, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x11, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x11, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x11, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x11, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x11, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x12, 0x01, 0x03, 0x02, "Easy"                           },
	{0x12, 0x01, 0x03, 0x03, "Medium"                         },
	{0x12, 0x01, 0x03, 0x01, "Hard"                           },
	{0x12, 0x01, 0x03, 0x00, "Hardest"                        },
};

STDDIPINFO(Qzquest)

static struct BurnDIPInfo SolfigtrDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xff, NULL                             },
	{0x16, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x15, 0x01, 0x02, 0x02, "Off"                            },
	{0x15, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x15, 0x01, 0x04, 0x04, "Off"                            },
	{0x15, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x15, 0x01, 0x08, 0x00, "Off"                            },
	{0x15, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x15, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x15, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x15, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x15, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x16, 0x01, 0x03, 0x02, "Easy"                           },
	{0x16, 0x01, 0x03, 0x03, "Medium"                         },
	{0x16, 0x01, 0x03, 0x01, "Hard"                           },
	{0x16, 0x01, 0x03, 0x00, "Hardest"                        },
};

STDDIPINFO(Solfigtr)

static struct BurnDIPInfo SsiDIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfe, NULL                             },
	{0x14, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x13, 0x01, 0x01, 0x00, "Upright"                        },
	{0x13, 0x01, 0x01, 0x01, "Cocktail"                       },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x13, 0x01, 0x02, 0x02, "Off"                            },
	{0x13, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x13, 0x01, 0x04, 0x04, "Off"                            },
	{0x13, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x13, 0x01, 0x08, 0x00, "Off"                            },
	{0x13, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x13, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x13, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x13, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x13, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x14, 0x01, 0x03, 0x02, "Easy"                           },
	{0x14, 0x01, 0x03, 0x03, "Medium"                         },
	{0x14, 0x01, 0x03, 0x01, "Hard"                           },
	{0x14, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Shields"                        },
	{0x14, 0x01, 0x0c, 0x00, "None"                           },
	{0x14, 0x01, 0x0c, 0x0c, "1"                              },
	{0x14, 0x01, 0x0c, 0x04, "2"                              },
	{0x14, 0x01, 0x0c, 0x08, "3"                              },

	{0   , 0xfe, 0   , 2   , "Lives"                          },
	{0x14, 0x01, 0x10, 0x00, "2"                              },
	{0x14, 0x01, 0x10, 0x10, "3"                              },

	{0   , 0xfe, 0   , 4   , "2 Player Mode"                  },
	{0x14, 0x01, 0xa0, 0xa0, "Simultaneous"                   },
	{0x14, 0x01, 0xa0, 0x80, "Alternate, Single"              },
	{0x14, 0x01, 0xa0, 0x00, "Alternate, Dual"                },
	{0x14, 0x01, 0xa0, 0x20, "Not Allowed"                    },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x14, 0x01, 0x40, 0x00, "Off"                            },
	{0x14, 0x01, 0x40, 0x40, "On"                             },
};

STDDIPINFO(Ssi)

static struct BurnDIPInfo Majest12DIPList[]=
{
	// Default Values
	{0x13, 0xff, 0xff, 0xfe, NULL                             },
	{0x14, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Cabinet"                        },
	{0x13, 0x01, 0x01, 0x00, "Upright"                        },
	{0x13, 0x01, 0x01, 0x01, "Cocktail"                       },

	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x13, 0x01, 0x02, 0x02, "Off"                            },
	{0x13, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x13, 0x01, 0x04, 0x04, "Off"                            },
	{0x13, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x13, 0x01, 0x08, 0x00, "Off"                            },
	{0x13, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x13, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x13, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x13, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x13, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x13, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x13, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x14, 0x01, 0x03, 0x02, "Easy"                           },
	{0x14, 0x01, 0x03, 0x03, "Medium"                         },
	{0x14, 0x01, 0x03, 0x01, "Hard"                           },
	{0x14, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 4   , "Shields"                        },
	{0x14, 0x01, 0x0c, 0x00, "None"                           },
	{0x14, 0x01, 0x0c, 0x0c, "1"                              },
	{0x14, 0x01, 0x0c, 0x04, "2"                              },
	{0x14, 0x01, 0x0c, 0x08, "3"                              },

	{0   , 0xfe, 0   , 2   , "Lives"                          },
	{0x14, 0x01, 0x10, 0x00, "2"                              },
	{0x14, 0x01, 0x10, 0x10, "3"                              },

	{0   , 0xfe, 0   , 4   , "2 Player Mode"                  },
	{0x14, 0x01, 0xa0, 0xa0, "Simultaneous"                   },
	{0x14, 0x01, 0xa0, 0x80, "Alternate, Single"              },
	{0x14, 0x01, 0xa0, 0x00, "Alternate, Dual"                },
	{0x14, 0x01, 0xa0, 0x20, "Not Allowed"                    },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x14, 0x01, 0x40, 0x00, "Off"                            },
	{0x14, 0x01, 0x40, 0x40, "On"                             },
};

STDDIPINFO(Majest12)

static struct BurnDIPInfo ThundfoxDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xff, NULL                             },
	{0x16, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x15, 0x01, 0x02, 0x02, "Off"                            },
	{0x15, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x15, 0x01, 0x04, 0x04, "Off"                            },
	{0x15, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x15, 0x01, 0x08, 0x00, "Off"                            },
	{0x15, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x15, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin 2 Credits"               },
	{0x15, 0x01, 0xc0, 0x80, "1 Coin 3 Credits"               },
	{0x15, 0x01, 0xc0, 0x40, "1 Coin 4 Credits"               },
	{0x15, 0x01, 0xc0, 0x00, "1 Coin 6 Credits"               },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x16, 0x01, 0x03, 0x02, "Easy"                           },
	{0x16, 0x01, 0x03, 0x03, "Medium"                         },
	{0x16, 0x01, 0x03, 0x01, "Hard"                           },
	{0x16, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 2   , "Timer"                          },
	{0x16, 0x01, 0x04, 0x00, "Off"                            },
	{0x16, 0x01, 0x04, 0x04, "On"                             },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x16, 0x01, 0x30, 0x20, "2"                              },
	{0x16, 0x01, 0x30, 0x30, "3"                              },
	{0x16, 0x01, 0x30, 0x10, "4"                              },
	{0x16, 0x01, 0x30, 0x00, "5"                              },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x16, 0x01, 0x40, 0x00, "Off"                            },
	{0x16, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x16, 0x01, 0x80, 0x00, "Single"                         },
	{0x16, 0x01, 0x80, 0x80, "Dual"                           },
};

STDDIPINFO(Thundfox)

static struct BurnDIPInfo ThundfoxjDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xff, NULL                             },
	{0x16, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x15, 0x01, 0x02, 0x02, "Off"                            },
	{0x15, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x15, 0x01, 0x04, 0x04, "Off"                            },
	{0x15, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x15, 0x01, 0x08, 0x00, "Off"                            },
	{0x15, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x15, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x15, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x15, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x15, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x16, 0x01, 0x03, 0x02, "Easy"                           },
	{0x16, 0x01, 0x03, 0x03, "Medium"                         },
	{0x16, 0x01, 0x03, 0x01, "Hard"                           },
	{0x16, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 2   , "Timer"                          },
	{0x16, 0x01, 0x04, 0x00, "Off"                            },
	{0x16, 0x01, 0x04, 0x04, "On"                             },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x16, 0x01, 0x30, 0x20, "2"                              },
	{0x16, 0x01, 0x30, 0x30, "3"                              },
	{0x16, 0x01, 0x30, 0x10, "4"                              },
	{0x16, 0x01, 0x30, 0x00, "5"                              },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x16, 0x01, 0x40, 0x00, "Off"                            },
	{0x16, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x16, 0x01, 0x80, 0x00, "Single"                         },
	{0x16, 0x01, 0x80, 0x80, "Dual"                           },
};

STDDIPINFO(Thundfoxj)

static struct BurnDIPInfo ThundfoxuDIPList[]=
{
	// Default Values
	{0x15, 0xff, 0xff, 0xff, NULL                             },
	{0x16, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x15, 0x01, 0x02, 0x02, "Off"                            },
	{0x15, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x15, 0x01, 0x04, 0x04, "Off"                            },
	{0x15, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x15, 0x01, 0x08, 0x00, "Off"                            },
	{0x15, 0x01, 0x08, 0x08, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coinage"                        },
	{0x15, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },

	{0   , 0xfe, 0   , 4   , "Price to Continue"              },
	{0x15, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x15, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x15, 0x01, 0xc0, 0x80, "1 Coin 1 Credit"                },
	{0x15, 0x01, 0xc0, 0xc0, "Same as start"                  },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x16, 0x01, 0x03, 0x02, "Easy"                           },
	{0x16, 0x01, 0x03, 0x03, "Medium"                         },
	{0x16, 0x01, 0x03, 0x01, "Hard"                           },
	{0x16, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 2   , "Timer"                          },
	{0x16, 0x01, 0x04, 0x00, "Off"                            },
	{0x16, 0x01, 0x04, 0x04, "On"                             },

	{0   , 0xfe, 0   , 4   , "Lives"                          },
	{0x16, 0x01, 0x30, 0x20, "2"                              },
	{0x16, 0x01, 0x30, 0x30, "3"                              },
	{0x16, 0x01, 0x30, 0x10, "4"                              },
	{0x16, 0x01, 0x30, 0x00, "5"                              },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x16, 0x01, 0x40, 0x00, "Off"                            },
	{0x16, 0x01, 0x40, 0x40, "On"                             },

	{0   , 0xfe, 0   , 2   , "Upright Controls"               },
	{0x16, 0x01, 0x80, 0x00, "Single"                         },
	{0x16, 0x01, 0x80, 0x80, "Dual"                           },
};

STDDIPINFO(Thundfoxu)

static struct BurnDIPInfo YesnojDIPList[]=
{
	// Default Values
	{0x09, 0xff, 0xff, 0xff, NULL                             },
	{0x0a, 0xff, 0xff, 0xfe, NULL                             },

	// Dip 1

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Results Printer"                },
	{0x0a, 0x01, 0x01, 0x00, "Off"                            },
	{0x0a, 0x01, 0x01, 0x01, "On"                             },

	{0   , 0xfe, 0   , 2   , "Demo Sounds"                    },
	{0x0a, 0x01, 0x02, 0x00, "Off"                            },
	{0x0a, 0x01, 0x02, 0x02, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coinage"                        },
	{0x0a, 0x01, 0x30, 0x00, "4 Coins 1 Credit"               },
	{0x0a, 0x01, 0x30, 0x10, "3 Coins 1 Credit"               },
	{0x0a, 0x01, 0x30, 0x20, "2 Coins 1 Credit"               },
	{0x0a, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
};

STDDIPINFO(Yesnoj)

static struct BurnDIPInfo YuyugogoDIPList[]=
{
	// Default Values
	{0x11, 0xff, 0xff, 0xff, NULL                             },
	{0x12, 0xff, 0xff, 0xff, NULL                             },

	// Dip 1
	{0   , 0xfe, 0   , 2   , "Flip Screen"                    },
	{0x11, 0x01, 0x02, 0x02, "Off"                            },
	{0x11, 0x01, 0x02, 0x00, "On"                             },

	{0   , 0xfe, 0   , 2   , "Service Mode"                   },
	{0x11, 0x01, 0x04, 0x04, "Off"                            },
	{0x11, 0x01, 0x04, 0x00, "On"                             },

	{0   , 0xfe, 0   , 4   , "Coin A"                         },
	{0x11, 0x01, 0x30, 0x00, "3 Coins 1 Credit"               },
	{0x11, 0x01, 0x30, 0x10, "2 Coins 1 Credit"               },
	{0x11, 0x01, 0x30, 0x30, "1 Coin  1 Credit"               },
	{0x11, 0x01, 0x30, 0x20, "1 Coin  2 Credits"              },

	{0   , 0xfe, 0   , 4   , "Coin B"                         },
	{0x11, 0x01, 0xc0, 0x00, "3 Coins 1 Credit"               },
	{0x11, 0x01, 0xc0, 0x40, "2 Coins 1 Credit"               },
	{0x11, 0x01, 0xc0, 0xc0, "1 Coin  1 Credit"               },
	{0x11, 0x01, 0xc0, 0x80, "1 Coin  2 Credits"              },

	// Dip 2
	{0   , 0xfe, 0   , 4   , "Difficulty"                     },
	{0x12, 0x01, 0x03, 0x02, "Easy"                           },
	{0x12, 0x01, 0x03, 0x03, "Medium"                         },
	{0x12, 0x01, 0x03, 0x01, "Hard"                           },
	{0x12, 0x01, 0x03, 0x00, "Hardest"                        },

	{0   , 0xfe, 0   , 2   , "Allow Continue"                 },
	{0x12, 0x01, 0x40, 0x00, "Off"                            },
	{0x12, 0x01, 0x40, 0x40, "On"                             },
};

STDDIPINFO(Yuyugogo)

static struct BurnRomInfo CameltryRomDesc[] = {
	{ "c38-11.ic10",        0x020000, 0xbe172da0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c38-13.ic11",        0x020000, 0x2c6a6ef7, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c38-08.ic25",        0x010000, 0x7ff78873, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c38-01.ic1",         0x080000, 0xc170ff36, BRF_GRA | TAITO_SPRITESA },

	{ "c38-03.ic2",         0x020000, 0x59fa59a7, BRF_SND | TAITO_YM2610A },

	{ "c38-02.ic27",        0x020000, 0x1a11714b, BRF_GRA | TAITO_CHARS_PIVOT },
};

STD_ROM_PICK(Cameltry)
STD_ROM_FN(Cameltry)

static struct BurnRomInfo CameltryuRomDesc[] = {
	{ "c38-11",             0x020000, 0xbe172da0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c38-14",             0x020000, 0xffa430de, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c38-08.bin",         0x010000, 0x7ff78873, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c38-01.bin",         0x080000, 0xc170ff36, BRF_GRA | TAITO_SPRITESA },

	{ "c38-03.bin",         0x020000, 0x59fa59a7, BRF_SND | TAITO_YM2610A },

	{ "c38-02.bin",         0x020000, 0x1a11714b, BRF_GRA | TAITO_CHARS_PIVOT },
};

STD_ROM_PICK(Cameltryu)
STD_ROM_FN(Cameltryu)

static struct BurnRomInfo CameltryauRomDesc[] = {
	{ "c38-11",             0x020000, 0xbe172da0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c38-14",             0x020000, 0xffa430de, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c38-15.bin",         0x010000, 0x0e60faac, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c38-01.bin",         0x080000, 0xc170ff36, BRF_GRA | TAITO_SPRITESA },

	{ "c38-02.bin",         0x020000, 0x1a11714b, BRF_GRA | TAITO_CHARS_PIVOT },

	{ "c38-04.bin",         0x020000, 0x53d330bb, BRF_SND | TAITO_MSM6295 },
};

STD_ROM_PICK(Cameltryau)
STD_ROM_FN(Cameltryau)

static struct BurnRomInfo CameltryaRomDesc[] = {
	{ "c38-11",             0x020000, 0xbe172da0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c38-16",             0x020000, 0x66ad6164, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c38-15.bin",         0x010000, 0x0e60faac, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c38-01.bin",         0x080000, 0xc170ff36, BRF_GRA | TAITO_SPRITESA },

	{ "c38-02.bin",         0x020000, 0x1a11714b, BRF_GRA | TAITO_CHARS_PIVOT },

	{ "c38-04.bin",         0x020000, 0x53d330bb, BRF_SND | TAITO_MSM6295 },
};

STD_ROM_PICK(Cameltrya)
STD_ROM_FN(Cameltrya)

static struct BurnRomInfo CameltryjRomDesc[] = {
	{ "c38-09.bin",         0x020000, 0x2ae01120, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c38-10.bin",         0x020000, 0x48d8ff56, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c38-08.bin",         0x010000, 0x7ff78873, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c38-01.bin",         0x080000, 0xc170ff36, BRF_GRA | TAITO_SPRITESA },

	{ "c38-03.bin",         0x020000, 0x59fa59a7, BRF_SND | TAITO_YM2610A },

	{ "c38-02.bin",         0x020000, 0x1a11714b, BRF_GRA | TAITO_CHARS_PIVOT },
};

STD_ROM_PICK(Cameltryj)
STD_ROM_FN(Cameltryj)

static struct BurnRomInfo DeadconxRomDesc[] = {
	{ "d28-06.3",           0x040000, 0x5b4bff51, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d28-12.5",           0x040000, 0x9b74e631, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d28-09.2",           0x040000, 0x143a0cc1, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d28-08.4",           0x040000, 0x4c872bd9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "d28-10.6",           0x010000, 0x40805d74, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "d28-04.16",          0x080000, 0xdcabc26b, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "d28-05.17",          0x080000, 0x862f9665, BRF_GRA | TAITO_CHARS_BYTESWAP },

	{ "d28-01.8",           0x100000, 0x181d7b69, BRF_GRA | TAITO_SPRITESA },
	{ "d28-02.9",           0x100000, 0xd301771c, BRF_GRA | TAITO_SPRITESA },

	{ "d28-03.10",          0x100000, 0xa1804b52, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Deadconx)
STD_ROM_FN(Deadconx)

static struct BurnRomInfo DeadconxjRomDesc[] = {
	{ "d28-06.3",           0x040000, 0x5b4bff51, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d28-07.5",           0x040000, 0x3fb8954c, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d28-09.2",           0x040000, 0x143a0cc1, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d28-08.4",           0x040000, 0x4c872bd9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "d28-10.6",           0x010000, 0x40805d74, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "d28-04.16",          0x080000, 0xdcabc26b, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "d28-05.17",          0x080000, 0x862f9665, BRF_GRA | TAITO_CHARS_BYTESWAP },

	{ "d28-01.8",           0x100000, 0x181d7b69, BRF_GRA | TAITO_SPRITESA },
	{ "d28-02.9",           0x100000, 0xd301771c, BRF_GRA | TAITO_SPRITESA },

	{ "d28-03.10",          0x100000, 0xa1804b52, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Deadconxj)
STD_ROM_FN(Deadconxj)

static struct BurnRomInfo DinorexRomDesc[] = {
	{ "d39-14.9",           0x080000, 0xe6aafdac, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d39-16.8",           0x080000, 0xcedc8537, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d39-04.6",           0x100000, 0x3800506d, BRF_ESS | BRF_PRG | TAITO_68KROM1 },
	{ "d39-05.7",           0x100000, 0xe2ec3b5d, BRF_ESS | BRF_PRG | TAITO_68KROM1 },

	{ "d39-12.5",           0x010000, 0x8292c7c1, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "d39-06.2",           0x100000, 0x52f62835, BRF_GRA | TAITO_CHARS },

	{ "d39-01.29",          0x200000, 0xd10e9c7d, BRF_GRA | TAITO_SPRITESA },
	{ "d39-02.28",          0x200000, 0x6c304403, BRF_GRA | TAITO_SPRITESA },
	{ "d39-03.27",          0x200000, 0xfc9cdab4, BRF_GRA | TAITO_SPRITESA },

	{ "d39-07.10",          0x100000, 0x28262816, BRF_SND | TAITO_YM2610A },
	{ "d39-08.4",           0x080000, 0x377b8b7b, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Dinorex)
STD_ROM_FN(Dinorex)

static struct BurnRomInfo DinorexjRomDesc[] = {
	{ "d39-14.9",           0x080000, 0xe6aafdac, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d39-13.8",           0x080000, 0xae496b2f, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d39-04.6",           0x100000, 0x3800506d, BRF_ESS | BRF_PRG | TAITO_68KROM1 },
	{ "d39-05.7",           0x100000, 0xe2ec3b5d, BRF_ESS | BRF_PRG | TAITO_68KROM1 },

	{ "d39-12.5",           0x010000, 0x8292c7c1, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "d39-06.2",           0x100000, 0x52f62835, BRF_GRA | TAITO_CHARS },

	{ "d39-01.29",          0x200000, 0xd10e9c7d, BRF_GRA | TAITO_SPRITESA },
	{ "d39-02.28",          0x200000, 0x6c304403, BRF_GRA | TAITO_SPRITESA },
	{ "d39-03.27",          0x200000, 0xfc9cdab4, BRF_GRA | TAITO_SPRITESA },

	{ "d39-07.10",          0x100000, 0x28262816, BRF_SND | TAITO_YM2610A },
	{ "d39-08.4",           0x080000, 0x377b8b7b, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Dinorexj)
STD_ROM_FN(Dinorexj)

static struct BurnRomInfo DinorexuRomDesc[] = {
	{ "d39-14.9",           0x080000, 0xe6aafdac, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d39-15.8",           0x080000, 0xfe96723b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d39-04.6",           0x100000, 0x3800506d, BRF_ESS | BRF_PRG | TAITO_68KROM1 },
	{ "d39-05.7",           0x100000, 0xe2ec3b5d, BRF_ESS | BRF_PRG | TAITO_68KROM1 },

	{ "d39-12.5",           0x010000, 0x8292c7c1, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "d39-06.2",           0x100000, 0x52f62835, BRF_GRA | TAITO_CHARS },

	{ "d39-01.29",          0x200000, 0xd10e9c7d, BRF_GRA | TAITO_SPRITESA },
	{ "d39-02.28",          0x200000, 0x6c304403, BRF_GRA | TAITO_SPRITESA },
	{ "d39-03.27",          0x200000, 0xfc9cdab4, BRF_GRA | TAITO_SPRITESA },

	{ "d39-07.10",          0x100000, 0x28262816, BRF_SND | TAITO_YM2610A },
	{ "d39-08.4",           0x080000, 0x377b8b7b, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Dinorexu)
STD_ROM_FN(Dinorexu)

static struct BurnRomInfo DondokodRomDesc[] = {
	{ "b95-12.bin",          0x020000, 0xd0fce87a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b95-11-1.bin",        0x020000, 0xdad40cd3, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b95-10.bin",          0x020000, 0xa46e1f0b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b95-14.bin",          0x020000, 0x6e4e1351, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "b95-08.bin",          0x010000, 0xb5aa49e1, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "b95-02.bin",          0x080000, 0x67b4e979, BRF_GRA | TAITO_CHARS },

	{ "b95-01.bin",          0x080000, 0x51c176ce, BRF_GRA | TAITO_SPRITESA },

	{ "b95-04.bin",          0x080000, 0xac4c1716, BRF_SND | TAITO_YM2610A },

	{ "b95-03.bin",          0x080000, 0x543aa0d1, BRF_GRA | TAITO_CHARS_PIVOT },
};

STD_ROM_PICK(Dondokod)
STD_ROM_FN(Dondokod)

static struct BurnRomInfo DondokodjRomDesc[] = {
	{ "b95-12.bin",          0x020000, 0xd0fce87a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b95-11-1.bin",        0x020000, 0xdad40cd3, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b95-10.bin",          0x020000, 0xa46e1f0b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b95-09.bin",          0x020000, 0xd8c86d39, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "b95-08.bin",          0x010000, 0xb5aa49e1, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "b95-02.bin",          0x080000, 0x67b4e979, BRF_GRA | TAITO_CHARS },

	{ "b95-01.bin",          0x080000, 0x51c176ce, BRF_GRA | TAITO_SPRITESA },

	{ "b95-04.bin",          0x080000, 0xac4c1716, BRF_SND | TAITO_YM2610A },

	{ "b95-03.bin",          0x080000, 0x543aa0d1, BRF_GRA | TAITO_CHARS_PIVOT },
};

STD_ROM_PICK(Dondokodj)
STD_ROM_FN(Dondokodj)

static struct BurnRomInfo DondokoduRomDesc[] = {
	{ "b95-12.bin",          0x020000, 0xd0fce87a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b95-11-1.bin",        0x020000, 0xdad40cd3, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b95-10.bin",          0x020000, 0xa46e1f0b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b95-13.bin",          0x020000, 0x350d2c65, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "b95-08.bin",          0x010000, 0xb5aa49e1, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "b95-02.bin",          0x080000, 0x67b4e979, BRF_GRA | TAITO_CHARS },

	{ "b95-01.bin",          0x080000, 0x51c176ce, BRF_GRA | TAITO_SPRITESA },

	{ "b95-04.bin",          0x080000, 0xac4c1716, BRF_SND | TAITO_YM2610A },

	{ "b95-03.bin",          0x080000, 0x543aa0d1, BRF_GRA | TAITO_CHARS_PIVOT },
};

STD_ROM_PICK(Dondokodu)
STD_ROM_FN(Dondokodu)

static struct BurnRomInfo DriftoutRomDesc[] = {
	{ "ic46.rom",           0x080000, 0x71303738, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "ic45.rom",           0x080000, 0x43f81eca, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "do_50.rom",          0x010000, 0xffe10124, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "do_obj.rom",         0x080000, 0x5491f1c4, BRF_GRA | TAITO_SPRITESA },

	{ "do_snd.rom",         0x080000, 0xf2deb82b, BRF_SND | TAITO_YM2610A },

	{ "do_piv.rom",         0x080000, 0xc4f012f7, BRF_GRA | TAITO_CHARS_PIVOT },
};

STD_ROM_PICK(Driftout)
STD_ROM_FN(Driftout)

static struct BurnRomInfo DriftoutjRomDesc[] = {
	{ "do_46.rom",          0x080000, 0xf960363e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "do_45.rom",          0x080000, 0xe3fe66b9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "do_50.rom",          0x010000, 0xffe10124, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "do_obj.rom",         0x080000, 0x5491f1c4, BRF_GRA | TAITO_SPRITESA },

	{ "do_snd.rom",         0x080000, 0xf2deb82b, BRF_SND | TAITO_YM2610A },

	{ "do_piv.rom",         0x080000, 0xc4f012f7, BRF_GRA | TAITO_CHARS_PIVOT },
};

STD_ROM_PICK(Driftoutj)
STD_ROM_FN(Driftoutj)

static struct BurnRomInfo DriveoutRomDesc[] = {
	{ "4.u3",       		0x080000, 0xdc431e4e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "5.u2",       		0x080000, 0x6f9063f4, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "3.u20",       		0x008000, 0x99aaeb2e, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "6.u84",       		0x040000, 0x530ac420, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "7.u81",       		0x040000, 0x0e9a3e9e, BRF_GRA | TAITO_SPRITESA_BYTESWAP },

	{ "8.u37",         		0x080000, 0xc4f012f7, BRF_GRA | TAITO_CHARS_PIVOT },

	{ "1.u28",       		0x080000, 0xcbde0b66, BRF_SND | TAITO_MSM6295 },
	{ "2.u29",       		0x020000, 0x0aba2026, BRF_SND | TAITO_MSM6295 },
};

STD_ROM_PICK(Driveout)
STD_ROM_FN(Driveout)

static struct BurnRomInfo FinalbRomDesc[] = {
	{ "b82-09.ic23",        0x20000, 0x632f1ecd, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b82-17.ic11",        0x20000, 0xe91b2ec9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "b82_10.ic5",         0x10000, 0xa38aaaed, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "b82-06.ic33",        0x20000, 0xfc450a25, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "b82-07.ic34",        0x20000, 0xec3df577, BRF_GRA | TAITO_CHARS_BYTESWAP },

	{ "b82-04.ic8",         0x80000, 0x6346f98e, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "b82-03.ic9",         0x80000, 0xdaa11561, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "b82-05.ic7",         0x80000, 0xaa90b93a, BRF_GRA | TAITO_SPRITESA },

	{ "b82-02.ic1",         0x80000, 0x5dd06bdd, BRF_SND | TAITO_YM2610A },

	{ "b82-01.ic2",         0x80000, 0xf0eb6846, BRF_SND | TAITO_YM2610B },

	{ "tibpal16l8.ic41",    0x00104, 0x11a0a19a, BRF_OPT },
	{ "tibpal16l8.ic42",    0x00104, 0xcc53deb8, BRF_OPT },
	{ "tibpal16l8.ic51",    0x00104, 0xf2878537, BRF_OPT },
	{ "gal16v8.ic13",       0x00117, 0xa4f75fd0, BRF_OPT },
	{ "gal16v8.ic35",       0x00117, 0xca4eb3e1, BRF_OPT },
};

STD_ROM_PICK(Finalb)
STD_ROM_FN(Finalb)

static struct BurnRomInfo FinalbjRomDesc[] = {
	{ "b82-09.ic23",        0x20000, 0x632f1ecd, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b82-08.ic11",        0x20000, 0x07154fe5, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "b82_10.ic5",         0x10000, 0xa38aaaed, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "b82-06.ic33",        0x20000, 0xfc450a25, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "b82-07.ic34",        0x20000, 0xec3df577, BRF_GRA | TAITO_CHARS_BYTESWAP },

	{ "b82-04.ic8",         0x80000, 0x6346f98e, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "b82-03.ic9",         0x80000, 0xdaa11561, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "b82-05.ic7",         0x80000, 0xaa90b93a, BRF_GRA | TAITO_SPRITESA },

	{ "b82-02.ic1",         0x80000, 0x5dd06bdd, BRF_SND | TAITO_YM2610A },

	{ "b82-01.ic2",         0x80000, 0xf0eb6846, BRF_SND | TAITO_YM2610B },

	{ "tibpal16l8.ic41",    0x00104, 0x11a0a19a, BRF_OPT },
	{ "tibpal16l8.ic42",    0x00104, 0xcc53deb8, BRF_OPT },
	{ "tibpal16l8.ic51",    0x00104, 0xf2878537, BRF_OPT },
	{ "gal16v8.ic13",       0x00117, 0xa4f75fd0, BRF_OPT },
	{ "gal16v8.ic35",       0x00117, 0xca4eb3e1, BRF_OPT },
};

STD_ROM_PICK(Finalbj)
STD_ROM_FN(Finalbj)

static struct BurnRomInfo FinalbuRomDesc[] = {
	{ "b82-09-1.ic23",      0x20000, 0x66729cb9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "b82-16-1.ic11",      0x20000, 0x879387fa, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "b82_10.ic5",         0x10000, 0xa38aaaed, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "b82-06.ic33",        0x20000, 0xfc450a25, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "b82-07.ic34",        0x20000, 0xec3df577, BRF_GRA | TAITO_CHARS_BYTESWAP },

	{ "b82-04.ic8",         0x80000, 0x6346f98e, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "b82-03.ic9",         0x80000, 0xdaa11561, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "b82-05.ic7",         0x80000, 0xaa90b93a, BRF_GRA | TAITO_SPRITESA },

	{ "b82-02.ic1",         0x80000, 0x5dd06bdd, BRF_SND | TAITO_YM2610A },

	{ "b82-01.ic2",         0x80000, 0xf0eb6846, BRF_SND | TAITO_YM2610B },

	{ "tibpal16l8.ic41",    0x00104, 0x11a0a19a, BRF_OPT },
	{ "tibpal16l8.ic42",    0x00104, 0xcc53deb8, BRF_OPT },
	{ "tibpal16l8.ic51",    0x00104, 0xf2878537, BRF_OPT },
	{ "gal16v8.ic13",       0x00117, 0xa4f75fd0, BRF_OPT },
	{ "gal16v8.ic35",       0x00117, 0xca4eb3e1, BRF_OPT },
};

STD_ROM_PICK(Finalbu)
STD_ROM_FN(Finalbu)

static struct BurnRomInfo FootchmpRomDesc[] = {
	{ "c80-11.6",           0x020000, 0xf78630fb, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c80-10.4",           0x020000, 0x32c109cb, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c80-12.7",           0x020000, 0x80d46fef, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c80-14.5",           0x020000, 0x40ac4828, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c80-15.70",          0x010000, 0x05aa7fd7, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c80-04.1",           0x080000, 0x9a17fe8c, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "c80-05.2",           0x080000, 0xacde7071, BRF_GRA | TAITO_CHARS_BYTESWAP },

	{ "c80-01.9",           0x100000, 0xf43782e6, BRF_GRA | TAITO_SPRITESA },
	{ "c80-02.10",          0x100000, 0x060a8b61, BRF_GRA | TAITO_SPRITESA },

	{ "c80-03.57",          0x100000, 0x609938d5, BRF_SND | TAITO_YM2610A },

	{ "c80-09.ic46",        0x000104, 0x4ca48869, BRF_OPT },
	{ "c80-08.ic45",        0x000104, 0x6137dd15, BRF_OPT },
};

STD_ROM_PICK(Footchmp)
STD_ROM_FN(Footchmp)

static struct BurnRomInfo HtherouRomDesc[] = {
	{ "c80-11.6",           0x020000, 0xf78630fb, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c80-10.4",           0x020000, 0x32c109cb, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c80-12.7",           0x020000, 0x80d46fef, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c80-13.5",           0x020000, 0x37ab78be, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c80-15.70",          0x010000, 0x05aa7fd7, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c80-04.1",           0x080000, 0x9a17fe8c, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "c80-05.2",           0x080000, 0xacde7071, BRF_GRA | TAITO_CHARS_BYTESWAP },

	{ "c80-01.9",           0x100000, 0xf43782e6, BRF_GRA | TAITO_SPRITESA },
	{ "c80-02.10",          0x100000, 0x060a8b61, BRF_GRA | TAITO_SPRITESA },

	{ "c80-03.57",          0x100000, 0x609938d5, BRF_SND | TAITO_YM2610A },

	{ "c80-08.ic45",        0x000104, 0x6137dd15, BRF_OPT },
	{ "c80-09.ic46",        0x000104, 0x4ca48869, BRF_OPT },
};

STD_ROM_PICK(Htherou)
STD_ROM_FN(Htherou)

static struct BurnRomInfo HtherojRomDesc[] = {
	{ "c80-16.6",           0x020000, 0x4e795b52, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c80-17.4",           0x020000, 0x42c0a838, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c80-12.7",           0x020000, 0x80d46fef, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c80-18.5",           0x020000, 0xaea22904, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c80-15.70",          0x010000, 0x05aa7fd7, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c80-04.1",           0x080000, 0x9a17fe8c, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "c80-05.2",           0x080000, 0xacde7071, BRF_GRA | TAITO_CHARS_BYTESWAP },

	{ "c80-01.9",           0x100000, 0xf43782e6, BRF_GRA | TAITO_SPRITESA },
	{ "c80-02.10",          0x100000, 0x060a8b61, BRF_GRA | TAITO_SPRITESA },

	{ "c80-03.57",          0x100000, 0x609938d5, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Htheroj)
STD_ROM_FN(Htheroj)

static struct BurnRomInfo Euroch92RomDesc[] = {
	{ "ec92_25.rom",        0x020000, 0x98482202, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "ec92_23.rom",        0x020000, 0xae5e75e9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "ec92_26.rom",        0x020000, 0xb986ccb2, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "ec92_24.rom",        0x020000, 0xb31d94ac, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "ec92_27.rom",        0x010000, 0x2db48e65, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "ec92_21.rom",        0x080000, 0x5759ed37, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "ec92_22.rom",        0x080000, 0xd9a0d38e, BRF_GRA | TAITO_CHARS_BYTESWAP },

	{ "ec92_19.rom",        0x100000, 0x219141a5, BRF_GRA | TAITO_SPRITESA },
	{ "c80-02.10",          0x100000, 0x060a8b61, BRF_GRA | TAITO_SPRITESA },

	{ "c80-03.57",          0x100000, 0x609938d5, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Euroch92)
STD_ROM_FN(Euroch92)

static struct BurnRomInfo Euroch92jRomDesc[] = {
	{ "c80-25.ic6",         0x020000, 0x98482202, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c80-23.ic4",         0x020000, 0xae5e75e9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c80-26.ic7",         0x020000, 0xb986ccb2, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c80-28.ic5",         0x020000, 0x5d13f580, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c80-27.ic70",        0x010000, 0x2db48e65, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c80-21.ic1",         0x080000, 0x5759ed37, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "c80-22.ic2",         0x080000, 0xd9a0d38e, BRF_GRA | TAITO_CHARS_BYTESWAP },

	{ "c80-19.ic9",         0x100000, 0x219141a5, BRF_GRA | TAITO_SPRITESA },
	{ "c80-20.ic10",        0x100000, 0x060a8b61, BRF_GRA | TAITO_SPRITESA },

	{ "c80-03.ic57",        0x100000, 0x609938d5, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Euroch92j)
STD_ROM_FN(Euroch92j)

static struct BurnRomInfo GrowlRomDesc[] = {
	{ "c74-10-1.ic59",      0x040000, 0x8bf17a85, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c74-08-1.ic61",      0x040000, 0xbc70396f, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c74-11.ic58",        0x040000, 0xee3bd6d5, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c74-14.ic60",        0x040000, 0xb6c24ec7, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c74-12.ic62",        0x010000, 0xbb6ed668, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c74-01.ic34",        0x100000, 0x3434ce80, BRF_GRA | TAITO_CHARS },

	{ "c74-03.ic12",        0x100000, 0x1a0d8951, BRF_GRA | TAITO_SPRITESA },
	{ "c74-02.ic11",        0x100000, 0x15a21506, BRF_GRA | TAITO_SPRITESA },

	{ "c74-04.ic28",        0x100000, 0x2d97edf2, BRF_SND | TAITO_YM2610A },
	{ "c74-05.ic29",        0x080000, 0xe29c0828, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Growl)
STD_ROM_FN(Growl)

static struct BurnRomInfo GrowlaRomDesc[] = {
	{ "c74-10.ic59",        0x040000, 0xca81a20b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c74-08.ic61",        0x040000, 0xaa35dd9e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c74-11.ic58",        0x040000, 0xee3bd6d5, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c74-14.ic60",        0x040000, 0xb6c24ec7, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c74-12.ic62",        0x010000, 0xbb6ed668, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c74-01.ic34",        0x100000, 0x3434ce80, BRF_GRA | TAITO_CHARS },

	{ "c74-03.ic12",        0x100000, 0x1a0d8951, BRF_GRA | TAITO_SPRITESA },
	{ "c74-02.ic11",        0x100000, 0x15a21506, BRF_GRA | TAITO_SPRITESA },

	{ "c74-04.ic28",        0x100000, 0x2d97edf2, BRF_SND | TAITO_YM2610A },
	{ "c74-05.ic29",        0x080000, 0xe29c0828, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Growla)
STD_ROM_FN(Growla)

static struct BurnRomInfo GrowluRomDesc[] = {
	{ "c74-10.ic59",        0x040000, 0xca81a20b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c74-08.ic61",        0x040000, 0xaa35dd9e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c74-11.ic58",        0x040000, 0xee3bd6d5, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c74-13.ic60",        0x040000, 0xc1c57e51, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c74-12.ic62",        0x010000, 0xbb6ed668, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c74-01.ic34",        0x100000, 0x3434ce80, BRF_GRA | TAITO_CHARS },

	{ "c74-03.ic12",        0x100000, 0x1a0d8951, BRF_GRA | TAITO_SPRITESA },
	{ "c74-02.ic11",        0x100000, 0x15a21506, BRF_GRA | TAITO_SPRITESA },

	{ "c74-04.ic28",        0x100000, 0x2d97edf2, BRF_SND | TAITO_YM2610A },
	{ "c74-05.ic29",        0x080000, 0xe29c0828, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Growlu)
STD_ROM_FN(Growlu)

static struct BurnRomInfo RunarkRomDesc[] = {
	{ "c74-10.ic59",        0x040000, 0xca81a20b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c74-08.ic61",        0x040000, 0xaa35dd9e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c74-11.ic58",        0x040000, 0xee3bd6d5, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c74-09.ic14",        0x040000, 0x58cc2feb, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c74-12.ic62",        0x010000, 0xbb6ed668, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c74-01.ic34",        0x100000, 0x3434ce80, BRF_GRA | TAITO_CHARS },

	{ "c74-03.ic12",        0x100000, 0x1a0d8951, BRF_GRA | TAITO_SPRITESA },
	{ "c74-02.ic11",        0x100000, 0x15a21506, BRF_GRA | TAITO_SPRITESA },

	{ "c74-04.ic28",        0x100000, 0x2d97edf2, BRF_SND | TAITO_YM2610A },
	{ "c74-05.ic29",        0x080000, 0xe29c0828, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Runark)
STD_ROM_FN(Runark)

static struct BurnRomInfo GrowlpRomDesc[] = {
	{ "growl_ic15_japan_0h_fb09.bin",    0x40000, 0x3a9141dc, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "growl_ic13_japan_0l_a80a.bin",    0x40000, 0xa8547fd6, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "growl_ic16_japan_1h_41bb.bin",    0x40000, 0x64aa6f4b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "growl_ic14_europe_1l_726b.bin",   0x40000, 0xc38bbb05, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "growl_ic3_snd.bin",               0x10000, 0xf75929e0, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "growl_ic11_scro-0-l_a971.bin",    0x40000, 0x769ddaab, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "growl_ic13_scro-0-0-h_2e7a.bin",  0x40000, 0x4e220e34, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "growl_ic12_scro-1-l_026e.bin",    0x40000, 0x486925b4, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "growl_ic14_scro-0-1-h_f0fa.bin",  0x40000, 0x42c2a2d0, BRF_GRA | TAITO_CHARS_BYTESWAP },

	{ "growl_ic17_obj0-0-l_90b9.bin",    0x40000, 0xcd94025a, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "growl_ic19_obj0-0-h_b652.bin",    0x40000, 0x6838c1b0, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "growl_ic18_obj0-1-l_a299.bin",    0x40000, 0x0ddf592e, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "growl_ic20_obj0-1-h_9f1a.bin",    0x40000, 0x0f0407f1, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "growl_ic4_obj1-l_7d96.bin",       0x40000, 0xbed51bd6, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "growl_ic6_obj1-0-h_3a22.bin",     0x40000, 0x5b696d20, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "growl_ic5_obj1-1-l_d34f.bin",     0x40000, 0xf34d83ec, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "growl_ic7_obj1-1-h_b5af.bin",     0x40000, 0xe9fda1fa, BRF_GRA | TAITO_SPRITESA_BYTESWAP },

	{ "growl_ic23_ch-a-0_b5d9.bin",      0x40000, 0xcc9ffbf8, BRF_SND | TAITO_YM2610A },
	{ "growl_ic24_ch-a-1_3c70.bin",      0x40000, 0x7177b4ad, BRF_SND | TAITO_YM2610A },
	{ "growl_ic25_ch-a-2_9614.bin",      0x40000, 0x7c9b1423, BRF_SND | TAITO_YM2610A },
	{ "growl_ic26_ch-a-3_fca6.bin",      0x40000, 0xdb1ecefe, BRF_SND | TAITO_YM2610A },
	{ "growl_ic21_ch-b-0_d743.bin",      0x40000, 0x7a7eab62, BRF_SND | TAITO_YM2610B },
	{ "growl_ic22_ch-b-1_a5f1.bin",      0x40000, 0x567df833, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Growlp)
STD_ROM_FN(Growlp)

static struct BurnRomInfo GunfrontRomDesc[] = {
	{ "c71-09.ic42",        0x020000, 0x10a544a2, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c71-08.ic41",        0x020000, 0xc17dc0a0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c71-10.ic40",        0x020000, 0xf39c0a06, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c71-14.ic39",        0x020000, 0x312da036, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c71-16.ic38",        0x020000, 0x1bbcc2d4, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c71-15.ic37",        0x020000, 0xdf3e00bb, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c71-12.ic49",        0x010000, 0x0038c7f8, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c71-02.ic59",        0x100000, 0x2a600c92, BRF_GRA | TAITO_CHARS },

	{ "c71-03.ic19",        0x100000, 0x9133c605, BRF_GRA | TAITO_SPRITESA },

	{ "c71-01.ic29",        0x100000, 0x0e73105a, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Gunfront)
STD_ROM_FN(Gunfront)

static struct BurnRomInfo GunfrontjRomDesc[] = {
	{ "c71-09.ic42",        0x020000, 0x10a544a2, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c71-08.ic41",        0x020000, 0xc17dc0a0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c71-10.ic40",        0x020000, 0xf39c0a06, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c71-11.ic39",        0x020000, 0xdf23c11a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c71-16.ic38",        0x020000, 0x1bbcc2d4, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c71-15.ic37",        0x020000, 0xdf3e00bb, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c71-12.ic49",        0x010000, 0x0038c7f8, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c71-02.ic59",        0x100000, 0x2a600c92, BRF_GRA | TAITO_CHARS },

	{ "c71-03.ic19",        0x100000, 0x9133c605, BRF_GRA | TAITO_SPRITESA },

	{ "c71-01.ic29",        0x100000, 0x0e73105a, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Gunfrontj)
STD_ROM_FN(Gunfrontj)

static struct BurnRomInfo KoshienRomDesc[] = {
	{ "c81-11.bin",         0x020000, 0xb44ea8c9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c81-10.bin",         0x020000, 0x8f98c40a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c81-04.bin",         0x080000, 0x1592b460, BRF_ESS | BRF_PRG | TAITO_68KROM1 },

	{ "c81-12.bin",         0x010000, 0x6e8625b6, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c81-03.bin",         0x100000, 0x29bbf492, BRF_GRA | TAITO_CHARS },

	{ "c81-01.bin",         0x100000, 0x64b15d2a, BRF_GRA | TAITO_SPRITESA },
	{ "c81-02.bin",         0x100000, 0x962461e8, BRF_GRA | TAITO_SPRITESA },

	{ "c81-05.bin",         0x080000, 0x9c3d71be, BRF_SND | TAITO_YM2610A },
	{ "c81-06.bin",         0x080000, 0x927833b4, BRF_SND | TAITO_YM2610B },

	{ "pal16l8b-c81-07.bin",0x000104, 0x46341732, BRF_OPT },
	{ "pal16l8b-c81-08.bin",0x000104, 0xe7d2d300, BRF_OPT },
	{ "pal16l8b-c81-09.bin",0x000104, 0xe4c012a1, BRF_OPT },
};

STD_ROM_PICK(Koshien)
STD_ROM_FN(Koshien)

static struct BurnRomInfo LiquidkRomDesc[] = {
	{ "c49-09.ic47",        0x20000, 0x6ae09eb9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c49-11.ic48",        0x20000, 0x42d2be6e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c49-10.ic45",        0x20000, 0x50bef2e0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c49-12.ic46",        0x20000, 0xcb16bad5, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c49-08.ic32",        0x10000, 0x413c310c, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c49-03.ic76",        0x80000, 0xc3364f9b, BRF_GRA | TAITO_CHARS },

	{ "c49-01.ic54",        0x80000, 0x67cc3163, BRF_GRA | TAITO_SPRITESA },
	{ "c49-02.ic53",        0x80000, 0xd2400710, BRF_GRA | TAITO_SPRITESA },

	{ "c49-04.ic33",        0x80000, 0x474d45a4, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Liquidk)
STD_ROM_FN(Liquidk)

static struct BurnRomInfo LiquidkuRomDesc[] = {
	{ "c49-09.ic47",        0x20000, 0x6ae09eb9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c49-11.ic48",        0x20000, 0x42d2be6e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c49-10.ic45",        0x20000, 0x50bef2e0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c49-14.ic46",        0x20000, 0xbc118a43, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c49-08.ic32",        0x10000, 0x413c310c, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c49-03.ic76",        0x80000, 0xc3364f9b, BRF_GRA | TAITO_CHARS },

	{ "c49-01.ic54",        0x80000, 0x67cc3163, BRF_GRA | TAITO_SPRITESA },
	{ "c49-02.ic53",        0x80000, 0xd2400710, BRF_GRA | TAITO_SPRITESA },

	{ "c49-04.ic33",        0x80000, 0x474d45a4, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Liquidku)
STD_ROM_FN(Liquidku)

static struct BurnRomInfo MizubakuRomDesc[] = {
	{ "c49-09.ic47",        0x20000, 0x6ae09eb9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c49-11.ic48",        0x20000, 0x42d2be6e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c49-10.ic45",        0x20000, 0x50bef2e0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c49-13.ic46",        0x20000, 0x2518dbf9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c49-08.ic32",        0x10000, 0x413c310c, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c49-03.ic76",        0x80000, 0xc3364f9b, BRF_GRA | TAITO_CHARS },

	{ "c49-01.ic54",        0x80000, 0x67cc3163, BRF_GRA | TAITO_SPRITESA },
	{ "c49-02.ic53",        0x80000, 0xd2400710, BRF_GRA | TAITO_SPRITESA },

	{ "c49-04.ic33",        0x80000, 0x474d45a4, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Mizubaku)
STD_ROM_FN(Mizubaku)

// Taito C-Chip BIOS
static struct BurnRomInfo emptyRomDesc[] = {
	{ "",                    0,          0, 0 },
};

static struct BurnRomInfo cchipRomDesc[] = {
#if !defined ROM_VERIFY
	{ "cchip_upd78c11.bin",		0x01000, 0x43021521, BRF_BIOS | TAITO_CCHIP_BIOS},
#endif
};

static struct BurnRomInfo MegablstRomDesc[] = {
	{ "c11-07.55",             0x020000, 0x11d228b6, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c11-08.39",             0x020000, 0xa79d4dca, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c11-06.54",             0x020000, 0x7c249894, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c11-11.38",             0x020000, 0x263ecbf9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c11-12.3",              0x010000, 0xb11094f1, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c11-05.58",             0x080000, 0x733e6d8e, BRF_GRA | TAITO_CHARS },

	{ "c11-03.32",             0x080000, 0x46718c7a, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "c11-04.31",             0x080000, 0x663f33cc, BRF_GRA | TAITO_SPRITESA_BYTESWAP },

	{ "c11-01.29",             0x080000, 0xfd1ea532, BRF_SND | TAITO_YM2610A },
	{ "c11-02.30",             0x080000, 0x451cc187, BRF_SND | TAITO_YM2610B },

	{ "c11-15.ic59",           0x002000, 0xaf49ee7f, BRF_ESS | BRF_PRG | TAITO_CCHIP_EEPROM },

	{ "pal16l8b-b89-01.ic8",   0x000104, 0x4095b97a, BRF_OPT },
	{ "pal16l8b-b89-02.ic28",  0x000104, 0x6430b559, BRF_OPT },
	{ "pal16l8b-b89-03.bin",   0x000104, 0x634592e2, BRF_OPT },
	{ "palce16v8-b89-04.ic27", 0x000117, 0xfc136ae2, BRF_OPT },
	{ "pal16l8b-c11-13.ic13",  0x000104, 0x421d7ea8, BRF_OPT },
	{ "pal16l8b-c11-14.ic23",  0x000104, 0x5c740aee, BRF_OPT },
};

STDROMPICKEXT(Megablst, Megablst, cchip)
STD_ROM_FN(Megablst)

static struct BurnRomInfo MegablstjRomDesc[] = {
	{ "c11-07.55",             0x020000, 0x11d228b6, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c11-08.39",             0x020000, 0xa79d4dca, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c11-06.54",             0x020000, 0x7c249894, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c11-09.38",             0x020000, 0xc830aad5, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c11-12.3",              0x010000, 0xb11094f1, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c11-05.58",             0x080000, 0x733e6d8e, BRF_GRA | TAITO_CHARS },

	{ "c11-03.32",             0x080000, 0x46718c7a, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "c11-04.31",             0x080000, 0x663f33cc, BRF_GRA | TAITO_SPRITESA_BYTESWAP },

	{ "c11-01.29",             0x080000, 0xfd1ea532, BRF_SND | TAITO_YM2610A },
	{ "c11-02.30",             0x080000, 0x451cc187, BRF_SND | TAITO_YM2610B },

	{ "c11-15.ic59",           0x002000, 0xaf49ee7f, BRF_ESS | BRF_PRG | TAITO_CCHIP_EEPROM },
};

STDROMPICKEXT(Megablstj, Megablstj, cchip)
STD_ROM_FN(Megablstj)

static struct BurnRomInfo MegablstuRomDesc[] = {
	{ "c11-07.55",             0x020000, 0x11d228b6, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c11-08.39",             0x020000, 0xa79d4dca, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c11-06.54",             0x020000, 0x7c249894, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c11-10.38",             0x020000, 0xbf379a43, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c11-12.3",              0x010000, 0xb11094f1, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c11-05.58",             0x080000, 0x733e6d8e, BRF_GRA | TAITO_CHARS },

	{ "c11-03.32",             0x080000, 0x46718c7a, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "c11-04.31",             0x080000, 0x663f33cc, BRF_GRA | TAITO_SPRITESA_BYTESWAP },

	{ "c11-01.29",             0x080000, 0xfd1ea532, BRF_SND | TAITO_YM2610A },
	{ "c11-02.30",             0x080000, 0x451cc187, BRF_SND | TAITO_YM2610B },

	{ "c11-15.ic59",           0x002000, 0xaf49ee7f, BRF_ESS | BRF_PRG | TAITO_CCHIP_EEPROM },
};

STDROMPICKEXT(Megablstu, Megablstu, cchip)
STD_ROM_FN(Megablstu)

static struct BurnRomInfo MetalbRomDesc[] = {
	{ "d16-16.8",           0x040000, 0x3150be61, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d16-18.7",           0x040000, 0x5216d092, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d12-07.9",           0x020000, 0xe07f5136, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d12-06.6",           0x020000, 0x131df731, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "d12-13.5",           0x020000, 0xbcca2649, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "d12-03.14",          0x080000, 0x46b498c0, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "d12-04.13",          0x080000, 0xab66d141, BRF_GRA | TAITO_CHARS_BYTESWAP },

	{ "d12-01.20",          0x100000, 0xb81523b9, BRF_GRA | TAITO_SPRITESA },

	{ "d12-02.10",          0x100000, 0x79263e74, BRF_SND | TAITO_YM2610A },
	{ "d12-05.16",          0x080000, 0x7fd036c5, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Metalb)
STD_ROM_FN(Metalb)

static struct BurnRomInfo MetalbaRomDesc[] = {
	{ "ic48",               0x040000, 0x556f82b2, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "ic37",               0x040000, 0x419083a1, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d12-07.ic47",        0x020000, 0xe07f5136, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d12-06.ic36",        0x020000, 0x131df731, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "ic57",               0x020000, 0xbcca2649, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "d12-03.ic55",        0x080000, 0x46b498c0, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "d12-04.ic54",        0x080000, 0xab66d141, BRF_GRA | TAITO_CHARS_BYTESWAP },

	{ "d12-01.ic19",        0x100000, 0xb81523b9, BRF_GRA | TAITO_SPRITESA },

	{ "d12-02.ic29",        0x100000, 0x79263e74, BRF_SND | TAITO_YM2610A },
	{ "d12-05.ic30",        0x080000, 0x7fd036c5, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Metalba)
STD_ROM_FN(Metalba)

static struct BurnRomInfo MetalbjRomDesc[] = {
	{ "d12-12.8",           0x040000, 0x556f82b2, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d12-11.7",           0x040000, 0xaf9ee28d, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d12-07.9",           0x020000, 0xe07f5136, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d12-06.6",           0x020000, 0x131df731, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "d12-13.5",           0x020000, 0xbcca2649, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "d12-03.14",          0x080000, 0x46b498c0, BRF_GRA | TAITO_CHARS_BYTESWAP },
	{ "d12-04.13",          0x080000, 0xab66d141, BRF_GRA | TAITO_CHARS_BYTESWAP },

	{ "d12-01.20",          0x100000, 0xb81523b9, BRF_GRA | TAITO_SPRITESA },

	{ "d12-02.10",          0x100000, 0x79263e74, BRF_SND | TAITO_YM2610A },
	{ "d12-05.16",          0x080000, 0x7fd036c5, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Metalbj)
STD_ROM_FN(Metalbj)

static struct BurnRomInfo MjnquestRomDesc[] = {
	{ "c77-09",                0x020000, 0x0a005d01, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c77-08",                0x020000, 0x4244f775, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c77-04",                0x080000, 0xc2e7e038, BRF_ESS | BRF_PRG | TAITO_68KROM1 },

	{ "c77-10",                0x010000, 0xf16b2c1e, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c77-01",                0x100000, 0x5ba51205, BRF_GRA | TAITO_CHARS },
	{ "c77-02",                0x100000, 0x6a6f3040, BRF_GRA | TAITO_CHARS },

	{ "c77-05",                0x080000, 0xc5a54678, BRF_GRA | TAITO_SPRITESA },

	{ "c77-03",                0x080000, 0x312f17b1, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Mjnquest)
STD_ROM_FN(Mjnquest)

static struct BurnRomInfo MjnquestbRomDesc[] = {
	{ "c77-09a",               0x020000, 0xfc17f1c2, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c77-08",                0x020000, 0x4244f775, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c77-04",                0x080000, 0xc2e7e038, BRF_ESS | BRF_PRG | TAITO_68KROM1 },

	{ "c77-10",                0x010000, 0xf16b2c1e, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c77-01",                0x100000, 0x5ba51205, BRF_GRA | TAITO_CHARS },
	{ "c77-02",                0x100000, 0x6a6f3040, BRF_GRA | TAITO_CHARS },

	{ "c77-05",                0x080000, 0xc5a54678, BRF_GRA | TAITO_SPRITESA },

	{ "c77-03",                0x080000, 0x312f17b1, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Mjnquestb)
STD_ROM_FN(Mjnquestb)

static struct BurnRomInfo NinjakRomDesc[] = {
	{ "c85-10x.ic50",          0x020000, 0xba7e6e74, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c85-13x.ic49",          0x020000, 0x0ac2cba2, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c85-07.ic48",           0x020000, 0x3eccfd0a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c85-06.ic47",           0x020000, 0xd126ded1, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c85-14.ic54",           0x010000, 0xf2a52a51, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c85-03.ic65",           0x080000, 0x4cc7b9df, BRF_GRA | TAITO_CHARS },

	{ "c85-01.ic19",           0x100000, 0xa711977c, BRF_GRA | TAITO_SPRITESA },
	{ "c85-02.ic17",           0x100000, 0xa6ad0f3d, BRF_GRA | TAITO_SPRITESA },

	{ "c85-04.ic31",           0x080000, 0x5afb747e, BRF_SND | TAITO_YM2610A },
	{ "c85-05.ic33",           0x080000, 0x3c1b0ed0, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Ninjak)
STD_ROM_FN(Ninjak)

static struct BurnRomInfo NinjakjRomDesc[] = {
	{ "c85-10x.ic50",          0x020000, 0xba7e6e74, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c85-11x.ic49",          0x020000, 0xe4ccaa8e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c85-07.ic48",           0x020000, 0x3eccfd0a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c85-06.ic47",           0x020000, 0xd126ded1, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c85-14.ic54",           0x010000, 0xf2a52a51, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c85-03.ic65",           0x080000, 0x4cc7b9df, BRF_GRA | TAITO_CHARS },

	{ "c85-01.ic19",           0x100000, 0xa711977c, BRF_GRA | TAITO_SPRITESA },
	{ "c85-02.ic17",           0x100000, 0xa6ad0f3d, BRF_GRA | TAITO_SPRITESA },

	{ "c85-04.ic31",           0x080000, 0x5afb747e, BRF_SND | TAITO_YM2610A },
	{ "c85-05.ic33",           0x080000, 0x3c1b0ed0, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Ninjakj)
STD_ROM_FN(Ninjakj)

static struct BurnRomInfo NinjakuRomDesc[] = {
	{ "c85-15x.ic50",          0x020000, 0x719a481b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c85-12x.ic49",          0x020000, 0x6c3f7e1e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c85-07.ic48",           0x020000, 0x3eccfd0a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c85-06.ic47",           0x020000, 0xd126ded1, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c85-14.ic54",           0x010000, 0xf2a52a51, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c85-03.ic65",           0x080000, 0x4cc7b9df, BRF_GRA | TAITO_CHARS },

	{ "c85-01.ic19",           0x100000, 0xa711977c, BRF_GRA | TAITO_SPRITESA },
	{ "c85-02.ic17",           0x100000, 0xa6ad0f3d, BRF_GRA | TAITO_SPRITESA },

	{ "c85-04.ic31",           0x080000, 0x5afb747e, BRF_SND | TAITO_YM2610A },
	{ "c85-05.ic33",           0x080000, 0x3c1b0ed0, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Ninjaku)
STD_ROM_FN(Ninjaku)

static struct BurnRomInfo PulirulaRomDesc[] = {
	{ "c98-12.rom",          0x040000, 0x816d6cde, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c98-16.rom",          0x040000, 0x59df5c77, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c98-06.rom",          0x020000, 0x64a71b45, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c98-07.rom",          0x020000, 0x90195bc0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c98-14.rom",          0x020000, 0xa858e17c, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c98-04.rom",          0x100000, 0x0e1fe3b2, BRF_GRA | TAITO_CHARS },

	{ "c98-02.rom",          0x100000, 0x4a2ad2b3, BRF_GRA | TAITO_SPRITESA },
	{ "c98-03.rom",          0x100000, 0x589a678f, BRF_GRA | TAITO_SPRITESA },

	{ "c98-01.rom",          0x100000, 0x197f66f5, BRF_SND | TAITO_YM2610A },

	{ "c98-05.rom",          0x080000, 0x9ddd9c39, BRF_GRA | TAITO_CHARS_PIVOT },
};

STD_ROM_PICK(Pulirula)
STD_ROM_FN(Pulirula)

static struct BurnRomInfo PulirulaaRomDesc[] = {
	// dumped from an original PCB without original ROM labels. The maincpu and audiocpu ROMs differ from the parent.
	{ "ic46.bin",            0x040000, 0x584ae599, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "ic45.bin",            0x040000, 0x08024086, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "ic44.bin",            0x020000, 0xda9d31fd, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "ic43.bin",            0x020000, 0x1feea319, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "ic50.bin",            0x020000, 0xe8b68cb8, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c98-04.rom",          0x100000, 0x0e1fe3b2, BRF_GRA | TAITO_CHARS },

	{ "c98-02.rom",          0x100000, 0x4a2ad2b3, BRF_GRA | TAITO_SPRITESA },
	{ "c98-03.rom",          0x100000, 0x589a678f, BRF_GRA | TAITO_SPRITESA },

	{ "c98-01.rom",          0x100000, 0x197f66f5, BRF_SND | TAITO_YM2610A },

	{ "c98-05.rom",          0x080000, 0x9ddd9c39, BRF_GRA | TAITO_CHARS_PIVOT },
};

STD_ROM_PICK(Pulirulaa)
STD_ROM_FN(Pulirulaa)

static struct BurnRomInfo PulirulajRomDesc[] = {
	{ "c98-12.rom",          0x040000, 0x816d6cde, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c98-13.rom",          0x040000, 0xb7d13d5b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c98-06.rom",          0x020000, 0x64a71b45, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c98-07.rom",          0x020000, 0x90195bc0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c98-14.rom",          0x020000, 0xa858e17c, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c98-04.rom",          0x100000, 0x0e1fe3b2, BRF_GRA | TAITO_CHARS },

	{ "c98-02.rom",          0x100000, 0x4a2ad2b3, BRF_GRA | TAITO_SPRITESA },
	{ "c98-03.rom",          0x100000, 0x589a678f, BRF_GRA | TAITO_SPRITESA },

	{ "c98-01.rom",          0x100000, 0x197f66f5, BRF_SND | TAITO_YM2610A },

	{ "c98-05.rom",          0x080000, 0x9ddd9c39, BRF_GRA | TAITO_CHARS_PIVOT },
};

STD_ROM_PICK(Pulirulaj)
STD_ROM_FN(Pulirulaj)

static struct BurnRomInfo QcrayonRomDesc[] = {
	{ "d55-13",                0x040000, 0x16afbfc7, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d55-14",                0x040000, 0x2fb3057f, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d55-03",                0x100000, 0x4d161e76, BRF_ESS | BRF_PRG | TAITO_68KROM1 },

	{ "d55-15",                0x010000, 0xba782eff, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "d55-02",                0x100000, 0xf3db2f1c, BRF_GRA | TAITO_CHARS },

	{ "d55-05",                0x100000, 0xf0e59902, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "d55-04",                0x100000, 0x412975ce, BRF_GRA | TAITO_SPRITESA_BYTESWAP },

	{ "d55-01",                0x100000, 0xa8309af4, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Qcrayon)
STD_ROM_FN(Qcrayon)

static struct BurnRomInfo Qcrayon2RomDesc[] = {
	{ "d63-12",                0x040000, 0x0f445a38, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d63-13",                0x040000, 0x74455752, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d63-01",                0x080000, 0x872e38b4, BRF_ESS | BRF_PRG | TAITO_68KROM1 },

	{ "d63-11",                0x010000, 0x2c7ac9e5, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "d63-03",                0x100000, 0xd24843af, BRF_GRA | TAITO_CHARS },

	{ "d63-06",                0x200000, 0x58b1e4a8, BRF_GRA | TAITO_SPRITESA },

	{ "d63-02",                0x100000, 0x162ae165, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Qcrayon2)
STD_ROM_FN(Qcrayon2)

static struct BurnRomInfo QjinseiRomDesc[] = {
	{ "d48-09",                0x040000, 0xa573b68d, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d48-10",                0x040000, 0x37143a5b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d48-03",                0x100000, 0xfb5ea8dc, BRF_ESS | BRF_PRG | TAITO_68KROM1 },

	{ "d48-11",                0x010000, 0x656c5b54, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "d48-04",                0x100000, 0x61e4b078, BRF_GRA | TAITO_CHARS },

	{ "d48-02",                0x100000, 0xa7b68e63, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "d48-01",                0x100000, 0x72a94b73, BRF_GRA | TAITO_SPRITESA_BYTESWAP },

	{ "d48-05",                0x080000, 0x3fefd058, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Qjinsei)
STD_ROM_FN(Qjinsei)

static struct BurnRomInfo QtorimonRomDesc[] = {
	{ "c41-04.bin",         0x020000, 0x0fbf5223, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c41-05.bin",         0x020000, 0x174bd5db, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "mask-51.bin",        0x020000, 0x12e14aca, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "mask-52.bin",        0x020000, 0xb3ef66f3, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c41-06.bin",         0x010000, 0x753a98d8, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c41-02.bin",         0x020000, 0x05dcd36d, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "c41-01.bin",         0x020000, 0x39ff043c, BRF_GRA | TAITO_SPRITESA_BYTESWAP },

	{ "c41-03.bin",         0x020000, 0xb2c18e89, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Qtorimon)
STD_ROM_FN(Qtorimon)

static struct BurnRomInfo QuizhqRomDesc[] = {
	{ "c53-05.bin",         0x020000, 0xc798fc20, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c53-01.bin",         0x020000, 0xbf44c93e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c53-52.bin",         0x020000, 0x12e14aca, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c53-51.bin",         0x020000, 0xb3ef66f3, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c53-08.bin",         0x010000, 0x25187e81, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c53-03.bin",         0x020000, 0x47596e70, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "c53-07.bin",         0x020000, 0x4f9fa82f, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "c53-02.bin",         0x020000, 0xd704c6f4, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "c53-06.bin",         0x020000, 0xf77f63fc, BRF_GRA | TAITO_SPRITESA_BYTESWAP },

	{ "c53-04.bin",         0x020000, 0x99890ad4, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Quizhq)
STD_ROM_FN(Quizhq)

static struct BurnRomInfo QzchikyuRomDesc[] = {
	{ "d19-06.8",           0x020000, 0xde8c8e55, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d19-05.7",           0x020000, 0xc6d099d0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d19-03.6",           0x080000, 0x5c1b92c0, BRF_ESS | BRF_PRG | TAITO_68KROM1 },

	{ "d19-07.5",           0x010000, 0xa8935f84, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "d19-02.10",          0x100000, 0xf2dce2f2, BRF_GRA | TAITO_CHARS },

	{ "d19-01.21",          0x100000, 0x6c4342d0, BRF_GRA | TAITO_SPRITESA },

	{ "d19-04.9",           0x080000, 0xd3c44905, BRF_SND | TAITO_YM2610A },

	{ "pal16l8b-d19-08.bin",0x000104, 0xc6240d10, BRF_OPT },
	{ "pal16l8b-d19-09.bin",0x000104, 0x576f5db9, BRF_OPT },
	{ "pal16l8b-d19-10.bin",0x000104, 0xea1232a5, BRF_OPT },
};

STD_ROM_PICK(Qzchikyu)
STD_ROM_FN(Qzchikyu)

static struct BurnRomInfo QzquestRomDesc[] = {
	{ "c92-06.8",           0x020000, 0x424be722, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c92-05.7",           0x020000, 0xda470f93, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c92-03.6",           0x080000, 0x1d697606, BRF_ESS | BRF_PRG | TAITO_68KROM1 },

	{ "c92-07.5",           0x010000, 0x3e313db9, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c92-02.10",          0x100000, 0x2daccecf, BRF_GRA | TAITO_CHARS },

	{ "c92-01.21",          0x100000, 0x9976a285, BRF_GRA | TAITO_SPRITESA },

	{ "c92-04.9",           0x080000, 0xe421bb43, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Qzquest)
STD_ROM_FN(Qzquest)

static struct BurnRomInfo SsiRomDesc[] = {
	{ "c64_15-1.ic9",       0x040000, 0xce9308a6, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c64_16-1.ic8",       0x040000, 0x470a483a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c64-09.13",          0x010000, 0x88d7f65c, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c64-01.1",           0x100000, 0xa1b4f486, BRF_GRA | TAITO_SPRITESA },

	{ "c64-02.2",           0x020000, 0x3cb0b907, BRF_SND | TAITO_YM2610A },

	// originals are protected PAL16L8s, these are brute-forced and reversed for GAL16V8s
	{ "c64-10.ic42",		0x000117, 0x08e8c3d6, BRF_OPT },
	{ "c64-11.ic43",		0x000117, 0xf116413e, BRF_OPT },

};

STD_ROM_PICK(Ssi)
STD_ROM_FN(Ssi)

static struct BurnRomInfo SsiaRomDesc[] = {
	{ "c64_15.ic9",       	0x040000, 0x3a6d591b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c64_16.ic8",       	0x040000, 0x8a567a4f, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c64-09.13",          0x010000, 0x88d7f65c, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c64-01.1",           0x100000, 0xa1b4f486, BRF_GRA | TAITO_SPRITESA },

	{ "c64-02.2",           0x020000, 0x3cb0b907, BRF_SND | TAITO_YM2610A },

	// originals are protected PAL16L8s, these are brute-forced and reversed for GAL16V8s
	{ "c64-10.ic42",		0x000117, 0x08e8c3d6, BRF_OPT },
	{ "c64-11.ic43",		0x000117, 0xf116413e, BRF_OPT },
};

STD_ROM_PICK(Ssia)
STD_ROM_FN(Ssia)

static struct BurnRomInfo SsibRomDesc[] = {
	{ "c64_12.ic9",       	0x040000, 0xd5716d7e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c64_13.ic8",       	0x040000, 0x99e3dd1c, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c64-09.ic38",        0x010000, 0x88d7f65c, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c64-01.ic51",        0x100000, 0xa1b4f486, BRF_GRA | TAITO_SPRITESA },

	{ "c64-02.ic1",         0x020000, 0x3cb0b907, BRF_SND | TAITO_YM2610A },

	// originals are protected PAL16L8s, these are brute-forced and reversed for GAL16V8s
	{ "c64-10.ic42",		0x000117, 0x08e8c3d6, BRF_OPT },
	{ "c64-11.ic43",		0x000117, 0xf116413e, BRF_OPT },
};

STD_ROM_PICK(Ssib)
STD_ROM_FN(Ssib)

static struct BurnRomInfo Majest12uRomDesc[] = {
	{ "c64_15-1.ic9",       0x040000, 0xce9308a6, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c64_17-1.ic8",       0x040000, 0x300d78ac, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c64-09.13",          0x010000, 0x88d7f65c, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c64-01.1",           0x100000, 0xa1b4f486, BRF_GRA | TAITO_SPRITESA },

	{ "c64-02.2",           0x020000, 0x3cb0b907, BRF_SND | TAITO_YM2610A },

	// originals are protected PAL16L8s, these are brute-forced and reversed for GAL16V8s
	{ "c64-10.ic42",		0x000117, 0x08e8c3d6, BRF_OPT },
	{ "c64-11.ic43",		0x000117, 0xf116413e, BRF_OPT },
};

STD_ROM_PICK(Majest12u)
STD_ROM_FN(Majest12u)

static struct BurnRomInfo Majest12uaRomDesc[] = {
	{ "c64_15.ic9",         0x040000, 0x3a6d591b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c64_17.ic8",         0x040000, 0xfd514ad9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c64-09.13",          0x010000, 0x88d7f65c, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c64-01.1",           0x100000, 0xa1b4f486, BRF_GRA | TAITO_SPRITESA },

	{ "c64-02.2",           0x020000, 0x3cb0b907, BRF_SND | TAITO_YM2610A },

	// originals are protected PAL16L8s, these are brute-forced and reversed for GAL16V8s
	{ "c64-10.ic42",		0x000117, 0x08e8c3d6, BRF_OPT },
	{ "c64-11.ic43",		0x000117, 0xf116413e, BRF_OPT },
};

STD_ROM_PICK(Majest12ua)
STD_ROM_FN(Majest12ua)

static struct BurnRomInfo Majest12ubRomDesc[] = {
	{ "c64_12.ic9",         0x040000, 0xd5716d7e, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c64_14.ic8",         0x040000, 0xeee4ed8a, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c64-09.13",          0x010000, 0x88d7f65c, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c64-01.1",           0x100000, 0xa1b4f486, BRF_GRA | TAITO_SPRITESA },

	{ "c64-02.2",           0x020000, 0x3cb0b907, BRF_SND | TAITO_YM2610A },

	// originals are protected PAL16L8s, these are brute-forced and reversed for GAL16V8s
	{ "c64-10.ic42",		0x000117, 0x08e8c3d6, BRF_OPT },
	{ "c64-11.ic43",		0x000117, 0xf116413e, BRF_OPT },
};

STD_ROM_PICK(Majest12ub)
STD_ROM_FN(Majest12ub)

static struct BurnRomInfo Majest12jRomDesc[] = {
	{ "c64-07.10",          0x020000, 0xf29ed5c9, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c64-08.11",          0x020000, 0xddfd33d5, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c64-06.4",           0x020000, 0x18dc71ac, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c64-05.5",           0x020000, 0xb61866c0, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c64-09.13",          0x010000, 0x88d7f65c, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c64-01.1",           0x100000, 0xa1b4f486, BRF_GRA | TAITO_SPRITESA },

	{ "c64-02.2",           0x020000, 0x3cb0b907, BRF_SND | TAITO_YM2610A },

	// originals are protected PAL16L8s, these are brute-forced and reversed for GAL16V8s
	{ "c64-10.ic42",		0x000117, 0x08e8c3d6, BRF_OPT },
	{ "c64-11.ic43",		0x000117, 0xf116413e, BRF_OPT },
};

STD_ROM_PICK(Majest12j)
STD_ROM_FN(Majest12j)

static struct BurnRomInfo SolfigtrRomDesc[] = {
	{ "c91-05.59",           0x040000, 0xc1260e7c, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c91-09.61",           0x040000, 0xd82b5266, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c91-07.62",           0x010000, 0xe471a05a, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c91-03.34",           0x100000, 0x8965da12, BRF_GRA | TAITO_CHARS },

	{ "c91-01.12",           0x100000, 0x0f3f4e00, BRF_GRA | TAITO_SPRITESA },
	{ "c91-02.11",           0x100000, 0xe14ab98e, BRF_GRA | TAITO_SPRITESA },

	{ "c91-04.28",           0x080000, 0x390b1065, BRF_SND | TAITO_YM2610A },

	{ "pal16l8b-c74-06.ic48",0x000104, 0xc868dc16, BRF_OPT },
};

STD_ROM_PICK(Solfigtr)
STD_ROM_FN(Solfigtr)

static struct BurnRomInfo ThundfoxRomDesc[] = {
	{ "c28-13-1.51",         0x020000, 0xacb07013, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c28-16-1.40",         0x020000, 0x1e43d55b, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c28-08.50",           0x020000, 0x38e038f1, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c28-07.39",           0x020000, 0x24419abb, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c28-14.3",            0x010000, 0x45ef3616, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c28-02.61",           0x080000, 0x6230a09d, BRF_GRA | TAITO_CHARS },

	{ "c28-01.63",           0x080000, 0x44552b25, BRF_GRA | TAITO_CHARSB },

	{ "c28-03.29",           0x080000, 0x51bdc7af, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "c28-04.28",           0x080000, 0xba7ed535, BRF_GRA | TAITO_SPRITESA_BYTESWAP },

	{ "c28-06.41",           0x080000, 0xdb6983db, BRF_SND | TAITO_YM2610A },
	{ "c28-05.42",           0x080000, 0xd3b238fa, BRF_SND | TAITO_YM2610B },

	{ "pal16l8b-b89-01.ic19",0x000104, 0x4095b97a, BRF_OPT },
	{ "pal16l8b-b89-03.ic37",0x000104, 0x634592e2, BRF_OPT },
	{ "pal16v8b-b89-04.ic33",0x000117, 0xfc136ae2, BRF_OPT },
	{ "pal16l8b-c28-09.ic25",0x000104, 0x383e7305, BRF_OPT },
	{ "pal16l8b-c28-10.ic26",0x000104, 0x47fccc07, BRF_OPT },
	{ "pal16l8b-c28-11.ic35",0x000104, 0x33414fe8, BRF_OPT },
};

STD_ROM_PICK(Thundfox)
STD_ROM_FN(Thundfox)

static struct BurnRomInfo ThundfoxjRomDesc[] = {
	{ "c28-13-1.51",         0x020000, 0xacb07013, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c28-12-1.40",         0x020000, 0xf04db477, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c28-08.50",           0x020000, 0x38e038f1, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c28-07.39",           0x020000, 0x24419abb, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c28-14.3",            0x010000, 0x45ef3616, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c28-02.61",           0x080000, 0x6230a09d, BRF_GRA | TAITO_CHARS },

	{ "c28-01.63",           0x080000, 0x44552b25, BRF_GRA | TAITO_CHARSB },

	{ "c28-03.29",           0x080000, 0x51bdc7af, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "c28-04.28",           0x080000, 0xba7ed535, BRF_GRA | TAITO_SPRITESA_BYTESWAP },

	{ "c28-06.41",           0x080000, 0xdb6983db, BRF_SND | TAITO_YM2610A },
	{ "c28-05.42",           0x080000, 0xd3b238fa, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Thundfoxj)
STD_ROM_FN(Thundfoxj)

static struct BurnRomInfo ThundfoxuRomDesc[] = {
	{ "c28-13-1.51",         0x020000, 0xacb07013, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c28-15-1.40",         0x020000, 0x874a84e1, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c28-08.50",           0x020000, 0x38e038f1, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c28-07.39",           0x020000, 0x24419abb, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "c28-14.3",            0x010000, 0x45ef3616, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c28-02.61",           0x080000, 0x6230a09d, BRF_GRA | TAITO_CHARS },

	{ "c28-01.63",           0x080000, 0x44552b25, BRF_GRA | TAITO_CHARSB },

	{ "c28-03.29",           0x080000, 0x51bdc7af, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "c28-04.28",           0x080000, 0xba7ed535, BRF_GRA | TAITO_SPRITESA_BYTESWAP },

	{ "c28-06.41",           0x080000, 0xdb6983db, BRF_SND | TAITO_YM2610A },
	{ "c28-05.42",           0x080000, 0xd3b238fa, BRF_SND | TAITO_YM2610B },
};

STD_ROM_PICK(Thundfoxu)
STD_ROM_FN(Thundfoxu)

static struct BurnRomInfo YesnojRomDesc[] = {
	{ "d20-05-2.2",            0x040000, 0x68adb929, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "d20-04-2.4",            0x040000, 0xa84762f8, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },

	{ "d20-06.5",              0x010000, 0x3eb537dc, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "d20-01.11",             0x080000, 0x9d8a4d57, BRF_GRA | TAITO_CHARS },

	{ "d20-02.12",             0x080000, 0xe71a8e40, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "d20-03.13",             0x080000, 0x6a51a1b4, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
};

STD_ROM_PICK(Yesnoj)
STD_ROM_FN(Yesnoj)

static struct BurnRomInfo YuyugogoRomDesc[] = {
	{ "c83-10.3",              0x020000, 0x4d185d03, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c83-09.2",              0x020000, 0xf9892792, BRF_ESS | BRF_PRG | TAITO_68KROM1_BYTESWAP },
	{ "c83-03.10",             0x100000, 0xeed9acc2, BRF_ESS | BRF_PRG | TAITO_68KROM1 },

	{ "c83-11.1",              0x010000, 0x461e702a, BRF_ESS | BRF_PRG | TAITO_Z80ROM1 },

	{ "c83-05.21",             0x020000, 0xeca57fb1, BRF_GRA | TAITO_CHARS },

	{ "c83-01.12",             0x100000, 0x8bf0d416, BRF_GRA | TAITO_SPRITESA_BYTESWAP },
	{ "c83-02.11",             0x100000, 0x20bb1c15, BRF_GRA | TAITO_SPRITESA_BYTESWAP },

	{ "c83-04.5",              0x100000, 0x2600093a, BRF_SND | TAITO_YM2610A },
};

STD_ROM_PICK(Yuyugogo)
STD_ROM_FN(Yuyugogo)

static INT32 MemIndex()
{
	UINT8 *Next; Next = TaitoMem;

	Taito68KRom1                    = Next; Next += Taito68KRom1Size;
	TaitoZ80Rom1                    = Next; Next += TaitoZ80Rom1Size;
	TaitoYM2610ARom                 = Next; Next += TaitoYM2610ARomSize;
	TaitoYM2610BRom                 = Next; Next += TaitoYM2610BRomSize;
	TaitoMSM6295Rom                 = Next; Next += TaitoMSM6295RomSize;

	cchip_rom                       = Next; Next += TaitoCCHIPBIOSSize;
	cchip_eeprom                    = Next; Next += TaitoCCHIPEEPROMSize;

	TaitoRamStart                   = Next;

	Taito68KRam1                    = Next; Next += 0x020000;
	TaitoZ80Ram1                    = Next; Next += 0x002000;
	TaitoSpriteRam                  = Next; Next += 0x010000;
	TaitoSpriteRamBuffered          = Next; Next += 0x010000;
	TaitoSpriteRamDelayed           = Next; Next += 0x010000;
	TaitoSpriteExtension            = Next; Next += 0x004000;
	TaitoPaletteRam                 = Next; Next += 0x004000;

	TaitoRamEnd                     = Next;

	TaitoChars                      = Next; Next += TaitoNumChar * TaitoCharWidth * TaitoCharHeight;
	if (TaitoNumCharB) { TaitoCharsB  = Next; Next += TaitoNumCharB * TaitoCharBWidth * TaitoCharBHeight; }
	TaitoCharsPivot                 = Next; Next += TaitoNumCharPivot * TaitoCharPivotWidth * TaitoCharPivotHeight;
	TaitoSpritesA                   = Next; Next += TaitoNumSpriteA * TaitoSpriteAWidth * TaitoSpriteAHeight;
	TaitoPalette                    = (UINT32*)Next; Next += 0x02000 * sizeof(UINT32);
	TaitoPriorityMap                = pPrioDraw; // from tiles_generic! //Next; Next += nScreenWidth * nScreenHeight;
	TaitoF2SpriteList               = (TaitoF2SpriteEntry*)Next; Next += 0x400 * sizeof(TaitoF2SpriteEntry);

	TaitoMemEnd                     = Next;

	return 0;
}

static INT32 TaitoF2DoReset()
{
	TaitoDoReset();

	YesnoDip = 0;
	MjnquestInput = 0;
	DriveoutSoundNibble = 0;
	DriveoutOkiBank = 0;

	TaitoF2SpritesFlipScreen = 0;
	TaitoF2SpriteBlendMode = 0;
	TaitoF2TilePriority[0] = TaitoF2TilePriority[1] = TaitoF2TilePriority[2] = TaitoF2TilePriority[3] = TaitoF2TilePriority[4] = 0;
	TaitoF2SpritePriority[0] = TaitoF2SpritePriority[1] = TaitoF2SpritePriority[2] = TaitoF2SpritePriority[3] = 0;

	return 0;
}

static void TaitoF2SpriteBankWrite(INT32 Offset, UINT16 Data)
{
	INT32 i, j;

	if (Offset < 2) return;
	if (Offset < 4) {
		j = (Offset & 1) << 1;
		i = Data << 11;
		TaitoF2SpriteBankBuffered[j + 0] = i;
		TaitoF2SpriteBankBuffered[j + 1] = i + 0x400;
	} else {
		i = Data << 10;
		TaitoF2SpriteBankBuffered[Offset] = i;
	}
}

static UINT8 __fastcall Cameltry68KReadByte(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x300000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Cameltry68KWriteByte(UINT32 a, UINT8 d)
{
	TC0220IOCHalfWordWrite_Map(0x300000)
	TC0100SCN0ByteWrite_Map(0x800000, 0x813fff)

	switch (a) {
		case 0x320000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x320002: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Cameltry68KReadWord(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x300000)

	switch (a) {
		case 0x300018: {
			INT32 Temp = TaitoAnalogPort0 >> 6;
			if (Temp >= 0x14 && Temp < 0x80) Temp = 0x14;
			if (Temp <= 0x3ec && Temp > 0x80) Temp = 0x3ec;
			return Temp;
		}

		case 0x30001c: {
			INT32 Temp = TaitoAnalogPort1 >> 6;
			if (Temp >= 0x14 && Temp < 0x80) Temp = 0x14;
			if (Temp <= 0x3ec && Temp > 0x80) Temp = 0x3ec;
			return Temp;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Cameltry68KWriteWord(UINT32 a, UINT16 d)
{
	TC0220IOCHalfWordWrite_Map(0x300000)
	TC0100SCN0WordWrite_Map(0x800000, 0x813fff)
	TC0100SCN0CtrlWordWrite_Map(0x820000)
	TC0280GRDCtrlWordWrite_Map(0xa02000)
	TC0360PRIHalfWordWrite_Map(0xd00000)

	if (a >= 0x814000 && a <= 0x814fff) return; //???

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Deadconx68KReadByte(UINT32 a)
{
	switch (a) {
		case 0x700001: {
			return TaitoDip[0];
		}

		case 0x700003: {
			return TaitoDip[1];
		}

		case 0x700005: {
			return TaitoInput[2];
		}

		case 0x700007: {
			return 0xff;
		}

		case 0x70000b: {
			return TaitoInput[0];
		}

		case 0x70000d: {
			return TaitoInput[1];
		}

		case 0x70000f: {
			return 0xff;
		}

		case 0x700011: {
			return 0xff;
		}

		case 0xa00002: {
			return TC0140SYTCommRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Deadconx68KWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0x700007: {
			// coin write
			return;
		}

		case 0xa00000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0xa00002: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Deadconx68KReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Deadconx68KWriteWord(UINT32 a, UINT16 d)
{
	TC0480SCPCtrlWordWrite_Map(0x430000)
	TC0360PRIHalfWordWrite_Map(0x500000)

	switch (a) {
		case 0x300000:
		case 0x300002:
		case 0x300004:
		case 0x300006:
		case 0x300008:
		case 0x30000a:
		case 0x30000c:
		case 0x30000e: {
			TaitoF2SpriteBankWrite((a - 0x300000) >> 1, d);
			return;
		}

		case 0x700006: {
			// coin write
			return;
		}

		case 0x700008:
		case 0x700012:
		case 0x700014:
		case 0x700016:
		case 0x700018: {
			// ???
			return;
		}

		case 0x800000: {
			// watchdog
			return;
		}

		case 0x900000: {
			// ???
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Dinorex68KReadByte(UINT32 a)
{
	TC0510NIOHalfWordRead_Map(0x300000)

	switch (a) {
		case 0xa00002: {
			return TC0140SYTCommRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Dinorex68KWriteByte(UINT32 a, UINT8 d)
{
	TC0510NIOHalfWordWrite_Map(0x300000)
	TC0100SCN0ByteWrite_Map(0x900000, 0x90ffff)

	switch (a) {
		case 0xa00000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0xa00002: {
			TC0140SYTCommWrite(d);
			return;
		}

		case 0xb00000: {
			// nop
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Dinorex68KReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Dinorex68KWriteWord(UINT32 a, UINT16 d)
{
	TC0510NIOHalfWordWrite_Map(0x300000)
	TC0360PRIHalfWordWrite_Map(0x700000)
	TC0100SCN0WordWrite_Map(0x900000, 0x90ffff)
	TC0100SCN0CtrlWordWrite_Map(0x920000)

	switch (a) {
		case 0xb00000: {
			// nop
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Dondokod68KReadByte(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x300000)

	switch (a) {
		case 0x320002: {
			return TC0140SYTCommRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Dondokod68KWriteByte(UINT32 a, UINT8 d)
{
	TC0220IOCHalfWordWrite_Map(0x300000)
	TC0100SCN0ByteWrite_Map(0x800000, 0x80ffff)

	switch (a) {
		case 0x320000:
		case 0x320001: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x320002:
		case 0x320003: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Dondokod68KReadWord(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x300000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Dondokod68KWriteWord(UINT32 a, UINT16 d)
{
	TC0220IOCHalfWordWrite_Map(0x300000)
	TC0100SCN0WordWrite_Map(0x800000, 0x80ffff)
	TC0100SCN0CtrlWordWrite_Map(0x820000)
	TC0280GRDCtrlWordWrite_Map(0xa02000)
	TC0360PRIHalfWordWrite_Map(0xb00000)

	switch (a) {
		case 0x360000: {
			//???
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Driftout68KReadByte(UINT32 a)
{
	TC0510NIOHalfWordRead_Map(0xb00000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Driftout68KWriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0x800000, 0x80ffff)
	TC0510NIOHalfWordWrite_Map(0xb00000)

	switch (a) {
		case 0x200000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x200002: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Driftout68KReadWord(UINT32 a)
{
	TC0510NIOHalfWordRead_Map(0xb00000)

	switch (a) {
		case 0x200000: {
			return 0;
		}

		case 0xb00018:
		case 0xb0001a: {
			//driftout_paddle_r
			return 0xff;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Driftout68KWriteWord(UINT32 a, UINT16 d)
{
	TC0430GRWCtrlWordWrite_Map(0x402000)
	TC0100SCN0WordWrite_Map(0x800000, 0x80ffff)
	TC0100SCN0CtrlWordWrite_Map(0x820000)
	TC0360PRIHalfWordSwapWrite_Map(0xa00000)
	TC0510NIOHalfWordWrite_Map(0xb00000)

	switch (a) {
		case 0xa00000: {
			//???
			return;
		}

		case 0x20019c: {
			//???
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static void __fastcall Driveout68KWriteByte(UINT32 a, UINT8 d)
{
	TC0510NIOHalfWordWrite_Map(0xb00000)

	switch (a) {
		case 0x200000: {
			DriveoutSoundNibble = d & 1;
			return;
		}

		case 0x200002: {
			if (!DriveoutSoundNibble) {
				TaitoSoundLatch = (d & 0x0f) | (TaitoSoundLatch & 0xf0);
			} else {
				TaitoSoundLatch = ((d << 4) & 0xf0) | (TaitoSoundLatch & 0x0f);
				ZetOpen(0);
				ZetSetIRQLine(0, CPU_IRQSTATUS_AUTO);
				ZetClose();
			}
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Finalb68KReadByte(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x300000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Finalb68KWriteByte(UINT32 a, UINT8 d)
{
	TC0220IOCHalfWordWrite_Map(0x300000)
	TC0100SCN0ByteWrite_Map(0x800000, 0x80ffff)

	switch (a) {
		case 0x320001: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x320003: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Finalb68KReadWord(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x300000)

	switch (a) {
		case 0x200002: {
			return TC0110PCRWordRead(0);
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Finalb68KWriteWord(UINT32 a, UINT16 d)
{
	TC0220IOCHalfWordWrite_Map(0x300000)
	TC0100SCN0WordWrite_Map(0x800000, 0x80ffff)
	TC0100SCN0CtrlWordWrite_Map(0x820000)

	if (a >= 0x810000 && a <= 0x81ffff) return; // NOP

	switch (a) {
		case 0x200000:
		case 0x200002: {
			TC0110PCRWordWrite(0, (a - 0x200000) >> 1, d);
			return;
		}

		case 0x200004: // hm?
		    return;

	    default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Footchmp68KReadByte(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Footchmp68KWriteByte(UINT32 a, UINT8 d)
{
	switch (a) {
		case 0xa00001: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0xa00003: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Footchmp68KReadWord(UINT32 a)
{
	switch (a) {
		case 0x700000: {
			return TaitoDip[0];
		}

		case 0x700002: {
			return TaitoDip[1];
		}

		case 0x700004: {
			return TaitoInput[2];
		}

		case 0x70000a: {
			return TaitoInput[0];
		}

		case 0x70000c: {
			return TaitoInput[1];
		}

		case 0x70000e: {
			return TaitoInput[3];
		}

		case 0x700010: {
			return TaitoInput[4];
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0xffff;
}

static void __fastcall Footchmp68KWriteWord(UINT32 a, UINT16 d)
{
	TC0480SCPCtrlWordWrite_Map(0x430000)
	TC0360PRIHalfWordWrite_Map(0x500000)

	switch (a) {
		case 0x300000:
		case 0x300002:
		case 0x300004:
		case 0x300006:
		case 0x300008:
		case 0x30000a:
		case 0x30000c:
		case 0x30000e: {
			TaitoF2SpriteBankWrite((a - 0x300000) >> 1, d);
			return;
		}

		case 0x700006:
		case 0x700008:
		case 0x700012:
		case 0x700014:
		case 0x700016:
		case 0x700018: {
			return;
		}

		case 0x800000: {
			// watchdog
			return;
		}

		case 0x900000: {
			// ????
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Growl68KReadByte(UINT32 a)
{
	switch (a) {
		case 0x320001: {
			return TaitoInput[0];
		}

		case 0x320003: {
			return TaitoInput[1];
		}

		case 0x400002: {
			return TC0140SYTCommRead();
		}

		case 0x508000: {
			return TaitoInput[4];
		}

		case 0x508001: {
			return TaitoInput[3];
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Growl68KWriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0x800000, 0x80ffff)

	switch (a) {
		case 0x400000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x400002: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Growl68KReadWord(UINT32 a)
{
	switch (a) {
		case 0x300000: {
			return TaitoDip[0];
		}

		case 0x300002: {
			return TaitoDip[1];
		}

		case 0x320000: {
			return TaitoInput[0];
		}

		case 0x320004: {
			return TaitoInput[2];
		}

		case 0x50c000: {
			return TaitoInput[5];
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Growl68KWriteWord(UINT32 a, UINT16 d)
{
	TC0360PRIHalfWordWrite_Map(0xb00000)
	TC0100SCN0WordWrite_Map(0x800000, 0x80ffff)
	TC0100SCN0CtrlWordWrite_Map(0x820000)

	switch (a) {
		case 0x300004: {
			// coin write
			return;
		}

		case 0x340000: {
			// watchdog
			return;
		}

		case 0x500000:
		case 0x500002:
		case 0x500004:
		case 0x500006:
		case 0x500008:
		case 0x50000a:
		case 0x50000c:
		case 0x50000e: {
			TaitoF2SpriteBankWrite((a - 0x500000) >> 1, d);
			return;
		}

		case 0x504000: {
			// nop
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Gunfront68KReadByte(UINT32 a)
{
	TC0510NIOHalfWordSwapRead_Map(0x300000)

	switch (a) {
		case 0x320002: {
			return TC0140SYTCommRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Gunfront68KWriteByte(UINT32 a, UINT8 d)
{
	TC0510NIOHalfWordSwapWrite_Map(0x300000)
	TC0100SCN0ByteWrite_Map(0x800000, 0x80ffff)

	switch (a) {
		case 0x320000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x320002: {
			TC0140SYTCommWrite(d);
			return;
		}

		case 0xa00000: {
			// nop
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Gunfront68KReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Gunfront68KWriteWord(UINT32 a, UINT16 d)
{
	TC0510NIOHalfWordSwapWrite_Map(0x300000)
	TC0100SCN0WordWrite_Map(0x800000, 0x80ffff)
	TC0100SCN0CtrlWordWrite_Map(0x820000)
	TC0360PRIHalfWordWrite_Map(0xb00000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Koshien68KReadByte(UINT32 a)
{
	TC0510NIOHalfWordRead_Map(0x300000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Koshien68KWriteByte(UINT32 a, UINT8 d)
{
	TC0510NIOHalfWordWrite_Map(0x300000)
	TC0100SCN0ByteWrite_Map(0x800000, 0x80ffff)
	TC0360PRIHalfWordWrite_Map(0xb00000)

	switch (a) {
		case 0x320000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x320002: {
			TC0140SYTCommWrite(d);
			return;
		}

		case 0x340000: {
			//???
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Koshien68KReadWord(UINT32 a)
{
	TC0510NIOHalfWordRead_Map(0x300000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Koshien68KWriteWord(UINT32 a, UINT16 d)
{
	TC0510NIOHalfWordWrite_Map(0x300000)
	TC0100SCN0WordWrite_Map(0x800000, 0x80ffff)
	TC0100SCN0CtrlWordWrite_Map(0x820000)
	TC0360PRIHalfWordWrite_Map(0xb00000)

	switch (a) {
		case 0xa20000: {
			TaitoF2SpriteBankBuffered[0] = 0x0000;
			TaitoF2SpriteBankBuffered[1] = 0x0400;
			TaitoF2SpriteBankBuffered[2] =  ((d & 0x00f) + 1) * 0x800;
			TaitoF2SpriteBankBuffered[4] = (((d & 0x0f0) >> 4) + 1) * 0x800;
			TaitoF2SpriteBankBuffered[6] = (((d & 0xf00) >> 8) + 1) * 0x800;
			TaitoF2SpriteBankBuffered[3] = TaitoF2SpriteBankBuffered[2] + 0x400;
			TaitoF2SpriteBankBuffered[5] = TaitoF2SpriteBankBuffered[4] + 0x400;
			TaitoF2SpriteBankBuffered[7] = TaitoF2SpriteBankBuffered[6] + 0x400;
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Liquidk68KReadByte(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x300000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Liquidk68KWriteByte(UINT32 a, UINT8 d)
{
	TC0220IOCHalfWordWrite_Map(0x300000)
	TC0100SCN0ByteWrite_Map(0x800000, 0x80ffff)

	switch (a) {
		case 0x320001: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x320003: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Liquidk68KReadWord(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x300000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Liquidk68KWriteWord(UINT32 a, UINT16 d)
{
	TC0220IOCHalfWordWrite_Map(0x300000)
	TC0100SCN0WordWrite_Map(0x800000, 0x80ffff)
	TC0100SCN0CtrlWordWrite_Map(0x820000)
	TC0360PRIHalfWordWrite_Map(0xb00000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Megablst68KReadByte(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x120000)

	CCHIP_READ(0x180000)

	switch (a) {
		case 0x100002: {
			return TC0140SYTCommRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Megablst68KWriteByte(UINT32 a, UINT8 d)
{
	TC0220IOCHalfWordWrite_Map(0x120000)
	TC0360PRIHalfWordWrite_Map(0x400000)
	TC0100SCN0ByteWrite_Map(0x600000, 0x60ffff)

	CCHIP_WRITE(0x180000)

	switch (a) {
		case 0x100000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x100002: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Megablst68KReadWord(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x120000)

	CCHIP_READ(0x180000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Megablst68KWriteWord(UINT32 a, UINT16 d)
{
	TC0220IOCHalfWordWrite_Map(0x120000)
	TC0360PRIHalfWordWrite_Map(0x400000)
	TC0100SCN0WordWrite_Map(0x600000, 0x60ffff)
	TC0100SCN0CtrlWordWrite_Map(0x620000)

	CCHIP_WRITE(0x180000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Metalb68KReadByte(UINT32 a)
{
	TC0510NIOHalfWordSwapRead_Map(0x800000)

	switch (a) {
		case 0x900002: {
			return TC0140SYTCommRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Metalb68KWriteByte(UINT32 a, UINT8 d)
{
	TC0510NIOHalfWordSwapWrite_Map(0x800000)

	switch (a) {
		case 0x900000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x900002: {
			TC0140SYTCommWrite(d);
			return;
		}

		case 0xa00000: {
			//???
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Metalb68KReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Metalb68KWriteWord(UINT32 a, UINT16 d)
{
	TC0480SCPCtrlWordWrite_Map(0x530000)
	TC0360PRIHalfWordWrite_Map(0x600000)
	TC0510NIOHalfWordSwapWrite_Map(0x800000)

	switch (a) {
		case 0x42000c:
		case 0x42000e: {
			//???
			return;
		}

		case 0xa00000: {
			//???
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Mjnquest68KReadByte(UINT32 a)
{
	switch (a) {
		case 0x300000: {
			return TaitoInput[4];
		}

		case 0x300001: {
			return TaitoDip[0];
		}

		case 0x300002: {
			return TaitoInput[5];
		}

		case 0x300003: {
			return TaitoDip[1];
		}

		case 0x360002: {
			return TC0140SYTCommRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Mjnquest68KWriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0x400000, 0x40ffff)

	switch (a) {
		case 0x330000:
		case 0x330001: {
			//nop
			return;
		}

		case 0x360000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x360002: {
			TC0140SYTCommWrite(d);
			return;
		}

		case 0x360005: {
			//???
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Mjnquest68KReadWord(UINT32 a)
{
	switch (a) {
		case 0x310000: {
			switch (MjnquestInput) {
				case 0x01: return TaitoInput[0];
				case 0x02: return TaitoInput[1];
				case 0x04: return TaitoInput[2];
				case 0x08: return TaitoInput[3];
				case 0x10: return 0xff;
			}
			return 0xff;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Mjnquest68KWriteWord(UINT32 a, UINT16 d)
{
	TC0100SCN0WordWrite_Map(0x400000, 0x40ffff)
	TC0100SCN0CtrlWordWrite_Map(0x420000)

	switch (a) {
		case 0x200000:
		case 0x200002: {
			TC0110PCRWordWrite(0, (a - 0x200000) >> 1, d);
			return;
		}

		case 0x200004: {
			//nop
			return;
		}

		case 0x300000: {
			//???
			return;
		}

		case 0x320000: {
			MjnquestInput = d >> 6;
			return;
		}

		case 0x350000: {
			//nop
			return;
		}

		case 0x380000: {
			TC0100SCNSetGfxBank(0, d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Ninjak68KReadByte(UINT32 a)
{
	switch (a) {
		case 0x300000: {
			return TaitoDip[0];
		}

		case 0x300002: {
			return TaitoDip[1];
		}

		case 0x300004: {
			return TaitoInput[0];
		}

		case 0x300006: {
			return TaitoInput[1];
		}

		case 0x300008: {
			return TaitoInput[3];
		}

		case 0x30000a: {
			return TaitoInput[4];
		}

		case 0x30000c: {
			return TaitoInput[2];
		}

		case 0x400002: {
			return TC0140SYTCommRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Ninjak68KWriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0x800000, 0x80ffff)

	switch (a) {
		case 0x30000e: {
			//coin write
			return;
		}

		case 0x300010: {
			//???
			return;
		}

		case 0x400000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x400002: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Ninjak68KReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Ninjak68KWriteWord(UINT32 a, UINT16 d)
{
	TC0100SCN0WordWrite_Map(0x800000, 0x80ffff)
	TC0100SCN0CtrlWordWrite_Map(0x820000)
	TC0360PRIHalfWordWrite_Map(0xb00000)

	switch (a) {
		case 0x300000:
		case 0x300012:
		case 0x300014:
		case 0x300016:
		case 0x300018: {
			//???
			return;
		}

		case 0x380000: {
			//watchdog
			return;
		}

		case 0x600000:
		case 0x600002:
		case 0x600004:
		case 0x600006:
		case 0x600008:
		case 0x60000a:
		case 0x60000c:
		case 0x60000e: {
			TaitoF2SpriteBankWrite((a - 0x600000) >> 1, d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Pulirula68KReadByte(UINT32 a)
{
	TC0510NIOHalfWordRead_Map(0xb00000)

	switch (a) {
		case 0x200002: {
			return TC0140SYTCommRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Pulirula68KWriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0x800000, 0x80ffff)
	TC0510NIOHalfWordWrite_Map(0xb00000)

	switch (a) {
		case 0x200000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x200002: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Pulirula68KReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Pulirula68KWriteWord(UINT32 a, UINT16 d)
{
	TC0430GRWCtrlWordWrite_Map(0x402000)
	TC0100SCN0WordWrite_Map(0x800000, 0x80ffff)
	TC0100SCN0CtrlWordWrite_Map(0x820000)
	TC0360PRIHalfWordSwapWrite_Map(0xa00000)
	TC0510NIOHalfWordWrite_Map(0xb00000)

	switch (a) {
		case 0x500000: {
			//???
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Qcrayon68KReadByte(UINT32 a)
{
	TC0510NIOHalfWordRead_Map(0xa00000)

	switch (a) {
		case 0x500002: {
			return TC0140SYTCommRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Qcrayon68KWriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0x900000, 0x90ffff)
	TC0510NIOHalfWordWrite_Map(0xa00000)

	switch (a) {
		case 0x200000: {
			//???
			return;
		}

		case 0x500000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x500002: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Qcrayon68KReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Qcrayon68KWriteWord(UINT32 a, UINT16 d)
{
	TC0100SCN0WordWrite_Map(0x900000, 0x90ffff)
	TC0100SCN0CtrlWordWrite_Map(0x920000)
	TC0510NIOHalfWordWrite_Map(0xa00000)
	TC0360PRIHalfWordWrite_Map(0xb00000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Qcrayon268KReadByte(UINT32 a)
{
	TC0510NIOHalfWordRead_Map(0x700000)

	switch (a) {
		case 0xa00002: {
			return TC0140SYTCommRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Qcrayon268KWriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0x500000, 0x50ffff)
	TC0510NIOHalfWordWrite_Map(0x700000)

	switch (a) {
		case 0x800000: {
			//???
			return;
		}

		case 0xa00000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0xa00002: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Qcrayon268KReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Qcrayon268KWriteWord(UINT32 a, UINT16 d)
{
	TC0100SCN0WordWrite_Map(0x500000, 0x50ffff)
	TC0100SCN0CtrlWordWrite_Map(0x520000)
	TC0510NIOHalfWordWrite_Map(0x700000)
	TC0360PRIHalfWordWrite_Map(0x900000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Qjinsei68KReadByte(UINT32 a)
{
	TC0510NIOHalfWordRead_Map(0xb00000)

	switch (a) {
		case 0x200002: {
			return TC0140SYTCommRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Qjinsei68KWriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0x800000, 0x80ffff)
	TC0510NIOHalfWordWrite_Map(0xb00000)

	switch (a) {
		case 0x200000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x200002: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Qjinsei68KReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Qjinsei68KWriteWord(UINT32 a, UINT16 d)
{
	TC0100SCN0WordWrite_Map(0x800000, 0x80ffff)
	TC0100SCN0CtrlWordWrite_Map(0x820000)
	TC0360PRIHalfWordWrite_Map(0xa00000)
	TC0510NIOHalfWordWrite_Map(0xb00000)

	switch (a) {
		case 0x500000: {
			//nop
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Qtorimon68KReadByte(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x500000)

	switch (a) {
		case 0x600002: {
			return TC0140SYTCommRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Qtorimon68KWriteByte(UINT32 a, UINT8 d)
{
	TC0220IOCHalfWordWrite_Map(0x500000)
	TC0100SCN0ByteWrite_Map(0x800000, 0x80ffff)

	switch (a) {
		case 0x600000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x600002: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Qtorimon68KReadWord(UINT32 a)
{
	switch (a) {
		case 0x200002: {
			return TC0110PCRWordRead(0);
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Qtorimon68KWriteWord(UINT32 a, UINT16 d)
{
	TC0220IOCHalfWordWrite_Map(0x500000)
	TC0100SCN0WordWrite_Map(0x800000, 0x80ffff)
	TC0100SCN0CtrlWordWrite_Map(0x820000)

	if (a >= 0x810000 && a <= 0x81ffff) return; // NOP

	switch (a) {
		case 0x200000:
		case 0x200002: {
			TC0110PCRWordWrite(0, (a - 0x200000) >> 1, d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Quizhq68KReadByte(UINT32 a)
{
	switch (a) {
		case 0x500001: {
			return TaitoDip[1];
		}

		case 0x500003: {
			return TaitoInput[0];
		}

		case 0x580001: {
			return TaitoDip[0];
		}

		case 0x580003: {
			return TaitoInput[1];
		}

		case 0x580005: {
			return TaitoInput[2];
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Quizhq68KWriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0x800000, 0x80ffff)

	switch (a) {
		case 0x500005: {
			//coin write
			return;
		}

		case 0x500007: {
			//???
			return;
		}

		case 0x580007: {
			//nop
			return;
		}

		case 0x600001: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x600003: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Quizhq68KReadWord(UINT32 a)
{
	switch (a) {
		case 0x500000: {
			return TaitoDip[1];
		}

		case 0x500002: {
			return TaitoInput[0];
		}

		case 0x580000: {
			return TaitoDip[0];
		}

		case 0x580002: {
			return TaitoInput[1];
		}

		case 0x580004: {
			return TaitoInput[2];
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Quizhq68KWriteWord(UINT32 a, UINT16 d)
{
	TC0100SCN0WordWrite_Map(0x800000, 0x80ffff)
	TC0100SCN0CtrlWordWrite_Map(0x820000)

	if (a >= 0x810000 && a <= 0x81ffff) return; // NOP

	switch (a) {
		case 0x200000:
		case 0x200002: {
			TC0110PCRWordWrite(0, (a - 0x200000) >> 1, d);
			return;
		}

		case 0x680000: {
			//nop
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Qzchikyu68KReadByte(UINT32 a)
{
	TC0510NIOHalfWordRead_Map(0x200000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Qzchikyu68KWriteByte(UINT32 a, UINT8 d)
{
	TC0510NIOHalfWordWrite_Map(0x200000)
	TC0100SCN0ByteWrite_Map(0x700000, 0x70ffff)

	switch (a) {
		case 0x300001: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x300003: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Qzchikyu68KReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Qzchikyu68KWriteWord(UINT32 a, UINT16 d)
{
	TC0510NIOHalfWordWrite_Map(0x200000)
	TC0100SCN0WordWrite_Map(0x700000, 0x70ffff)
	TC0100SCN0CtrlWordWrite_Map(0x720000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Qzquest68KReadByte(UINT32 a)
{
	TC0510NIOHalfWordRead_Map(0x200000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Qzquest68KWriteByte(UINT32 a, UINT8 d)
{
	TC0510NIOHalfWordWrite_Map(0x200000)
	TC0100SCN0ByteWrite_Map(0x700000, 0x70ffff)

	switch (a) {
		case 0x300001: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x300003: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Qzquest68KReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Qzquest68KWriteWord(UINT32 a, UINT16 d)
{
	TC0510NIOHalfWordWrite_Map(0x200000)
	TC0100SCN0WordWrite_Map(0x700000, 0x70ffff)
	TC0100SCN0CtrlWordWrite_Map(0x720000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Solfigtr68KReadByte(UINT32 a)
{
	switch (a) {
		case 0x300001: {
			return TaitoDip[0];
		}

		case 0x300003: {
			return TaitoDip[1];
		}

		case 0x320001: {
			return TaitoInput[0];
		}

		case 0x320003: {
			return TaitoInput[1];
		}

		case 0x320005: {
			return TaitoInput[2];
		}

		case 0x400002: {
			return TC0140SYTCommRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Solfigtr68KWriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0x800000, 0x80ffff)

	switch (a) {
		case 0x300005: {
			//coin write
			return;
		}

		case 0x400000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x400002: {
			TC0140SYTCommWrite(d);
			return;
		}

		case 0x504000: {
			//nop
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Solfigtr68KReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Solfigtr68KWriteWord(UINT32 a, UINT16 d)
{
	TC0100SCN0WordWrite_Map(0x800000, 0x80ffff)
	TC0100SCN0CtrlWordWrite_Map(0x820000)
	TC0360PRIHalfWordWrite_Map(0xb00000)

	switch (a) {
		case 0x300006: {
			//???
			return;
		}

		case 0x320006: {
			//???
			return;
		}

		case 0x340000: {
			//watchdog
			return;
		}

		case 0x500000:
		case 0x500002:
		case 0x500004:
		case 0x500006:
		case 0x500008:
		case 0x50000a:
		case 0x50000c:
		case 0x50000e: {
			TaitoF2SpriteBankWrite((a - 0x500000) >> 1, d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Ssi68KReadByte(UINT32 a)
{
	TC0510NIOHalfWordRead_Map(0x100000)

	switch (a) {
		case 0x400002: {
			return TC0140SYTCommRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Ssi68KWriteByte(UINT32 a, UINT8 d)
{
	TC0510NIOHalfWordWrite_Map(0x100000)
	TC0100SCN0ByteWrite_Map(0x600000, 0x60ffff)

	switch (a) {
		case 0x400000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x400002: {
			TC0140SYTCommWrite(d);
			return;
		}

		case 0x500000: {
			// nop
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Ssi68KReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Ssi68KWriteWord(UINT32 a, UINT16 d)
{
	TC0510NIOHalfWordWrite_Map(0x100000)
	TC0100SCN0ByteWrite_Map(0x600000, 0x60ffff)
	TC0100SCN0CtrlWordWrite_Map(0x620000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Thundfox68KReadByte(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x200000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Thundfox68KWriteByte(UINT32 a, UINT8 d)
{
	TC0220IOCHalfWordWrite_Map(0x200000)
	TC0100SCN0ByteWrite_Map(0x400000, 0x40ffff)
	TC0360PRIHalfWordWrite_Map(0x800000)

	switch (a) {
		case 0x220000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x220002: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Thundfox68KReadWord(UINT32 a)
{
	TC0220IOCHalfWordRead_Map(0x200000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Thundfox68KWriteWord(UINT32 a, UINT16 d)
{
	TC0220IOCHalfWordWrite_Map(0x200000)
	TC0100SCN0WordWrite_Map(0x400000, 0x40ffff)
	TC0100SCN0CtrlWordWrite_Map(0x420000)
	TC0100SCN1WordWrite_Map(0x500000, 0x50ffff)
	TC0100SCN1CtrlWordWrite_Map(0x520000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Yesnoj68KReadByte(UINT32 a)
{
	if (a >= 0x700000 && a <= 0x70001f) return 0;

	switch (a) {
		case 0x800002: {
			return TC0140SYTCommRead();
		}

		case 0xa00001: {
			return TaitoInput[0];
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Yesnoj68KWriteByte(UINT32 a, UINT8 d)
{
	TC0100SCN0ByteWrite_Map(0x500000, 0x50ffff)

	switch (a) {
		case 0x700015:
		case 0x70001b:
		case 0x70001d:
		case 0x70001f: {
			//???
			return;
		}

		case 0x800000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x800002: {
			TC0140SYTCommWrite(d);
			return;
		}

		case 0x900003: {
			//nop
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Yesnoj68KReadWord(UINT32 a)
{
	switch (a) {
		case 0xa00004: {
			return TaitoInput[1];
		}

		case 0xb00000: {
			YesnoDip = 1 - YesnoDip;

			if (YesnoDip) {
				return TaitoDip[0];
			} else {
				return TaitoDip[1];
			}
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Yesnoj68KWriteWord(UINT32 a, UINT16 d)
{
	TC0100SCN0WordWrite_Map(0x500000, 0x50ffff)
	TC0100SCN0CtrlWordWrite_Map(0x520000)

	switch (a) {
		case 0x900002:
		case 0x900006:
		case 0xa00006:
		case 0xc00000:
		case 0xd00000: {
			//nop
			return;
		}
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static UINT8 __fastcall Yuyugogo68KReadByte(UINT32 a)
{
	TC0510NIOHalfWordRead_Map(0x200000)

	switch (a) {
		case 0x400002: {
			return TC0140SYTCommRead();
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read byte => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Yuyugogo68KWriteByte(UINT32 a, UINT8 d)
{
	TC0510NIOHalfWordWrite_Map(0x200000)
	TC0100SCN0ByteWrite_Map(0x800000, 0x80ffff)

	switch (a) {
		case 0x400000: {
			TC0140SYTPortWrite(d);
			return;
		}

		case 0x400002: {
			TC0140SYTCommWrite(d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write byte => %06X, %02X\n"), a, d);
		}
	}
}

static UINT16 __fastcall Yuyugogo68KReadWord(UINT32 a)
{
	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Read word => %06X\n"), a);
		}
	}

	return 0;
}

static void __fastcall Yuyugogo68KWriteWord(UINT32 a, UINT16 d)
{
	TC0510NIOHalfWordWrite_Map(0x200000)
	TC0100SCN0WordWrite_Map(0x800000, 0x80ffff)
	TC0100SCN0CtrlWordWrite_Map(0x820000)

	switch (a) {
		default: {
			bprintf(PRINT_NORMAL, _T("68K #1 Write word => %06X, %04X\n"), a, d);
		}
	}
}

static void z80_bank(INT32 bank)
{
	TaitoZ80Bank = bank;

	ZetMapMemory(TaitoZ80Rom1 + 0x4000 + (TaitoZ80Bank * 0x4000), 0x4000, 0x7fff, MAP_ROM);
}

static UINT8 __fastcall TaitoF2Z80Read(UINT16 a)
{
	switch (a) {
		case 0xe000: {
			return BurnYM2610Read(0);
		}

		case 0xe002: {
			return BurnYM2610Read(2);
		}

		case 0xe200: {
			// NOP
			return 0;
		}

		case 0xe201: {
			return TC0140SYTSlaveCommRead();
		}

		case 0xea00: {
			// NOP
			return 0;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

static void __fastcall TaitoF2Z80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0xe000: {
			BurnYM2610Write(0, d);
			return;
		}

		case 0xe001: {
			BurnYM2610Write(1, d);
			return;
		}

		case 0xe002: {
			BurnYM2610Write(2, d);
			return;
		}

		case 0xe003: {
			BurnYM2610Write(3, d);
			return;
		}

		case 0xe200: {
			TC0140SYTSlavePortWrite(d);
			return;
		}

		case 0xe201: {
			TC0140SYTSlaveCommWrite(d);
			return;
		}

		case 0xe400:
		case 0xe401:
		case 0xe402:
		case 0xe403: {
			return;
		}

		case 0xe600: {
			return;
		}

		case 0xee00: {
			return;
		}

		case 0xf000: {
			return;
		}

		case 0xf200: {
			z80_bank((d - 1) & 7);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

static UINT8 __fastcall CamltryaZ80Read(UINT16 a)
{
	switch (a) {
		case 0x9000: {
			return BurnYM2203Read(0, 0);
		}

		case 0xa001: {
			return TC0140SYTSlaveCommRead();
		}

		case 0xb000: {
			return MSM6295Read(0);
		}

		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

static void __fastcall CamltryaZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0x9000: {
			BurnYM2203Write(0, 0, d);
			return;
		}

		case 0x9001: {
			BurnYM2203Write(0, 1, d);
			return;
		}

		case 0xa000: {
			TC0140SYTSlavePortWrite(d);
			return;
		}

		case 0xa001: {
			TC0140SYTSlaveCommWrite(d);
			return;
		}

		case 0xb000:
		case 0xb001: {
			MSM6295Write(0, d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

static UINT8 __fastcall DriveoutZ80Read(UINT16 a)
{
	switch (a) {
		case 0x9800: {
			return MSM6295Read(0);
		}

		case 0xa000: {
			return TaitoSoundLatch;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Read => %04X\n"), a);
		}
	}

	return 0;
}

static void DriveoutMSM6295Bank(INT32 bank)
{
	DriveoutOkiBank = bank;

	MSM6295SetBank(0, TaitoMSM6295Rom + ((DriveoutOkiBank & 3) * 0x20000), 0x00000, 0x1ffff);
	MSM6295SetBank(0, TaitoMSM6295Rom + 0x80000, 0x20000, 0x3ffff);
}

static void __fastcall DriveoutZ80Write(UINT16 a, UINT8 d)
{
	switch (a) {
		case 0x9000: {
			if (d & 4) {
				DriveoutMSM6295Bank(d);
			}
			return;
		}

		case 0x9800: {
			MSM6295Write(0, d);
			return;
		}

		default: {
			bprintf(PRINT_NORMAL, _T("Z80 Write => %04X, %02X\n"), a, d);
		}
	}
}

static INT32 CharPlaneOffsets[4]          = { 0, 1, 2, 3 };
static INT32 CharXOffsets[8]              = { 8, 12, 0, 4, 24, 28, 16, 20 };
static INT32 CharYOffsets[8]              = { 0, 32, 64, 96, 128, 160, 192, 224 };
static INT32 PivotPlaneOffsets[4]         = { 0, 1, 2, 3 };
static INT32 PivotXOffsets[8]             = { 0, 4, 8, 12, 16, 20, 24, 28 };
static INT32 PivotYOffsets[8]             = { 0, 32, 64, 96, 128, 160, 192, 224 };
static INT32 YuyugogoCharPlaneOffsets[1]  = { 0 };
static INT32 YuyugogoCharXOffsets[8]      = { 0, 1, 2, 3, 4, 5, 6, 7 };
static INT32 YuyugogoCharYOffsets[8]      = { 0, 8, 16, 24, 32, 40, 48, 56 };
static INT32 TC0480SCPCharPlaneOffsets[4] = { 0, 1, 2, 3 };
static INT32 TC0480SCPCharXOffsets[16]    = { 4, 0, 20, 16, 12, 8, 28, 24, 36, 32, 52, 48, 44, 40, 60, 56 };
static INT32 TC0480SCPCharYOffsets[16]    = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 };
static INT32 SpritePlaneOffsets[4]        = { 0, 1, 2, 3 };
static INT32 SpriteXOffsets[16]           = { 4, 0, 12, 8, 20, 16, 28, 24, 36, 32, 44, 40, 52, 48, 60, 56 };
static INT32 SpriteYOffsets[16]           = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 };
static INT32 FinalbSpritePlaneOffsets[6]  = { 0x800000, 0x800001, 0, 1, 2, 3 };
static INT32 FinalbSpriteXOffsets[16]     = { 12, 8, 4, 0, 28, 24, 20, 16, 44, 40, 36, 32, 60, 56, 52, 48 };
static INT32 FinalbSpriteYOffsets[16]     = { 0, 64, 128, 192, 256, 320, 384, 448, 512, 576, 640, 704, 768, 832, 896, 960 };

static void TaitoF2FMIRQHandler(INT32, INT32 nStatus)
{
	ZetSetIRQLine(0, (nStatus) ? CPU_IRQSTATUS_ACK : CPU_IRQSTATUS_NONE);
}

static void TaitoF2SoundInit()
{
	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(TaitoF2Z80Read);
	ZetSetWriteHandler(TaitoF2Z80Write);
	ZetMapArea(0x0000, 0x3fff, 0, TaitoZ80Rom1                 );
	ZetMapArea(0x0000, 0x3fff, 2, TaitoZ80Rom1                 );
	ZetMapArea(0x4000, 0x7fff, 0, TaitoZ80Rom1 + 0x4000        );
	ZetMapArea(0x4000, 0x7fff, 2, TaitoZ80Rom1 + 0x4000        );
	ZetMapArea(0xc000, 0xdfff, 0, TaitoZ80Ram1                 );
	ZetMapArea(0xc000, 0xdfff, 1, TaitoZ80Ram1                 );
	ZetMapArea(0xc000, 0xdfff, 2, TaitoZ80Ram1                 );
	ZetClose();

	BurnYM2610Init(24000000 / 3, TaitoYM2610ARom, (INT32*)&TaitoYM2610ARomSize, TaitoYM2610BRom, (INT32*)&TaitoYM2610BRomSize, &TaitoF2FMIRQHandler, 0);
	BurnTimerAttachZet(24000000 / 6);
	BurnYM2610SetRoute(BURN_SND_YM2610_YM2610_ROUTE_1, 1.00, BURN_SND_ROUTE_LEFT);
	BurnYM2610SetRoute(BURN_SND_YM2610_YM2610_ROUTE_2, 1.00, BURN_SND_ROUTE_RIGHT);
	BurnYM2610SetRoute(BURN_SND_YM2610_AY8910_ROUTE, 0.25, BURN_SND_ROUTE_BOTH);
}

#ifdef BUILD_A68K
static void SwitchToMusashi()
{
	if (bBurnUseASMCPUEmulation) {
#if 1 && defined FBNEO_DEBUG
		bprintf(PRINT_NORMAL, _T("Switching to Musashi 68000 core\n"));
#endif
		bUseAsm68KCoreOldValue = bBurnUseASMCPUEmulation;
		bBurnUseASMCPUEmulation = false;
	}
}
#endif

static void TaitoF2Init()
{
	GenericTilesInit();

	TaitoCharModulo = 0x100;
	TaitoCharNumPlanes = 4;
	TaitoCharWidth = 8;
	TaitoCharHeight = 8;
	TaitoCharPlaneOffsets = CharPlaneOffsets;
	TaitoCharXOffsets = CharXOffsets;
	TaitoCharYOffsets = CharYOffsets;
	TaitoNumChar = 0;

	TaitoSpriteAModulo = 0x400;
	TaitoSpriteANumPlanes = 4;
	TaitoSpriteAWidth = 16;
	TaitoSpriteAHeight = 16;
	TaitoSpriteAPlaneOffsets = SpritePlaneOffsets;
	TaitoSpriteAXOffsets = SpriteXOffsets;
	TaitoSpriteAYOffsets = SpriteYOffsets;
	TaitoNumSpriteA = 0;

	TaitoNum68Ks = 1;
	TaitoNumZ80s = 1;
	TaitoNumYM2610 = 1;

	TaitoF2SpritesDisabled = 1;
	TaitoF2SpritesActiveArea = 0;
	TaitoXOffset = 0;
	TaitoF2SpriteType = 0;

	TaitoF2SpriteBufferFunction = TaitoF2NoBuffer;

	PaletteType = TAITOF2Palette; // 0 taito, 1 metalb, 2 qzquest
	SpritePriWritebackMode = 1; // default to no priority writeback for sprite blender

	for (INT32 i = 0; i < 8; i++) {
		TaitoF2SpriteBankBuffered[i] = 0x400 * i;
		TaitoF2SpriteBank[i] = TaitoF2SpriteBankBuffered[i];
	}

	nTaitoCyclesTotal[0] = (24000000 / 2) / 60;
	nTaitoCyclesTotal[1] = (24000000 / 6) / 60;
}

static INT32 CameltryInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoCharPivotModulo = 0x100;
	TaitoCharPivotNumPlanes = 4;
	TaitoCharPivotWidth = 8;
	TaitoCharPivotHeight = 8;
	TaitoCharPivotPlaneOffsets = PivotPlaneOffsets;
	TaitoCharPivotXOffsets = PivotXOffsets;
	TaitoCharPivotYOffsets = PivotYOffsets;
	TaitoNumCharPivot = 0x1000;

	TaitoNumSpriteA = 0x1000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, TaitoPriorityMap);
	TC0140SYTInit(0);
	TC0220IOCInit();
	TC0280GRDInit(-16, -16, TaitoCharsPivot);
	TC0280GRDSetPriMap(TaitoPriorityMap);
	TC0360PRIInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x03ffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x100000, 0x10ffff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x200000, 0x201fff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x813fff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekMapMemory(TC0280GRDRam             , 0xa00000, 0xa01fff, MAP_RAM);
	SekSetReadByteHandler(0, Cameltry68KReadByte);
	SekSetWriteByteHandler(0, Cameltry68KWriteByte);
	SekSetReadWordHandler(0, Cameltry68KReadWord);
	SekSetWriteWordHandler(0, Cameltry68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;

	SpritePriWritebackMode = 0;
	bNoClearOpposites = 1;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 CamltryaInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumYM2610 = 0;
	TaitoNumYM2203 = 1;
	TaitoNumMSM6295 = 1;

	TaitoCharPivotModulo = 0x100;
	TaitoCharPivotNumPlanes = 4;
	TaitoCharPivotWidth = 8;
	TaitoCharPivotHeight = 8;
	TaitoCharPivotPlaneOffsets = PivotPlaneOffsets;
	TaitoCharPivotXOffsets = PivotXOffsets;
	TaitoCharPivotYOffsets = PivotYOffsets;
	TaitoNumCharPivot = 0x1000;

	TaitoNumSpriteA = 0x1000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, TaitoPriorityMap);
	TC0140SYTInit(0);
	TC0220IOCInit();
	TC0280GRDInit(-16, -16, TaitoCharsPivot);
	TC0280GRDSetPriMap(TaitoPriorityMap);
	TC0360PRIInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x03ffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x100000, 0x10ffff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x200000, 0x201fff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x813fff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekMapMemory(TC0280GRDRam             , 0xa00000, 0xa01fff, MAP_RAM);
	SekSetReadByteHandler(0, Cameltry68KReadByte);
	SekSetWriteByteHandler(0, Cameltry68KWriteByte);
	SekSetReadWordHandler(0, Cameltry68KReadWord);
	SekSetWriteWordHandler(0, Cameltry68KWriteWord);
	SekClose();

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(CamltryaZ80Read);
	ZetSetWriteHandler(CamltryaZ80Write);
	ZetMapArea(0x0000, 0x7fff, 0, TaitoZ80Rom1                 );
	ZetMapArea(0x0000, 0x7fff, 2, TaitoZ80Rom1                 );
	ZetMapArea(0x8000, 0x8fff, 0, TaitoZ80Ram1                 );
	ZetMapArea(0x8000, 0x8fff, 1, TaitoZ80Ram1                 );
	ZetMapArea(0x8000, 0x8fff, 2, TaitoZ80Ram1                 );
	ZetClose();

	BurnYM2203Init(1, 24000000 / 8, &TaitoF2FMIRQHandler, 0);
	BurnTimerAttachZet(24000000 / 4);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_YM2203_ROUTE, 0.60, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_1, 0.20, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_2, 0.20, BURN_SND_ROUTE_BOTH);
	BurnYM2203SetRoute(0, BURN_SND_YM2203_AY8910_ROUTE_3, 0.20, BURN_SND_ROUTE_BOTH);

	MSM6295Init(0, (4224000 / 4) / 132, 1);
	MSM6295SetRoute(0, 0.10, BURN_SND_ROUTE_BOTH);
	MSM6295SetBank(0, TaitoMSM6295Rom, 0x00000, 0x3ffff);

	nTaitoCyclesTotal[1] = (24000000 / 4) / 60;

	TaitoXOffset = 3;
	SpritePriWritebackMode = 0;
	bNoClearOpposites = 1;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 DeadconxInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoCharModulo = 0x400;
	TaitoCharNumPlanes = 4;
	TaitoCharWidth = 16;
	TaitoCharHeight = 16;
	TaitoCharPlaneOffsets = TC0480SCPCharPlaneOffsets;
	TaitoCharXOffsets = TC0480SCPCharXOffsets;
	TaitoCharYOffsets = TC0480SCPCharYOffsets;
	TaitoNumChar = 0x2000;

	TaitoSpriteAModulo = 0x400;
	TaitoSpriteANumPlanes = 4;
	TaitoSpriteAWidth = 16;
	TaitoSpriteAHeight = 16;
	TaitoSpriteAPlaneOffsets = SpritePlaneOffsets;
	TaitoSpriteAXOffsets = SpriteXOffsets;
	TaitoSpriteAYOffsets = SpriteYOffsets;
	TaitoNumSpriteA = 0x4000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "deadconxj")) {
		TC0480SCPInit(TaitoNumChar, 3, 0x34, -5, -1, 0, 26);
	} else {
		TC0480SCPInit(TaitoNumChar, 3, 0x1e, 8, -1, 0, 0);
	}
	TC0480SCPSetPriMap(TaitoPriorityMap);

	TC0140SYTInit(0);
	TC0360PRIInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x0fffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x100000, 0x10ffff, MAP_RAM);
	SekMapMemory(TaitoSpriteRam           , 0x200000, 0x20ffff, MAP_RAM);
	SekMapMemory(TC0480SCPRam             , 0x400000, 0x40ffff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x600000, 0x601fff, MAP_RAM);
	SekSetReadByteHandler(0, Deadconx68KReadByte);
	SekSetWriteByteHandler(0, Deadconx68KWriteByte);
	SekSetReadWordHandler(0, Deadconx68KReadWord);
	SekSetWriteWordHandler(0, Deadconx68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 DinorexInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumChar = 0x8000;
	TaitoNumSpriteA = 0xc000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, TaitoPriorityMap);
	TC0140SYTInit(0);
	TC0360PRIInit();
	TC0510NIOInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x2fffff, MAP_ROM);
	SekMapMemory(TaitoSpriteExtension     , 0x400000, 0x400fff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x500000, 0x501fff, MAP_RAM);
	SekMapMemory(Taito68KRam1             , 0x600000, 0x60ffff, MAP_RAM);
	SekMapMemory(TaitoSpriteRam           , 0x800000, 0x80ffff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x900000, 0x90ffff, MAP_READ);
	SekSetReadByteHandler(0, Dinorex68KReadByte);
	SekSetWriteByteHandler(0, Dinorex68KWriteByte);
	SekSetReadWordHandler(0, Dinorex68KReadWord);
	SekSetWriteWordHandler(0, Dinorex68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;
	TaitoF2SpriteType = 3;

	SpritePriWritebackMode = 0;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 DondokodInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoCharPivotModulo = 0x100;
	TaitoCharPivotNumPlanes = 4;
	TaitoCharPivotWidth = 8;
	TaitoCharPivotHeight = 8;
	TaitoCharPivotPlaneOffsets = PivotPlaneOffsets;
	TaitoCharPivotXOffsets = PivotXOffsets;
	TaitoCharPivotYOffsets = PivotYOffsets;
	TaitoNumCharPivot = 0x4000;

	TaitoNumChar = 0x4000;
	TaitoNumSpriteA = 0x1000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, TaitoPriorityMap);
	TC0140SYTInit(0);
	TC0220IOCInit();
	TC0280GRDInit(-16, -16, TaitoCharsPivot);
	TC0280GRDSetPriMap(TaitoPriorityMap);
	TC0360PRIInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x07ffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x100000, 0x10ffff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x200000, 0x201fff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x80ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekMapMemory(TC0280GRDRam             , 0xa00000, 0xa01fff, MAP_RAM);
	SekSetReadByteHandler(0, Dondokod68KReadByte);
	SekSetWriteByteHandler(0, Dondokod68KWriteByte);
	SekSetReadWordHandler(0, Dondokod68KReadWord);
	SekSetWriteWordHandler(0, Dondokod68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;
	TaitoF2SpriteBufferFunction = TaitoF2PartialBufferDelayed;
	PaletteType = TAITOF2Palette;

	SpritePriWritebackMode = 0;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 DriftoutInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoCharPivotModulo = 0x100;
	TaitoCharPivotNumPlanes = 4;
	TaitoCharPivotWidth = 8;
	TaitoCharPivotHeight = 8;
	TaitoCharPivotPlaneOffsets = PivotPlaneOffsets;
	TaitoCharPivotXOffsets = PivotXOffsets;
	TaitoCharPivotYOffsets = PivotYOffsets;
	TaitoNumCharPivot = 0x4000;
	TaitoNumSpriteA = 0x1000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, NULL);
	TC0140SYTInit(0);
	TC0360PRIInit();
	TC0430GRWInit(-16, 0, TaitoCharsPivot);
	TC0510NIOInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x0fffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x300000, 0x30ffff, MAP_RAM);
	SekMapMemory(TC0430GRWRam             , 0x400000, 0x401fff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x700000, 0x701fff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x80ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekSetReadByteHandler(0, Driftout68KReadByte);
	SekSetWriteByteHandler(0, Driftout68KWriteByte);
	SekSetReadWordHandler(0, Driftout68KReadWord);
	SekSetWriteWordHandler(0, Driftout68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;

	PaletteType = QZQUESTPalette;

	SpritePriWritebackMode = 0;

	Driftout = 1;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 DriveoutInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumYM2610 = 0;
	TaitoNumMSM6295 = 1;

	TaitoCharPivotModulo = 0x100;
	TaitoCharPivotNumPlanes = 4;
	TaitoCharPivotWidth = 8;
	TaitoCharPivotHeight = 8;
	TaitoCharPivotPlaneOffsets = PivotPlaneOffsets;
	TaitoCharPivotXOffsets = PivotXOffsets;
	TaitoCharPivotYOffsets = PivotYOffsets;
	TaitoNumCharPivot = 0x4000;
	TaitoNumSpriteA = 0x1000;

	TaitoLoadRoms(0);

	TaitoMSM6295RomSize = 0x100000;

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, NULL);
	TC0360PRIInit();
	TC0430GRWInit(-16, 0, TaitoCharsPivot);
	TC0510NIOInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x0fffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x300000, 0x30ffff, MAP_RAM);
	SekMapMemory(TC0430GRWRam             , 0x400000, 0x401fff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x700000, 0x701fff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x80ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekSetReadByteHandler(0, Driftout68KReadByte);
	SekSetWriteByteHandler(0, Driveout68KWriteByte);
	SekSetReadWordHandler(0, Driftout68KReadWord);
	SekSetWriteWordHandler(0, Driftout68KWriteWord);
	SekClose();

	// Setup the Z80 emulation
	ZetInit(0);
	ZetOpen(0);
	ZetSetReadHandler(DriveoutZ80Read);
	ZetSetWriteHandler(DriveoutZ80Write);
	ZetMapArea(0x0000, 0x7fff, 0, TaitoZ80Rom1                 );
	ZetMapArea(0x0000, 0x7fff, 2, TaitoZ80Rom1                 );
	ZetMapArea(0x8000, 0x87ff, 0, TaitoZ80Ram1                 );
	ZetMapArea(0x8000, 0x87ff, 1, TaitoZ80Ram1                 );
	ZetMapArea(0x8000, 0x87ff, 2, TaitoZ80Ram1                 );
	ZetClose();

	MSM6295Init(0, 1056000 / 132, 0);
	MSM6295SetRoute(0, 1.00, BURN_SND_ROUTE_BOTH);

	TaitoXOffset = 3;

	PaletteType = QZQUESTPalette;

	SpritePriWritebackMode = 0;

	Driftout = 1;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 FinalbInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumChar = 0x2000;

	TaitoSpriteAModulo = 0x400;
	TaitoSpriteANumPlanes = 6;
	TaitoSpriteAWidth = 16;
	TaitoSpriteAHeight = 16;
	TaitoSpriteAPlaneOffsets = FinalbSpritePlaneOffsets;
	TaitoSpriteAXOffsets = FinalbSpriteXOffsets;
	TaitoSpriteAYOffsets = FinalbSpriteYOffsets;
	TaitoNumSpriteA = 0x2000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	TaitoNumSpriteA = 0x0000;  // Need to load these outside of the usual routine

	if (TaitoLoadRoms(1)) return 1;

	TC0110PCRInit(1, 0x1000);
	TC0100SCNInit(0, TaitoNumChar, 1, 8, 0, NULL);
	TC0140SYTInit(0);
	TC0220IOCInit();

	// Load and decode Sprites
	INT32 nRet;
	TaitoNumSpriteA = 0x2000;
	UINT8 *TempRom = (UINT8*)BurnMalloc(0x200000);
	memset(TempRom, 0, 0x200000);
	nRet = BurnLoadRom(TempRom + 0x000000 ,  5, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(TempRom + 0x000001 ,  6, 2); if (nRet != 0) return 1;
	nRet = BurnLoadRom(TempRom + 0x180000 ,  7, 1); if (nRet != 0) return 1;

	INT32 Offset = 0x100000;
	UINT8 Data, d1, d2, d3, d4;
	for (INT32 i = 0x180000; i < 0x200000; i++) {
		Data = TempRom[i];
		d1 = (Data >> 0) & 3;
		d2 = (Data >> 2) & 3;
		d3 = (Data >> 4) & 3;
		d4 = (Data >> 6) & 3;

		TempRom[Offset] = (d3 << 2) | (d4 << 6);
		Offset++;

		TempRom[Offset] = (d1 << 2) | (d2 << 6);
		Offset++;
	}
	GfxDecode(TaitoNumSpriteA, TaitoSpriteANumPlanes, TaitoSpriteAWidth, TaitoSpriteAHeight, FinalbSpritePlaneOffsets, FinalbSpriteXOffsets, FinalbSpriteYOffsets, TaitoSpriteAModulo, TempRom, TaitoSpritesA);
	BurnFree(TempRom);

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x03ffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x100000, 0x10ffff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x80ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekSetReadByteHandler(0, Finalb68KReadByte);
	SekSetWriteByteHandler(0, Finalb68KWriteByte);
	SekSetReadWordHandler(0, Finalb68KReadWord);
	SekSetWriteWordHandler(0, Finalb68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 1;
	TaitoF2SpriteBufferFunction = TaitoF2PartialBufferDelayed;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 FootchmpInit()
{
	INT32 nLen;

	TaitoF2Init();

	Footchmp = 1;

	TaitoCharModulo = 0x400;
	TaitoCharNumPlanes = 4;
	TaitoCharWidth = 16;
	TaitoCharHeight = 16;
	TaitoCharPlaneOffsets = TC0480SCPCharPlaneOffsets;
	TaitoCharXOffsets = TC0480SCPCharXOffsets;
	TaitoCharYOffsets = TC0480SCPCharYOffsets;
	TaitoNumChar = 0x2000;

	TaitoSpriteAModulo = 0x400;
	TaitoSpriteANumPlanes = 4;
	TaitoSpriteAWidth = 16;
	TaitoSpriteAHeight = 16;
	TaitoSpriteAPlaneOffsets = SpritePlaneOffsets;
	TaitoSpriteAXOffsets = SpriteXOffsets;
	TaitoSpriteAYOffsets = SpriteYOffsets;
	TaitoNumSpriteA = 0x4000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (!strcmp(BurnDrvGetTextA(DRV_NAME), "htheroj")) {
		TC0480SCPInit(TaitoNumChar, 3, 0x33, -4, -1, 0, 24);
	} else {
		TC0480SCPInit(TaitoNumChar, 3, 0x1d, 8, -1, 0, 0);
	}
	TC0480SCPSetPriMap(TaitoPriorityMap);
	TC0140SYTInit(0);
	TC0360PRIInit();

	if (TaitoLoadRoms(1)) return 1;

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x07ffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x100000, 0x10ffff, MAP_RAM);
	SekMapMemory(TaitoSpriteRam           , 0x200000, 0x20ffff, MAP_RAM);
	SekMapMemory(TC0480SCPRam             , 0x400000, 0x40ffff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x600000, 0x601fff, MAP_RAM);
	SekSetReadByteHandler(0, Footchmp68KReadByte);
	SekSetWriteByteHandler(0, Footchmp68KWriteByte);
	SekSetReadWordHandler(0, Footchmp68KReadWord);
	SekSetWriteWordHandler(0, Footchmp68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;
	TaitoF2SpriteBufferFunction = TaitoF2FullBufferDelayed;
	SpritePriWritebackMode = 0;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 GrowlInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumChar = 0x8000;
	TaitoNumSpriteA = 0x4000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, TaitoPriorityMap);
	TC0140SYTInit(0);
	TC0360PRIInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x0fffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x100000, 0x10ffff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x200000, 0x201fff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x80ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekSetReadByteHandler(0, Growl68KReadByte);
	SekSetWriteByteHandler(0, Growl68KWriteByte);
	SekSetReadWordHandler(0, Growl68KReadWord);
	SekSetWriteWordHandler(0, Growl68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;

	SpritePriWritebackMode = 0;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 GunfrontInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumChar = 0x8000;
	TaitoNumSpriteA = 0x2000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, TaitoPriorityMap);
	TC0140SYTInit(0);
	TC0360PRIInit();
	TC0510NIOInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x0bffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x100000, 0x10ffff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x200000, 0x201fff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x80ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekSetReadByteHandler(0, Gunfront68KReadByte);
	SekSetWriteByteHandler(0, Gunfront68KWriteByte);
	SekSetReadWordHandler(0, Gunfront68KReadWord);
	SekSetWriteWordHandler(0, Gunfront68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoF2SpriteBufferFunction = TaitoF2PartialBufferDelayed;

	SpritePriWritebackMode = 0;

	TaitoXOffset = 3;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 KoshienInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumChar = 0x8000;
	TaitoNumSpriteA = 0x4000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 1, 8, 0, TaitoPriorityMap);
	TC0140SYTInit(0);
	TC0360PRIInit();
	TC0510NIOInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x03ffff, MAP_ROM);
	SekMapMemory(Taito68KRom1 + 0x40000   , 0x080000, 0x0fffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x100000, 0x10ffff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x200000, 0x201fff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x80ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekSetReadByteHandler(0, Koshien68KReadByte);
	SekSetWriteByteHandler(0, Koshien68KWriteByte);
	SekSetReadWordHandler(0, Koshien68KReadWord);
	SekSetWriteWordHandler(0, Koshien68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 1;

	SpritePriWritebackMode = 0;
	bNoClearOpposites = 1;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 LiquidkInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumChar = 0x4000;
	TaitoNumSpriteA = 0x2000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, TaitoPriorityMap);
	TC0100SCNLiquidKludge();
	TC0140SYTInit(0);
	TC0360PRIInit();
	TC0220IOCInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x07ffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x100000, 0x10ffff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x200000, 0x201fff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x80ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekSetReadByteHandler(0, Liquidk68KReadByte);
	SekSetWriteByteHandler(0, Liquidk68KWriteByte);
	SekSetReadWordHandler(0, Liquidk68KReadWord);
	SekSetWriteWordHandler(0, Liquidk68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;
	TaitoF2SpriteBufferFunction = TaitoF2PartialBufferDelayed;
	SpritePriWritebackMode = 0;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 MegablstInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumChar = 0x4000;
	TaitoNumSpriteA = 0x2000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, TaitoPriorityMap);
	TC0140SYTInit(0);
	TC0220IOCInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x07ffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x200000, 0x20ffff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x300000, 0x301fff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x600000, 0x60ffff, MAP_READ);
	SekMapMemory(Taito68KRam1 + 0x10000   , 0x610000, 0x61ffff, MAP_RAM);
	SekMapMemory(TaitoSpriteRam           , 0x800000, 0x80ffff, MAP_RAM);
	SekSetReadByteHandler(0, Megablst68KReadByte);
	SekSetWriteByteHandler(0, Megablst68KWriteByte);
	SekSetReadWordHandler(0, Megablst68KReadWord);
	SekSetWriteWordHandler(0, Megablst68KWriteWord);
	SekClose();

	TaitoF2SoundInit();
	TaitoF2SpriteBufferFunction = TaitoF2PartialBufferDelayed;

	cchip_init();

	TaitoXOffset = 3;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 MetalbInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoCharModulo = 0x400;
	TaitoCharNumPlanes = 4;
	TaitoCharWidth = 16;
	TaitoCharHeight = 16;
	TaitoCharPlaneOffsets = TC0480SCPCharPlaneOffsets;
	TaitoCharXOffsets = TC0480SCPCharXOffsets;
	TaitoCharYOffsets = TC0480SCPCharYOffsets;
	TaitoNumChar = 0x2000;

	TaitoNumSpriteA = 0x2000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0480SCPInit(TaitoNumChar, 3, 0x32, -4, 1, 0, 24);
	TC0480SCPSetColourBase(256);
	TC0480SCPSetPriMap(TaitoPriorityMap);
	TC0140SYTInit(0);
	TC0360PRIInit();
	TC0510NIOInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x0bffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x100000, 0x10ffff, MAP_RAM);
	SekMapMemory(TaitoSpriteRam           , 0x300000, 0x30ffff, MAP_RAM);
	SekMapMemory(TC0480SCPRam             , 0x500000, 0x50ffff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x700000, 0x703fff, MAP_RAM);
	SekSetReadByteHandler(0, Metalb68KReadByte);
	SekSetWriteByteHandler(0, Metalb68KWriteByte);
	SekSetReadWordHandler(0, Metalb68KReadWord);
	SekSetWriteWordHandler(0, Metalb68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;

	SpritePriWritebackMode = 0;
	PaletteType = METALBPalette;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 MjnquestInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumChar = 0x10000;
	TaitoNumSpriteA = 0x1000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	// Load and decode Sprites
	INT32 nRet;
	UINT8 *TempRom = (UINT8*)BurnMalloc(TaitoSpriteARomSize);
	memset(TempRom, 0, TaitoSpriteARomSize);
	nRet = BurnLoadRom(TempRom + 0x000000 ,  6, 1); if (nRet != 0) return 1;
	for (UINT32 i = 0; i < TaitoSpriteARomSize; i += 2) {
		INT32 Temp = TempRom[i];
		TempRom[i + 0] = (TempRom[i + 1] >> 4) | (TempRom[i + 1] << 4);
		TempRom[i + 1] = (Temp >> 4) | (Temp << 4);
	}
	GfxDecode(TaitoNumSpriteA, TaitoSpriteANumPlanes, TaitoSpriteAWidth, TaitoSpriteAHeight, TaitoSpriteAPlaneOffsets, TaitoSpriteAXOffsets, TaitoSpriteAYOffsets, TaitoSpriteAModulo, TempRom, TaitoSpritesA);
	BurnFree(TempRom);

	TC0110PCRInit(1, 0x1000);
	TC0100SCNInit(0, TaitoNumChar, 0, 8, 0, NULL);
	TC0100SCNSetGfxMask(0, 0x7fff);
	TC0140SYTInit(0);

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x03ffff, MAP_ROM);
	SekMapMemory(Taito68KRom1 + 0x40000   , 0x080000, 0x0fffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x110000, 0x12ffff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x400000, 0x40ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x500000, 0x50ffff, MAP_RAM);
	SekSetReadByteHandler(0, Mjnquest68KReadByte);
	SekSetWriteByteHandler(0, Mjnquest68KWriteByte);
	SekSetReadWordHandler(0, Mjnquest68KReadWord);
	SekSetWriteWordHandler(0, Mjnquest68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	bNoClearOpposites = 1;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 NinjakInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumChar = 0x4000;
	TaitoNumSpriteA = 0x4000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 0, 8, 0, TaitoPriorityMap);
	TC0140SYTInit(0);
	TC0360PRIInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x07ffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x100000, 0x10ffff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x200000, 0x201fff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x80ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekSetReadByteHandler(0, Ninjak68KReadByte);
	SekSetWriteByteHandler(0, Ninjak68KWriteByte);
	SekSetReadWordHandler(0, Ninjak68KReadWord);
	SekSetWriteWordHandler(0, Ninjak68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	// Reset the driver
	TaitoF2DoReset();
	SpritePriWritebackMode = 0;

	return 0;
}

static INT32 PulirulaInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoCharPivotModulo = 0x100;
	TaitoCharPivotNumPlanes = 4;
	TaitoCharPivotWidth = 8;
	TaitoCharPivotHeight = 8;
	TaitoCharPivotPlaneOffsets = PivotPlaneOffsets;
	TaitoCharPivotXOffsets = PivotXOffsets;
	TaitoCharPivotYOffsets = PivotYOffsets;
	TaitoNumCharPivot = 0x4000;

	TaitoNumChar = 0x8000;
	TaitoNumSpriteA = 0x4000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, TaitoPriorityMap);
	TC0140SYTInit(0);
	TC0360PRIInit();
	TC0430GRWInit(-10, 0, TaitoCharsPivot);
	TC0430GRDSetPriMap(TaitoPriorityMap);

	TC0510NIOInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x0bffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x300000, 0x30ffff, MAP_RAM);
	SekMapMemory(TC0430GRWRam             , 0x400000, 0x401fff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x700000, 0x701fff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x80ffff, MAP_READ);
	SekMapMemory(TaitoSpriteExtension     , 0x600000, 0x603fff, MAP_RAM);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekSetReadByteHandler(0, Pulirula68KReadByte);
	SekSetWriteByteHandler(0, Pulirula68KWriteByte);
	SekSetReadWordHandler(0, Pulirula68KReadWord);
	SekSetWriteWordHandler(0, Pulirula68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;
	TaitoF2SpriteType = 2;
	PaletteType = QZQUESTPalette;
	SpritePriWritebackMode = 0;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 QcrayonInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumChar = 0x8000;
	TaitoNumSpriteA = 0x4000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, TaitoPriorityMap);
	TC0140SYTInit(0);
	TC0360PRIInit();
	TC0510NIOInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x07ffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x100000, 0x10ffff, MAP_RAM);
	SekMapMemory(Taito68KRom1 + 0x80000   , 0x300000, 0x3fffff, MAP_ROM);
	SekMapMemory(TaitoSpriteExtension     , 0x600000, 0x603fff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x700000, 0x701fff, MAP_RAM);
	SekMapMemory(TaitoSpriteRam           , 0x800000, 0x80ffff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x900000, 0x90ffff, MAP_READ);
	SekSetReadByteHandler(0, Qcrayon68KReadByte);
	SekSetWriteByteHandler(0, Qcrayon68KWriteByte);
	SekSetReadWordHandler(0, Qcrayon68KReadWord);
	SekSetWriteWordHandler(0, Qcrayon68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;
	TaitoF2SpriteType = 3;
	SpritePriWritebackMode = 0;
	bNoClearOpposites = 1;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 Qcrayon2Init()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumChar = 0x8000;
	TaitoNumSpriteA = 0x4000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, TaitoPriorityMap);
	TC0140SYTInit(0);
	TC0360PRIInit();
	TC0510NIOInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x07ffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x200000, 0x20ffff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x300000, 0x301fff, MAP_RAM);
	SekMapMemory(TaitoSpriteRam           , 0x400000, 0x40ffff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x500000, 0x50ffff, MAP_READ);
	SekMapMemory(Taito68KRom1 + 0x80000   , 0x600000, 0x6fffff, MAP_ROM);
	SekMapMemory(TaitoSpriteExtension     , 0xb00000, 0xb017ff, MAP_RAM);
	SekSetReadByteHandler(0, Qcrayon268KReadByte);
	SekSetWriteByteHandler(0, Qcrayon268KWriteByte);
	SekSetReadWordHandler(0, Qcrayon268KReadWord);
	SekSetWriteWordHandler(0, Qcrayon268KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;
	TaitoF2SpriteType = 3;
	SpritePriWritebackMode = 0;
	bNoClearOpposites = 1;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 QjinseiInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumChar = 0x8000;
	TaitoNumSpriteA = 0x4000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, TaitoPriorityMap);
	TC0140SYTInit(0);
	TC0360PRIInit();
	TC0510NIOInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x07ffff, MAP_ROM);
	SekMapMemory(Taito68KRom1 + 0x80000   , 0x100000, 0x1fffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x300000, 0x30ffff, MAP_RAM);
	SekMapMemory(TaitoSpriteExtension     , 0x600000, 0x603fff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x700000, 0x701fff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x80ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekSetReadByteHandler(0, Qjinsei68KReadByte);
	SekSetWriteByteHandler(0, Qjinsei68KWriteByte);
	SekSetReadWordHandler(0, Qjinsei68KReadWord);
	SekSetWriteWordHandler(0, Qjinsei68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;
	TaitoF2SpriteType = 3;
	bNoClearOpposites = 1;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 QtorimonInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumSpriteA = 0x0800;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0110PCRInit(1, 0x1000);
	TC0100SCNInit(0, TaitoNumChar, 0, 8, 0, NULL);
	TC0140SYTInit(0);
	TC0220IOCInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x07ffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x100000, 0x10ffff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x80ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekSetReadByteHandler(0, Qtorimon68KReadByte);
	SekSetWriteByteHandler(0, Qtorimon68KWriteByte);
	SekSetReadWordHandler(0, Qtorimon68KReadWord);
	SekSetWriteWordHandler(0, Qtorimon68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoF2SpriteBufferFunction = TaitoF2PartialBufferDelayed;
	bNoClearOpposites = 1;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 QuizhqInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumSpriteA = 0x1000;

	TaitoLoadRoms(0);

	Taito68KRom1Size = 0xc0000;

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	memcpy(Taito68KRom1 + 0x80000, Taito68KRom1 + 0x40000, 0x40000);
	memset(Taito68KRom1 + 0x40000, 0, 0x40000);

	TC0110PCRInit(1, 0x1000);
	TC0100SCNInit(0, TaitoNumChar, 0, 8, 0, NULL);
	TC0140SYTInit(0);

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x0bffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x100000, 0x10ffff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x80ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekSetReadByteHandler(0, Quizhq68KReadByte);
	SekSetWriteByteHandler(0, Quizhq68KWriteByte);
	SekSetReadWordHandler(0, Quizhq68KReadWord);
	SekSetWriteWordHandler(0, Quizhq68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoF2SpriteBufferFunction = TaitoF2PartialBufferDelayed;
	bNoClearOpposites = 1;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 QzchikyuInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumChar = 0x8000;
	TaitoNumSpriteA = 0x2000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 0, 8, 0, NULL);
	TC0140SYTInit(0);
	TC0510NIOInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x03ffff, MAP_ROM);
	SekMapMemory(Taito68KRom1 + 0x40000   , 0x100000, 0x17ffff, MAP_ROM);
	SekMapMemory(TaitoPaletteRam          , 0x400000, 0x401fff, MAP_RAM);
	SekMapMemory(Taito68KRam1             , 0x500000, 0x50ffff, MAP_RAM);
	SekMapMemory(TaitoSpriteRam           , 0x600000, 0x60ffff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x700000, 0x70ffff, MAP_READ);
	SekSetReadByteHandler(0, Qzchikyu68KReadByte);
	SekSetWriteByteHandler(0, Qzchikyu68KWriteByte);
	SekSetReadWordHandler(0, Qzchikyu68KReadWord);
	SekSetWriteWordHandler(0, Qzchikyu68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoF2SpriteBufferFunction = TaitoF2PartialBufferDelayedQzchikyu;
	bNoClearOpposites = 1;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 QzquestInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumChar = 0x8000;
	TaitoNumSpriteA = 0x2000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 0, 8, 0, NULL);
	TC0140SYTInit(0);
	TC0510NIOInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x03ffff, MAP_ROM);
	SekMapMemory(Taito68KRom1 + 0x40000   , 0x100000, 0x1fffff, MAP_ROM);
	SekMapMemory(TaitoPaletteRam          , 0x400000, 0x401fff, MAP_RAM);
	SekMapMemory(Taito68KRam1             , 0x500000, 0x50ffff, MAP_RAM);
	SekMapMemory(TaitoSpriteRam           , 0x600000, 0x60ffff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x700000, 0x70ffff, MAP_READ);
	SekSetReadByteHandler(0, Qzquest68KReadByte);
	SekSetWriteByteHandler(0, Qzquest68KWriteByte);
	SekSetReadWordHandler(0, Qzquest68KReadWord);
	SekSetWriteWordHandler(0, Qzquest68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoF2SpriteBufferFunction = TaitoF2PartialBufferDelayed;
	bNoClearOpposites = 1;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 SolfigtrInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumChar = 0x8000;
	TaitoNumSpriteA = 0x4000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, TaitoPriorityMap);
	TC0140SYTInit(0);
	TC0360PRIInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x07ffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x100000, 0x10ffff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x200000, 0x201fff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x80ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekSetReadByteHandler(0, Solfigtr68KReadByte);
	SekSetWriteByteHandler(0, Solfigtr68KWriteByte);
	SekSetReadWordHandler(0, Solfigtr68KReadWord);
	SekSetWriteWordHandler(0, Solfigtr68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;

	SpritePriWritebackMode = 0;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 SsiInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumSpriteA = 0x2000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 0, 0, 0, NULL);
	TC0140SYTInit(0);
	TC0510NIOInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x07ffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x200000, 0x20ffff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0x300000, 0x301fff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x600000, 0x60ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x800000, 0x80ffff, MAP_RAM);
	SekSetReadByteHandler(0, Ssi68KReadByte);
	SekSetWriteByteHandler(0, Ssi68KWriteByte);
	SekSetReadWordHandler(0, Ssi68KReadWord);
	SekSetWriteWordHandler(0, Ssi68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;
	TaitoF2SpriteBufferFunction = TaitoF2PartialBufferDelayedThundfox;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 ThundfoxInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoNumChar = 0x4000;

	TaitoCharBModulo = 0x100;
	TaitoCharBNumPlanes = 4;
	TaitoCharBWidth = 8;
	TaitoCharBHeight = 8;
	TaitoCharBPlaneOffsets = CharPlaneOffsets;
	TaitoCharBXOffsets = CharXOffsets;
	TaitoCharBYOffsets = CharYOffsets;
	TaitoNumCharB = 0x4000;

	TaitoNumSpriteA = 0x2000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, NULL);
	TC0100SCNSetClipArea(0, nScreenWidth, nScreenHeight, 0);
	TC0100SCNInit(1, TaitoNumCharB, 3, 16, 0, NULL);
	TC0100SCNSetClipArea(1, nScreenWidth, nScreenHeight, 0);
	TC0140SYTInit(0);
	TC0220IOCInit();
	TC0360PRIInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x07ffff, MAP_ROM);
	SekMapMemory(TaitoPaletteRam          , 0x100000, 0x101fff, MAP_RAM);
	SekMapMemory(Taito68KRam1             , 0x300000, 0x30ffff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x400000, 0x40ffff, MAP_READ);
	SekMapMemory(TC0100SCNRam[1]          , 0x500000, 0x50ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x600000, 0x60ffff, MAP_RAM);
	SekSetReadByteHandler(0, Thundfox68KReadByte);
	SekSetWriteByteHandler(0, Thundfox68KWriteByte);
	SekSetReadWordHandler(0, Thundfox68KReadWord);
	SekSetWriteWordHandler(0, Thundfox68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;
	TaitoF2SpriteBufferFunction = TaitoF2PartialBufferDelayedThundfox;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 YesnojInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoCharModulo = 0x40;
	TaitoCharNumPlanes = 1;
	TaitoCharWidth = 8;
	TaitoCharHeight = 8;
	TaitoCharPlaneOffsets = YuyugogoCharPlaneOffsets;
	TaitoCharXOffsets = YuyugogoCharXOffsets;
	TaitoCharYOffsets = YuyugogoCharYOffsets;
	TaitoNumChar = 0x10000;

	TaitoNumSpriteA = 0x2000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, NULL);
	TC0140SYTInit(0);

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x07ffff, MAP_ROM);
	SekMapMemory(Taito68KRam1             , 0x200000, 0x20ffff, MAP_RAM);
	SekMapMemory(TaitoSpriteRam           , 0x400000, 0x40ffff, MAP_RAM);
	SekMapMemory(TC0100SCNRam[0]          , 0x500000, 0x50ffff, MAP_READ);
	SekMapMemory(TaitoPaletteRam          , 0x600000, 0x601fff, MAP_RAM);
	SekSetReadByteHandler(0, Yesnoj68KReadByte);
	SekSetWriteByteHandler(0, Yesnoj68KWriteByte);
	SekSetReadWordHandler(0, Yesnoj68KReadWord);
	SekSetWriteWordHandler(0, Yesnoj68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoXOffset = 3;
	bNoClearOpposites = 1;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 YuyugogoInit()
{
	INT32 nLen;

	TaitoF2Init();

	TaitoCharModulo = 0x40;
	TaitoCharNumPlanes = 1;
	TaitoCharWidth = 8;
	TaitoCharHeight = 8;
	TaitoCharPlaneOffsets = YuyugogoCharPlaneOffsets;
	TaitoCharXOffsets = YuyugogoCharXOffsets;
	TaitoCharYOffsets = YuyugogoCharYOffsets;
	TaitoNumChar = 0x4000;

	TaitoNumSpriteA = 0x4000;

	TaitoLoadRoms(0);

	// Allocate and Blank all required memory
	TaitoMem = NULL;
	MemIndex();
	nLen = TaitoMemEnd - (UINT8 *)0;
	if ((TaitoMem = (UINT8 *)BurnMalloc(nLen)) == NULL) return 1;
	memset(TaitoMem, 0, nLen);
	MemIndex();

	if (TaitoLoadRoms(1)) return 1;

	TC0100SCNInit(0, TaitoNumChar, 3, 8, 0, NULL);
	TC0140SYTInit(0);
	TC0510NIOInit();

#ifdef BUILD_A68K
	SwitchToMusashi();
#endif

	// Setup the 68000 emulation
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(Taito68KRom1             , 0x000000, 0x03ffff, MAP_ROM);
	SekMapMemory(TC0100SCNRam[0]          , 0x800000, 0x80ffff, MAP_READ);
	SekMapMemory(TaitoSpriteRam           , 0x900000, 0x90ffff, MAP_RAM);
	SekMapMemory(TaitoPaletteRam          , 0xa00000, 0xa01fff, MAP_RAM);
	SekMapMemory(Taito68KRam1             , 0xb00000, 0xb10fff, MAP_RAM);
	SekMapMemory(TaitoSpriteExtension     , 0xc00000, 0xc01fff, MAP_RAM);
	SekMapMemory(Taito68KRom1 + 0x40000   , 0xd00000, 0xdfffff, MAP_ROM);
	SekSetReadByteHandler(0, Yuyugogo68KReadByte);
	SekSetWriteByteHandler(0, Yuyugogo68KWriteByte);
	SekSetReadWordHandler(0, Yuyugogo68KReadWord);
	SekSetWriteWordHandler(0, Yuyugogo68KWriteWord);
	SekClose();

	TaitoF2SoundInit();

	TaitoF2SpriteType = 1;
	TaitoXOffset = 3;
	bNoClearOpposites = 1;

	// Reset the driver
	TaitoF2DoReset();

	return 0;
}

static INT32 TaitoF2Exit()
{
	TaitoExit();

	TaitoF2SpritesFlipScreen = 0;
	TaitoF2SpriteBlendMode = 0;
	TaitoF2TilePriority[0] = TaitoF2TilePriority[1] = TaitoF2TilePriority[2] = TaitoF2TilePriority[3] = TaitoF2TilePriority[4] = 0;
	TaitoF2SpritePriority[0] = TaitoF2SpritePriority[1] = TaitoF2SpritePriority[2] = TaitoF2SpritePriority[3] = 0;

	TaitoF2SpriteType = 0;
	Footchmp = 0;
	YesnoDip = 0;
	MjnquestInput = 0;
	DriveoutSoundNibble = 0;
	DriveoutOkiBank = 0;

	bNoClearOpposites = 0;

	Driftout = 0;

	TaitoF2SpriteBufferFunction = NULL;

	PaletteType = 0;
	SpritePriWritebackMode = 0;

#ifdef BUILD_A68K
	// Switch back CPU core if needed
	if (bUseAsm68KCoreOldValue) {
#if 1 && defined FBNEO_DEBUG
		bprintf(PRINT_NORMAL, _T("Switching back to A68K core\n"));
#endif
		bUseAsm68KCoreOldValue = false;
		bBurnUseASMCPUEmulation = true;
	}
#endif

	return 0;
}

static inline UINT8 pal4bit(UINT8 bits)
{
	bits &= 0x0f;
	return (bits << 4) | bits;
}

static inline UINT8 pal5bit(UINT8 bits)
{
	bits &= 0x1f;
	return (bits << 3) | (bits >> 2);
}


inline static INT32 CalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = pal4bit(BURN_ENDIAN_SWAP_INT16(nColour) >> 12);
	g = pal4bit(BURN_ENDIAN_SWAP_INT16(nColour) >>  8);
	b = pal4bit(BURN_ENDIAN_SWAP_INT16(nColour) >>  4);

	return BurnHighCol(r, g, b, 0);
}

inline static INT32 QzquestCalcCol(UINT16 nColour)
{
	INT32 r, g, b;

	r = pal5bit(BURN_ENDIAN_SWAP_INT16(nColour) >> 10);
	g = pal5bit(BURN_ENDIAN_SWAP_INT16(nColour) >>  5);
	b = pal5bit(BURN_ENDIAN_SWAP_INT16(nColour) >>  0);

	return BurnHighCol(r, g, b, 0);
}

static void TaitoF2CalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)TaitoPaletteRam, pd = TaitoPalette; i < 0x1000; i++, ps++, pd++) {
		*pd = CalcCol(*ps);
	}
}

static void MetalbCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)TaitoPaletteRam, pd = TaitoPalette; i < 0x2000; i++, ps++, pd++) {
		*pd = CalcCol(*ps);
	}
}

static void QzquestCalcPalette()
{
	INT32 i;
	UINT16* ps;
	UINT32* pd;

	for (i = 0, ps = (UINT16*)TaitoPaletteRam, pd = TaitoPalette; i < 0x1000; i++, ps++, pd++) {
		*pd = QzquestCalcCol(*ps);
	}
}

static void DynCalcPalette()
{
	switch (PaletteType) { // 0 taito, 1 metalb, 2 qzquest
		case 0: TaitoF2CalcPalette(); break;
		case 1: MetalbCalcPalette(); break;
		case 2: QzquestCalcPalette(); break;
	}
}

static void UpdateTaitoF2SpriteBanks()
{
	INT32 i;

	for (i = 0; i < 8; i++) {
		TaitoF2SpriteBank[i] = TaitoF2SpriteBankBuffered[i];
	}
}

void TaitoF2HandleSpriteBuffering()
{
	if (TaitoF2PrepareSprites) {
		memcpy(TaitoSpriteRamBuffered, TaitoSpriteRam, 0x10000);
		TaitoF2PrepareSprites = 0;
	}
}

static void TaitoF2UpdateSpritesActiveArea()
{
	INT32 Off;
	UINT16 *SpriteRamBuffered = (UINT16*)TaitoSpriteRamBuffered;

	UpdateTaitoF2SpriteBanks();

	TaitoF2HandleSpriteBuffering();

	if (TaitoF2SpritesActiveArea == 0x8000 && SpriteRamBuffered[(0x8000 + 6) / 2] == 0 && SpriteRamBuffered[(0x8000 + 10) / 2] == 0) TaitoF2SpritesActiveArea = 0;

	for (Off = 0; Off < 0x4000; Off += 16) {
		INT32 Offs = Off + TaitoF2SpritesActiveArea;

		if (BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 6) / 2]) & 0x8000) {
			TaitoF2SpritesDisabled = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 10) / 2]) & 0x1000;
			if (Footchmp) {
				TaitoF2SpritesActiveArea = 0x8000 * (BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 6) / 2]) & 0x0001);
			} else {
				TaitoF2SpritesActiveArea = 0x8000 * (BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 10) / 2]) & 0x0001);
			}
		}

		if ((BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 4) / 2]) & 0xf000) == 0xa000) {
			TaitoF2SpritesMasterScrollX = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 4) / 2]) & 0xfff;
			if (TaitoF2SpritesMasterScrollX >= 0x800) TaitoF2SpritesMasterScrollX -= 0x1000;

			TaitoF2SpritesMasterScrollY = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 6) / 2]) & 0xfff;
			if (TaitoF2SpritesMasterScrollY >= 0x800) TaitoF2SpritesMasterScrollY -= 0x1000;
		}
	}
}

static void RenderSpriteZoom(INT32 Code, INT32 sx, INT32 sy, INT32 Colour, INT32 xFlip, INT32 yFlip, INT32 xScale, INT32 yScale, INT32 Priority, UINT8* pSource)
{
	UINT8 *SourceBase = pSource + ((Code % TaitoNumSpriteA) * TaitoSpriteAWidth * TaitoSpriteAHeight);

	INT32 SpriteScreenHeight = (yScale * TaitoSpriteAHeight + 0x8000) >> 16;
	INT32 SpriteScreenWidth = (xScale * TaitoSpriteAWidth + 0x8000) >> 16;

	Colour = 0x10 * (Colour % 0x100);

	if (TaitoF2SpritesFlipScreen) {
		xFlip = !xFlip;
		sx = 320 - sx - (xScale >> 12);
		yFlip = !yFlip;
		sy = 256 - sy - (yScale >> 12);
	}

	if (SpriteScreenWidth && SpriteScreenHeight) {
		INT32 dx = (TaitoSpriteAWidth << 16) / SpriteScreenWidth;
		INT32 dy = (TaitoSpriteAHeight << 16) / SpriteScreenHeight;

		INT32 ex = sx + SpriteScreenWidth;
		INT32 ey = sy + SpriteScreenHeight;

		INT32 xIndexBase;
		INT32 yIndex;

		if (xFlip) {
			xIndexBase = (SpriteScreenWidth - 1) * dx;
			dx = -dx;
		} else {
			xIndexBase = 0;
		}

		if (yFlip) {
			yIndex = (SpriteScreenHeight - 1) * dy;
			dy = -dy;
		} else {
			yIndex = 0;
		}

		if (sx < 0) {
			INT32 Pixels = 0 - sx;
			sx += Pixels;
			xIndexBase += Pixels * dx;
		}

		if (sy < 0) {
			INT32 Pixels = 0 - sy;
			sy += Pixels;
			yIndex += Pixels * dy;
		}

		if (ex > nScreenWidth) {
			INT32 Pixels = ex - nScreenWidth;
			ex -= Pixels;
		}

		if (ey > nScreenHeight) {
			INT32 Pixels = ey - nScreenHeight;
			ey -= Pixels;
		}

		if (ex > sx) {
			INT32 y;

			for (y = sy; y < ey; y++) {
				UINT8 *Source = SourceBase + ((yIndex >> 16) * TaitoSpriteAWidth);
				UINT16* pPixel = pTransDraw + (y * nScreenWidth);

				INT32 x, xIndex = xIndexBase;
				for (x = sx; x < ex; x++) {
					INT32 c = Source[xIndex >> 16];
					UINT8 *pri = &TaitoPriorityMap[(y * nScreenWidth) + x];
					if (c && (*pri & 0x80) == 0) {
						if (TaitoIC_TC0360PRIInUse) {
							INT32 Pri = *pri;
							INT32 TilePri = 0;

							if (TaitoIC_TC0100SCNInUse || PaletteType == METALBPalette) {
								if (Pri & 0x01) TilePri = TaitoF2TilePriority[0];
								if (Pri & 0x02) TilePri = TaitoF2TilePriority[1];
								if (Pri & 0x04) TilePri = TaitoF2TilePriority[2];
								if (Pri & 0x08) TilePri = TaitoF2TilePriority[3];
								if (Pri & 0x10) TilePri = TaitoF2TilePriority[4];
							}

							if (((TaitoF2SpriteBlendMode & 0xc0) == 0xc0) && (Priority == (TilePri - 1)))
							{
								pPixel[x] = ((c | Colour) & 0xfff0) | (pPixel[x] & 0x0f);
							}
							else if (((TaitoF2SpriteBlendMode & 0xc0) == 0xc0) && (Priority == (TilePri + 1)))
							{
								if (pPixel[x] & 0x0f) {
									pPixel[x] = (pPixel[x] & 0xfff0) | ((c | Colour) & 0x0f);
								} else {
									pPixel[x] = c | Colour;
								}
							}
							else if (((TaitoF2SpriteBlendMode & 0xc0) == 0x80) && (Priority == (TilePri - 1)))
							{
								pPixel[x] = pPixel[x] & 0xffef;
							}
							else if (((TaitoF2SpriteBlendMode & 0xc0) == 0x80) && (Priority == (TilePri + 1)))
							{
								pPixel[x] = ((c | Colour) & 0xffef);
							}
							else
							{
								if (Priority > TilePri)
									pPixel[x] = c | Colour;
							}

							*pri |= ((SpritePriWritebackMode) ? 0x00 : 0x80);
						} else {
							pPixel[x] = c | Colour;
						}
					}
					xIndex += dx;
				}

				yIndex += dy;
			}
		}
	}
}

static void RenderSpriteZoomPriMask(INT32 Code, INT32 sx, INT32 sy, INT32 Colour, INT32 xFlip, INT32 yFlip, INT32 xScale, INT32 yScale, INT32 Priority, UINT8* pSource)
{
	UINT8 *SourceBase = pSource + ((Code % TaitoNumSpriteA) * TaitoSpriteAWidth * TaitoSpriteAHeight);

	INT32 SpriteScreenHeight = (yScale * TaitoSpriteAHeight + 0x8000) >> 16;
	INT32 SpriteScreenWidth = (xScale * TaitoSpriteAWidth + 0x8000) >> 16;

	Colour = 0x10 * (Colour % 0x100);
	Priority |= 1<<31;
	if (TaitoF2SpritesFlipScreen) {
		xFlip = !xFlip;
		sx = 320 - sx - (xScale >> 12);
		yFlip = !yFlip;
		sy = 256 - sy - (yScale >> 12);
	}

	if (SpriteScreenWidth && SpriteScreenHeight) {
		INT32 dx = (TaitoSpriteAWidth << 16) / SpriteScreenWidth;
		INT32 dy = (TaitoSpriteAHeight << 16) / SpriteScreenHeight;

		INT32 ex = sx + SpriteScreenWidth;
		INT32 ey = sy + SpriteScreenHeight;

		INT32 xIndexBase;
		INT32 yIndex;

		if (xFlip) {
			xIndexBase = (SpriteScreenWidth - 1) * dx;
			dx = -dx;
		} else {
			xIndexBase = 0;
		}

		if (yFlip) {
			yIndex = (SpriteScreenHeight - 1) * dy;
			dy = -dy;
		} else {
			yIndex = 0;
		}

		if (sx < 0) {
			INT32 Pixels = 0 - sx;
			sx += Pixels;
			xIndexBase += Pixels * dx;
		}

		if (sy < 0) {
			INT32 Pixels = 0 - sy;
			sy += Pixels;
			yIndex += Pixels * dy;
		}

		if (ex > nScreenWidth) {
			INT32 Pixels = ex - nScreenWidth;
			ex -= Pixels;
		}

		if (ey > nScreenHeight) {
			INT32 Pixels = ey - nScreenHeight;
			ey -= Pixels;
		}

		if (ex > sx) {
			INT32 y;

			for (y = sy; y < ey; y++) {
				UINT8 *Source = SourceBase + ((yIndex >> 16) * TaitoSpriteAWidth);
				UINT16* pPixel = pTransDraw + (y * nScreenWidth);
				UINT8 *pri = TaitoPriorityMap + (y * nScreenWidth);

				INT32 x, xIndex = xIndexBase;
				for (x = sx; x < ex; x++) {
					INT32 c = Source[xIndex >> 16];
					if (c) {
						if ((Priority & (1 << (pri[x]&0x1f))) == 0) {
							pPixel[x] = c | Colour;
						}
						pri[x] = 0x1f;
					}
					xIndex += dx;
				}

				yIndex += dy;
			}
		}
	}
}

void TaitoF2MakeSpriteList()
{
	INT32 i, x, y, Off, ExtOffs;
	INT32 Code, Colour, SpriteData, SpriteCont, xFlip, yFlip;
	INT32 xCurrent, yCurrent, BigSprite = 0;
	INT32 yNum = 0, xNum = 0, xLatch = 0, yLatch = 0, LastContinuationTile = 0;
	UINT32 ZoomWord, xZoom, yZoom, zx = 0, zy = 0, xZoomLatch = 0, yZoomLatch = 0;
	INT32 xScroll1, yScroll1;
	INT32 xScroll = 0, yScroll = 0;
	INT32 xCur, yCur;
	INT32 xOffset = 0;

	INT32 Disabled = TaitoF2SpritesDisabled;
	INT32 MasterScrollX = TaitoF2SpritesMasterScrollX;
	INT32 MasterScrollY = TaitoF2SpritesMasterScrollY;
	INT32 Area = TaitoF2SpritesActiveArea;

	UINT16 *SpriteRamBuffered = (UINT16*)TaitoSpriteRamBuffered;
	UINT16 *SpriteExtension = (UINT16*)TaitoSpriteExtension;
	struct TaitoF2SpriteEntry *SpritePtr = TaitoF2SpriteList;

	memset(TaitoF2SpriteList, 0, 0x400 * sizeof(TaitoF2SpriteEntry));

	xScroll1 = 0;
	yScroll1 = 0;
	x = y = 0;
	xCurrent = yCurrent = 0;
	Colour = 0;

	xOffset = TaitoXOffset;
	if (TaitoF2SpritesFlipScreen) xOffset = -TaitoXOffset;

	if (Area == 0x8000 && SpriteRamBuffered[(0x8000 + 6) / 2] == 0 && SpriteRamBuffered[(0x8000 + 10) / 2] == 0) Area = 0;

	for (Off = 0; Off < 0x4000; Off += 16) {
		INT32 Offs = Off + Area;

		if (BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 6) / 2]) & 0x8000) {
			Disabled = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 10) / 2]) & 0x1000;
			TaitoF2SpritesFlipScreen = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 10) / 2]) & 0x2000;

			xOffset = TaitoXOffset;
			if (TaitoF2SpritesFlipScreen) xOffset = -TaitoXOffset;

			if (Footchmp) {
				Area = 0x8000 * (BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 6) / 2]) & 0x0001);
			} else {
				Area = 0x8000 * (BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 10) / 2]) & 0x0001);
			}
		}

		if ((BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 4) / 2]) & 0xf000) == 0xa000) {
			MasterScrollX = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 4) / 2]) & 0xfff;
			if (MasterScrollX >= 0x800) MasterScrollX -= 0x1000;

			MasterScrollY = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 6) / 2]) & 0xfff;
			if (MasterScrollY >= 0x800) MasterScrollY -= 0x1000;
		}

		if ((BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 4) / 2]) & 0xf000) == 0x5000) {
			xScroll1 = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 4) / 2]) & 0xfff;
			if (xScroll1 >= 0x800) xScroll1 -= 0x1000;

			yScroll1 = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 6) / 2]) & 0xfff;
			if (yScroll1 >= 0x800) yScroll1 -= 0x1000;
		}

		if (Disabled)
			continue;

		SpriteData = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 8) / 2]);

		SpriteCont = (SpriteData & 0xff00) >> 8;

		if ((SpriteCont & 0x08) != 0) {
			if (BigSprite == 0) {
				xLatch = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 4) / 2]) & 0xfff;
				yLatch = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 6) / 2]) & 0xfff;
				xNum = 0;
				yNum = 0;
				ZoomWord = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 2) / 2]);
				yZoomLatch = (ZoomWord >> 8) & 0xff;
				xZoomLatch = (ZoomWord) & 0xff;
				BigSprite = 1;
			}
		} else if (BigSprite) {
			LastContinuationTile = 1;
		}

		if ((SpriteCont & 0x04) == 0) Colour = SpriteData & 0xff;

		if (BigSprite == 0 || (SpriteCont & 0xf0) == 0) {
			x = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 4) / 2]);

			if (x & 0x8000) {
				xScroll = -xOffset - 0x60;
				yScroll = 0;
			} else if (x & 0x4000) {
				xScroll = MasterScrollX - xOffset - 0x60;
				yScroll = MasterScrollY;
			} else {
				xScroll = xScroll1 + MasterScrollX - xOffset - 0x60;
				yScroll = yScroll1 + MasterScrollY;
			}

			x &= 0xfff;
			y = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 6) / 2]) & 0xfff;

			xCurrent = x;
			yCurrent = y;
		} else {
			if ((SpriteCont & 0x10) == 0) {
				y = yCurrent;
			} else if ((SpriteCont & 0x20) != 0) {
				y += 16;
				yNum++;
			}

			if ((SpriteCont & 0x40) == 0) {
				x = xCurrent;
			} else if ((SpriteCont & 0x80) != 0){
				x += 16;
				yNum=0;
				xNum++;
			}
		}

		if (BigSprite) {
			xZoom = xZoomLatch;
			yZoom = yZoomLatch;
			zx = 0x10;
			zy = 0x10;

			if (xZoom || yZoom) {
				x = xLatch + (xNum * (0x100 - xZoom) + 12) / 16;
				y = yLatch + (yNum * (0x100 - yZoom) + 12) / 16;
				zx = xLatch + ((xNum + 1) * (0x100 - xZoom) + 12) / 16 - x;
				zy = yLatch + ((yNum + 1) * (0x100 - yZoom) + 12) / 16 - y;
			}
		} else {
			ZoomWord = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[(Offs + 2) / 2]);
			yZoom = (ZoomWord >> 8) & 0xff;
			xZoom = (ZoomWord) & 0xff;
			zx = (0x100 - xZoom) / 16;
			zy = (0x100 - yZoom) / 16;
		}

		if (LastContinuationTile) {
			BigSprite = 0;
			LastContinuationTile = 0;
		}

		Code = 0;
		ExtOffs = Offs;
		if (ExtOffs >= 0x8000) ExtOffs -= 0x4000;

		if (TaitoF2SpriteType == 0) {
			Code = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[Offs / 2]) & 0x1fff;
			i = (Code & 0x1c00) >> 10;
			Code = TaitoF2SpriteBank[i] + (Code & 0x3ff);
		}

		if (TaitoF2SpriteType == 1) {
			Code = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[Offs / 2]) & 0x3ff;
			i = (BURN_ENDIAN_SWAP_INT16(SpriteExtension[(ExtOffs >> 4)]) & 0x3f) << 10;
			Code = (i | Code);
		}

		if (TaitoF2SpriteType == 2) {
			Code = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[Offs / 2]) & 0xff;
			i = (BURN_ENDIAN_SWAP_INT16(SpriteExtension[(ExtOffs >> 4)]) & 0xff00);
			Code = (i | Code);
		}

		if (TaitoF2SpriteType == 3) {
			Code = BURN_ENDIAN_SWAP_INT16(SpriteRamBuffered[Offs / 2]) & 0xff;
			i = (BURN_ENDIAN_SWAP_INT16(SpriteExtension[ExtOffs >> 4]) & 0xff) << 8;
			Code = (i | Code);
		}

		if (Code == 0) continue;

		xFlip = SpriteCont & 0x01;
		yFlip = SpriteCont & 0x02;

		xCur = (x + xScroll) & 0xfff;
		if (xCur >= 0x800) xCur -= 0x1000;

		yCur = (y + yScroll) & 0xfff;
		if (yCur >= 0x800) yCur -= 0x1000;

		if (TaitoF2SpritesFlipScreen) {
			yCur += 16;
		} else {
			yCur -= 16;
		}

		INT32 Priority = (Colour & 0xc0) >> 6;

		SpritePtr->Code = Code;
		SpritePtr->x = xCur;
		SpritePtr->y = yCur;
		SpritePtr->Colour = Colour;
		SpritePtr->xFlip = xFlip;
		SpritePtr->yFlip = yFlip;
		SpritePtr->xZoom = zx << 12;
		SpritePtr->yZoom = zy << 12;
		SpritePtr->Priority = TaitoF2SpritePriority[Priority];
		SpritePtr->Priority_Raw = Priority;
		SpritePtr++;
	}
}

void TaitoF2RenderSpriteList(INT32 TaitoF2SpritePriorityLevel)
{
	for (INT32 i = 0; i < 0x400; i++) {
		if (TaitoF2SpriteList[i].Priority == TaitoF2SpritePriorityLevel) RenderSpriteZoom(TaitoF2SpriteList[i].Code, TaitoF2SpriteList[i].x, TaitoF2SpriteList[i].y, TaitoF2SpriteList[i].Colour, TaitoF2SpriteList[i].xFlip, TaitoF2SpriteList[i].yFlip, TaitoF2SpriteList[i].xZoom, TaitoF2SpriteList[i].yZoom, TaitoF2SpritePriorityLevel, TaitoSpritesA);
	}
}

void TaitoF2RenderSpriteListBackwardsForPriority()
{
	for (INT32 i = 0x400-1; i > -1; i--) {
		RenderSpriteZoom(TaitoF2SpriteList[i].Code, TaitoF2SpriteList[i].x, TaitoF2SpriteList[i].y, TaitoF2SpriteList[i].Colour, TaitoF2SpriteList[i].xFlip, TaitoF2SpriteList[i].yFlip, TaitoF2SpriteList[i].xZoom, TaitoF2SpriteList[i].yZoom, TaitoF2SpriteList[i].Priority, TaitoSpritesA);
	}
}

void TaitoF2RenderSpriteListPriMasks(INT32 *primasks)
{
	for (INT32 i = 0x400-1; i > -1; i--) {
		RenderSpriteZoomPriMask(TaitoF2SpriteList[i].Code, TaitoF2SpriteList[i].x, TaitoF2SpriteList[i].y, TaitoF2SpriteList[i].Colour, TaitoF2SpriteList[i].xFlip, TaitoF2SpriteList[i].yFlip, TaitoF2SpriteList[i].xZoom, TaitoF2SpriteList[i].yZoom, primasks[TaitoF2SpriteList[i].Priority_Raw & 3], TaitoSpritesA);
	}
}

void TaitoF2NoBuffer()
{
	TaitoF2UpdateSpritesActiveArea();
	TaitoF2PrepareSprites = 1;
}

void TaitoF2PartialBufferDelayed()
{
	UINT16 *SpriteRam = (UINT16*)TaitoSpriteRam;
	UINT16 *SpriteRamBuffered = (UINT16*)TaitoSpriteRamBuffered;

	TaitoF2UpdateSpritesActiveArea();
	TaitoF2PrepareSprites = 0;
	memcpy(TaitoSpriteRamBuffered, TaitoSpriteRamDelayed, 0x10000);
	for (INT32 i = 0; i < 0x10000 / 2; i += 4) SpriteRamBuffered[i] = SpriteRam[i];
	memcpy(TaitoSpriteRamDelayed, TaitoSpriteRam, 0x10000);
}

void TaitoF2PartialBufferDelayedQzchikyu()
{
	UINT16 *SpriteRam = (UINT16*)TaitoSpriteRam;
	UINT16 *SpriteRamBuffered = (UINT16*)TaitoSpriteRamBuffered;

	TaitoF2UpdateSpritesActiveArea();
	TaitoF2PrepareSprites = 0;
	memcpy(TaitoSpriteRamBuffered, TaitoSpriteRamDelayed, 0x10000);
	for (INT32 i = 0; i < 0x10000 / 2; i += 8) {
		SpriteRamBuffered[i + 0] = SpriteRam[i + 0];
		SpriteRamBuffered[i + 1] = SpriteRam[i + 1];
		SpriteRamBuffered[i + 4] = SpriteRam[i + 4];
		SpriteRamBuffered[i + 5] = SpriteRam[i + 5];
		SpriteRamBuffered[i + 6] = SpriteRam[i + 6];
		SpriteRamBuffered[i + 7] = SpriteRam[i + 7];
	}
	memcpy(TaitoSpriteRamDelayed, TaitoSpriteRam, 0x10000);
}

void TaitoF2PartialBufferDelayedThundfox()
{
	UINT16 *SpriteRam = (UINT16*)TaitoSpriteRam;
	UINT16 *SpriteRamBuffered = (UINT16*)TaitoSpriteRamBuffered;

	TaitoF2UpdateSpritesActiveArea();
	TaitoF2PrepareSprites = 0;
	memcpy(TaitoSpriteRamBuffered, TaitoSpriteRamDelayed, 0x10000);
	for (INT32 i = 0; i < 0x10000 / 2; i += 8) {
		SpriteRamBuffered[i + 0] = SpriteRam[i + 0];
		SpriteRamBuffered[i + 1] = SpriteRam[i + 1];
		SpriteRamBuffered[i + 4] = SpriteRam[i + 4];
	}
	memcpy(TaitoSpriteRamDelayed, TaitoSpriteRam, 0x10000);
}

void TaitoF2FullBufferDelayed()
{
	UINT16 *SpriteRam = (UINT16*)TaitoSpriteRam;
	UINT16 *SpriteRamBuffered = (UINT16*)TaitoSpriteRamBuffered;

	TaitoF2UpdateSpritesActiveArea();
	TaitoF2PrepareSprites = 0;
	memcpy(TaitoSpriteRamBuffered, TaitoSpriteRamDelayed, 0x10000);
	for (INT32 i = 0;i < 0x10000 / 2; i++) SpriteRamBuffered[i] = SpriteRam[i];
	memcpy(TaitoSpriteRamDelayed, TaitoSpriteRam, 0x10000);
}

static INT32 TaitoF2Draw()
{
	INT32 i;
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;
	INT32 DrawLayer0 = 1;
	INT32 DrawLayer1 = 1;
	INT32 DrawLayer2 = 1;

	if (TC0100SCNBottomLayer(0)) {
		TaitoF2TilePriority[1] = TC0360PRIRegs[5] & 0x0f;
		TaitoF2TilePriority[0] = TC0360PRIRegs[5] >> 4;
	} else {
		TaitoF2TilePriority[0] = TC0360PRIRegs[5] & 0x0f;
		TaitoF2TilePriority[1] = TC0360PRIRegs[5] >> 4;
	}
	TaitoF2TilePriority[2] = TC0360PRIRegs[4] >> 4;

	if (TaitoF2TilePriority[1] < TaitoF2TilePriority[0]) TaitoF2TilePriority[1] = TaitoF2TilePriority[0];

	TaitoF2SpritePriority[0] = TC0360PRIRegs[6] & 0x0f;
	TaitoF2SpritePriority[1] = TC0360PRIRegs[6] >> 4;
	TaitoF2SpritePriority[2] = TC0360PRIRegs[7] & 0x0f;
	TaitoF2SpritePriority[3] = TC0360PRIRegs[7] >> 4;

	TaitoF2SpriteBlendMode = TC0360PRIRegs[0] & 0xc0;

	BurnTransferClear();
	//TaitoF2CalcPalette();
	DynCalcPalette();

	// Detect when we are using sprite blending with the sprite under the tile layer
	UINT8 TileAltPriority[3] = { 0xff, 0xff, 0xff };
	if (TaitoF2SpriteBlendMode) {
		for (i = 0; i < 4; i++) {
			if (TaitoF2SpritePriority[i] == TaitoF2TilePriority[0] - 1) {
				TileAltPriority[0] = TaitoF2SpritePriority[i];
				DrawLayer0 = 0;
			}

			if (TaitoF2SpritePriority[i] == TaitoF2TilePriority[1] - 1) {
				TileAltPriority[1] = TaitoF2SpritePriority[i];
				DrawLayer1 = 0;
			}

			if (TaitoF2SpritePriority[i] == TaitoF2TilePriority[2] - 1) {
				TileAltPriority[2] = TaitoF2SpritePriority[i];
				DrawLayer2 = 0;
			}
		}
	}

	TaitoF2MakeSpriteList();

	if (TC0100SCNBottomLayer(0)) {
		for (i = 0; i < 16; i++) {
			if (nBurnLayer & 1) if (TileAltPriority[0] == i) { if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars); }
			if (nBurnLayer & 2) if (TileAltPriority[1] == i) { if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars); }
			if (nBurnLayer & 4) if (TileAltPriority[2] == i) { if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0); }

			if (!TaitoIC_TC0360PRIInUse) {
				if (nSpriteEnable & 1) if (TaitoF2SpritePriority[3] == i) TaitoF2RenderSpriteList(TaitoF2SpritePriority[3]);
				if (nSpriteEnable & 2) if (TaitoF2SpritePriority[2] == i) TaitoF2RenderSpriteList(TaitoF2SpritePriority[2]);
				if (nSpriteEnable & 4) if (TaitoF2SpritePriority[1] == i) TaitoF2RenderSpriteList(TaitoF2SpritePriority[1]);
				if (nSpriteEnable & 8) if (TaitoF2SpritePriority[0] == i) TaitoF2RenderSpriteList(TaitoF2SpritePriority[0]);
			}

			if (nBurnLayer & 1) if (TaitoF2TilePriority[0] == i && DrawLayer0) { if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars); }
			if (nBurnLayer & 2) if (TaitoF2TilePriority[1] == i && DrawLayer1) { if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars); }
			if (nBurnLayer & 4) if (TaitoF2TilePriority[2] == i && DrawLayer2) { if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0); }
		}
	} else {
		for (i = 0; i < 16; i++) {
			if (nBurnLayer & 2) if (TileAltPriority[0] == i) { if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars); }
			if (nBurnLayer & 1) if (TileAltPriority[1] == i) { if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars); }
			if (nBurnLayer & 4) if (TileAltPriority[2] == i) { if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0); }

			if (!TaitoIC_TC0360PRIInUse) {
				if (nSpriteEnable & 1) if (TaitoF2SpritePriority[3] == i) TaitoF2RenderSpriteList(TaitoF2SpritePriority[3]);
				if (nSpriteEnable & 2) if (TaitoF2SpritePriority[2] == i) TaitoF2RenderSpriteList(TaitoF2SpritePriority[2]);
				if (nSpriteEnable & 4) if (TaitoF2SpritePriority[1] == i) TaitoF2RenderSpriteList(TaitoF2SpritePriority[1]);
				if (nSpriteEnable & 8) if (TaitoF2SpritePriority[0] == i) TaitoF2RenderSpriteList(TaitoF2SpritePriority[0]);
			}

			if (nBurnLayer & 2) if (TaitoF2TilePriority[0] == i && DrawLayer0) { if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars); }
			if (nBurnLayer & 1) if (TaitoF2TilePriority[1] == i && DrawLayer1) { if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars); }
			if (nBurnLayer & 4) if (TaitoF2TilePriority[2] == i && DrawLayer2) { if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0); }
		}
	}

	if (TaitoIC_TC0360PRIInUse)
		TaitoF2RenderSpriteListBackwardsForPriority();

	BurnTransferCopy(TaitoPalette);

	return 0;
}

static INT32 FinalbDraw()
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;

	BurnTransferClear();

	TaitoF2SpritePriority[0] = 0;
	TaitoF2SpritePriority[1] = 0;
	TaitoF2SpritePriority[2] = 0;
	TaitoF2SpritePriority[3] = 0;

	if (TC0100SCNBottomLayer(0)) {
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
	} else {
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
	}

	TaitoF2MakeSpriteList();

	TaitoF2RenderSpriteList(0);

	if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0);
	BurnTransferCopy(TC0110PCRPalette);

	return 0;
}

static INT32 FootchmpDraw() // and deadconx
{
	UINT8 Layer[5];
	UINT16 Priority = TC0480SCPGetBgPriority();

	Layer[0] = (Priority & 0xf000) >> 12;
	Layer[1] = (Priority & 0x0f00) >>  8;
	Layer[2] = (Priority & 0x00f0) >>  4;
	Layer[3] = (Priority & 0x000f) >>  0;
	Layer[4] = 4;

	TaitoF2TilePriority[0] = TC0360PRIRegs[4] >> 4;
	TaitoF2TilePriority[1] = TC0360PRIRegs[5] & 0x0f;
	TaitoF2TilePriority[2] = TC0360PRIRegs[5] >> 4;
	TaitoF2TilePriority[3] = TC0360PRIRegs[4] & 0x0f;
	TaitoF2TilePriority[4] = TC0360PRIRegs[7] >> 4;

	TaitoF2SpritePriority[0] = TC0360PRIRegs[6] & 0x0f;
	TaitoF2SpritePriority[1] = TC0360PRIRegs[6] >> 4;
	TaitoF2SpritePriority[2] = TC0360PRIRegs[7] & 0x0f;
	TaitoF2SpritePriority[3] = TC0360PRIRegs[7] >> 4;

	BurnTransferClear();
	TaitoF2CalcPalette();

	if (nBurnLayer & 1) TC0480SCPTilemapRenderPrio(Layer[0], 0, 1, TaitoChars);
	if (nBurnLayer & 2) TC0480SCPTilemapRenderPrio(Layer[1], 0, 2, TaitoChars);
	if (nBurnLayer & 4) TC0480SCPTilemapRenderPrio(Layer[2], 0, 4, TaitoChars);
	if (nBurnLayer & 8) TC0480SCPTilemapRenderPrio(Layer[3], 0, 8, TaitoChars);

	{ // sprite layer
		TaitoF2MakeSpriteList();

		INT32 primasks[4] = { 0, 0, 0, 0 };

		for (INT32 i = 0; i < 4; i++) {
			if (TaitoF2SpritePriority[i] < TaitoF2TilePriority[Layer[0]]) primasks[i] |= 0xaaaa;
			if (TaitoF2SpritePriority[i] < TaitoF2TilePriority[Layer[1]]) primasks[i] |= 0xcccc;
			if (TaitoF2SpritePriority[i] < TaitoF2TilePriority[Layer[2]]) primasks[i] |= 0xf0f0;
			if (TaitoF2SpritePriority[i] < TaitoF2TilePriority[Layer[3]]) primasks[i] |= 0xff00;
		}

		if (nSpriteEnable & 1)
			TaitoF2RenderSpriteListPriMasks((INT32 *)&primasks);
	}

	TC0480SCPRenderCharLayer(); // always over

	BurnTransferCopy(TaitoPalette);

	return 0;
}

static INT32 MetalbDraw()
{
	UINT8 Layer[4], InvLayer[4];
	UINT16 Priority = TC0480SCPGetBgPriority();

	Layer[0] = (Priority & 0xf000) >> 12;
	Layer[1] = (Priority & 0x0f00) >>  8;
	Layer[2] = (Priority & 0x00f0) >>  4;
	Layer[3] = (Priority & 0x000f) >>  0;

	InvLayer[Layer[0]] = 0;
	InvLayer[Layer[1]] = 1;
	InvLayer[Layer[2]] = 2;
	InvLayer[Layer[3]] = 3;

	TaitoF2TilePriority[InvLayer[0]] = TC0360PRIRegs[4] & 0x0f;
	TaitoF2TilePriority[InvLayer[1]] = TC0360PRIRegs[4] >> 4;
	TaitoF2TilePriority[InvLayer[2]] = TC0360PRIRegs[5] & 0x0f;
	TaitoF2TilePriority[InvLayer[3]] = TC0360PRIRegs[5] >> 4;
	TaitoF2TilePriority[4] = TC0360PRIRegs[9] & 0x0f;

	TaitoF2SpritePriority[0] = TC0360PRIRegs[6] & 0x0f;
	TaitoF2SpritePriority[1] = TC0360PRIRegs[6] >> 4;
	TaitoF2SpritePriority[2] = TC0360PRIRegs[7] & 0x0f;
	TaitoF2SpritePriority[3] = TC0360PRIRegs[7] >> 4;

	TaitoF2SpriteBlendMode = TC0360PRIRegs[0] & 0xc0;

	BurnTransferClear();
	MetalbCalcPalette();

	TaitoF2MakeSpriteList();

	if (nBurnLayer & 1) TC0480SCPTilemapRenderPrio(Layer[0], 0, 1, TaitoChars);
	if (nBurnLayer & 2) TC0480SCPTilemapRenderPrio(Layer[1], 0, 2, TaitoChars);
	if (nBurnLayer & 4) TC0480SCPTilemapRenderPrio(Layer[2], 0, 4, TaitoChars);
	if (nBurnLayer & 8) TC0480SCPTilemapRenderPrio(Layer[3], 0, 8, TaitoChars);

	if (nSpriteEnable & 1) TaitoF2RenderSpriteListBackwardsForPriority();

	if (nSpriteEnable & 2) TC0480SCPRenderCharLayer();
	BurnTransferCopy(TaitoPalette);

	return 0;
}

static void draw_layer(INT32 layer, INT32 pri)
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;

	switch (layer) {
		case 0:
			if ((nBurnLayer & 2) && (~Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars, pri);
			break;
		case 1:
			if ((nBurnLayer & 4) && (~Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars, pri);
			break;
		case 2:
			if ((nBurnLayer & 8) && (~Disable & 0x04)) TC0100SCNRenderCharLayer(0, pri);
			break;
	}
}

static INT32 TaitoF2PriDraw()
{
	INT32 layer[3] = { TC0100SCNBottomLayer(0), TC0100SCNBottomLayer(0) ^ 1, 2 };

	TaitoF2TilePriority[layer[0]] = TC0360PRIRegs[5] & 0x0f;
	TaitoF2TilePriority[layer[1]] = TC0360PRIRegs[5] >> 4;
	TaitoF2TilePriority[layer[2]] = TC0360PRIRegs[4] >> 4;

	TaitoF2SpritePriority[0] = TC0360PRIRegs[6] & 0x0f;
	TaitoF2SpritePriority[1] = TC0360PRIRegs[6] >> 4;
	TaitoF2SpritePriority[2] = TC0360PRIRegs[7] & 0x0f;
	TaitoF2SpritePriority[3] = TC0360PRIRegs[7] >> 4;
	TaitoF2SpriteBlendMode = TC0360PRIRegs[0] & 0xc0;

	BurnTransferClear();
	DynCalcPalette();

	TaitoF2MakeSpriteList();

	draw_layer(layer[0], 1);
	draw_layer(layer[1], 2);
	draw_layer(layer[2], 4);

	if (nSpriteEnable & 1) TaitoF2RenderSpriteListBackwardsForPriority();

	BurnTransferCopy(TaitoPalette);

	return 0;
}

static INT32 TaitoF2PriRozDraw()
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;
	INT32 RozPriority;

	INT32 layer[3] = { TC0100SCNBottomLayer(0), TC0100SCNBottomLayer(0) ^ 1, 2 };

	TaitoF2TilePriority[layer[0]] = TC0360PRIRegs[5] & 0x0f;
	TaitoF2TilePriority[layer[1]] = TC0360PRIRegs[5] >> 4;
	TaitoF2TilePriority[layer[2]] = TC0360PRIRegs[4] >> 4;

	// tileprilayers[] is a copy of TaitoF2TilePriority[], it needs to stay
	// constant. TaitoF2TilePriority[] gets dynamically altered to suit the
	// rendition.
	INT32 tileprilayers[3] = { TaitoF2TilePriority[0], TaitoF2TilePriority[1], TaitoF2TilePriority[2] };

	TaitoF2SpritePriority[0] = TC0360PRIRegs[6] & 0x0f;
	TaitoF2SpritePriority[1] = TC0360PRIRegs[6] >> 4;
	TaitoF2SpritePriority[2] = TC0360PRIRegs[7] & 0x0f;
	TaitoF2SpritePriority[3] = TC0360PRIRegs[7] >> 4;

	TaitoF2SpriteBlendMode = TC0360PRIRegs[0] & 0xc0;

	RozPriority = (TC0360PRIRegs[1] & 0xc0) >> 6;
	RozPriority = (TC0360PRIRegs[8 + (RozPriority / 2)] >> 4 * (RozPriority & 1)) & 0x0f;
	TC0280GRDBaseColour = (TC0360PRIRegs[1] & 0x3f) << 2;

	if (Driftout && RozPriority == 0) RozPriority = -1; // disable roz layer for driftout

	BurnTransferClear();
	DynCalcPalette();

	TaitoF2MakeSpriteList();

	INT32 drawn = 0;
	for (INT32 i = 0; i < 16; i++) {
		if (RozPriority == i) {
			if (nBurnLayer & 1) TC0430GRWRenderLayer(1 << drawn);

			TaitoF2TilePriority[drawn] = i;
			drawn++;
		}

		for (INT32 j = 0; j < 3; j++) {
			if (tileprilayers[layer[j]] == i) {
				if (nBurnLayer & 2) { if ((layer[j] == 0) && !(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars, 1 << drawn); }
				if (nBurnLayer & 4) { if ((layer[j] == 1) && !(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars, 1 << drawn); }
				if (nBurnLayer & 8) { if ((layer[j] == 2) && !(Disable & 0x04)) TC0100SCNRenderCharLayer(0, 1 << drawn); }

				TaitoF2TilePriority[drawn] = i;
				drawn++;
			}
		}
	}

	if (nSpriteEnable & 1) TaitoF2RenderSpriteListBackwardsForPriority();

	BurnTransferCopy(TaitoPalette);

	return 0;
}

static INT32 QtorimonDraw()
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;

	BurnTransferClear();

	TaitoF2SpritePriority[0] = 0;
	TaitoF2SpritePriority[1] = 0;
	TaitoF2SpritePriority[2] = 0;
	TaitoF2SpritePriority[3] = 0;

	TaitoF2MakeSpriteList();

	TaitoF2RenderSpriteList(0);

	if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0);
	BurnTransferCopy(TC0110PCRPalette);

	return 0;
}

static INT32 QzquestDraw()
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;

	BurnTransferClear();
	QzquestCalcPalette();

	if (TC0100SCNBottomLayer(0)) {
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
	} else {
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
	}

	TaitoF2SpritePriority[0] = 0;
	TaitoF2SpritePriority[1] = 0;
	TaitoF2SpritePriority[2] = 0;
	TaitoF2SpritePriority[3] = 0;

	TaitoF2MakeSpriteList();

	TaitoF2RenderSpriteList(0);

	if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0);
	BurnTransferCopy(TaitoPalette);

	return 0;
}

static INT32 SsiDraw()
{
	BurnTransferClear();
	TaitoF2CalcPalette();

	TaitoF2SpritePriority[0] = 0;
	TaitoF2SpritePriority[1] = 0;
	TaitoF2SpritePriority[2] = 0;
	TaitoF2SpritePriority[3] = 0;

	TaitoF2MakeSpriteList();

	TaitoF2RenderSpriteList(0);

	BurnTransferCopy(TaitoPalette);

	return 0;
}

static INT32 ThundfoxDraw()
{
	INT32 Disable1 = TC0100SCNCtrl[0][6] & 0xf7;
	INT32 Disable2 = TC0100SCNCtrl[1][6] & 0xf7;

	BurnTransferClear();
	TaitoF2CalcPalette();

	INT32 TilePri[2][3];
	INT32 Layer[2][3];

	Layer[0][0] = TC0100SCNBottomLayer(0);
	Layer[0][1] = Layer[0][0] ^ 1;
	Layer[0][2] = 2;
	TilePri[0][Layer[0][0]] = TC0360PRIRegs[5] & 0x0f;
	TilePri[0][Layer[0][1]] = TC0360PRIRegs[5] >> 4;
	TilePri[0][Layer[0][2]] = TC0360PRIRegs[4] >> 4;

	Layer[1][0] = TC0100SCNBottomLayer(1);
	Layer[1][1] = Layer[1][0] ^ 1;
	Layer[1][2] = 2;
	TilePri[1][Layer[1][0]] = TC0360PRIRegs[9] & 0x0f;
	TilePri[1][Layer[1][1]] = TC0360PRIRegs[9] >> 4;
	TilePri[1][Layer[1][2]] = TC0360PRIRegs[8] >> 4;

	TaitoF2SpritePriority[0] = TC0360PRIRegs[6] & 0x0f;
	TaitoF2SpritePriority[1] = TC0360PRIRegs[6] >> 4;
	TaitoF2SpritePriority[2] = TC0360PRIRegs[7] & 0x0f;
	TaitoF2SpritePriority[3] = TC0360PRIRegs[7] >> 4;

	TaitoF2MakeSpriteList();

//	bprintf(PRINT_NORMAL, _T("Layer0-0 %x, Layer 0-1 %x, Layer 0-2 %x, Layer1-0 %x, Layer 1-1 %x, Layer 1-2 %x, Sprite 0 %x, Sprite 1 %x, Sprite 2 %x, Sprite 3 %x\n"), TilePri[0][0], TilePri[0][1], TilePri[0][2], TilePri[1][0], TilePri[1][1], TilePri[1][2], TaitoF2SpritePriority[0], TaitoF2SpritePriority[1], TaitoF2SpritePriority[2], TaitoF2SpritePriority[3]);

	for (INT32 i = 0; i < 16; i++) {
		if (TilePri[1][0] == i) {
			if (TC0100SCNBottomLayer(1)) {
				if (!(Disable2 & 0x02)) TC0100SCNRenderFgLayer(1, 0, TaitoCharsB);
			} else {
				if (!(Disable2 & 0x01)) TC0100SCNRenderBgLayer(1, 0, TaitoCharsB);
			}
		}
		if (TilePri[0][0] == i) {
			if (TC0100SCNBottomLayer(0)) {
				if (!(Disable1 & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
			} else {
				if (!(Disable1 & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
			}
		}
		if (TilePri[1][1] == i) {
			if (!TC0100SCNBottomLayer(1)) {
				if (!(Disable2 & 0x02)) TC0100SCNRenderFgLayer(1, 0, TaitoCharsB);
			} else {
				if (!(Disable2 & 0x01)) TC0100SCNRenderBgLayer(1, 0, TaitoCharsB);
			}
		}
		if (TilePri[0][1] == i) {
			if (!TC0100SCNBottomLayer(0)) {
				if (!(Disable1 & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
			} else {
				if (!(Disable1 & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
			}
		}
		if (TaitoF2SpritePriority[3] == i) TaitoF2RenderSpriteList(TaitoF2SpritePriority[3]);
		if (TaitoF2SpritePriority[2] == i) TaitoF2RenderSpriteList(TaitoF2SpritePriority[2]);
		if (TaitoF2SpritePriority[1] == i) TaitoF2RenderSpriteList(TaitoF2SpritePriority[1]);
		if (TaitoF2SpritePriority[0] == i) TaitoF2RenderSpriteList(TaitoF2SpritePriority[0]);
		if (TilePri[1][2] == i) { if (!(Disable2 & 0x04)) TC0100SCNRenderCharLayer(1); }
		if (TilePri[0][2] == i) { if (!(Disable1 & 0x04)) TC0100SCNRenderCharLayer(0); }
	}

	BurnTransferCopy(TaitoPalette);

	return 0;
}

static INT32 YuyugogoDraw()
{
	INT32 Disable = TC0100SCNCtrl[0][6] & 0xf7;

	BurnTransferClear();
	TaitoF2CalcPalette();

	TaitoF2SpritePriority[0] = 0;
	TaitoF2SpritePriority[1] = 0;
	TaitoF2SpritePriority[2] = 0;
	TaitoF2SpritePriority[3] = 0;

	TaitoF2MakeSpriteList();

	TaitoF2RenderSpriteList(0);

	if (TC0100SCNBottomLayer(0)) {
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
	} else {
		if (!(Disable & 0x01)) TC0100SCNRenderBgLayer(0, 0, TaitoChars);
		if (!(Disable & 0x02)) TC0100SCNRenderFgLayer(0, 0, TaitoChars);
	}

	if (!(Disable & 0x04)) TC0100SCNRenderCharLayer(0);
	BurnTransferCopy(TaitoPalette);

	return 0;
}

static INT32 TaitoF2Frame()
{
	INT32 nInterleave = 40;

	if (TaitoReset) TaitoF2DoReset();

	if (TaitoIC_TC0220IOCInUse) {
		TC0220IOCMakeInputs();
	} else {
		if (TaitoIC_TC0510NIOInUse) {
			TC0510NIOMakeInputs();
		} else {
			TaitoF2MakeInputs();
		}
	}

	nTaitoCyclesDone[0] = nTaitoCyclesDone[1] = 0;

	SekNewFrame();
	ZetNewFrame();

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000 #1
		nCurrentCPU = 0;
		SekOpen(0);
		nNext = (i + 1) * nTaitoCyclesTotal[nCurrentCPU] / nInterleave;
		nTaitoCyclesSegment = nNext - nTaitoCyclesDone[nCurrentCPU];
		if (i == (nInterleave - 1)) nTaitoCyclesSegment -= 500;
		nTaitoCyclesDone[nCurrentCPU] += SekRun(nTaitoCyclesSegment);
		if (i == (nInterleave - 1)) {
			// draw @ vblank
			TaitoF2HandleSpriteBuffering();
			if (pBurnDraw) BurnDrvRedraw();
			TaitoF2SpriteBufferFunction();

			SekSetIRQLine(5, CPU_IRQSTATUS_AUTO);
			nTaitoCyclesDone[nCurrentCPU] += SekRun(500);
			SekSetIRQLine(6, CPU_IRQSTATUS_AUTO);
		}
		SekClose();

		nCurrentCPU = 1;
		ZetOpen(0);
		BurnTimerUpdate((i + 1) * (nTaitoCyclesTotal[1] / nInterleave));
		ZetClose();

		if (cchip_active) {
			cchip_run(12000000 / 60 / nInterleave);
			if (i == nInterleave-1) cchip_interrupt();
		}
	}

	ZetOpen(0);
	BurnTimerEndFrame(nTaitoCyclesTotal[1]);
	if (pBurnSoundOut) {
		if (TaitoNumYM2610) BurnYM2610Update(pBurnSoundOut, nBurnSoundLen);
		if (TaitoNumYM2203) BurnYM2203Update(pBurnSoundOut, nBurnSoundLen);
		if (TaitoNumMSM6295) MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}
	ZetClose();

	return 0;
}

static INT32 DriveoutFrame()
{
	INT32 nInterleave = 10;

	if (TaitoReset) TaitoF2DoReset();

	if (TaitoIC_TC0220IOCInUse) {
		TC0220IOCMakeInputs();
	} else {
		if (TaitoIC_TC0510NIOInUse) {
			TC0510NIOMakeInputs();
		} else {
			TaitoF2MakeInputs();
		}
	}

	nTaitoCyclesDone[0] = nTaitoCyclesDone[1] = 0;

	SekNewFrame();
	ZetNewFrame();

	for (INT32 i = 0; i < nInterleave; i++) {
		INT32 nCurrentCPU, nNext;

		// Run 68000 #1
		nCurrentCPU = 0;
		SekOpen(0);
		nNext = (i + 1) * nTaitoCyclesTotal[nCurrentCPU] / nInterleave;
		nTaitoCyclesSegment = nNext - nTaitoCyclesDone[nCurrentCPU];
		if (i == 9) nTaitoCyclesSegment -= 500;
		nTaitoCyclesDone[nCurrentCPU] += SekRun(nTaitoCyclesSegment);
		if (i == 9) {
			SekSetIRQLine(5, CPU_IRQSTATUS_AUTO);
			nTaitoCyclesDone[nCurrentCPU] += SekRun(500);
			SekSetIRQLine(6, CPU_IRQSTATUS_AUTO);
		}
		SekClose();

		// Run Z80
		nCurrentCPU = 1;
		ZetOpen(0);
		nNext = (i + 1) * nTaitoCyclesTotal[nCurrentCPU] / nInterleave;
		nTaitoCyclesSegment = nNext - nTaitoCyclesDone[nCurrentCPU];
		nTaitoCyclesSegment = ZetRun(nTaitoCyclesSegment);
		nTaitoCyclesDone[nCurrentCPU] += nTaitoCyclesSegment;
		ZetClose();
	}

	// Make sure the buffer is entirely filled.
	if (pBurnSoundOut) {
		MSM6295Render(0, pBurnSoundOut, nBurnSoundLen);
	}

	TaitoF2HandleSpriteBuffering();

	if (pBurnDraw) BurnDrvRedraw();

	TaitoF2SpriteBufferFunction();

	return 0;
}

static INT32 TaitoF2Scan(INT32 nAction, INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin != NULL) {			// Return minimum compatible version
		*pnMin = 0x029682;
	}

	if (nAction & ACB_MEMORY_RAM) {
		memset(&ba, 0, sizeof(ba));
		ba.Data	  = TaitoRamStart;
		ba.nLen	  = TaitoRamEnd-TaitoRamStart;
		ba.szName = "All Ram";
		BurnAcb(&ba);
	}

	TaitoICScan(nAction);

	if (nAction & ACB_DRIVER_DATA) {
		SekScan(nAction);
		ZetScan(nAction);

		if (TaitoNumYM2203) { // cameltrya..
			BurnYM2203Scan(nAction, pnMin);
			MSM6295Scan(nAction, pnMin);
		}

		if (TaitoNumYM2610) {
			BurnYM2610Scan(nAction, pnMin);
		}

		SCAN_VAR(TaitoZ80Bank);

		SCAN_VAR(TaitoF2SpritesFlipScreen);
		SCAN_VAR(TaitoF2PrepareSprites);
		SCAN_VAR(TaitoF2SpritesDisabled);
		SCAN_VAR(TaitoF2SpritesActiveArea);
		SCAN_VAR(TaitoF2SpritesMasterScrollX);
		SCAN_VAR(TaitoF2SpritesMasterScrollY);
		SCAN_VAR(TaitoF2SpriteBlendMode);
		SCAN_VAR(TaitoF2SpriteBank);
		SCAN_VAR(TaitoF2SpriteBankBuffered);
		SCAN_VAR(TaitoF2TilePriority);
		SCAN_VAR(TaitoF2SpritePriority);

		SCAN_VAR(YesnoDip);
		SCAN_VAR(MjnquestInput);
		SCAN_VAR(DriveoutSoundNibble);
		SCAN_VAR(DriveoutOkiBank);
	}

	if (nAction & ACB_WRITE) {
		ZetOpen(0);
		z80_bank(TaitoZ80Bank);
		ZetClose();

		if (DriveoutOkiBank) {
			DriveoutMSM6295Bank(DriveoutOkiBank);
		}
	}

	return 0;
}

struct BurnDriver BurnDrvCameltry = {
	"cameltry", NULL, NULL, NULL, "1989",
	"Cameltry (World, YM2610)\0", NULL, "Taito America Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_MAZE, 0,
	NULL, CameltryRomInfo, CameltryRomName, NULL, NULL, NULL, NULL, CameltryInputInfo, CameltryDIPInfo,
	CameltryInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriRozDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvCameltryu = {
	"cameltryu", "cameltry", NULL, NULL, "1989",
	"Cameltry (US, YM2610)\0", NULL, "Taito America Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_MAZE, 0,
	NULL, CameltryuRomInfo, CameltryuRomName, NULL, NULL, NULL, NULL, CameltryInputInfo, CameltryDIPInfo,
	CameltryInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriRozDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvCameltryau = {
	"cameltryau", "cameltry", NULL, NULL, "1989",
	"Cameltry (US, YM2203 + M6295)\0", NULL, "Taito America Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_MAZE, 0,
	NULL, CameltryauRomInfo, CameltryauRomName, NULL, NULL, NULL, NULL, CameltryInputInfo, CameltryDIPInfo,
	CamltryaInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriRozDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvCameltrya = {
	"cameltrya", "cameltry", NULL, NULL, "1989",
	"Cameltry (World, YM2203 + M6295)\0", NULL, "Taito America Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_MAZE, 0,
	NULL, CameltryaRomInfo, CameltryaRomName, NULL, NULL, NULL, NULL, CameltryInputInfo, CameltryDIPInfo,
	CamltryaInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriRozDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvCameltryj = {
	"cameltryj", "cameltry", NULL, NULL, "1989",
	"Cameltry (Japan, YM2610)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_MAZE, 0,
	NULL, CameltryjRomInfo, CameltryjRomName, NULL, NULL, NULL, NULL, CameltryInputInfo, CameltrjDIPInfo,
	CameltryInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriRozDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvDeadconx = {
	"deadconx", NULL, NULL, NULL, "1992",
	"Dead Connection (World)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_RUNGUN, 0,
	NULL, DeadconxRomInfo, DeadconxRomName, NULL, NULL, NULL, NULL, DeadconxInputInfo, DeadconxDIPInfo,
	DeadconxInit, TaitoF2Exit, TaitoF2Frame, FootchmpDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvDeadconxj = {
	"deadconxj", "deadconx", NULL, NULL, "1992",
	"Dead Connection (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_RUNGUN, 0,
	NULL, DeadconxjRomInfo, DeadconxjRomName, NULL, NULL, NULL, NULL, DeadconxInputInfo, DeadconxjDIPInfo,
	DeadconxInit, TaitoF2Exit, TaitoF2Frame, FootchmpDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvDinorex = {
	"dinorex", NULL, NULL, NULL, "1992",
	"Dino Rex (World)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_VSFIGHT, 0,
	NULL, DinorexRomInfo, DinorexRomName, NULL, NULL, NULL, NULL, DinorexInputInfo, DinorexDIPInfo,
	DinorexInit, TaitoF2Exit, TaitoF2Frame, TaitoF2Draw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvDinorexj = {
	"dinorexj", "dinorex", NULL, NULL, "1992",
	"Dino Rex (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_VSFIGHT, 0,
	NULL, DinorexjRomInfo, DinorexjRomName, NULL, NULL, NULL, NULL, DinorexInputInfo, DinorexjDIPInfo,
	DinorexInit, TaitoF2Exit, TaitoF2Frame, TaitoF2Draw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvDinorexu = {
	"dinorexu", "dinorex", NULL, NULL, "1992",
	"Dino Rex (US)\0", NULL, "Taito America Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_VSFIGHT, 0,
	NULL, DinorexuRomInfo, DinorexuRomName, NULL, NULL, NULL, NULL, DinorexInputInfo, DinorexDIPInfo,
	DinorexInit, TaitoF2Exit, TaitoF2Frame, TaitoF2Draw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvDondokod = {
	"dondokod", NULL, NULL, NULL, "1989",
	"Don Doko Don (World, rev 1)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_PLATFORM, 0,
	NULL, DondokodRomInfo, DondokodRomName, NULL, NULL, NULL, NULL, DondokodInputInfo, DondokodDIPInfo,
	DondokodInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriRozDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvDondokodj = {
	"dondokodj", "dondokod", NULL, NULL, "1989",
	"Don Doko Don (Japan, rev 1)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_PLATFORM, 0,
	NULL, DondokodjRomInfo, DondokodjRomName, NULL, NULL, NULL, NULL, DondokodInputInfo, DondokodjDIPInfo,
	DondokodInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriRozDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvDondokodu = {
	"dondokodu", "dondokod", NULL, NULL, "1989",
	"Don Doko Don (US, rev 1)\0", NULL, "Taito America Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_PLATFORM, 0,
	NULL, DondokoduRomInfo, DondokoduRomName, NULL, NULL, NULL, NULL, DondokodInputInfo, DondokoduDIPInfo,
	DondokodInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriRozDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvDriftout = {
	"driftout", NULL, NULL, NULL, "1991",
	"Drift Out (Europe)\0", NULL, "Visco", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL, 2, HARDWARE_TAITO_TAITOF2, GBF_RACING, 0,
	NULL, DriftoutRomInfo, DriftoutRomName, NULL, NULL, NULL, NULL, DriftoutInputInfo, DriftoutDIPInfo,
	DriftoutInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriRozDraw, TaitoF2Scan,
	NULL, 0x2000, 224, 320, 3, 4
};

struct BurnDriver BurnDrvDriftoutj = {
	"driftoutj", "driftout", NULL, NULL, "1991",
	"Drift Out (Japan)\0", NULL, "Visco", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_CLONE, 2, HARDWARE_TAITO_TAITOF2, GBF_RACING, 0,
	NULL, DriftoutjRomInfo, DriftoutjRomName, NULL, NULL, NULL, NULL, DriftoutInputInfo, DriftoutDIPInfo,
	DriftoutInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriRozDraw, TaitoF2Scan,
	NULL, 0x2000, 224, 320, 3, 4
};

struct BurnDriver BurnDrvDriveout = {
	"driveout", "driftout", NULL, NULL, "1991",
	"Drive Out (bootleg of Drift Out)\0", NULL, "bootleg (Electronic Devices)", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_CLONE | BDF_BOOTLEG, 2, HARDWARE_TAITO_TAITOF2, GBF_RACING, 0,
	NULL, DriveoutRomInfo, DriveoutRomName, NULL, NULL, NULL, NULL, DriftoutInputInfo, DriftoutDIPInfo,
	DriveoutInit, TaitoF2Exit, DriveoutFrame, TaitoF2PriRozDraw, TaitoF2Scan,
	NULL, 0x2000, 224, 320, 3, 4
};

struct BurnDriver BurnDrvFinalb = {
	"finalb", NULL, NULL, NULL, "1988",
	"Final Blow (World)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_VSFIGHT, 0,
	NULL, FinalbRomInfo, FinalbRomName, NULL, NULL, NULL, NULL, FinalbInputInfo, FinalbDIPInfo,
	FinalbInit, TaitoF2Exit, TaitoF2Frame, FinalbDraw, TaitoF2Scan,
	NULL, 0x1000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvFinalbj = {
	"finalbj", "finalb", NULL, NULL, "1988",
	"Final Blow (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_VSFIGHT, 0,
	NULL, FinalbjRomInfo, FinalbjRomName, NULL, NULL, NULL, NULL, FinalbInputInfo, FinalbjDIPInfo,
	FinalbInit, TaitoF2Exit, TaitoF2Frame, FinalbDraw, TaitoF2Scan,
	NULL, 0x1000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvFinalbu = {
	"finalbu", "finalb", NULL, NULL, "1988",
	"Final Blow (US, rev 1)\0", NULL, "Taito America Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_VSFIGHT, 0,
	NULL, FinalbuRomInfo, FinalbuRomName, NULL, NULL, NULL, NULL, FinalbInputInfo, FinalbjDIPInfo,
	FinalbInit, TaitoF2Exit, TaitoF2Frame, FinalbDraw, TaitoF2Scan,
	NULL, 0x1000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvFootchmp = {
	"footchmp", NULL, NULL, NULL, "1990",
	"Football Champ / Euro Football Champ (World)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TAITO_TAITOF2, GBF_SPORTSFOOTBALL, 0,
	NULL, FootchmpRomInfo, FootchmpRomName, NULL, NULL, NULL, NULL, FootchmpInputInfo, FootchmpDIPInfo,
	FootchmpInit, TaitoF2Exit, TaitoF2Frame, FootchmpDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvHtherou = {
	"htherou", "footchmp", NULL, NULL, "1990",
	"Hat Trick Hero (US)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TAITO_TAITOF2, GBF_SPORTSFOOTBALL, 0,
	NULL, HtherouRomInfo, HtherouRomName, NULL, NULL, NULL, NULL, FootchmpInputInfo, HtherouDIPInfo,
	FootchmpInit, TaitoF2Exit, TaitoF2Frame, FootchmpDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvHtheroj = {
	"htheroj", "footchmp", NULL, NULL, "1990",
	"Hat Trick Hero (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TAITO_TAITOF2, GBF_SPORTSFOOTBALL, 0,
	NULL, HtherojRomInfo, HtherojRomName, NULL, NULL, NULL, NULL, FootchmpInputInfo, HtherojDIPInfo,
	FootchmpInit, TaitoF2Exit, TaitoF2Frame, FootchmpDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvEuroch92 = {
	"euroch92", NULL, NULL, NULL, "1992",
	"Euro Champ '92 (World)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TAITO_TAITOF2, GBF_SPORTSFOOTBALL, 0,
	NULL, Euroch92RomInfo, Euroch92RomName, NULL, NULL, NULL, NULL, FootchmpInputInfo, FootchmpDIPInfo,
	FootchmpInit, TaitoF2Exit, TaitoF2Frame, FootchmpDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvEuroch92j = {
	"euroch92j", "euroch92", NULL, NULL, "1992",
	"Euro Champ '92 (Japan)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TAITO_TAITOF2, GBF_SPORTSFOOTBALL, 0,
	NULL, Euroch92jRomInfo, Euroch92jRomName, NULL, NULL, NULL, NULL, FootchmpInputInfo, FootchmpDIPInfo,
	FootchmpInit, TaitoF2Exit, TaitoF2Frame, FootchmpDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvGrowl = {
	"growl", NULL, NULL, NULL, "1990",
	"Growl (World, Rev 1)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TAITO_TAITOF2, GBF_SCRFIGHT, 0,
	NULL, GrowlRomInfo, GrowlRomName, NULL, NULL, NULL, NULL, GrowlInputInfo, GrowlDIPInfo,
	GrowlInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvGrowla = {
	"growla", "growl", NULL, NULL, "1990",
	"Growl (World)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TAITO_TAITOF2, GBF_SCRFIGHT, 0,
	NULL, GrowlaRomInfo, GrowlaRomName, NULL, NULL, NULL, NULL, GrowlInputInfo, GrowluDIPInfo,
	GrowlInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvGrowlu = {
	"growlu", "growl", NULL, NULL, "1990",
	"Growl (US)\0", NULL, "Taito America Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TAITO_TAITOF2, GBF_SCRFIGHT, 0,
	NULL, GrowluRomInfo, GrowluRomName, NULL, NULL, NULL, NULL, GrowlInputInfo, GrowluDIPInfo,
	GrowlInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvRunark = {
	"runark", "growl", NULL, NULL, "1990",
	"Runark (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TAITO_TAITOF2, GBF_SCRFIGHT, 0,
	NULL, RunarkRomInfo, RunarkRomName, NULL, NULL, NULL, NULL, GrowlInputInfo, RunarkDIPInfo,
	GrowlInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvGrowlp = {
	"growlp", "growl", NULL, NULL, "1990",
	"Growl (World, prototype)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_PROTOTYPE | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TAITO_TAITOF2, GBF_SCRFIGHT, 0,
	NULL, GrowlpRomInfo, GrowlpRomName, NULL, NULL, NULL, NULL, GrowlInputInfo, GrowluDIPInfo,
	GrowlInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvGunfront = {
	"gunfront", NULL, NULL, NULL, "1990",
	"Gun & Frontier (World)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_VERSHOOT, 0,
	NULL, GunfrontRomInfo, GunfrontRomName, NULL, NULL, NULL, NULL, GunfrontInputInfo, GunfrontDIPInfo,
	GunfrontInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriDraw, TaitoF2Scan,
	NULL, 0x2000, 224, 320, 3, 4
};

struct BurnDriver BurnDrvGunfrontj = {
	"gunfrontj", "gunfront", NULL, NULL, "1990",
	"Gun Frontier (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_VERSHOOT, 0,
	NULL, GunfrontjRomInfo, GunfrontjRomName, NULL, NULL, NULL, NULL, GunfrontInputInfo, GunfrontjDIPInfo,
	GunfrontInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriDraw, TaitoF2Scan,
	NULL, 0x2000, 224, 320, 3, 4
};

struct BurnDriver BurnDrvKoshien = {
	"koshien", NULL, NULL, NULL, "1990",
	"Ah Eikou no Koshien (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	L"\u7532\u5B50\u5712 \u6804\u5149\u306E \u55DA\u547C (Japan)\0Ah Eikou no Koshien\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOF2, GBF_SPORTSMISC, 0,
	NULL, KoshienRomInfo, KoshienRomName, NULL, NULL, NULL, NULL, KoshienInputInfo, KoshienDIPInfo,
	KoshienInit, TaitoF2Exit, TaitoF2Frame, TaitoF2Draw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvLiquidk = {
	"liquidk", NULL, NULL, NULL, "1990",
	"Liquid Kids (World)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_PLATFORM, 0,
	NULL, LiquidkRomInfo, LiquidkRomName, NULL, NULL, NULL, NULL, LiquidkInputInfo, LiquidkDIPInfo,
	LiquidkInit, TaitoF2Exit, TaitoF2Frame, TaitoF2Draw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvLiquidku = {
	"liquidku", "liquidk", NULL, NULL, "1990",
	"Liquid Kids (US)\0", NULL, "Taito America Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_PLATFORM, 0,
	NULL, LiquidkuRomInfo, LiquidkuRomName, NULL, NULL, NULL, NULL, LiquidkInputInfo, LiquidkuDIPInfo,
	LiquidkInit, TaitoF2Exit, TaitoF2Frame, TaitoF2Draw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvMizubaku = {
	"mizubaku", "liquidk", NULL, NULL, "1990",
	"Mizubaku Daibouken (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	L"\u30DF\u30BA\u30D0\u30AF \u5927\u5192\u967A \u30A2\u30C9\u30D9\u30F3\u30C1\u30E3\u30FC (Japan)\0Mizubaku Daibouken\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_PLATFORM, 0,
	NULL, MizubakuRomInfo, MizubakuRomName, NULL, NULL, NULL, NULL, LiquidkInputInfo, LiquidkuDIPInfo,
	LiquidkInit, TaitoF2Exit, TaitoF2Frame, TaitoF2Draw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvMegablst = {
	"megablst", NULL, "cchip", NULL, "1989",
	"Mega Blast (World)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_HORSHOOT, 0,
	NULL, MegablstRomInfo, MegablstRomName, NULL, NULL, NULL, NULL, MegablstInputInfo, MegablstDIPInfo,
	MegablstInit, TaitoF2Exit, TaitoF2Frame, TaitoF2Draw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvMegablstj = {
	"megablstj", "megablst", "cchip", NULL, "1989",
	"Mega Blast (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_HORSHOOT, 0,
	NULL, MegablstjRomInfo, MegablstjRomName, NULL, NULL, NULL, NULL, MegablstInputInfo, MegablstjDIPInfo,
	MegablstInit, TaitoF2Exit, TaitoF2Frame, TaitoF2Draw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvMegablstu = {
	"megablstu", "megablst", "cchip", NULL, "1989",
	"Mega Blast (US)\0", NULL, "Taito America Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_HORSHOOT, 0,
	NULL, MegablstuRomInfo, MegablstuRomName, NULL, NULL, NULL, NULL, MegablstInputInfo, MegablstuDIPInfo,
	MegablstInit, TaitoF2Exit, TaitoF2Frame, TaitoF2Draw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvMetalb = {
	"metalb", NULL, NULL, NULL, "1991",
	"Metal Black (World)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_HORSHOOT, 0,
	NULL, MetalbRomInfo, MetalbRomName, NULL, NULL, NULL, NULL, MetalbInputInfo, MetalbDIPInfo,
	MetalbInit, TaitoF2Exit, TaitoF2Frame, MetalbDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvMetalba = {
	"metalba", "metalb", NULL, NULL, "1991",
	"Metal Black (World, single PCB)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_HORSHOOT, 0,
	NULL, MetalbaRomInfo, MetalbaRomName, NULL, NULL, NULL, NULL, MetalbInputInfo, MetalbDIPInfo,
	MetalbInit, TaitoF2Exit, TaitoF2Frame, MetalbDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvMetalbj = {
	"metalbj", "metalb", NULL, NULL, "1991",
	"Metal Black (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_HORSHOOT, 0,
	NULL, MetalbjRomInfo, MetalbjRomName, NULL, NULL, NULL, NULL, MetalbInputInfo, MetalbjDIPInfo,
	MetalbInit, TaitoF2Exit, TaitoF2Frame, MetalbDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvMjnquest = {
	"mjnquest", NULL, NULL, NULL, "1990",
	"Mahjong Quest (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOF2, GBF_MAHJONG, 0,
	NULL, MjnquestRomInfo, MjnquestRomName, NULL, NULL, NULL, NULL, MjnquestInputInfo, MjnquestDIPInfo,
	MjnquestInit, TaitoF2Exit, TaitoF2Frame, FinalbDraw, TaitoF2Scan,
	NULL, 0x1000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvMjnquestb = {
	"mjnquestb", "mjnquest", NULL, NULL, "1990",
	"Mahjong Quest (Japan, No Nudity)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE, 2, HARDWARE_TAITO_TAITOF2, GBF_MAHJONG, 0,
	NULL, MjnquestbRomInfo, MjnquestbRomName, NULL, NULL, NULL, NULL, MjnquestInputInfo, MjnquestDIPInfo,
	MjnquestInit, TaitoF2Exit, TaitoF2Frame, FinalbDraw, TaitoF2Scan,
	NULL, 0x1000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvNinjak = {
	"ninjak", NULL, NULL, NULL, "1990",
	"The Ninja Kids (World)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TAITO_TAITOF2, GBF_SCRFIGHT, 0,
	NULL, NinjakRomInfo, NinjakRomName, NULL, NULL, NULL, NULL, NinjakInputInfo, NinjakDIPInfo,
	NinjakInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvNinjakj = {
	"ninjakj", "ninjak", NULL, NULL, "1990",
	"The Ninja Kids (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TAITO_TAITOF2, GBF_SCRFIGHT, 0,
	NULL, NinjakjRomInfo, NinjakjRomName, NULL, NULL, NULL, NULL, NinjakInputInfo, NinjakjDIPInfo,
	NinjakInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvNinjaku = {
	"ninjaku", "ninjak", NULL, NULL, "1990",
	"The Ninja Kids (US)\0", NULL, "Taito America Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TAITO_TAITOF2, GBF_SCRFIGHT, 0,
	NULL, NinjakuRomInfo, NinjakuRomName, NULL, NULL, NULL, NULL, NinjakInputInfo, NinjakuDIPInfo,
	NinjakInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvPulirula = {
	"pulirula", NULL, NULL, NULL, "1991",
	"PuLiRuLa (World, dual PCB)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_SCRFIGHT, 0,
	NULL, PulirulaRomInfo, PulirulaRomName, NULL, NULL, NULL, NULL, PulirulaInputInfo, PulirulaDIPInfo,
	PulirulaInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriRozDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvPulirulaa = {
	"pulirulaa", "pulirula", NULL, NULL, "1991",
	"PuLiRuLa (World, single PCB)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_SCRFIGHT, 0,
	NULL, PulirulaaRomInfo, PulirulaaRomName, NULL, NULL, NULL, NULL, PulirulaInputInfo, PulirulaDIPInfo,
	PulirulaInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriRozDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvPulirulaj = {
	"pulirulaj", "pulirula", NULL, NULL, "1991",
	"PuLiRuLa (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_SCRFIGHT, 0,
	NULL, PulirulajRomInfo, PulirulajRomName, NULL, NULL, NULL, NULL, PulirulaInputInfo, PulirulajDIPInfo,
	PulirulaInit, TaitoF2Exit, TaitoF2Frame, TaitoF2PriRozDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvQcrayon = {
	"qcrayon", NULL, NULL, NULL, "1993",
	"Quiz Crayon Shinchan (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	L"\u30AF\u30A4\u30BA \u30AF\u30EC\u30E8\u30F3\u3057\u3093\u3061\u3083\u3093 (Japan)\0Quiz Crayon Shinchan\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOF2, GBF_QUIZ, 0,
	NULL, QcrayonRomInfo, QcrayonRomName, NULL, NULL, NULL, NULL, QcrayonInputInfo, QcrayonDIPInfo,
	QcrayonInit, TaitoF2Exit, TaitoF2Frame, TaitoF2Draw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvQcrayon2 = {
	"qcrayon2", NULL, NULL, NULL, "1993",
	"Crayon Shinchan Orato Asobo (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	L"\u30AF\u30EC\u30E8\u30F3\u3057\u3093\u3061\u3083\u3093 \u30AA\u30E9\u3068\u904A\u307C (Japan)\0Crayon Shinchan Orato Asobo\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOF2, GBF_QUIZ, 0,
	NULL, Qcrayon2RomInfo, Qcrayon2RomName, NULL, NULL, NULL, NULL, Qcrayon2InputInfo, Qcrayon2DIPInfo,
	Qcrayon2Init, TaitoF2Exit, TaitoF2Frame, TaitoF2Draw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvQjinsei = {
	"qjinsei", NULL, NULL, NULL, "1992",
	"Quiz Jinsei Gekijoh (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	L"\u30AF\u30A4\u30BA \u4EBA\u751F\u5287\u5834 (Japan)\0Quiz Jinsei Gekijoh\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOF2, GBF_QUIZ, 0,
	NULL, QjinseiRomInfo, QjinseiRomName, NULL, NULL, NULL, NULL, QjinseiInputInfo, QjinseiDIPInfo,
	QjinseiInit, TaitoF2Exit, TaitoF2Frame, TaitoF2Draw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvQtorimon = {
	"qtorimon", NULL, NULL, NULL, "1990",
	"Quiz Torimonochou (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	L"\u304F\u3044\u305A \u82E6\u80C3\u982D \u6355\u7269\u5E33 (Japan)\0Quiz Torimonochou\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOF2, GBF_QUIZ, 0,
	NULL, QtorimonRomInfo, QtorimonRomName, NULL, NULL, NULL, NULL, QtorimonInputInfo, QtorimonDIPInfo,
	QtorimonInit, TaitoF2Exit, TaitoF2Frame, QtorimonDraw, TaitoF2Scan,
	NULL, 0x800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvQuizhq = {
	"quizhq", NULL, NULL, NULL, "1990",
	"Quiz H.Q. (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOF2, GBF_QUIZ, 0,
	NULL, QuizhqRomInfo, QuizhqRomName, NULL, NULL, NULL, NULL, QuizhqInputInfo, QuizhqDIPInfo,
	QuizhqInit, TaitoF2Exit, TaitoF2Frame, QtorimonDraw, TaitoF2Scan,
	NULL, 0x1000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvQzchikyu = {
	"qzchikyu", NULL, NULL, NULL, "1991",
	"Quiz Chikyu Bouei Gun (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	L"\u30AF\u30A4\u30BA \u5730\u7403\u9632\u885B\u8ECD (Japan)\0Quiz Chikyu Bouei Gun\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOF2, GBF_QUIZ, 0,
	NULL, QzchikyuRomInfo, QzchikyuRomName, NULL, NULL, NULL, NULL, QzchikyuInputInfo, QzchikyuDIPInfo,
	QzchikyuInit, TaitoF2Exit, TaitoF2Frame, QzquestDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvQzquest = {
	"qzquest", NULL, NULL, NULL, "1991",
	"Quiz Quest - Hime to Yuusha no Monogatari (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	L"\u30AF\u30A4\u30BA \u30AF\u30A8\u30B9\u30C8 \uFF0D\u59EB\u3068\u52C7\u8005\u306E\u7269\u8A9E\uFF0D (Japan)\0Quiz Quest - Hime to Yuusha no Monogatari\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOF2, GBF_QUIZ, 0,
	NULL, QzquestRomInfo, QzquestRomName, NULL, NULL, NULL, NULL, QzquestInputInfo, QzquestDIPInfo,
	QzquestInit, TaitoF2Exit, TaitoF2Frame, QzquestDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvSolfigtr = {
	"solfigtr", NULL, NULL, NULL, "1991",
	"Solitary Fighter (World)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_VSFIGHT, 0,
	NULL, SolfigtrRomInfo, SolfigtrRomName, NULL, NULL, NULL, NULL, SolfigtrInputInfo, SolfigtrDIPInfo,
	SolfigtrInit, TaitoF2Exit, TaitoF2Frame, TaitoF2Draw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvSsi = {
	"ssi", NULL, NULL, NULL, "1990",
	"Super Space Invaders '91 (World, revised code, Rev 1)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_SHOOT, 0,
	NULL, SsiRomInfo, SsiRomName, NULL, NULL, NULL, NULL, SsiInputInfo, SsiDIPInfo,
	SsiInit, TaitoF2Exit, TaitoF2Frame, SsiDraw, TaitoF2Scan,
	NULL, 0x2000, 224, 320, 3, 4
};

struct BurnDriver BurnDrvSsia = {
	"ssia", "ssi", NULL, NULL, "1990",
	"Super Space Invaders '91 (World, revised code)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_SHOOT, 0,
	NULL, SsiaRomInfo, SsiaRomName, NULL, NULL, NULL, NULL, SsiInputInfo, SsiDIPInfo,
	SsiInit, TaitoF2Exit, TaitoF2Frame, SsiDraw, TaitoF2Scan,
	NULL, 0x2000, 224, 320, 3, 4
};

struct BurnDriver BurnDrvSsib = {
	"ssib", "ssi", NULL, NULL, "1990",
	"Super Space Invaders '91 (World, earlier code base)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_SHOOT, 0,
	NULL, SsibRomInfo, SsibRomName, NULL, NULL, NULL, NULL, SsiInputInfo, SsiDIPInfo,
	SsiInit, TaitoF2Exit, TaitoF2Frame, SsiDraw, TaitoF2Scan,
	NULL, 0x2000, 224, 320, 3, 4
};

struct BurnDriver BurnDrvMajest12u = {
	"majest12u", "ssi", NULL, NULL, "1990",
	"Majestic Twelve - The Space Invaders Part IV (US, revised code, Rev 1)\0", NULL, "Taito America Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_SHOOT, 0,
	NULL, Majest12uRomInfo, Majest12uRomName, NULL, NULL, NULL, NULL, SsiInputInfo, Majest12DIPInfo,
	SsiInit, TaitoF2Exit, TaitoF2Frame, SsiDraw, TaitoF2Scan,
	NULL, 0x2000, 224, 320, 3, 4
};

struct BurnDriver BurnDrvMajest12ua = {
	"majest12ua", "ssi", NULL, NULL, "1990",
	"Majestic Twelve - The Space Invaders Part IV (US, revised code)\0", NULL, "Taito America Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_SHOOT, 0,
	NULL, Majest12uaRomInfo, Majest12uaRomName, NULL, NULL, NULL, NULL, SsiInputInfo, Majest12DIPInfo,
	SsiInit, TaitoF2Exit, TaitoF2Frame, SsiDraw, TaitoF2Scan,
	NULL, 0x2000, 224, 320, 3, 4
};

struct BurnDriver BurnDrvMajest12ub = {
	"majest12ub", "ssi", NULL, NULL, "1990",
	"Majestic Twelve - The Space Invaders Part IV (US, earlier code base)\0", NULL, "Taito America Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_SHOOT, 0,
	NULL, Majest12ubRomInfo, Majest12ubRomName, NULL, NULL, NULL, NULL, SsiInputInfo, Majest12DIPInfo,
	SsiInit, TaitoF2Exit, TaitoF2Frame, SsiDraw, TaitoF2Scan,
	NULL, 0x2000, 224, 320, 3, 4
};

struct BurnDriver BurnDrvMajest12j = {
	"majest12j", "ssi", NULL, NULL, "1990",
	"Majestic Twelve - The Space Invaders Part IV (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_ORIENTATION_VERTICAL | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_SHOOT, 0,
	NULL, Majest12jRomInfo, Majest12jRomName, NULL, NULL, NULL, NULL, SsiInputInfo, Majest12DIPInfo,
	SsiInit, TaitoF2Exit, TaitoF2Frame, SsiDraw, TaitoF2Scan,
	NULL, 0x2000, 224, 320, 3, 4
};

struct BurnDriver BurnDrvThundfox = {
	"thundfox", NULL, NULL, NULL, "1990",
	"Thunder Fox (World, rev 1)\0", NULL, "Taito Corporation Japan", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_SCRFIGHT, 0,
	NULL, ThundfoxRomInfo, ThundfoxRomName, NULL, NULL, NULL, NULL, ThundfoxInputInfo, ThundfoxDIPInfo,
	ThundfoxInit, TaitoF2Exit, TaitoF2Frame, ThundfoxDraw, TaitoF2Scan,
	NULL, 0x1000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvThundfoxj = {
	"thundfoxj", "thundfox", NULL, NULL, "1990",
	"Thunder Fox (Japan, rev 1)\0", NULL, "Taito Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_SCRFIGHT, 0,
	NULL, ThundfoxjRomInfo, ThundfoxjRomName, NULL, NULL, NULL, NULL, ThundfoxInputInfo, ThundfoxjDIPInfo,
	ThundfoxInit, TaitoF2Exit, TaitoF2Frame, ThundfoxDraw, TaitoF2Scan,
	NULL, 0x1000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvThundfoxu = {
	"thundfoxu", "thundfox", NULL, NULL, "1990",
	"Thunder Fox (US, rev 1)\0", NULL, "Taito America Corporation", "Taito F2",
	NULL, NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_TAITO_TAITOF2, GBF_SCRFIGHT, 0,
	NULL, ThundfoxuRomInfo, ThundfoxuRomName, NULL, NULL, NULL, NULL, ThundfoxInputInfo, ThundfoxuDIPInfo,
	ThundfoxInit, TaitoF2Exit, TaitoF2Frame, ThundfoxDraw, TaitoF2Scan,
	NULL, 0x1000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvYesnoj = {
	"yesnoj", NULL, NULL, NULL, "1992",
	"Yes/No Sinri Tokimeki Chart\0", NULL, "Taito Corporation", "Taito F2",
	L"\uFF39\uFF45\uFF53.\uFF2E\uFF4F \u5FC3\u7406 \u30C8\u30AD\u30E1\u30AD\u30C1\u30E3\u30FC\u30C8\0Yes/No Sinri Tokimeki Chart\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOF2, GBF_QUIZ, 0,
	NULL, YesnojRomInfo, YesnojRomName, NULL, NULL, NULL, NULL, YesnojInputInfo, YesnojDIPInfo,
	YesnojInit, TaitoF2Exit, TaitoF2Frame, YuyugogoDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};

struct BurnDriver BurnDrvYuyugogo = {
	"yuyugogo", NULL, NULL, NULL, "1990",
	"Yuuyu no Quiz de GO!GO! (Japan)\0", NULL, "Taito Corporation", "Taito F2",
	L"\u3086\u3046\u3086 \u306E\u30AF\u30A4\u30BA\u3067 \uFF27\uFF4F!\uFF27\uFF4F! (Japan)\0Yuuyu no Quiz de GO!GO!\0", NULL, NULL, NULL,
	BDF_GAME_WORKING, 2, HARDWARE_TAITO_TAITOF2, GBF_QUIZ, 0,
	NULL, YuyugogoRomInfo, YuyugogoRomName, NULL, NULL, NULL, NULL, YuyugogoInputInfo, YuyugogoDIPInfo,
	YuyugogoInit, TaitoF2Exit, TaitoF2Frame, YuyugogoDraw, TaitoF2Scan,
	NULL, 0x2000, 320, 224, 4, 3
};
