/******************************************************
 ICS 2115 sound synthesizer.

   ICS WaveFront ics2115V Wavetable Midi Synthesizer,
 clocked at 33.8688MHz

 Original ics2115.c in MAME
   By O. Galibert, with a lot of help from the nebula
   ics emulation by Elsemi.

 Port to FB Alpha by OopsWare
 ******************************************************/

#include "burnint.h"
#include "z80_intf.h"
#include "ics2115v.h"

static UINT8* m_rom = NULL; // ics2115 rom
static INT32 m_rom_len;
static INT32 sound_cpu_clock = 0;
static void (*m_irq_cb)(INT32) = NULL; // cpu irq callback

enum { V_ON = 1, V_DONE = 2 };

struct ics2115v {
	UINT8* rom;
	INT16 ulaw[256];
	struct {
		UINT16 fc, addrh, addrl, strth, endh, volacc;
		UINT8 strtl, endl, saddr, pan, conf, ctl;
		UINT8 vstart, vend, vctl;
		UINT8 state;
	} voice[32];
	struct {
		UINT8 scale, preset; 
		bool active;
		UINT32 period;
		INT32 cycles_left;
		INT32 cycles_total;
		void (*timer_cb)(INT32);
	} timer[2];
	UINT8 reg, osc;
	UINT8 irq_en, irq_pend;
	INT32 irq_on;
	//sound_stream * stream;
};

static struct ics2115v* chip = NULL;
static INT16* sndbuffer = NULL;

#define	ics2115v_RATE				33075
#define ics2115v_FRAME_BUFFER_SIZE	(ics2115v_RATE / 60 + 1)

static UINT32 nSoundDelta;

/* ics2115vV chip emu */

static void recalc_irq()
{
	INT32 irq = 0;
	if (chip->irq_en & chip->irq_pend) irq = 1;
	for (INT32 i = 0; !irq && i < 32; i++)
		if (chip->voice[i].state & V_DONE) irq = 1;
	if (irq != chip->irq_on) {
		chip->irq_on = irq;
		if (m_irq_cb)
			m_irq_cb(irq ? CPU_IRQSTATUS_ACK : CPU_IRQSTATUS_NONE);
	}
}

void timer_cb_0(INT32 param)
{
	chip->irq_pend |= 1 << 0;
	recalc_irq();
}

void timer_cb_1(INT32 param)
{
	chip->irq_pend |= 1 << 1;
	recalc_irq();
}

void recalc_timer(INT32 timer)
{
	float period = 0;

	if (chip->timer[timer].scale) {
		INT32 sc = chip->timer[timer].scale;
		UINT64 counter = ((sc & 31) + 1) * (chip->timer[timer].preset + 1);
		counter = (counter << (4 + (sc >> 5))) * 78125 / 2646;
		period = (float)counter;

		if (period) {
			float cycles_until_irq = sound_cpu_clock * ((counter * 1.0) / 1000000000.0);
			chip->timer[timer].cycles_left = (INT32)cycles_until_irq;
			chip->timer[timer].cycles_total = chip->timer[timer].cycles_left;
		}
	} else {
		period = 0;
	}

	if (chip->timer[timer].period != (UINT32)period) {
		chip->timer[timer].period = (UINT32)period;
		if (period) chip->timer[timer].active = true;
		else  chip->timer[timer].active = false;
	}
}

// add-on function to deal with timer.
void ics2115v_adjust_timer(INT32 ticks)
{
	for (INT32 i = 0; i < 2; i++) {
		if (chip->timer[i].active) {
			chip->timer[i].cycles_left -= ticks;
			if (chip->timer[i].cycles_left <= 0) {
				chip->timer[i].timer_cb(0);
				chip->timer[i].cycles_left = chip->timer[i].cycles_total;
			}
		}
	}
}

