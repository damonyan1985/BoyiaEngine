
#ifdef _WIN32
#if defined(_MSC_VER) && !defined(_CRT_SECURE_NO_WARNINGS)
#define _CRT_SECURE_NO_WARNINGS // _CRT_SECURE_NO_WARNINGS for sscanf errors in MSVC2013 Express
#endif
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <WS2tcpip.h>
#include <WinSock2.h>
#include <fcntl.h>
#pragma comment(lib, "ws2_32")
#include <io.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#ifndef _SSIZE_T_DEFINED
typedef int ssize_t;
#define _SSIZE_T_DEFINED
#endif
#ifndef _SOCKET_T_DEFINED
typedef SOCKET socket_t;
#define _SOCKET_T_DEFINED
#endif
#ifndef snprintf
#define snprintf _snprintf
//#define snprintf _snprintf_s
#endif
#if _MSC_VER >= 1600
// vs2010 or later
#include <stdint.h>
#else
typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;
typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;
typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;
#endif
#define socketerrno WSAGetLastError()
#define SOCKET_EAGAIN_EINPROGRESS WSAEINPROGRESS
#define SOCKET_EWOULDBLOCK WSAEWOULDBLOCK
#define BOYIA_LOG(format_str, ...) fprintf(stdout, format_str##"\n", __VA_ARGS__)
#else
#include <fcntl.h>
#include <netdb.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#ifndef _SOCKET_T_DEFINED
typedef int socket_t;
#define _SOCKET_T_DEFINED
#endif
#ifndef INVALID_SOCKET
#define INVALID_SOCKET (-1)
#endif
#ifndef SOCKET_ERROR
#define SOCKET_ERROR (-1)
#endif
#define closesocket(s) ::close(s)
#include <errno.h>
#define socketerrno errno
#define SOCKET_EAGAIN_EINPROGRESS EAGAIN
#define SOCKET_EWOULDBLOCK EWOULDBLOCK
#define BOYIA_LOG(format_str, ...)
#endif

#include <string>
#include <vector>

#include "Mutex.h"
#include "AutoLock.h"
#include "WebSocket.h"
#include "KList.h"
#include "OwnerPtr.h"
#include "ThreadPool.h"

namespace yanbo {

static socket_t hostname_connect(const std::string& hostname, int port)
{
    struct addrinfo hints;
    struct addrinfo* result;
    struct addrinfo* p;
    int ret;
    socket_t sockfd = INVALID_SOCKET;
    char sport[16];
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    snprintf(sport, 16, "%d", port);
    
    if ((ret = getaddrinfo(hostname.c_str(), sport, &hints, &result)) != 0) {
        BOYIA_LOG("getaddrinfo: %s", gai_strerror(ret));
        return 1;
    }

    for (p = result; p != NULL; p = p->ai_next) {
        sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (sockfd == INVALID_SOCKET) {
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) != SOCKET_ERROR) {
            break;
        }
        closesocket(sockfd);
        sockfd = INVALID_SOCKET;
    }
    freeaddrinfo(result);
    return sockfd;
}

class SocketSendTask : public TaskBase {
public:
    SocketSendTask(const String& msg, int type, socket_t sockfd, bool mask)
        : message(msg)
        , headerType(type)
        , wsockfd(sockfd)
        , useMask(mask)
    {
    }

    virtual LVoid execute()
    {
        std::string msg(GET_STR(message));
        sendData(headerType, msg.size(), msg.begin(), msg.end());
    }

    void sendImpl(std::vector<uint8_t>& txbuf)
    {
        while (txbuf.size()) {
            int ret = ::send(wsockfd, (char*)& txbuf[0], txbuf.size(), 0);
            if (ret < 0 && (socketerrno == SOCKET_EWOULDBLOCK || socketerrno == SOCKET_EAGAIN_EINPROGRESS)) {
                break;
            } else if (ret <= 0) {
                closesocket(wsockfd);
                //readyState = CLOSED;
                BOYIA_LOG(ret < 0 ? "Connection error! %d" : "Connection closed! %d", 1);
                break;
            } else {
                txbuf.erase(txbuf.begin(), txbuf.begin() + ret);
            }
        }

        /*
        if (!txbuf.size() && readyState == CLOSING) {
            closesocket(sockfd);
            readyState = CLOSED;
        }*/
    }

