#include <cmath>
#include <algorithm>
#include <cpprest/json.h> // JSON library 
#include <jsoncpp/json/config.h>
#include <jsoncpp/json/value.h>
#include <jsoncpp/json/reader.h>
#include "RestHandler.h"
#include "Configuration.h"
#include "../common/Utility.h"

#define REST_INFO_PRINT \
	LOG_DBG << "Method: " << message.method(); \
	LOG_DBG << "URI: " << http::uri::decode(message.relative_uri().path()); \
	LOG_DBG << "Query: " << http::uri::decode(message.relative_uri().query());

RestHandler::RestHandler(int port)
{
	const static char fname[] = "RestHandler::RestHandler() ";

	// init
	std::string address = "http://0.0.0.0:";
	address += std::to_string(port);

	m_listener = std::make_shared<http_listener>(address);
	m_listener->support(methods::GET, std::bind(&RestHandler::handle_get, this, std::placeholders::_1));
	m_listener->support(methods::PUT, std::bind(&RestHandler::handle_put, this, std::placeholders::_1));
	m_listener->support(methods::POST, std::bind(&RestHandler::handle_post, this, std::placeholders::_1));
	m_listener->support(methods::DEL, std::bind(&RestHandler::handle_delete, this, std::placeholders::_1));

	this->open();

	LOG_INF << fname << "Listening for requests at:" << address;
}

RestHandler::~RestHandler()
{
	this->close();
}

void RestHandler::open()
{
	m_listener->open().wait();
}

void RestHandler::close()
{
	m_listener->close();// .wait();
}

void RestHandler::handle_get(http_request message)
{
	const static char fname[] = "RestHandler::handle_get() ";
	bool locked = false;
	try
	{
		REST_INFO_PRINT;
		auto path = GET_STD_STRING(http::uri::decode(message.relative_uri().path()));

		if (path == string("/app-manager/applications"))
		{
			message.reply(status_codes::OK, Configuration::instance()->getApplicationJson());
		}
		else if (path == "/app-manager/config")
		{
			message.reply(status_codes::OK, Configuration::prettyJson(GET_STD_STRING(Configuration::instance()->getConfigContentStr())));
		}
		else if (Utility::startWith(path, "/app/"))
		{
			// Get app name from path
			std::string app;
			std::vector<std::string> pathVec = Utility::splitString(path, "/");
			if (pathVec.size() >= 2) app = pathVec[1];
			// /app/someapp
			std::string getPath = std::string("/app/").append(app);
			// /app/someapp/output
			std::string outputPath = getPath + "/output";
			if (path == getPath)
			{
				message.reply(status_codes::OK, Configuration::prettyJson(GET_STD_STRING(Configuration::instance()->getApp(app)->AsJson(true).serialize())));
			}
			else if (path == outputPath)
			{
				locked = m_testAppMutex.try_lock();
				if (locked)
				{
					auto querymap = web::uri::split_query(web::http::uri::decode(message.relative_uri().query()));
					int timeout = 5; // default use 5 seconds
					const int maxTimeout = 100; // set max timeout to 100s
					if (querymap.find(U("timeout")) != querymap.end())
					{
						// max than 1 and less than 100
						auto requestTimeout = std::max(std::stoi(GET_STD_STRING(querymap.find(U("timeout"))->second)), 1);
						timeout = std::min(requestTimeout, maxTimeout);
						LOG_DBG << fname << "Use timeout :" << timeout;
					}
					else
					{
						LOG_DBG << fname << "Use default timeout :" << timeout;
					}
					message.reply(status_codes::OK, Configuration::instance()->getApp(app)->testRun(timeout));
				}
				else
				{
					throw std::invalid_argument("Do not allow test more than one application at the same time");
				}
			}
			else
			{
				throw std::invalid_argument("No such path");
			}
		}
		else
		{
			throw std::invalid_argument("No such path");
		}
	}
	catch (const std::exception& e)
	{
		message.reply(web::http::status_codes::InternalError, e.what());
	}
	catch (...)
	{
		message.reply(web::http::status_codes::InternalError, U("unknown exception"));
	}
	if (locked) m_testAppMutex.unlock();
}

