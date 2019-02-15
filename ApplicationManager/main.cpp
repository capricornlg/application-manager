#include <stdio.h>
#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <fstream>
#include <ace/Init_ACE.h>

#include "RestHandler.h"
#include "../common/Utility.h"
#include "Application.h"
#include "Configuration.h"
#include "Timer.h"
#include "ResourceCollection.h"

using namespace std;

// whether use a dedicate thread for timer event
#define USE_SEPERATE_TIMER_THREAD false

static std::shared_ptr<Configuration> readConfiguration();
static std::string                    m_applicationPath;
static std::shared_ptr<RestHandler>   m_httpHandler;
static std::shared_ptr<boost::asio::deadline_timer> m_timer;
void monitorAllApps(const boost::system::error_code &ec);

int main(int argc, char * argv[])
{
	const static char fname[] = "main() ";
	PRINT_VERSION();

	try
	{
		ACE::init();
		Utility::initLogging();
		LOG_DBG << fname << "Entered.";

		m_applicationPath = Utility::getSelfFullPath();
		auto config = readConfiguration();
		Utility::setLogLevel(config->getLogLevel());
		m_httpHandler = std::make_shared<RestHandler>(config->getRestListenPort());

		auto apps = config->getApps();
		auto process = Utility::getProcessList();
		std::for_each(apps.begin(), apps.end(), [&process](std::vector<std::shared_ptr<Application>>::reference p) {p->attach(process); });

		ResourceCollection::instance()->getHostResource();
		ResourceCollection::instance()->dump();

		if (USE_SEPERATE_TIMER_THREAD)
		{
			// Timer will have a new thread, main thread will used to monitor process
			TIMER.init();
			while (true)
			{
				std::this_thread::sleep_for(std::chrono::seconds(config->getScheduleInterval()));

				auto apps = config->getApps();

				for (const auto& app : apps)
				{
					app->invoke();
				}
			}
		}
		else
		{
			// main thread will be used to handle asio::io_service events and block here
			// Application monitor will use an async_wait timer event
			monitorAllApps(boost::system::error_code());
			TIMER.runTimerThread();
		}
	}
	catch (const std::exception& e)
	{
		LOG_ERR << fname << e.what();
	}
	catch (...)
	{
		LOG_ERR << fname << "unknown exception";
	}
	LOG_ERR << "ERROR exited";
	TIMER.stop();
	ACE::fini();
	_exit(0);
	return 0;
}

std::shared_ptr<Configuration> readConfiguration()
{
	const static char fname[] = "readConfiguration() ";

	try
	{
		std::shared_ptr<Configuration> config;
		web::json::value jsonValue;
		string jsonPath = m_applicationPath + ".json";
		ifstream jsonFile(jsonPath);
		if (!jsonFile.is_open())
		{
			LOG_ERR << "can not open configuration file <" << jsonPath << ">";
			config = std::make_shared<Configuration>();
			throw std::runtime_error("can not open configuration file");
		}
		else
		{
			std::string str((std::istreambuf_iterator<char>(jsonFile)), std::istreambuf_iterator<char>());
			jsonFile.close();

			LOG_DBG << str;
			config = Configuration::FromJson(str);
			config->dump();
		}

		return config;
	}
	catch (const std::exception& e)
	{
		LOG_ERR << fname << e.what();
		throw;
	}
	catch (...)
	{
		LOG_ERR << fname << "unknown exception";
		throw;
	}
}

void monitorAllApps(const boost::system::error_code &ec)
{
	auto apps = Configuration::instance()->getApps();
	for (const auto& app : apps)
	{
		app->invoke();
	}
	// Set timer
	if (nullptr == m_timer)
	{
		m_timer = std::make_shared<boost::asio::deadline_timer>(TIMER.getIO(), boost::posix_time::seconds(Configuration::instance()->getScheduleInterval()));
	}
	else
	{
		m_timer->expires_at(m_timer->expires_at() + boost::posix_time::seconds(Configuration::instance()->getScheduleInterval()));
	}
	m_timer->async_wait(&monitorAllApps);
}
