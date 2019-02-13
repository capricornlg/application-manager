#ifndef __STOUT_POSIX_NET_HPP__
#define __STOUT_POSIX_NET_HPP__

#include <unistd.h>
#include <sys/types.h>
#include <ifaddrs.h>

#include <list>
#include <string>

#include "../../common/Utility.h"

namespace net {

	struct NetInterface
	{
		std::string name;
		bool ipv4;
		string address;
	};

	inline struct addrinfo createAddrInfo(int socktype, int family, int flags)
	{
		struct addrinfo addr;
		memset(&addr, 0, sizeof(addr));
		addr.ai_socktype = socktype;
		addr.ai_family = family;
		addr.ai_flags |= flags;

		return addr;
	}


	// Returns a Try of the IP for the provided hostname or an error if no IP is
	// obtained.
	inline std::shared_ptr<sockaddr> getIP(const std::string& hostname, int family = AF_UNSPEC)
	{
		const static char fname[] = "net::getIP() ";

		struct addrinfo hints = createAddrInfo(SOCK_STREAM, family, 0);
		struct addrinfo* result = nullptr;

		int error = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);

		if (error != 0) {
			LOG_ERR << fname << "getaddrinfo failed with error :" << std::strerror(errno);
			return nullptr;
		}

		if (result->ai_addr == nullptr) {
			freeaddrinfo(result);
			LOG_WAR << fname << "No addresses found";
			return nullptr;
		}

		auto ip = std::make_shared<sockaddr>(*result->ai_addr);

		freeaddrinfo(result);
		return ip;
	}

	inline std::string getAddressStr(const struct sockaddr* storage)
	{
		const static char fname[] = "net::getAddressStr() ";

		char hostname[MAXHOSTNAMELEN];
		socklen_t length;

		if (storage->sa_family == AF_INET) {
			length = sizeof(struct sockaddr_in);
		}
		else if (storage->sa_family == AF_INET6) {
			length = sizeof(struct sockaddr_in6);
		}
		else {
			LOG_WAR << fname << "Unsupported family :" << storage->sa_family;
			return "";
		}

		int error = getnameinfo(storage, length, hostname, NI_MAXHOST, NULL, 0, NI_NUMERICHOST);

		if (error != 0) {
			LOG_ERR << fname << "getnameinfo failed, error :" << std::strerror(errno);
			return "";
		}
		return std::string(hostname);
	}

	// Returns the names of all the link devices in the system.
	inline std::list<NetInterface> links()
	{
		const static char fname[] = "net::links() ";

		struct ifaddrs* ifaddr = nullptr;
		if (getifaddrs(&ifaddr) == -1) {
			LOG_ERR << fname << "getifaddrs failed, error :" << std::strerror(errno);
			return std::list<NetInterface>();
		}

		std::list<NetInterface> names;
		for (struct ifaddrs* ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
			if (ifa->ifa_name != nullptr && (ifa->ifa_addr->sa_family == AF_INET || ifa->ifa_addr->sa_family == AF_INET6)) {
				NetInterface mi;
				mi.name = ifa->ifa_name;
				mi.ipv4 = (AF_INET == ifa->ifa_addr->sa_family);
				mi.address = getAddressStr(ifa->ifa_addr);
				names.push_back(mi);
			}
		}

		freeifaddrs(ifaddr);
		return names;
	}


	inline std::string hostname()
	{
		const static char fname[] = "net::hostname() ";

		char host[512];

		if (gethostname(host, sizeof(host)) < 0) {
			LOG_ERR << fname << "gethostname failed, error :" << std::strerror(errno);
			return "";
		}

		struct addrinfo hints = createAddrInfo(SOCK_STREAM, AF_UNSPEC, AI_CANONNAME);
		struct addrinfo* result = nullptr;

		int error = getaddrinfo(host, nullptr, &hints, &result);

		if (error != 0) {
			LOG_ERR << fname << "getaddrinfo failed, error :" << std::strerror(error);
			return "";
		}

		std::string hostname = result->ai_canonname;
		freeaddrinfo(result);

		return hostname;
	}


} // namespace net {

#endif // __STOUT_POSIX_NET_HPP__
