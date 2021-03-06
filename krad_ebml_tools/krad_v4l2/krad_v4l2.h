#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
#include <getopt.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <malloc.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <math.h>
#include <asm/types.h>
#include <linux/videodev2.h>

#include <turbojpeg.h>

#define DEFAULT_V4L2_DEVICE "/dev/video0"

#define CLEAR(x) memset (&(x), 0, sizeof (x))

typedef struct krad_v4l2_ret_buffer_St krad_v4l2_ret_buffer_t;
typedef struct krad_v4l2_buffer_St krad_v4l2_buffer_t;
typedef struct krad_v4l2_St krad_v4l2_t;

typedef enum {
	IO_METHOD_READ,
	IO_METHOD_MMAP,
	IO_METHOD_USERPTR,
} krad_v4l2_io_method;

struct krad_v4l2_buffer_St {
	void *start;
	size_t length;
};

struct krad_v4l2_ret_buffer_St {
	void *data;
	size_t length;
	struct timeval timestamp;
};

struct krad_v4l2_St {

	int width;
	int height;
	int fps;


	int frames;
	
	int fd;
	
	struct timeval timestamp;
	krad_v4l2_ret_buffer_t ret_buffer;
	
	krad_v4l2_buffer_t *buffers;
	unsigned int n_buffers;
	struct v4l2_buffer buf;
	
	char device[512];

	krad_v4l2_io_method io;

	tjhandle jpeg_dec;

	unsigned int jpeg_size;


	unsigned char *jpeg_buffer;

};



/* public */

void kradv4l2_jpeg_to_rgb (krad_v4l2_t *kradv4l2, unsigned char *argb_buffer, unsigned char *jpeg_buffer, unsigned int jpeg_size);

krad_v4l2_t *kradv4l2_create();
void kradv4l2_destroy(krad_v4l2_t *kradv4l2);

void kradv4l2_close (krad_v4l2_t *kradv4l2);
void kradv4l2_open (krad_v4l2_t *kradv4l2, char *device, int width, int height, int fps);

void kradv4l2_stop_capturing (krad_v4l2_t *kradv4l2);
void kradv4l2_start_capturing (krad_v4l2_t *kradv4l2);

char *kradv4l2_read_frame_wait (krad_v4l2_t *kradv4l2);
char *kradv4l2_read_frame (krad_v4l2_t *kradv4l2);
void kradv4l2_read_frames (krad_v4l2_t *kradv4l2);
char *kradv4l2_read_frame_adv (krad_v4l2_t *kradv4l2);
char *kradv4l2_read_frame_wait_adv (krad_v4l2_t *kradv4l2);
void kradv4l2_frame_done (krad_v4l2_t *kradv4l2);

/* private */

void kradv4l2_init_device (krad_v4l2_t *kradv4l2);
void kradv4l2_uninit_device (krad_v4l2_t *kradv4l2);

void kradv4l2_init_userp (krad_v4l2_t *kradv4l2, unsigned int buffer_size);
void kradv4l2_init_mmap (krad_v4l2_t *kradv4l2);
void kradv4l2_init_read (krad_v4l2_t *kradv4l2, unsigned int buffer_size);

void errno_exit (const char *s);
int xioctl (int fd, int request, void *arg);
