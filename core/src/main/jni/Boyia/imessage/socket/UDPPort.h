#ifndef UDPPort_h
#define UDPPort_h

#include "BoyiaSocket.h"

namespace yanbo
{
class UDPPort
{
public:
	UDPPort();

private:
    BoyiaSocket m_socket;
};
}

#endif