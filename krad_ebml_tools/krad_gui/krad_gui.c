#include "krad_gui.h"

kradgui_t *kradgui_create(int width, int height) {

	kradgui_t *kradgui;

	if ((kradgui = calloc (1, sizeof (kradgui_t))) == NULL) {
		fprintf(stderr, "kradgui mem alloc fail\n");
		exit (1);
	}
	
	kradgui->clear = 1;
	kradgui->width = width;
	kradgui->height = height;
	kradgui_set_playback_state(kradgui, KRADGUI_PLAYING);
	kradgui_reset_elapsed_time(kradgui);
	kradgui_set_total_track_time_ms(kradgui, 45 * 60 * 1000);
	
	clock_gettime(CLOCK_MONOTONIC, &kradgui->start_time);
	
	return kradgui;

}

kradgui_t *kradgui_create_with_internal_surface(int width, int height) {

	kradgui_t *kradgui;

	kradgui = kradgui_create(width, height);
		
	kradgui_create_internal_surface(kradgui);
	
	//incase calloc takes time
	kradgui_reset_elapsed_time(kradgui);
	clock_gettime(CLOCK_MONOTONIC, &kradgui->start_time);

	return kradgui;

}

void kradgui_create_internal_surface(kradgui_t *kradgui) {

	kradgui->stride = cairo_format_stride_for_width (CAIRO_FORMAT_ARGB32, kradgui->width);
	kradgui->bytes = kradgui->stride * kradgui->height;
	kradgui->data = calloc (1, kradgui->bytes);
	kradgui->cst = cairo_image_surface_create_for_data (kradgui->data, CAIRO_FORMAT_ARGB32, kradgui->width, kradgui->height, kradgui->stride);
	kradgui->cr = cairo_create(kradgui->cst);
	kradgui->internal_surface = 1;
	
}

void kradgui_destroy_internal_surface(kradgui_t *kradgui) {

	if (kradgui->internal_surface == 1) {
		if (kradgui->cr != NULL) {
			cairo_destroy (kradgui->cr);
		}
		cairo_surface_destroy (kradgui->cst);
		free(kradgui->data);
		kradgui->cst = NULL;
		kradgui->data = NULL;
		kradgui->stride = 0;
		kradgui->bytes = 0;
		kradgui->internal_surface = 0;
	}
}

void kradgui_set_size(kradgui_t *kradgui, int width, int height) {

	kradgui->width = width;
	kradgui->height = height;

}


void kradgui_set_background_color(kradgui_t *kradgui, float r, float g, float b, float a) {


	//BGCOLOR_TRANS


}

void kradgui_destroy(kradgui_t *kradgui) {

	if (kradgui->reel_to_reel != NULL) {
		kradgui_reel_to_reel_destroy(kradgui->reel_to_reel);
	}
	
	if (kradgui->playback_state_status != NULL) {
		kradgui_playback_state_status_destroy(kradgui->playback_state_status);
	}
	
	if (kradgui->bug != NULL) {
		cairo_surface_destroy ( kradgui->bug );
	}
	
	if (kradgui->internal_surface == 1) {
		kradgui_destroy_internal_surface(kradgui);
	}

	free(kradgui);

}

void kradgui_add_item(kradgui_t *kradgui, kradgui_item_t item) {

	switch (item) {
	
	case REEL_TO_REEL:
		kradgui->reel_to_reel = kradgui_reel_to_reel_create(kradgui);
		break;

	case PLAYBACK_STATE_STATUS:
		kradgui->playback_state_status = kradgui_playback_state_status_create(kradgui);
		break;
	}

}

void kradgui_remove_item(kradgui_t *kradgui, kradgui_item_t item) {

	switch (item) {
	
	case REEL_TO_REEL:
		kradgui_reel_to_reel_destroy(kradgui->reel_to_reel);
		kradgui->reel_to_reel = NULL;
		break;
	
	case PLAYBACK_STATE_STATUS:
		kradgui_playback_state_status_destroy(kradgui->playback_state_status);
		kradgui->playback_state_status = NULL;
		break;
	}

}


kradgui_reel_to_reel_t *kradgui_reel_to_reel_create(kradgui_t *kradgui) {

	kradgui_reel_to_reel_t *kradgui_reel_to_reel;

	if ((kradgui_reel_to_reel = calloc (1, sizeof (kradgui_reel_to_reel_t))) == NULL) {
		fprintf(stderr, "kradgui kradgui reel to reel mem alloc fail\n");
		exit (1);
	}
	
	kradgui_reel_to_reel->kradgui = kradgui;
	
	kradgui_reel_to_reel->reel_size = NORMAL_REEL_SIZE;
	kradgui_reel_to_reel->reel_speed = SLOW_NORMAL_REEL_SPEED;
	//kradgui->reel_size = PRO_REEL_SIZE;
	//kradgui->reel_speed = PRO_REEL_SPEED;
	
	kradgui_update_reel_to_reel_information(kradgui_reel_to_reel);
	
	return kradgui_reel_to_reel;

}


void kradgui_reel_to_reel_destroy(kradgui_reel_to_reel_t *kradgui_reel_to_reel) {

	free(kradgui_reel_to_reel);

}

kradgui_playback_state_status_t *kradgui_playback_state_status_create(kradgui_t *kradgui) {

	kradgui_playback_state_status_t *kradgui_playback_state_status;

	if ((kradgui_playback_state_status = calloc (1, sizeof (kradgui_playback_state_status_t))) == NULL) {
		fprintf(stderr, "kradgui kradgui reel to reel mem alloc fail\n");
		exit (1);
	}
	
	kradgui_playback_state_status->kradgui = kradgui;

	return kradgui_playback_state_status;

}

void kradgui_playback_state_status_destroy(kradgui_playback_state_status_t *kradgui_playback_state_status) {

	free(kradgui_playback_state_status);	

}


