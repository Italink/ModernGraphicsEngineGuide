#include "QSequence.h"
#include <QDebug>

void QSequence::play() {
	std::sort(mSections.begin(), mSections.end(), [](const Section& a, const Section& b) {
		return a.startMs < b.startMs;
	});
	for (int i = 0; i < mSections.size(); i++) {
		mSections[i].status = Section::Status::Pending;
	}
	mCurrStartIndex = 0;
	mLastTimeMs = 0.0f;
	mTimer.setInterval(10);
	connect(&mTimer, &QTimer::timeout, this, &QSequence::tick);
	mTimer.start();
	mElapsedTimer.restart();
}

void QSequence::tick() {
	float currTimeMs = mElapsedTimer.elapsed() / 1000.0f;
	float delta = currTimeMs - mLastTimeMs;
	for (int i = mCurrStartIndex; i < mSections.size(); i++) {
		Section& section = mSections[i];
		Context context;
		context.startMs = section.startMs;
		context.durationMs = section.durationMs;
		context.currTimeMs = currTimeMs;
		context.deltaMs = delta;
		if (section.startMs < currTimeMs) {
			if (section.durationMs > 0.0f) {
				if (section.status == Section::Status::Pending) {
					section.status = Section::Status::Start;
					if (section.begin) {
						section.begin(context);
					}
				}
				else if (section.startMs + section.durationMs < currTimeMs && section.status != Section::Status::End) {
					section.status = Section::Status::End;
					if (section.end) {
						section.end(context);
					}
				}
				else if (section.status == Section::Status::Start) {
					section.status = Section::Status::Tick;
				}
				if (section.status == Section::Status::Tick) {
					if (section.tick) {
						section.tick(context);
					}
				}
			}
			else {
				if (section.startMs <= currTimeMs && section.status == Section::Status::Pending) {
					section.status = Section::Status::Start;
					if (section.begin) {
						section.begin(context);
					}
				}
			}
		}
		if ((section.durationMs > 0.0f && section.status == Section::Status::End)
			|| (section.durationMs < 0.0f && section.status == Section::Status::Start)) {
			mCurrStartIndex = i+1;
		}
	}
	mLastTimeMs = currTimeMs;
}

void QSequence::addSection(float startMs, Section::BeginFunc begin) {
	Section section;
	section.startMs = startMs;
	section.begin = begin;
	mSections << section;
}

void QSequence::addSection(float startMs, float durationMs, Section::BeginFunc begin, Section::TickFunc tick, Section::EndFunc end) {
	Section section;
	section.startMs = startMs;
	section.durationMs = durationMs;
	section.begin = begin;
	section.tick = tick;
	section.end = end;
	mSections << section;
}
