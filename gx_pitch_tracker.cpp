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

/* ------- This is the guitarix tuner, part of gx_engine_audio.cpp ------- */

/****************************************************************
 ** Pitch Tracker
 **
 ** some code and ideas taken from K4Guitune (William Spinelli)
 ** changed to NSDF method (some code from tartini / Philip McLeod)
 **
 */

#include "./gx_pitch_tracker.h"

// downsampling factor
static const int DOWNSAMPLE = 2;
static const float SIGNAL_THRESHOLD_ON = 0.001;
static const float SIGNAL_THRESHOLD_OFF = 0.0009;
static const float TRACKER_PERIOD = 0.1;
// The size of the read buffer
static const int FFT_SIZE = 2048;

#define max(x, y) (((x) > (y)) ? (x) : (y))
#define min(x, y) (((x) < (y)) ? (x) : (y))


inline void Dsp::clear_state_f()
{
	for (int i=0; i<2; i++) iVec0[i] = 0;
	for (int i=0; i<2; i++) fRec4[i] = 0;
	for (int i=0; i<2; i++) fVec1[i] = 0;
	for (int i=0; i<2; i++) fRec3[i] = 0;
	for (int i=0; i<2; i++) fRec2[i] = 0;
	for (int i=0; i<3; i++) fRec1[i] = 0;
	for (int i=0; i<3; i++) fRec0[i] = 0;
}

inline void Dsp::init(int samplingFreq)
{
	fSamplingFreq = samplingFreq;
	iConst0 = min(192000, max(1, fSamplingFreq));
	fConst1 = tan((3138.4510609362032 / double(iConst0)));
	fConst2 = (2 * (1 - (1.0 / pow(fConst1,2))));
	fConst3 = (1.0 / fConst1);
	fConst4 = (1 + ((fConst3 - 0.7653668647301795) / fConst1));
	fConst5 = (1.0 / (1 + ((0.7653668647301795 + fConst3) / fConst1)));
	fConst6 = (1 + ((fConst3 - 1.8477590650225735) / fConst1));
	fConst7 = (1.0 / (1 + ((fConst3 + 1.8477590650225735) / fConst1)));
	fConst8 = (72.25663103256524 / double(iConst0));
	fConst9 = (1 - fConst8);
	fConst10 = (1.0 / (1 + fConst8));
	clear_state_f();
}

void Dsp::compute(int count, float *input0, float *output0)
{
	for (int i=0; i<count; i++) {
		iVec0[0] = 1;
		fRec4[0] = ((1e-20 * (1 - iVec0[1])) - fRec4[1]);
		double fTemp0 = ((double)input0[i] + fRec4[0]);
		fVec1[0] = fTemp0;
		fRec3[0] = (fConst10 * ((fVec1[0] - fVec1[1]) + (fConst9 * fRec3[1])));
		fRec2[0] = (fConst10 * ((fRec3[0] - fRec3[1]) + (fConst9 * fRec2[1])));
		fRec1[0] = (fRec2[0] - (fConst7 * ((fConst6 * fRec1[2]) + (fConst2 * fRec1[1]))));
		fRec0[0] = ((fConst7 * (fRec1[2] + (fRec1[0] + (2 * fRec1[1])))) - (fConst5 * ((fConst4 * fRec0[2]) + (fConst2 * fRec0[1]))));
		output0[i] = (fConst5 * (fRec0[2] + (fRec0[0] + (2 * fRec0[1]))));
		// post processing
		fRec0[2] = fRec0[1]; fRec0[1] = fRec0[0];
		fRec1[2] = fRec1[1]; fRec1[1] = fRec1[0];
		fRec2[1] = fRec2[0];
		fRec3[1] = fRec3[0];
		fVec1[1] = fVec1[0];
		fRec4[1] = fRec4[0];
		iVec0[1] = iVec0[0];
	}
}


void *PitchTracker::static_run(void *p) {
    (reinterpret_cast<PitchTracker *>(p))->run();
    return NULL;
}

