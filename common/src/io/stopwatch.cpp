#include "Stopwatch.h"

namespace common {
	Stopwatch::Stopwatch() : running(false) {}

	nanoseconds Stopwatch::getElapsedTime() {
		if (running) {
			endTime=high_resolution_clock::now();
		}
		return duration_cast<nanoseconds>(endTime-startTime);
	}

	void Stopwatch::start() {
		if (!running) {
			startTime=high_resolution_clock::now();
			running=true;
		}
	}

	void Stopwatch::stop() {
		if (running) {
			endTime=high_resolution_clock::now();
			running=false;
		}
	}

	float Stopwatch::getSeconds() {
		return duration_cast<seconds>(getElapsedTime()).count();
	}

	float Stopwatch::getMillis() {
		return duration_cast<milliseconds>(getElapsedTime()).count();
	}

	float Stopwatch::getMicros() {
		return duration_cast<microseconds>(getElapsedTime()).count();
	}

	float Stopwatch::getNanos() {
		return duration_cast<nanoseconds>(getElapsedTime()).count();
	}
}