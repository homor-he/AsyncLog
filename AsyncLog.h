#pragma once

#include <atomic>
#include <string>
#include "FixedBuffer.h"
#include <memory>
#include <vector>

using namespace std;

class AsyncLog
{
public:
	AsyncLog(const string & name, uint resetSize, int flushInterval = 3);
	~AsyncLog();

	void Append(const char* cont, int len);
	void Start();
	void Stop();
private:
	void ThreadFunc();

private:
	typedef FixBuffer<BIGBUFFSIZE> Buffer;

	const string m_name;
	const uint m_resetSize;
	const int m_flushInterval;
	atomic<bool> m_running;
	
	Thread m_thread;
	Mutex m_lock;
	Event m_cond;
	unique_ptr<Buffer> m_curBuffer;
	unique_ptr<Buffer> m_nextBuffer;
	vector<unique_ptr<Buffer>> m_bufferList;
};