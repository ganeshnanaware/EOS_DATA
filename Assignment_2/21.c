/* Attempt to read from and write to a selected address in the data (R/W)
 * segment.  The signal handler keeps an invalid address from terminating the
 * program, which also makes the experiment easier to observe. */

#include <inttypes.h>
#include <setjmp.h>
#include <signal.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

static volatile unsigned char data_segment_byte = 'A';
static sigjmp_buf recovery_point;

static void fault_handler(int signal_number)
{
	(void)signal_number;
	siglongjmp(recovery_point, 1);
}

int main(void)
{
	struct sigaction action = {0};
	char input[64];
	uintptr_t address;
	volatile unsigned char *selected;

	action.sa_handler = fault_handler;  
	sigemptyset(&action.sa_mask);
	action.sa_flags = 0;
	sigaction(SIGSEGV, &action, NULL);
	sigaction(SIGBUS, &action, NULL);

	printf("A data-segment byte is at %p and contains '%c'.\n",
		   (void *)&data_segment_byte, data_segment_byte);
	printf("Enter an address to access (hex, or press Enter for the above): ");
	if (!fgets(input, sizeof input, stdin))
		return EXIT_FAILURE;

	if (input[0] == '\n' || input[0] == '\0') {
		address = (uintptr_t)&data_segment_byte;
	} else {
		char *end;
		address = (uintptr_t)strtoumax(input, &end, 0);
		if (end == input) {
			fprintf(stderr, "Invalid address.\n");
			return EXIT_FAILURE;
		}
	}

	selected = (volatile unsigned char *)address;
	if (sigsetjmp(recovery_point, 1) == 0) {
		unsigned char old_value = *selected;
		printf("Read succeeded: address %p contains 0x%02X.\n",
			   (void *)selected, old_value);
		*selected = old_value ^ 0xFF;
		printf("Write succeeded; restoring the original value.\n");
		*selected = old_value;
	} else {
		printf("Access fault: address %p is not readable/writable.\n",
			   (void *)selected);
	}

	return EXIT_SUCCESS;
}
