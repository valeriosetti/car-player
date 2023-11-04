#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <endian.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>

//#define DEBUG

#define BIT(n)					(1UL << n)
#define DATA_BLOCK_SIZE			25 // bytes
#define MAX_FINGERS				5
#define DEFAULT_HIDRAW_DEVICE	"/dev/hidraw0"
#define DEFAULT_UINPUT_DEVICE	"/dev/uinput"
#define MAX_X_VALUE				800
#define MAX_Y_VALUE				480


/* Here's how raw input data should be captured for analysis:
 * 		xxd -g1 -c25 /dev/hidraw0
 * 
 * And here's how this application can be tested:
 * 		evtest /dev/input/event1
 */

struct finger_pos {
	uint16_t x;
	uint16_t y;
} __attribute__((packed));

struct data_format {
	uint8_t _unused1; //aa
	uint8_t first_finger_touch;
	struct finger_pos first_finger_pos;
	uint8_t _unused2; //bb
	uint8_t fingers_mask; // 1 bit for each finger
	struct finger_pos others_fingers_pos[MAX_FINGERS-1];
	uint8_t _unused3; //cc
} __attribute__((packed));

struct finger_data {
	struct finger_pos pos;
	uint8_t was_present:1;
	uint8_t is_present:1;
};

struct finger_data fingers_data[MAX_FINGERS];

FILE* hidraw_fp;
int uinput_fd;

void quit_app(void)
{
	if (hidraw_fp != NULL) {
		fclose(hidraw_fp);
	}
	if (uinput_fd > 0) {
		close(uinput_fd);
	}
	abort();
}

void create_input_device(void)
{
	struct uinput_setup usetup;
	struct uinput_abs_setup abs_setup;
	
	ioctl(uinput_fd, UI_SET_EVBIT, EV_ABS);
	ioctl(uinput_fd, UI_SET_ABSBIT, ABS_MT_SLOT);
	ioctl(uinput_fd, UI_SET_ABSBIT, ABS_MT_TRACKING_ID);
	ioctl(uinput_fd, UI_SET_ABSBIT, ABS_MT_POSITION_X);
	ioctl(uinput_fd, UI_SET_ABSBIT, ABS_MT_POSITION_Y);

	memset(&usetup, 0, sizeof(usetup));
	usetup.id.bustype = BUS_USB;
	usetup.id.vendor = 0x0eef;
	usetup.id.product = 0x0005;
	strcpy(usetup.name, "Waveshare touch");

	ioctl(uinput_fd, UI_DEV_SETUP, &usetup);
	
	memset(&abs_setup, 0, sizeof(abs_setup));
	abs_setup.code = ABS_MT_POSITION_X;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = MAX_X_VALUE;
	abs_setup.absinfo.resolution = 1;
	ioctl(uinput_fd, UI_ABS_SETUP, &abs_setup);
	
	memset(&abs_setup, 0, sizeof(abs_setup));
	abs_setup.code = ABS_MT_POSITION_Y;
	abs_setup.absinfo.minimum = 0;
	abs_setup.absinfo.maximum = MAX_Y_VALUE;
	abs_setup.absinfo.resolution = 1;
	ioctl(uinput_fd, UI_ABS_SETUP, &abs_setup);
	
	ioctl(uinput_fd, UI_DEV_CREATE);
}

void emit(int fd, int type, int code, int val)
{
   struct input_event ie;

   ie.type = type;
   ie.code = code;
   ie.value = val;
   /* timestamp values below are ignored */
   ie.time.tv_sec = 0;
   ie.time.tv_usec = 0;

   write(fd, &ie, sizeof(ie));
}

void update_fingers_status(void)
{
	int i;
	struct finger_data* curr_finger;
	
	for (i=0; i<MAX_FINGERS; i++) {
		curr_finger = &fingers_data[i];
		// select slot
		emit(uinput_fd, EV_ABS, ABS_MT_SLOT, i);
		// check if finger status changed
		if (curr_finger->is_present != curr_finger->was_present) {
			// is finger pressed pressed?
			if (curr_finger->is_present) {
				emit(uinput_fd, EV_ABS, ABS_MT_TRACKING_ID, i);
				// send finger position
				emit(uinput_fd, EV_ABS, ABS_MT_POSITION_X, curr_finger->pos.x);
				emit(uinput_fd, EV_ABS, ABS_MT_POSITION_Y, curr_finger->pos.y);
			} else { // finger released
				emit(uinput_fd, EV_ABS, ABS_MT_TRACKING_ID, -1);
			}
			// update status
			fingers_data[i].was_present = fingers_data[i].is_present;
		} else {
			// finger status is not changed, so just send new coordinates
			// in case it's stil pressed
			if (curr_finger->is_present) {
				emit(uinput_fd, EV_ABS, ABS_MT_POSITION_X, curr_finger->pos.x);
				emit(uinput_fd, EV_ABS, ABS_MT_POSITION_Y, curr_finger->pos.y);
			}
		}
	}
	
	emit(uinput_fd, EV_SYN, SYN_REPORT, 0);
}

int main(int argc, char *argv[])
{
	struct data_format read_data;
	int i;
#ifdef DEBUG
	char output_string[128];
	char* output_string_pos;
#endif //DEBUG
	
	hidraw_fp = fopen(DEFAULT_HIDRAW_DEVICE, "rb");
	if (hidraw_fp == NULL) {
		fprintf(stderr, "Error: unable to open input device %s\n", DEFAULT_HIDRAW_DEVICE);
		quit_app();
	}
	
	uinput_fd = open(DEFAULT_UINPUT_DEVICE, O_WRONLY | O_NONBLOCK);
	if (uinput_fd < 0) {
		fprintf(stderr, "Error: unable to open input device %s\n", DEFAULT_UINPUT_DEVICE);
		quit_app();
	}
	
	create_input_device();
	
	while (1) {
		// when there are no finger on the screen this read is blocking
		if (fread(&read_data, sizeof(read_data), 1, hidraw_fp) != 1) {
			fprintf(stderr, "Error: unable to read from input device %s\n", DEFAULT_HIDRAW_DEVICE);
			quit_app();
		}
		
		#ifdef DEBUG
		memset(output_string, 0, sizeof(output_string));
		output_string_pos = output_string;
		#endif //DEBUG
		
		for (i=0; i<MAX_FINGERS; i++) {
			if (i == 0) {
				fingers_data[i].pos.x = be16toh(read_data.first_finger_pos.x);
				fingers_data[i].pos.y = be16toh(read_data.first_finger_pos.y);
			} else {
				fingers_data[i].pos.x = be16toh(read_data.others_fingers_pos[i-1].x);
				fingers_data[i].pos.y = be16toh(read_data.others_fingers_pos[i-1].y);
			}
			
			if (read_data.fingers_mask & BIT(i)) {
				fingers_data[i].is_present = 1;
				#ifdef DEBUG
				output_string_pos += sprintf(output_string_pos, "X=%03d Y=%03d | ", curr_finger.x, curr_finger.y);
				#endif //DEBUG
			} else {
				fingers_data[i].is_present = 0;
				#ifdef DEBUG
				output_string_pos += sprintf(output_string_pos, "X=--- Y=--- | ");
				#endif //DEBUG
			}
		}
		
		#ifdef DEBUG
		fprintf(stdout, "%s\n", output_string);
		#endif //DEBUG
		
		update_fingers_status();
	}
	
	fclose(hidraw_fp);
	close(uinput_fd);
	
	return 0;
};