void kradgui_render_playback_state_status(kradgui_playback_state_status_t *kradgui_playback_state_status) {

	cairo_t *cr;
	cr = kradgui_playback_state_status->kradgui->cr;
	
	cairo_select_font_face (cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
	cairo_set_font_size (cr, 20.0);
	
	switch (kradgui_playback_state_status->kradgui->playback_state) {
		case KRADGUI_PLAYING:
			cairo_set_source_rgb (cr, GREEN);
			break;
			
		case KRADGUI_PAUSED:
			cairo_set_source_rgb (cr, BLUE);
			break;
			
		case KRADGUI_STOPPED:
			cairo_set_source_rgb (cr, GREY);
			break;
	}
	
	cairo_move_to (cr, kradgui_playback_state_status->kradgui->width/8.0 * 6, kradgui_playback_state_status->kradgui->height - 232.0);
	
	cairo_show_text (cr, kradgui_playback_state_status->kradgui->playback_state_status_string);

}

void kradgui_set_playback_state(kradgui_t *kradgui, kradgui_playback_state_t playback_state) {

	kradgui->playback_state = playback_state;
	kradgui_update_playback_state_status_string(kradgui);

}


void kradgui_control_speed_up(kradgui_t *kradgui) {

	if (kradgui->control_speed_up_callback != NULL) {
		kradgui->control_speed_up_callback(kradgui->callback_pointer);
	}


}

void kradgui_control_speed_down(kradgui_t *kradgui) {

	if (kradgui->control_speed_down_callback != NULL) {
		kradgui->control_speed_down_callback(kradgui->callback_pointer);
	}

}

void kradgui_control_speed(kradgui_t *kradgui, float value) {

	if (kradgui->control_speed_callback != NULL) {
		kradgui->control_speed_callback(kradgui->callback_pointer, value);
	}

}


void kradgui_set_control_speed_callback(kradgui_t *kradgui, void control_speed_callback(void *, float)) {

	kradgui->control_speed_callback = control_speed_callback;


}

void kradgui_set_control_speed_down_callback(kradgui_t *kradgui, void control_speed_down_callback(void *)) {

	kradgui->control_speed_down_callback = control_speed_down_callback;


}

void kradgui_set_control_speed_up_callback(kradgui_t *kradgui, void control_speed_up_callback(void *)) {

	kradgui->control_speed_up_callback = control_speed_up_callback;


}

void kradgui_set_callback_pointer(kradgui_t *kradgui, void *callback_pointer) {

	kradgui->callback_pointer = callback_pointer;


}


void kradgui_update_reel_to_reel_information(kradgui_reel_to_reel_t *kradgui_reel_to_reel) {

	// calc rotation example
	// determine rotation in degrees per millisecond
	// pro is 0.0381cm per millisecond 38.1 / 1000
	// 1 degree in pro is 0.074083333cm 26.67 / 360 
	// time ms * 0.381 = distance
	// distance / 0.74083333 = rotation
	
	kradgui_reel_to_reel->distance = kradgui_reel_to_reel->kradgui->current_track_time_ms * (kradgui_reel_to_reel->reel_speed / 1000);
	kradgui_reel_to_reel->total_distance = kradgui_reel_to_reel->kradgui->total_track_time_ms * (kradgui_reel_to_reel->reel_speed / 1000);
	if (kradgui_reel_to_reel->distance >= kradgui_reel_to_reel->total_distance) {
		kradgui_reel_to_reel->distance = kradgui_reel_to_reel->total_distance;
	}
	
	kradgui_reel_to_reel->angle = (unsigned int)(kradgui_reel_to_reel->distance / (kradgui_reel_to_reel->reel_size / 360)) % 360;
	
	//printf("MS: %zu Reel Size: %fcm Reel Speed: %fcm/s Distance: %fcm Rotation: %fdegrees\n", kradgui_reel_to_reel->kradgui->current_track_time_ms, kradgui_reel_to_reel->reel_size, kradgui_reel_to_reel->reel_speed, kradgui_reel_to_reel->distance, kradgui_reel_to_reel->angle);
	
}

void kradgui_render_circles (kradgui_t *kradgui, int w, int h) {

	cairo_t *cr;
	
	cr = kradgui->cr;


	cairo_set_source_rgb(cr, GREEN);
	cairo_arc (cr, w, h, 0.4 * h, 0, 2*M_PI);
	cairo_stroke (cr);


	cairo_set_source_rgb(cr, ORANGE);
	cairo_arc (cr, w, h, 0.8 * h, 0, 2*M_PI);
	cairo_stroke (cr);
	
}

void kradgui_set_bug (kradgui_t *kradgui, char *filename, int x, int y) {


	if ( filename != NULL ) {
	
		kradgui->bug_x = x;
		kradgui->bug_y = y;
		//kradgui_load_bug ( kradgui, filename );
		kradgui->render_bug = 1;
		kradgui->next_bug = filename;
	}

}

void kradgui_remove_bug (kradgui_t *kradgui) {

	//kradgui_load_bug ( kradgui, NULL );
	kradgui->next_bug = "none";
}

void kradgui_load_bug (kradgui_t *kradgui, char *filename) {

	if (kradgui->bug_alpha == 0.0f) {
		kradgui->bug_alpha = 1.0f;
	}

	
	if (kradgui->bug != NULL) {
		cairo_surface_destroy ( kradgui->bug );
		kradgui->bug = NULL;
	}
	
	if ( filename != NULL ) {
		kradgui->bug = cairo_image_surface_create_from_png ( filename );
		kradgui->bug_width = cairo_image_surface_get_width ( kradgui->bug );
		kradgui->bug_height = cairo_image_surface_get_height ( kradgui->bug );
		//kradgui->start_frame = kradgui->frame;
		kradgui->bug_fade = 0;
		kradgui->bug_fade_speed = 0.04;
		kradgui->bug_fader = -1.3f;
		kradgui->render_bug = 1;
	} else {
		kradgui->render_bug = 0;
		kradgui->bug_x = 0;
		kradgui->bug_y = 0;
		kradgui->bug_width = 0;
		kradgui->bug_height = 0;
		kradgui->bug_fade = 0;
		kradgui->bug = NULL;
	}

}


void kradgui_render_bug (kradgui_t *kradgui) {

	if (kradgui->next_bug != NULL) {
		
		if (kradgui->bug_fade <= 0.0f) {
			//printf("%f %f\n", kradgui->bug_fade, kradgui->bug_fader);
			kradgui->bug_fade = 0.0f;
			if (strncmp(kradgui->next_bug, "none", 4) == 0) {
				kradgui_load_bug (kradgui, NULL);
			} else {
				kradgui_load_bug (kradgui, kradgui->next_bug);
			}
			kradgui->next_bug = NULL;
		} else {
		
			//kradgui->bug_fade -= 0.01f;
	
			kradgui->bug_fade = (0.5f) + 1.0f * sin(kradgui->bug_fader / 2);
			kradgui->bug_fader -= kradgui->bug_fade_speed;
		
		}
	} else {
	
		if (kradgui->bug_fade < kradgui->bug_alpha) {
			//kradgui->bug_fade += 0.01f;
			kradgui->bug_fade = (0.5f) + 1.0f * sin(kradgui->bug_fader / 2);
			kradgui->bug_fader += kradgui->bug_fade_speed;
		} else {
			if (kradgui->bug_fade > kradgui->bug_alpha + 0.1f) {
				//kradgui->bug_fade += 0.01f;
				kradgui->bug_fade = (0.5f) + 1.0f * sin(kradgui->bug_fader / 2);
				kradgui->bug_fader -= kradgui->bug_fade_speed;
			} 
		}
	
	}
	
	//printf("%f %f\n", kradgui->bug_fade, kradgui->bug_fader);
	
	if (kradgui->bug_fader >= 2 * M_PI) {
		kradgui->bug_fader -= 2 * M_PI;
	}

	if (kradgui->render_bug == 1) {
		cairo_set_source_surface ( kradgui->cr, kradgui->bug, kradgui->bug_x, kradgui->bug_y );
		cairo_paint_with_alpha ( kradgui->cr , kradgui->bug_fade);
	}
}



void kradgui_render_meter (kradgui_t *kradgui, int x, int y, int size, float pos) {

	cairo_t *cr;
	
	cr = kradgui->cr;


	pos = pos * 1.8f - 90.0f;
//	printf("peak %f %f\n", pos);

	cairo_save(cr);
	
	cairo_new_path ( cr );
	
	cairo_translate (cr, x, y);
	cairo_set_line_width(cr, 0.05 * size);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_source_rgb(cr, GREY3);
	cairo_arc (cr, 0, 0, 0.8 * size, M_PI, 0);
	cairo_stroke (cr);
	
	cairo_set_source_rgb(cr, ORANGE);
	cairo_arc (cr, 0, 0, 0.65 * size, 1.8 * M_PI, 0);
	cairo_stroke (cr);

	cairo_arc (cr, size - 0.56 * size, -0.15 * size, 0.07 * size, 0, 2 * M_PI);
	cairo_fill(cr);

	cairo_save(cr);
	cairo_translate (cr, 0.05 * size, 0);
	cairo_rotate (cr, pos * (M_PI/180.0));	
	/*
	cairo_pattern_t *pat;
	pat = cairo_pattern_create_linear (0, 0, 0.11 * size, 0);
	cairo_pattern_add_color_stop_rgba (pat, 0, 0, 0, 0, 0);
	cairo_pattern_add_color_stop_rgba (pat, 0.3, 0, 0, 0, 0);	
	cairo_pattern_add_color_stop_rgba (pat, 0.4, 0, 0, 0, 1);	
	cairo_pattern_add_color_stop_rgba (pat, 1, 0, 0, 0, 0);
	cairo_set_source (cr, pat);
	cairo_rectangle (cr, 0, 0, 0.11 * size, -size);
	cairo_fill (cr);
	*/
	cairo_set_source_rgb(cr, WHITE);
	cairo_move_to (cr, 0, 0);
	cairo_line_to (cr, 0, -size * 0.93);
	cairo_stroke (cr);
	
	cairo_restore(cr);
	cairo_set_source_rgb(cr, WHITE);
	cairo_arc (cr, 0.05 * size, 0, 0.1 * size, 0, 2 * M_PI);
	cairo_fill(cr);
	
	cairo_restore(cr);
	
}

void kradgui_render_curve (kradgui_t *kradgui, int w, int h) {

	cairo_t *cr;

	cr = kradgui->cr;

	cairo_set_line_width(cr, 3.5);
	cairo_set_source_rgb(cr, BLUE);

	static float pointy_positioner = 0.0f;
	int pointy;
	int pointy_start = 15;
	int pointy_range = 83;
	float pointy_final;
	float pointy_final_adj;
	float speed = 0.021;
	
	pointy = pointy_start + (pointy_range / 2) + round(pointy_range * sin(pointy_positioner) / 2);

	pointy_positioner += speed;

	if (pointy_positioner >= 2 * M_PI) {
		pointy_positioner -= 2 * M_PI;
		//new_speed = 1;
	}
	
	pointy_final = pointy * 0.01f;
	
	pointy_final_adj = pointy_final / 3.0f;
	
	int point1_x, point1_y, point2_x, point2_y;
	
	point1_x =  (0.250 + pointy_final_adj) * w;
	point1_y =  0.25 * h;
	point2_x =  (0.75 - pointy_final_adj) * w;
	point2_y =  0.25 * h;
	
	point1_y =  pointy_final * h;
	point2_y =  pointy_final * h;
		
	///printf("f1 %f f2 %f\n", pointy_final, pointy_final_adj);
	cairo_move_to (cr, 20, 70);
	cairo_curve_to (cr, point1_x, point1_y, point2_x, point2_y, 0.85 * w, 0.85 * h);
	cairo_stroke (cr);
	
}

void kradgui_render_grid (kradgui_t *kradgui, int x, int y, int w, int h, int lines) {


	cairo_t *cr;
	cr = kradgui->cr;
	int count;

	//srand(time(NULL));
	cairo_save(cr);
	cairo_translate (cr, x + w/2, y + h/2);
	
	cairo_translate (cr, -x + -w/2, -y + -h/2);
	cairo_set_line_width(cr, 1.5);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);
	cairo_set_source_rgb(cr, GREEN);
	
	cairo_save(cr);

	for (count = 1; count != lines; count++) {
	
		cairo_move_to (cr, x + (w/lines) * count, y);
		cairo_line_to (cr, x + (w/lines) * count, y + h);
		cairo_stroke (cr);

	}

	cairo_restore(cr);
	
	for (count = 1; count != lines; count++) {
		
		cairo_move_to (cr, x, y + (h/lines) * count);
		cairo_line_to (cr, x + w, y + (h/lines) * count);
		cairo_stroke (cr);
		
	}
	
	
	cairo_rectangle (cr, x, y, w, h);
	cairo_stroke (cr);
	
	cairo_restore(cr);
}

