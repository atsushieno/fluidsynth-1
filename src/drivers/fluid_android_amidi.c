/* FluidSynth - A Software Synthesizer
 *
 * Copyright (C) 2003  Peter Hanappe and others.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 2.1 of
 * the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free
 * Software Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA
 */


/* fluid_android_amidi.c
 *
 * Driver for Android AMIDI
 *
 */

#include "fluidsynth_priv.h"

#if ANDROID_AMIDI_SUPPORT

#include <dlfcn.h>
#include <sys/system_properties.h>
#include "fluid_midi.h"
#include "fluid_mdriver.h"
#include "fluid_settings.h"

#define AMIDI_RECEIVER_MAX_BYTES 0x10000;

/*
 * Those libamidi functions are dlopen() based, for the same reason as
 * https://github.com/google/oboe/blob/c278091a/src/aaudio/AAudioLoader.h#L67
 */
typedef media_status_t (*fluid_amidi_output_open_func) (const AMidiDevice*, int32_t, AMidiOutputPort**);
typedef void (*fluid_amidi_output_close_func) (const AMidiOutputPort*);
typedef ssize_t (*fluid_amidi_output_receive_func) (const AMidiOutputPort*, int32_t*, uint8_t*, size_t, size_t*, int64_t*);
typedef media_status_t (*fluid_amidi_device_release_func) (const AMidiDevice*);

fluid_amidi_output_open_func amidi_open = NULL;
fluid_amidi_output_receive_func amidi_receive = NULL;
fluid_amidi_output_close_func amidi_close = NULL;
fluid_amidi_device_release_func amidi_release = NULL;


typedef struct
{
	void *libamidi;
    fluid_midi_driver_t driver;
    AMidiOutputPort* port; /* You know, this is the *input* receiver. Android API naming is selfish and subjective. */
    void *fluid_thread;
    int reading;
    fluid_midi_parser_t *parser;
    uint8_t *receiver_buffer;
    size_t max_bytes;
} fluid_android_amidi_driver_t;

static fluid_thread_return_t
fluid_android_amidi_run(void* d);

static void *libamidi;
static int libamidi_refcount;

/*
 * fluid_android_amidi_driver_settings
 */
void fluid_android_amidi_driver_settings(fluid_settings_t *settings)
{
    fluid_settings_register_num(settings, "midi.android-amidi.device-handle", 0, -0x80000000, 0x7FFFFFFF, 0);
    fluid_settings_register_num(settings, "midi.android-amidi.port", 0, DBL_MIN, DBL_MAX, 0);
}

/*
 * new_fluid_android_amidi_driver
 */
