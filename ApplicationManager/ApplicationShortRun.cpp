
#include <mutex>
#include <iomanip>
#include <time.h>
#include <chrono>
#include <ace/Time_Value.h>
#include "ApplicationShortRun.h"
#include "../common/Utility.h"
#include "../common/TimeZoneHelper.h"

ApplicationShortRun::ApplicationShortRun()
	:m_startInterval(0), m_bufferTime(0), m_timer(NULL)
{
	const static char fname[] = "ApplicationShortRun::ApplicationShortRun() ";
	LOG_INF << fname << "Entered.";
}


ApplicationShortRun::~ApplicationShortRun()
{
	const static char fname[] = "ApplicationShortRun::~ApplicationShortRun() ";
	LOG_INF << fname << "Entered.";
	if (m_timer != NULL) m_timer->cancelTimer();
}

void ApplicationShortRun::FromJson(std::shared_ptr<ApplicationShortRun>& app, const web::json::object& jobj)
{
	std::shared_ptr<Application> fatherApp = app;
	Application::FromJson(fatherApp, jobj);
	app->m_startInterval = GET_JSON_INT_VALUE(jobj, "start_interval_seconds");
	auto start_time = GET_JSON_STR_VALUE(jobj, "start_time");
	app->m_startTime = Utility::convertStr2Time(start_time);
	if (HAS_JSON_FIELD(jobj, "start_interval_timeout"))
	{
		app->m_bufferTime = GET_JSON_INT_VALUE(jobj, "start_interval_timeout");
	}
	if (start_time.length() && app->m_posixTimeZone.length())
	{
		app->m_startTime = TimeZoneHelper::convert2tzTime(app->m_startTime, app->m_posixTimeZone);
	}
}


void ApplicationShortRun::refreshPid()
{
	Application::refreshPid();
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	if (nullptr != m_bufferProcess && m_bufferProcess->running())
	{
		ACE_Time_Value tv;
		tv.msec(5);
		int ret = m_bufferProcess->wait(tv);
		if (ret > 0)
		{
			m_return = m_bufferProcess->return_value();
		}
	}
}

void ApplicationShortRun::invoke()
{
	// Only refresh Pid for short running
	refreshPid();
}

void ApplicationShortRun::invokeNow(std::shared_ptr<Application>& self)
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	if (m_bufferTime > 0)
	{
		m_bufferProcess = m_process;
		// This will be deleted automaticlly, so should not use auto_ptr here.
		auto bufferTimer = new TimerActionKill(m_bufferProcess, m_bufferTime);
	}
	else
	{
		terminateProcess(m_process);
	}
	// Spawn new process
	m_process = std::make_shared<Process>();
	if (this->isInDailyTimeRange())
	{
		spawnProcess();
	}
}

web::json::value ApplicationShortRun::AsJson(bool returnRuntimeInfo)
{
	const static char fname[] = "ApplicationShortRun::AsJson() ";
	LOG_INF << fname;
	web::json::value result = Application::AsJson(returnRuntimeInfo);

	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	result[GET_STRING_T("start_time")] = web::json::value::string(GET_STRING_T(Utility::convertTime2Str(m_startTime)));
	result[GET_STRING_T("start_interval_seconds")] = web::json::value::number(m_startInterval);
	result[GET_STRING_T("start_interval_timeout")] = web::json::value::number(m_bufferTime);
	return result;
}

void ApplicationShortRun::start(std::shared_ptr<Application>& self)
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	if (m_active == STOPPED)
	{
		m_active = NORMAL;
		initTimer(std::dynamic_pointer_cast<ApplicationShortRun>(self));
	}
}

void ApplicationShortRun::initTimer(std::shared_ptr<ApplicationShortRun> app)
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	if (m_timer != NULL) m_timer->cancelTimer();
	m_timer = new TimerAction(app);
}

int ApplicationShortRun::getStartInterval()
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	return m_startInterval;
}

std::chrono::system_clock::time_point ApplicationShortRun::getStartTime()
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	return m_startTime;
}

void ApplicationShortRun::dump()
{
	const static char fname[] = "ApplicationShortRun::dump() ";

	Application::dump();
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	LOG_INF << fname << "m_startTime:" << Utility::convertTime2Str(m_startTime);
	LOG_INF << fname << "m_startInterval:" << m_startInterval;
	LOG_INF << fname << "m_bufferTime:" << m_bufferTime;
}