void kradgui_render_viper (kradgui_t *kradgui, int x, int y, int size, int direction) {

	cairo_t *cr;

	cr = kradgui->cr;

	cairo_save(cr);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_BUTT);

	cairo_set_line_width(cr, size/7);
	cairo_set_source_rgb(cr, WHITE);
	cairo_translate (cr, x, y);

	cairo_arc (cr, 0.0, 0.0, size/2, 0, 2*M_PI);
	cairo_stroke(cr);
	
	cairo_set_source_rgb(cr, GREEN);

	cairo_arc (cr, 0.0, -size/5, size/14, 0, 2*M_PI);
	cairo_stroke(cr);

	cairo_rotate (cr, 40 * (M_PI/180.0));
	cairo_arc (cr, size/3.8, 0.0, size/14, 0, 2*M_PI);
	cairo_stroke(cr);
	cairo_move_to(cr, size/1.8,  0.0);
	cairo_rel_line_to (cr, size/2.3, 0.0);
	cairo_stroke(cr);

	cairo_rotate (cr, 100 * (M_PI/180.0));
	cairo_arc (cr, size/3.8, 0.0, size/14, 0, 2*M_PI);
	cairo_stroke(cr);
	cairo_move_to(cr, size/1.8,  0.0);
	cairo_rel_line_to (cr, size/2.3, 0.0);
	cairo_stroke(cr);

	cairo_rotate (cr, 130 * (M_PI/180.0));
	cairo_move_to(cr, size/1.8,  0.0);
	cairo_rel_line_to (cr, size/2.3, 0.0);
	cairo_stroke(cr);

	cairo_set_source_rgb(cr, WHITE);
	cairo_rotate (cr, (180 + direction - 90) * (M_PI/180.0));
	cairo_move_to(cr, size/1.6,  size/6);
	cairo_rel_line_to (cr, size/6, -size/6);
	cairo_rel_line_to (cr, -size/6, -size/6);
	//cairo_rel_line_to(cr, size/1.6,  size/6);
	cairo_fill(cr);

	cairo_set_line_width(cr, size/7);
	cairo_rotate (cr, 180 * (M_PI/180.0));
	cairo_move_to(cr, size/1.6, 0.0);
	cairo_rel_line_to (cr, size/8, 0.0);
	cairo_stroke(cr);
	
	cairo_restore(cr);

}

void kradgui_render_hex (kradgui_t *kradgui, int x, int y, int w) {


	cairo_t *cr;
	cairo_pattern_t *pat;
	
	static float hexrot = 0;
	
	cr = kradgui->cr;	
	
	cairo_save(cr);
		
	cairo_set_line_width(cr, 1);
	cairo_set_source_rgb(cr, BLUE);

	int r1;
	float scale;

	scale = 2.5;
		
	r1 = ((w)/2 * sqrt(3));

	cairo_translate (cr, x, y);
	cairo_rotate (cr, hexrot * (M_PI/180.0));
	cairo_translate (cr, -(w/2), -r1);
	
	// draw radius 
	//cairo_move_to (cr, w/2, 0);
	//cairo_line_to (cr, w/2, r1);
	//cairo_stroke (cr);

	cairo_move_to (cr, 0, 0);
	cairo_rel_line_to (cr, w, 0);
	cairo_rotate (cr, 60 * (M_PI/180.0));
	cairo_rel_line_to (cr, w, 0);
	cairo_rotate (cr, 60 * (M_PI/180.0));
	cairo_rel_line_to (cr, w, 0);
	cairo_rotate (cr, 60 * (M_PI/180.0));
	cairo_rel_line_to (cr, w, 0);
	cairo_rotate (cr, 60 * (M_PI/180.0));
	cairo_rel_line_to (cr, w, 0);
	cairo_rotate (cr, 60 * (M_PI/180.0));
	cairo_rel_line_to (cr, w, 0);
	//cairo_stroke_preserve (cr);


	hexrot += 0.4;
	cairo_fill (cr);
	
	cairo_restore(cr);


//-----------------------

	cairo_save(cr);
		
	cairo_set_line_width(cr, 1.5);
	cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
	cairo_set_source_rgb(cr, GREY);


	cairo_translate (cr, x, y);
	cairo_rotate (cr, hexrot * (M_PI/180.0));
	cairo_translate (cr, -((w * scale)/2), -r1 * scale);
	cairo_scale(cr, scale, scale);
	hexrot += 0.11;

	cairo_move_to (cr, 0, 0);
	cairo_rel_line_to (cr, w, 0);
	cairo_rotate (cr, 60 * (M_PI/180.0));
	cairo_rel_line_to (cr, w, 0);
	cairo_rotate (cr, 60 * (M_PI/180.0));
	cairo_rel_line_to (cr, w, 0);
	cairo_rotate (cr, 60 * (M_PI/180.0));
	cairo_rel_line_to (cr, w, 0);
	cairo_rotate (cr, 60 * (M_PI/180.0));
	cairo_rel_line_to (cr, w, 0);
	cairo_rotate (cr, 60 * (M_PI/180.0));
	cairo_rel_line_to (cr, w, 0);

	//cairo_stroke_preserve (cr);
	
	cairo_rotate (cr, 60 * (M_PI/180.0));
	
	
	// draw radius 
	//cairo_move_to (cr, w/2, 0);
	//cairo_line_to (cr, w/2, r1);
	//cairo_stroke_preserve (cr);
	
	cairo_set_operator(cr, CAIRO_OPERATOR_ADD);
	pat = cairo_pattern_create_radial (w/2, r1, 3, w/2, r1, r1*scale);
	cairo_pattern_add_color_stop_rgba (pat, 0, 0, 0, 1, 1);
	//cairo_pattern_add_color_stop_rgba (pat, 0.6, 0, 0, 1, 0.3);
	//cairo_pattern_add_color_stop_rgba (pat, 0.8, 0, 0, 1, 0.05);
	cairo_pattern_add_color_stop_rgba (pat, 0.4, 0, 0, 0, 0);
	cairo_set_source (cr, pat);
	
	cairo_fill (cr);
	cairo_pattern_destroy (pat);


	
	cairo_restore(cr);

}


