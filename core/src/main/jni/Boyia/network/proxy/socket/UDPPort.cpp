#include "UDPPort.h"

namespace yanbo
{
UDPPort::UDPPort()
{
	m_socket.create(SOCK_DGRAM);
}
}