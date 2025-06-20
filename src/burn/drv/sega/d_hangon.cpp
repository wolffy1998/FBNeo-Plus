#include "sys16.h"

/*====================================================
Input Defs
====================================================*/

#define A(a, b, c, d) {a, b, (UINT8*)(c), d}

static struct BurnInputInfo EndurorInputList[] = {
	{"Coin 1"            , BIT_DIGITAL   , System16InputPort0 + 0, "p1 coin"    },
	{"Start 1"           , BIT_DIGITAL   , System16InputPort0 + 6, "p1 start"   },
	{"Coin 2"            , BIT_DIGITAL   , System16InputPort0 + 1, "p2 coin"    },

	A("Steering"         , BIT_ANALOG_REL, &System16AnalogPort0,   "p1 x-axis"  ),
	A("Bank Up/Down"     , BIT_ANALOG_REL, &System16AnalogPort1,   "p1 y-axis"  ),
	A("Accelerate"       , BIT_ANALOG_REL, &System16AnalogPort2,   "p1 fire 1"  ),
	A("Brake"            , BIT_ANALOG_REL, &System16AnalogPort3,   "p1 fire 2"  ),
	
	{"Service"           , BIT_DIGITAL   , System16InputPort0 + 3 , "service"   },
	{"Diagnostics"       , BIT_DIGITAL   , System16InputPort0 + 2 , "diag"      },
	{"Reset"             , BIT_DIGITAL   , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , System16Dip + 1        , "dip"       },
};

STDINPUTINFO(Enduror)

static struct BurnInputInfo HangonInputList[] = {
	{"Coin 1"            , BIT_DIGITAL   , System16InputPort0 + 0, "p1 coin"    },
	{"Start 1"           , BIT_DIGITAL   , System16InputPort0 + 4, "p1 start"   },
	{"Coin 2"            , BIT_DIGITAL   , System16InputPort0 + 1, "p2 coin"    },

	A("Steering"         , BIT_ANALOG_REL, &System16AnalogPort0,   "p1 x-axis"  ),
	A("Accelerate"       , BIT_ANALOG_REL, &System16AnalogPort1,   "p1 fire 1"  ),
	A("Brake"            , BIT_ANALOG_REL, &System16AnalogPort2,   "p1 fire 2"  ),
	
	{"Service"           , BIT_DIGITAL   , System16InputPort0 + 3 , "service"   },
	{"Diagnostics"       , BIT_DIGITAL   , System16InputPort0 + 2 , "diag"      },
	{"Reset"             , BIT_DIGITAL   , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , System16Dip + 1        , "dip"       },
};

STDINPUTINFO(Hangon)

static struct BurnInputInfo ShangonrbInputList[] = {
	{"Coin 1"            , BIT_DIGITAL   , System16InputPort0 + 0, "p1 coin"    },
	{"Start 1"           , BIT_DIGITAL   , System16InputPort0 + 4, "p1 start"   },
	{"Coin 2"            , BIT_DIGITAL   , System16InputPort0 + 1, "p2 coin"    },

	A("Steering"         , BIT_ANALOG_REL, &System16AnalogPort0,   "p1 x-axis"  ),
	A("Accelerate"       , BIT_ANALOG_REL, &System16AnalogPort1,   "p1 fire 1"  ),
	A("Brake"            , BIT_ANALOG_REL, &System16AnalogPort2,   "p1 fire 2"  ),
	{"Super Charger"     , BIT_DIGITAL   , System16InputPort0 + 5, "p1 fire 3"  },
	
	{"Service"           , BIT_DIGITAL   , System16InputPort0 + 3 , "service"   },
	{"Diagnostics"       , BIT_DIGITAL   , System16InputPort0 + 2 , "diag"      },
	{"Reset"             , BIT_DIGITAL   , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , System16Dip + 1        , "dip"       },
};

STDINPUTINFO(Shangonrb)

static struct BurnInputInfo SharrierInputList[] = {
	{"Coin 1"            , BIT_DIGITAL   , System16InputPort0 + 0, "p1 coin"    },
	{"Start 1"           , BIT_DIGITAL   , System16InputPort0 + 4, "p1 start"   },
	{"Coin 2"            , BIT_DIGITAL   , System16InputPort0 + 1, "p2 coin"    },

	A("Left/Right"       , BIT_ANALOG_REL, &System16AnalogPort0,   "p1 x-axis"  ),
	A("Up/Down"          , BIT_ANALOG_REL, &System16AnalogPort1,   "p1 y-axis"  ),
	{"Fire 1"            , BIT_DIGITAL   , System16InputPort0 + 5, "p1 fire 1"  },
	{"Fire 2"            , BIT_DIGITAL   , System16InputPort0 + 6, "p1 fire 2"  },
	{"Fire 3"            , BIT_DIGITAL   , System16InputPort0 + 7, "p1 fire 3"  },
	
	{"Service"           , BIT_DIGITAL   , System16InputPort0 + 3 , "service"   },
	{"Diagnostics"       , BIT_DIGITAL   , System16InputPort0 + 2 , "diag"      },
	{"Reset"             , BIT_DIGITAL   , &System16Reset         , "reset"     },
	{"Dip 1"             , BIT_DIPSWITCH , System16Dip + 0        , "dip"       },
	{"Dip 2"             , BIT_DIPSWITCH , System16Dip + 1        , "dip"       },
};

STDINPUTINFO(Sharrier)

#undef A

/*====================================================
Dip defs
====================================================*/

#define HANGON_COINAGE(dipval)								\
	{0   , 0xfe, 0   , 16  , "Coin A"                               },			\
	{dipval, 0x01, 0x0f, 0x07, "4 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0x0f, 0x08, "3 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0x0f, 0x09, "2 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0x0f, 0x05, "2 Coins 1 Credit 5/3 6/4"           },			\
	{dipval, 0x01, 0x0f, 0x04, "2 Coins 1 Credit 4/3"               },			\
	{dipval, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit"                   },			\
	{dipval, 0x01, 0x0f, 0x01, "1 Coin  1 Credit 2/3"               },			\
	{dipval, 0x01, 0x0f, 0x02, "1 Coin  1 Credit 4/5"               },			\
	{dipval, 0x01, 0x0f, 0x03, "1 Coin  1 Credit 5/6"               },			\
	{dipval, 0x01, 0x0f, 0x06, "2 Coins 3 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x00, "Free Play (if coin B too) or 1C/1C" },			\
												\
	{0   , 0xfe, 0   , 16  , "Coin B"                               },			\
	{dipval, 0x01, 0xf0, 0x70, "4 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0xf0, 0x80, "3 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0xf0, 0x90, "2 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0xf0, 0x50, "2 Coins 1 Credit 5/3 6/4"           },			\
	{dipval, 0x01, 0xf0, 0x40, "2 Coins 1 Credit 4/3"               },			\
	{dipval, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit"                   },			\
	{dipval, 0x01, 0xf0, 0x10, "1 Coin  1 Credit 2/3"               },			\
	{dipval, 0x01, 0xf0, 0x20, "1 Coin  1 Credit 4/5"               },			\
	{dipval, 0x01, 0xf0, 0x30, "1 Coin  1 Credit 5/6"               },			\
	{dipval, 0x01, 0xf0, 0x60, "2 Coins 3 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0x00, "Free Play (if coin A too) or 1C/1C" },

