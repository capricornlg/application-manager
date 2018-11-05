
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
#if	!defined(WIN32)
#include <dirent.h>
#include <sys/stat.h>
#include <errno.h>
#include <pwd.h>
#endif
#ifdef _WIN32
#include <process.h>
#else
#include <unistd.h>
#endif
#include <thread>
#include <iomanip>
#include <boost/asio/ip/host_name.hpp>
#include <boost/archive/iterators/binary_from_base64.hpp>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/algorithm/string.hpp>

#include <boost/log/support/date_time.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <boost/log/sinks/text_ostream_backend.hpp>
#include <boost/log/sinks/text_file_backend.hpp>
#include <boost/log/utility/setup/file.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/sources/severity_logger.hpp>
#include <boost/log/sources/record_ostream.hpp>
#include <boost/core/null_deleter.hpp>
namespace logging = boost::log;
namespace src = boost::log::sources;
namespace sinks = boost::log::sinks;
namespace expr = boost::log::expressions;
namespace keywords = boost::log::keywords;


using namespace std;

Utility::Utility()
{
}


Utility::~Utility()
{
}

std::map<std::string, int> Utility::getProcessList()
{
	const static char fname[] = "Utility::getProcessList() ";

	std::map<std::string, int> processList;
#if	!defined(WIN32)
	DIR* dir = opendir("/proc/");
	if (dir != NULL)
	{
		struct stat fstat;
		struct dirent* rent = NULL;
		while ((rent = readdir(dir)))
		{
			try
			{
				if (rent->d_type == DT_DIR && isNumber(rent->d_name))
				{
					string path = "/proc/";
					path += rent->d_name;
					path += "/cmdline";

					if (stat(path.c_str(), &fstat)) continue;
					if (S_ISDIR(fstat.st_mode)) continue;

					ifstream file(path.c_str());
					if (file.is_open() && !file.eof() && !file.fail() && !file.bad())
					{
						char maxCmdStrBuf[1024];
						memset(maxCmdStrBuf, 0, sizeof(maxCmdStrBuf));
						size_t getSize = file.get(maxCmdStrBuf, 1023, '\r').gcount();
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
							processList[str] = atoi(rent->d_name);
						}
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
		}
		closedir(dir);
	}
#endif

	//for_each(processList.begin(), processList.end(), [](std::map<std::string, int>::reference p) {cout << "Process:[" << p.second << "]" << p.first << endl; });

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
	#define MAXBUFSIZE 1024
	int count = 0;
	char buf[MAXBUFSIZE] = { 0 };
#if	!defined(WIN32)
	count = (int)readlink("/proc/self/exe", buf, MAXBUFSIZE);
#endif
	if (count < 0 || count >= MAXBUFSIZE)
	{
		printf("Failed\n");
		return buf;
	}
	else
	{
		buf[count] = '\0';
		return buf;
	}
}

void Utility::initLogging()
{
	// 1. Initialize terminal logger
	// Construct the sink
	typedef boost::log::sinks::synchronous_sink<boost::log::sinks::text_ostream_backend> text_sink;
	boost::shared_ptr<text_sink> console_sink = boost::make_shared<text_sink>();
	boost::shared_ptr<std::ostream> console_stream(&std::clog, boost::null_deleter());
	console_sink->locked_backend()->add_stream(console_stream);
	console_sink->set_formatter(
		expr::format("[%1%][%2%]: %3%")
		% expr::attr<boost::posix_time::ptime>("TimeStamp")
		% expr::attr<boost::log::attributes::current_thread_id::value_type>("ThreadID")
		% expr::smessage
	);
	// flush
	console_sink->locked_backend()->auto_flush(true);
	// register sink
	logging::core::get()->add_sink(console_sink);
	

	// 2. Initialize file logger
	char* bufferPath = get_current_dir_name();
	auto file_sink = logging::add_file_log
	(
		keywords::auto_flush = true,
		keywords::file_name = string(bufferPath) + "/log/" + "appsvc_%N.log",          /*< file name pattern >*/
		keywords::rotation_size = 100 * 1024 * 1024,                                   /*< rotate files every 100 MiB... >*/
		keywords::time_based_rotation = sinks::file::rotation_at_time_point(0, 0, 0),  /*< ...or at midnight >*/
		keywords::format = "[%TimeStamp%][%ThreadID%]: %Message%"                      /*< log record format >*/
	);
	logging::core::get()->set_filter(logging::trivial::severity >= logging::trivial::info);
	logging::add_common_attributes();

	logging::core::get()->add_sink(file_sink);

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
