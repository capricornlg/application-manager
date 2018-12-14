#include <iostream>
#include <mutex>
#include <algorithm>
#include <ace/Time_Value.h>
#include <ace/OS.h>
#include "Application.h"
#include "../common/Utility.h"
#include "../common/TimeZoneHelper.h"
#include "Process.h"
using namespace std;


Application::Application()
	:m_active(NORMAL), m_return(0),m_pid(-1)
{
	const static char fname[] = "Application::Application() ";
	LOG_DBG << fname << "Entered.";
	m_process = std::make_shared<Process>();
}


Application::~Application()
{
	const static char fname[] = "Application::~Application() ";
	LOG_DBG << fname << "Entered.";
}

std::string Application::getName()
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	return m_name;
}

bool Application::isNormal()
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	return (m_active == NORMAL);
}

void Application::FromJson(std::shared_ptr<Application>& app, const web::json::object& jobj)
{
	app->m_name = Utility::stdStringTrim(GET_JSON_STR_VALUE(jobj, "name"));
	app->m_user = Utility::stdStringTrim(GET_JSON_STR_VALUE(jobj, "run_as"));
	app->m_commandLine = Utility::stdStringTrim(GET_JSON_STR_VALUE(jobj, "command_line"));
	if (app->m_commandLine.find('>') != std::string::npos)
	{
		throw std::invalid_argument("char '>' is not supported for command line");
	}
	app->m_workdir = Utility::stdStringTrim(GET_JSON_STR_VALUE(jobj, "working_dir"));
	if (HAS_JSON_FIELD(jobj, "active"))
	{
		app->m_active = static_cast<STATUS>GET_JSON_INT_VALUE(jobj, "active");
	}
	if (HAS_JSON_FIELD(jobj, "daily_limitation"))
	{
		app->m_dailyLimit = DailyLimitation::FromJson(jobj.at(GET_STRING_T("daily_limitation")).as_object());
	}
	if (HAS_JSON_FIELD(jobj, "env"))
	{
		auto env = jobj.at(GET_STRING_T("env")).as_object();
		for (auto it = env.begin(); it != env.end(); it++)
		{
			app->m_envMap[GET_STD_STRING((*it).first)] = GET_STD_STRING((*it).second.as_string());
		}
	}
	app->m_posixTimeZone = GET_JSON_STR_VALUE(jobj, "posix_timezone");
	if (app->m_posixTimeZone.length() && app->m_dailyLimit != nullptr)
	{
		app->m_dailyLimit->m_startTime = TimeZoneHelper::convert2tzTime(app->m_dailyLimit->m_startTime, app->m_posixTimeZone);
		app->m_dailyLimit->m_endTime = TimeZoneHelper::convert2tzTime(app->m_dailyLimit->m_endTime, app->m_posixTimeZone);
	}
}

void Application::refreshPid()
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	if (m_process!= nullptr)
	{
		if (m_process->running())
		{
			m_pid = m_process->getpid();
			ACE_Time_Value tv;
			tv.msec(5);
			int ret = m_process->wait(tv);
			if (ret > 0)
			{
				m_return = m_process->return_value();
			}
		}
		else if (m_pid > 0)
		{
			m_pid = -1;
		}		
	}
}

void Application::attach(std::map<std::string, int>& process)
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	auto iter = process.find(m_commandLine);
	if (iter != process.end())
	{
		m_process->attach(iter->second);
		m_pid = m_process->getpid();
		LOG_INF << "Process <" << m_commandLine << "> is running with pid <" << m_pid << ">.";
		process.erase(iter);
	}
}

void Application::invoke()
{
	const static char fname[] = "Application::invoke() ";
	{
		std::lock_guard<std::recursive_mutex> guard(m_mutex);
		if (this->isNormal())
		{
			if (this->isInDailyTimeRange())
			{
				if (!m_process->running())
				{
					LOG_INF << fname << "Starting application <" << m_name << ">.";
					this->spawnProcess();
				}
			}
			else
			{
				if (m_process->running())
				{
					LOG_INF << fname << "Application <" << m_name << "> was not in daily start time";
					m_process->killgroup();
				}
			}
		}
	}
	refreshPid();
}

void Application::invokeNow(std::shared_ptr<Application>& self)
{
	Application::invoke();
}

void Application::stop()
{
	const static char fname[] = "Application::stop() ";

	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	if (m_active != STOPPED)
	{
		if (m_process != nullptr) m_process->killgroup();
		m_active = STOPPED;
		m_return = -1;
		LOG_INF << fname << "Application <" << m_name << "> stopped.";
	}
}