PitchTracker::PitchTracker()
    : error(false),
      busy(false),
      tick(0),
      m_pthr(0),
      resamp(),
      m_sampleRate(),
      fixed_sampleRate(41000),
      m_freq(-1),
      signal_threshold_on(SIGNAL_THRESHOLD_ON),
      signal_threshold_off(SIGNAL_THRESHOLD_OFF),
      tracker_period(TRACKER_PERIOD),
      m_buffersize(),
      m_fftSize(),
      m_buffer(new float[FFT_SIZE]),
      m_bufferIndex(0),
      m_input(new float[FFT_SIZE]),
      m_audioLevel(false),
      m_fftwPlanFFT(0),
      m_fftwPlanIFFT(0) {
    const int size = FFT_SIZE + (FFT_SIZE+1) / 2;
    m_fftwBufferTime = reinterpret_cast<float*>
                       (fftwf_malloc(size * sizeof(*m_fftwBufferTime)));
    m_fftwBufferFreq = reinterpret_cast<float*>
                       (fftwf_malloc(size * sizeof(*m_fftwBufferFreq)));

    memset(m_buffer, 0, FFT_SIZE * sizeof(*m_buffer));
    memset(m_input, 0, FFT_SIZE * sizeof(*m_input));
    memset(m_fftwBufferTime, 0, size * sizeof(*m_fftwBufferTime));
    memset(m_fftwBufferFreq, 0, size * sizeof(*m_fftwBufferFreq));

    sem_init(&m_trig, 0, 0);

    if (!m_buffer || !m_input || !m_fftwBufferTime || !m_fftwBufferFreq) {
        error = true;
    }
}


PitchTracker::~PitchTracker() {
    stop_thread();
    fftwf_destroy_plan(m_fftwPlanFFT);
    fftwf_destroy_plan(m_fftwPlanIFFT);
    fftwf_free(m_fftwBufferTime);
    fftwf_free(m_fftwBufferFreq);
    delete[] m_input;
    delete[] m_buffer;
}

void PitchTracker::set_threshold(float v) {
    signal_threshold_on = v;
    signal_threshold_off = v*0.9;
}

float PitchTracker::get_threshold() {
    // Value of the threshold above which the processing is activated.
    return signal_threshold_on;
}

void PitchTracker::set_fast_note_detection(bool v) {
    if (v) {
	signal_threshold_on = SIGNAL_THRESHOLD_ON * 5;
	signal_threshold_off = SIGNAL_THRESHOLD_OFF * 5;
	tracker_period = TRACKER_PERIOD / 10;
    } else {
	signal_threshold_on = SIGNAL_THRESHOLD_ON;
	signal_threshold_off = SIGNAL_THRESHOLD_OFF;
	tracker_period = TRACKER_PERIOD;
    }
}

bool PitchTracker::setParameters(int sampleRate, int buffersize, pthread_t j_thread) {
    assert(buffersize <= FFT_SIZE);

    if (error) {
        return false;
    }
    m_sampleRate = fixed_sampleRate / DOWNSAMPLE;
    resamp.setup(sampleRate, m_sampleRate, 1, 16); // 16 == least quality
    jack_thread = j_thread;

    if (m_buffersize != buffersize) {
        m_buffersize = buffersize;
        m_fftSize = m_buffersize + (m_buffersize+1) / 2;
        fftwf_destroy_plan(m_fftwPlanFFT);
        fftwf_destroy_plan(m_fftwPlanIFFT);
        m_fftwPlanFFT = fftwf_plan_r2r_1d(
                            m_fftSize, m_fftwBufferTime, m_fftwBufferFreq,
                            FFTW_R2HC, FFTW_ESTIMATE);
        m_fftwPlanIFFT = fftwf_plan_r2r_1d(
                             m_fftSize, m_fftwBufferFreq, m_fftwBufferTime,
                             FFTW_HC2R, FFTW_ESTIMATE);
    }

    if (!m_fftwPlanFFT || !m_fftwPlanIFFT) {
        error = true;
        return false;
    }

    if (!m_pthr) {
        start_thread();
    }
    low_high_cut.init(sampleRate);
    return !error;
}

void PitchTracker::stop_thread() {
    pthread_cancel (m_pthr);
    pthread_join (m_pthr, NULL);
}

