#include <linux/soundcard.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <stdio.h>
#include <stdlib.h>
#include "midi.h"

int
note(void)
{
	char *device = "/dev/midi2";
	unsigned char g_on[3] = {0x90, 0x43, 0x40};
	unsigned char g_off[3] = {0x80, 0x43, 0x00};
	int f = open(device, O_WRONLY, 0);
	if(f < 0)
		return 0;
	printf("Note ON\n");
	if(!write(f, g_on, sizeof(g_on)))
		return 0;
	sleep(2);
	printf("Note OFF\n");
	if(!write(f, g_off, sizeof(g_off)))
		return 0;
	close(f);
	return 1;
}
