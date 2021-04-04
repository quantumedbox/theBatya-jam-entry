#pragma once

#include <sys/time.h>
#include <inttypes.h>
#include <windows.h>

uint64_t timeDelta;
uint64_t curTime;

uint64_t getSeconds(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec * 1000 + t.tv_usec / 1000;
}

static uint64_t prevFrameTime;

void updateFrameTime(void)
{
	uint64_t time = getSeconds();
	if (prevFrameTime)
		timeDelta = time - prevFrameTime;
	prevFrameTime = time;
	curTime = time;
}

double getFrameTime(void)
{
	return (double)getSeconds() - prevFrameTime;
}

/*
*	@brief returns processor time that elapsed between getProfileTime() calls
*/

double getProfileTime()
{
	static double prevTime = 0; 

    LARGE_INTEGER t, f;
    QueryPerformanceCounter(&t);
    QueryPerformanceFrequency(&f);

    double time = (double)t.QuadPart/(double)f.QuadPart;
    double rtrn = time - prevTime;
    prevTime = time;

    return rtrn;
}

/*
*	@brief prints the getProfileTime() result with given description text to stdout
*/

void printProfileTime(char* description)
{
	fprintf(stdout, "%s: %lf\n", description, getProfileTime());
}
