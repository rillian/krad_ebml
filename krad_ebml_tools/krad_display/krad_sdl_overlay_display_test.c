#include "krad_sdl_overlay_display.h"


#include "SDL.h"

int main (int argc, char *argv[]) {


	krad_overlay_display_t *krad_overlay_display;
	
	int width, height;
	
	width = 1920;
	height = 1080;
	
	krad_overlay_display = krad_overlay_display_create(width, height, width, height);

	//krad_overlay_display_activate(krad_overlay_display);


	krad_overlay_display_test(krad_overlay_display);


	usleep(5000000);


	krad_overlay_display_destroy(krad_overlay_display);

	return 0;

}