void PitchTracker::start_thread() {
    int                min = 0, max = 0;
    pthread_attr_t      attr;
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
    if (pthread_create(&m_pthr, &attr, static_run,
                       reinterpret_cast<void*>(this))) {
        error = true;
    }
    pthread_attr_destroy(&attr);
}

void PitchTracker::init(int samplerate, pthread_t j_thread) {
    setParameters(samplerate, FFT_SIZE, j_thread);
}

void PitchTracker::reset() {
    tick = 0;
    m_bufferIndex = 0;
    resamp.reset();
    m_freq = -1;
}

void PitchTracker::add(int count, float* input) {
    if (error) {
        return;
    }
    float output[count];
    low_high_cut.compute(count,input,output);
    resamp.inp_count = count;
    resamp.inp_data = output;
    for (;;) {
        resamp.out_data = &m_buffer[m_bufferIndex];
        int n = FFT_SIZE - m_bufferIndex;
        resamp.out_count = n;
        resamp.process();
        n -= resamp.out_count; // n := number of output samples
        if (!n) { // all soaked up by filter
            return;
        }
        m_bufferIndex = (m_bufferIndex + n) % FFT_SIZE;
        if (resamp.inp_count == 0) {
            break;
        }
    }
    if (++tick * count >= m_sampleRate * DOWNSAMPLE * tracker_period) {
        if (busy) {
            return;
        }
        busy = true;
        tick = 0;
        copy();
        sem_post(&m_trig);
    }
}

void PitchTracker::copy() {
    int start = (FFT_SIZE + m_bufferIndex - m_buffersize) % FFT_SIZE;
    int end = (FFT_SIZE + m_bufferIndex) % FFT_SIZE;
    int cnt = 0;
    if (start >= end) {
        cnt = FFT_SIZE - start;
        memcpy(m_input, &m_buffer[start], cnt * sizeof(*m_input));
        start = 0;
    }
    memcpy(&m_input[cnt], &m_buffer[start], (end - start) * sizeof(*m_input));
}

inline float sq(float x) {
    return x * x;
}

inline void parabolaTurningPoint(float y_1, float y0, float y1, float xOffset, float *x) {
    float yTop = y_1 - y1;
    float yBottom = y1 + y_1 - 2 * y0;
    if (yBottom != 0.0) {
        *x = xOffset + yTop / (2 * yBottom);
    } else {
        *x = xOffset;
    }
}

static int findMaxima(float *input, int len, int *maxPositions, int *length, int maxLen) {
    int pos = 0;
    int curMaxPos = 0;
    int overallMaxIndex = 0;

    while (pos < (len-1)/3 && input[pos] > 0.0) {
        pos += 1;  // find the first negitive zero crossing
    }
    while (pos < len-1 && input[pos] <= 0.0) {
        pos += 1;  // loop over all the values below zero
    }
    if (pos == 0) {
        pos = 1;  // can happen if output[0] is NAN
    }
    while (pos < len-1) {
        if (input[pos] > input[pos-1] && input[pos] >= input[pos+1]) {  // a local maxima
            if (curMaxPos == 0) {
                curMaxPos = pos;  // the first maxima (between zero crossings)
            } else if (input[pos] > input[curMaxPos]) {
                curMaxPos = pos;  // a higher maxima (between the zero crossings)
            }
        }
        pos += 1;
        if (pos < len-1 && input[pos] <= 0.0) {  // a negative zero crossing
            if (curMaxPos > 0) {  // if there was a maximum
                maxPositions[*length] = curMaxPos;  // add it to the vector of maxima
                *length += 1;
                if (overallMaxIndex == 0) {
                    overallMaxIndex = curMaxPos;
                } else if (input[curMaxPos] > input[overallMaxIndex]) {
                    overallMaxIndex = curMaxPos;
                }
                if (*length >= maxLen) {
                    return overallMaxIndex;
                }
                curMaxPos = 0;  // clear the maximum position, so we start looking for a new ones
            }
            while (pos < len-1 && input[pos] <= 0.0) {
                pos += 1;  // loop over all the values below zero
            }
        }
    }
    if (curMaxPos > 0) {  // if there was a maximum in the last part
        maxPositions[*length] = curMaxPos;  // add it to the vector of maxima
        *length += 1;
        if (overallMaxIndex == 0) {
            overallMaxIndex = curMaxPos;
        } else if (input[curMaxPos] > input[overallMaxIndex]) {
            overallMaxIndex = curMaxPos;
        }
        curMaxPos = 0;  // clear the maximum position, so we start looking for a new ones
    }
    return overallMaxIndex;
}

