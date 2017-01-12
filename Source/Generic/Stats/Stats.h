// ===================================================================
//	Copyright (C) 2013 Tim Leonard
// ===================================================================
#ifndef _GENERIC_STATS_
#define _GENERIC_STATS_

#include "Generic/Types/CircleBuffer.h"
#include "Generic/Helper/StringHelper.h"
#include "Generic/Memory/Memory.h"
#include "Engine/Platform/Platform.h"
#include <vector>

#define DEFINE_GLOBAL_STATISTIC(path, type, var_name, display)						Global_Stat<type> var_name(path, display);
#define DEFINE_FRAME_STATISTIC(path, type, var_name, display)						Frame_Stat<type> var_name(path, display);
#define DEFINE_RANGE_STATISTIC(path, type, var_name, display)						Range_Stat<type> var_name(path, display);
#define DEFINE_AVERAGE_STATISTIC(path, type, value_count, var_name, display)		Average_Stat<type, value_count> var_name(path, display);
#define DEFINE_TIME_AVERAGE_STATISTIC(path, type, time, var_name, display)			Time_Average_Stat<type,30> var_name(path, display, time);

#define STATISTICS_TIMED_SCOPE(stat) StatTimedScope _s_##__LINE__##_stat_scope(&stat)

class Stat
{
public:
	enum 
	{
		max_path_segments = 8,
		max_path_segment_length = 256,
	};

	char Path[max_path_segments][max_path_segment_length];
	int Path_Count;

	bool Display;

public:
	Stat(const char* path, bool display);

	virtual std::string To_String() = 0;
	virtual float To_Float() = 0;
	virtual void New_Frame();
};

template <typename T>
struct Stat_Formatter
{
	static std::string Do(T)
	{
		return "<unknown-data-type>";
	}
};

template <>
struct Stat_Formatter<int>
{
	static std::string Do(int val)
	{
		return StringHelper::Format("%i", val);
	}
};

template <>
struct Stat_Formatter<float>
{
	static std::string Do(float val)
	{
		return StringHelper::Format("%.2f", val);
	}
};

template <>
struct Stat_Formatter<std::string>
{
	static std::string Do(std::string val)
	{
		return val;
	}
};

template <typename T>
class Range_Stat : public Stat
{
private:
	T m_value;
	T m_range;

public:
	Range_Stat(const char* path, bool display)
		: Stat(path, display)
		, m_value(T())
		, m_range(T())
	{
	}

	virtual void Set(T val, T max)
	{
		m_value = val;
		m_range = max;
	}

	virtual std::string To_String()
	{
		return StringHelper::Format(
			"%s / %s", 
			Stat_Formatter<T>::Do(m_value).c_str(), 
			Stat_Formatter<T>::Do(m_range).c_str());
	}

	virtual float To_Float()
	{
		return m_value;
	}
};

template <typename T>
class Frame_Stat : public Stat
{
private:
	T m_value;

public:
	Frame_Stat(const char* path, bool display)
		: Stat(path, display)
		, m_value(T())
	{
	}

	virtual void Set(T val)
	{
		m_value += val;
	}

	virtual std::string To_String()
	{
		return Stat_Formatter<T>::Do(m_value);
	}

	virtual void New_Frame()
	{
		m_value = T();
	}

	virtual float To_Float()
	{
		return (float)m_value;
	}
};

template <typename T>
class Global_Stat : public Stat
{
private:
	T m_value;

public:
	Global_Stat(const char* path, bool display)
		: Stat(path, display)
		, m_value(T())
	{
	}

	virtual void Set(T val)
	{
		m_value = val;
	}

	virtual std::string To_String()
	{
		return Stat_Formatter<T>::Do(m_value);
	}

	virtual float To_Float()
	{
		return (float)m_value;
	}
};

template <typename T, int capacity>
class Average_Stat : public Stat
{
private:
	CircleBuffer<T, capacity> m_values;

public:
	Average_Stat(const char* path, bool display)
		: Stat(path, display)
	{
	}

	virtual void Set(T val)
	{
		m_values.Push(val);
	}

	virtual std::string To_String()
	{
		if (m_values.Size() == 0)
		{
			return "";
		}

		T total = 0;
		for (int i = 0; i < m_values.Size(); i++)
		{
			total += m_values.Get(i);
		}
		T avg = total / m_values.Size();

		return Stat_Formatter<T>::Do(avg);
	}

	virtual float To_Float()
	{
		T total = 0;
		for (int i = 0; i < m_values.Size(); i++)
		{
			total += m_values.Get(i);
		}
		T avg = total / m_values.Size();

		return (float)avg;
	}
};

template <typename T, int capacity>
class Time_Average_Stat : public Stat
{
private:
	CircleBuffer<T, capacity> m_values;
	float m_interval;
	float m_total_value;
	double m_time;

public:
	Time_Average_Stat(const char* path, bool display, float time)
		: Stat(path, display)
	{
		m_time = 0.0f;
		m_interval = time;
	}

	virtual void Set(T val)
	{
		m_total_value += val;
	}

	virtual void New_Frame()
	{
		double ticks = platform_get_ticks();
		double elapsed = ticks - m_time;

		if (elapsed >= m_interval)
		{
			m_values.Push(m_total_value);
			m_time = ticks;
			m_total_value = 0.0f;
		}
	}

	virtual std::string To_String()
	{
		if (m_values.Size() == 0)
		{
			return "";
		}

		T total = 0;
		for (int i = 0; i < m_values.Size(); i++)
		{
			total += m_values.Get(i);
		}
		T avg = total / m_values.Size();

		return Stat_Formatter<T>::Do(avg);
	}

	virtual float To_Float()
	{
		T total = 0;
		for (int i = 0; i < m_values.Size(); i++)
		{
			total += m_values.Get(i);
		}
		T avg = total / m_values.Size();

		return (float)avg;
	}
};

struct StatTimedScope
{
private:
	Frame_Stat<float>* m_stat;
	double m_start_time;

public:

	StatTimedScope(Frame_Stat<float>* stat)
		: m_stat(stat)
	{
		m_start_time = Platform::Get()->Get_Ticks();
	}

	~StatTimedScope()
	{
		float elapsed = (float)(Platform::Get()->Get_Ticks() - m_start_time);
		m_stat->Set(elapsed);
	}
};

class StatManager
{
private:
	enum
	{
		max_statistics = 1024
	};

	static Stat* s_statistics[max_statistics];
	static int s_statistic_count;

	~StatManager();

public:
	static void Register(Stat* stat);
	static int Get_Statistics(Stat**& output);
	static void New_Frame();

};


#endif
