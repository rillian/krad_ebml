gcc -g -Wall krad_ebml_file_broadcast.c -I../krad_display/ ../krad_display/krad_sdl_opengl_display.c -I../krad_gui/ ../krad_gui/krad_gui.c ../krad_opus/krad_opus.c -I../krad_opus ../krad_dirac/krad_dirac.c -I../krad_dirac ../krad_flac/krad_flac.c -I../krad_flac ../krad_vorbis/krad_vorbis.c -I../krad_vorbis ../krad_vpx/krad_vpx.c -I../krad_vpx ../krad_tone/krad_tone.c -I../krad_tone ../krad_audio/krad_audio.c -I../krad_audio ../krad_pulse/krad_pulse.c -I../krad_pulse  ../krad_jack/krad_jack.c -I../krad_jack ../krad_alsa/krad_alsa.c -I../krad_alsa ../../krad_ebml/krad_ebml.c ../../../opus-tools/src/opus_header.c -I../../../schroedinger-1.0.10/ -I../../schroedinger-1.0.10/schroedinger /usr/local/lib/libschroedinger-1.0.a ~/kode/schroedinger-1.0.10/testsuite/.libs/libschrotest.a ~/kode/orc-0.4.16/orc-test/.libs/liborc-test-0.4.a /usr/local/lib/liborc-0.4.a -I../../../orc-0.4.16/orc-test/ -I../../../orc-0.4.16/ ../../../opus-sup/.libs/libopus.a /usr/lib/libspeexdsp.a -I../../../opus/include -I../../../opus-tools/src ../../krad_ebml/halloc/src/halloc.c -I../../krad_ebml/ -I/usr/local/include/SDL -I../../../kradebml/halloc -I. ../../../libvpx/libvpx_g.a /usr/lib/libswscale.a /usr/lib/libavutil.a -o krad_ebml_file_broadcast `sdl-config --cflags --libs` `pkg-config --cflags --libs gtk+-2.0` -lXext -lX11 -lXmu -lXi -lGL -lGLU -lm -ljack -lpulse -lasound -lSDL