void RestHandler::handle_put(http_request message)
{
	try
	{
		REST_INFO_PRINT;
		checkToken(getToken(message));

		auto path = GET_STD_STRING(http::uri::decode(message.relative_uri().path()));
		if (Utility::startWith(path, "/app/"))
		{
			auto jsonApp = message.extract_json(true).get();
			if (jsonApp.is_null())
			{
				throw std::invalid_argument("invalid json format");
			}
			auto app = Configuration::instance()->addApp(jsonApp);
			message.reply(status_codes::OK, Configuration::prettyJson(GET_STD_STRING(app->AsJson(true).serialize())));
		}
		else
		{
			message.reply(status_codes::ServiceUnavailable, "Require action query flag");
		}
	}
	catch (const std::exception& e)
	{
		message.reply(web::http::status_codes::InternalError, e.what());
	}
	catch (...)
	{
		message.reply(web::http::status_codes::InternalError, U("unknown exception"));
	}
	return;
}

void RestHandler::handle_post(http_request message)
{
	try
	{
		REST_INFO_PRINT;

		checkToken(getToken(message));
		auto path = GET_STD_STRING(http::uri::decode(message.relative_uri().path()));
		auto querymap = web::uri::split_query(web::http::uri::decode(message.relative_uri().query()));
		if (Utility::startWith(path, "/app/"))
		{
			auto appName = path.substr(strlen("/app/"), path.length() - strlen("/app/"));


			if (querymap.find(U("action")) != querymap.end())
			{
				auto action = GET_STD_STRING(querymap.find(U("action"))->second);
				if (action == "start")
				{
					Configuration::instance()->startApp(appName);
					message.reply(status_codes::OK, "Success");
				}
				else if (action == "stop")
				{
					Configuration::instance()->stopApp(appName);
					message.reply(status_codes::OK, "Success");
				}
				else
				{
					message.reply(status_codes::ServiceUnavailable, "No such action query flag");
				}
			}
			else
			{
				message.reply(status_codes::ServiceUnavailable, "Require action query flag");
			}
		}
		else
		{
			message.reply(status_codes::ServiceUnavailable, "No such path");
		}
	}
	catch (const std::exception& e)
	{
		message.reply(web::http::status_codes::InternalError, e.what());
	}
	catch (...)
	{
		message.reply(web::http::status_codes::InternalError, U("unknown exception"));
	}
}

void RestHandler::handle_delete(http_request message)
{
	try
	{
		REST_INFO_PRINT;

		checkToken(getToken(message));
		auto path = GET_STD_STRING(message.relative_uri().path());
		
		if (Utility::startWith(path, "/app/"))
		{
			auto appName = path.substr(strlen("/app/"), path.length() - strlen("/app/"));

			Configuration::instance()->removeApp(appName);
			message.reply(status_codes::OK, "Success");
		}
		else
		{
			message.reply(status_codes::ServiceUnavailable, "No such path");
		}
	}
	catch (const std::exception& e)
	{
		message.reply(web::http::status_codes::InternalError, e.what());
	}
	catch (...)
	{
		message.reply(web::http::status_codes::InternalError, U("unknown exception"));
	}
	return;
}

void RestHandler::handle_error(pplx::task<void>& t)
{
	const static char fname[] = "Configuration::handle_error() ";

	try
	{
		t.get();
	}
	catch (const std::exception& e)
	{
		LOG_ERR << fname << e.what();
	}
	catch (...)
	{
		LOG_ERR << fname << "unknown exception";
	}
}


bool RestHandler::checkToken(const std::string& token)
{
	auto clientTime = Utility::convertStr2Time(token);
	auto diff = std::abs(std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now() - clientTime).count());
	if (diff > 15)
	{
		throw std::invalid_argument("token invalid: untrusted");
	}
	// TODO: token check here.
	return true;
}

std::string RestHandler::getToken(const http_request& message)
{
	std::string token;
	if (message.headers().has("token"))
	{
		auto tokenInHeader = message.headers().find("token");
		token = Utility::decode64(tokenInHeader->second);
	}
	
	if (token.empty())
	{
		throw std::invalid_argument("Access denied:must have token.");
	}
	return token;
}
