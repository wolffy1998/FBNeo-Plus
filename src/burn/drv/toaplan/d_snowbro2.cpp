// FB Alpha Snow Bros. 2 driver module
// Driver and emulation by Jan Klaassen

#include "toaplan.h"
#include "tiles_generic.h"
// Snow Bros. 2

static UINT8 DrvButton[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvButtonF[8] = {0, 0, 0, 0, 0, 0, 0, 0}; // fake inputs to make symmetric coin
static UINT8 DrvJoy1[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy2[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy3[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvJoy4[8] = {0, 0, 0, 0, 0, 0, 0, 0};
static UINT8 DrvInput[8] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

static UINT8 *Mem = NULL, *MemEnd = NULL;
static UINT8 *RamStart, *RamEnd;
static UINT8 *Rom01;
static UINT8 *Ram01, *RamPal;

static INT32 nColCount = 0x0800;

static UINT8 DrvReset = 0;

static HoldCoin<2> hold_coin;

static struct BurnInputInfo snowbro2InputList[] = {
	{"P1 Coin",		BIT_DIGITAL,	DrvButton + 3,	"p1 coin"},
	{"P1 Start",	BIT_DIGITAL,	DrvButton + 5,	"p1 start"},
	{"P1 Up",		BIT_DIGITAL,	DrvJoy1 + 0,	"p1 up"},
	{"P1 Down",		BIT_DIGITAL,	DrvJoy1 + 1,	"p1 down"},
	{"P1 Left",		BIT_DIGITAL,	DrvJoy1 + 2,	"p1 left"},
	{"P1 Right",	BIT_DIGITAL,	DrvJoy1 + 3,	"p1 right"},
	{"P1 Button 1",	BIT_DIGITAL,	DrvJoy1 + 4,	"p1 fire 1"},
	{"P1 Button 2",	BIT_DIGITAL,	DrvJoy1 + 5,	"p1 fire 2"},

	{"P2 Coin",		BIT_DIGITAL,	DrvButton + 4,	"p2 coin"},
	{"P2 Start",	BIT_DIGITAL,	DrvButton + 6,	"p2 start"},
	{"P2 Up",		BIT_DIGITAL,	DrvJoy2 + 0,	"p2 up"},
	{"P2 Down",		BIT_DIGITAL,	DrvJoy2 + 1,	"p2 down"},
	{"P2 Left",		BIT_DIGITAL,	DrvJoy2 + 2,	"p2 left"},
	{"P2 Right",	BIT_DIGITAL,	DrvJoy2 + 3,	"p2 right"},
	{"P2 Button 1",	BIT_DIGITAL,	DrvJoy2 + 4,	"p2 fire 1"},
	{"P2 Button 2",	BIT_DIGITAL,	DrvJoy2 + 5,	"p2 fire 2"},

	{"P3 Coin",		BIT_DIGITAL,	DrvButtonF + 3,	"p3 coin"},
	{"P3 Start",	BIT_DIGITAL,	DrvJoy3 + 6,	"p3 start"},
	{"P3 Up",		BIT_DIGITAL,	DrvJoy3 + 0,	"p3 up"},
	{"P3 Down",		BIT_DIGITAL,	DrvJoy3 + 1,	"p3 down"},
	{"P3 Left",		BIT_DIGITAL,	DrvJoy3 + 2,	"p3 left"},
	{"P3 Right",	BIT_DIGITAL,	DrvJoy3 + 3,	"p3 right"},
	{"P3 Button 1",	BIT_DIGITAL,	DrvJoy3 + 4,	"p3 fire 1"},
	{"P3 Button 2",	BIT_DIGITAL,	DrvJoy3 + 5,	"p3 fire 2"},

	{"P4 Coin",		BIT_DIGITAL,	DrvButtonF + 4,	"p4 coin"},
	{"P4 Start",	BIT_DIGITAL,	DrvJoy4 + 6,	"p4 start"},
	{"P4 Up",		BIT_DIGITAL,	DrvJoy4 + 0,	"p4 up"},
	{"P4 Down",		BIT_DIGITAL,	DrvJoy4 + 1,	"p4 down"},
	{"P4 Left",		BIT_DIGITAL,	DrvJoy4 + 2,	"p4 left"},
	{"P4 Right",	BIT_DIGITAL,	DrvJoy4 + 3,	"p4 right"},
	{"P4 Button 1",	BIT_DIGITAL,	DrvJoy4 + 4,	"p4 fire 1"},
	{"P4 Button 2",	BIT_DIGITAL,	DrvJoy4 + 5,	"p4 fire 2"},

	{"Reset",		BIT_DIGITAL,	&DrvReset,		"reset"},
	{"Service",		BIT_DIGITAL,	DrvButton + 0,	"service"},
	{"Dip A",		BIT_DIPSWITCH,	DrvInput + 3,	"dip"},
	{"Dip B",		BIT_DIPSWITCH,	DrvInput + 4,	"dip"},
	{"Dip C",		BIT_DIPSWITCH,	DrvInput + 5,	"dip"},
};

STDINPUTINFO(snowbro2)

static struct BurnDIPInfo snowbro2DIPList[] = 
{
	DIP_OFFSET(0x22)

	// Defaults
	{0x00,	0xFF, 0xFF,	0x00, NULL},
	{0x01,	0xFF, 0xFF,	0x00, NULL},
	{0x02,	0xFF, 0xFF,	0x00, NULL},

	// DIP 1
	{0   , 0xFE, 0   , 2   , "Continue Mode"                },
	{0x00, 0x01, 0x01, 0x00, "Normal"                       },
	{0x00, 0x01, 0x01, 0x01, "Discount"                     },

	{0   , 0xFE, 0   , 2   , "Flip Screen"                  },
	{0x00, 0x01, 0x02, 0x00, "Off"                          },
	{0x00, 0x01, 0x02, 0x02, "On"                           },

	{0   , 0xFE, 0   , 2   , "Service Mode"                 },
	{0x00, 0x01, 0x04, 0x00, "Off"                          },
	{0x00, 0x01, 0x04, 0x04, "On"                           },

	{0   , 0xFE, 0   , 2   , "Demo Sounds"                  },
	{0x00, 0x01, 0x08, 0x08, "Off"                          },
	{0x00, 0x01, 0x08, 0x00, "On"                           },

	// Normal coin settings
	{0,		0xFE, 0,	4,	  "Coin A"},
	{0x00,	0x82, 0x30,	0x00, "1 coin 1 play"},
	{0x02,	0x00, 0x0F, 0x08, NULL},
	{0x00,	0x82, 0x30,	0x10, "1 coin 2 plays"},
	{0x02,	0x00, 0x0F, 0x08, NULL},
	{0x00,	0x82, 0x30,	0x20, "2 coins 1 play"},
	{0x02,	0x00, 0x0F, 0x08, NULL},
	{0x00,	0x82, 0x30,	0x30, "2 coins 3 plays"},
	{0x02,	0x00, 0x0F, 0x08, NULL},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x00,	0x82, 0xC0,	0x00, "1 coin 1 play"},
	{0x02,	0x00, 0x0F, 0x08, NULL},
	{0x00,	0x82, 0xC0,	0x40, "1 coin 2 plays"},
	{0x02,	0x00, 0x0F, 0x08, NULL},
	{0x00,	0x82, 0xC0,	0x80, "2 coins 1 play"},
	{0x02,	0x00, 0x0F, 0x08, NULL},
	{0x00,	0x82, 0xC0,	0xC0, "2 coins 3 plays"},
	{0x02,	0x00, 0x0F, 0x08, NULL},

	// European coin settings
	{0,		0xFE, 0,	4,	  "Coin A"},
	{0x00,	0x02, 0x30,	0x00, "1 coin 1 play"},
	{0x02,	0x00, 0x0F, 0x08, NULL},
	{0x00,	0x02, 0x30,	0x10, "2 coins 1 play"},
	{0x02,	0x00, 0x0F, 0x08, NULL},
	{0x00,	0x02, 0x30,	0x20, "3 coins 1 play"},
	{0x02,	0x00, 0x0F, 0x08, NULL},
	{0x00,	0x02, 0x30,	0x30, "4 coins 1 play"},
	{0x02,	0x00, 0x0F, 0x08, NULL},
	{0,		0xFE, 0,	4,	  "Coin B"},
	{0x00,	0x02, 0xC0,	0x00, "1 coin 2 plays"},
	{0x02,	0x00, 0x0F, 0x08, NULL},
	{0x00,	0x02, 0xC0,	0x40, "1 coin 3 plays"},
	{0x02,	0x00, 0x0F, 0x08, NULL},
	{0x00,	0x02, 0xC0,	0x80, "1 coin 4 play"},
	{0x02,	0x00, 0x0F, 0x08, NULL},
	{0x00,	0x02, 0xC0,	0xC0, "1 coin 6 plays"},
	{0x02,	0x00, 0x0F, 0x08, NULL},

	// DIP 2
	{0   , 0xFE, 0   , 4   , "Difficulty"                   },
	{0x01, 0x01, 0x03, 0x01, "Easy"                         },
	{0x01, 0x01, 0x03, 0x00, "Normal"                       },
	{0x01, 0x01, 0x03, 0x02, "Hard"                         },
	{0x01, 0x01, 0x03, 0x03, "Very Hard"                    },

	{0   , 0xFE, 0   , 4   , "Bonus Life"                   },
	{0x01, 0x01, 0x0C, 0x04, "100000 / 500000"              },
	{0x01, 0x01, 0x0C, 0x00, "100000 only"                  },
	{0x01, 0x01, 0x0C, 0x08, "200000 only"                  },
	{0x01, 0x01, 0x0C, 0x0C, "None"                         },

	{0   , 0xFE, 0   , 4   , "Lives"                        },
	{0x01, 0x01, 0x30, 0x30, "1"                            },
	{0x01, 0x01, 0x30, 0x20, "2"                            },
	{0x01, 0x01, 0x30, 0x00, "3"                            },
	{0x01, 0x01, 0x30, 0x10, "4"                            },

	{0   , 0xFE, 0   , 2   , "Game Type"                    },
	{0x01, 0x01, 0x40, 0x00, "Normal"                       },
	{0x01, 0x01, 0x40, 0x40, "No Death & Stop Mode"         },

	{0   , 0xFE, 0   , 2   , "Max Players"                  },
	{0x01, 0x01, 0x80, 0x80, "2"                            },
	{0x01, 0x01, 0x80, 0x00, "4"                            },

	// Dip 3
	{0   , 0xFE, 0   , 7   , "Territory"                    },
	{0x02, 0x01, 0x1C, 0x08, "Europe"                       },
	{0x02, 0x01, 0x1C, 0x10, "Hong Kong"                    },
	{0x02, 0x01, 0x1C, 0x00, "Japan"                        },
	{0x02, 0x01, 0x1C, 0x0c, "Korea"                        },
	{0x02, 0x01, 0x1C, 0x18, "South East Asia"              },
	{0x02, 0x01, 0x1C, 0x14, "Taiwan"                       },
	{0x02, 0x01, 0x1C, 0x04, "USA"                          },

	{0   , 0xFE, 0   , 2   , "Show All Rights Reserved"     },
	{0x02, 0x01, 0x20, 0x00, "No"                           },
	{0x02, 0x01, 0x20, 0x20, "Yes"                          },
};

STDDIPINFO(snowbro2)

static UINT8 __fastcall snowbro2ReadByte(UINT32 sekAddress)
{
	switch (sekAddress) {

		case 0x30000D:
			return ToaVBlankRegister();

		case 0x70000D:								// Player 1 inputs
			return DrvInput[0];
		case 0x700011:								// Player 2 inputs
			return DrvInput[1];
		case 0x700015:								// Player 3 inputs
			return DrvInput[6];
		case 0x700019:								// Player 4 inputs
			return DrvInput[7];
		case 0x70001D:								// Other inputs
			return DrvInput[2];
		case 0x700005:								// Dipswitch A
			return DrvInput[3];
		case 0x700009:								// Dipswitch B
			return DrvInput[4];
		case 0x700000:								// Dipswitch C - Territory
			return DrvInput[5];

		case 0x600001:
			return MSM6295Read(0);
		case 0x500003:
			return BurnYM2151Read();

		default: {
//			printf("Attempt to read byte value of location %x\n", sekAddress);
		}
	}
	return 0;
}

static UINT16 __fastcall snowbro2ReadWord(UINT32 sekAddress)
{
	switch (sekAddress) {

		case 0x300004:
			return ToaGP9001ReadRAM_Hi(0);
		case 0x300006:
			return ToaGP9001ReadRAM_Lo(0);

		case 0x30000C:
			return ToaVBlankRegister();

		case 0x70000C:								// Player 1 inputs
			return DrvInput[0];
		case 0x700010:								// Player 2 inputs
			return DrvInput[1];
		case 0x700014:								// Player 3 inputs
			return DrvInput[6];
		case 0x700018:								// Player 4 inputs
			return DrvInput[7];
		case 0x70001C:								// Other inputs
			return DrvInput[2];
		case 0x700004:								// Dipswitch A
			return DrvInput[3];
		case 0x700008:								// Dipswitch B
			return DrvInput[4];

		case 0x600000:
			return MSM6295Read(0);
		case 0x500002:
			return BurnYM2151Read();

		default: {
// 			printf("Attempt to read word value of location %x\n", sekAddress);
		}
	}
	return 0;
}

static void __fastcall snowbro2WriteByte(UINT32 sekAddress, UINT8 byteValue)
{
	switch (sekAddress) {
		case 0x600001:
			MSM6295Write(0, byteValue);
			break;

		case 0x500001:
			BurnYM2151SelectRegister(byteValue);
			break;
		case 0x500003:
			BurnYM2151WriteRegister(byteValue);
			break;

		default: {
//			printf("Attempt to write byte value %x to location %x\n", byteValue, sekAddress);
		}
	}
}

static void __fastcall snowbro2WriteWord(UINT32 sekAddress, UINT16 wordValue)
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

		case 0x30000C:
			ToaGP9001WriteRegister(wordValue);
			break;

		case 0x600000:
			MSM6295Write(0, wordValue & 0xFF);
			break;
		case 0x700030: {
			MSM6295SetBank(0, MSM6295ROM + ((wordValue & 0x01) << 18), 0x00000, 0x3ffff);
			break;
		}

		case 0x500000:
			BurnYM2151SelectRegister(wordValue);
			break;
		case 0x500002:
			BurnYM2151WriteRegister(wordValue);
			break;

		default: {
//			printf("Attempt to write word value %x to location %x\n", wordValue, sekAddress);
		}
	}
}

static INT32 DrvExit()
{
	MSM6295Exit(0);
	BurnYM2151Exit();

	ToaPalExit();

	ToaExitGP9001();
	SekExit();				// Deallocate 68000s

	BurnFree(Mem);
	
	return 0;
}

static INT32 DrvDoReset()
{
	SekOpen(0);
	SekReset();
	SekClose();

	MSM6295Reset(0);
	BurnYM2151Reset();

	hold_coin.reset();

	HiscoreReset();

	return 0;
}

static INT32 DrvDraw()
{
	ToaClearScreen(0);

	ToaGetBitmap();
	ToaRenderGP9001();						// Render GP9001 graphics

	ToaPalUpdate();							// Update the palette

	return 0;
}

static INT32 DrvFrame()
{
	INT32 nInterleave = 4;

	if (DrvReset) {														// Reset machine
		DrvDoReset();
	}

	// Compile digital inputs
	DrvInput[0] = 0x00;													// Buttons
	DrvInput[1] = 0x00;													// Player 1
	DrvInput[2] = 0x00;													// Player 2
	DrvInput[6] = 0x00;
	DrvInput[7] = 0x00;

	DrvButton[3] |= DrvButtonF[3]; // p3,p4 coin clone buttons
	DrvButton[4] |= DrvButtonF[4];

	for (INT32 i = 0; i < 8; i++) {
		DrvInput[0] |= (DrvJoy1[i] & 1) << i;
		DrvInput[1] |= (DrvJoy2[i] & 1) << i;
		DrvInput[2] |= (DrvButton[i] & 1) << i;
		DrvInput[6] |= (DrvJoy3[i] & 1) << i;
		DrvInput[7] |= (DrvJoy4[i] & 1) << i;
	}
	ToaClearOpposites(&DrvInput[0]);
	ToaClearOpposites(&DrvInput[1]);
	ToaClearOpposites(&DrvInput[6]);
	ToaClearOpposites(&DrvInput[7]);

	hold_coin.check(0, DrvInput[2], 1 << 3, 1);
	hold_coin.check(1, DrvInput[2], 1 << 4, 1);

	SekNewFrame();

	nCyclesTotal[0] = (int)((INT64)16000000 * nBurnCPUSpeedAdjust / (0x0100 * 60));
	nCyclesDone[0] = 0;
	
	SekOpen(0);

	SekSetCyclesScanline(nCyclesTotal[0] / 262);
	nToaCyclesDisplayStart = nCyclesTotal[0] - ((nCyclesTotal[0] * (TOA_VBLANK_LINES + 240)) / 262);
	nToaCyclesVBlankStart = nCyclesTotal[0] - ((nCyclesTotal[0] * TOA_VBLANK_LINES) / 262);
	bool bVBlank = false;

	INT32 nSoundBufferPos = 0;

	for (INT32 i = 0; i < nInterleave; i++) {
    	INT32 nCurrentCPU;
		INT32 nNext;

		// Run 68000

		nCurrentCPU = 0;
		nNext = (i + 1) * nCyclesTotal[nCurrentCPU] / nInterleave;

		// Trigger VBlank interrupt
		if (!bVBlank && nNext > nToaCyclesVBlankStart) {
			if (nCyclesDone[nCurrentCPU] < nToaCyclesVBlankStart) {
				nCyclesSegment = nToaCyclesVBlankStart - nCyclesDone[nCurrentCPU];
				nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);
			}

			bVBlank = true;

			ToaBufferGP9001Sprites();

			SekSetIRQLine(4, CPU_IRQSTATUS_AUTO);
		}

		nCyclesSegment = nNext - nCyclesDone[nCurrentCPU];
		nCyclesDone[nCurrentCPU] += SekRun(nCyclesSegment);

		{
			// Render sound segment
			if (pBurnSoundOut) {
				INT32 nSegmentLength = nBurnSoundLen / nInterleave;
				INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
				BurnYM2151Render(pSoundBuf, nSegmentLength);
				MSM6295Render(0, pSoundBuf, nSegmentLength);
				nSoundBufferPos += nSegmentLength;
			}
		}
	}

	{
		// Make sure the buffer is entirely filled.
		if (pBurnSoundOut) {
			INT32 nSegmentLength = nBurnSoundLen - nSoundBufferPos;
			INT16* pSoundBuf = pBurnSoundOut + (nSoundBufferPos << 1);
			if (nSegmentLength) {
				BurnYM2151Render(pSoundBuf, nSegmentLength);
				MSM6295Render(0, pSoundBuf, nSegmentLength);
			}
		}
	}

	SekClose();

	if (pBurnDraw != NULL) {
		DrvDraw();												// Draw screen if needed
	}

	return 0;
}

