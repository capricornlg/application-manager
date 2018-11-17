#include "Process.h"



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
	if (this->running() && this->getpid() > 1)
	{
		ACE_OS::kill(-(this->getpid()), 9);
		this->terminate();
		//avoid  zombie process
		this->wait();
	}
}