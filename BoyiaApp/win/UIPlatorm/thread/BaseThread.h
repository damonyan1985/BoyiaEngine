#ifndef BaseThread_h
#define BaseThread_h

namespace yanbo 
{
class BaseThread {
public:
	virtual void run() = 0;
	virtual int getId() = 0;
};
}
#endif // !BaseThread_h
