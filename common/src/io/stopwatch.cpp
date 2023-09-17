#include "stopwatch.h"

namespace common {
	stopwatch::stopwatch() : running(false) {}

	nanoseconds stopwatch::getElapsedTime() {
		if (running) {
			endTime=high_resolution_clock::now();
		}
		return duration_cast<nanoseconds>(endTime-startTime);
	}

	void stopwatch::start() {
		if (!running) {
			startTime=high_resolution_clock::now();
			running=true;
		}
	}

	void stopwatch::stop() {
		if (running) {
			endTime=high_resolution_clock::now();
			running=false;
		}
	}

	float stopwatch::getSeconds() {
		return duration_cast<seconds>(getElapsedTime()).count();
	}

	float stopwatch::getMillis() {
		return duration_cast<milliseconds>(getElapsedTime()).count();
	}

	float stopwatch::getMicros() {
		return duration_cast<microseconds>(getElapsedTime()).count();
	}

	float stopwatch::getNanos() {
		return duration_cast<nanoseconds>(getElapsedTime()).count();
	}
}