void kradgui_render_cube (kradgui_t *kradgui, int x, int y, int w, int h) {

	cairo_t *cr;
	
	static float shear = -0.579595f;
	static float scale = 0.86062;
	static float rot = 30.0f;
	
	cr = kradgui->cr;
	
	int lines;
	
	lines = 5;
	
	cairo_matrix_t matrix_rot;
	cairo_matrix_t matrix_rot2;
	cairo_matrix_t matrix_rot3;

	cairo_matrix_t matrix_trans;
	cairo_matrix_t matrix_trans2;
	cairo_matrix_t matrix_trans3;
	
	cairo_matrix_t matrix_sher;
	cairo_matrix_t matrix_sher2;
	cairo_matrix_t matrix_sher3;

	cairo_matrix_t matrix_scal;
	cairo_matrix_t matrix_scal2;
	cairo_matrix_t matrix_scal3;
	cairo_matrix_t matrix_scal4;
	
	cairo_matrix_t matrix_scal_sher;
	cairo_matrix_t matrix_scal_sher_rot;
	cairo_matrix_t matrix_scal_sher_rot_trans;

	
	cairo_matrix_t matrix_scal_sher2;
	cairo_matrix_t matrix_scal_sher_rot2;
	cairo_matrix_t matrix_scal_sher_rot_trans2;
	
	cairo_matrix_t matrix_scal_sher3;
	cairo_matrix_t matrix_scal_sher_rot3;
	cairo_matrix_t matrix_scal_sher_rot_trans3;
	
	cairo_matrix_init_translate(&matrix_trans, x - 1, y + 0.5);
	cairo_matrix_init_translate(&matrix_trans2, x, y - h);
	cairo_matrix_init_translate(&matrix_trans3, x + 1, y + 0.5);
	
	cairo_matrix_init_scale(&matrix_scal, 1.0, scale);
	
	cairo_matrix_init_scale(&matrix_scal2, 1.0, 1.0);
	cairo_matrix_init_scale(&matrix_scal3, 1.0, 1.0);
	cairo_matrix_init_scale(&matrix_scal4, 1.0, 1.0);
		
	cairo_matrix_init_rotate(&matrix_rot, (90.0f) * (M_PI/180.0));
	cairo_matrix_init_rotate(&matrix_rot2, (rot) * (M_PI/180.0));
	cairo_matrix_init_rotate(&matrix_rot3, (-30.0f) * (M_PI/180.0));
	
	cairo_matrix_init(&matrix_sher, 1.0, 0.0, shear, 1.0, 0.0, 0.0);
	cairo_matrix_init(&matrix_sher2, 1.0, 0.0, shear, 1.0, 0.0, 0.0);
	cairo_matrix_init(&matrix_sher3, 1.0, 0.0, shear, 1.0, 0.0, 0.0);

	
	cairo_matrix_multiply (&matrix_scal_sher, &matrix_scal, &matrix_sher);
	cairo_matrix_multiply (&matrix_scal_sher2, &matrix_scal, &matrix_sher2);
	cairo_matrix_multiply (&matrix_scal_sher3, &matrix_scal, &matrix_sher3);
	
	
	cairo_matrix_multiply (&matrix_scal_sher_rot, &matrix_scal_sher, &matrix_rot);
	cairo_matrix_multiply (&matrix_scal_sher_rot2, &matrix_scal_sher2, &matrix_rot2);
	cairo_matrix_multiply (&matrix_scal_sher_rot3, &matrix_scal_sher3, &matrix_rot3);	

	cairo_matrix_multiply (&matrix_scal_sher_rot_trans, &matrix_scal_sher_rot, &matrix_trans);
	
	cairo_matrix_multiply (&matrix_scal_sher_rot_trans2, &matrix_scal_sher_rot2, &matrix_trans2);
	cairo_matrix_multiply (&matrix_scal_sher_rot_trans3, &matrix_scal_sher_rot3, &matrix_trans3);
	
	cairo_save(cr);
	cairo_set_line_width(cr, 2);
	cairo_set_source_rgb(cr, GREEN);
	
	cairo_save(cr);
	
	cairo_transform(cr, &matrix_scal_sher_rot_trans2);
	//cairo_transform(cr, &matrix_scal4);
	cairo_set_source_rgba(cr, LGREEN);
	//kradgui_render_grid (kradgui, w, h, w, h, lines);
	cairo_stroke (cr);
	
	cairo_set_source_rgb(cr, GREEN);
	cairo_rectangle (cr, 0, 0, w, h);
	//cairo_fill (cr);
	cairo_stroke (cr);
	
	cairo_restore(cr);
	cairo_save(cr);

	//cairo_set_source_rgb(cr, ORANGE);
	cairo_transform(cr, &matrix_scal_sher_rot_trans);
	//cairo_transform(cr, &matrix_scal2);
	//cairo_transform(cr, &matrix_scal4);
	
	cairo_set_source_rgba(cr, LGREEN);
	cairo_move_to(cr, 0, 0);
	cairo_line_to (cr, w, h);
	cairo_stroke (cr);
	
	cairo_set_source_rgb(cr, GREEN);
	kradgui_render_grid (kradgui, 0, 0, w, h, lines);

	cairo_rectangle (cr, 0, 0, w, h);
	//cairo_fill (cr);
	cairo_stroke (cr);

	cairo_restore(cr);
	cairo_save(cr);

	//cairo_set_source_rgb(cr, BLUE);
	cairo_transform(cr, &matrix_scal_sher_rot_trans3);
	//cairo_transform(cr, &matrix_scal3);
	//cairo_transform(cr, &matrix_scal4);
	
	cairo_set_source_rgba(cr, LGREEN);
	cairo_move_to(cr, 0, 0);
	cairo_line_to (cr, w, h);
	cairo_stroke (cr);
	
	
	cairo_set_source_rgb(cr, GREEN);
	//kradgui_render_grid (kradgui, 0, 0, w, h, lines);
	cairo_rectangle (cr, 0, 0, w, h);
	//cairo_fill (cr);	
	cairo_stroke (cr);

	cairo_restore(cr);
	cairo_restore(cr);


	// other method
	if (0) {

		cairo_save(cr);

		cairo_transform(cr, &matrix_scal2);	
		cairo_transform(cr, &matrix_scal_sher_rot_trans2);
	
		static float h_test = 170;

		int t_x, t_y, t_w, t_h;
		int t2_x, t2_y, t2_w, t2_h;
	
		t_x = 500;
		t_y = 200;
	
		t_x = 0;
		t_y = 0;
	
	
		t_w = 200;
		t_h = 200;
	
		t2_w = t_w;
		t2_h = t_h;
	
		t2_x = (t_x + t2_w/2) - h_test;
		t2_y = (t_y + t2_h/2) - h_test;
	
	
		cairo_set_line_width(cr, 3);
		cairo_set_source_rgb(cr, ORANGE);
	
	
		cairo_move_to (cr, t_x, t_y);
		cairo_line_to (cr, t2_x, t2_y);	
	
		cairo_move_to (cr, t_x + t_w, t_y);
		cairo_line_to (cr, t2_x + t2_w, t2_y);	
	
		cairo_move_to (cr, t_x, t_y + t_h);
		cairo_line_to (cr, t2_x, t2_y + t2_h);	
	
		cairo_move_to (cr, t_x + t_w, t_y + t_h);
		cairo_line_to (cr, t2_x + t2_w, t2_y + t2_h);	
	
		cairo_stroke (cr);
	
		cairo_rectangle (cr, t_x, t_y, t_w, t_h);


		cairo_rectangle (cr, t2_x, t2_y, t2_w, t2_h);
	
		cairo_stroke (cr);

		cairo_restore(cr);

	}

}

void kradgui_render_vtest (kradgui_t *kradgui) {

	
	//#ifdef SDLGUI
	//SDL_ShowCursor(0);

	//#endif
	//printf("x %d y %d\n", mx, my);

/*
	kradgui_render_cube (kradgui, 590, 230, 100, 100);
	kradgui_render_cube (kradgui, 375, 260, 100, 100);
	kradgui_render_cube (kradgui,  550,  430, 20, 20);
	
	kradgui_render_curve(kradgui, mx, my);
	
	kradgui_render_circles (kradgui, mx,  my);
*/
static int g = 0;

g += (rand() % 5);
g -= (rand() % 5);

if (g > 100) {
	g = 100;
}

if (g < 0) {
	g = 0;
}

g = (kradgui->cursor_x * 100) / kradgui->width;


	kradgui_render_hex (kradgui, kradgui->cursor_x, kradgui->cursor_y, 14);


static int vposx = 0;
static int vposy = 0;
static int vdir = 0;
if (kradgui->cursor_x > vposx) {
vposx++;
} else {
vposx--;
}

if (kradgui->cursor_y > vposy) {
vposy++;
} else {
vposy--;
}
vdir = atan2 (kradgui->cursor_y - vposy, kradgui->cursor_x - vposx) * 180 / M_PI;

static float vposx2 = 666;
static float vposy2 = 666;
static int vdir2 = 0;
if (vposx > vposx2) {
vposx2 += 0.2;
} else {
vposx2 -= 0.2;
}

if (vposy > vposy2) {
vposy2 += 0.3;
} else {
vposy2 -= 0.7;
}
vdir2 = atan2 (vposy - vposy2, vposx - vposx2) * 180 / M_PI;


	//printf("\nvdr is --%d--\n", vdir);
	kradgui_render_viper (kradgui, vposx, vposy, 32, vdir);	
	kradgui_render_viper (kradgui, vposx2, vposy2, 32, vdir2);

	
}

