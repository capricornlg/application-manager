
#ifndef TIMER_ACTION_H
#define TIMER_ACTION_H
#include <mutex>
#include <memory>
#include <boost/asio/deadline_timer.hpp>

class ApplicationShortRun;
//////////////////////////////////////////////////////////////////////////
// Each timer action assotiate to a short running application
//////////////////////////////////////////////////////////////////////////
class TimerAction
{
public:
	explicit TimerAction(std::shared_ptr<ApplicationShortRun> app);
	virtual ~TimerAction();

	// Callback function from timer thread
	virtual void onTimeOut(const boost::system::error_code& ec);

	virtual void cancelTimer();

protected:
	std::weak_ptr<ApplicationShortRun> m_app;
	std::shared_ptr<boost::asio::deadline_timer> m_timer;
};

#endif

