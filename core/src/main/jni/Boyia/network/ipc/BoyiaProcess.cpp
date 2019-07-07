#include "BoyiaProcess.h"
#include <unistd.h>

namespace yanbo {
BoyiaProcess::BoyiaProcess()
    : m_pid(0)
{
}

void BoyiaProcess::init()
{
    m_pid = fork();
    if (m_pid < 0) {
        // 创建进程失败
    } else if (m_pid == 0) {
        // 创建进程成功，进入进程空间
        // 子进程代码段空间
        procExec();
    } else { // m_pid为子进程ID
        // 父进程
        // 父进程代码段空间
    }
}

void BoyiaProcess::procExec()
{
}
}
