#include "AsyncLog.h"
#include "LogToFile.h"

AsyncLog::AsyncLog(const string & name, uint resetSize, int flushInterval):
	m_name(name),
	m_resetSize(resetSize),
	m_flushInterval(flushInterval),
	m_running(false),
	m_cond(true,false),
	m_curBuffer(new Buffer),
	m_nextBuffer(new Buffer),
	m_bufferList(),
	m_thread(ThreadFunc)
{
	m_curBuffer->ClearData();
	m_nextBuffer->ClearData();
}

AsyncLog::~AsyncLog()
{
	if (m_running)
		Stop();
}

void AsyncLog::Append(const char * cont, int len)
{
	AutoLock lock(m_lock);
	if (m_curBuffer->Avail() > len)
	{
		m_curBuffer->Append(cont, len);
	}
	else
	{
		m_bufferList.push_back(move(m_curBuffer));
		if (m_nextBuffer)
		{
			m_curBuffer = move(m_nextBuffer);
		}
		else
		{
			m_curBuffer.reset(new Buffer);
		}
		m_curBuffer->Append(cont, len);
		m_cond.notify();
	}
}

void AsyncLog::Start()
{
	m_running = true;
	m_thread.start();
}

void AsyncLog::Stop()
{
	m_running = false;
	m_cond.notify();
	m_thread.join();
}

void AsyncLog::ThreadFunc()
{
	LogToFile output(m_name, 5 * 1024);
	unique_ptr<Buffer> newBuff1(new Buffer);
	unique_ptr<Buffer> newBuff2(new Buffer);
	newBuff1->ClearData();
	newBuff2->ClearData();
	vector<unique_ptr<Buffer>> tempBuffList;
	while (m_running)
	{
		{
			AutoLock lock(m_lock);
			if (m_bufferList.empty())
				m_cond.WaitforSeconds(m_flushInterval);
			m_bufferList.push_back(move(m_curBuffer));
			m_curBuffer = move(newBuff1);
			tempBuffList.swap(m_bufferList);
			if (!m_nextBuffer)
				m_nextBuffer = move(newBuff2);
		}


		for (const auto & buffer : m_bufferList)
		{
			output.append(buffer->Data(), buffer->length());
		}

		if (tempBuffList.size() > 2)
		{
			tempBuffList.resize(2);
		}

		if (!newBuff1)
		{
			newBuff1 = move(tempBuffList.back());
			tempBuffList.pop_back();
			newBuff1->Reset();
		}
		
		if (!newBuff2)
		{
			newBuff2 = move(tempBuffList.back());
			tempBuffList.pop_back();
			newBuff2->Reset();
		}
		tempBuffList.clear();
		output.Flush();
	}
	output.Flush();
}