UINT16 ics2115vread_reg(UINT8 reg)
{
	switch (reg) {
	case 0x00: // [osc] Oscillator Configuration
		return chip->voice[chip->osc].conf << 8;
	case 0x0d: // [osc] Volume Enveloppe Control
		return 0x100;
	case 0x0E: // Active Voices
		return 0x20;
	case 0x0f: {// [osc] Interrupt source/oscillator
		UINT8 res = 0xff;
		for (INT32 osc = 0; osc < 32; osc++) {
			if (chip->voice[osc].state & V_DONE) {
				chip->voice[osc].state &= ~V_DONE;
				recalc_irq();
				res = 0x40 | osc; // 0x40 ? 0x80 ?
				break;
			}
		}
		return res << 8;
	}
	case 0x10: // [osc] Oscillator Control
		//ics2115_update(get_stream_pos());
		return chip->voice[chip->osc].ctl << 8;
	case 0x40: // Timer 0 clear irq
		chip->irq_pend &= ~(1 << 0);
		recalc_irq();
		return chip->timer[0].preset;
	case 0x41: // Timer 1 clear irq
		chip->irq_pend &= ~(1 << 1);
		recalc_irq();
		return chip->timer[1].preset;
	case 0x43: // Timer status
		return chip->irq_pend & 3;
	case 0x4a: // IRQ Pending
		return chip->irq_pend;
	case 0x4b: // Address of Interrupting Oscillator
		return 0x80;
	case 0x4c: // Chip revision
		return 0x01;
	default:
		return 0;
	}
}

void ics2115vwrite_reg(UINT8 reg, UINT8 data, INT32 msb)
{
	//	bprintf(PRINT_NORMAL, _T("ics2115vwrite_reg(%02x, %02x, %d);  %4.1f%%\n"), reg, data, msb, 6.0 * ZetTotalCycles() / 8468.0 ); 

	switch (reg) {
	case 0x00: // [osc] Oscillator Configuration
		if (msb) {
			// bprintf(0, ("    ICS2115 voice %02X osc conf -> %02X\n"), m_osc_select, data);

			if (~data & 0x20)
			{
				/* if (voice.osc_conf.bitflags.irq)
					bprintf(0, ("    ICS2115 voice %02X wavetable irq disabled\n"), m_osc_select); */

				chip->voice[chip->osc].conf = data & 0x7f;

				if (chip->irq_on)
					recalc_irq();
			}
			else
			{
				/* if (!voice.osc_conf.bitflags.irq)
					bprintf(0, ("    ICS2115 voice %02X wavetable irq enabled\n"), m_osc_select); */

				chip->voice[chip->osc].conf = data;

				if ((chip->voice[chip->osc].conf & 0x80) != (data & 0x80))
					recalc_irq();
			}
		}
		break;
	case 0x01: // [osc] Wavesample frequency
		if (msb)	chip->voice[chip->osc].fc = (chip->voice[chip->osc].fc & 0xff) | (data << 8);
		else	chip->voice[chip->osc].fc = (chip->voice[chip->osc].fc & 0xff00) | data;
		//bprintf(PRINT_NORMAL, _T("ics2115v: %2d: fc = %04x (%dHz)\n"), chip->osc,chip->voice[chip->osc].fc, chip->voice[chip->osc].(fc*33075+512)/1024);
		break;
	case 0x02: // [osc] Wavesample loop start address 19-4
		if (msb)	chip->voice[chip->osc].strth = (chip->voice[chip->osc].strth & 0xff) | (data << 8);
		else	chip->voice[chip->osc].strth = (chip->voice[chip->osc].strth & 0xff00) | data;
		break;
	case 0x03: // [osc] Wavesample loop start address 3-0.3-0
		if (msb) chip->voice[chip->osc].strtl = data;
		break;
	case 0x04: // [osc] Wavesample loop end address 19-4
		if (msb)	chip->voice[chip->osc].endh = (chip->voice[chip->osc].endh & 0xff) | (data << 8);
		else	chip->voice[chip->osc].endh = (chip->voice[chip->osc].endh & 0xff00) | data;
		break;
	case 0x05: // [osc] Wavesample loop end address 3-0.3-0
		if (msb)	chip->voice[chip->osc].endl = data;
		break;
	case 0x07: // [osc] Volume Start
		if (msb) chip->voice[chip->osc].vstart = data;
		break;
	case 0x08: // [osc] Volume End
		if (msb) chip->voice[chip->osc].vend = data;
		break;
	case 0x09: // [osc] Volume accumulator
		if (msb)	chip->voice[chip->osc].volacc = (chip->voice[chip->osc].volacc & 0xff) | (data << 8);
		else	chip->voice[chip->osc].volacc = (chip->voice[chip->osc].volacc & 0xff00) | data;
		//bprintf(PRINT_NORMAL, _T("ics2115v: %2d: volacc = %04x\n"), chip->osc,chip->voice[chip->osc].volacc);
		break;
	case 0x0a: // [osc] Wavesample address 19-4
		if (msb)	chip->voice[chip->osc].addrh = (chip->voice[chip->osc].addrh & 0xff) | (data << 8);
		else	chip->voice[chip->osc].addrh = (chip->voice[chip->osc].addrh & 0xff00) | data;
		break;
	case 0x0b: // [osc] Wavesample address 3-0.8-0
		if (msb)	chip->voice[chip->osc].addrl = (chip->voice[chip->osc].addrl & 0xff) | (data << 8);
		else	chip->voice[chip->osc].addrl = (chip->voice[chip->osc].addrl & 0xff00) | data;
		break;
	case 0x0c: // [osc] Pan
		if (msb) chip->voice[chip->osc].pan = data;
		break;
	case 0x0d: // [osc] Volume Enveloppe Control
		if (msb) chip->voice[chip->osc].vctl = data;
		break;
	case 0x10: // [osc] Oscillator Control
		if (msb) {
			chip->voice[chip->osc].ctl = data;
			if (data == 0) {
				chip->voice[chip->osc].state |= V_ON;
				//				bprintf(PRINT_NORMAL, _T("ics2115v: KEYON %2d volacc = %04x fc = %04x (%dHz)\n"),
				//						chip->osc, chip->voice[chip->osc].volacc, chip->voice[chip->osc].fc, (chip->voice[chip->osc].fc*33075 + 512) / 1024  );
			}
		} break;
	case 0x11: // [osc] Wavesample static address 27-20
		if (msb) chip->voice[chip->osc].saddr = data;
		break;
	case 0x40: // Timer 1 Preset
		if (!msb) {
			chip->timer[0].preset = data;
			recalc_timer(0);
		} break;
	case 0x41: // Timer 2 Preset
		if (!msb) {
			chip->timer[1].preset = data;
			recalc_timer(1);
		} break;
	case 0x42: // Timer 1 Prescaler
		if (!msb) {
			chip->timer[0].scale = data;
			recalc_timer(0);
		} break;
	case 0x43: // Timer 2 Prescaler
		if (!msb) {
			chip->timer[1].scale = data;
			recalc_timer(1);
		} break;
	case 0x4a: // IRQ Enable
		if (!msb) {
			chip->irq_en = data;
			recalc_irq();
		} break;
	case 0x4f: // Oscillator Address being Programmed
		if (!msb) chip->osc = data & 31;
		break;
	}
}

