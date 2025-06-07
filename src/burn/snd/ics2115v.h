
extern unsigned char ics2115vread(unsigned char offset);
extern void ics2115vwrite(unsigned char offset, unsigned char data);

extern void ics2115v_init(INT32 cpu_clock, void (*cpu_irq_cb)(INT32), UINT8* sample_rom, INT32 sample_rom_size);
extern void ics2115v_exit();
extern void ics2115v_reset();

extern void ics2115v_adjust_timer(INT32 ticks);
extern void ics2115v_update(INT16* outputs, int samples);
extern void ics2115v_scan(INT32 nAction, INT32* /*pnMin*/);
