#ifndef _XBOOK_POWER_H
#define _XBOOK_POWER_H

/*
 * Magic values required to use _reboot() system call.
 */
#define	XBOOK_REBOOT_MAGIC1	0xfee1dead
#define	XBOOK_REBOOT_MAGIC2	672274793
#define	XBOOK_REBOOT_MAGIC2A	85072278
#define	XBOOK_REBOOT_MAGIC2B	369367448
/*
 * Commands accepted by the _reboot() system call.
 *
 * RESTART     Restart system using default command and mode.
 * HALT        Stop OS and give system control to ROM monitor, if any.
 * CAD_ON      Ctrl-Alt-Del sequence causes RESTART command.
 * CAD_OFF     Ctrl-Alt-Del sequence sends SIGINT to init task.
 * POWER_OFF   Stop OS and remove all power from system, if possible.
 * RESTART2    Restart system using given command string.
 */
#define	XBOOK_REBOOT_CMD_RESTART	0x01234567
#define	XBOOK_REBOOT_CMD_HALT		0xCDEF0123
#define	XBOOK_REBOOT_CMD_CAD_ON		0x89ABCDEF
#define	XBOOK_REBOOT_CMD_CAD_OFF	0x00000000
#define	XBOOK_REBOOT_CMD_POWER_OFF	0x4321FEDC
#define	XBOOK_REBOOT_CMD_RESTART2	0xA1B2C3D4

int sys_reboot(int magic, int magic2, int cmd);
void do_reboot();
void do_halt();
void do_poweroff();

#endif   /* _XBOOK_POWER_H */
