/*
 * HostPortSocket class to handle serial communication - source
 */

#include "HostPortSocket.h"

//Begins
bool HostPortSocket::begin(const std::string& ipaddr, unsigned short port) {
    _terminator = TERMINATOR;
    _header = HEADER;
    _timeout = TIMEOUT;
    _port = port;
    _ip = ipaddr;
    if (init(ipaddr, port, _timeout)) {
        _isFirstRead = true;
        return true;
    }
    return false;
}

bool HostPortSocket::begin(const std::string& ipaddr, unsigned short port, unsigned int header, unsigned int terminator) {
    _terminator = terminator;
    _header = header;
    _timeout = TIMEOUT;
    _port = port;
    _ip = ipaddr;
    if (init(ipaddr, port, _timeout)) {
        _isFirstRead = true;
        return true;
    }
    return false;
}

bool HostPortSocket::begin(const std::string& ipaddr, unsigned short port, unsigned int header, unsigned int terminator, unsigned int timeout) {
    _terminator = terminator;
    _header = header;
    _port = port;
    _ip = ipaddr;
    _timeout = timeout;
    if (init(ipaddr, port, _timeout)) {
        _isFirstRead = true;
        return true;
    }
    return false;
}

HostPortSocket::~HostPortSocket() {
    client.close();
}

HostPortSocket::HostPortSocket() {
    //empty
}

//set/get funs
bool HostPortSocket::isInit(void) {
    return (bool)client;
}
bool HostPortSocket::setPort(unsigned short port) {
    //if (!serial) {
        _port = port;
        return true;
    //}
    //return false;
}
bool HostPortSocket::setIP(const std::string& ipaddr) {
   // if (!serial) {
        _ip = ipaddr;
        return true;
    //}
    //return false;
}
bool HostPortSocket::setHeader(unsigned int header) {
    //if (!serial) {
        _header = header;
        return true;
    //}
    //return false;
}
bool HostPortSocket::setTerminator(unsigned int terminator) {
    //if (!serial) {
        _terminator = terminator;
        return true;
    //}
    //return false;
}
bool HostPortSocket::setTimeout(unsigned int timeout) {
    //if (!serial) {
        _timeout = timeout;
        return true;
    //}
    //return false;
}
unsigned short HostPortSocket::getPort(void) {
    return _port;
}
std::string HostPortSocket::getIP(void) {
    return _ip;
}
unsigned int HostPortSocket::getHeader(void) {
    return _header;
}
unsigned int HostPortSocket::getTerminator(void) {
    return _terminator;
}
unsigned int HostPortSocket::getTimeout(void) {
    return _timeout;
}

//reset & close fun
bool HostPortSocket::close(void) {
    return client.close();
}
bool HostPortSocket::restart(void) {
    client.close();
    return init(_ip, _port, _timeout);
}

bool HostPortSocket::flush(void) {
    if (isInit()) {
        return restart();
    }
    return false;
}

//write
bool HostPortSocket::write(unsigned char* packetPtr, unsigned int size) {

    if (!client) {
        return false;
    }
    
    /*if (size > TX_BUF_SIZE) {
        return false;
    }*/

    size_t c = 0; //counter for sent size

    unsigned char* _tx_buf = (unsigned char*) malloc(size+8); //transmit buffer

    //put header
    _tx_buf[c++] = _header & MASK;
    _tx_buf[c++] = (_header >> 8) & MASK;
    _tx_buf[c++] = (_header >> 16) & MASK;
    _tx_buf[c++] = (_header >> 24) & MASK;

    //add data
    for (int i = 0; i < size; ++i) _tx_buf[c++] = *(packetPtr + i);
    
    //put terminator
    _tx_buf[c++] = _terminator & MASK;
    _tx_buf[c++] = (_terminator >> 8) & MASK;
    _tx_buf[c++] = (_terminator >> 16) & MASK;
    _tx_buf[c++] = (_terminator >> 24) & MASK;

    //put start bytes in buf
    client.write_n(_tx_buf, c);

    //free
    free(_tx_buf);

    return true;

}

//read
bool HostPortSocket::read(unsigned char* packetPtr, unsigned int size) {

    if (!client) {
        return false;
    }

    if (_isFirstRead) {
        _isFirstRead = false;
    }

    unsigned int c = 0;
    unsigned char b;
    while (1) {
        if (c < 4) { //check header
            if (client.read_n(&b, 1)>0) {
                if (b == ((_header >> (8 * c)) & MASK)) {
                    c++;
                    continue;
                }
            } else { 
                return false; 
            }
        } else if (c==4) { //read data
            if (client.read_n(packetPtr, size) == 0) {
                return false; 
            };
            c++;
            continue;
        } else {
            if (client.read_n(&b, 1) > 0) {
                if (b == ((_terminator >> (8 * (c - 5))) & MASK)) {
                    c++;
                    if (c == 9) { //all ok
                        return true;
                    }
                    continue;
                }
            } else { 
                break;
            }
        }
        break;
    }
    return false; //is ok???
}

//init
bool HostPortSocket::init(const std::string& ipaddr, unsigned short port, unsigned int timeout) { 
    client.connect({ipaddr, port});
    if (client) {
        client.read_timeout(std::chrono::milliseconds(timeout));
        client.write_timeout(std::chrono::milliseconds(timeout));
    }
    return (bool) client;
}

