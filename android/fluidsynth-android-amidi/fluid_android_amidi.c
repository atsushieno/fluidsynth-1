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

#if defined(ANDROID) || defined(__DOXYGEN__)

#define FLUID_DEPRECATED 
#define FLUIDSYNTH_API 
#include <jni.h>
#include <amidi/AMidi.h>
#include "fluid_android_amidi.h"
#include <fluidsynth/midi.h>
#include <fluidsynth/synth.h>

void* Java_fluidsynth_androidextensions_NativeHandler_getAMidiDevice(JNIEnv *env, jobject _this, jobject midiDevice)
{
  AMidiDevice *amidi_device;

  AMidiDevice_fromJava(env, midiDevice, &amidi_device);
  return amidi_device;
}

void Java_fluidsynth_androidextensions_NativeHandler_releaseAMidiDevice(void *amidi)
{
  AMidiDevice_release((AMidiDevice*) amidi);
}

int handle_midi_event (void *synthAsData, fluid_midi_event_t *event)
{
  return fluid_synth_handle_midi_event(synthAsData, event);
}

void fluid_android_connect_midi_driver_to_synth(fluid_settings_t *settings, fluid_synth_t *synth, void *amidi)
{
  fluid_settings_setnum(settings, "midi.android-amidi.device-handle", (double) (long) amidi);
  
  new_fluid_midi_driver(settings, handle_midi_event, synth);
}

#endif /* if defined(ANDROID) || defined(__DOXYGEN__) */
