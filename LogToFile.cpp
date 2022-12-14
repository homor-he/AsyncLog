#include "LogToFile.h"

FileIOHandler::FileIOHandler(string fileName):
	m_fp(fopen(fileName.c_str(),"ae")),
	m_writtenBytes(0)
{
	assert(m_fp);
	setbuffer(m_fp, m_Buff,sizeof(m_Buff));
}

FileIOHandler::~FileIOHandler()
{
	fclose(m_fp);
}

void FileIOHandler::Append(const char * cont, size_t len)
{
	size_t written = 0;

	while (written != len)
	{
		size_t remain = len - written;
		size_t n = Write(cont + written, remain);
		if (n != remain)
		{
			int err = ferror(m_fp);
			fprintf(stderr, "%s:%d, append error %d, %s", err, strerror_tl(err));
			break;
		}
		written += n;
	}
	m_writtenBytes += written;
}

void FileIOHandler::Flush()
{
	fflush(m_fp);
}

uint FileIOHandler::GetWrittenBytes()
{
	return m_writtenBytes;
}

size_t FileIOHandler::Write(const char * cont, size_t len)
{
	fwrite_unlocked(cont, 1, len, m_fp);
}

///////////////////////////////////////////////////

LogToFile::LogToFile(const string & name, uint resetSize, int flushInterval, int checkFrequency):
	m_name(name),
	m_resetSize(m_resetSize),
	m_flushInterval(flushInterval),
	m_checkFrequency(checkFrequency),
	m_lastUpdate(0),
	m_lastFlush(0),
	m_appendCnt(0)
{
}

LogToFile::~LogToFile()
{
}

void LogToFile::Append(const char * cont, int len)
{
	{
		Autolock lock(m_mutex);
		Append_unlocked(cont, len);
	}
}

void LogToFile::Flush()
{
	{
		Autolock lock(m_mutex);
		m_fileIOHandler->Flush();
	}
	
}

bool LogToFile::UpdateFile()
{
	time_t now = 0;
	string fileName = GetLogFileName(m_name, &now);
	time_t start = now / s_updateInterval * s_updateInterval;
	//判断now是否和lastupdate相差1天或以上 
	if (now > m_lastUpdate)
	{
		m_lastUpdate = now;
		m_lastFlush = now;
		m_lastPeriod = start;
		m_fileIOHandler.reset(new FileIOHandler(fileName));
		return true;
	}
	return false;
}

void LogToFile::Append_unlocked(const char * data, int len)
{
	m_fileIOHandler->Append(data, len);

	//超过预定内存写一次 适用于频率低，单条日志数据量大的情况
	if (m_fileIOHandler->GetWrittenBytes() > m_resetSize)
	{
		updateFile();
	}
	else
	{
		//超过预定频次写一次，适用于频率高，单条日志数据量小的情况
		m_appendCnt++;
		if (m_appendCnt > m_checkFrequency)
		{
			m_appendCnt = 0;
			time_t now = time(NULL);
			time_t thisPeriod = now / s_updateInterval*s_updateInterval;
			//非同一天
			if (thisPeriod != m_lastPeriod)
			{
				updateFile();
			}
			else if (now - m_lastFlush > m_checkFrequency)
			{
				m_lastFlush = now;
				m_fileIOHandler->Flush();
			}
		}
	}
}

string LogToFile::GetLogFileName(const string & name, time_t * updateTime)
{
	struct tm tmTime;
	*updateTime = time(NULL);
	localtime_s(&tmTime, updateTime);

	char fileName[256];
	sprintf_s(fileName,"%d%02d%02d-%s-%d",tmTime.tm_year+1900, tmTime.tm_mon+1,tmTime.tm_mday, name.c_str(),getpid());

	return string(fileName);
}


