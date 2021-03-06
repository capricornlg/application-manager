#ifndef TIMER_ACTION_ONCE_H
#define TIMER_ACTION_ONCE_H
#include <mutex>
#include <memory>
#include <boost/asio/deadline_timer.hpp>

class Process;
//////////////////////////////////////////////////////////////////////////
// TimerActionKill will be used to safe kill a Process after some seconds
// This timer only run one time
//////////////////////////////////////////////////////////////////////////
class TimerActionKill
{
public:
	TimerActionKill(std::shared_ptr<Process> process, int bufferTimeSeconds);
	virtual ~TimerActionKill();

	// Callback function from timer thread
	virtual void onTimeOut(const boost::system::error_code& ec);

private:
	std::shared_ptr<Process> m_process;
	std::shared_ptr<boost::asio::deadline_timer> m_timer;
};

#endif

