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

const HostResource& ResourceCollection::getHostResource()
{
	static auto cpus = os::cpus();
	auto nets = net::links();
	auto mem = os::memory();

	std::lock_guard<std::recursive_mutex> guard(m_mutex);

	m_resources.m_ipaddress.clear();

	// CPU
	std::set<int> sockets;
	std::set<int> processers;
	for (auto c : cpus)
	{
		sockets.insert(c.socket);
		processers.insert(c.id);
	}
	m_resources.m_cores = cpus.size();
	m_resources.m_sockets = sockets.size();
	m_resources.m_processors = processers.size();

	// Memory
	if (mem != nullptr)
	{
		m_resources.m_total_bytes = mem->total_bytes;
		m_resources.m_totalSwap_bytes = mem->totalSwap_bytes;
		m_resources.m_free_bytes = mem->free_bytes;
		m_resources.m_freeSwap_bytes = mem->freeSwap_bytes;
	}
	else
	{
		m_resources.m_total_bytes = m_resources.m_totalSwap_bytes = m_resources.m_free_bytes = m_resources.m_freeSwap_bytes = 0;
	}
	
	// Net
	for (auto net : nets)
	{
		// Only show ipv4 here, and do not need show lo
		if (net.ipv4 && net.address != "127.0.0.1")
		{
			m_resources.m_ipaddress[net.name]  = net.address;
		}
	}

	return m_resources;
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
	for (auto &pair : m_resources.m_ipaddress)
	{
		LOG_DBG << fname << "m_ipaddress: " << pair.first << "," << pair.second;
	}
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
	this->getHostResource();
	std::lock_guard<std::recursive_mutex> guard(m_mutex);

	web::json::value result = web::json::value::object();
	result[GET_STRING_T("host_name")] = web::json::value::string(GET_STRING_T(getHostName()));
	auto arr = web::json::value::array(m_resources.m_ipaddress.size());
	int idx = 0;
	std::for_each(m_resources.m_ipaddress.begin(), m_resources.m_ipaddress.end(), [&arr, &idx](const std::pair<std::string, std::string>& pair)
	{
		web::json::value net = web::json::value::object();
		net[GET_STRING_T(pair.first)] = web::json::value::string(GET_STRING_T(pair.second));
		arr[idx++] = net;
	});
	result[GET_STRING_T("net_interfaces")] = arr;
	result[GET_STRING_T("cpu_cores")] = web::json::value::number(m_resources.m_cores);
	result[GET_STRING_T("cpu_sockets")] = web::json::value::number(m_resources.m_sockets);
	result[GET_STRING_T("cpu_processors")] = web::json::value::number(m_resources.m_processors);
	result[GET_STRING_T("mem_total_bytes")] = web::json::value::number(m_resources.m_total_bytes);
	result[GET_STRING_T("mem_free_bytes")] = web::json::value::number(m_resources.m_free_bytes);
	result[GET_STRING_T("mem_totalSwap_bytes")] = web::json::value::number(m_resources.m_totalSwap_bytes);
	result[GET_STRING_T("mem_freeSwap_bytes")] = web::json::value::number(m_resources.m_freeSwap_bytes);
	auto allAppMem = os::pstree();
	if (nullptr != allAppMem)
	{
		result[GET_STRING_T("mem_applications")] = web::json::value::number(allAppMem->totalRSS());
	}
	return result;
}