#define HANGON_COINAGE_NO_FREEPLAY(dipval)								\
	{0   , 0xfe, 0   , 15  , "Coin A"                               },			\
	{dipval, 0x01, 0x0f, 0x07, "4 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0x0f, 0x08, "3 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0x0f, 0x09, "2 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0x0f, 0x05, "2 Coins 1 Credit 5/3 6/4"           },			\
	{dipval, 0x01, 0x0f, 0x04, "2 Coins 1 Credit 4/3"               },			\
	{dipval, 0x01, 0x0f, 0x0f, "1 Coin  1 Credit"                   },			\
	{dipval, 0x01, 0x0f, 0x01, "1 Coin  1 Credit 2/3"               },			\
	{dipval, 0x01, 0x0f, 0x02, "1 Coin  1 Credit 4/5"               },			\
	{dipval, 0x01, 0x0f, 0x03, "1 Coin  1 Credit 5/6"               },			\
	{dipval, 0x01, 0x0f, 0x06, "2 Coins 3 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x0e, "1 Coin  2 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x0d, "1 Coin  3 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x0c, "1 Coin  4 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x0b, "1 Coin  5 Credits"                  },			\
	{dipval, 0x01, 0x0f, 0x0a, "1 Coin  6 Credits"                  },			\
												\
	{0   , 0xfe, 0   , 15  , "Coin B"                               },			\
	{dipval, 0x01, 0xf0, 0x70, "4 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0xf0, 0x80, "3 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0xf0, 0x90, "2 Coins 1 Credit"                   },			\
	{dipval, 0x01, 0xf0, 0x50, "2 Coins 1 Credit 5/3 6/4"           },			\
	{dipval, 0x01, 0xf0, 0x40, "2 Coins 1 Credit 4/3"               },			\
	{dipval, 0x01, 0xf0, 0xf0, "1 Coin  1 Credit"                   },			\
	{dipval, 0x01, 0xf0, 0x10, "1 Coin  1 Credit 2/3"               },			\
	{dipval, 0x01, 0xf0, 0x20, "1 Coin  1 Credit 4/5"               },			\
	{dipval, 0x01, 0xf0, 0x30, "1 Coin  1 Credit 5/6"               },			\
	{dipval, 0x01, 0xf0, 0x60, "2 Coins 3 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0xe0, "1 Coin  2 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0xd0, "1 Coin  3 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0xc0, "1 Coin  4 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0xb0, "1 Coin  5 Credits"                  },			\
	{dipval, 0x01, 0xf0, 0xa0, "1 Coin  6 Credits"                  },

static struct BurnDIPInfo EndurorDIPList[]=
{
	// Default Values
	{0x0a, 0xff, 0xff, 0xff, NULL                                 },
	{0x0b, 0xff, 0xff, 0x7e, NULL                                 },

	// Dip 1
	HANGON_COINAGE(0x0a)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Cabinet"                            },
	{0x0b, 0x01, 0x01, 0x00, "Upright"                            },
	{0x0b, 0x01, 0x01, 0x01, "Wheelie"                            },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x0b, 0x01, 0x06, 0x04, "Easy"                               },
	{0x0b, 0x01, 0x06, 0x06, "Medium"                             },
	{0x0b, 0x01, 0x06, 0x02, "Hard"                               },
	{0x0b, 0x01, 0x06, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Time Adjust"                        },
	{0x0b, 0x01, 0x18, 0x10, "Easy"                               },
	{0x0b, 0x01, 0x18, 0x18, "Medium"                             },
	{0x0b, 0x01, 0x18, 0x08, "Hard"                               },
	{0x0b, 0x01, 0x18, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Time Control"                       },
	{0x0b, 0x01, 0x60, 0x40, "Easy"                               },
	{0x0b, 0x01, 0x60, 0x60, "Medium"                             },
	{0x0b, 0x01, 0x60, 0x20, "Hard"                               },
	{0x0b, 0x01, 0x60, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x0b, 0x01, 0x80, 0x80, "Off"                                },
	{0x0b, 0x01, 0x80, 0x00, "On"                                 },
};

STDDIPINFO(Enduror)

static struct BurnDIPInfo HangonDIPList[]=
{
	// Default Values
	{0x09, 0xff, 0xff, 0xff, NULL                                 },
	{0x0a, 0xff, 0xff, 0xfe, NULL                                 },

	// Dip 1
	HANGON_COINAGE_NO_FREEPLAY(0x09)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x0a, 0x01, 0x01, 0x01, "Off"                                },
	{0x0a, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x0a, 0x01, 0x06, 0x04, "Easy"                               },
	{0x0a, 0x01, 0x06, 0x06, "Medium"                             },
	{0x0a, 0x01, 0x06, 0x02, "Hard"                               },
	{0x0a, 0x01, 0x06, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Time Adjust"                        },
	{0x0a, 0x01, 0x18, 0x18, "Normal"                             },
	{0x0a, 0x01, 0x18, 0x10, "Medium"                             },
	{0x0a, 0x01, 0x18, 0x08, "Hard"                               },
	{0x0a, 0x01, 0x18, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 2   , "Play Music"                         },
	{0x0a, 0x01, 0x20, 0x00, "Off"                                },
	{0x0a, 0x01, 0x20, 0x20, "On"                                 },
};

STDDIPINFO(Hangon)

static struct BurnDIPInfo ShangonrbDIPList[]=
{
	// Default Values
	{0x0a, 0xff, 0xff, 0xff, NULL                                 },
	{0x0b, 0xff, 0xff, 0x1e, NULL                                 },

	// Dip 1
	HANGON_COINAGE(0x0a)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x0b, 0x01, 0x01, 0x01, "Off"                                },
	{0x0b, 0x01, 0x01, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x0b, 0x01, 0x06, 0x04, "Easy"                               },
	{0x0b, 0x01, 0x06, 0x06, "Normal"                             },
	{0x0b, 0x01, 0x06, 0x02, "Hard"                               },
	{0x0b, 0x01, 0x06, 0x00, "Hardest"                            },
	
	{0   , 0xfe, 0   , 4   , "Time Adjust"                        },
	{0x0b, 0x01, 0x18, 0x10, "Easy"                               },
	{0x0b, 0x01, 0x18, 0x18, "Normal"                             },
	{0x0b, 0x01, 0x18, 0x08, "Hard"                               },
	{0x0b, 0x01, 0x18, 0x00, "Hardest"                            },
};

STDDIPINFO(Shangonrb)

static struct BurnDIPInfo SharrierDIPList[]=
{
	// Default Values
	{0x0b, 0xff, 0xff, 0xff, NULL                                 },
	{0x0c, 0xff, 0xff, 0xfc, NULL                                 },

	// Dip 1
	HANGON_COINAGE_NO_FREEPLAY(0x0b)

	// Dip 2
	{0   , 0xfe, 0   , 2   , "Cabinet"                            },
	{0x0c, 0x01, 0x01, 0x00, "Upright"                            },
	{0x0c, 0x01, 0x01, 0x01, "Moving"                             },
	
	{0   , 0xfe, 0   , 2   , "Demo Sounds"                        },
	{0x0c, 0x01, 0x02, 0x02, "Off"                                },
	{0x0c, 0x01, 0x02, 0x00, "On"                                 },
	
	{0   , 0xfe, 0   , 4   , "Lives"                              },
	{0x0c, 0x01, 0x0c, 0x08, "2"                                  },
	{0x0c, 0x01, 0x0c, 0x0c, "3"                                  },
	{0x0c, 0x01, 0x0c, 0x04, "4"                                  },
	{0x0c, 0x01, 0x0c, 0x00, "5"                                  },
	
	{0   , 0xfe, 0   , 2   , "Bonus Life"                         },
	{0x0c, 0x01, 0x10, 0x10, "5000000"                            },
	{0x0c, 0x01, 0x10, 0x00, "7000000"                            },
	
	{0   , 0xfe, 0   , 2   , "Trial Time"                         },
	{0x0c, 0x01, 0x20, 0x20, "Off"                                },
	{0x0c, 0x01, 0x20, 0x00, "On"                                 },
		
	{0   , 0xfe, 0   , 4   , "Difficulty"                         },
	{0x0c, 0x01, 0xc0, 0x80, "Easy"                               },
	{0x0c, 0x01, 0xc0, 0xc0, "Medium"                             },
	{0x0c, 0x01, 0xc0, 0x40, "Hard"                               },
	{0x0c, 0x01, 0xc0, 0x00, "Hardest"                            },
};

STDDIPINFO(Sharrier)

#undef HANGON_COINAGE

/*====================================================
Rom Defs
====================================================*/

static struct BurnRomInfo EndurorRomDesc[] = {
	{ "epr-7640a.ic97",   0x08000, 0x1d1dc5d4, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7636a.ic84",   0x08000, 0x84131639, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7641.ic98",    0x08000, 0x2503ae7c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7637.ic85",    0x08000, 0x82a27a8c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7642.ic99",    0x08000, 0x1c453bea, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7638.ic86",    0x08000, 0x70544779, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7634a.ic54",   0x08000, 0xaec83731, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-7635a.ic67",   0x08000, 0xb2fce96f, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "epr-7644.ic31",    0x08000, 0xe7a4ff90, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7645.ic46",    0x08000, 0x4caa0095, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7646.ic60",    0x08000, 0x7e432683, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-7678.ic36",    0x08000, 0x9fb5e656, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7670.ic28",    0x08000, 0xdbbe2f6e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7662.ic18",    0x08000, 0xcb0c13c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7654.ic8",     0x08000, 0x2db6520d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7677.ic35",    0x08000, 0x7764765b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7669.ic27",    0x08000, 0xf9525faa, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7661.ic17",    0x08000, 0xfe93a79b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7653.ic7",     0x08000, 0x46a52114, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7676.ic34",    0x08000, 0x2e42e0d4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7668.ic26",    0x08000, 0xe115ce33, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7660.ic16",    0x08000, 0x86dfbb68, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7652.ic6",     0x08000, 0x2880cfdb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7675.ic33",    0x08000, 0x05cd2d61, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7667.ic25",    0x08000, 0x923bde9d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7659.ic15",    0x08000, 0x629dc8ce, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7651.ic5",     0x08000, 0xd7902bad, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7674.ic32",    0x08000, 0x1a129acf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7666.ic24",    0x08000, 0x23697257, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7658.ic14",    0x08000, 0x1677f24f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7650.ic4",     0x08000, 0x642635ec, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7673.ic31",    0x08000, 0x82602394, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7665.ic23",    0x08000, 0x12d77607, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7657.ic13",    0x08000, 0x8158839c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7649.ic3",     0x08000, 0x4edba14c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7672.ic30",    0x08000, 0xd11452f7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7664.ic22",    0x08000, 0x0df2cfad, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7656.ic12",    0x08000, 0x6c741272, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7648.ic2",     0x08000, 0x983ea830, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7671.ic29",    0x08000, 0xb0c7fdc6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7663.ic21",    0x08000, 0x2b0b8f08, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7655.ic11",    0x08000, 0x3433fe7b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7647.ic1",     0x08000, 0x2e7fbec0, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-7633.ic1",     0x08000, 0x6f146210, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-7682.ic58",    0x08000, 0xc4efbf48, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7681.ic8",     0x08000, 0xbc0c4d12, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-7680.ic7",     0x08000, 0x627b3c8c, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "epr-6844.ic123",   0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
	
	{ "317-0013a.key",    0x02000, 0xa965b2da, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Enduror)
STD_ROM_FN(Enduror)

static struct BurnRomInfo EndurordRomDesc[] = {
	{ "bootleg_epr-7640a.ic97",   0x08000, 0xf52fd496, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "bootleg_epr-7636a.ic84",   0x08000, 0x666136b3, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "bootleg_epr-7641.ic98",    0x08000, 0x2153154a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "bootleg_epr-7637.ic85",    0x08000, 0x0a97992c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "bootleg_epr-7642.ic99",    0x08000, 0xf6391091, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "bootleg_epr-7638.ic86",    0x08000, 0x79b367d7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7634a.ic54",   0x08000, 0xaec83731, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-7635a.ic67",   0x08000, 0xb2fce96f, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "epr-7644.ic31",    0x08000, 0xe7a4ff90, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7645.ic46",    0x08000, 0x4caa0095, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7646.ic60",    0x08000, 0x7e432683, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-7678.ic36",    0x08000, 0x9fb5e656, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7670.ic28",    0x08000, 0xdbbe2f6e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7662.ic18",    0x08000, 0xcb0c13c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7654.ic8",     0x08000, 0x2db6520d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7677.ic35",    0x08000, 0x7764765b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7669.ic27",    0x08000, 0xf9525faa, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7661.ic17",    0x08000, 0xfe93a79b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7653.ic7",     0x08000, 0x46a52114, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7676.ic34",    0x08000, 0x2e42e0d4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7668.ic26",    0x08000, 0xe115ce33, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7660.ic16",    0x08000, 0x86dfbb68, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7652.ic6",     0x08000, 0x2880cfdb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7675.ic33",    0x08000, 0x05cd2d61, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7667.ic25",    0x08000, 0x923bde9d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7659.ic15",    0x08000, 0x629dc8ce, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7651.ic5",     0x08000, 0xd7902bad, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7674.ic32",    0x08000, 0x1a129acf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7666.ic24",    0x08000, 0x23697257, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7658.ic14",    0x08000, 0x1677f24f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7650.ic4",     0x08000, 0x642635ec, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7673.ic31",    0x08000, 0x82602394, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7665.ic23",    0x08000, 0x12d77607, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7657.ic13",    0x08000, 0x8158839c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7649.ic3",     0x08000, 0x4edba14c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7672.ic30",    0x08000, 0xd11452f7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7664.ic22",    0x08000, 0x0df2cfad, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7656.ic12",    0x08000, 0x6c741272, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7648.ic2",     0x08000, 0x983ea830, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7671.ic29",    0x08000, 0xb0c7fdc6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7663.ic21",    0x08000, 0x2b0b8f08, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7655.ic11",    0x08000, 0x3433fe7b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7647.ic1",     0x08000, 0x2e7fbec0, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-7633.ic1",     0x08000, 0x6f146210, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-7682.ic58",    0x08000, 0xc4efbf48, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7681.ic8",     0x08000, 0xbc0c4d12, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-7680.ic7",     0x08000, 0x627b3c8c, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "epr-6844.ic123",   0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
};


STD_ROM_PICK(Endurord)
STD_ROM_FN(Endurord)

static struct BurnRomInfo Enduror1RomDesc[] = {
	{ "epr-7630.ic97",    0x08000, 0xa1bdadab, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7629.ic84",    0x08000, 0xf50f4169, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7641.ic98",    0x08000, 0x2503ae7c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7637.ic85",    0x08000, 0x82a27a8c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7642.ic99",    0x08000, 0x1c453bea, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7638.ic86",    0x08000, 0x70544779, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7634.ic54",    0x08000, 0x3e07fd32, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-7635.ic67",    0x08000, 0x22f762ab, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "epr-7644.ic31",    0x08000, 0xe7a4ff90, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7645.ic46",    0x08000, 0x4caa0095, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7646.ic60",    0x08000, 0x7e432683, SYS16_ROM_TILES | BRF_GRA },

	{ "epr-7678.ic36",    0x08000, 0x9fb5e656, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7670.ic28",    0x08000, 0xdbbe2f6e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7662.ic18",    0x08000, 0xcb0c13c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7654.ic8",     0x08000, 0x2db6520d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7677.ic35",    0x08000, 0x7764765b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7669.ic27",    0x08000, 0xf9525faa, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7661.ic17",    0x08000, 0xfe93a79b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7653.ic7",     0x08000, 0x46a52114, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7676.ic34",    0x08000, 0x2e42e0d4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7668.ic26",    0x08000, 0xe115ce33, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7660.ic16",    0x08000, 0x86dfbb68, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7652.ic6",     0x08000, 0x2880cfdb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7675.ic33",    0x08000, 0x05cd2d61, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7667.ic25",    0x08000, 0x923bde9d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7659.ic15",    0x08000, 0x629dc8ce, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7651.ic5",     0x08000, 0xd7902bad, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7674.ic32",    0x08000, 0x1a129acf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7666.ic24",    0x08000, 0x23697257, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7658.ic14",    0x08000, 0x1677f24f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7650.ic4",     0x08000, 0x642635ec, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7673.ic31",    0x08000, 0x82602394, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7665.ic23",    0x08000, 0x12d77607, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7657.ic13",    0x08000, 0x8158839c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7649.ic3",     0x08000, 0x4edba14c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7672.ic30",    0x08000, 0xd11452f7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7664.ic22",    0x08000, 0x0df2cfad, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7656.ic12",    0x08000, 0x6c741272, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7648.ic2",     0x08000, 0x983ea830, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7671.ic29",    0x08000, 0xb0c7fdc6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7663.ic21",    0x08000, 0x2b0b8f08, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7655.ic11",    0x08000, 0x3433fe7b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7647.ic1",     0x08000, 0x2e7fbec0, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-7633.ic1",     0x08000, 0x6f146210, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-7765.ic73",    0x04000, 0x81c82fc9, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	{ "epr-7764.ic72",    0x04000, 0x755bfdad, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7762.ic5",     0x08000, 0xbc0c4d12, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-7763.ic6",     0x08000, 0x627b3c8c, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "epr-6844.ic123",   0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
	
	{ "317-0013a.key",    0x02000, 0xa965b2da, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Enduror1)
STD_ROM_FN(Enduror1)

static struct BurnRomInfo Enduror1dRomDesc[] = {
	{ "bootleg_epr-7630.ic97",    0x08000, 0xb041e995, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "bootleg_epr-7629.ic84",    0x08000, 0xdb4eff5f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "bootleg_epr-7641.ic98",    0x08000, 0x2153154a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "bootleg_epr-7637.ic85",    0x08000, 0x0a97992c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "bootleg_epr-7642.ic99",    0x08000, 0xf6391091, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "bootleg_epr-7638.ic86",    0x08000, 0x79b367d7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7634.ic54",    0x08000, 0x3e07fd32, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-7635.ic67",    0x08000, 0x22f762ab, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "epr-7644.ic31",    0x08000, 0xe7a4ff90, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7645.ic46",    0x08000, 0x4caa0095, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7646.ic60",    0x08000, 0x7e432683, SYS16_ROM_TILES | BRF_GRA },

	{ "epr-7678.ic36",    0x08000, 0x9fb5e656, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7670.ic28",    0x08000, 0xdbbe2f6e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7662.ic18",    0x08000, 0xcb0c13c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7654.ic8",     0x08000, 0x2db6520d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7677.ic35",    0x08000, 0x7764765b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7669.ic27",    0x08000, 0xf9525faa, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7661.ic17",    0x08000, 0xfe93a79b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7653.ic7",     0x08000, 0x46a52114, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7676.ic34",    0x08000, 0x2e42e0d4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7668.ic26",    0x08000, 0xe115ce33, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7660.ic16",    0x08000, 0x86dfbb68, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7652.ic6",     0x08000, 0x2880cfdb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7675.ic33",    0x08000, 0x05cd2d61, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7667.ic25",    0x08000, 0x923bde9d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7659.ic15",    0x08000, 0x629dc8ce, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7651.ic5",     0x08000, 0xd7902bad, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7674.ic32",    0x08000, 0x1a129acf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7666.ic24",    0x08000, 0x23697257, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7658.ic14",    0x08000, 0x1677f24f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7650.ic4",     0x08000, 0x642635ec, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7673.ic31",    0x08000, 0x82602394, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7665.ic23",    0x08000, 0x12d77607, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7657.ic13",    0x08000, 0x8158839c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7649.ic3",     0x08000, 0x4edba14c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7672.ic30",    0x08000, 0xd11452f7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7664.ic22",    0x08000, 0x0df2cfad, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7656.ic12",    0x08000, 0x6c741272, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7648.ic2",     0x08000, 0x983ea830, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7671.ic29",    0x08000, 0xb0c7fdc6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7663.ic21",    0x08000, 0x2b0b8f08, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7655.ic11",    0x08000, 0x3433fe7b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7647.ic1",     0x08000, 0x2e7fbec0, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-7633.ic1",     0x08000, 0x6f146210, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-7765.ic73",    0x04000, 0x81c82fc9, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	{ "epr-7764.ic72",    0x04000, 0x755bfdad, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7762.ic5",     0x08000, 0xbc0c4d12, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-7763.ic6",     0x08000, 0x627b3c8c, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "epr-6844.ic123",   0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
};


STD_ROM_PICK(Enduror1d)
STD_ROM_FN(Enduror1d)

static struct BurnRomInfo EnduroraRomDesc[] = {
	{ "epr-7640a.ic97",   0x08000, 0x1d1dc5d4, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7636a.ic84",   0x08000, 0x84131639, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7641.ic98",    0x08000, 0x2503ae7c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7637.ic85",    0x08000, 0x82a27a8c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7642.ic99",    0x08000, 0x1c453bea, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7638.ic86",    0x08000, 0x70544779, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7634a.ic54",   0x08000, 0xaec83731, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-7635a.ic67",   0x08000, 0xb2fce96f, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "epr-7644.ic31",    0x08000, 0xe7a4ff90, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7645.ic46",    0x08000, 0x4caa0095, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7646.ic60",    0x08000, 0x7e432683, SYS16_ROM_TILES | BRF_GRA },
	
	// sprites - ASSY ROM BD 837-6004-02 with jumpers J2, J3, J7 & J8 made
	{ "mpr-10146.ic36",   0x20000, 0x85564401, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10144.ic28",   0x20000, 0x03569803, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10142.ic18",   0x20000, 0x4a72251b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10140.ic8",    0x20000, 0x68ff1691, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10145.ic32",   0x20000, 0x3e64eec0, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10143.ic24",   0x20000, 0xbdad5fd2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10141.ic14",   0x20000, 0x560360b9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "mpr-10139.ic4",    0x20000, 0x863c7d9e, SYS16_ROM_SPRITES | BRF_GRA },
		
	{ "epr-7633.ic1",     0x08000, 0x6f146210, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-7682.ic58",    0x08000, 0xc4efbf48, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7681.ic8",     0x08000, 0xbc0c4d12, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-7680.ic7",     0x08000, 0x627b3c8c, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "epr-6844.ic123",   0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
	
	{ "317-0013a.key",    0x02000, 0xa965b2da, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Endurora)
STD_ROM_FN(Endurora)

static struct BurnRomInfo EndurorbRomDesc[] = {
	{ "epr-7640.ic97",    0x08000, 0x193a495b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7636.ic84",    0x08000, 0xd8cedbbe, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7641.ic98",    0x08000, 0x2503ae7c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7637.ic85",    0x08000, 0x82a27a8c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7642.ic99",    0x08000, 0x1c453bea, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7638.ic86",    0x08000, 0x70544779, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7634.ic54",    0x08000, 0x3e07fd32, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-7635.ic67",    0x08000, 0x22f762ab, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "epr-7644.ic31",    0x08000, 0xe7a4ff90, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7645.ic46",    0x08000, 0x4caa0095, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7646.ic60",    0x08000, 0x7e432683, SYS16_ROM_TILES | BRF_GRA },

	{ "epr-7678.ic36",    0x08000, 0x9fb5e656, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7670.ic28",    0x08000, 0xdbbe2f6e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7662.ic18",    0x08000, 0xcb0c13c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7654.ic8",     0x08000, 0x2db6520d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7677.ic35",    0x08000, 0x7764765b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7669.ic27",    0x08000, 0xf9525faa, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7661.ic17",    0x08000, 0xfe93a79b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7653.ic7",     0x08000, 0x46a52114, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7676.ic34",    0x08000, 0x2e42e0d4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7668.ic26",    0x08000, 0xe115ce33, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7660.ic16",    0x08000, 0x86dfbb68, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7652.ic6",     0x08000, 0x2880cfdb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7675.ic33",    0x08000, 0x05cd2d61, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7667.ic25",    0x08000, 0x923bde9d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7659.ic15",    0x08000, 0x629dc8ce, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7651.ic5",     0x08000, 0xd7902bad, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7674.ic32",    0x08000, 0x1a129acf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7666.ic24",    0x08000, 0x23697257, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7658.ic14",    0x08000, 0x1677f24f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7650.ic4",     0x08000, 0x642635ec, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7673.ic31",    0x08000, 0x82602394, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7665.ic23",    0x08000, 0x12d77607, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7657.ic13",    0x08000, 0x8158839c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7649.ic3",     0x08000, 0x4edba14c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7672.ic30",    0x08000, 0xd11452f7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7664.ic22",    0x08000, 0x0df2cfad, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7656.ic12",    0x08000, 0x6c741272, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7648.ic2",     0x08000, 0x983ea830, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7671.ic29",    0x08000, 0xb0c7fdc6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7663.ic21",    0x08000, 0x2b0b8f08, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7655.ic11",    0x08000, 0x3433fe7b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7647.ic1",     0x08000, 0x2e7fbec0, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-7633.ic1",     0x08000, 0x6f146210, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-7682.ic58",    0x08000, 0xc4efbf48, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7681.ic8",     0x08000, 0xbc0c4d12, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-7680.ic7",     0x08000, 0x627b3c8c, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "epr-6844.ic123",   0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
	
	{ "317-0013a.key",    0x02000, 0xa965b2da, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Endurorb)
STD_ROM_FN(Endurorb)

static struct BurnRomInfo EnduroblRomDesc[] = {
	{ "7.13j",            0x10000, 0xf1d6b4b7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "4.13h",            0x10000, 0x43bff873, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "8.14j",            0x08000, 0x2153154a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "5.14h",            0x08000, 0x0a97992c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "9.15j",            0x08000, 0xdb3bff1c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "6.15h",            0x08000, 0x54b1885a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7634.ic54",    0x08000, 0x3e07fd32, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-7635.ic67",    0x08000, 0x22f762ab, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "epr-7644.ic31",    0x08000, 0xe7a4ff90, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7645.ic46",    0x08000, 0x4caa0095, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7646.ic60",    0x08000, 0x7e432683, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-7678.ic36",    0x08000, 0x9fb5e656, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7670.ic28",    0x08000, 0xdbbe2f6e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7662.ic18",    0x08000, 0xcb0c13c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7654.ic8",     0x08000, 0x2db6520d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7677.ic35",    0x08000, 0x7764765b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7669.ic27",    0x08000, 0xf9525faa, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7661.ic17",    0x08000, 0xfe93a79b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7653.ic7",     0x08000, 0x46a52114, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7676.ic34",    0x08000, 0x2e42e0d4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7668.ic26",    0x08000, 0xe115ce33, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7660.ic16",    0x08000, 0x86dfbb68, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7652.ic6",     0x08000, 0x2880cfdb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7675.ic33",    0x08000, 0x05cd2d61, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7667.ic25",    0x08000, 0x923bde9d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7659.ic15",    0x08000, 0x629dc8ce, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7651.ic5",     0x08000, 0xd7902bad, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7674.ic32",    0x08000, 0x1a129acf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7666.ic24",    0x08000, 0x23697257, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7658.ic14",    0x08000, 0x1677f24f, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7650.ic4",     0x08000, 0x642635ec, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7673.ic31",    0x08000, 0x82602394, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7665.ic23",    0x08000, 0x12d77607, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7657.ic13",    0x08000, 0x8158839c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7649.ic3",     0x08000, 0x4edba14c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7672.ic30",    0x08000, 0xd11452f7, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7664.ic22",    0x08000, 0x0df2cfad, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7656.ic12",    0x08000, 0x6c741272, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7648.ic2",     0x08000, 0x983ea830, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7671.ic29",    0x08000, 0xb0c7fdc6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7663.ic21",    0x08000, 0x2b0b8f08, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7655.ic11",    0x08000, 0x3433fe7b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7647.ic1",     0x08000, 0x2e7fbec0, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-7633.ic1",     0x08000, 0x6f146210, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-7765.ic73",    0x04000, 0x81c82fc9, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	{ "epr-7764.ic72",    0x04000, 0x755bfdad, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7762.ic5",     0x08000, 0xbc0c4d12, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-7763.ic6",     0x08000, 0x627b3c8c, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "epr-6844.ic123",   0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
};


STD_ROM_PICK(Endurobl)
STD_ROM_FN(Endurobl)

static struct BurnRomInfo HangonRomDesc[] = {
	{ "epr-6918a.ic22",   0x08000, 0x20b1c2b0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-6916a.ic8",    0x08000, 0x7d9db1bf, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-6917a.ic20",   0x08000, 0xfea12367, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-6915a.ic6",    0x08000, 0xac883240, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-6920.ic63",    0x08000, 0x1c95013e, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-6919.ic51",    0x08000, 0x6ca30d69, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "epr-6841.ic38",    0x08000, 0x54d295dc, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-6842.ic23",    0x08000, 0xf677b568, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-6843.ic7",     0x08000, 0xa257f0da, SYS16_ROM_TILES | BRF_GRA },

	{ "epr-6819.ic27",    0x08000, 0x469dad07, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6820.ic34",    0x08000, 0x87cbc6de, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6821.ic28",    0x08000, 0x15792969, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6822.ic35",    0x08000, 0xe9718de5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6823.ic29",    0x08000, 0x49422691, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6824.ic36",    0x08000, 0x701deaa4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6825.ic30",    0x08000, 0x6e23c8b4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6826.ic37",    0x08000, 0x77d0de2c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6827.ic31",    0x08000, 0x7fa1bfb6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6828.ic38",    0x08000, 0x8e880c93, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6829.ic32",    0x08000, 0x7ca0952d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6830.ic39",    0x08000, 0xb1a63aef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6845.ic18",    0x08000, 0xba08c9b8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6846.ic25",    0x08000, 0xf21e57a3, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-6840.ic108",   0x08000, 0x581230e3, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-6833.ic73",    0x04000, 0x3b942f5f, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-6831.ic5",     0x08000, 0xcfef5481, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-6832.ic6",     0x08000, 0x4165aea5, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "epr-6844.ic123",   0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
	
	{ "315-5118.bin",     0x0008f, 0x51d448a2, BRF_OPT },
	{ "315-5119.bin",     0x0008f, 0xa37f00e1, BRF_OPT },
	{ "315-5120.bin",     0x0008f, 0xba5f92ec, BRF_OPT },
};


STD_ROM_PICK(Hangon)
STD_ROM_FN(Hangon)

static struct BurnRomInfo Hangon1RomDesc[] = {
	{ "epr-6918.ic22",    0x08000, 0x0bf4f2ac, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-6916.ic8",     0x08000, 0x06c21c8a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-6917.ic20",    0x08000, 0xf48a6cbc, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-6915.ic6",     0x08000, 0x75d3b5ee, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-6920.ic63",    0x08000, 0x1c95013e, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-6919.ic51",    0x08000, 0x6ca30d69, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "epr-6841.ic38",    0x08000, 0x54d295dc, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-6842.ic23",    0x08000, 0xf677b568, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-6843.ic7",     0x08000, 0xa257f0da, SYS16_ROM_TILES | BRF_GRA },

	{ "epr-6819.ic27",    0x08000, 0x469dad07, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6820.ic34",    0x08000, 0x87cbc6de, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6821.ic28",    0x08000, 0x15792969, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6822.ic35",    0x08000, 0xe9718de5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6823.ic29",    0x08000, 0x49422691, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6824.ic36",    0x08000, 0x701deaa4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6825.ic30",    0x08000, 0x6e23c8b4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6826.ic37",    0x08000, 0x77d0de2c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6827.ic31",    0x08000, 0x7fa1bfb6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6828.ic38",    0x08000, 0x8e880c93, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6829.ic32",    0x08000, 0x7ca0952d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6830.ic39",    0x08000, 0xb1a63aef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6845.ic18",    0x08000, 0xba08c9b8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6846.ic25",    0x08000, 0xf21e57a3, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-6840.ic108",   0x08000, 0x581230e3, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-6833.ic73",    0x04000, 0x3b942f5f, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-6831.ic5",     0x08000, 0xcfef5481, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-6832.ic6",     0x08000, 0x4165aea5, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "epr-6844.ic123",   0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
	
	{ "315-5118.bin",     0x0008f, 0x51d448a2, BRF_OPT },
	{ "315-5119.bin",     0x0008f, 0xa37f00e1, BRF_OPT },
	{ "315-5120.bin",     0x0008f, 0xba5f92ec, BRF_OPT },
};


STD_ROM_PICK(Hangon1)
STD_ROM_FN(Hangon1)

static struct BurnRomInfo Hangon2RomDesc[] = {
	{ "epr-6851a.ic22",   0x08000, 0x1e4d2217, SYS16_ROM_PROG | BRF_ESS | BRF_PRG }, // as per the manual
	{ "epr-6849a.ic8",    0x08000, 0x3793e50e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG }, // as per the manual
	{ "epr-6850a.ic20",   0x08000, 0x5d715e3b, SYS16_ROM_PROG | BRF_ESS | BRF_PRG }, // as per the manual
	{ "epr-6848a.ic6",    0x08000, 0xf1439a30, SYS16_ROM_PROG | BRF_ESS | BRF_PRG }, // as per the manual
	
	{ "epr-6839.ic63",    0x08000, 0x2747b794, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-6838.ic51",    0x08000, 0x73e9fa6e, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "epr-6841.ic38",    0x08000, 0x54d295dc, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-6842.ic23",    0x08000, 0xf677b568, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-6843.ic7",     0x08000, 0xa257f0da, SYS16_ROM_TILES | BRF_GRA },

	{ "epr-6819.ic27",    0x08000, 0x469dad07, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6820.ic34",    0x08000, 0x87cbc6de, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6821.ic28",    0x08000, 0x15792969, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6822.ic35",    0x08000, 0xe9718de5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6823.ic29",    0x08000, 0x49422691, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6824.ic36",    0x08000, 0x701deaa4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6825.ic30",    0x08000, 0x6e23c8b4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6826.ic37",    0x08000, 0x77d0de2c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6827.ic31",    0x08000, 0x7fa1bfb6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6828.ic38",    0x08000, 0x8e880c93, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6829.ic32",    0x08000, 0x7ca0952d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6830.ic39",    0x08000, 0xb1a63aef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6845.ic18",    0x08000, 0xba08c9b8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-6846.ic25",    0x08000, 0xf21e57a3, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-6840.ic108",   0x08000, 0x581230e3, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-6833.ic73",    0x04000, 0x3b942f5f, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-6831.ic5",     0x08000, 0xcfef5481, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-6832.ic6",     0x08000, 0x4165aea5, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "epr-6844.ic119",   0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
	
	{ "315-5118.bin",     0x0008f, 0x51d448a2, BRF_OPT },
	{ "315-5119.bin",     0x0008f, 0xa37f00e1, BRF_OPT },
	{ "315-5120.bin",     0x0008f, 0xba5f92ec, BRF_OPT },
};


STD_ROM_PICK(Hangon2)
STD_ROM_FN(Hangon2)

static struct BurnRomInfo HangonvfRomDesc[] = {
	{ "9.3n",   		  0x08000, 0x20b1c2b0, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "11.1n",    		  0x08000, 0x7d9db1bf, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "8.3k",   		  0x08000, 0xfea12367, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "10.1k",    		  0x08000, 0xac883240, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "6.6l",    		  0x08000, 0x1c95013e, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "7.5l",    		  0x08000, 0x6ca30d69, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "2.3j",    		  0x08000, 0x255a3a58, SYS16_ROM_TILES | BRF_GRA },
	{ "3.2j",    		  0x08000, 0x88b9ffd9, SYS16_ROM_TILES | BRF_GRA },
	{ "4.1j",     		  0x08000, 0x18882328, SYS16_ROM_TILES | BRF_GRA },

	{ "19.5b",    		  0x08000, 0x469dad07, SYS16_ROM_SPRITES | BRF_GRA },
	{ "25.6b",    		  0x08000, 0x87cbc6de, SYS16_ROM_SPRITES | BRF_GRA },
	{ "18.5c",    		  0x08000, 0x15792969, SYS16_ROM_SPRITES | BRF_GRA },
	{ "24.6c",    		  0x08000, 0xe9718de5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "17.5d",    		  0x08000, 0x49422691, SYS16_ROM_SPRITES | BRF_GRA },
	{ "23.6d",    		  0x08000, 0x701deaa4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "16.5e",    		  0x08000, 0xf003a000, SYS16_ROM_SPRITES | BRF_GRA },
	{ "22.6e",    		  0x08000, 0x08b007e2, SYS16_ROM_SPRITES | BRF_GRA },
	{ "15.5f",    		  0x08000, 0x7fa1bfb6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "21.6f",    		  0x08000, 0x8e880c93, SYS16_ROM_SPRITES | BRF_GRA },
	{ "14.5h",    		  0x08000, 0x47e63dd1, SYS16_ROM_SPRITES | BRF_GRA },
	{ "20.6h",    		  0x08000, 0x33d1aa6e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "12.3f",    		  0x08000, 0x22fc088e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "13.4f",    		  0x08000, 0x032738ba, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "5.9r",   		  0x08000, 0x581230e3, SYS16_ROM_ROAD | BRF_GRA },

	{ "28.12h",    		  0x04000, 0x3b942f5f, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "26.1e",     		  0x08000, 0xcfef5481, SYS16_ROM_PCMDATA | BRF_SND },
	{ "27.1g",     		  0x08000, 0x4165aea5, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "1.9d",   		  0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
	
	{ "a_pal16r4a.9e",    0x00104, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "b_pls153an.9f",    0x000eb, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "c_pal16r6a.7m",    0x00104, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "d_pal16r6a.7r",    0x00104, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "e_pal16r6a.7s",    0x00104, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "f_pls153n.6s",     0x000eb, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "g_pal16l8a.3r",    0x00104, 0x00000000, BRF_OPT | BRF_NODUMP },
	// no PLD marked 'H'
	{ "i_pls153n_snd.6f", 0x000eb, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "j_pal16l8a_db.bin",0x00104, 0x00000000, BRF_OPT | BRF_NODUMP },
	{ "k_pal16r4a_db.bin",0x00104, 0x00000000, BRF_OPT | BRF_NODUMP },
};


STD_ROM_PICK(Hangonvf)
STD_ROM_FN(Hangonvf)

static struct BurnRomInfo ShangonroRomDesc[] = {
	{ "epr-10842.22",     0x08000, 0x24289138, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10839.8",      0x08000, 0x70f92d5e, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10841.20",     0x08000, 0x3bb2186c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10838.6",      0x08000, 0x6aded05a, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10840.18",     0x08000, 0x12ee8716, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10837.4",      0x08000, 0x155e0cfd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10831.25",     0x10000, 0x3a2de9eb, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10833.31",     0x10000, 0x13ba98bc, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10830.24",     0x10000, 0x2ae4e53a, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },	
	{ "epr-10832.30",     0x10000, 0x543cd7bb, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },	

	{ "epr-10652.38",     0x08000, 0x260286f9, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10651.23",     0x08000, 0xc609ee7b, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10650.7",      0x08000, 0xb236a403, SYS16_ROM_TILES | BRF_GRA },

	{ "epr-10675.22",     0x10000, 0xd6ac012b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10682.13",     0x10000, 0xd9d83250, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10676.21",     0x10000, 0x25ebf2c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10683.12",     0x10000, 0x6365d2e9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10677.20",     0x10000, 0x8a57b8d6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10684.11",     0x10000, 0x3aff8910, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10678.19",     0x10000, 0xaf473098, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10685.10",     0x10000, 0x80bafeef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10679.18",     0x10000, 0x03bc4878, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10686.9",      0x10000, 0x274b734e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10680.17",     0x10000, 0x9f0677ed, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10687.8",      0x10000, 0x508a4701, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10681.16",     0x10000, 0xb176ea72, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10688.7",      0x10000, 0x42fcd51d, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-10866.108",    0x08000, 0x1bbe4fc8, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-10834a.52",    0x08000, 0x83347dc0, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10835.55",     0x10000, 0xda08ca2b, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-10836.56",     0x10000, 0x8b10e601, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "epr-6844.119",     0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
	
	{ "317-0038.key",     0x02000, 0x85943925, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Shangonro)
STD_ROM_FN(Shangonro)

static struct BurnRomInfo ShangonhoRomDesc[] = {
	{ "epr-10865.ic22",   0x08000, 0x98e861dd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10862.ic8",    0x08000, 0xd6f058c7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10864.ic20",   0x08000, 0xb3048f44, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10861.ic6",    0x08000, 0x0a131e14, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10863.ic18",   0x08000, 0x12ee8716, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-10860.ic4",    0x08000, 0x155e0cfd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10857.ic25",   0x10000, 0x064827a3, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10859.ic31",   0x10000, 0xa22bc1a2, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-10856.ic24",   0x10000, 0x000ad595, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },	
	{ "epr-10858.ic30",   0x10000, 0x8f8f4af0, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },	

	{ "epr-10652.38",     0x08000, 0x260286f9, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10651.23",     0x08000, 0xc609ee7b, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10650.7",      0x08000, 0xb236a403, SYS16_ROM_TILES | BRF_GRA },

	{ "epr-10675.22",     0x10000, 0xd6ac012b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10682.13",     0x10000, 0xd9d83250, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10676.21",     0x10000, 0x25ebf2c5, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10683.12",     0x10000, 0x6365d2e9, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10677.20",     0x10000, 0x8a57b8d6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10684.11",     0x10000, 0x3aff8910, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10678.19",     0x10000, 0xaf473098, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10685.10",     0x10000, 0x80bafeef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10679.18",     0x10000, 0x03bc4878, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10686.9",      0x10000, 0x274b734e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10680.17",     0x10000, 0x9f0677ed, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10687.8",      0x10000, 0x508a4701, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10681.16",     0x10000, 0xb176ea72, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10688.7",      0x10000, 0x42fcd51d, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-10866.108",    0x08000, 0x1bbe4fc8, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-10834a.52",    0x08000, 0x83347dc0, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10835.55",     0x10000, 0xda08ca2b, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-10836.56",     0x10000, 0x8b10e601, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "epr-6844.119",     0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
	
	{ "317-0039.key",     0x02000, 0x97b05dd6, SYS16_ROM_KEY | BRF_ESS | BRF_PRG },
};


STD_ROM_PICK(Shangonho)
STD_ROM_FN(Shangonho)

static struct BurnRomInfo ShangonrbRomDesc[] = {
	{ "s-hangon.30",      0x10000, 0xd95e82fc, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "s-hangon.32",      0x10000, 0x2ee4b4fb, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "s-hangon.29",      0x08000, 0x12ee8716, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "s-hangon.31",      0x08000, 0x155e0cfd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "s-hangon.09",      0x10000, 0x070c8059, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "s-hangon.05",      0x10000, 0x9916c54b, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "s-hangon.08",      0x10000, 0x000ad595, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "s-hangon.04",      0x10000, 0x8f8f4af0, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "epr-10652.38",     0x08000, 0x260286f9, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10651.23",     0x08000, 0xc609ee7b, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-10650.7",      0x08000, 0xb236a403, SYS16_ROM_TILES | BRF_GRA },

	{ "epr-10675.22",     0x10000, 0xd6ac012b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10682.11",     0x10000, 0xd9d83250, SYS16_ROM_SPRITES | BRF_GRA },
	{ "s-hangon.20",      0x10000, 0xeef23b3d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "s-hangon.14",      0x10000, 0x0f26d131, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10677.20",     0x10000, 0x8a57b8d6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10684.11",     0x10000, 0x3aff8910, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10678.19",     0x10000, 0xaf473098, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10685.10",     0x10000, 0x80bafeef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10679.18",     0x10000, 0x03bc4878, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10686.9",      0x10000, 0x274b734e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10680.17",     0x10000, 0x9f0677ed, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10687.8",      0x10000, 0x508a4701, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10681.16",     0x10000, 0xb176ea72, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-10688.7",      0x10000, 0x42fcd51d, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-10866.108",    0x08000, 0x1bbe4fc8, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-10834a.52",    0x08000, 0x83347dc0, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-10835.55",     0x10000, 0xda08ca2b, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-10836.56",     0x10000, 0x8b10e601, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "epr-6844.119",     0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
};


STD_ROM_PICK(Shangonrb)
STD_ROM_FN(Shangonrb)

static struct BurnRomInfo Shangonrb2RomDesc[] = {
	{ "sho-philco-s-30-r9-10.bin",      0x10000, 0xeccf7004, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "sho-philco-s-32-r12-13.bin",     0x10000, 0x90613f42, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "sho-philco-s-29-l9-10.bin",      0x08000, 0x12ee8716, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "sho-philco-s-31-l12-13.bin",     0x08000, 0x155e0cfd, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	
	{ "sho-philco-s-9-h4.bin",      	0x10000, 0x070c8059, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "sho-philco-s-5-g4.bin",      	0x10000, 0x9916c54b, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "sho-philco-s-8-h3.bin",      	0x10000, 0x000ad595, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "sho-philco-s-4-g3.bin",      	0x10000, 0x8f8f4af0, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "sho-philco-s-23-n-p8.bin",     	0x08000, 0x260286f9, SYS16_ROM_TILES | BRF_GRA },
	{ "sho-philco-s-24-n-p9.bin",     	0x08000, 0xc609ee7b, SYS16_ROM_TILES | BRF_GRA },
	{ "sho-philco-s-25-n-p10.bin",      0x08000, 0xb236a403, SYS16_ROM_TILES | BRF_GRA },

	{ "sho-philco-s-21-m6.bin",     	0x10000, 0xd6ac012b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "sho-philco-s-15-l6.bin",     	0x10000, 0xd9d83250, SYS16_ROM_SPRITES | BRF_GRA },
	{ "sho-philco-s-20-m5.bin",      	0x10000, 0xeef23b3d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "sho-philco-s-14-l5.bin",      	0x10000, 0x0f26d131, SYS16_ROM_SPRITES | BRF_GRA },
	{ "sho-philco-s-19-m4.bin",     	0x10000, 0x8a57b8d6, SYS16_ROM_SPRITES | BRF_GRA },
	{ "sho-philco-s-13-l4.bin",     	0x10000, 0x3aff8910, SYS16_ROM_SPRITES | BRF_GRA },
	{ "sho-philco-s-18-m3.bin",     	0x10000, 0xaf473098, SYS16_ROM_SPRITES | BRF_GRA },
	{ "sho-philco-s-12-l3.bin",     	0x10000, 0x80bafeef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "sho-philco-s-17-m2.bin",			0x10000, 0x03bc4878, SYS16_ROM_SPRITES | BRF_GRA },
	{ "sho-philco-s-11-l2.bin",     	0x10000, 0x274b734e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "sho-philco-s-16-m1.bin",     	0x10000, 0x9f0677ed, SYS16_ROM_SPRITES | BRF_GRA },
	{ "sho-philco-s-10-l1.bin",     	0x10000, 0x508a4701, SYS16_ROM_SPRITES | BRF_GRA },
	{ "sho-philco-s-7-h2.bin",     		0x10000, 0xb176ea72, SYS16_ROM_SPRITES | BRF_GRA },
	{ "sho-philco-s-6-h1.bin",      	0x10000, 0x42fcd51d, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "sho-philco-s-26-t1.bin",    		0x08000, 0x1bbe4fc8, SYS16_ROM_ROAD | BRF_GRA },

	{ "sho-philco-s-3-g12.bin",    		0x08000, 0x83347dc0, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "sho-philco-s-2-a16.bin",     	0x10000, 0xda08ca2b, SYS16_ROM_PCMDATA | BRF_SND },
	{ "sho-philco-s-1-a14.bin",     	0x10000, 0x8b10e601, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "sho-philco-s-22-c-d2.bin",     	0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
};


STD_ROM_PICK(Shangonrb2)
STD_ROM_FN(Shangonrb2)

static struct BurnRomInfo SharrierRomDesc[] = {
	{ "epr-7188a.ic97",   0x08000, 0x45e173c3, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7184a.ic84",   0x08000, 0xe1934a51, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7189.ic98",    0x08000, 0x40b1309f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7185.ic85",    0x08000, 0xce78045c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7190.ic99",    0x08000, 0xf6391091, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7186.ic86",    0x08000, 0x79b367d7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7191.ic100",   0x08000, 0x6171e9d3, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7187.ic87",    0x08000, 0x70cb72ef, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
		
	{ "epr-7182.ic54",    0x08000, 0xd7c535b6, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-7183.ic67",    0x08000, 0xa6153af8, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },

	{ "epr-7196.ic31",    0x08000, 0x347fa325, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7197.ic46",    0x08000, 0x39d98bd1, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7198.ic60",    0x08000, 0x3da3ea6b, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-7230.ic36",    0x08000, 0x93e2d264, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7222.ic28",    0x08000, 0xedbf5fc3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7214.ic18",    0x08000, 0xe8c537d8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7206.ic8",     0x08000, 0x22844fa4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7229.ic35",    0x08000, 0xcd6e7500, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7221.ic27",    0x08000, 0x41f25a9c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7213.ic17",    0x08000, 0x5bb09a67, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7205.ic7",     0x08000, 0xdcaa2ebf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7228.ic34",    0x08000, 0xd5e15e66, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7220.ic26",    0x08000, 0xac62ae2e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7212.ic16",    0x08000, 0x9c782295, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7204.ic6",     0x08000, 0x3711105c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7227.ic33",    0x08000, 0x60d7c1bb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7219.ic25",    0x08000, 0xf6330038, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7211.ic15",    0x08000, 0x60737b98, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7203.ic5",     0x08000, 0x70fb5ebb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7226.ic32",    0x08000, 0x6d7b5c97, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7218.ic24",    0x08000, 0xcebf797c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7210.ic14",    0x08000, 0x24596a8b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7202.ic4",     0x08000, 0xb537d082, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7225.ic31",    0x08000, 0x5e784271, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7217.ic23",    0x08000, 0x510e5e10, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7209.ic13",    0x08000, 0x7a2dad15, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7201.ic3",     0x08000, 0xf5ba4e08, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7224.ic30",    0x08000, 0xec42c9ef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7216.ic22",    0x08000, 0x6d4a7d7a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7208.ic12",    0x08000, 0x0f732717, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7200.ic2",     0x08000, 0xfc3bf8f3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7223.ic29",    0x08000, 0xed51fdc4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7215.ic21",    0x08000, 0xdfe75f3d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7207.ic11",    0x08000, 0xa2c07741, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7199.ic1",     0x08000, 0xb191e22f, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-7181.ic2",     0x08000, 0xb4740419, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-7234.ic73",    0x04000, 0xd6397933, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	{ "epr-7233.ic72",    0x04000, 0x504e76d9, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7231.ic5",     0x08000, 0x871c6b14, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-7232.ic6",     0x08000, 0x4b59340c, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "epr-6844.ic123",   0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
	
	{ "315-5163a.ic32",   0x01000, 0x203dffeb, SYS16_ROM_I8751 | BRF_PRG | BRF_ESS },
};


STD_ROM_PICK(Sharrier)
STD_ROM_FN(Sharrier)

static struct BurnRomInfo Sharrier1RomDesc[] = {
	{ "epr-7188.ic97",    0x08000, 0x7c30a036, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7184.ic84",    0x08000, 0x16deaeb1, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7189.ic98",    0x08000, 0x40b1309f, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7185.ic85",    0x08000, 0xce78045c, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7190.ic99",    0x08000, 0xf6391091, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7186.ic86",    0x08000, 0x79b367d7, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7191.ic100",   0x08000, 0x6171e9d3, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
	{ "epr-7187.ic87",    0x08000, 0x70cb72ef, SYS16_ROM_PROG | BRF_ESS | BRF_PRG },
		
	{ "epr-7182.ic54",    0x08000, 0xd7c535b6, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	{ "epr-7183.ic67",    0x08000, 0xa6153af8, SYS16_ROM_PROG2 | BRF_ESS | BRF_PRG },
	
	{ "epr-7196.ic31",    0x08000, 0x347fa325, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7197.ic46",    0x08000, 0x39d98bd1, SYS16_ROM_TILES | BRF_GRA },
	{ "epr-7198.ic60",    0x08000, 0x3da3ea6b, SYS16_ROM_TILES | BRF_GRA },
	
	{ "epr-7230.ic36",    0x08000, 0x93e2d264, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7222.ic28",    0x08000, 0xedbf5fc3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7214.ic18",    0x08000, 0xe8c537d8, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7206.ic8",     0x08000, 0x22844fa4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7229.ic35",    0x08000, 0xcd6e7500, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7221.ic27",    0x08000, 0x41f25a9c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7213.ic17",    0x08000, 0x5bb09a67, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7205.ic7",     0x08000, 0xdcaa2ebf, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7228.ic34",    0x08000, 0xd5e15e66, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7220.ic26",    0x08000, 0xac62ae2e, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7212.ic16",    0x08000, 0x9c782295, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7204.ic6",     0x08000, 0x3711105c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7227.ic33",    0x08000, 0x60d7c1bb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7219.ic25",    0x08000, 0xf6330038, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7211.ic15",    0x08000, 0x60737b98, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7203.ic5",     0x08000, 0x70fb5ebb, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7226.ic32",    0x08000, 0x6d7b5c97, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7218.ic24",    0x08000, 0xcebf797c, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7210.ic14",    0x08000, 0x24596a8b, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7202.ic4",     0x08000, 0xb537d082, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7225.ic31",    0x08000, 0x5e784271, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7217.ic23",    0x08000, 0x510e5e10, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7209.ic13",    0x08000, 0x7a2dad15, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7201.ic3",     0x08000, 0xf5ba4e08, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7224.ic30",    0x08000, 0xec42c9ef, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7216.ic22",    0x08000, 0x6d4a7d7a, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7208.ic12",    0x08000, 0x0f732717, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7200.ic2",     0x08000, 0xfc3bf8f3, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7223.ic29",    0x08000, 0xed51fdc4, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7215.ic21",    0x08000, 0xdfe75f3d, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7207.ic11",    0x08000, 0xa2c07741, SYS16_ROM_SPRITES | BRF_GRA },
	{ "epr-7199.ic1",     0x08000, 0xb191e22f, SYS16_ROM_SPRITES | BRF_GRA },
	
	{ "epr-7181.ic2",     0x08000, 0xb4740419, SYS16_ROM_ROAD | BRF_GRA },

	{ "epr-7234.ic73",    0x04000, 0xd6397933, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	{ "epr-7233.ic72",    0x04000, 0x504e76d9, SYS16_ROM_Z80PROG | BRF_ESS | BRF_PRG },
	
	{ "epr-7231.ic5",     0x08000, 0x871c6b14, SYS16_ROM_PCMDATA | BRF_SND },
	{ "epr-7232.ic6",     0x08000, 0x4b59340c, SYS16_ROM_PCMDATA | BRF_SND },
	
	{ "epr-6844.ic123",   0x02000, 0xe3ec7bd6, SYS16_ROM_PROM | BRF_GRA },
	
	{ "315-5163.ic32",    0x01000, 0x52b0c81a, SYS16_ROM_I8751 | BRF_PRG | BRF_ESS },
};


STD_ROM_PICK(Sharrier1)
STD_ROM_FN(Sharrier1)

/*====================================================
Memory Handlers
====================================================*/

static INT32 dontrecurse = 0;

// we must sync audiocpu before all ppi #0's reads/writes -dink
static void sys16_sync_audiocpu()
{
	ZetCPUPush(0);
	INT32 todo = ((double)SekTotalCycles(0) * 4000000 / System16ClockSpeed);
	if (todo > 0) {
		BurnTimerUpdate(todo);
	}
	ZetCPUPop();
}

void HangonPPI0WritePortA(UINT8 data)
{
	System16SoundLatch = data & 0xff;
}

void HangonPPI0WritePortB(UINT8 data)
{
	System16VideoEnable = data & 0x10;
	System16SpriteShadow = ~data & 0x40;
	System16ScreenFlip = data & 0x80;

	ZetSetRESETLine(0, (~data & 0x20) ? 1 : 0);
}

void HangonPPI0WritePortC(UINT8 data)
{
	System16ColScroll = ~data & 0x04;
	System16RowScroll = ~data & 0x02;
	System16SoundMute = ~data & 0x01;

	ZetSetIRQLine(0, 0x20, (data & 0x80) ? CPU_IRQSTATUS_NONE : CPU_IRQSTATUS_ACK);
}

UINT8 HangonPPI1ReadPortC()
{
	return 0x00;
}

void HangonPPI1WritePortA(UINT8 data)
{
	System16AnalogSelect = (data >> 2) & 3;

	if (~data & 0x40) {
		SekSetIRQLine(1, 4, CPU_IRQSTATUS_AUTO);
	}

	SekSetRESETLine(1, (data & 0x20) ? 1 : 0);
}

UINT16 __fastcall HangonReadWord(UINT32 a)
{
	switch (a) {	
		case 0xe00000:
		case 0xe00002:
		case 0xe00004:
		case 0xe00006: {
			sys16_sync_audiocpu();
			return ppi8255_r(0, (a & 7) >> 1);
		}
	
		case 0xe01000: {
			return 0xff - System16Input[0];
		}
		
		case 0xe0100a: {
			return System16Dip[0];
		}
		
		case 0xe0100c: {
			return System16Dip[1];
		}

		case 0xe03000:
		case 0xe03002:
		case 0xe03004:
		case 0xe03006: {
			return ppi8255_r(1, (a & 7) >> 1);
		}
	}
	
#if 0 && defined FBNEO_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Read Word -> 0x%06X\n"), a);
#endif

	return 0;
}

UINT8 __fastcall HangonReadByte(UINT32 a)
{
	switch (a) {
		case 0xe00001:
		case 0xe00003:
		case 0xe00005:
		case 0xe00007: {
			sys16_sync_audiocpu();
			return ppi8255_r(0, (a & 7) >> 1);
		}
		
		case 0xe01001: {
			return 0xff - System16Input[0];
		}
		
		case 0xe0100b: {
			return System16Dip[0];
		}
		
		case 0xe0100d: {
			return System16Dip[1];
		}
		
		case 0xe03001:
		case 0xe03003:
		case 0xe03005:
		case 0xe03007: {
			return ppi8255_r(1, (a & 7) >> 1);
		}
		
		case 0xe03021: {
			if (System16ProcessAnalogControlsDo) return System16ProcessAnalogControlsDo(System16AnalogSelect);
			return 0xff;
		}
	}

#if 0 && defined FBNEO_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Read Byte -> 0x%06X\n"), a);
#endif

	return 0;
}

void __fastcall HangonWriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0x400000 && a <= 0x403fff) {
		System16ATileByteWrite((a - 0x400000) ^ 1, d);
		return;
	}

	switch (a) {
		case 0xe00001:
		case 0xe00003: 
		case 0xe00005: 
		case 0xe00007: {
			sys16_sync_audiocpu();
			ppi8255_w(0, (a & 7) >> 1, d & 0xff);
			return;
		}
		
		case 0xe03001:
		case 0xe03003:
		case 0xe03005:
		case 0xe03007: {
			ppi8255_w(1, (a & 7) >> 1, d & 0xff);
			return;
		}
		
		case 0xe03021: {
			return;
		}
		
		
		case 0xe00000:
		case 0xe00002: {
			return;
		}
	}

#if 0 && defined FBNEO_DEBUG
	bprintf(PRINT_NORMAL, _T("68000 Write Byte -> 0x%06X, 0x%02X\n"), a, d);
#endif
}

void __fastcall HangonWriteWord(UINT32 a, UINT16 d)
{
	if (a >= 0x400000 && a <= 0x403fff) {
		System16ATileWordWrite(a - 0x400000, d);
		return;
	}
	
	switch (a) {
		case 0xe00000:
		case 0xe00002: 
		case 0xe00004: 
		case 0xe00006: {
			sys16_sync_audiocpu();
			ppi8255_w(0, (a & 7) >> 1, d & 0xff);
			return;
		}

		case 0xe03000:
		case 0xe03002:
		case 0xe03004:
		case 0xe03006: {
			ppi8255_w(1, (a & 7) >> 1, d & 0xff);
			return;
		}
	}

#if 0 && defined FBNEO_DEBUG	
	bprintf(PRINT_NORMAL, _T("68000 Write Word -> 0x%06X, 0x%04X\n"), a, d);
#endif
}

static UINT16 __fastcall SharrierReadWord(UINT32 a)
{
	if (a >= 0x40000 && a <= 0x43fff) {
		if (dontrecurse == 0) sys16_sync_mcu();
		return *(UINT16*)(System16Ram + (a & 0x3fff));
	}

	switch (a) {
		case 0x140000:
		case 0x140002:
		case 0x140004:
		case 0x140006: {
			sys16_sync_audiocpu();
			return ppi8255_r(0, (a & 7) >> 1);
		}

		case 0x140010: {
			return (UINT16)(0xff - System16Input[0]);
		}
		
		case 0x140012: {
			return 0xffff;
		}

		case 0x140014: {
			return (UINT16)System16Dip[0];
		}
		
		case 0x140016: {
			return (UINT16)System16Dip[1];
		}

		case 0x140020:
		case 0x140022:
		case 0x140024:
		case 0x140026: {
			return ppi8255_r(1, (a & 7) >> 1);
		}
	}

	return 0;
}

static UINT8 __fastcall SharrierReadByte(UINT32 a)
{
	if (a >= 0x40000 && a <= 0x43fff) {
		if (dontrecurse == 0) sys16_sync_mcu();
		return System16Ram[(a & 0x3fff) ^ 1];
	}

	switch (a) {
		case 0x140001:
		case 0x140003:
		case 0x140005:
		case 0x140007: {
			sys16_sync_audiocpu();
			return ppi8255_r(0, (a & 7) >> 1);
		}
		
		case 0x140011: {
			return 0xff - System16Input[0];
		}
		
		case 0x140015: {
			return System16Dip[0];
		}
		
		case 0x140021:
		case 0x140023:
		case 0x140025:
		case 0x140027: {
			return ppi8255_r(1, (a & 7) >> 1);
		}
		
		case 0x140031: {
			if (System16ProcessAnalogControlsDo) return System16ProcessAnalogControlsDo(System16AnalogSelect);
			return 0xff;
		}
	}

	return 0;
}

static void __fastcall SharrierWriteByte(UINT32 a, UINT8 d)
{
	if (a >= 0x40000 && a <= 0x43fff) {
		if (dontrecurse == 0) sys16_sync_mcu();
#if 0
		// 0x40385 issue mcu debug (for later)
		INT32 z = a&0x3fff;
		if ((z& ~1) == 0x384) {
			bprintf(0, _T("[68k.b] frame: %d / cyc: %d  -  addr  %x   data %x\n"), nCurrentFrame, SekTotalCycles(), a, d);
		}
#endif
		System16Ram[(a & 0x3fff) ^ 1] = d;
		return;
	}

	if (a >= 0x100000 && a <= 0x107fff) {
		System16ATileByteWrite((a - 0x100000) ^ 1, d);
		return;
	}
	
	switch (a) {
		case 0x140001:
		case 0x140003: 
		case 0x140005: 
		case 0x140007: {
			sys16_sync_audiocpu();
			ppi8255_w(0, (a & 7) >> 1, d & 0xff);
			return;
		}
		
		case 0x140021:
		case 0x140023:
		case 0x140025:
		case 0x140027: {
			ppi8255_w(1, (a & 7) >> 1, d & 0xff);
			return;
		}
		
		case 0x140031: {
			return;
		}
	}
}

static void __fastcall SharrierWriteWord(UINT32 a, UINT16 d)
{
	if (a >= 0x40000 && a <= 0x43fff) {
		if (dontrecurse == 0) sys16_sync_mcu();
#if 0
		// 0x40385 issue mcu debug (for later)
		INT32 z = a&0x3fff;
		if ((z& ~1) == 0x384) {
			bprintf(0, _T("[68k.b] frame: %d / cyc: %d  -  addr  %x   data %x\n"), nCurrentFrame, SekTotalCycles(), a, d);
		}
#endif
		*(UINT16*)(System16Ram + (a & 0x3fff)) = d;
		return;
	}

	if (a >= 0x100000 && a <= 0x107fff) {
		System16ATileWordWrite(a - 0x100000, d);
		return;
	}

	switch (a) {
		case 0x140000:
		case 0x140002:
		case 0x140004:
		case 0x140006: {
			sys16_sync_audiocpu();
			ppi8255_w(0, (a & 7) >> 1, d & 0xff);
			return;
		}
		
		case 0x140020:
		case 0x140022:
		case 0x140024:
		case 0x140026: {
			ppi8255_w(1, (a & 7) >> 1, d & 0xff);
			return;
		}
	}
}

/*====================================================
Driver Inits
====================================================*/

static UINT8 EndurorProcessAnalogControls(UINT16 value)
{
	UINT8 temp = 0;
	
	switch (value) {

		// Accelerate
		case 0: {
			return ProcessAnalog(System16AnalogPort2, 0, INPUT_DEADZONE | INPUT_LINEAR | INPUT_MIGHTBEDIGITAL, 0x00, 0xff);
		}

		// Brake
		case 1: {
			return ProcessAnalog(System16AnalogPort3, 0, INPUT_DEADZONE | INPUT_LINEAR | INPUT_MIGHTBEDIGITAL, 0x00, 0xff);
		}

		// Bank Up / Down
		case 2: {
			temp = ProcessAnalog(System16AnalogPort1, 0, INPUT_DEADZONE, 0x01, 0xff);

			if (temp > 0x80) {
				temp = scalerange(temp, 0x80, 0xff, 0x20, 0xff);
			} else if (temp < 0x80) {
				temp = scalerange(temp, 0x00, 0x80, 0x00, 0x20);
			} else {
				temp = 0x20;
			}

			return temp;
		}

		// Steering
		case 3: {
			return ProcessAnalog(System16AnalogPort0, 1, INPUT_DEADZONE, 0x01, 0xff);
		}
	}
	
	return 0;
}

static UINT8 HangonProcessAnalogControls(UINT16 value)
{
	switch (value) {

		// Steering
		case 0: {
			return ProcessAnalog(System16AnalogPort0, 1, INPUT_DEADZONE, 0x20, 0xe0);
		}
		
		// Accelerate
		case 1: {
			return ProcessAnalog(System16AnalogPort1, 0, INPUT_DEADZONE | INPUT_LINEAR | INPUT_MIGHTBEDIGITAL, 0x00, 0xff);
		}
		
		// Brake
		case 2: {
			return ProcessAnalog(System16AnalogPort2, 0, INPUT_DEADZONE | INPUT_LINEAR | INPUT_MIGHTBEDIGITAL, 0x00, 0xff);
		}
	}
	
	return 0;
}

static UINT8 SharrierProcessAnalogControls(UINT16 value)
{
	switch (value) {

		// Left / Right
		case 0: {
			return ProcessAnalog(System16AnalogPort0, 1, INPUT_DEADZONE, 0x20, 0xe0);
		}

		// Up / Down
		case 1: {
			return ProcessAnalog(System16AnalogPort1, 1, INPUT_DEADZONE, 0x20, 0xe0);
		}
	}
	
	return 0;
}

UINT8 Hangon_I8751ReadPort(INT32 port)
{
	if (port >= 0x0000 && port <= 0xffff) {
		SekCPUPush(0);
		dontrecurse = 1; // 68k read handler syncs mcu, we don't want to do that here!
		UINT8 b = SekReadByte((System16MCUData << 16) | (port ^ 1));
		dontrecurse = 0;
		SekCPUPop();

		mcs51RunEnd(); // break out of mcu so 68k can catch up

		return b;
	}

	return 0xff;
}

void Hangon_I8751WritePort(INT32 port, UINT8 data)
{
	if (port >= 0x0000 && port <= 0xffff) {
		UINT32 addr = (System16MCUData << 16) | (port ^ 1);

		if (addr == 0x40385) { // enabling writes here breaks inputs(!)
			//bprintf(0, _T("[mcu] frame: %d / cyc: %d  -  port  %x   addr  %x   data %x\n"), nCurrentFrame, mcs51TotalCycles(), port, addr, data);
			return;
		}

		SekCPUPush(0);
		dontrecurse = 1;
		SekWriteByte(addr, data);
		dontrecurse = 0;
		SekCPUPop();

		mcs51RunEnd(); // break out of mcu so 68k can catch up

		return;
	}

	switch (port) {
		case MCS51_PORT_P1: {
			System16MCUData = (BIT(data, 6) << 4) | ((data & 0x38) >> 3);
			INT32 irq_line = ~data & 0x07;
			if (irq_line) {
				//bprintf(0, _T("mcu -> 68k irq line %d\tframe: %d\n"), irq_line, nCurrentFrame);
				SekSetIRQLine(0, irq_line, CPU_IRQSTATUS_AUTO);
			}
			break;
		}
		default: {
			//bprintf(0, _T("unmapped port   %x    %x\n"), port, data);
		}
	}
}


static void SharrierMap68K()
{
	SekInit(0, 0x68000);
	SekOpen(0);
	SekMapMemory(System16Rom             , 0x000000, 0x03ffff, MAP_READ);
	SekMapMemory(System16Code            , 0x000000, 0x03ffff, MAP_FETCH);
	// RAM (40000 - 43fff) is written/read through handlers
	// because we need mega-tight sync w/mcu -dink
	//SekMapMemory(System16Ram             , 0x040000, 0x043fff, MAP_RAM); // in handler
	SekMapMemory(System16TileRam         , 0x100000, 0x107fff, MAP_READ);
	SekMapMemory(System16TextRam         , 0x108000, 0x108fff, MAP_RAM);
	SekMapMemory(System16PaletteRam      , 0x110000, 0x110fff, MAP_RAM);
	SekMapMemory(System16ExtraRam        , 0x124000, 0x127fff, MAP_RAM);
	SekMapMemory(System16SpriteRam       , 0x130000, 0x130fff, MAP_RAM);
	SekMapMemory(System16RoadRam         , 0xc68000, 0xc68fff, MAP_RAM);

	SekSetReadWordHandler(0, SharrierReadWord);
	SekSetReadByteHandler(0, SharrierReadByte);
	SekSetWriteByteHandler(0, SharrierWriteByte);
	SekSetWriteWordHandler(0, SharrierWriteWord);
	SekClose();
}

static INT32 EndurorInit()
{
	System16Map68KDo = SharrierMap68K;
	
	System16ProcessAnalogControlsDo = EndurorProcessAnalogControls;
	
	System16ClockSpeed = 10000000;
	
	System16PCMDataSizePreAllocate = 0x18000;
	
	INT32 nRet = System16Init();
	
	UINT8 *pTemp = (UINT8*)BurnMalloc(0x10000);
	memcpy(pTemp, System16PCMData, 0x10000);
	memset(System16PCMData, 0, 0x18000);
	memcpy(System16PCMData + 0x00000, pTemp + 0x00000, 0x8000);
	memcpy(System16PCMData + 0x10000, pTemp + 0x08000, 0x8000);
	BurnFree(pTemp);
	
	return nRet;
}

static INT32 Enduror1Init()
{
	System16Map68KDo = SharrierMap68K;
	
	System16ProcessAnalogControlsDo = EndurorProcessAnalogControls;
	
	System16ClockSpeed = 10000000;
	
	return System16Init();
}

static INT32 EnduroblLoadRom()
{
	INT32 nRet = 1;
	UINT8 *pTemp = (UINT8*)BurnMalloc(0x40000);
	
	if (pTemp) {
		memcpy(pTemp, System16Rom, 0x40000);
		memset(System16Rom, 0, 0x40000);
		memcpy(System16Rom + 0x00000, pTemp + 0x10000, 0x10000);
		memcpy(System16Rom + 0x10000, pTemp + 0x20000, 0x20000);
		memcpy(System16Rom + 0x30000, pTemp + 0x00000, 0x10000);
		BurnFree(pTemp);
		nRet = 0;
	}

	return nRet;
}

static INT32 EnduroblDecryptOpCode()
{
	UINT16 *rom = (UINT16*)System16Rom;
	UINT16 *decrypt = (UINT16*)System16Code;
	memcpy(decrypt + 0x00000/2, rom + 0x30000/2, 0x10000);
	memcpy(decrypt + 0x10000/2, rom + 0x10000/2, 0x20000);
	
	return 0;
}

static INT32 EnduroblInit()
{
	System16CustomLoadRomDo = EnduroblLoadRom;
	
	System16CustomDecryptOpCodeDo = EnduroblDecryptOpCode;
	
	System16Map68KDo = SharrierMap68K;
	
	System16ProcessAnalogControlsDo = EndurorProcessAnalogControls;
	
	System16ClockSpeed = 10000000;
	
	return System16Init();
}

static INT32 HangonInit()
{
	System16ProcessAnalogControlsDo = HangonProcessAnalogControls;
	
	System16ClockSpeed = 25174800 / 4;
	
	INT32 nRet = System16Init();
	
	if (!nRet) Hangon = true;
	
	return nRet;
}

static INT32 ShangonrbInit()
{
	System16ProcessAnalogControlsDo = HangonProcessAnalogControls;
	
	System16ClockSpeed = 10000000;
	
	INT32 nRet = System16Init();
	
	if (!nRet) Hangon = true;
	
	return nRet;
}

static INT32 SharrierInit()
{	
	System16Map68KDo = SharrierMap68K;

	System16ProcessAnalogControlsDo = SharrierProcessAnalogControls;
	
	System16ClockSpeed = 10000000;
	
	return System16Init();
}

/*====================================================
Driver defs
====================================================*/

struct BurnDriver BurnDrvEnduror = {
	"enduror", NULL, NULL, NULL, "1986",
	"Enduro Racer (Rev A, YM2151, FD1089B 317-0013A)\0", NULL, "Sega", "Hang-On",
	L"\u4E16\u5609\u8D8A\u91CE\u673A\u8F66\u8D5B (\u4FEE\u8BA2\u7248 A, YM2151, FD1089B 317-0013A)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON | HARDWARE_SEGA_FD1089B_ENC | HARDWARE_SEGA_SPRITE_LOAD32, GBF_RACING, 0,
	NULL, EndurorRomInfo, EndurorRomName, NULL, NULL, NULL, NULL, EndurorInputInfo, EndurorDIPInfo,
	EndurorInit, System16Exit, HangonFrame, HangonRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvEndurord = {
	"endurord", "enduror", NULL, NULL, "1986",
	"Enduro Racer (bootleg of Rev A, YM2151, FD1089B 317-0013A set)\0", NULL, "bootleg", "Hang-On",
	L"\u4E16\u5609\u8D8A\u91CE\u673A\u8F66\u8D5B (\u4FEE\u8BA2\u7248 A, YM2151, FD1089B 317-0013A set \u7684\u76D7\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON | HARDWARE_SEGA_SPRITE_LOAD32, GBF_RACING, 0,
	NULL, EndurordRomInfo, EndurordRomName, NULL, NULL, NULL, NULL, EndurorInputInfo, EndurorDIPInfo,
	EndurorInit, System16Exit, HangonFrame, HangonRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvEnduror1 = {
	"enduror1", "enduror", NULL, NULL, "1986",
	"Enduro Racer (YM2203, FD1089B 317-0013A)\0", NULL, "Sega", "Hang-On",
	L"\u4E16\u5609\u8D8A\u91CE\u673A\u8F66\u8D5B (YM2203, FD1089B 317-0013A)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON | HARDWARE_SEGA_FD1089B_ENC | HARDWARE_SEGA_SPRITE_LOAD32 | HARDWARE_SEGA_YM2203, GBF_RACING, 0,
	NULL, Enduror1RomInfo, Enduror1RomName, NULL, NULL, NULL, NULL, EndurorInputInfo, EndurorDIPInfo,
	Enduror1Init, System16Exit, HangonYM2203Frame, HangonRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvEnduror1d = {
	"enduror1d", "enduror", NULL, NULL, "1986",
	"Enduro Racer (bootleg of YM2203, FD1089B 317-0013A set)\0", NULL, "bootleg", "Hang-On",
	L"\u4E16\u5609\u8D8A\u91CE\u673A\u8F66\u8D5B (YM2203, FD1089B 317-0013A set \u7684\u76D7\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON | HARDWARE_SEGA_SPRITE_LOAD32 | HARDWARE_SEGA_YM2203, GBF_RACING, 0,
	NULL, Enduror1dRomInfo, Enduror1dRomName, NULL, NULL, NULL, NULL, EndurorInputInfo, EndurorDIPInfo,
	Enduror1Init, System16Exit, HangonYM2203Frame, HangonRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvEndurora = {
	"endurora", "enduror", NULL, NULL, "1986",
	"Enduro Racer (Rev A, YM2151, mask ROM sprites, FD1089B 317-0013A)\0", NULL, "Sega", "Hang-On",
	L"\u4E16\u5609\u8D8A\u91CE\u673A\u8F66\u8D5B (\u4FEE\u8BA2\u7248 A, YM2151, mask ROM sprites, FD1089B 317-0013A)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON | HARDWARE_SEGA_FD1089B_ENC | HARDWARE_SEGA_SPRITE_LOAD32, GBF_RACING, 0,
	NULL, EnduroraRomInfo, EnduroraRomName, NULL, NULL, NULL, NULL, EndurorInputInfo, EndurorDIPInfo,
	EndurorInit, System16Exit, HangonFrame, HangonRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvEndurorb = {
	"endurorb", "enduror", NULL, NULL, "1986",
	"Enduro Racer (YM2151, FD1089B 317-0013A)\0", NULL, "Sega", "Hang-On",
	L"\u4E16\u5609\u8D8A\u91CE\u673A\u8F66\u8D5B (YM2151, FD1089B 317-0013A)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON | HARDWARE_SEGA_FD1089B_ENC | HARDWARE_SEGA_SPRITE_LOAD32, GBF_RACING, 0,
	NULL, EndurorbRomInfo, EndurorbRomName, NULL, NULL, NULL, NULL, EndurorInputInfo, EndurorDIPInfo,
	EndurorInit, System16Exit, HangonFrame, HangonRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvEndurobl = {
	"endurobl", "enduror", NULL, NULL, "1986",
	"Enduro Racer (bootleg set 1)\0", NULL, "bootleg", "Hang-On",
	L"\u4E16\u5609\u8D8A\u91CE\u673A\u8F66\u8D5B (\u76D7\u7248 \u7B2C\u4E00\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON | HARDWARE_SEGA_SPRITE_LOAD32 | HARDWARE_SEGA_YM2203, GBF_RACING, 0,
	NULL, EnduroblRomInfo, EnduroblRomName, NULL, NULL, NULL, NULL, EndurorInputInfo, EndurorDIPInfo,
	EnduroblInit, System16Exit, HangonYM2203Frame, HangonRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvHangon = {
	"hangon", NULL, NULL, NULL, "1985",
	"Hang-On (Rev A)\0", NULL, "Sega", "Hang-On",
	L"\u6469\u6258\u8F66\u5927\u8D5B (\u4FEE\u8BA2\u7248 A)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON | HARDWARE_SEGA_YM2203, GBF_RACING, 0,
	NULL, HangonRomInfo, HangonRomName, NULL, NULL, NULL, NULL, HangonInputInfo, HangonDIPInfo,
	HangonInit, System16Exit, HangonYM2203Frame, HangonAltRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvHangon1 = {
	"hangon1", "hangon", NULL, NULL, "1985",
	"Hang-On\0", NULL, "Sega", "Hang-On",
	L"\u6469\u6258\u8F66\u5927\u8D5B\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON | HARDWARE_SEGA_YM2203, GBF_RACING, 0,
	NULL, Hangon1RomInfo, Hangon1RomName, NULL, NULL, NULL, NULL, HangonInputInfo, HangonDIPInfo,
	HangonInit, System16Exit, HangonYM2203Frame, HangonAltRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvHangon2 = {
	"hangon2", "hangon", NULL, NULL, "1985",
	"Hang-On (Rev A, ride-on)\0", NULL, "Sega", "Hang-On",
	L"\u6469\u6258\u8F66\u5927\u8D5B (\u4FEE\u8BA2\u7248 A, ride-on)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON | HARDWARE_SEGA_YM2203, GBF_RACING, 0,
	NULL, Hangon2RomInfo, Hangon2RomName, NULL, NULL, NULL, NULL, HangonInputInfo, HangonDIPInfo,
	HangonInit, System16Exit, HangonYM2203Frame, HangonAltRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvHangonvf = {
	"hangonvf", "hangon", NULL, NULL, "1985",
	"VF (bootleg of Hang-On)\0", NULL, "bootleg", "Hang-On",
	L"\u6469\u6258\u8F66\u5927\u8D5B VF (Hang-On \u7684\u76D7\u7248)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON | HARDWARE_SEGA_YM2203, GBF_RACING, 0,
	NULL, HangonvfRomInfo, HangonvfRomName, NULL, NULL, NULL, NULL, HangonInputInfo, HangonDIPInfo,
	HangonInit, System16Exit, HangonYM2203Frame, HangonAltRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriverD BurnDrvShangonro = {
	"shangonro", "shangon", NULL, NULL, "1987",
	"Super Hang-On (Hang-On conversion, ride-on, Japan, FD1094 317-0038)\0", NULL, "Sega", "Hang-On",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON | HARDWARE_SEGA_FD1094_ENC_CPU2, GBF_RACING, 0,
	NULL, ShangonroRomInfo, ShangonroRomName, NULL, NULL, NULL, NULL, ShangonrbInputInfo, ShangonrbDIPInfo,
	ShangonrbInit, System16Exit, HangonFrame, HangonRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriverD BurnDrvShangonho = {
	"shangonho", "shangon", NULL, NULL, "1987",
	"Super Hang-On (Hang-On conversion, Japan, FD1094 317-0039)\0", NULL, "Sega", "Hang-On",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON | HARDWARE_SEGA_FD1094_ENC_CPU2, GBF_RACING, 0,
	NULL, ShangonhoRomInfo, ShangonhoRomName, NULL, NULL, NULL, NULL, ShangonrbInputInfo, ShangonrbDIPInfo,
	ShangonrbInit, System16Exit, HangonFrame, HangonRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriverD BurnDrvShangonrb = {
	"shangonrb", "shangon", NULL, NULL, "1992",
	"Super Hang-On (Hang-On conversion, bootleg)\0", NULL, "bootleg", "Hang-On",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON, GBF_RACING, 0,
	NULL, ShangonrbRomInfo, ShangonrbRomName, NULL, NULL, NULL, NULL, ShangonrbInputInfo, ShangonrbDIPInfo,
	ShangonrbInit, System16Exit, HangonFrame, HangonRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriverD BurnDrvShangonrb2 = {
	"shangonrb2", "shangon", NULL, NULL, "1987",
	"Super Hang-On (Hang-On conversion, Beta bootleg)\0", NULL, "bootleg (Beta)", "Hang-On",
	NULL, NULL, NULL, NULL,
	BDF_CLONE | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON, GBF_RACING, 0,
	NULL, Shangonrb2RomInfo, Shangonrb2RomName, NULL, NULL, NULL, NULL, ShangonrbInputInfo, ShangonrbDIPInfo,
	ShangonrbInit, System16Exit, HangonFrame, HangonRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvSharrier = {
	"sharrier", NULL, NULL, NULL, "1985",
	"Space Harrier (Rev A, 8751 315-5163A)\0", NULL, "Sega", "Hang-On",
	L"\u65F6\u7A7A\u54C8\u5229 (\u4FEE\u8BA2\u7248 A, 8751 315-5163A)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON | HARDWARE_SEGA_SPRITE_LOAD32 | HARDWARE_SEGA_YM2203, GBF_SHOOT, 0,
	NULL, SharrierRomInfo, SharrierRomName, NULL, NULL, NULL, NULL, SharrierInputInfo, SharrierDIPInfo,
	SharrierInit, System16Exit, HangonYM2203Frame, HangonRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};

struct BurnDriver BurnDrvSharrier1 = {
	"sharrier1", "sharrier", NULL, NULL, "1985",
	"Space Harrier (8751 315-5163)\0", NULL, "Sega", "Hang-On",
	L"\u65F6\u7A7A\u54C8\u5229 (8751 315-5163)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 2, HARDWARE_SEGA_HANGON | HARDWARE_SEGA_SPRITE_LOAD32 | HARDWARE_SEGA_YM2203, GBF_SHOOT, 0,
	NULL, Sharrier1RomInfo, Sharrier1RomName, NULL, NULL, NULL, NULL, SharrierInputInfo, SharrierDIPInfo,
	SharrierInit, System16Exit, HangonYM2203Frame, HangonRender, System16Scan,
	NULL, 0x1800, 320, 224, 4, 3
};
