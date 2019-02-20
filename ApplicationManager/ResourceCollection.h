#ifndef RESOURCE_COLLECTION_H
#define RESOURCE_COLLECTION_H
#include <mutex>
#include <memory>
#include <string>
#include <map>
#include <unistd.h>

#include <cpprest/json.h>

//////////////////////////////////////////////////////////////////////////
// Collect host and application resource usage metrics
//////////////////////////////////////////////////////////////////////////
struct HostResource
{
	HostResource() :m_cores(0), m_sockets(0), m_processors(0), m_total_bytes(0), m_free_bytes(0), m_totalSwap_bytes(0), m_freeSwap_bytes(0) {}
	
	// CPU
	size_t m_cores;
	size_t m_sockets;
	size_t m_processors;
	// MEM
	uint64_t m_total_bytes;
	uint64_t m_free_bytes;
	uint64_t m_totalSwap_bytes;
	uint64_t m_freeSwap_bytes;
	// TODO: disk

	// NET
	std::map<std::string, std::string> m_ipaddress;
};

class ResourceCollection
{
private:
	ResourceCollection();
public:
	virtual ~ResourceCollection();
	// Internal Singleton.
	static ResourceCollection* instance();

	std::string getHostName();
	const HostResource& getHostResource();

	uint64_t getRssMemory(pid_t pid = getpid());

	void dump();

	web::json::value AsJson();
	
private:
	HostResource m_resources;
	std::recursive_mutex m_mutex;
};

#endif