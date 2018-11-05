
#include "ApplicationPeriodRun.h"
#include "Configuration.h"
#include  "../common/Utility.h"

ApplicationPeriodRun::ApplicationPeriodRun()
{
	const static char fname[] = "ApplicationPeriodRun::ApplicationPeriodRun() ";
	LOG_INF << fname << "Entered.";
}


ApplicationPeriodRun::~ApplicationPeriodRun()
{
	const static char fname[] = "ApplicationPeriodRun::~ApplicationPeriodRun() ";
	LOG_INF << fname << "Entered.";
}

void ApplicationPeriodRun::refreshPid()
{
	ApplicationShortRun::refreshPid();

	auto app = Configuration::instance()->getApp(this->getName());
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	if (!m_process->running() && this->isNormal() && std::chrono::system_clock::now() > getStartTime())
	{
		this->invokeNow(app);
	}
}

void ApplicationPeriodRun::FromJson(std::shared_ptr<ApplicationPeriodRun>& app, const web::json::object & jobj)
{
	std::shared_ptr<ApplicationShortRun> fatherApp = app;
	ApplicationShortRun::FromJson(fatherApp, jobj);
}

web::json::value ApplicationPeriodRun::AsJson(bool returnRuntimeInfo)
{
	const static char fname[] = "ApplicationPeriodRun::AsJson() ";
	LOG_INF << fname;

	web::json::value result = ApplicationShortRun::AsJson(returnRuntimeInfo);
	result[GET_STRING_T("keep_running")] = web::json::value::boolean(true);
	return result;
}

void ApplicationPeriodRun::dump()
{
	const static char fname[] = "ApplicationPeriodRun::dump() ";

	ApplicationShortRun::dump();
	LOG_INF << fname << "keep_running:" << "true";
}


