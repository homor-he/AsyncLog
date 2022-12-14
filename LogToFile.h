#pragma once

#include <time.h>
#include <memory>
#include <stdio.h>

using namespace std;


class FileIOHandler
{
public:
	FileIOHandler(string fileName);
	~FileIOHandler();

	void Append(const char* cont, size_t len);
	void Flush();

	uint GetWrittenBytes();

private:
	size_t Write(const char* cont,size_t len);

	File * m_fp;
	char m_Buff[64 * 1024];
	uint m_writtenBytes;
};

class Mutex;

class LogToFile
{
public:
	LogToFile(const string & name, uint resetSize,int flushInterval=3,int checkFrequency = 1024);
	~LogToFile();

	void Append(const char* cont, int len);
	void Flush();
	bool UpdateFile();

private:
	void Append_unlocked(const char* data,int len);

	static string GetLogFileName(const string & name, time_t*  updateTime);

	const string m_name;
	const uint m_resetSize;
	const int m_flushInterval;
	const int m_checkFrequency;

	Mutex m_mutex;
	unique_ptr<FileIOHandler> m_fileIOHandler;
	time_t m_lastUpdate;
	time_t m_lastFlush;
	time_t m_lastPeriod;
	uint m_appendCnt;

	const static int s_updateInterval = 60 * 60 * 24;
};
