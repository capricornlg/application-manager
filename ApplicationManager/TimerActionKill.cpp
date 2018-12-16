#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/asio/deadline_timer.hpp>
#include <chrono>

#include "TimerActionKill.h"
#include "../common/Utility.h"
#include "Timer.h"
#include "Application.h"

TimerActionKill::TimerActionKill(std::shared_ptr<Process> process, int bufferTimeSeconds)
	:m_process(process)
{
	const static char fname[] = "TimerActionKill::TimerActionKill() ";
	LOG_DBG << fname << "Entered.";
		
	m_timer = std::make_shared<boost::asio::deadline_timer>(TIMER.getIO(), boost::posix_time::seconds(bufferTimeSeconds));
	m_timer->async_wait(boost::bind(&TimerActionKill::onTimeOut, this, boost::asio::placeholders::error));

	LOG_INF << "Timer will sleep <" << bufferTimeSeconds << "> seconds for process <" << process->getpid() << "> to stop.";
}

TimerActionKill::~TimerActionKill()
{
	const static char fname[] = "TimerActionKill::~TimerActionKill() ";
	LOG_DBG << fname << "Entered.";
}

void TimerActionKill::onTimeOut(const boost::system::error_code& ec)
{
	const static char fname[] = "TimerActionKill::onTimeOut() ";

	if (ec)
	{
		LOG_WAR << "TimerActionKill is canceled.";
	}
	else
	{
		LOG_INF << fname << "Timeout for process <" << m_process->getpid() << ">.";
		if (m_process != nullptr && m_process->running()) m_process->killgroup();
	}
	delete this;
}