// This routine is called first to determine how much memory is needed (MemEnd-(UINT8 *)0),
// and then afterwards to set up all the pointers
static INT32 MemIndex()
{
	UINT8 *Next; Next = Mem;
	Rom01		= Next; Next += 0x080000;		//
	GP9001ROM[0]= Next; Next += nGP9001ROMSize[0];	// GP9001 tile data
	MSM6295ROM	= Next; Next += 0x080000;
	RamStart	= Next;
	Ram01		= Next; Next += 0x010000;		// CPU #0 work RAM
	RamPal		= Next; Next += 0x001000;		// palette
	GP9001RAM[0]= Next; Next += 0x004000;
	GP9001Reg[0]= (UINT16*)Next; Next += 0x0100 * sizeof(UINT16);
	RamEnd		= Next;
	ToaPalette	= (UINT32 *)Next; Next += nColCount * sizeof(UINT32);
	MemEnd		= Next;

	return 0;
}

// Scan ram
static INT32 DrvScan(INT32 nAction,INT32 *pnMin)
{
	struct BurnArea ba;

	if (pnMin) {						// Return minimum compatible version
		*pnMin = 0x020997;
	}
	if (nAction & ACB_VOLATILE) {		// Scan volatile ram
		memset(&ba, 0, sizeof(ba));
    	ba.Data		= RamStart;
		ba.nLen		= RamEnd-RamStart;
		ba.szName	= "All Ram";
		BurnAcb(&ba);

		SekScan(nAction);				// scan 68000 states

		MSM6295Scan(nAction, pnMin);
		BurnYM2151Scan(nAction, pnMin);

		ToaScanGP9001(nAction, pnMin);

		SCAN_VAR(DrvInput);

		hold_coin.scan();
	}
	return 0;
}

