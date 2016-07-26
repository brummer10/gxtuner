/*
 * Copyright (C) 2009, 2010 Hermann Meyer, James Warden, Andreas Degert
 * Copyright (C) 2011 Pete Shorthose
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 * --------------------------------------------------------------------------
 */


#pragma once

#ifndef GX_PITCH_TRACKER_H_
#define GX_PITCH_TRACKER_H_

#include <fftw3.h>
#include <semaphore.h>
#include <assert.h> 
#include <pthread.h>

#include <cstring> 
#include <cmath>
#include <cstdlib>

#include "resample.h"

/* ------------- Pitch Tracker ------------- */

class PitchTracker {
 public:
    explicit PitchTracker();
    ~PitchTracker();
    void            init(int samplerate, pthread_t j_thread);
    void            add(int count, float *input);
    float           get_estimated_freq() { return m_freq < 0 ? 0 : m_freq; }
    float           get_estimated_note();
    void            stop_thread();
    void            reset();
    void            set_threshold(float v);
    float            get_threshold();
    void            set_fast_note_detection(bool v);
    //Glib::Dispatcher new_freq;
 private:
    bool            setParameters(int sampleRate, int buffersize, pthread_t j_thread);
    void            run();
    static void     *static_run(void* p);
    void            start_thread();
    void            copy();
    bool            error;
    volatile bool   busy;
    int             tick;
    sem_t           m_trig;
    pthread_t       m_pthr;
    pthread_t       jack_thread;
    Resampler       resamp;
    int             m_sampleRate;
    int             fixed_sampleRate;
    float           m_freq;
    // Value of the threshold above which
    // the processing is activated.
    float           signal_threshold_on;
    // Value of the threshold below which
    // the input audio signal is deactivated.
    float           signal_threshold_off;
    // Time between frequency estimates (in seconds)
    float           tracker_period;
    // number of samples in input buffer
    int             m_buffersize;
    // Size of the FFT window.
    int             m_fftSize;
    // The audio buffer that stores the input signal.
    float           *m_buffer;
    // Index of the first empty position in the buffer.
    int             m_bufferIndex;
    // buffer for input signal
    float           *m_input;
    // Whether or not the input level is high enough.
    bool            m_audioLevel;
    // Support buffer used to store signals in the time domain.
    float          *m_fftwBufferTime;
    // Support buffer used to store signals in the frequency domain.
    float          *m_fftwBufferFreq;
    // Plan to compute the FFT of a given signal.
    fftwf_plan      m_fftwPlanFFT;
    // Plan to compute the IFFT of a given signal (with additional zero-padding).
    fftwf_plan      m_fftwPlanIFFT;
};

extern PitchTracker pitch_tracker;

#endif  // SRC_HEADERS_GX_PITCH_TRACKER_H_