    template <class Iterator>
    void sendData(int type, uint64_t message_size, Iterator message_begin, Iterator message_end)
    {
        // TODO:
        // Masking key should (must) be derived from a high quality random
        // number generator, to mitigate attacks on non-WebSocket friendly
        // middleware:

        const uint8_t masking_key[4] = { 0x12, 0x34, 0x56, 0x78 };
        // TODO: consider acquiring a lock on txbuf...
        /*
        if (readyState == CLOSING || readyState == CLOSED) {
            return;
        }*/
        std::vector<uint8_t> header;
        header.assign(2 + (message_size >= 126 ? 2 : 0) + (message_size >= 65536 ? 6 : 0) + (useMask ? 4 : 0), 0);
        header[0] = 0x80 | type;
        if (message_size < 126) {
            header[1] = (message_size & 0xff) | (useMask ? 0x80 : 0);
            if (useMask) {
                header[2] = masking_key[0];
                header[3] = masking_key[1];
                header[4] = masking_key[2];
                header[5] = masking_key[3];
            }
        } else if (message_size < 65536) {
            header[1] = 126 | (useMask ? 0x80 : 0);
            header[2] = (message_size >> 8) & 0xff;
            header[3] = (message_size >> 0) & 0xff;
            if (useMask) {
                header[4] = masking_key[0];
                header[5] = masking_key[1];
                header[6] = masking_key[2];
                header[7] = masking_key[3];
            }
        } else { // TODO: run coverage testing here
            header[1] = 127 | (useMask ? 0x80 : 0);
            header[2] = (message_size >> 56) & 0xff;
            header[3] = (message_size >> 48) & 0xff;
            header[4] = (message_size >> 40) & 0xff;
            header[5] = (message_size >> 32) & 0xff;
            header[6] = (message_size >> 24) & 0xff;
            header[7] = (message_size >> 16) & 0xff;
            header[8] = (message_size >> 8) & 0xff;
            header[9] = (message_size >> 0) & 0xff;
            if (useMask) {
                header[10] = masking_key[0];
                header[11] = masking_key[1];
                header[12] = masking_key[2];
                header[13] = masking_key[3];
            }
        }
        // N.B. - txbuf will keep growing until it can be transmitted over the socket:
        // Add message header
        std::vector<uint8_t> txbuf;
        txbuf.insert(txbuf.end(), header.begin(), header.end());
        // Add message body
        txbuf.insert(txbuf.end(), message_begin, message_end);
        if (useMask) {
            size_t message_offset = txbuf.size() - message_size;
            for (size_t i = 0; i != message_size; ++i) {
                txbuf[message_offset + i] ^= masking_key[i & 0x3];
            }
        }

        sendImpl(txbuf);
    }

private:
    String message;
    int headerType;
    socket_t wsockfd;
    bool useMask;
};

class DummyWebSocket : public WebSocket {
public:
    void poll(int timeout) {}
    void send(const String& message) {}
    void sendBinary(const std::string& message) {}
    void sendBinary(const std::vector<uint8_t>& message) {}
    void sendPing() {}
    void close() {}
    readyStateValues getReadyState() const { return CLOSED; }
    void dispatch() {}
    void setHandler(WebSocketHandler* handler) {}
};

class WebSocketImpl : public WebSocket {
public:
    // http://tools.ietf.org/html/rfc6455#section-5.2  Base Framing Protocol
    //
    //  0                   1                   2                   3
    //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
    // +-+-+-+-+-------+-+-------------+-------------------------------+
    // |F|R|R|R| opcode|M| Payload len |    Extended payload length    |
    // |I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
    // |N|V|V|V|       |S|             |   (if payload len==126/127)   |
    // | |1|2|3|       |K|             |                               |
    // +-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
    // |     Extended payload length continued, if payload len == 127  |
    // + - - - - - - - - - - - - - - - +-------------------------------+
    // |                               |Masking-key, if MASK set to 1  |
    // +-------------------------------+-------------------------------+
    // | Masking-key (continued)       |          Payload Data         |
    // +-------------------------------- - - - - - - - - - - - - - - - +
    // :                     Payload Data continued ...                :
    // + - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
    // |                     Payload Data continued ...                |
    // +---------------------------------------------------------------+
    struct wsheader_type {
        unsigned header_size;
        bool fin;
        bool mask;
        enum opcode_type {
            CONTINUATION = 0x0,
            TEXT_FRAME = 0x1,
            BINARY_FRAME = 0x2,
            CLOSE = 8,
            PING = 9,
            PONG = 0xa,
        } opcode;
        int N0;
        uint64_t N;
        uint8_t masking_key[4];
    };