void Application::start(std::shared_ptr<Application>& self)
{
	const static char fname[] = "Application::start() ";

	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	if (m_active == STOPPED)
	{
		m_active = NORMAL;
		invokeNow(self);
		LOG_INF << fname << "Application <" << m_name << "> started.";
	}
}

web::json::value Application::AsJson(bool returnRuntimeInfo)
{
	web::json::value result = web::json::value::object();

	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	result[GET_STRING_T("name")] = web::json::value::string(GET_STRING_T(m_name));
	result[GET_STRING_T("run_as")] = web::json::value::string(GET_STRING_T(m_user));
	result[GET_STRING_T("command_line")] = web::json::value::string(GET_STRING_T(m_commandLine));
	result[GET_STRING_T("working_dir")] = web::json::value::string(GET_STRING_T(m_workdir));
	result[GET_STRING_T("active")] = web::json::value::number(m_active);
	if (returnRuntimeInfo)
	{
		result[GET_STRING_T("pid")] = web::json::value::number(m_pid);
		result[GET_STRING_T("return")] = web::json::value::number(m_return);
	}
	if (m_dailyLimit != nullptr)
	{
		result[GET_STRING_T("daily_limitation")] = m_dailyLimit->AsJson();
	}
	if (m_envMap.size())
	{
		web::json::value envs = web::json::value::object();
		std::for_each(m_envMap.begin(), m_envMap.end(), [&envs](const std::pair<std::string, string>& pair)
		{
			envs[GET_STRING_T(pair.first)] = web::json::value::string(GET_STRING_T(pair.second));
		});
		result[GET_STRING_T("env")] = envs;
	}
	result[GET_STRING_T("posix_timezone")] = web::json::value::string(m_posixTimeZone);
	return result;
}

void Application::dump()
{
	const static char fname[] = "Application::dump() ";

	std::lock_guard<std::recursive_mutex> guard(m_mutex);

	LOG_DBG << fname << "m_name:" << m_name;
	LOG_DBG << fname << "m_commandLine:" << m_commandLine;
	LOG_DBG << fname << "m_workdir:" << m_workdir;
	LOG_DBG << fname << "m_user:" << m_user;
	LOG_DBG << fname << "m_status:" << m_active;
	LOG_DBG << fname << "m_pid:" << m_pid;
	LOG_DBG << fname << "m_posixTimeZone:" << m_posixTimeZone;
	if (m_dailyLimit != nullptr)
	{
		m_dailyLimit->dump();
	}
}

void Application::spawnProcess()
{
	const static char fname[] = "Application::spawnProcess() ";

	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	long gid, uid;
	Utility::getUid(m_user, uid, gid);
	size_t cmdLenth = m_commandLine.length() + ACE_Process_Options::DEFAULT_COMMAND_LINE_BUF_LEN;
	int totalEnvSize = 0;
	int totalEnvArgs = 0;
	Utility::getEnvironmentSize(m_envMap, totalEnvSize, totalEnvArgs);
	ACE_Process_Options option(1, cmdLenth, totalEnvSize, totalEnvArgs);
	option.command_line(m_commandLine.c_str());
	//option.avoid_zombies(1);
	option.seteuid(uid);
	option.setruid(uid);
	option.setegid(gid);
	option.setrgid(gid);
	option.setgroup(0);
	option.inherit_environment(true);
	option.handle_inheritance(0);
	option.working_directory(m_workdir.c_str());
	std::for_each(m_envMap.begin(), m_envMap.end(), [&option](const std::pair<std::string, string>& pair)
	{
		option.setenv(pair.first.c_str(), "%s", pair.second.c_str());
	});
	if (m_process->spawn(option) >= 0)
	{
		m_pid = m_process->getpid();
		LOG_INF << fname << "Process <" << m_commandLine << "> started with pid <" << m_pid << ">.";
	}
	else
	{
		m_pid = -1;
		LOG_ERR << fname << "Process:<" << m_commandLine << "> start failed with error : " << std::strerror(errno);
	}
}

bool Application::isInDailyTimeRange()
{
	if (m_dailyLimit != nullptr)
	{
		auto now = Utility::convertStr2DayTime(Utility::convertDayTime2Str(std::chrono::system_clock::now()));

		if (m_dailyLimit->m_startTime < m_dailyLimit->m_endTime)
		{
			return (now >= m_dailyLimit->m_startTime && now < m_dailyLimit->m_endTime);
		}
		else if (m_dailyLimit->m_startTime > m_dailyLimit->m_endTime)
		{
			return !(now >= m_dailyLimit->m_endTime && now < m_dailyLimit->m_startTime);
		}
	}
	return true;
}

