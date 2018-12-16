
#ifndef APPLICATION_DEFINITION
#define APPLICATION_DEFINITION

#include <memory>
#include <string>
#include <map>
#include <mutex>
#include <ace/Pipe.h>
#include <cpprest/json.h>
#include "Process.h"
#include "DailyLimitation.h"
enum STATUS
{
	STOPPED = 0,
	NORMAL
};

//////////////////////////////////////////////////////////////////////////
// Application is one process used to manage
//////////////////////////////////////////////////////////////////////////
class Application
{
public:
	Application();
	virtual ~Application();
	std::string getName();
	bool isNormal();
	static void FromJson(std::shared_ptr<Application>& app, const web::json::object& obj);

	virtual void refreshPid();
	void attach(std::map<std::string, int>& process);

	// Invoke immediately
	virtual void invokeNow(std::shared_ptr<Application>& self);
	// Invoke by scheduler
	virtual void invoke();
	
	virtual void stop();
	virtual void start(std::shared_ptr<Application>& self);
	std::string testRun(size_t timeoutSeconds);

	virtual web::json::value AsJson(bool returnRuntimeInfo);
	virtual void dump();

	int spawnProcess(std::shared_ptr<Process> process, std::shared_ptr<ACE_Pipe> pipe = nullptr);
	bool isInDailyTimeRange();

protected:
	STATUS m_active;
	std::string m_name;
	std::string m_commandLine;
	std::string m_user;
	std::string m_workdir;
	//the exit code of last instance
	int m_return;
	std::string m_posixTimeZone;
	
	std::shared_ptr<Process> m_process;
	int m_pid;
	std::recursive_mutex m_mutex;
	std::shared_ptr<DailyLimitation> m_dailyLimit;
	std::map<std::string, std::string> m_envMap;
};

#endif 