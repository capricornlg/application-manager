#ifndef APP_PROCESS
#define APP_PROCESS
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