static int findsubMaximum(float *input, int len, float threshold) {
    int indices[10];
    int length = 0;
    int overallMaxIndex = findMaxima(input, len, indices, &length, 10);
    if (length == 0) {
        return -1;
    }
    threshold += (1.0 - threshold) * (1.0 - input[overallMaxIndex]);
    float cutoff = input[overallMaxIndex] * threshold;
    for (int j = 0; j < length; j++) {
        if (input[indices[j]] >= cutoff) {
            return indices[j];
        }
    }
    // should never get here
    return -1;
}

void PitchTracker::run() {
    for (;;) {
        busy = false;
        sem_wait(&m_trig);
        if (error) {
            continue;
        }
        float sum = 0.0;
        for (int k = 0; k < m_buffersize; ++k) {
            sum += fabs(m_input[k]);
        }
        float threshold = (m_audioLevel ? signal_threshold_off : signal_threshold_on);
        m_audioLevel = (sum / m_buffersize >= threshold);
        if ( m_audioLevel == false ) {
	    if (m_freq != 0) {
		m_freq = 0;
		//new_freq();
	    }
            continue;
        }

        memcpy(m_fftwBufferTime, m_input, m_buffersize * sizeof(*m_fftwBufferTime));
        memset(m_fftwBufferTime+m_buffersize, 0, (m_fftSize - m_buffersize) * sizeof(*m_fftwBufferTime));
        fftwf_execute(m_fftwPlanFFT);
        for (int k = 1; k < m_fftSize/2; k++) {
            m_fftwBufferFreq[k] = sq(m_fftwBufferFreq[k]) + sq(m_fftwBufferFreq[m_fftSize-k]);
            m_fftwBufferFreq[m_fftSize-k] = 0.0;
        }
        m_fftwBufferFreq[0] = sq(m_fftwBufferFreq[0]);
        m_fftwBufferFreq[m_fftSize/2] = sq(m_fftwBufferFreq[m_fftSize/2]);

        fftwf_execute(m_fftwPlanIFFT);

        double sumSq = 2.0 * static_cast<double>(m_fftwBufferTime[0]) / static_cast<double>(m_fftSize);
        for (int k = 0; k < m_fftSize - m_buffersize; k++) {
            m_fftwBufferTime[k] = m_fftwBufferTime[k+1] / static_cast<float>(m_fftSize);
        }

        int count = (m_buffersize + 1) / 2;
        for (int k = 0; k < count; k++) {
            sumSq  -= sq(m_input[m_buffersize-1-k]) + sq(m_input[k]);
            // dividing by zero is very slow, so deal with it seperately
            if (sumSq > 0.0) {
                m_fftwBufferTime[k] *= 2.0 / sumSq;
            } else {
                m_fftwBufferTime[k] = 0.0;
            }
        }
	const float thres = 0.99; // was 0.6
        int maxAutocorrIndex = findsubMaximum(m_fftwBufferTime, count, thres);

        float x = 0.0;
        if (maxAutocorrIndex >= 0) {
            parabolaTurningPoint(m_fftwBufferTime[maxAutocorrIndex-1],
                                 m_fftwBufferTime[maxAutocorrIndex],
                                 m_fftwBufferTime[maxAutocorrIndex+1],
                                 maxAutocorrIndex+1, &x);
            x = m_sampleRate / x;
            if (x > 1060.0) {  // precision drops above 1000 Hz
                x = 0.0;
            }
        }
	if (m_freq != x) {
	    m_freq = x;
	    //new_freq();
	}
    }
}

float PitchTracker::get_estimated_note() {
    return m_freq <= 0.0 ? 1000.0 : 12 * log2f(2.272727e-03f * m_freq);
}

PitchTracker pitch_tracker;

