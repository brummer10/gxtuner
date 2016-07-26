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
 *        file: pitchtracker.cpp      guitar tuner for jack
 *
 * ----------------------------------------------------------------------------
 */


#include "./pitchtracker.h"


// downsampling factor
const int DOWNSAMPLE = 16;
// Number of times that the FFT is zero-padded to increase frequency resolution.
const int ZERO_PADDING_FACTOR = 64;
// Time between frequency estimates (in seconds)
const float TRACKER_PERIOD = 0.1;

void *PitchTracker::static_run(void *p) {
    (reinterpret_cast<PitchTracker *>(p))->run();
    return NULL;
}

void PitchTracker::set_threshold(float t) {
    // Value of the threshold above which the processing is activated.
    SIGNAL_THRESHOLD_ON = t;
    // Value of the threshold below which the input audio signal is deactivated.
    SIGNAL_THRESHOLD_OFF = t-0.0001;
}

float PitchTracker::get_threshold() {
    // Value of the threshold above which the processing is activated.
    return SIGNAL_THRESHOLD_ON;
}

PitchTracker::PitchTracker()
    : error(false),
    busy(false),
    tick(0),
    m_pthr(0),
    resamp(new Resampler),
    m_buffer(new float[MAX_FFT_SIZE]),
    m_bufferIndex(0),
    m_audioLevel(false),
    m_fftwPlanFFT(0),
    m_fftwPlanIFFT(0) {
    const int fftw_buffer_size = MAX_FFT_SIZE * ZERO_PADDING_FACTOR;
    m_fftwBufferTime = reinterpret_cast<float*>
                       (fftwf_malloc(fftw_buffer_size * sizeof(float)));
    m_fftwBufferFreq = reinterpret_cast<fftwf_complex*>
                       (fftwf_malloc(fftw_buffer_size * sizeof(fftwf_complex)));

    memset(m_buffer, 0, MAX_FFT_SIZE * sizeof(float));
    memset(m_fftwBufferTime, 0, fftw_buffer_size * sizeof(float));
    memset(m_fftwBufferFreq, 0, fftw_buffer_size * sizeof(fftwf_complex));

    sem_init(&m_trig, 0, 0);

    if (!m_buffer || !m_fftwBufferTime || !m_fftwBufferFreq) {
        error = true;
    }
}

PitchTracker::~PitchTracker() {
    fftwf_destroy_plan(m_fftwPlanFFT);
    fftwf_destroy_plan(m_fftwPlanIFFT);
    fftwf_free(m_fftwBufferTime);
    fftwf_free(m_fftwBufferFreq);
    delete[] m_buffer;
}

bool PitchTracker::setParameters(int sampleRate, int fftSize, pthread_t j_thread) {
    assert(fftSize <= MAX_FFT_SIZE);

    if (error) {
        return false;
    }
    m_sampleRate = sampleRate / DOWNSAMPLE;
    resamp->setup(sampleRate, m_sampleRate, 1, 16); // 16 == least quality
    SIGNAL_THRESHOLD_ON = 0.001;
    SIGNAL_THRESHOLD_OFF = 0.0009;
    jack_thread = j_thread;
    estimated_freq = 0.0;

    if (m_fftSize != fftSize) {
        m_fftSize = fftSize;
        fftwf_destroy_plan(m_fftwPlanFFT);
        fftwf_destroy_plan(m_fftwPlanIFFT);
        m_fftwPlanFFT = fftwf_plan_dft_r2c_1d(
            m_fftSize, m_fftwBufferTime, m_fftwBufferFreq, FFTW_ESTIMATE); // FFT
        m_fftwPlanIFFT = fftwf_plan_dft_c2r_1d(
            ZERO_PADDING_FACTOR * m_fftSize, m_fftwBufferFreq,
            m_fftwBufferTime, FFTW_ESTIMATE); // IFFT zero-padded
    }

    if (!m_fftwPlanFFT || !m_fftwPlanIFFT) {
        error = true;
        return false;
    }

    if (!m_pthr) {
        start_thread();
    }
    pt_initialized = true;
    return !error;
}

void PitchTracker::stop_thread() {
    pthread_cancel (m_pthr);
    pthread_join (m_pthr, NULL);
    sem_post(&m_trig);
    delete resamp;
    resamp = 0;
}

void PitchTracker::start_thread() {
    int                min = 0, max = 0;
    pthread_attr_t     attr;
    struct sched_param  spar;
    int priority, policy;
    pthread_getschedparam(jack_thread, &policy, &spar);
    priority = spar.sched_priority;
    min = sched_get_priority_min(policy);
    max = sched_get_priority_max(policy);
    priority -= 6; // zita-convoler uses 5 levels
    if (priority > max) priority = max;
    if (priority < min) priority = min;
    spar.sched_priority = priority;
    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr,PTHREAD_CREATE_JOINABLE );
    pthread_setcancelstate (PTHREAD_CANCEL_ENABLE, NULL);
    pthread_attr_setschedpolicy(&attr, policy);
    pthread_attr_setschedparam(&attr, &spar);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setinheritsched(&attr, PTHREAD_EXPLICIT_SCHED);
    // pthread_attr_setstacksize(&attr, 0x10000);
    if (pthread_create(&m_pthr, &attr, static_run, reinterpret_cast<void*>(this))) {
        error = true;
    }
    pthread_attr_destroy(&attr);
}

