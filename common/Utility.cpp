
#include "../common/Utility.h"
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <stdio.h>
#include <algorithm>
#include <cctype>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifdef _WIN32
#include <process.h>
#include <Windows.h>
#else
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h>
#include <unistd.h>
#include <limits.h>
#endif
#include <thread>
#include <iomanip>
#include <boost/asio/ip/host_name.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/RollingFileAppender.hh>
#include <log4cpp/OstreamAppender.hh>
using namespace log4cpp;

using namespace std;

Utility::Utility()
{
}


Utility::~Utility()
{
}

#if	!defined(WIN32)
using file_filter_type = std::function<bool(const char*)>;

static  std::vector<std::string> for_each_file(const std::string&dirName, file_filter_type filter)
{
	std::vector<std::string> fileList;
	auto dir = opendir(dirName.data());
	struct dirent *rent = NULL;
	if (dir)
	{
		while ((rent = readdir(dir)) != NULL)
		{
			if (rent->d_name == nullptr || strlen(rent->d_name) == 0 || 
				0 == strcmp(rent->d_name, "..") || 0 == strcmp(rent->d_name, "."))
			{
				continue;
			}
			auto path = std::string(dirName).append("/").append(rent->d_name);
			if (filter(rent->d_name))
			{
				//LOG_INF << "Process:" << rent->d_name;
				fileList.emplace_back(rent->d_name);
			}
		}
		closedir(dir);
	}
	return std::move(fileList);
}
#endif

std::map<std::string, int> Utility::getProcessList()
{
	const static char fname[] = "Utility::getProcessList() ";

	std::map<std::string, int> processList;

#if	!defined(WIN32)
	auto procDir = "/proc";
	auto filterFunc = [](const char* name) 
	{ 
		return isNumber(name) && access(std::string("/proc/").append(name).append("/cmdline").c_str(), F_OK) == 0;
	};
	auto fileList = for_each_file(procDir, filterFunc);
	std::for_each(fileList.begin(), fileList.end(), [&](std::vector<std::string>::reference p)
	{
		try
		{
			// /proc/123/cmdline
			ifstream file(std::string(procDir).append("/").append(p).append("/cmdline"));
			if (file.is_open() && !file.eof() && !file.fail() && !file.bad())
			{
				char maxCmdStrBuf[PATH_MAX+1];
				memset(maxCmdStrBuf, 0, sizeof(maxCmdStrBuf));
				size_t getSize = file.get(maxCmdStrBuf, PATH_MAX, '\r').gcount();
				file.close();
				if (getSize > 0 && !file.fail() && !file.bad())
				{
					// Parse cmdline : https://stackoverflow.com/questions/1585989/how-to-parse-proc-pid-cmdline
					for (size_t i = 0; i < getSize; i++)
					{
						if (maxCmdStrBuf[i] == '\0') maxCmdStrBuf[i] = ' ';
						if (maxCmdStrBuf[i] == '\r') maxCmdStrBuf[i] = '\0';
					}
					std::string str = maxCmdStrBuf;
					str = stdStringTrim(str);
					processList[str] = atoi(p.c_str());
				}
			}
		}
		catch (const std::exception& e)
		{
			LOG_ERR << fname << "ERROR:" << e.what();
		}
		catch (...)
		{
			LOG_ERR << fname << "ERROR:" << "unknown exception";
		}
	});
#endif

	for_each(processList.begin(), processList.end(), [&](std::map<std::string, int>::reference p) {LOG_INF << "Process:[" << p.second << "]" << p.first; });

	return processList;
}

bool Utility::isNumber(string s)
{
	return !s.empty() && std::find_if(s.begin(), s.end(), [](char c) { return !std::isdigit(c); }) == s.end();
}

std::string Utility::stdStringTrim(const std::string & str)
{
	char *line = const_cast <char *> (str.c_str());
	// trim the line on the left and on the right
	size_t len = str.length();
	size_t start = 0;
	while (isspace(*line))
	{
		++line;
		--len;
		++start;
	}
	while (len > 0 && isspace(line[len - 1]))
	{
		--len;
	}
	return len >= start ? str.substr(start, len) : str.substr(start);
}

