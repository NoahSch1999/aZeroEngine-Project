#pragma once
#include <chrono>

class Timer
{
public:
	Timer()
	{
		deltaTime = 0.0;
		previousTime = 0;
		currentTime = 0;
		numFrames = 0;
		totalTime = 0;

		__int64 tempSecondsPerCount;
		QueryPerformanceFrequency((LARGE_INTEGER*)&tempSecondsPerCount);
		secondsPerCount = 1.0 / (double)tempSecondsPerCount;
	}

	void Update()
	{
		// Time of this frame
		QueryPerformanceCounter((LARGE_INTEGER*)&currentTime);

		// Time difference between this and the last frame
		deltaTime = (currentTime - previousTime) * secondsPerCount;

		// Setup for next frames calculations
		previousTime = currentTime;

		// Can eventually be negative if the processor goes into power saving mode
		if (deltaTime < 0.0)
			deltaTime = 0.0;

		totalTime += currentTime - previousTime;
	}

	void StartCountDown()
	{
		startTime = std::chrono::steady_clock::now();
	}

	template <typename T>
	int StopCountDown()
	{
		return std::chrono::duration_cast<T> (std::chrono::steady_clock::now() - startTime).count();
	}

	std::chrono::time_point<std::chrono::steady_clock> startTime;
	double deltaTime;
	__int64 currentTime;
	__int64 previousTime;
	double secondsPerCount;
	int numFrames;
	double totalTime;
};