int PitchTracker::find_minimum() {
    const int peakwidth = 3;
    float *p = &m_fftwBufferTime[peakwidth];
    for ( ; p < &m_fftwBufferTime[ZERO_PADDING_FACTOR * m_fftSize / 2 + 1 - peakwidth]; p++) {
        int i;
        for (i = -peakwidth; i <= peakwidth; i++) {
            if (*p > p[i]) {
                break;
            }
        }
        if (i > peakwidth) {
            break;
        }
    }
    return static_cast<int>((p - m_fftwBufferTime));
}

int PitchTracker::find_maximum(int l) {
    float    maxAutocorr            = 0.0;
    int        maxAutocorrIndex    = 0;
    while (l < ZERO_PADDING_FACTOR * m_fftSize / 2 + 1) {
        if (m_fftwBufferTime[l] > maxAutocorr) {
            maxAutocorr = m_fftwBufferTime[l];
            maxAutocorrIndex = l;
        }
        ++l;
    }
    if (maxAutocorr == 0.0) {
        return -1;
    }
    return maxAutocorrIndex;
}

float show_level(int n, float *buf) {
    float sum = 0.0;
    for (int k = 0; k < n; ++k) {
        sum += fabs(buf[k]);
    }
    return sum;
}

void PitchTracker::add(int count, float* input) {
    if (error) {
        return;
    }
    resamp->inp_count = count;
    resamp->inp_data = input;
    for (;;) {
        resamp->out_data = &m_buffer[m_bufferIndex];
        int n = MAX_FFT_SIZE - m_bufferIndex;
        resamp->out_count = n;
        resamp->process();
        n -= resamp->out_count; // n := number of output samples
        if (!n) { // all soaked up by filter
            return;
        }
        m_bufferIndex = (m_bufferIndex + n) % MAX_FFT_SIZE;
        if (resamp->inp_count == 0) {
            break;
        }
    }
    if (++tick * count >= m_sampleRate * DOWNSAMPLE * TRACKER_PERIOD) {
        if (busy) {
            return;
        }
        tick = 0;
        copy();
        sem_post(&m_trig);
    }
}

void PitchTracker::copy() {
    int start = (MAX_FFT_SIZE + m_bufferIndex - m_fftSize) % MAX_FFT_SIZE;
    int end = (MAX_FFT_SIZE + m_bufferIndex) % MAX_FFT_SIZE;
    int cnt = 0;
    if (start >= end) {
        cnt = MAX_FFT_SIZE - start;
        memcpy(m_fftwBufferTime, &m_buffer[start], cnt * sizeof(float));
        start = 0;
    }
    memcpy(&m_fftwBufferTime[cnt], &m_buffer[start], (end - start) * sizeof(float));
}

void PitchTracker::run() {
    for (;;) {
        busy = false;
        sem_wait(&m_trig);
        pthread_testcancel();
        busy = true;
        if (error) {
            continue;
        }
        float sum = 0.0;
        for (int k = 0; k < m_fftSize; ++k) {
            sum += fabs(m_fftwBufferTime[k]);
        }
        float threshold = (m_audioLevel ? SIGNAL_THRESHOLD_OFF : SIGNAL_THRESHOLD_ON);
        m_audioLevel = (sum / m_fftSize >= threshold);
        if ( m_audioLevel == false ) {
            setEstimatedFrequency(0.0);
            continue;
        }

        /* Compute the transform of the autocorrelation given in time domain by
         *           k=-N
         *    r[t] = sum( x[k] * x[t-k] )
         *            N
         * or in the frequency domain (for a real signal) by
         *    R[f] = X[f] * X[f]' = |X[f]|^2 = Re(X[f])^2 + Im(X[f])^2
         * When computing the FFT with fftwf_plan_dft_r2c_1d there are only N/2+1
         * significant samples, so |.|^2 is computed for m_fftSize/2+1 samples only
         */
        int fftRSize = m_fftSize/2 + 1;
        fftwf_execute(m_fftwPlanFFT);
        for (int k = 0; k < fftRSize; ++k) {
            fftwf_complex& v = m_fftwBufferFreq[k];
            v[0] = v[0]*v[0] + v[1]*v[1];
            v[1] = 0.0;
        }

        // pad the FFT with zeros to increase resolution in time domain after IFFT
        int size_with_padding = ZERO_PADDING_FACTOR * m_fftSize - fftRSize;
        memset(&m_fftwBufferFreq[fftRSize][0], 0, size_with_padding * sizeof(fftwf_complex));
        fftwf_execute(m_fftwPlanIFFT);

        // search for a minimum and then for the next maximum to get the estimated frequency
        int maxAutocorrIndex = find_maximum(find_minimum());

        // compute the frequency of the maximum considering the padding factor
        if (maxAutocorrIndex >= 0) {
            setEstimatedFrequency(ZERO_PADDING_FACTOR * m_sampleRate
                                  / static_cast<float>(maxAutocorrIndex));
        } else {
            setEstimatedFrequency(0.0);
        }
        busy = false;
    }
}

void PitchTracker::setEstimatedFrequency(float freq) {
    estimated_freq = freq;
}

PitchTracker pitch_tracker;


