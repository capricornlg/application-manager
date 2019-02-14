#ifndef ARGUMENT_PARSER_H
#define ARGUMENT_PARSER_H
#include <string>
#include <iomanip>
#include <cpprest/json.h>
#include <cpprest/http_client.h>
#include <boost/program_options.hpp>

using namespace web;                        // Common features like URIs.
using namespace web::http;                  // Common HTTP functionality
using namespace web::http::client;          // HTTP client features
using namespace concurrency::streams;       // Asynchronous streams

using namespace std;
namespace po = boost::program_options;

class ArgumentParser
{
public:
	ArgumentParser(int argc, char* argv[], int listenPort);
	virtual ~ArgumentParser();

	void parse();

private:
	void printMainHelp();
	void processReg();
	void processUnReg();
	void processView();
	void processConfig();
	void processResource();
	void processStartStop(bool start);
	void processTest();

	bool confirmInput(const char* msg);
	http_response requestHttp(const method & mtd, const string& path);
	http_response requestHttp(const method & mtd, const string& path, web::json::value& body);
	http_response requestHttp(const method & mtd, const string& path, std::map<string,string>& query, web::json::value * body = nullptr);
	
	void addHttpHeader(http_request& request);

private:
	bool isAppExist(const std::string& appName);
	std::map<std::string, bool> getAppList();
	void printApps(web::json::value json, bool reduce);
	void moveForwardCommandLineVariables(po::options_description& desc);
	string reduceStr(string source, int limit);

private:
	po::variables_map m_commandLineVariables;
	std::vector<po::option> m_pasrsedOptions;
	int m_listenPort;
};
#endif

