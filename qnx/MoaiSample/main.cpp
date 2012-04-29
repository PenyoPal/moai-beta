/*
 * main.cpp
 *
 *  Created on: Apr 24, 2012
 *      Author: james
 */
#include <ctype.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/keycodes.h>
#include <screen/screen.h>
#include <assert.h>
#include <bps/sensor.h>
#include <bps/navigator.h>
#include <bps/screen.h>
#include <bps/bps.h>
#include <bps/event.h>
#include <bps/orientation.h>
#include <math.h>
#include <time.h>
#include <screen/screen.h>
#include <EGL/egl.h>
#include <GLES2/gl2.h>

#include <moaicore/moaicore.h>
#include <aku/AKU.h>
//#include <aku/AKU-untz.h>
#include <aku/AKU-luaext.h>
#include <lua-headers/moai_lua.h>

//#include "bbutil.h"
#define bbutil_terminate() fprintf(stderr, "Terminate at %d \n", __LINE__)
#define bbutil_egl_perror(ERR) fprintf(stderr, "EGL error at %d: " ERR, __LINE__)

namespace MoaiInputDeviceID {
	enum {
		DEVICE,
		TOTAL,
	};
}

namespace MoaiInputDeviceSensorID {
	enum {
		COMPASS,
		LEVEL,
		LOCATION,
		TOUCH,
		TOTAL,
	};
}

bool stop_running;
screen_context_t screen_ctx;
screen_buffer_t screen_buf;
float width, height, max_size;
static AKUContextID mAku;

EGLDisplay egl_disp;
EGLSurface egl_surf;

 EGLConfig egl_conf;
 EGLContext egl_ctx;

 screen_window_t screen_win;
 screen_display_t screen_disp;
 int nbuffers = 2;
 int initialized = 0;


GLint mWidth, mHeight;

const char *luaFolder = "./app/native/resources/lua";