void kradgui_render_ftest (kradgui_t *kradgui) {

	//int w, h;
	cairo_t *cr;
	
	cr = kradgui->cr;

	//w = kradgui->width;
	//h = kradgui->height;

	cairo_set_line_width(cr, 22);
	cairo_set_line_cap (cr, CAIRO_LINE_CAP_ROUND);
	
	//#ifdef SDLGUI
	//SDL_ShowCursor(0);

	//#endif
	//printf("m x %d y %d\n", kradgui->cursor_x, kradgui->cursor_y);

/*
	kradgui_render_cube (kradgui, 590, 230, 100, 100);
	kradgui_render_cube (kradgui, 375, 260, 100, 100);
	kradgui_render_cube (kradgui,  550,  430, 20, 20);
	
	kradgui_render_curve(kradgui, mx, my);
	
	kradgui_render_circles (kradgui, mx,  my);
*/
static int g = 0;

g += (rand() % 5);
g -= (rand() % 5);

if (g > 100) {
	g = 100;
}

if (g < 0) {
	g = 0;
}

g = (kradgui->cursor_x * 100) / kradgui->width;


	kradgui_render_hex (kradgui, kradgui->cursor_x, kradgui->cursor_y, 33);


static int vposx = 0;
static int vposy = 0;
static int vdir = 0;
if (kradgui->cursor_x > vposx) {
vposx++;
} else {
vposx--;
}

if (kradgui->cursor_y > vposy) {
vposy++;
} else {
vposy--;
}
vdir = atan2 (kradgui->cursor_y - vposy, kradgui->cursor_x - vposx) * 180 / M_PI;

static float vposx2 = 666;
static float vposy2 = 666;
static int vdir2 = 0;
if (vposx > vposx2) {
vposx2 += 0.2;
} else {
vposx2 -= 0.2;
}

if (vposy > vposy2) {
vposy2 += 0.3;
} else {
vposy2 -= 0.7;
}
vdir2 = atan2 (vposy - vposy2, vposx - vposx2) * 180 / M_PI;


	//printf("\nvdr is --%d--\n", vdir);
	kradgui_render_viper (kradgui, vposx, vposy, 32, vdir);
	
	kradgui_render_viper (kradgui, vposx2, vposy2, 32, vdir2);
//	kradgui_render_viper (kradgui, 474, 333, 13, 99);

//	kradgui_render_viper (kradgui, 544, 555, 53, 222);
	
/*
	kradgui_render_hex (kradgui, 444, 222, 33);
	kradgui_render_hex (kradgui, 474, 333, 13);
	kradgui_render_hex (kradgui, mx, my, 44);
	kradgui_render_hex (kradgui, 544, 555, 53);
	kradgui_render_hex (kradgui, 644, 290, 63);
	kradgui_render_hex (kradgui, 744, 410, 23);
*/	
//	kradgui_render_meter (kradgui, 63, 33, 33, g);

	//kradgui_render_meter (kradgui, 233, 233, 44, g);
	
	//kradgui_render_meter (kradgui, 233, 533, 88, g);
	
//	kradgui_render_meter (kradgui, 633, 533, 188, g);


}

void kradgui_render_wheel (kradgui_t *kradgui) {

	int diameter;
	int inner_diameter;

	int s;
	int num_spokes;
	int spoke_distance;
	
	num_spokes = 10;
	spoke_distance = 360 / num_spokes;
	

	diameter = (kradgui->height/4 * 3.5) / 2;
	inner_diameter = diameter/8;

	cairo_t *cr;
	cr = kradgui->cr;

	cairo_save(cr);
	cairo_new_path (cr);
	// wheel
	cairo_set_line_width(cr, 5);
	cairo_set_source_rgb(cr, WHITE);
	cairo_translate (cr, kradgui->width/3, kradgui->height/2);
	cairo_arc (cr, 0.0, 0.0, inner_diameter, 0, 2*M_PI);
	cairo_stroke(cr);
	cairo_arc(cr, 0.0, 0.0, diameter, 0, 2 * M_PI);
	cairo_stroke(cr);

	// spokes
	cairo_set_source_rgb(cr, WHITE);
	cairo_set_line_width(cr, 35);
	
	cairo_rotate (cr, (kradgui->wheel_angle % 360) * (M_PI/180.0));
	
	for (s = 0; s < num_spokes; s++) {
		cairo_rotate (cr, spoke_distance * (M_PI/180.0));
		cairo_move_to(cr, inner_diameter,  0.0);
		cairo_line_to (cr, diameter, 0.0);
		cairo_move_to(cr, 0.0, 0.0);
		cairo_stroke(cr);
	}

	kradgui->wheel_angle += 1;

	cairo_restore(cr);

}


void kradgui_render_reel(kradgui_reel_to_reel_t *kradgui_reel_to_reel, int x, int y) {


	cairo_t *cr;
	int rotation_offset;
	float reel_inner_diameter;
	float reel_diameter;
	float tape_on_dest_reel;
	float tape_on_source_reel;
	float max_tape_on_reel;
	//float min_tape_on_reel;
	float middle_of_tape_on_dest_reel;
	float middle_of_tape_on_source_reel;
	float tape_exit_point;
	float tape_entrance_point;
	
	cr = kradgui_reel_to_reel->kradgui->cr;
	
	reel_inner_diameter = 13;
	reel_diameter = 45;
	
	max_tape_on_reel = reel_diameter - 5 - (reel_inner_diameter + 2);
	//min_tape_on_reel = 2 + reel_inner_diameter;
	
	tape_on_dest_reel = (kradgui_reel_to_reel->kradgui->current_track_progress * 0.01) * max_tape_on_reel;
	tape_on_source_reel = ((100.0 - kradgui_reel_to_reel->kradgui->current_track_progress) * 0.01) * max_tape_on_reel;
	middle_of_tape_on_source_reel = reel_inner_diameter + (tape_on_source_reel / 2) + 2;
	middle_of_tape_on_dest_reel = reel_inner_diameter + (tape_on_dest_reel / 2) + 2;
	tape_exit_point = reel_inner_diameter + tape_on_source_reel;
	tape_entrance_point = reel_inner_diameter + tape_on_dest_reel;

	// this is so the reels don't appear identical
	rotation_offset = y;

	// reel
	cairo_set_line_width(cr, 1.5);
	cairo_set_source_rgb(cr, WHITE);
	cairo_save(cr);
	cairo_translate (cr, x, y);
	cairo_save(cr);
	cairo_arc (cr, 0.0, 0.0, 3.0, 0, 2*M_PI);
	cairo_stroke(cr);
	cairo_arc (cr, 0.0, 0.0, 9.0, 0, 2*M_PI);
	cairo_stroke(cr);
	cairo_arc (cr, 0.0, 0.0, reel_inner_diameter, 0, 2*M_PI);
	cairo_stroke(cr);
	cairo_arc(cr, 0.0, 0.0, reel_diameter, 0, 2 * M_PI);
	cairo_stroke(cr);



	//tape on source reel
	if (x > 200) {
		
		cairo_set_source_rgb(cr, GREY3);
		cairo_set_line_width(cr, tape_on_source_reel);
		cairo_arc (cr, 0.0, 0.0, middle_of_tape_on_source_reel, 0, 2*M_PI);
		cairo_stroke(cr);
		cairo_set_line_width(cr, 2);
	}

	//tape on dest reel
	if (x < 200) {
		cairo_set_source_rgb(cr, GREY3);
		cairo_set_line_width(cr, tape_on_dest_reel);
		cairo_arc (cr, 0.0, 0.0, middle_of_tape_on_dest_reel, 0, 2*M_PI);
		cairo_stroke(cr);
		cairo_set_line_width(cr, 2);
	}
	
	// spokes
//	cairo_set_source_rgba(cr, WHITE_TRANS);
	cairo_set_source_rgb(cr, WHITE);
	cairo_set_line_width(cr, 2);
	cairo_rotate (cr, (kradgui_reel_to_reel->angle + rotation_offset % 360) * (M_PI/180.0));
	cairo_rotate (cr, 120 * (M_PI/180.0));
	cairo_move_to(cr, 19.0,  0.0);
	cairo_line_to (cr, 37.0, 0.0);
	cairo_move_to(cr, 0.0, 0.0);
	cairo_stroke(cr);
	
	cairo_rotate (cr, 120 * (M_PI/180.0));
	cairo_move_to(cr, 19.0,  0.0);
	cairo_line_to (cr, 37.0, 0.0);
	cairo_move_to(cr, 0.0, 0.0);
	cairo_stroke(cr);

	cairo_rotate (cr, 120 * (M_PI/180.0));
	cairo_move_to(cr, 19.0,  0.0);
	cairo_line_to (cr, 37.0, 0.0);
	cairo_move_to(cr, 0.0, 0.0);
	cairo_stroke(cr);
	
	// tape transfer from reel to reel
	cairo_restore(cr);
	if ((x > 200) && (kradgui_reel_to_reel->kradgui->current_track_progress < 100.0)) {
		cairo_set_source_rgb(cr, GREY3);
		cairo_move_to (cr, tape_exit_point, 7);
		cairo_line_to (cr, 4, 63);
		cairo_stroke(cr);
		
		cairo_move_to (cr, 2, 63);
		cairo_line_to (cr, -24, 57);
		cairo_stroke(cr);
		

		cairo_move_to (cr, -25, 56);
		cairo_line_to (cr, -66, 67);
		cairo_stroke(cr);
	
		cairo_move_to (cr, -81, 67);
		cairo_line_to (cr, -121, 56);
		cairo_stroke(cr);

		cairo_move_to (cr, -122, 56);
		cairo_line_to (cr, -154, 63);
		cairo_stroke(cr);
			
		cairo_move_to (cr, -155, 62);
		cairo_line_to (cr, -153 - tape_entrance_point, 7);
		cairo_stroke(cr);

	}

	cairo_restore(cr);
	
}