UINT8 ics2115vread(UINT8 offset)
{
	switch (offset) {
	case 0x00: {
		UINT8 res = 0;
		if (chip->irq_on) {
			res |= 0x80;
			if (chip->irq_en & chip->irq_pend & 3) res |= 1; // Timer irq
			for (INT32 i = 0; i < 32; i++)
				if (chip->voice[i].state & V_DONE) {
					res |= 2;
					break;
				}
		}
		return res;
	}
	case 0x01:
		return chip->reg;
	case 0x02:
		return ics2115vread_reg(chip->reg) & 0xff;
	case 0x03:
	default:
		return ics2115vread_reg(chip->reg) >> 8;
	}
}

void ics2115vwrite(UINT8 offset, UINT8 data)
{
	switch (offset) {
	case 0x01:
		chip->reg = data;
		break;
	case 0x02:
		ics2115vwrite_reg(chip->reg, data, 0);
		break;
	case 0x03:
		ics2115vwrite_reg(chip->reg, data, 1);
		break;
	default:
		break;
	}
}

static void recalculate_ulaw()
{
	for (INT32 i = 0; i < 256; i++) {
		UINT8 c = ((~i) & 0xFF);
		INT32 v = ((c & 15) << 1) + 33;
		v <<= ((c & 0x70) >> 4);
		if (c & 0x80) v = 33 - v;
		else		 v = v - 33;
		chip->ulaw[i] = v;
	}
}

