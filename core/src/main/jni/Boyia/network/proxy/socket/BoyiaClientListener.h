#ifndef BoyiaClientListener_h
#define BoyiaClientListener_h

namespace yanbo
{
class BoyiaClientListener
{
public:
	virtual void onSocketDataReceive(const char* data, int length) = 0;
	virtual void onSocketDataFinished() = 0;
	virtual void onSocketDataError(int code) = 0;
};
}

#endif