int init_egl_crap() {
	//Simple egl initialization
    int usage;
    int format = SCREEN_FORMAT_RGBX8888;
    EGLint interval = 1;
    int rc, num_configs;

    EGLint attrib_list[]= { EGL_RED_SIZE,        8,
                            EGL_GREEN_SIZE,      8,
                            EGL_BLUE_SIZE,       8,
                            EGL_SURFACE_TYPE,    EGL_WINDOW_BIT,
                            EGL_RENDERABLE_TYPE, EGL_OPENGL_ES2_BIT,
                            EGL_NONE};
    EGLint attributes[] = { EGL_CONTEXT_CLIENT_VERSION, 2, EGL_NONE };

	egl_disp = eglGetDisplay(EGL_DEFAULT_DISPLAY);
	if (egl_disp == EGL_NO_DISPLAY) {
		bbutil_egl_perror("eglGetDisplay");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	rc = eglInitialize(egl_disp, NULL, NULL);
	if (rc != EGL_TRUE) {
		bbutil_egl_perror("eglInitialize");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	rc = eglBindAPI(EGL_OPENGL_ES_API);

	if (rc != EGL_TRUE) {
		bbutil_egl_perror("eglBindApi");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	if(!eglChooseConfig(egl_disp, attrib_list, &egl_conf, 1, &num_configs)) {
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	egl_ctx = eglCreateContext(egl_disp, egl_conf, EGL_NO_CONTEXT, attributes);
	if (egl_ctx == EGL_NO_CONTEXT) {
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	usage = SCREEN_USAGE_OPENGL_ES2;

	rc = screen_create_window(&screen_win, screen_ctx);
	if (rc) {
		perror("screen_create_window");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_FORMAT, &format);
	if (rc) {
		perror("screen_set_window_property_iv(SCREEN_PROPERTY_FORMAT)");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_USAGE, &usage);
	if (rc) {
		perror("screen_set_window_property_iv(SCREEN_PROPERTY_USAGE)");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	rc = screen_get_window_property_pv(screen_win, SCREEN_PROPERTY_DISPLAY, (void **)&screen_disp);
	if (rc) {
		perror("screen_get_window_property_pv");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	int screen_resolution[2];

	rc = screen_get_display_property_iv(screen_disp, SCREEN_PROPERTY_SIZE, screen_resolution);
	if (rc) {
		perror("screen_get_display_property_iv");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	int angle = atoi(getenv("ORIENTATION"));

	screen_display_mode_t screen_mode;
	rc = screen_get_display_property_pv(screen_disp, SCREEN_PROPERTY_MODE, (void**)&screen_mode);
	if (rc) {
		perror("screen_get_display_property_pv");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	int size[2];
	rc = screen_get_window_property_iv(screen_win, SCREEN_PROPERTY_BUFFER_SIZE, size);
	if (rc) {
		perror("screen_get_window_property_iv");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	int buffer_size[2] = {size[0], size[1]};

	if ((angle == 0) || (angle == 180)) {
		if (((screen_mode.width > screen_mode.height) && (size[0] < size[1])) ||
				((screen_mode.width < screen_mode.height) && (size[0] > size[1]))) {
			buffer_size[1] = size[0];
			buffer_size[0] = size[1];
		}
	} else if ((angle == 90) || (angle == 270)){
		if (((screen_mode.width > screen_mode.height) && (size[0] > size[1])) ||
				((screen_mode.width < screen_mode.height && size[0] < size[1]))) {
			buffer_size[1] = size[0];
			buffer_size[0] = size[1];
		}
	} else {
		fprintf(stderr, "Navigator returned an unexpected orientation angle.\n");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_BUFFER_SIZE, buffer_size);
	if (rc) {
		perror("screen_set_window_property_iv");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	rc = screen_set_window_property_iv(screen_win, SCREEN_PROPERTY_ROTATION, &angle);
	if (rc) {
		perror("screen_set_window_property_iv");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	rc = screen_create_window_buffers(screen_win, nbuffers);
	if (rc) {
		perror("screen_create_window_buffers");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	egl_surf = eglCreateWindowSurface(egl_disp, egl_conf, screen_win, NULL);
	if (egl_surf == EGL_NO_SURFACE) {
		bbutil_egl_perror("eglCreateWindowSurface");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	rc = eglMakeCurrent(egl_disp, egl_surf, egl_surf, egl_ctx);
	if (rc != EGL_TRUE) {
		bbutil_egl_perror("eglMakeCurrent");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	rc = eglSwapInterval(egl_disp, interval);
	if (rc != EGL_TRUE) {
		bbutil_egl_perror("eglSwapInterval");
		bbutil_terminate();
		return EXIT_FAILURE;
	}

	initialized = 1;

	return EXIT_SUCCESS;
}


int init_moai() {

	if (init_egl_crap() != EXIT_SUCCESS) {
		fprintf(stderr, "EGL init failed\n");
		return EXIT_FAILURE;
	}

    EGLint surface_width, surface_height;

    eglQuerySurface(egl_disp, egl_surf, EGL_WIDTH, &surface_width);
    EGLint err = eglGetError();
    if (err != 0x3000) {
        return EXIT_FAILURE;
    }

    eglQuerySurface(egl_disp, egl_surf, EGL_HEIGHT, &surface_height);
    err = eglGetError();
    if (err != 0x3000) {
        return EXIT_FAILURE;
    }

    mWidth = surface_width; mHeight = surface_height;
    width = (float) surface_width;
    height = (float) surface_height;

	mAku = AKUCreateContext ();

	AKUExtLoadLuasql ();
	AKUExtLoadLuacurl ();
	AKUExtLoadLuacrypto ();
	AKUExtLoadLuasocket ();

	//AKUUntzInit ();

	AKUSetInputConfigurationName ( "QNX" );

	AKUReserveInputDevices			( MoaiInputDeviceID::TOTAL );
	AKUSetInputDevice				( MoaiInputDeviceID::DEVICE, "device" );
	AKUReserveInputDeviceSensors	( MoaiInputDeviceID::DEVICE, MoaiInputDeviceSensorID::TOTAL );
	AKUSetInputDeviceTouch			( MoaiInputDeviceID::DEVICE, MoaiInputDeviceSensorID::TOUCH,		"touch" );

	AKUSetScreenSize ( width, height );
	AKUDetectGfxContext ();

	// init aku
	AKURunBytecode ( moai_lua, moai_lua_SIZE );

	AKUSetWorkingDirectory(luaFolder);

	return EXIT_SUCCESS;
}

//static double simTime = 0.0f;

void updateAku() {
	AKUSetContext(mAku);
	/*
	double newSimTime = MOAISim::Get().GetSimTime();
	if (newSimTime > simTime)
		fprintf(stderr, "Elapsed sim time: %f\n", newSimTime);
	simTime = newSimTime;
	*/
	AKUUpdate();
}

void beginDrawing() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

void endDrawing() {
	eglSwapBuffers(egl_disp, egl_surf);
}

void drawView() {
	beginDrawing();

	AKUSetContext(mAku);
	AKUSetViewSize(width, height);
	AKURender();

	endDrawing();
}

void handle_touches(int *pair, bool down) {
	fprintf(stderr, "Saw touch event\n");

	AKUEnqueueTouchEvent(MoaiInputDeviceID::DEVICE, MoaiInputDeviceSensorID::TOUCH, (int)(&down),
			down, pair[0], pair[1], 1);
}

void run(string filename) {
	AKUSetContext(mAku);
	AKURunScript(filename.c_str());
}

void handleScreenEvent(bps_event_t *event) {
    int screen_val, buttons;
    int pair[2];

    static bool mouse_pressed = false;

    screen_event_t screen_event = screen_event_get_event(event);

    //Query type of screen event and its location on the screen
    screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_TYPE,
            &screen_val);
    screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_SOURCE_POSITION,
            pair);

    //There is a difference between touch screen events and mouse events
    switch (screen_val) {
    case SCREEN_EVENT_MTOUCH_TOUCH:
    case SCREEN_EVENT_MTOUCH_MOVE:
    	handle_touches(pair, true);
    	break;
    case SCREEN_EVENT_MTOUCH_RELEASE:
    	handle_touches(pair, false);
    	break;
    case SCREEN_EVENT_POINTER:
    	//This is a mouse move event, it is applicable to a device with a usb mouse or simulator
    	screen_get_event_property_iv(screen_event, SCREEN_PROPERTY_BUTTONS,
    			&buttons);
    	if (buttons == SCREEN_LEFT_MOUSE_BUTTON) {
    		//Left mouse button is pressed
    		mouse_pressed = true;
    		handle_touches(pair, true);
    	} else if (mouse_pressed) {
    		//Left mouse button was released
    		mouse_pressed = false;
    		handle_touches(pair, false);
    	}
    	break;
    }
}

void handleNavigatorEvent(bps_event_t *event) {
    switch (bps_event_get_code(event)) {
    case NAVIGATOR_SWIPE_DOWN:
    	break;
    case NAVIGATOR_EXIT:
        stop_running = true;
        break;
    }
}

void handle_events() {
    int screen_domain = screen_get_domain();
    int navigator_domain = navigator_get_domain();

    int rc;

    //Request and process available BPS events
    for(;;) {
        bps_event_t *event = NULL;
        rc = bps_get_event(&event, 0);
        assert(rc == BPS_SUCCESS);

        if (event) {
            int domain = bps_event_get_domain(event);

            if (domain == screen_domain) {
                handleScreenEvent(event);
            } else if (domain == navigator_domain) {
                handleNavigatorEvent(event);
            }
        } else {
            //No more events in the queue
            break;
        }
    }
}



int main(int argc, char **argv) {
    stop_running = false;

    //Create a screen context that will be used to create an EGL surface to to receive libscreen events
    screen_create_context(&screen_ctx, 0);

    //Initialize BPS library
    bps_initialize();

    //Initialize application logic
    if (EXIT_SUCCESS != init_moai()) {
        fprintf(stderr, "initialize failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_ctx);
        return 0;
    }

    //Signal BPS library that navigator and screen events will be requested
    if (BPS_SUCCESS != screen_request_events(screen_ctx)) {
        fprintf(stderr, "screen_request_events failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_ctx);
        return 0;
    }

    if (BPS_SUCCESS != navigator_request_events(0)) {
        fprintf(stderr, "navigator_request_events failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_ctx);
        return 0;
    }

    //Signal BPS library that navigator orientation is not to be locked
    if (BPS_SUCCESS != navigator_rotation_lock(false)) {
        fprintf(stderr, "navigator_rotation_lock failed\n");
        bbutil_terminate();
        screen_destroy_context(screen_ctx);
        return 0;
    }

    run("main.lua");
    AKUPause(false);
    while (!stop_running) {
    	handle_events();
    	updateAku();
    	drawView();
    }

    AKUFinalize();

    //Stop requesting events from libscreen
    screen_stop_events(screen_ctx);

    //Shut down BPS library for this process
    bps_shutdown();

    //Destroy libscreen context
    screen_destroy_context(screen_ctx);
    return 0;
}
