#include "UDPWrapper.h"

UDPWrapper::UDPWrapper(unsigned short portIncoming, const std::string remoteIPAddress, unsigned short portOutgoing) :
    localAddress({0}), remoteAddress({0})
{
	initializeNetworkConfig(portIncoming, remoteIPAddress, portOutgoing);
	initializeNetwork();
}

UDPWrapper::UDPWrapper(const NetworkConfiguration& networkConfig) :
    UDPWrapper(networkConfig.localPort, networkConfig.remoteIPAddress, networkConfig.remotePort)
{
}

UDPWrapper::~UDPWrapper()
{
    if(Socket >= 0)
    {
        closeNetwork();
    }
}


void UDPWrapper::initializeNetwork()
{
	startWinsock();
	createSocket();
}

void UDPWrapper::startWinsock()
{
	// Starting Winsock for Windows
	#ifdef _WIN32
	WSADATA w;
	if (int result = WSAStartup(MAKEWORD(2, 2), &w) != 0)
	{
		std::cerr << "Failed to start Winsock 2! Error #" << result << std::endl;
	}
	#endif
}

void UDPWrapper::initializeNetworkConfig(unsigned short localPort, const std::string remoteIPAddress, unsigned short remotePort)
{
    if(NetworkWrapper::isIPv6(remoteIPAddress))
    {
        std::cout << "Using IPv6 ..." << std::endl;
        isIPv6 = true;
        localAddress.ipv6.sin6_family = AF_INET6;
        //listen on any address of this computer (localhost, local IP, ...)
        localAddress.ipv6.sin6_addr = in6addr_any;
        localAddress.ipv6.sin6_port = htons(localPort);
        localAddress.ipv6.sin6_flowinfo = {0};
        localAddress.ipv6.sin6_scope_id = {0};
        remoteAddress.ipv6.sin6_family = AF_INET6;
        inet_pton(AF_INET6, remoteIPAddress.c_str(), &(remoteAddress.ipv6.sin6_addr));
        remoteAddress.ipv6.sin6_port = htons(remotePort);
    }
    else
    {
        std::cout << "Using IPv4 ..." << std::endl;
        isIPv6 = false;
        localAddress.ipv4.sin_family = AF_INET;
        //listen on any address of this computer (localhost, local IP, ...)
        localAddress.ipv4.sin_addr.s_addr = INADDR_ANY;
        localAddress.ipv4.sin_port = htons(localPort);
        remoteAddress.ipv4.sin_family = AF_INET;
        inet_pton(AF_INET, remoteIPAddress.c_str(), &(remoteAddress.ipv4.sin_addr));
        remoteAddress.ipv4.sin_port = htons(remotePort);
    }
}


bool UDPWrapper::createSocket()
{
    unsigned int addressLength = getSocketAddressLength();
    // AF_INET - creating an IPv4 based socket
    // AF_INET6 - creating an IPv6 based socket
    if(isIPv6)
    {
        this->Socket = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    }
    else
    {
        this->Socket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    }
    if (Socket == INVALID_SOCKET)
    {
        std::wcerr << "Error on creating socket: " << getLastError() << std::endl;
        return false;
    }
    else
    {
        std::cout << "Socket created." << std::endl;
    }
    
    if (bind(Socket, (sockaddr*)&(this->localAddress), addressLength) == SOCKET_ERROR)
    {
        std::wcerr << "Error binding the socket: " << getLastError() << std::endl;
        return false;
    }
    else
    {
        std::cout << "Local port bound." << std::endl;
    }
    //set socket timeout to 1sec
#ifdef _WIN32
    DWORD timeout = 1000;
#else
    timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;
#endif
    setsockopt(Socket, SOL_SOCKET, SO_RCVTIMEO, (char*) &timeout, sizeof(timeout));
    return true;
}

int UDPWrapper::sendData(const void *buffer, const unsigned int bufferSize)
{
    return sendto(this->Socket, (char*)buffer, (int)bufferSize, 0, (sockaddr*)&(this->remoteAddress), getSocketAddressLength());
}

int UDPWrapper::receiveData(void *buffer, unsigned int bufferSize)
{
    #ifdef _WIN32
    int localAddrLen = sizeof(localAddress);
    #else
    unsigned int localAddrLen = getSocketAddressLength();
    #endif
    int result = recvfrom(this->Socket, (char*)buffer, (int)bufferSize, 0, (sockaddr*)&(this->localAddress), &localAddrLen);
    if (result == -1)
    {
        if(hasTimedOut())
        {
            //we have timed-out, so notify caller and return
            return RECEIVE_TIMEOUT;
        }
        std::wcerr << this->getLastError();
    }
    return result;
}


std::wstring UDPWrapper::getLastError() const
{
    int error;
    #ifdef _WIN32
    error = WSAGetLastError();
	wchar_t* tmp;
    FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER|FORMAT_MESSAGE_FROM_SYSTEM,0, error, LANG_USER_DEFAULT, (wchar_t*)&tmp, 0, nullptr);
    #else
    error = errno;
    wchar_t tmp[255];
    char* errPtr = strerror(error);
    mbstowcs(tmp, errPtr, 255);
    #endif
    return (std::to_wstring(error) + L" - ") + tmp;
}

void UDPWrapper::closeNetwork()
{
    shutdown(Socket, SHUTDOWN_BOTH);
    #ifdef _WIN32
    closesocket(Socket);
    #else
    close(Socket);
    #endif
    Socket = INVALID_SOCKET;
}

const int UDPWrapper::getSocketAddressLength()
{
    if(isIPv6)
    {
        return sizeof(sockaddr_in6);
    }
    return sizeof(sockaddr_in);
}
