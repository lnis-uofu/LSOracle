/*********************************************************
MiniSat -- Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
CryptoMiniSat -- Copyright (c) 2009 Mate Soos

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
***********************************************************/

#ifndef TIME_MEM_H
#define TIME_MEM_H
#include <assert.h>
#include <time.h>

// note: MinGW64 defines both __MINGW32__ and __MINGW64__
#if defined (_MSC_VER) || defined (__MINGW32__) || defined(_WIN32)
static inline double cpuTime(void)
{
    return (double)clock() / CLOCKS_PER_SEC;
}
static inline double cpuTimeTotal(void)
{
    return (double)clock() / CLOCKS_PER_SEC;
}

#else //_MSC_VER
#include <sys/time.h>
#include <sys/resource.h>
#include <unistd.h>

static inline double cpuTime(void)
{
    struct rusage ru;
    #ifdef RUSAGE_THREAD
    int ret = getrusage(RUSAGE_THREAD, &ru);
    #else
    int ret = getrusage(RUSAGE_SELF, &ru);
    #endif

    //NOTE: This is needed because Windows' Linux subsystem returns non-zero
    //and I can't figure out a way to detect Windows.
    if (ret != 0) {
        return (double)clock() / CLOCKS_PER_SEC;
    }

    return (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000.0;
}

static inline double cpuTimeTotal(void)
{
    struct rusage ru;
    int ret = getrusage(RUSAGE_SELF, &ru);
    assert(ret == 0);

    return (double)ru.ru_utime.tv_sec + (double)ru.ru_utime.tv_usec / 1000000.0;
}

#endif

#endif //TIME_MEM_H
