#pragma once
#ifndef STOPWATCH_H
#define STOPWATCH_H

#include <chrono>

namespace common {
	using namespace std::chrono;

	class Stopwatch {
		bool running;
		high_resolution_clock::time_point startTime, endTime;

		nanoseconds getElapsedTime();

	public:
		Stopwatch();

		void start();
		void stop();

		float getSeconds();
		float getMillis();
		float getMicros();
		float getNanos();
	};
}
#endif