std::string Utility::getSelfFullPath()
{
	const static char fname[] = "Utility::getSelfFullPath() ";
#if	defined(WIN32)
	char buf[MAX_PATH] = { 0 };
	::GetModuleFileNameA(NULL, buf, MAX_PATH);
	// Remove ".exe"
	size_t idx = 0;
	while (buf[idx] != '\0')
	{
		if (buf[idx] == '.' && buff[idx + 1] == 'e' && buff[idx + 2] == 'x' && buff[idx + 3] == 'e';
		{
			buf[idx] = '\0';
		}
	}
#else
	#define MAX_PATH PATH_MAX
	char buf[MAX_PATH] = { 0 };
	int count = (int)readlink("/proc/self/exe", buf, MAX_PATH);
	if (count < 0 || count >= MAX_PATH)
	{
		LOG_ERR << fname << "ERROR:" << "unknown exception";
	}
	else
	{
		buf[count] = '\0';
	}
#endif
	return buf;
}

void Utility::initLogging()
{
	auto consoleLayout = new PatternLayout();
	consoleLayout->setConversionPattern("%d: %p %c %x: %m%n");
	auto consoleAppender = new OstreamAppender("console", &std::cout);
	consoleAppender->setLayout(consoleLayout);

	//RollingFileAppender(const std::string&name, const std::string&fileName,
	//	size_tmaxFileSize = 10 * 1024 * 1024, unsigned intmaxBackupIndex = 1,
	//	boolappend = true, mode_t mode = 00644);
	auto rollingFileAppender = new RollingFileAppender(
		"rollingFileAppender",
		"log/appsvc.log",
		10 * 1024 * 1024,
		10);
	
	auto pLayout = new PatternLayout();
	pLayout->setConversionPattern("%d: %p %c %x: %m%n");
	rollingFileAppender->setLayout(pLayout);

	Category & root = Category::getRoot();
	root.addAppender(rollingFileAppender);
	root.addAppender(consoleAppender);
	root.setPriority(Priority::DEBUG);

	LOG_INF << "Process:" << getpid();
}

unsigned long long Utility::getThreadId()
{
	std::ostringstream oss;
	oss << std::this_thread::get_id();
	std::string stid = oss.str();
	return std::stoull(stid);
}

std::chrono::system_clock::time_point Utility::convertStr2Time(const std::string & strTime)
{
	struct tm tm = { 0 };
	std::istringstream ss(strTime);
	// ss.imbue(std::locale("de_DE.utf-8"));
	ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");
	if (ss.fail())
	{
		string msg = "error when convert string to time :";
		msg += strTime;
		throw std::invalid_argument(msg);
	}
	return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::string Utility::convertTime2Str(const std::chrono::system_clock::time_point & time)
{
	// https://en.cppreference.com/w/cpp/io/manip/put_time
	auto timet = std::chrono::system_clock::to_time_t(time);
	std::tm tm = *std::localtime(&timet);
	std::stringstream ss;
	ss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
	return ss.str();
}

std::chrono::system_clock::time_point Utility::convertStr2DayTime(const std::string & strTime)
{
	struct tm tm = { 0 };
	std::istringstream ss(strTime);
	// ss.imbue(std::locale("de_DE.utf-8"));
	ss >> std::get_time(&tm, "%H:%M:%S");
	if (ss.fail())
	{
		string msg = "error when convert string to time :";
		msg += strTime;
		throw std::invalid_argument(msg);
	}
	// Give a fixed date.
	tm.tm_year = 2000 - 1900;
	tm.tm_mon = 1;
	tm.tm_mday = 17;
	return std::chrono::system_clock::from_time_t(std::mktime(&tm));
}

std::string Utility::convertDayTime2Str(const std::chrono::system_clock::time_point & time)
{
	// https://en.cppreference.com/w/cpp/io/manip/put_time
	auto timet = std::chrono::system_clock::to_time_t(time);
	std::tm tm = *std::localtime(&timet);
	std::stringstream ss;
	ss << std::put_time(&tm, "%H:%M:%S");
	return ss.str();
}

std::string Utility::getSystemPosixTimeZone()
{
	// https://stackoverflow.com/questions/2136970/how-to-get-the-current-time-zone/28259774#28259774
	struct tm local_tm;
	time_t cur_time = 0; // time(NULL);
	localtime_r(&cur_time, &local_tm);

	std::stringstream ss;
	ss << std::put_time(&local_tm, "%Z%z");
	auto str = ss.str();

	// remove un-used zero post-fix : 
	// CST+0800  => CST+08
	auto len = str.length();
	for (size_t i = len - 1; i > 0; i--)
	{
		if (str[i] == '0')
		{
			str[i] = '\0';
		}
		else
		{
			str = str.c_str();
			break;
		}
	}
	return str;
}

std::string Utility::encode64(const std::string & val)
{
	using namespace boost::archive::iterators;
	using It = base64_from_binary<transform_width<std::string::const_iterator, 6, 8>>;
	auto tmp = std::string(It(std::begin(val)), It(std::end(val)));
	return tmp.append((3 - val.size() % 3) % 3, '=');
}

std::string Utility::decode64(const std::string & val)
{
	using namespace boost::archive::iterators;
	using It = transform_width<binary_from_base64<std::string::const_iterator>, 8, 6>;
	return boost::algorithm::trim_right_copy_if(std::string(It(std::begin(val)), It(std::end(val))), [](char c) {
		return c == '\0';
	});
}

void Utility::splitString(const std::string & source, std::vector<std::string>& result, const std::string & splitFlag)
{
	std::string::size_type pos1, pos2;
	pos2 = source.find(splitFlag);
	pos1 = 0;
	while (std::string::npos != pos2)
	{
		string str = stdStringTrim(source.substr(pos1, pos2 - pos1));
		if (str.length() > 0) result.push_back(str);

		pos1 = pos2 + splitFlag.size();
		pos2 = source.find(splitFlag, pos1);
	}
	if (pos1 != source.length())
	{
		string str = stdStringTrim(source.substr(pos1));
		if (str.length() > 0) result.push_back(str);
	}
}

bool Utility::startWith(const std::string & str, std::string head)
{
	if (str.length() >= head.length())
	{
		return (str.compare(0, head.size(), head) == 0);
	}
	return false;
}

bool Utility::getUid(std::string userName, long& uid, long& groupid)
{
	bool rt = false;
	struct passwd pwd;
	struct passwd *result = NULL;
	static auto bufsize = sysconf(_SC_GETPW_R_SIZE_MAX);
	if (bufsize == -1) bufsize = 16384;
	std::shared_ptr<char> buff(new char[bufsize], std::default_delete<char[]>());
	getpwnam_r(userName.c_str(), &pwd, buff.get(), bufsize, &result);
	if (result)
	{
		uid = pwd.pw_uid;
		groupid = pwd.pw_gid;
		rt = true;
	}
	else
	{
		LOG_ERR << "User does not exist: <" << userName << ">.";
	}
	return rt;
}

void Utility::getEnvironmentSize(const std::map<std::string, std::string>& envMap, int & totalEnvSize, int & totalEnvArgs)
{
	// get env size
	if (!envMap.empty())
	{
		auto it = envMap.begin();
		while (it != envMap.end())
		{
			totalEnvSize += (int)(it->first.length() + it->second.length() + 2); // add for = and terminator
			totalEnvArgs++;
			it++;
		}
	}

	// initialize our environment size estimates
	const int numEntriesConst = 256;
	const int bufferSizeConst = 4 * 1024;

	totalEnvArgs += numEntriesConst;
	totalEnvSize += bufferSizeConst;
}