void ics2115v_init(INT32 cpu_clock, void (*cpu_irq_cb)(INT32), UINT8* sample_rom, INT32 sample_rom_size)
{
	chip = (struct ics2115v*)BurnMalloc(sizeof(struct ics2115v));	// ics2115vV
	if (chip == NULL) return;

	sndbuffer = (INT16*)BurnMalloc(ics2115v_FRAME_BUFFER_SIZE * sizeof(INT16*));
	if (sndbuffer == NULL) return;

	m_rom = sample_rom;
	m_rom_len = sample_rom_size;
	sound_cpu_clock = cpu_clock;
	m_irq_cb = cpu_irq_cb;

	return;
}

void ics2115v_exit()
{
	BurnFree(chip);
	m_rom = NULL;
	m_rom_len = 0;
	sound_cpu_clock = 0;
	m_irq_cb = NULL;
	BurnFree(sndbuffer);
}

void ics2115v_reset()
{
	memset(chip, 0, sizeof(struct ics2115v));

	chip->rom = m_rom;

	if (nBurnSoundLen) {
		nSoundDelta = ics2115v_FRAME_BUFFER_SIZE * 0x10000 / nBurnSoundLen;
	}
	else {
		nSoundDelta = ics2115v_FRAME_BUFFER_SIZE * 0x10000 / 184; // 11025Hz
	}

	recalculate_ulaw();

	chip->timer[0].timer_cb = timer_cb_0;
	chip->timer[1].timer_cb = timer_cb_1;
}

void ics2115v_update(INT16* outputs, int samples)
{
	INT32 rec_irq = 0;

	//short* pSoundBuf = pBurnSoundOut;

	memset(sndbuffer, 0, ics2115v_FRAME_BUFFER_SIZE * sizeof(INT16));

	for (INT32 osc = 0; osc < 32; osc++)
		if (chip->voice[osc].state & V_ON) {
			UINT32 badr = (chip->voice[osc].saddr << 20) & 0x0f00000;

			UINT32 adr = (chip->voice[osc].addrh << 16) | chip->voice[osc].addrl;
			UINT32 end = (chip->voice[osc].endh << 16) | (chip->voice[osc].endl << 8);
			UINT32 loop = (chip->voice[osc].strth << 16) | (chip->voice[osc].strtl << 8);
			UINT32 conf = chip->voice[osc].conf;
			INT32 vol = chip->voice[osc].volacc;
			vol = (((vol & 0xff0) | 0x1000) << (vol >> 12)) >> 12;
			UINT32 delta = chip->voice[osc].fc << 2;

			for (INT32 i = 0; i < ics2115v_FRAME_BUFFER_SIZE; i++) {
				UINT32 address = badr + (adr / 0x1000);
				INT32 v = 0;

				if (address < m_rom_len)
					v = chip->rom[address];

				if (conf & 1)v = chip->ulaw[v];
				else		v = ((INT8)v) << 6;

				v = (v * vol) >> (16 + 5);

				sndbuffer[i] += v;

				adr += delta;
				if (adr >= end) {
					//if (ics2115vLOGERROR) logerror("ics2115v: KEYDONE %2d\n", osc);
					adr -= (end - loop);
					chip->voice[osc].state &= ~V_ON;
					chip->voice[osc].state |= V_DONE;
					rec_irq = 1;
					break;
				}
			}

			chip->voice[osc].addrh = adr >> 16;
			chip->voice[osc].addrl = adr;
		}

	if (rec_irq) recalc_irq();

	if (outputs) {
		INT32 pos = 0;
		INT16* pOut = (INT16*)outputs;
		for (INT32 i = 0; i < nBurnSoundLen; i++, pOut += 2, pos += nSoundDelta)
			pOut[0] = pOut[1] = sndbuffer[pos >> 16] << 4;
	}

}

void ics2115v_scan(INT32 nAction, INT32* /*pnMin*/)
{
	struct BurnArea ba;

	if (nAction & ACB_DRIVER_DATA) {
		UINT8* rom = chip->rom;

		ba.Data = chip;
		ba.nLen = sizeof(struct ics2115v);
		ba.nAddress = 0;
		ba.szName = "ICS 2115";
		BurnAcb(&ba);

		chip->rom = rom;
	}
}