static INT32 DrvInit(INT32 (*pLoadRoms)())
{
	INT32 nLen;

#ifdef DRIVER_ROTATION
	bToaRotateScreen = false;
#endif

	nGP9001ROMSize[0] = 0x400000;

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
	if (pLoadRoms) {
		if (pLoadRoms()) {
			return 1;
		}
	}

	{
		SekInit(0, 0x68000);										// Allocate 68000
		SekOpen(0);

		// Map 68000 memory:
		SekMapMemory(Rom01,		0x000000, 0x07FFFF, MAP_ROM);	// CPU 0 ROM
		SekMapMemory(Ram01,		0x100000, 0x10FFFF, MAP_RAM);
		SekMapMemory(RamPal,	0x400000, 0x400FFF, MAP_RAM);	// Palette RAM

		SekSetReadWordHandler(0, snowbro2ReadWord);
		SekSetReadByteHandler(0, snowbro2ReadByte);
		SekSetWriteWordHandler(0, snowbro2WriteWord);
		SekSetWriteByteHandler(0, snowbro2WriteByte);

		SekClose();
	}

	nLayer0XOffset = -0x01D6;
	nLayer1XOffset = -0x01D8;
	nLayer2XOffset = -0x01DA;

	nSpriteYOffset = 0x0011;
	ToaInitGP9001();

	nToaPalLen = nColCount;
	ToaPalSrc = RamPal;
	ToaPalInit();

	BurnYM2151Init(27000000 / 8);
	BurnYM2151SetAllRoutes(0.35, BURN_SND_ROUTE_BOTH);
	MSM6295Init(0, 16000000 / 4 / MSM6295_PIN7_LOW, 1);
	MSM6295SetRoute(0, 0.35, BURN_SND_ROUTE_BOTH);

	DrvDoReset(); // Reset machine
	return 0;
}