void kradgui_render_reel_to_reel(kradgui_reel_to_reel_t *kradgui_reel_to_reel) {

	cairo_t *cr;
	
	cr = kradgui_reel_to_reel->kradgui->cr;

	cairo_save(cr);
	cairo_scale(cr, 3.0, 3.0);

	// reels
	kradgui_render_reel(kradgui_reel_to_reel, 82, 57);
	kradgui_render_reel(kradgui_reel_to_reel, 235, 57);

	// roller wheels
	cairo_set_source_rgb (cr, WHITE);
	cairo_arc (cr, 236.0, 116.0, 4.0, 0, 2*M_PI);
	cairo_stroke(cr);

	cairo_arc (cr, 210.0, 121.0, 7.0, 0, 2*M_PI);
	cairo_stroke(cr);
	
	cairo_arc (cr, 111.0, 120.0, 7.0, 0, 2*M_PI);
	cairo_stroke(cr);
	cairo_arc (cr, 111.0, 120.0, 2.0, 0, 2*M_PI);
	cairo_fill(cr);
	
	
	cairo_arc (cr, 82.0, 116.0, 4.0, 0, 2*M_PI);
	cairo_stroke(cr);		

	// tape head	
	cairo_rectangle (cr, 154, 111, 14, 13);
	cairo_stroke(cr);
	
	cairo_restore(cr);
	
}