    std::vector<uint8_t> rxbuf;
    std::vector<uint8_t> txbuf;
    std::vector<uint8_t> receivedData;

    socket_t sockfd;
    readyStateValues readyState;
    bool useMask;
    bool isRxBad;
    WebSocketHandler* wsHandler;
    Mutex mutex;
    KList<OwnerPtr<String>> msgList;
    

    WebSocketImpl(socket_t sockfd, bool useMask)
        : sockfd(sockfd)
        , readyState(OPEN)
        , useMask(useMask)
        , isRxBad(false)
        , wsHandler(NULL)
    {
    }

    virtual void setHandler(WebSocketHandler* handler)
    {
        wsHandler = handler;
    }

    readyStateValues getReadyState() const
    {
        return readyState;
    }

    virtual void poll(int timeout)
    { 
        // timeout in milliseconds
        if (readyState == CLOSED) {
            if (timeout > 0) {
                timeval tv = { timeout / 1000, (timeout % 1000) * 1000 };
                select(0, NULL, NULL, NULL, &tv);
            }
            return;
        }

        //if (timeout != 0) {
        fd_set rfds;
        fd_set wfds;
        timeval tv = { timeout / 1000, (timeout % 1000) * 1000 };
        FD_ZERO(&rfds);
        FD_ZERO(&wfds);
        FD_SET(sockfd, &rfds);
        if (txbuf.size()) {
            FD_SET(sockfd, &wfds);
        }
        select(sockfd + 1, &rfds, &wfds, 0, timeout > 0 ? &tv : 0);
        //}

        while (true) {
            // FD_ISSET(0, &rfds) will be true
            int N = rxbuf.size();
            ssize_t ret;
            rxbuf.resize(N + 1500);
            ret = recv(sockfd, (char*)&rxbuf[0] + N, 1500, 0);
            if (ret < 0 && (socketerrno == SOCKET_EWOULDBLOCK || socketerrno == SOCKET_EAGAIN_EINPROGRESS)) {
                rxbuf.resize(N);
                break;
            } else if (ret <= 0) {
                rxbuf.resize(N);
                closesocket(sockfd);
                readyState = CLOSED;
                fputs(ret < 0 ? "Connection error!\n" : "Connection closed!\n", stderr);
                break;
            } else {
                rxbuf.resize(N + ret);
            }
        }

        /*
        while (txbuf.size()) {
            int ret = ::send(sockfd, (char*)&txbuf[0], txbuf.size(), 0);
            if (ret < 0 && (socketerrno == SOCKET_EWOULDBLOCK || socketerrno == SOCKET_EAGAIN_EINPROGRESS)) {
                break;
            } else if (ret <= 0) {
                closesocket(sockfd);
                readyState = CLOSED;
                fputs(ret < 0 ? "Connection error!\n" : "Connection closed!\n", stderr);
                break;
            } else {
                txbuf.erase(txbuf.begin(), txbuf.begin() + ret);
            }
        }
        if (!txbuf.size() && readyState == CLOSING) {
            closesocket(sockfd);
            readyState = CLOSED;
        }*/
    }