// Snow Bros. 2 - With New Elves / Otenki Paradise (Hanafram)

static struct BurnRomInfo snowbro2RomDesc[] = {
	{ "pro-4",        0x080000, 0x4c7ee341, BRF_ESS | BRF_PRG }, //  0 CPU #0 code

	{ "rom2-l",       0x100000, 0xe9d366a9, BRF_GRA },			 //  1 GP9001 Tile data
	{ "rom2-h",       0x080000, 0x9aab7a62, BRF_GRA },			 //  2
	{ "rom3-l",       0x100000, 0xeb06e332, BRF_GRA },			 //  3
	{ "rom3-h",       0x080000, 0xdf4a952a, BRF_GRA },			 //  4

	{ "rom4",         0x080000, 0x638f341e, BRF_SND },			 //  5 MSM6295 ADPCM data
};


STD_ROM_PICK(snowbro2)
STD_ROM_FN(snowbro2)

static INT32 Snowbro2LoadRoms()
{
	// Load 68000 ROM
	BurnLoadRom(Rom01, 0, 1);

	// Load GP9001 tile data
	ToaLoadGP9001Tiles(GP9001ROM[0], 1, 4, nGP9001ROMSize[0]);

	// Load MSM6295 ADPCM data
	BurnLoadRom(MSM6295ROM, 5, 1);

	return 0;
}

