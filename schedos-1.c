#include "schedos-app.h"
#include "x86sync.h"

/*****************************************************************************
 * schedos-1
 *
 *   This tiny application prints red "1"s to the console.
 *   It yields the CPU to the kernel after each "1" using the sys_yield()
 *   system call.  This lets the kernel (schedos-kern.c) pick another
 *   application to run, if it wants.
 *
 *   The other schedos-* processes simply #include this file after defining
 *   PRINTCHAR appropriately.
 *
 *****************************************************************************/

#ifndef PRINTCHAR
#define PRINTCHAR	('1' | 0x0C00)
#endif

// preprocessor signal for alternate synchronization
#define ALTSYNC

#ifndef PRIORITY
#define PRIORITY 6
#endif

#ifndef SHARE
#define SHARE 10
#endif

void
start(void)
{
	sys_setpriority(PRIORITY);	// set process priority
	sys_setshare(SHARE);		// set process share
	sys_yield();				// yield after setting priority/shares

	int i;
	for (i = 0; i < RUNCOUNT; i++) {
		// Write characters to the console, yielding after each one.

		#ifndef ALTSYNC // primary sync method
			sys_print(PRINTCHAR);
		#endif
		#ifdef ALTSYNC // alternate sync method
			// attempt get write lock
			while (atomic_swap(&write_lock, 1) != 0)
				continue;

			// write
			*cursorpos++ = PRINTCHAR;

			// release write lock
			atomic_swap(&write_lock, 0);
		#endif
		sys_yield();
	}

	sys_exit(0);
}