    virtual void dispatch()
    {
        // TODO: consider acquiring a lock on rxbuf...
        if (isRxBad) {
            return;
        }
        while (true) {
            wsheader_type ws;
            if (rxbuf.size() < 2) {
                return; /* Need at least 2 */
            }
            const uint8_t* data = (uint8_t*)&rxbuf[0]; // peek, but don't consume
            ws.fin = (data[0] & 0x80) == 0x80;
            ws.opcode = (wsheader_type::opcode_type)(data[0] & 0x0f);
            ws.mask = (data[1] & 0x80) == 0x80;
            ws.N0 = (data[1] & 0x7f);
            ws.header_size = 2 + (ws.N0 == 126 ? 2 : 0) + (ws.N0 == 127 ? 8 : 0) + (ws.mask ? 4 : 0);
            if (rxbuf.size() < ws.header_size) {
                return; /* Need: ws.header_size - rxbuf.size() */
            }
            int i = 0;
            if (ws.N0 < 126) {
                ws.N = ws.N0;
                i = 2;
            } else if (ws.N0 == 126) {
                ws.N = 0;
                ws.N |= ((uint64_t)data[2]) << 8;
                ws.N |= ((uint64_t)data[3]) << 0;
                i = 4;
            } else if (ws.N0 == 127) {
                ws.N = 0;
                ws.N |= ((uint64_t)data[2]) << 56;
                ws.N |= ((uint64_t)data[3]) << 48;
                ws.N |= ((uint64_t)data[4]) << 40;
                ws.N |= ((uint64_t)data[5]) << 32;
                ws.N |= ((uint64_t)data[6]) << 24;
                ws.N |= ((uint64_t)data[7]) << 16;
                ws.N |= ((uint64_t)data[8]) << 8;
                ws.N |= ((uint64_t)data[9]) << 0;
                i = 10;
                if (ws.N & 0x8000000000000000ull) {
                    // https://tools.ietf.org/html/rfc6455 writes the "the most
                    // significant bit MUST be 0."
                    //
                    // We can't drop the frame, because (1) we don't we don't
                    // know how much data to skip over to find the next header,
                    // and (2) this would be an impractically long length, even
                    // if it were valid. So just close() and return immediately
                    // for now.
                    isRxBad = true;
                    BOYIA_LOG("ERROR: Frame has invalid frame length. Closing. %d", 1);
                    close();
                    return;
                }
            }
            if (ws.mask) {
                ws.masking_key[0] = ((uint8_t)data[i + 0]) << 0;
                ws.masking_key[1] = ((uint8_t)data[i + 1]) << 0;
                ws.masking_key[2] = ((uint8_t)data[i + 2]) << 0;
                ws.masking_key[3] = ((uint8_t)data[i + 3]) << 0;
            } else {
                ws.masking_key[0] = 0;
                ws.masking_key[1] = 0;
                ws.masking_key[2] = 0;
                ws.masking_key[3] = 0;
            }

            // Note: The checks above should hopefully ensure this addition
            //       cannot overflow:
            if (rxbuf.size() < ws.header_size + ws.N) {
                return; /* Need: ws.header_size+ws.N - rxbuf.size() */
            }

            // We got a whole message, now do something with it:
            if (ws.opcode == wsheader_type::TEXT_FRAME
                || ws.opcode == wsheader_type::BINARY_FRAME
                || ws.opcode == wsheader_type::CONTINUATION) {
                if (ws.mask) {
                    for (size_t i = 0; i != ws.N; ++i) {
                        rxbuf[i + ws.header_size] ^= ws.masking_key[i & 0x3];
                    }
                }
                receivedData.insert(receivedData.end(), rxbuf.begin() + ws.header_size, rxbuf.begin() + ws.header_size + (size_t)ws.N); // just feed
                if (ws.fin) {
                    if (wsHandler) {
                        //wsHandler->handleMessage(std::string(receivedData.begin(), receivedData.end()));
                        wsHandler->handleMessage(String(receivedData.data(), receivedData.size()));
                    }
                    receivedData.erase(receivedData.begin(), receivedData.end());
                    std::vector<uint8_t>().swap(receivedData); // free memory
                }
            } else if (ws.opcode == wsheader_type::PING) {
                if (ws.mask) {
                    for (size_t i = 0; i != ws.N; ++i) {
                        rxbuf[i + ws.header_size] ^= ws.masking_key[i & 0x3];
                    }
                }
                std::string data(rxbuf.begin() + ws.header_size, rxbuf.begin() + ws.header_size + (size_t)ws.N);
                sendData(wsheader_type::PONG, data.size(), data.begin(), data.end());
            } else if (ws.opcode == wsheader_type::PONG) {
            } else if (ws.opcode == wsheader_type::CLOSE) {
                close();
            } else {
                BOYIA_LOG("ERROR: Got unexpected WebSocket message. %d", 1);
                close();
            }

            rxbuf.erase(rxbuf.begin(), rxbuf.begin() + ws.header_size + (size_t)ws.N);
        }
    }

    void send(const String& message)
    {
        ThreadPool::getInstance()->sendTask(
            new SocketSendTask(
                message,
                wsheader_type::TEXT_FRAME,
                sockfd,
                useMask
                ));
        //std::string msg(GET_STR(message));
        //sendData(wsheader_type::TEXT_FRAME, msg.size(), msg.begin(), msg.end());
    }