static INT32 Snowbro2Init()
{
	return DrvInit(Snowbro2LoadRoms);
}

struct BurnDriver BurnDrvSnowbro2 = {
	"snowbro2", NULL, NULL, NULL, "1994",
	"Snow Bros. 2 - With New Elves / Otenki Paradise (Hanafram)\0", NULL, "Hanafram", "Toaplan GP9001 based",
	L"\u96EA\u4EBA\u5144\u5F1F 2 - \u65B0\u7684\u7CBE\u7075 (Hanafram)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TOAPLAN_68K_ONLY, GBF_PLATFORM, 0,
	NULL, snowbro2RomInfo, snowbro2RomName, NULL, NULL, NULL, NULL, snowbro2InputInfo, snowbro2DIPInfo,
	Snowbro2Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	320, 240, 4, 3
};


// Snow Bros. 2 - With New Elves / Otenki Paradise (Nyanko)
// not a bootleg, has original parts (the "GP9001 L7A0498 TOA PLAN" IC and the three mask ROMs)

static struct BurnRomInfo snowbro2nyRomDesc[] = {
	{ "rom1_c8.u61",  			0x080000, 0x9e6eb76b, BRF_ESS | BRF_PRG }, //  0 CPU #0 code

	{ "rom2-l_tp-033.u13",		0x100000, 0xe9d366a9, BRF_GRA },		   //  1 GP9001 Tile data
	{ "rom2-h_c10.u26",     	0x080000, 0x9aab7a62, BRF_GRA },		   //  2
	{ "rom3-l_tp-033.u12",  	0x100000, 0xeb06e332, BRF_GRA },		   //  3
	{ "rom3-h_c9.u27",      	0x080000, 0x6de2b059, BRF_GRA },		   //  4

