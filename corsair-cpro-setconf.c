#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <hidapi/hidapi.h>

void printBuffer(unsigned char *buffer, int length)
{
	int x;

	for (x = 0; x < length; x++)
	{
		fprintf(stderr, "%02x ", buffer[x]);
		if ((x + 1) % 16 == 0 && (x + 1) != length)
			printf("\n");
	}
	fprintf(stderr, "\n-DEZ:-\n");
	for (x = 0; x < length; x++)
	{
		fprintf(stderr, "%d ", buffer[x]);
		if ((x + 1) % 16 == 0 && (x + 1) != length)
			printf("\n");
	}
	printf("\n--------------------\n");
}

void clear_buffer(unsigned char *buffer)
{
	int i = 0;

	for (; i < 63; ++i)
	    buffer[i] = 0x00;

	return;
}

void printUsage() {
	fprintf(stderr, "Usage: -f fan -m mode\n");
	fprintf(stderr, "Modes: 0 = auto, 1 = 3pin, 2 = 4pin, 3 = disconnected\n");
	return;
}

int main(int argc, char* argv[])
{
	int opt;
	int fan_number = -1;
	int fan_mode = -1;
	int res;
	unsigned char buffer[63];

	hid_device *handle;
	
	while ((opt = getopt(argc, argv, "f:m:")) != -1) {
		switch (opt) {
			case 'f':
				fan_number = atoi(optarg) - 1;
				break;
			case 'm':
				fan_mode = atoi(optarg);
				
				break;
			default:
				printUsage();
				return 1;
				break;
		}
	}
	
	if (fan_number < 0 || fan_number > 5) {
		printUsage();
		return 1;
	}
	
	if (fan_mode < 0 || fan_mode > 3) {
		printUsage();
		return 1;
	}
	
	res = hid_init();
	if (res) {
		fprintf(stderr, "Could not initialize hid_api\n");
		return 1;
	}
	
	handle = hid_open(0x1b1c, 0x0c10, NULL); // Try Corsair Commander Pro
	if (handle == NULL) {
		handle = hid_open(0x1b1c, 0x1d00, NULL); // Try Corsair 1000D
		if (handle == NULL) {
			fprintf(stderr, "Could not find device\n");
			return 1;
		}
	}

	clear_buffer(buffer);
	buffer[0] = 0x28; // Set fan config
	buffer[1] = 0x02; // Set fan config
	buffer[2] = fan_number; // Fan number - 1 e.g. fan6 -> 5
	buffer[3] = fan_mode; // Mode, 0x01 -> Voltage Control, 0x02 -> PWM Control

	res = hid_write(handle, buffer, 63);
	if (res == -1) {
		fprintf(stderr, "Could not write to device\n");
		return 1;
	}
	
	res = hid_read(handle, buffer, 16);
	if (res == -1) {
		fprintf(stderr, "Could not read from device\n");
	}
	if (buffer[0] != 0) {
		fprintf(stderr, "Device error:\n");
		printBuffer(buffer, 16);
	}

    return 0;
}
