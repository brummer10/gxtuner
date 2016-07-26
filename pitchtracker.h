/*
 * Copyright (C) 2011 Hermann Meyer, Andreas Degert
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
 * ---------------------------------------------------------------------------
 *
 *        file: pitchtracker.h   guitar tuner for jack
 *
 * ----------------------------------------------------------------------------
 */

#pragma once

#ifndef PITCH_TRACKER_H_
#define PITCH_TRACKER_H_

#include <fftw3.h>
#include <semaphore.h>
#include <assert.h> 
#include <pthread.h>

#include <cstring> 
#include <cmath>
#include <cstdlib>

#include "resample.h"

/* ------------- Pitch Tracker ------------- */

const int MAX_FFT_SIZE = 512; // The size of the read buffer (max FFT window size).

class PitchTracker {
 public:
    explicit PitchTracker();
    ~PitchTracker();
    bool                pt_initialized;
    bool                setParameters(int sampleRate, int fftSize, pthread_t j_thread);
    void                init(int samplerate, pthread_t j_thread)
                        {setParameters(samplerate, MAX_FFT_SIZE, j_thread);}
    void                add(int count, float *input);
    float               tuner_estimate();
    void                set_threshold(float t);
    float               get_threshold();
    void                stop_thread();
    float               estimated_freq;

 private:
    void                run();
    static void*        static_run(void* p);
    void                setEstimatedFrequency(float freq);
    int                 find_minimum();
    int                 find_maximum(int l);
    void                start_thread();
    void                copy();
    bool                error;
    volatile bool       busy;
    int                 tick;
    sem_t               m_trig;
    pthread_t           m_pthr;
    pthread_t           jack_thread;
    Resampler           *resamp;
    int                 m_sampleRate;
    float               SIGNAL_THRESHOLD_ON;
    float               SIGNAL_THRESHOLD_OFF;
    // Size of the FFT window.
    int                 m_fftSize;
    // The audio buffer that stores the input signal.
    float*              m_buffer;
    // Index of the first empty position in the buffer.
    int                 m_bufferIndex;
    // Whether or not the input level is high enough.
    bool                m_audioLevel;
    // Support buffer used to store signals in the time domain.
    float*              m_fftwBufferTime;
    // Support buffer used to store signals in the frequency domain.
    fftwf_complex*      m_fftwBufferFreq;
    // Plan to compute the FFT of a given signal.
    fftwf_plan          m_fftwPlanFFT;
    // Plan to compute the IFFT of a given signal (with additional zero-padding).
    fftwf_plan          m_fftwPlanIFFT;
};

extern PitchTracker pitch_tracker;

#endif // PITCH_TRACKER_H_