	{ "rom4-tp-033.u33",    	0x080000, 0x638f341e, BRF_SND },		   //  5 MSM6295 ADPCM data
	
	{ "13_gal16v8-25lnc.u91",	0x0117, 0x00000000, BRF_NODUMP },     	   	   //  6 PLDs
	{ "14_gal16v8-25lnc.u92",	0x0117, 0x00000000, BRF_NODUMP },     	   	   //  7
	{ "15_gal16v8-25lnc.u93",	0x0117, 0x00000000, BRF_NODUMP },     	   	   //  8
};


STD_ROM_PICK(snowbro2ny)
STD_ROM_FN(snowbro2ny)

struct BurnDriver BurnDrvSnowbro2ny = {
	"snowbro2ny", "snowbro2", NULL, NULL, "1994",
	"Snow Bros. 2 - With New Elves / Otenki Paradise (Nyanko)\0", NULL, "Nyanko", "Toaplan GP9001 based",
	L"\u96EA\u4EBA\u5144\u5F1F 2 - \u65B0\u7684\u7CBE\u7075 (Nyanko)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TOAPLAN_68K_ONLY, GBF_PLATFORM, 0,
	NULL, snowbro2nyRomInfo, snowbro2nyRomName, NULL, NULL, NULL, NULL, snowbro2InputInfo, snowbro2DIPInfo,
	Snowbro2Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	320, 240, 4, 3
};