    void sendPing()
    {
        std::string empty;
        sendData(wsheader_type::PING, empty.size(), empty.begin(), empty.end());
    }

    void sendBinary(const std::string& message)
    {
        sendData(wsheader_type::BINARY_FRAME, message.size(), message.begin(), message.end());
    }

    void sendBinary(const std::vector<uint8_t>& message)
    {
        sendData(wsheader_type::BINARY_FRAME, message.size(), message.begin(), message.end());
    }

    
    template <class Iterator>
    void sendData(wsheader_type::opcode_type type, uint64_t message_size, Iterator message_begin, Iterator message_end)
    {
        // TODO:
        // Masking key should (must) be derived from a high quality random
        // number generator, to mitigate attacks on non-WebSocket friendly
        // middleware:
        const uint8_t masking_key[4] = { 0x12, 0x34, 0x56, 0x78 };
        // TODO: consider acquiring a lock on txbuf...
        if (readyState == CLOSING || readyState == CLOSED) {
            return;
        }
        std::vector<uint8_t> header;
        header.assign(2 + (message_size >= 126 ? 2 : 0) + (message_size >= 65536 ? 6 : 0) + (useMask ? 4 : 0), 0);
        header[0] = 0x80 | type;
        if (message_size < 126) {
            header[1] = (message_size & 0xff) | (useMask ? 0x80 : 0);
            if (useMask) {
                header[2] = masking_key[0];
                header[3] = masking_key[1];
                header[4] = masking_key[2];
                header[5] = masking_key[3];
            }
        } else if (message_size < 65536) {
            header[1] = 126 | (useMask ? 0x80 : 0);
            header[2] = (message_size >> 8) & 0xff;
            header[3] = (message_size >> 0) & 0xff;
            if (useMask) {
                header[4] = masking_key[0];
                header[5] = masking_key[1];
                header[6] = masking_key[2];
                header[7] = masking_key[3];
            }
        } else { // TODO: run coverage testing here
            header[1] = 127 | (useMask ? 0x80 : 0);
            header[2] = (message_size >> 56) & 0xff;
            header[3] = (message_size >> 48) & 0xff;
            header[4] = (message_size >> 40) & 0xff;
            header[5] = (message_size >> 32) & 0xff;
            header[6] = (message_size >> 24) & 0xff;
            header[7] = (message_size >> 16) & 0xff;
            header[8] = (message_size >> 8) & 0xff;
            header[9] = (message_size >> 0) & 0xff;
            if (useMask) {
                header[10] = masking_key[0];
                header[11] = masking_key[1];
                header[12] = masking_key[2];
                header[13] = masking_key[3];
            }
        }
        // N.B. - txbuf will keep growing until it can be transmitted over the socket:
        // Add message header
        txbuf.insert(txbuf.end(), header.begin(), header.end());
        // Add message body
        txbuf.insert(txbuf.end(), message_begin, message_end);
        if (useMask) {
            size_t message_offset = txbuf.size() - message_size;
            for (size_t i = 0; i != message_size; ++i) {
                txbuf[message_offset + i] ^= masking_key[i & 0x3];
            }
        }
    }

    void close()
    {
        if (readyState == CLOSING || readyState == CLOSED) {
            return;
        }
        readyState = CLOSING;
        uint8_t closeFrame[6] = { 0x88, 0x80, 0x00, 0x00, 0x00, 0x00 }; // last 4 bytes are a masking key
        std::vector<uint8_t> header(closeFrame, closeFrame + 6);
        txbuf.insert(txbuf.end(), header.begin(), header.end());
    }
};