void kradgui_render_elapsed_timecode(kradgui_t *kradgui) {

	kradgui_update_elapsed_time(kradgui);

	cairo_select_font_face (kradgui->cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (kradgui->cr, 22.0);
	cairo_set_source_rgb (kradgui->cr, ORANGE);
	
	cairo_move_to (kradgui->cr, kradgui->width/4.0, kradgui->height - 174.0);	
	cairo_show_text (kradgui->cr, "Elapsed Time");
	
	cairo_move_to (kradgui->cr, kradgui->width/4.0, kradgui->height - 131.0);
	cairo_show_text (kradgui->cr, kradgui->elapsed_time_timecode_string);
	
	cairo_stroke (kradgui->cr);
	
}

void kradgui_render_timecode(kradgui_t *kradgui) {

	cairo_select_font_face (kradgui->cr, "monospace", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (kradgui->cr, 22.0);
	cairo_set_source_rgb (kradgui->cr, ORANGE);

	cairo_move_to (kradgui->cr, kradgui->width/4.0, kradgui->height - 84.0);
	cairo_show_text (kradgui->cr, "Track Time");

	
	cairo_move_to (kradgui->cr, kradgui->width/4.0, kradgui->height - 40.0);
	cairo_show_text (kradgui->cr, kradgui->current_track_time_timecode_string);
}

void kradgui_render_rgb(kradgui_t *kradgui) {

	int size;
	int xoffset;
	int yoffset;
	
	size = kradgui->width / 7;
	xoffset = size;
	yoffset = kradgui->height / 7;
	
	cairo_set_source_rgb (kradgui->cr, 1.0, 0.0, 0.0);
	cairo_rectangle (kradgui->cr, xoffset, yoffset, size, size);
	cairo_fill (kradgui->cr);
	
	cairo_set_source_rgb (kradgui->cr, 0.0, 1.0, 0.0);
	cairo_rectangle (kradgui->cr, xoffset + size, yoffset, size, size);
	cairo_fill (kradgui->cr);
	
	cairo_set_source_rgb (kradgui->cr, 0.0, 0.0, 1.0);
	cairo_rectangle (kradgui->cr, xoffset + (size * 2), yoffset, size, size);
	cairo_fill (kradgui->cr);
		
}


void kradgui_render_tearbar(kradgui_t *kradgui) {

	if (kradgui->tearbar_speed == 0.0) {		
		kradgui->tearbar_speed = 0.1;
		kradgui->tearbar_speed_adj = 0.01;
		kradgui->tearbar_width = kradgui->width / 15;
		kradgui->movement_range = kradgui->width - kradgui->tearbar_width;
	}

	kradgui->tearbar_position = 0 + (kradgui->movement_range / 2) + round(kradgui->movement_range * sin(kradgui->tearbar_positioner) / 2);

	//printf("speed is %f adj is %f position is: %d positioner is %f\n", kradgui->tearbar_speed, kradgui->tearbar_speed_adj, kradgui->tearbar_position, kradgui->tearbar_positioner);

	kradgui->tearbar_positioner += kradgui->tearbar_speed;

	if (kradgui->tearbar_positioner >= 2 * M_PI) {
		kradgui->tearbar_positioner -= 2 * M_PI;
		kradgui->new_speed = 1;
	}
	
	if ((kradgui->new_speed) && (kradgui->tearbar_position < kradgui->last_tearbar_position)) {

		if (kradgui->tearbar_speed > 0.17) {
			kradgui->tearbar_speed_adj = -0.01;
		}
	
		if (kradgui->tearbar_speed <= 0.02) {
			kradgui->tearbar_speed_adj = 0.01;
		}
		
		kradgui->tearbar_speed += kradgui->tearbar_speed_adj;
		kradgui->new_speed = 0;
	}
	
	kradgui->last_tearbar_position = kradgui->tearbar_position;
	
	cairo_set_source_rgb (kradgui->cr, 1.0, 1.0, 1.0);
	cairo_rectangle (kradgui->cr, kradgui->tearbar_position, 0, kradgui->tearbar_width, kradgui->height);
	cairo_fill (kradgui->cr);

}


void kradgui_test_screen(kradgui_t *kradgui, char *info) {

	time_t t;

	t = time(0);
	sprintf(kradgui->test_start_time, "Test Started: %s", ctime(&t));
	kradgui->test_start_time[strlen(kradgui->test_start_time) - 1] = '\0';

	if (info != NULL) {
		strncpy(kradgui->test_info_text, info, 510);
	} else {
		strcpy(kradgui->test_info_text, "No Test Information");
	}
	
	
	kradgui_go_live(kradgui);
	kradgui->render_rgb = 1;
	kradgui->render_test_text = 1;
	kradgui->render_rotator = 1;

}

void kradgui_render_test_text(kradgui_t *kradgui) {

	time_t t;

	cairo_select_font_face (kradgui->cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (kradgui->cr, 62.0);
	cairo_set_source_rgb (kradgui->cr, 1.0, 0.1, 0.1);
	cairo_move_to (kradgui->cr, kradgui->width/10, kradgui->height - 360);
	cairo_show_text (kradgui->cr, "KRAD TEST SIGNAL");

	cairo_set_font_size (kradgui->cr, 20.0);
	cairo_set_source_rgb (kradgui->cr, 0.9, 0.9, 0.9);
	cairo_move_to (kradgui->cr, 50, kradgui->height - 280);
	cairo_show_text (kradgui->cr, kradgui->test_info_text);


	cairo_select_font_face (kradgui->cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_source_rgb (kradgui->cr, 0.8, 0.8, 0.8);
	cairo_set_font_size (kradgui->cr, 32.0);

	t = time(0);
	sprintf(kradgui->test_text_time, "%s", ctime(&t));
	kradgui->test_text_time[strlen(kradgui->test_text_time) - 1] = '\0';
	cairo_move_to (kradgui->cr, kradgui->width/10.0, kradgui->height - 84.0);
	cairo_show_text (kradgui->cr, kradgui->test_text_time);

	cairo_set_source_rgb (kradgui->cr, 0.4, 0.4, 0.4);
	cairo_set_font_size (kradgui->cr, 26.0);
	cairo_move_to (kradgui->cr, 44, 50);
	cairo_show_text (kradgui->cr, kradgui->test_start_time);

	cairo_set_source_rgb (kradgui->cr, 0.8, 0.8, 0.8);
	cairo_set_font_size (kradgui->cr, 32.0);
	sprintf(kradgui->test_text, "W: %d H: %d F: %llu", kradgui->width, kradgui->height, kradgui->frame);
	cairo_move_to (kradgui->cr, kradgui->width/2, kradgui->height - 84.0);
	cairo_show_text (kradgui->cr, kradgui->test_text);

}

void kradgui_render_rotator(kradgui_t *kradgui) {

	int width;
	
	
	width = 16;
	kradgui->rotator_speed = 2;
	
	
	cairo_set_source_rgb (kradgui->cr, 0.5, 0.5, 0.5);
	
	cairo_save (kradgui->cr);

	cairo_translate (kradgui->cr, kradgui->width/4 * 3, kradgui->height/5 * 3);
	cairo_rotate (kradgui->cr, (kradgui->rotator_angle % 360) * (M_PI/180.0));
	cairo_translate (kradgui->cr, width/2 * -1, (width * 16)/2 * -1);
	cairo_rectangle (kradgui->cr, 0, 0, width, width * 16);
	cairo_fill (kradgui->cr);
	cairo_restore (kradgui->cr);

	kradgui->rotator_angle += kradgui->rotator_speed;

}

void kradgui_render_recording(kradgui_t *kradgui) {

	kradgui->recording_elapsed_time = timespec_diff(kradgui->recording_start_time, kradgui->current_time);
	sprintf(kradgui->recording_time_timecode_string, "%03ld:%02ld:%02ld:%02ld:%03ld", kradgui->recording_elapsed_time.tv_sec / 86400 % 365, kradgui->recording_elapsed_time.tv_sec / 3600 % 24, kradgui->recording_elapsed_time.tv_sec / 60 % 60, kradgui->recording_elapsed_time.tv_sec % 60, kradgui->recording_elapsed_time.tv_nsec / 1000000);

	cairo_set_source_rgba (kradgui->cr, 0.7, 0.0, 0.0, 0.3);
	cairo_rectangle (kradgui->cr, kradgui->recording_box_margin, kradgui->recording_box_margin, kradgui->recording_box_width, kradgui->recording_box_height);
	cairo_fill (kradgui->cr);

	cairo_select_font_face (kradgui->cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (kradgui->cr, kradgui->recording_box_font_size);
	cairo_set_source_rgb (kradgui->cr, 1.0, 0.0, 0.0);

	cairo_move_to (kradgui->cr, kradgui->recording_box_margin + kradgui->recording_box_padding,  kradgui->recording_box_margin + (kradgui->recording_box_height - kradgui->recording_box_padding));
	cairo_show_text (kradgui->cr, "RECORDING");


//	cairo_move_to (kradgui->cr, 50, 22);
//	cairo_show_text (kradgui->cr, kradgui->recording_time_timecode_string);

}

void kradgui_render_live(kradgui_t *kradgui) {

	kradgui->live_elapsed_time = timespec_diff(kradgui->live_start_time, kradgui->current_time);
	sprintf(kradgui->live_time_timecode_string, "%03ld:%02ld:%02ld:%02ld:%03ld", kradgui->live_elapsed_time.tv_sec / 86400 % 365, kradgui->live_elapsed_time.tv_sec / 3600 % 24, kradgui->live_elapsed_time.tv_sec / 60 % 60, kradgui->live_elapsed_time.tv_sec % 60, kradgui->live_elapsed_time.tv_nsec / 1000000);

	cairo_set_source_rgba (kradgui->cr, 0.0, 0.5, 0.0, 0.1);
	cairo_rectangle (kradgui->cr, kradgui->width - (kradgui->live_box_margin + kradgui->live_box_width), kradgui->live_box_margin, kradgui->live_box_width, kradgui->live_box_height);
	cairo_fill (kradgui->cr);

	cairo_select_font_face (kradgui->cr, "sans-serif", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_BOLD);
	cairo_set_font_size (kradgui->cr, kradgui->live_box_font_size);
	cairo_set_source_rgb (kradgui->cr, GREEN);

	cairo_move_to (kradgui->cr, kradgui->width - (kradgui->live_box_width + kradgui->live_box_margin) + kradgui->live_box_padding,  kradgui->live_box_margin + (kradgui->live_box_height - kradgui->live_box_padding));
	cairo_show_text (kradgui->cr, "LIVE");

	cairo_set_font_size (kradgui->cr, kradgui->live_box_font_size / 4);
	cairo_move_to (kradgui->cr, (kradgui->width - (kradgui->live_box_width + kradgui->live_box_margin)) + kradgui->live_box_padding,  kradgui->live_box_height + (kradgui->live_box_padding + kradgui->live_box_margin * 2));
	cairo_show_text (kradgui->cr, kradgui->live_time_timecode_string);

}

void kradgui_go_live(kradgui_t *kradgui) {

	clock_gettime(CLOCK_MONOTONIC, &kradgui->live_start_time);
	
	kradgui->live_box_width = 270;
	kradgui->live_box_height = 114;
	kradgui->live_box_margin = 44;
	kradgui->live_box_padding = (kradgui->live_box_height / 5);
	kradgui->live_box_font_size = (kradgui->live_box_height / 8) * 6.5;
	kradgui->live = 1;

}

void kradgui_go_off(kradgui_t *kradgui) {

	kradgui->live = 0;

}

void kradgui_start_recording(kradgui_t *kradgui) {

	clock_gettime(CLOCK_MONOTONIC, &kradgui->recording_start_time);
	
	kradgui->recording_box_width = 654;
	kradgui->recording_box_height = 114;
	kradgui->recording_box_margin = 44;
	kradgui->recording_box_padding = (kradgui->recording_box_height / 5);
	kradgui->recording_box_font_size = (kradgui->recording_box_height / 8) * 6.5;
	
	
	kradgui->recording = 1;

}

void kradgui_stop_recording(kradgui_t *kradgui) {

	kradgui->recording = 0;
	
}



void kradgui_update_current_track_progress(kradgui_t *kradgui) {
	
	if ((kradgui->total_track_time_ms != 0) && (kradgui->current_track_time_ms != 0)) {
		kradgui->current_track_progress = (kradgui->current_track_time_ms * 1000 / kradgui->total_track_time_ms) / 10.0;
	} else {
		kradgui->current_track_progress = 0;
	}
	
	if (kradgui->current_track_progress > 100.0) {
		kradgui->current_track_progress = 100.0;
	}
	
	if (kradgui->current_track_progress < 0.0) {
		kradgui->current_track_progress = 0.0;
	}
	
	//printf("Progress is: %f%%\n", kradgui->current_track_progress);
}

void kradgui_update_current_track_time_ms(kradgui_t *kradgui) {
	
	kradgui->current_track_time_ms = ((kradgui->current_track_time.tv_sec * 1000) + (kradgui->current_track_time.tv_nsec / 1000000));

}

void kradgui_update_total_track_time_ms(kradgui_t *kradgui) {
	
	kradgui->total_track_time_ms = ((kradgui->total_track_time.tv_sec * 1000) + (kradgui->total_track_time.tv_nsec / 1000000));

}

void kradgui_update_elapsed_time_ms(kradgui_t *kradgui) {
	
	kradgui->elapsed_time_ms = ((kradgui->elapsed_time.tv_sec * 1000) + (kradgui->elapsed_time.tv_nsec / 1000000));

}


void kradgui_update_current_track_time_timecode_string(kradgui_t *kradgui) {

	sprintf(kradgui->current_track_time_timecode_string, "%03ld:%02ld:%02ld:%02ld:%03ld", kradgui->current_track_time.tv_sec / 86400 % 365, kradgui->current_track_time.tv_sec / 3600 % 24, kradgui->current_track_time.tv_sec / 60 % 60, kradgui->current_track_time.tv_sec % 60, kradgui->current_track_time.tv_nsec / 1000000);

}

void kradgui_update_total_track_time_timecode_string(kradgui_t *kradgui) {

	sprintf(kradgui->total_track_time_timecode_string, "%03ld:%02ld:%02ld:%02ld:%03ld", kradgui->total_track_time.tv_sec / 86400 % 365, kradgui->total_track_time.tv_sec / 3600 % 24, kradgui->total_track_time.tv_sec / 60 % 60, kradgui->total_track_time.tv_sec % 60, kradgui->total_track_time.tv_nsec / 1000000);

}

void kradgui_update_playback_state_status_string(kradgui_t *kradgui) {

	switch (kradgui->playback_state) {
		case KRADGUI_PLAYING:
			strcpy(kradgui->playback_state_status_string, "Playing");
			break;
			
		case KRADGUI_PAUSED:
			strcpy(kradgui->playback_state_status_string, "Paused");
			break;
			
		case KRADGUI_STOPPED:
			strcpy(kradgui->playback_state_status_string, "Stopped");
			break;
	}

}

void kradgui_update_elapsed_time_timecode_string(kradgui_t *kradgui) {

	sprintf(kradgui->elapsed_time_timecode_string, "%03ld:%02ld:%02ld:%02ld:%03ld", kradgui->elapsed_time.tv_sec / 86400 % 365, kradgui->elapsed_time.tv_sec / 3600 % 24, kradgui->elapsed_time.tv_sec / 60 % 60, kradgui->elapsed_time.tv_sec % 60, kradgui->elapsed_time.tv_nsec / 1000000);

}


void kradgui_start_draw_time(kradgui_t *kradgui) {

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &kradgui->start_draw_time);
	sprintf(kradgui->elapsed_time_timecode_string, "%03ld:%02ld:%02ld:%02ld:%03ld", kradgui->elapsed_time.tv_sec / 86400 % 365, kradgui->elapsed_time.tv_sec / 3600 % 24, kradgui->elapsed_time.tv_sec / 60 % 60, kradgui->elapsed_time.tv_sec % 60, kradgui->elapsed_time.tv_nsec / 1000000);

}

void kradgui_end_draw_time(kradgui_t *kradgui) {

	clock_gettime(CLOCK_THREAD_CPUTIME_ID, &kradgui->end_draw_time);
	kradgui->draw_time = timespec_diff(kradgui->start_draw_time, kradgui->end_draw_time);
	sprintf(kradgui->draw_time_string, "%02ld:%03ld", kradgui->draw_time.tv_sec % 60, kradgui->draw_time.tv_nsec / 1000000);

}

void kradgui_reset_elapsed_time(kradgui_t *kradgui) {

	clock_gettime(CLOCK_MONOTONIC, &kradgui->start_time);
	memcpy(&kradgui->current_time, &kradgui->start_time, sizeof(struct timespec));
	kradgui->elapsed_time = timespec_diff(kradgui->start_time, kradgui->current_time);
	kradgui_update_elapsed_time_ms(kradgui);
	kradgui_update_elapsed_time_timecode_string(kradgui);

}

void kradgui_update_elapsed_time(kradgui_t *kradgui) {

	clock_gettime(CLOCK_MONOTONIC, &kradgui->current_time);
	kradgui->elapsed_time = timespec_diff(kradgui->start_time, kradgui->current_time);
	kradgui_update_elapsed_time_ms(kradgui);
	kradgui_update_elapsed_time_timecode_string(kradgui);

}

void kradgui_set_current_track_time(kradgui_t *kradgui, struct timespec current_track_time) {

	kradgui->current_track_time = current_track_time;
	kradgui_update_current_track_time_ms(kradgui);
	kradgui_update_current_track_progress(kradgui);
	kradgui_update_current_track_time_timecode_string(kradgui);

}

void kradgui_set_total_track_time(kradgui_t *kradgui, struct timespec total_track_time) {

	kradgui->total_track_time = total_track_time;
	kradgui_update_current_track_progress(kradgui);
	kradgui_update_total_track_time_timecode_string(kradgui);
}

void kradgui_set_current_track_time_ms(kradgui_t *kradgui, unsigned int current_track_time_ms) {

	//printf("Set current track time to: %zums\n", current_track_time_ms);

	kradgui->current_track_time.tv_sec = (current_track_time_ms - (current_track_time_ms % 1000)) / 1000;
	kradgui->current_track_time.tv_nsec = (current_track_time_ms % 1000) * 1000000;
	kradgui_update_current_track_time_ms(kradgui);
	kradgui_update_current_track_progress(kradgui);
	kradgui_update_current_track_time_timecode_string(kradgui);

}

void kradgui_add_current_track_time_ms(kradgui_t *kradgui, unsigned int additional_ms) {

	unsigned int current_track_time_ms;
	
	current_track_time_ms = ((kradgui->current_track_time.tv_sec * 1000) + (kradgui->current_track_time.tv_nsec / 1000000));

	current_track_time_ms += additional_ms;

	//printf("Set current track time to: %zums\n", current_track_time_ms);

	kradgui->current_track_time.tv_sec = (current_track_time_ms - (current_track_time_ms % 1000)) / 1000;
	kradgui->current_track_time.tv_nsec = (current_track_time_ms % 1000) * 1000000;
	kradgui_update_current_track_time_ms(kradgui);
	kradgui_update_current_track_progress(kradgui);
	kradgui_update_current_track_time_timecode_string(kradgui);

}

void kradgui_set_total_track_time_ms(kradgui_t *kradgui, unsigned int total_track_time_ms) {

	kradgui->total_track_time.tv_sec = (total_track_time_ms - (total_track_time_ms % 1000)) / 1000;
	kradgui->total_track_time.tv_nsec = (total_track_time_ms % 1000) * 1000000;
	kradgui_update_total_track_time_ms(kradgui);
	kradgui_update_current_track_progress(kradgui);
	kradgui_update_total_track_time_timecode_string(kradgui);

}


void kradgui_render(kradgui_t *kradgui) {

	if (kradgui->update_drawtime) {
		kradgui_start_draw_time(kradgui);
	}

	if (kradgui->clear == 1) {
		if (kradgui->overlay) {
			cairo_save (kradgui->cr);
			cairo_set_source_rgba (kradgui->cr, BGCOLOR_TRANS);
			cairo_set_operator (kradgui->cr, CAIRO_OPERATOR_SOURCE);
			cairo_paint (kradgui->cr);
			cairo_restore (kradgui->cr);
		} else {
			cairo_set_source_rgb (kradgui->cr, BGCOLOR);
			cairo_paint (kradgui->cr);
		}
	}

	if (kradgui->reel_to_reel != NULL) {
		kradgui_update_reel_to_reel_information(kradgui->reel_to_reel);
		kradgui_render_reel_to_reel(kradgui->reel_to_reel);
	}
	
	if (kradgui->playback_state_status != NULL) {
		kradgui_render_playback_state_status(kradgui->playback_state_status);
	}
	
	if (kradgui->render_timecode) {
		kradgui_render_timecode(kradgui);
		kradgui_render_elapsed_timecode(kradgui);
	}
	
	if ((kradgui->render_test_text) || (kradgui->live)) {
		kradgui_update_elapsed_time(kradgui);
	}
	
	if (kradgui->live) {
		kradgui_render_live(kradgui);
	}
	
	if (kradgui->recording) {
		kradgui_render_recording(kradgui);
	}
	
	if (kradgui->render_rgb) {
		kradgui_render_rgb(kradgui);
	}
	
	if (kradgui->render_rotator) {
		kradgui_render_rotator(kradgui);
	}
	
	if (kradgui->render_test_text) {
		kradgui_render_test_text(kradgui);
	}
	
	if (kradgui->render_tearbar) {
		kradgui_render_tearbar(kradgui);
	}
	
	if (kradgui->render_wheel) {
		kradgui_render_wheel(kradgui);
	}
	
	if (kradgui->render_ftest) {
		kradgui_render_ftest(kradgui);
	}
	
	if ( kradgui->render_bug ) {
		kradgui_render_bug ( kradgui );
	}
	
	if (kradgui->update_drawtime) {
		kradgui_end_draw_time(kradgui);
	}
	
	if (kradgui->print_drawtime) {
		printf("Frame: %llu :: %s\r", kradgui->frame, kradgui->draw_time_string);
		fflush(stdout);
	}
	
	kradgui->frame++;

}


struct timespec timespec_diff(struct timespec start, struct timespec end)
{
	struct timespec temp;
	if ((end.tv_nsec-start.tv_nsec)<0) {
		temp.tv_sec = end.tv_sec-start.tv_sec-1;
		temp.tv_nsec = 1000000000+end.tv_nsec-start.tv_nsec;
	} else {
		temp.tv_sec = end.tv_sec-start.tv_sec;
		temp.tv_nsec = end.tv_nsec-start.tv_nsec;
	}
	return temp;
}

