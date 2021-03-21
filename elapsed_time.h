#pragma once

#include <sys/time.h>
#include <inttypes.h>

uint64_t timeDelta;
uint64_t curTime;

static uint64_t _getSeconds(void)
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return t.tv_sec * 1000 + t.tv_usec / 1000;
}

static uint64_t prevFrameTime;

void updateFrameTime(void)
{
	uint64_t time = _getSeconds();
	if (prevFrameTime)
		timeDelta = time - prevFrameTime;
	prevFrameTime = time;
	curTime = time;
}

double getFrameTime(void)
{
	return (double)_getSeconds() - prevFrameTime;
}
