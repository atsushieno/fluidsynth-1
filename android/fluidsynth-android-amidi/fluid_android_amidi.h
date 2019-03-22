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


#ifndef _PRIV_FLUID_ANDROID_AMIDI_H
#define _PRIV_FLUID_ANDROID_AMIDI_H

#ifdef __cplusplus
extern "C" {
#endif

#include <fluidsynth/types.h>
#include <fluidsynth/settings.h>

void * Java_fluidsynth_androidextensions_NativeHandler_acquireMidiDeviceHandle(JNIEnv *env, jobject _this, jobject midiDevice);
void Java_fluidsynth_androidextensions_NativeHandler_releaseAMidiDevice(void *amidi);
void fluid_android_connect_midi_driver_to_synth(fluid_settings_t *settings, fluid_synth_t *synth, void *amidiDevice);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* _PRIV_FLUID_ANDROID_AMIDI_H */
