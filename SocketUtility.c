#include "SocketUtility.h"

bool get_local_interfaces()
{
	int status;
	char buffer[RTPUDPV4TRANS_IFREQBUFSIZE];
	struct ifconf ifc;
	struct ifreq *ifr;
	struct sockaddr *sa;
	char *startptr,*endptr;
	int remlen;
	
	ifc.ifc_len = RTPUDPV4TRANS_IFREQBUFSIZE;
	ifc.ifc_buf = buffer;
	status = ioctl(rtpsock,SIOCGIFCONF,&ifc);
	if (status < 0)
		return false;
	
	startptr = (char *)ifc.ifc_req;
	endptr = startptr + ifc.ifc_len;
	remlen = ifc.ifc_len;
	while((startptr < endptr) && remlen >= (int)sizeof(struct ifreq))
	{
		ifr = (struct ifreq *)startptr;
		sa = &(ifr->ifr_addr);
#ifdef RTP_HAVE_SOCKADDR_LEN
		if (sa->sa_len <= sizeof(struct sockaddr))
		{
			if (sa->sa_len == sizeof(struct sockaddr_in) && sa->sa_family == PF_INET)
			{
				uint32_t ip;
				struct sockaddr_in *addr = (struct sockaddr_in *)sa;
				
				ip = ntohl(addr->sin_addr.s_addr);
				localIPs.push_back(ip);
			}
			remlen -= sizeof(struct ifreq);
			startptr += sizeof(struct ifreq);
		}
		else
		{
			int l = sa->sa_len-sizeof(struct sockaddr)+sizeof(struct ifreq);
			
			remlen -= l;
			startptr += l;
		}
#else // don't have sa_len in struct sockaddr
		if (sa->sa_family == PF_INET)
		{
			uint32_t ip;
			struct sockaddr_in *addr = (struct sockaddr_in *)sa;
		
			ip = ntohl(addr->sin_addr.s_addr);
			localIPs.push_back(ip);
		}
		remlen -= sizeof(struct ifreq);
		startptr += sizeof(struct ifreq);
	
#endif // RTP_HAVE_SOCKADDR_LEN
	}

	if (localIPs.empty())
		return false;
	return true;
}
























