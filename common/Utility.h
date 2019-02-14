
#ifndef UTILITY_H
#define UTILITY_H

#include <string>
#include <map>
#include <chrono>
#include <cpprest/json.h>

#include <log4cpp/Category.hh>
#include <log4cpp/Appender.hh>
#include <log4cpp/FileAppender.hh>
#include <log4cpp/Priority.hh>
#include <log4cpp/PatternLayout.hh>
#include <log4cpp/RollingFileAppender.hh>

using namespace log4cpp;

#define LOG_TRC    log4cpp::Category::getRoot() << log4cpp::Priority::TRACE
#define LOG_DBG    log4cpp::Category::getRoot() << log4cpp::Priority::DEBUG
#define LOG_INF    log4cpp::Category::getRoot() << log4cpp::Priority::INFO
#define LOG_WAR    log4cpp::Category::getRoot() << log4cpp::Priority::WARN
#define LOG_ERR    log4cpp::Category::getRoot() << log4cpp::Priority::ERROR

#define __MICRO_KEY__(str) #str                // No expand micro
#define __MICRO_VAR__(str) __MICRO_KEY__(str)  // Expand micro

#define PRINT_VERSION() if (argc >= 2 && (std::string("version") == argv[1] || std::string("-v") == argv[1] || std::string("-V") == argv[1])) \
                        { std::cout << "Build: " << __MICRO_VAR__(BUILD_TAG) << std::endl; return 0; }

#define GET_STRING_T(sstr) utility::conversions::to_string_t(std::string(sstr))
#define GET_STD_STRING(sstr)  utility::conversions::to_utf8string(sstr)

// Get attribute from json Object
#define GET_JSON_STR_VALUE(jsonObj, key) Utility::stdStringTrim(GET_STD_STRING(GET_JSON_STR_T_VALUE(jsonObj, key)))
#define GET_JSON_STR_T_VALUE(jsonObj, key) (jsonObj.find(GET_STRING_T(key)) == jsonObj.end() ? GET_STRING_T("") : jsonObj.at(GET_STRING_T(key)).as_string())
#define GET_JSON_INT_VALUE(jsonObj, key) (jsonObj.find(GET_STRING_T(key)) == jsonObj.end() ? 0 : jsonObj.at(GET_STRING_T(key)).as_integer())
#define GET_JSON_BOOL_VALUE(jsonObj, key) (jsonObj.find(GET_STRING_T(key)) == jsonObj.end() ? false : jsonObj.at(GET_STRING_T(key)).as_bool())
#define HAS_JSON_FIELD(jsonObj, key) (jsonObj.find(GET_STRING_T(key)) == jsonObj.end() ? false : true)

#define DEFAULT_REST_LISTEN_PORT 6060

//////////////////////////////////////////////////////////////////////////
// All common functions
//////////////////////////////////////////////////////////////////////////
class Utility
{
public:
	Utility();
	virtual ~Utility();

	// OS related
	static std::map<std::string, int> getProcessList();
	static std::string getSelfFullPath();
	static bool isDirExist(std::string path);
	static bool isFileExist(std::string path);
	static bool createDirectory(const std::string& path, mode_t mode = 0775);
	static bool createRecursiveDirectory(const std::string& path, mode_t mode = 0775);
	static bool removeDir(const std::string& path);

	// String related
	static bool isNumber(std::string s);
	static std::string stdStringTrim(const std::string &str);
	static std::string stdStringTrim(const std::string &str, char trimChar, bool trimStart = true, bool trimEnd = true);
	static std::vector<std::string> splitString(const std::string& s, const std::string& c);
	static bool startWith(const std::string& str, std::string head);
	static std::string stringReplace(const std::string &strBase, const std::string strSrc, const std::string strDst);

	static void initLogging();
	static void setLogLevel(const std::string & level);

	static unsigned long long getThreadId();
	static bool getUid(std::string userName, long& uid, long& groupid);

	static void getEnvironmentSize(const std::map<std::string, std::string> &envMap, int &totalEnvSize, int &totalEnvArgs);

	// %Y-%m-%d %H:%M:%S
	static std::chrono::system_clock::time_point convertStr2Time(const std::string & strTime);
	static std::string convertTime2Str(const std::chrono::system_clock::time_point & time);
	// %H:%M:%S
	static std::chrono::system_clock::time_point convertStr2DayTime(const std::string & strTime);
	static std::string convertDayTime2Str(const std::chrono::system_clock::time_point & time);
	// Timezone
	static std::string getSystemPosixTimeZone();

	// Base64
	static std::string encode64(const std::string &val);
	static std::string decode64(const std::string &val);

	static std::string readFile(const std::string &path);

	static std::string createUUID();
};

#endif