// Snow Bros. 2 - With New Elves / Otenki Paradise (bootleg, set 1)

static struct BurnRomInfo snowbro2bRomDesc[] = {
	{ "sb2-prg1.u39",			0x040000, 0xe1fec8a2, BRF_PRG | BRF_ESS }, //  0 CPU #0 code
	{ "sb2-prg0.u23",			0x040000, 0xb473cd57, BRF_PRG | BRF_ESS }, //  1

	{ "sb2-gfx.u177",			0x200000, 0xebeec910, BRF_GRA },           //  2 GP9001 Tile data
	{ "sb2-gfx.u175",			0x200000, 0xe349c75b, BRF_GRA },           //  3

	{ "sb2-snd-4.u17",			0x080000, 0x638f341e, BRF_SND },           //  4 MSM6295 ADPCM data

	{ "sb2-unk.u100",			0x008000, 0x456dd16e, BRF_OPT },           //  5 Unknown
};


STD_ROM_PICK(snowbro2b)
STD_ROM_FN(snowbro2b)

static INT32 Snowbro2bLoadRoms()
{
	// Load 68000 ROM
	BurnLoadRom(Rom01 + 0, 1, 2);
	BurnLoadRom(Rom01 + 1, 0, 2);

	// Load GP9001 tile data
	ToaLoadGP9001Tiles(GP9001ROM[0], 2, 2, nGP9001ROMSize[0]);

	// Load MSM6295 ADPCM data
	BurnLoadRom(MSM6295ROM, 4, 1);

	return 0;
}

static INT32 Snowbro2bInit()
{
	return DrvInit(Snowbro2bLoadRoms);
}

struct BurnDriver BurnDrvSnowbro2b = {
	"snowbro2b", "snowbro2", NULL, NULL, "1998",
	"Snow Bros. 2 - With New Elves / Otenki Paradise (bootleg, set 1)\0", NULL, "bootleg", "Toaplan GP9001 based",
	L"\u96EA\u4EBA\u5144\u5F1F 2 - \u65B0\u7684\u7CBE\u7075 / \u5929\u6C14\u4E50\u56ED (\u76D7\u7248, \u7B2C\u4E00\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TOAPLAN_68K_ONLY, GBF_PLATFORM, 0,
	NULL, snowbro2bRomInfo, snowbro2bRomName, NULL, NULL, NULL, NULL, snowbro2InputInfo, snowbro2DIPInfo,
	Snowbro2bInit, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	320, 240, 4, 3
};


