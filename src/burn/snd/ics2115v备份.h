
extern UINT8 ics2115vread(UINT8 offset);
extern void ics2115vwrite(UINT8 offset, UINT8 data);

extern INT32 ics2115v_init();
extern void ics2115v_exit();
extern void ics2115v_reset();

extern void ics2115v_frame();
extern void ics2115v_update(INT32 length);
extern void ics2115v_scan(INT32 nAction,INT32 *pnMin);
