#ifndef APP_PROCESS_H
#define APP_PROCESS_H
#include <map>
#include <ace/Process.h>


//////////////////////////////////////////////////////////////////////////
// Process Object
//////////////////////////////////////////////////////////////////////////
class Process :public ACE_Process
{
public:
	Process();
	virtual ~Process();

	void attach(int pid);
	void killgroup();
};

#endif 