fluid_midi_driver_t *
new_fluid_android_amidi_driver(fluid_settings_t *settings,
                         handle_midi_event_func_t handler, void *data)
{
    fluid_android_amidi_driver_t *dev;
    AMidiDevice* device;
    AMidiOutputPort* outputPort;
    int portNumber;
    double deviceHandle; /* it is allocated in bigger unit than int so that long pointer can remain valid */

	if (!libamidi) {
		libamidi = dlopen("libamidi.so", RTLD_NOW);
		if (!libamidi) {
			FLUID_LOG(FLUID_ERR, "Android Native MIDI platform shared library libamidi.so not found.");
			return NULL;
		}
		amidi_open = (fluid_amidi_output_open_func) dlsym (libamidi, "AMidiOutputPort_open");
		amidi_close = (fluid_amidi_output_close_func) dlsym (libamidi, "AMidiOutputPort_close");
		amidi_receive = (fluid_amidi_output_receive_func) dlsym (libamidi, "AMidiOutputPort_receive");
		amidi_release = (fluid_amidi_device_release_func) dlsym (libamidi, "AMidiDevice_release");
		if (!amidi_open || !amidi_close || !amidi_receive || !amidi_release) {
			FLUID_LOG(FLUID_ERR, "Failed to load Android Native MIDI platform shared library libamidi.so. Expected symbols not found");
			dlclose(libamidi);
			return NULL;
		}
	}
	libamidi_refcount++;

    dev = FLUID_MALLOC(sizeof(fluid_android_amidi_driver_t));

    if(dev == NULL)
    {
        FLUID_LOG(FLUID_ERR, "Could not allocate memory for AMIDI driver");
        return NULL;
    }

    FLUID_MEMSET(dev, 0, sizeof(fluid_android_amidi_driver_t));
    dev->driver.handler = handler;
    dev->driver.data = data;
    dev->max_bytes = AMIDI_RECEIVER_MAX_BYTES;
    
    dev->parser = FLUID_NEW(fluid_midi_parser_t);
    if (!dev->parser)
    {
        FLUID_LOG(FLUID_ERR, "Could not allocate MIDI parser for AMIDI driver");
        return NULL;
    }
    
    dev->receiver_buffer = FLUID_MALLOC(dev->max_bytes);
    if (!dev->receiver_buffer)
    {
        FLUID_LOG(FLUID_ERR, "Could not allocate buffer for AMIDI driver");
        return NULL;
    }

    fluid_settings_getint (settings, "midi.android-amidi.port", &portNumber);
    fluid_settings_getnum (settings, "midi.android-amidi.device-handle", &deviceHandle);

    device = (AMidiDevice*) (void*) (long) deviceHandle;
    
    if (amidi_open(device, portNumber, &outputPort) != AMEDIA_OK)
    {
        FLUID_LOG(FLUID_ERR, "Could not open port %d", portNumber);
        goto error_recovery;
    }
    dev->port = outputPort;
    
    /* start receiver thread*/
    dev->fluid_thread = new_fluid_thread ("android-amidi", fluid_android_amidi_run, dev, 0, FALSE);
    if (!dev->fluid_thread)
    {
        FLUID_LOG(FLUID_ERR, "Failed to start a new MIDI input thread");
        goto error_recovery;
    }
    
    return (fluid_midi_driver_t*) dev;

error_recovery:
    delete_fluid_android_amidi_driver ((fluid_midi_driver_t*) dev);
    return NULL;
}

static fluid_thread_return_t
fluid_android_amidi_run(void* d)
{
    fluid_android_amidi_driver_t *dev;
    fluid_midi_parser_t *parser;
    uint8_t *buf;
    int32_t opcode;
    ssize_t numMessages;
    size_t numBytesReceived;
    int64_t timestamp;
    size_t bufIdx;
    fluid_midi_event_t *evt;
    
    dev = (fluid_android_amidi_driver_t*) d;
    parser = dev->parser;
    buf = dev->receiver_buffer;
        
    while (dev->reading)
    {
        numMessages = amidi_receive(dev->port, &opcode, buf, dev->max_bytes, &numBytesReceived, &timestamp);
        
        for (bufIdx = 0; bufIdx < numBytesReceived; bufIdx++)
        {
            evt = fluid_midi_parser_parse(parser, buf [bufIdx]);
            if (evt != NULL)
                /* TODO: deal with timestamp (but there is no point to take it into consideration) */
                (*dev->driver.handler)(dev->driver.data, evt);
        }
    }
    
    return NULL;
}


/*
 * delete_fluid_android_amidi_driver
 */
void
delete_fluid_android_amidi_driver(fluid_midi_driver_t *d)
{
    fluid_android_amidi_driver_t *dev;

    dev = (fluid_android_amidi_driver_t*) d;
    
    if (dev == NULL)
        return;
    
    dev->reading = 0;
    
    if (dev->fluid_thread)
        fluid_thread_join(dev->fluid_thread);

    amidi_close(dev->port);
    
    amidi_release((AMidiDevice*) dev->driver.data);
    
    if (dev->receiver_buffer)
        FLUID_FREE(dev->receiver_buffer);

    if (dev->parser)
        delete_fluid_midi_parser(dev->parser);

    FLUID_FREE(dev);

	libamidi_refcount--;
	if (libamidi_refcount == 0)
		dlclose(libamidi);
}

#endif /* ANDROID_AMIDI_SUPPORT */
