#include <set>
#include "ResourceCollection.h"

#include "../common/Utility.h"

#include "../3rdparty/os/linux.hpp"
#include "../3rdparty/os/net.hpp"
#include "../3rdparty/os/pstree.hpp"


ResourceCollection::ResourceCollection()
{
}

ResourceCollection::~ResourceCollection()
{
}

ResourceCollection * ResourceCollection::instance()
{
	static ResourceCollection* singleton = new ResourceCollection();
	return singleton;
}

std::string ResourceCollection::getHostName()
{
	return net::hostname();
}

HostResource ResourceCollection::getHostResource()
{
	HostResource res;

	std::set<int> sockets;
	std::set<int> processers;
	auto cpus = os::cpus();
	for (auto c : cpus)
	{
		sockets.insert(c.socket);
		processers.insert(c.id);
	}
	res.m_cores = cpus.size();
	res.m_sockets = sockets.size();
	res.m_processors = processers.size();

	auto mem = os::memory();
	if (mem != nullptr)
	{
		res.m_total_bytes = mem->total_bytes;
		res.m_totalSwap_bytes = mem->totalSwap_bytes;
		res.m_free_bytes = mem->free_bytes;
		res.m_freeSwap_bytes = mem->freeSwap_bytes;
	}
	else
	{
		res.m_total_bytes = res.m_totalSwap_bytes = res.m_free_bytes = res.m_freeSwap_bytes = 0;
	}
	auto addr = net::getIP(net::hostname(), 2);
	if (addr != nullptr)
	{
		res.m_ipaddress = net::getAddressStr(addr.get());
	}
	
	std::lock_guard<std::recursive_mutex> guard(m_mutex);
	m_resources = res;
	return res;
}

uint64_t ResourceCollection::getRssMemory(pid_t pid)
{
	const static char fname[] = "ResourceCollection::getRssMemory() ";
	if (pid > 0)
	{
		auto tree = os::pstree(pid);
		if (nullptr != tree)
		{
			return tree->totalRSS();
		}
		else
		{
			LOG_WAR << fname << " Failed to find process" << pid;
			return 0;
		}
	}
	return 0;
}

void ResourceCollection::dump()
{
	const static char fname[] = "ResourceCollection::dump() ";

	std::lock_guard<std::recursive_mutex> guard(m_mutex);

	LOG_DBG << fname << "host_name:" << getHostName();
	LOG_DBG << fname << "m_ipaddress:" << m_resources.m_ipaddress;
	LOG_DBG << fname << "m_cores:" << m_resources.m_cores;
	LOG_DBG << fname << "m_sockets:" << m_resources.m_sockets;
	LOG_DBG << fname << "m_processors:" << m_resources.m_processors;
	LOG_DBG << fname << "m_total_bytes:" << m_resources.m_total_bytes;
	LOG_DBG << fname << "m_free_bytes:" << m_resources.m_free_bytes;
	LOG_DBG << fname << "m_totalSwap_bytes:" << m_resources.m_totalSwap_bytes;
	LOG_DBG << fname << "m_freeSwap_bytes:" << m_resources.m_freeSwap_bytes;
	
}

web::json::value ResourceCollection::AsJson()
{
	std::lock_guard<std::recursive_mutex> guard(m_mutex);

	web::json::value result = web::json::value::object();
	result[GET_STRING_T("host_name")] = web::json::value::string(GET_STRING_T(getHostName()));
	result[GET_STRING_T("ip")] = web::json::value::string(GET_STRING_T(m_resources.m_ipaddress));
	result[GET_STRING_T("cores")] = web::json::value::number(m_resources.m_cores);
	result[GET_STRING_T("sockets")] = web::json::value::number(m_resources.m_sockets);
	result[GET_STRING_T("processors")] = web::json::value::number(m_resources.m_processors);
	result[GET_STRING_T("total_bytes")] = web::json::value::number(m_resources.m_total_bytes);
	result[GET_STRING_T("free_bytes")] = web::json::value::number(m_resources.m_free_bytes);
	result[GET_STRING_T("totalSwap_bytes")] = web::json::value::number(m_resources.m_totalSwap_bytes);
	result[GET_STRING_T("freeSwap_bytes")] = web::json::value::number(m_resources.m_freeSwap_bytes);
	return result;
}
