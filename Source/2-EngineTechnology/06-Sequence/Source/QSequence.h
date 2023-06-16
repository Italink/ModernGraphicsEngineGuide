
#ifndef QSequence_h__
#define QSequence_h__

#include <QObject>
#include <QElapsedTimer>
#include <QTimer>

class QSequence : public QObject {
protected:
	struct Context {
		float startMs;
		float durationMs;
		float currTimeMs;
		float deltaMs;
	};

	struct Section{
		float startMs;
		float durationMs = -1.0f;
		using BeginFunc = std::function<void(const Context&)>;
		using TickFunc = std::function<void(const Context&)>;
		using EndFunc = std::function<void(const Context&)>;
		BeginFunc begin;
		TickFunc tick;
		EndFunc end;
		enum class Status {
			Pending,
			Start,
			Tick,
			End
		}status = Status::Pending;
	};
public:
	void play();
protected:
	void tick();
	void addSection(float startMs, Section::BeginFunc begin);
	void addSection(float startMs, float durationMs, Section::BeginFunc begin, Section::TickFunc tick , Section::EndFunc end);
private:
	QElapsedTimer mElapsedTimer;
	QTimer mTimer;
	QVector<Section> mSections;
	int mCurrStartIndex = 0;
	float mLastTimeMs = 0.0f;
};

#endif // QSequence_h__