#ifndef APP_PROCESS_H
#define APP_PROCESS_H
#include <map>
#include <string>
#include <algorithm>
#include <ace/Process.h>
#include "LinuxCgroup.h"
#include "ResourceLimitation.h"

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
	void setCgroup(std::string appName,int index, std::shared_ptr<ResourceLimitation>& limit);
	std::string getuuid();
private:
	std::shared_ptr<LinuxCgroup> m_cgroup;
	std::shared_ptr<ResourceLimitation> m_resourceLimit;
	std::string m_uuid;
};

#endif 

