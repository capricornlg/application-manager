#include <thread>
#include "Process.h"
#include "../common/Utility.h"


Process::Process()
{
}


Process::~Process()
{
}

void Process::attach(int pid)
{
	this->child_id_ = pid;
}

void Process::killgroup()
{
	const static char fname[] = "Process::killgroup() ";

	if (this->running() && this->getpid() > 1)
	{
		ACE_OS::kill(-(this->getpid()), 9);
		this->terminate();
		if (this->wait() < 0)
		{
			//avoid  zombie process (Interrupted system call)
			LOG_WAR << fname << "Wait process <" << getpid() << "> to exit failed with error : " << std::strerror(errno);
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
			if (this->wait() < 0)
			{
				LOG_ERR << fname << "Retry wait process <" << getpid() << "> failed with error : " << std::strerror(errno);
			}
			else
			{
				LOG_INF << fname << "Retry wait process <" << getpid() << "> success";
			}
		}
	}
}