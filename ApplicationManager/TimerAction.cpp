#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <chrono>

#include "TimerAction.h"
#include "../common/Utility.h"
#include "Timer.h"
#include "ApplicationShortRun.h"



TimerAction::TimerAction()
{
}

TimerAction::TimerAction(std::shared_ptr<ApplicationShortRun> app)
	:m_app(app)
{
	const static char fname[] = "TimerAction::TimerAction() ";
	LOG_DBG << fname << "Entered.";

	LOG_INF << "Current time <" << Utility::convertTime2Str(std::chrono::system_clock::now())
		<< ">, start time <" << Utility::convertTime2Str(app->getStartTime()) << "> App <" << app->getName() << ">.";

	int firstSleepSec = 0;
	if (app->getStartTime() > std::chrono::system_clock::now())
	{
		firstSleepSec = std::chrono::duration_cast<std::chrono::seconds>(app->getStartTime() - std::chrono::system_clock::now()).count();
	}
	else
	{
		auto totalSec = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - app->getStartTime()).count();
		firstSleepSec = totalSec % app->getStartInterval();
	}
	
	m_timer = std::make_shared<boost::asio::deadline_timer>(TIMER.getIO(), boost::posix_time::seconds(firstSleepSec));
	m_timer->async_wait(boost::bind(&TimerAction::onTimeOut, this, boost::asio::placeholders::error));

	LOG_INF << "Timer will sleep <" << firstSleepSec << "> seconds for app <" << app->getName() <<
		">, next start time is <" << Utility::convertTime2Str(std::chrono::system_clock::now() + std::chrono::seconds(firstSleepSec))
		<< ">.";
}

TimerAction::~TimerAction()
{
	const static char fname[] = "TimerAction::~TimerAction() ";
	LOG_DBG << fname << "Entered.";
}

void TimerAction::onTimeOut(const boost::system::error_code& ec)
{
	const static char fname[] = "TimerAction::onTimeOut() ";

	if (ec)
	{
		LOG_WAR << "TimerAction is canceled.";
		delete this;
	}
	else
	{
		std::shared_ptr<ApplicationShortRun> app = m_app.lock();
		
		if (app != nullptr && app->isNormal())
		{
			LOG_INF << fname << "Timeout for app <" << app->getName() << ">.";
			std::shared_ptr<Application> self = app;
			app->invokeNow(self);

			// Set next timer
			m_timer->expires_at(m_timer->expires_at() + boost::posix_time::seconds(app->getStartInterval()));
			m_timer->async_wait(boost::bind(&TimerAction::onTimeOut, this, boost::asio::placeholders::error));
		}
		else if (nullptr == app)
		{
			cancelTimer();
		}
	}
}

void TimerAction::cancelTimer()
{
	const static char fname[] = "TimerAction::cancelTimer() ";
	try
	{
		m_timer->cancel();
	}
	catch (...)
	{
		LOG_WAR << fname << "Cancel timer failed";
	}
}