static WebSocket::pointer createWebSocket(const String& url, bool useMask, const std::string& origin)
{
    char host[512];
    int port;
    char path[512];
    if (url.GetLength() >= 512) {
        BOYIA_LOG("ERROR: url size limit exceeded: %s", GET_STR(url));
        return NULL;
    }
    if (origin.size() >= 200) {
        BOYIA_LOG("ERROR: origin size limit exceeded: %s", origin.c_str());
        return NULL;
    }
    if (sscanf(GET_STR(url), "ws://%[^:/]:%d/%s", host, &port, path) == 3) {
    } else if (sscanf(GET_STR(url), "ws://%[^:/]/%s", host, path) == 2) {
        port = 80;
    } else if (sscanf(GET_STR(url), "ws://%[^:/]:%d", host, &port) == 2) {
        path[0] = '\0';
    } else if (sscanf(GET_STR(url), "ws://%[^:/]", host) == 1) {
        port = 80;
        path[0] = '\0';
    } else {
        BOYIA_LOG("ERROR: Could not parse WebSocket url: %s", GET_STR(url));
        return NULL;
    }
    //fprintf(stderr, "easywsclient: connecting: host=%s port=%d path=/%s\n", host, port, path);
    socket_t sockfd = hostname_connect(host, port);
    if (sockfd == INVALID_SOCKET) {
        BOYIA_LOG("Unable to connect to %s:%d\n", host, port);
        return NULL;
    }
    {
        // Tips: We run this block in thread
        char* line = new char[1024];
        memset(line, 0, 1024);
        snprintf(line, 1024, "GET /%s HTTP/1.1\r\n", path);
        //::send(sockfd, line, strlen(line), 0);
        if (port == 80) {
            snprintf(line + strlen(line), 1024, "Host: %s\r\n", host);
            //::send(sockfd, line, strlen(line), 0);
        } else {
            snprintf(line + strlen(line), 1024, "Host: %s:%d\r\n", host, port);
            //::send(sockfd, line, strlen(line), 0);
        }
        snprintf(line + strlen(line), 1024, "Upgrade: websocket\r\n");
        //::send(sockfd, line, strlen(line), 0);
        snprintf(line + strlen(line), 1024, "Connection: Upgrade\r\n");
        //::send(sockfd, line, strlen(line), 0);
        if (!origin.empty()) {
            snprintf(line + strlen(line), 1024, "Origin: %s\r\n", origin.c_str());
            //::send(sockfd, line, strlen(line), 0);
        }
        snprintf(line + strlen(line), 1024, "Sec-WebSocket-Key: x3JJHMbDL1EzLkh9GBhXDw==\r\n");
        //::send(sockfd, line, strlen(line), 0);
        snprintf(line + strlen(line), 1024, "Sec-WebSocket-Version: 13\r\n");
        //::send(sockfd, line, strlen(line), 0);
        snprintf(line + strlen(line), 1024, "\r\n");
        ::send(sockfd, line, strlen(line), 0);
       
        int i = 0;
        for (; i < 2 || (i < 1023 && line[i - 2] != '\r' && line[i - 1] != '\n'); ++i) {
            if (recv(sockfd, line + i, 1, 0) == 0) {
                return NULL;
            }
        }
        
        if (i == 1023) {
            BOYIA_LOG("ERROR: Got invalid status line connecting to: %s", GET_STR(url));
            return NULL;
        }

        int status;
        if (sscanf(line, "HTTP/1.1 %d", &status) != 1 || status != 101) {
            BOYIA_LOG("ERROR: Got bad status connecting to %s: %s", GET_STR(url), line);
            return NULL;
        }
        // TODO: verify response headers,
        while (true) {
            for (i = 0; i < 2 || (i < 1023 && line[i - 2] != '\r' && line[i - 1] != '\n'); ++i) {
                if (recv(sockfd, line + i, 1, 0) == 0) {
                    return NULL;
                }
            }
            if (line[0] == '\r' && line[1] == '\n') {
                break;
            }
        }

        delete[] line;
    }
    
    int flag = 1;
    setsockopt(sockfd, IPPROTO_TCP, TCP_NODELAY, (char*)&flag, sizeof(flag)); // Disable Nagle's algorithm
#ifdef _WIN32
    u_long on = 1;
    ioctlsocket(sockfd, FIONBIO, &on);
#else
    fcntl(sockfd, F_SETFL, O_NONBLOCK);
#endif
    //fprintf(stderr, "Connected to: %s\n", url.c_str());
    return WebSocket::pointer(new WebSocketImpl(sockfd, useMask));
}

WebSocket::pointer WebSocket::create_dummy()
{
    static pointer dummy = pointer(new DummyWebSocket);
    return dummy;
}

WebSocket::pointer WebSocket::create(const String& url, const std::string& origin)
{
    return yanbo::createWebSocket(url, true, origin);
}

WebSocket::pointer WebSocket::createNoMask(const String& url, const std::string& origin)
{
    return yanbo::createWebSocket(url, false, origin);
}

void WebSocket::networkInit()
{
#ifdef _WIN32
    INT rc;
    WSADATA wsaData;
    rc = WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

void WebSocket::networkDestroy()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

}
