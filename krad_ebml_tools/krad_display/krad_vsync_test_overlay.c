#include "krad_sdl_overlay_display.h"
#include "krad_gui.h"


#define TEST_COUNT 500


int main (int argc, char *argv[]) {

	krad_overlay_display_t *krad_overlay_display;
	//krad_sdl_opengl_display_t *krad_sdl_opengl_display;
	kradgui_t *kradgui;
	
	int width, height;
	int count;
	int stride;
	int gui_byte_size;
	unsigned char *gui_data;
	
	width = 1280;
	height = 720;

	width = 1920;
	height = 1080;

	count = 0;
	
	cairo_surface_t *cst;
	cairo_t *cr;
	
	stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, width);
	gui_byte_size = stride * height;
	gui_data = calloc (1, gui_byte_size);
	
	cst = cairo_image_surface_create_for_data (gui_data, CAIRO_FORMAT_ARGB32, width, height, stride);

	krad_overlay_display = krad_overlay_display_create(width, height, width, height);
//	krad_sdl_opengl_display = krad_sdl_opengl_display_create(width, height, width, height);

	kradgui = kradgui_create(width, height);

	kradgui->render_tearbar = 1;
	
//	kradgui->render_wheel = 1;
	

//	kradgui_test_screen(kradgui, "Overlay Test");
	
	while (count < TEST_COUNT) {

		cr = cairo_create(cst);
		kradgui->cr = cr;
		kradgui_render(kradgui);
		cairo_destroy(cr);


		krad_overlay_display_render_rgb(krad_overlay_display, gui_data, width, height);
	
		//memcpy(krad_sdl_opengl_display->rgb_frame_data, gui_data, gui_byte_size);
	
		//krad_sdl_opengl_draw_screen( krad_sdl_opengl_display );

		//krad_overlay_display_test(krad_overlay_display);

		count++;

	}

	krad_overlay_display_destroy(krad_overlay_display);
//	krad_sdl_opengl_display_destroy(krad_sdl_opengl_display);
	kradgui_destroy(kradgui);
	
	cairo_surface_destroy(cst);
	
	return 0;

}
