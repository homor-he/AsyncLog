#pragma once

#include "String.h"

const int SMALLBUFFSIZE = 4096;
const int BIGBUFFSIZE = 4096 * 1000;


template<int Size>
class FixBuffer
{
public:
	FixBuffer();
	~FixBuffer();

	void Append(const char* buf, int len);
	const char* Data();
	int length();
	int Avail();
	void Reset();
	void ClearData();
private:
	const char* End();

private:
	char m_data[Size];
	char* m_curPos;
};

template<int Size>
inline FixBuffer<Size>::FixBuffer():
	m_curPos(m_data)
{}

template<int Size>
inline FixBuffer<Size>::~FixBuffer()
{}

template<int Size>
inline void FixBuffer<Size>::Append(const char * buf, int len)
{
	if (len > 0 && (Avail() > len))
	{
		memcpy_s(m_curPos, len, buf, len);
		m_curPos += len;
	}
}

template<int Size>
inline const char * FixBuffer<Size>::Data()
{
	return m_data;
}

template<int Size>
inline int FixBuffer<Size>::length()
{
	return static_cast<int>(m_curPos - m_data);
}

template<int Size>
inline int FixBuffer<Size>::Avail()
{
	return static_cast<int>(end() - m_curPos);
}

template<int Size>
inline void FixBuffer<Size>::Reset()
{
	m_curPos = m_data;
}

template<int Size>
inline void FixBuffer<Size>::ClearData()
{
	memset(m_data, 0, sizeof(m_data));
}

template<int Size>
inline const char * FixBuffer<Size>::End()
{
	return m_data + sizeof(m_data);
}