// Snow Bros. 2 - With New Elves / Otenki Paradise (bootleg, set 2)

static struct BurnRomInfo snowbro2b2RomDesc[] = {
	{ "rom10.bin",	0x80000, 0x3e96da41, BRF_PRG | BRF_ESS }, //  0 CPU #0 code

	{ "rom07.bin",	0x80000, 0xc54ae0b3, BRF_GRA },           //  1 GP9001 Tile data
	{ "rom05.bin",	0x80000, 0xaf3c74d1, BRF_GRA },           //  2
	{ "rom08.bin",	0x40000, 0x72812088, BRF_GRA },           //  3
	{ "rom06.bin",	0x40000, 0xc8f80774, BRF_GRA },           //  4
	{ "rom03.bin",	0x80000, 0x42fecbd7, BRF_GRA },           //  5
	{ "rom01.bin",	0x80000, 0xe7134937, BRF_GRA },           //  6
	{ "rom04.bin",	0x40000, 0x3343b7a7, BRF_GRA },           //  7
	{ "rom02.bin",	0x40000, 0xaf4d9551, BRF_GRA },           //  8

	{ "rom09.bin",	0x80000, 0x638f341e, BRF_SND },           //  9 MSM6295 ADPCM data
};


STD_ROM_PICK(snowbro2b2)
STD_ROM_FN(snowbro2b2)

static INT32 Snowbro2b2LoadRoms()
{
	// Load 68000 ROM
	BurnLoadRom(Rom01 + 0, 0, 1);

	// Load & decode GP9001 tile data
	{
		INT32 Plane[4] = { 0x180000*8+8, 0x180000*8+0, 8, 0 };
		INT32 XOffs[8] = { STEP8(0,1) };
		INT32 YOffs[8] = { STEP8(0,16) };

		UINT8 *tmp = (UINT8*)BurnMalloc(0x800000);

		BurnLoadRom(GP9001ROM[0] + 0x000000, 1, 2);
		BurnLoadRom(GP9001ROM[0] + 0x000001, 2, 2);
		BurnLoadRom(GP9001ROM[0] + 0x100000, 3, 2);
		BurnLoadRom(GP9001ROM[0] + 0x100001, 4, 2);
		BurnLoadRom(GP9001ROM[0] + 0x180000, 5, 2);
		BurnLoadRom(GP9001ROM[0] + 0x180001, 6, 2);
		BurnLoadRom(GP9001ROM[0] + 0x280000, 7, 2);
		BurnLoadRom(GP9001ROM[0] + 0x280001, 8, 2);

		GfxDecode(0x18000, 4, 8, 8, Plane, XOffs, YOffs, 0x080, GP9001ROM[0], tmp);

		for (INT32 i = 0; i < 0x600000; i+=2) {
			GP9001ROM[0][i/2] = (tmp[i+1] << 4) | (tmp[i+0] & 0xf);
		}

		BurnFree (tmp);
	}
	
	// Load MSM6295 ADPCM data
	BurnLoadRom(MSM6295ROM, 9, 1);

	return 0;
}

static INT32 Snowbro2b2Init()
{
	return DrvInit(Snowbro2b2LoadRoms);
}

struct BurnDriver BurnDrvSnowbro2b2 = {
	"snowbro2b2", "snowbro2", NULL, NULL, "1994",
	"Snow Bros. 2 - With New Elves / Otenki Paradise (bootleg, set 2)\0", NULL, "bootleg (Q Elec)", "Toaplan GP9001 based",
	L"\u96EA\u4EBA\u5144\u5F1F 2 - \u65B0\u7684\u7CBE\u7075 / \u5929\u6C14\u4E50\u56ED (\u76D7\u7248, \u7B2C\u4E8C\u5957)\0", NULL, NULL, NULL,
	BDF_GAME_WORKING | BDF_CLONE | BDF_BOOTLEG | BDF_HISCORE_SUPPORTED, 4, HARDWARE_TOAPLAN_68K_ONLY, GBF_PLATFORM, 0,
	NULL, snowbro2b2RomInfo, snowbro2b2RomName, NULL, NULL, NULL, NULL, snowbro2InputInfo, snowbro2DIPInfo,
	Snowbro2b2Init, DrvExit, DrvFrame, DrvDraw, DrvScan, &ToaRecalcPalette, 0x800,
	320, 240, 4, 3
};