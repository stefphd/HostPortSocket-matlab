
//include MATLAB mex
#include "mex.h"

//include std
#include <map>
#include <string>
#include <cstring>
#include <memory> 

//include HostPortSocket class
#include "HostPortSocket.h"

//define
#define getUint64 (uint64_t*) mxGetData
#define getUint32 (uint32_t*) mxGetData
#define getUint16 (uint16_t*) mxGetData
#define getUint8 (uint8_t*) mxGetData

//#define DEBUG

//List of class method
enum class Action {
    New,
    Delete,
    Begin,
    Close,
    Restart,
    Flush,
    Read,
    Write,
    SetPort,
    SetIP,
    SetHeader,
    SetTerminator,
    SetTimeout,
    GetPort,
    GetIP,
    GetHeader,
    GetTerminator,
    GetTimeout,
    IsInit,
    GetHandles,
    HEADER,
    TERMINATOR,
    TIMEOUT
};

//Map string to Action
std::map<std::string, Action> actionTypeMap = {
    {"new",             Action::New             },
    {"delete",          Action::Delete          },
    {"begin",           Action::Begin           },
    {"close",           Action::Close           },
    {"restart",         Action::Restart         },
    {"flush",           Action::Flush           },
    {"read",            Action::Read            },
    {"write",           Action::Write           },
    {"setPort",         Action::SetPort         },
    {"setIP",           Action::SetIP           },
    {"setHeader",       Action::SetHeader       },
    {"setTerminator",   Action::SetTerminator   },
    {"setTimeout",      Action::SetTimeout      },
    {"getPort",         Action::GetPort         },
    {"getIP",           Action::GetIP           },
    {"getHeader",       Action::GetHeader       },
    {"getTerminator",   Action::GetTerminator   },
    {"getTimeout",      Action::GetTimeout      },
    {"isInit",          Action::IsInit          },
    {"getHandles",      Action::GetHandles      },
    {"HEADER",          Action::HEADER          },
    {"TERMINATOR",      Action::TERMINATOR      },
    {"TIMEOUT",         Action::TIMEOUT         },
};

//Map string to Data types
std::map<std::string, mxClassID> typeSizeMap = {
//    {"char",            mxCHAR_CLASS            },
    {"double",          mxDOUBLE_CLASS          },
    {"single",          mxSINGLE_CLASS          },
    {"int8",            mxINT8_CLASS            },
    {"uint8",           mxUINT8_CLASS           },
    {"int16",           mxINT16_CLASS           },
    {"uint16",          mxUINT16_CLASS          },
    {"int32",           mxINT32_CLASS           },
    {"uint32",          mxUINT32_CLASS          },
    {"int64",           mxINT64_CLASS           },
    {"uint64",          mxUINT64_CLASS          },      
};

//Map handle to pointer
std::map<uint64_t, std::shared_ptr<HostPortSocket>> handleMap;

//exit fnc
int mexAtExit(void (*ExitFcn)(void)) {
    for (const auto& [k, v] : handleMap) 
        handleMap.erase(k), mexUnlock();
    return 0;
}

//main
void mexFunction(int nlhs, mxArray *plhs[],int nrhs, const mxArray *prhs[]) {
    
    Action action;

    if (nrhs > 0 && !mxIsChar(prhs[0])) //check input
        mexErrMsgTxt("First input must be an action string ('new', 'delete', or a method name).");
    
    if (nrhs > 0) { 
        char *actionCstr = mxArrayToString(prhs[0]);
        std::string actionStr(actionCstr);
        mxFree(actionCstr);
        
        if (actionTypeMap.count(actionStr) == 0)
            mexErrMsgTxt(("Unrecognized action (not in actionTypeMap): " + actionStr).c_str());
        
        action = actionTypeMap.at(actionStr);
    } else //default 'new'
        action = Action::New;
    
    //single-argument methods
    switch (action) {

        case Action::New: {
            plhs[0] = mxCreateNumericMatrix(1,1,mxUINT64_CLASS,mxREAL);
            std::shared_ptr<HostPortSocket> handle(new HostPortSocket());
            uint64_t handle_id = (uint64_t) handle.get();
            handleMap[handle_id] = handle;
            *(getUint64(plhs[0])) = handle_id;
            mexLock();
            return;
        }
        
        case Action::GetHandles: {
            plhs[0] = mxCreateNumericMatrix(handleMap.size(),1,mxUINT64_CLASS,mxREAL);
            int i = 0;
            for (const auto& [k, v] : handleMap)
                *(getUint64(plhs[0])+i) = k, ++i;
            return;
        }

        case Action::Delete: {
            if (nrhs == 1) { //delete all if no ptr given
                uint32_t size = handleMap.size();
                handleMap.clear();
                for (int i = 0; i < size; ++i)
                    mexUnlock();
                return;
            }
            break;
        }
        case Action::HEADER: {
            plhs[0] = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
            *(getUint32(plhs[0])) = HostPortSocket::HEADER;
            return;
        }
        case Action::TERMINATOR: {
            plhs[0] = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
            *(getUint32(plhs[0])) = HostPortSocket::TERMINATOR;
            return;
        }
        case Action::TIMEOUT: {
            plhs[0] = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
            *(getUint32(plhs[0])) = HostPortSocket::TIMEOUT;
            return;
        }
    }
    
    //multiple-argument methods
    if (nrhs < 2 || (mxGetClassID(prhs[1]) != mxUINT64_CLASS)) //check inputs
                mexErrMsgTxt("Second input must be an unsigned integer.");
    
    uint64_t handle_id = static_cast<uint64_t>(mxGetScalar(prhs[1]));
    if (handleMap.count(handle_id) == 0) //check if valid handle
        mexErrMsgTxt("Unrecognized handle");

    HostPortSocket* handle = handleMap[handle_id].get(); //pointer to HostPortSocket object
    
    switch (action) {

        case Action::Delete: {
            handleMap.erase(handle_id);
            mexUnlock();
            return;
        }
        
        case Action::Begin: { //ptr, ip, port, header, terminator
            if (nrhs < 4 || (!mxIsChar(prhs[2])) || (mxGetClassID(prhs[3]) != mxUINT32_CLASS))
                mexErrMsgTxt("Third and fourth inputs must be a char array and a scalar.");
            char *ipCstr = mxArrayToString(prhs[2]);
            std::string ipaddr(ipCstr);
            mxFree(ipCstr);
            unsigned int port = static_cast<unsigned int>(mxGetScalar(prhs[3]));
            unsigned int header = HostPortSocket::HEADER;
            unsigned int terminator = HostPortSocket::TERMINATOR;
            unsigned int timeout = HostPortSocket::TIMEOUT;
            if (nrhs > 4) header = static_cast<unsigned int>(mxGetScalar(prhs[4]));
            if (nrhs > 5) terminator = static_cast<unsigned int>(mxGetScalar(prhs[5]));
            if (nrhs > 6) timeout = static_cast<unsigned int>(mxGetScalar(prhs[6]));
            plhs[0] = mxCreateLogicalMatrix(1,1); 
            *(mxGetLogicals(plhs[0])) = handle->begin(ipaddr, port, header, terminator, timeout);        
            #ifdef DEBUG
            printf("begin with\n");
            printf("\tport:\t%u\n",handle->getPort());
            printf("\tip:\t%s\n",handle->getIP());
            printf("\theader:\t%u\n",handle->getHeader());
            printf("\tterminator:\t%u\n",handle->getTerminator());
            #endif
            return;
        }

        case Action::Close: {
            plhs[0] = mxCreateLogicalMatrix(1,1); 
            *(mxGetLogicals(plhs[0])) = handle->close();
            return;
        }

        case Action::Restart: {
            plhs[0] = mxCreateLogicalMatrix(1,1); 
            *(mxGetLogicals(plhs[0])) = handle->restart();
            return;
        }
        
        case Action::Flush: {
            plhs[0] = mxCreateLogicalMatrix(1,1); 
            *(mxGetLogicals(plhs[0])) = handle->flush();
            return;
        }

        case Action::IsInit: {
            plhs[0] = mxCreateLogicalMatrix(1,1); 
            *(mxGetLogicals(plhs[0])) = handle->isInit();
            return;
        }

        case Action::SetPort: {
            if (nrhs > 3 || (mxGetClassID(prhs[2]) != mxUINT32_CLASS))
                mexErrMsgTxt("Third input must be a scalar.");
            unsigned int port = static_cast<unsigned int>(mxGetScalar(prhs[2]));
            plhs[0] = mxCreateLogicalMatrix(1,1); 
            *(mxGetLogicals(plhs[0])) = handle->setPort(port);
            return;
        }

        case Action::SetIP: {
            if (nrhs < 3 || (!mxIsChar(prhs[2])))
                mexErrMsgTxt("Third input must be a char array.");
            char *ipCstr = mxArrayToString(prhs[2]);
            std::string ipaddr(ipCstr);
            mxFree(ipCstr);
            plhs[0] = mxCreateLogicalMatrix(1,1); 
            *(mxGetLogicals(plhs[0])) = handle->setIP(ipaddr);
            return;
        }

        case Action::SetHeader: {
            unsigned int header = HostPortSocket::HEADER;
            if (nrhs > 2) header = static_cast<unsigned int>(mxGetScalar(prhs[2]));
            plhs[0] = mxCreateLogicalMatrix(1,1); 
            *(mxGetLogicals(plhs[0])) = handle->setHeader(header);
            return;
        }

        case Action::SetTerminator: {
            unsigned int terminator = HostPortSocket::TERMINATOR;
            if (nrhs > 2) terminator = static_cast<unsigned int>(mxGetScalar(prhs[2]));
            plhs[0] = mxCreateLogicalMatrix(1,1); 
            *(mxGetLogicals(plhs[0])) = handle->setTerminator(terminator);
            return;
        }

        case Action::SetTimeout: {
            unsigned int timeout = HostPortSocket::TIMEOUT;
            if (nrhs > 2) timeout = static_cast<unsigned int>(mxGetScalar(prhs[2]));
            plhs[0] = mxCreateLogicalMatrix(1,1); 
            *(mxGetLogicals(plhs[0])) = handle->setTimeout(timeout);
            return;
        }

        case Action::GetPort: {
            plhs[0] = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
            *(getUint32(plhs[0])) = handle->getPort();
            return;
        }

        case Action::GetIP: {
            std::string ipaddr = handle->getIP();
            const char * cstr = ipaddr.c_str();
            plhs[0] = mxCreateCharMatrixFromStrings(1, &cstr);
            return;
        }

        case Action::GetHeader: {
            plhs[0] = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
            *(getUint32(plhs[0])) = handle->getHeader();
            return;
        }

        case Action::GetTerminator: {
            plhs[0] = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
            *(getUint32(plhs[0])) = handle->getTerminator();
            return;
        }

        case Action::GetTimeout: {
            plhs[0] = mxCreateNumericMatrix(1,1,mxUINT32_CLASS,mxREAL);
            *(getUint32(plhs[0])) = handle->getTimeout();
            return;
        }

        case Action::Read: {
            if (nrhs < 3 || (mxGetClassID(prhs[2]) != mxDOUBLE_CLASS))
                mexErrMsgTxt("Third input must be a scalar.");
            if (nrhs < 4 || !mxIsChar(prhs[3]))
                mexErrMsgTxt("Fourth input must be a type string ('single', 'double', etc.).");
            
            char *typeCstr = mxArrayToString(prhs[3]);
            std::string typeStr(typeCstr);
            mxFree(typeCstr);
            if (typeSizeMap.count(typeStr) == 0)
                mexErrMsgTxt(("Unrecognized data type (not in typeSizeMap): " + typeStr).c_str());
            
            unsigned int len = static_cast<unsigned int>(mxGetScalar(prhs[2]));
            plhs[0] = mxCreateNumericMatrix(len,1,typeSizeMap.at(typeStr),mxREAL);
            plhs[1] = mxCreateLogicalMatrix(1,1);
            unsigned int size = len*mxGetElementSize(plhs[0]);
            
            unsigned char* buf;
            buf = (unsigned char*) malloc(size);
            if (buf == NULL)
                mexErrMsgTxt("Fatal error in memory allocation");
            if (handle->read(buf, size)) {
                memcpy(getUint8(plhs[0]), buf, size);
                *(mxGetLogicals(plhs[1])) = true;
            } else 
                *(mxGetLogicals(plhs[1])) = false;
            free(buf);
            return;
        }

        case Action::Write: {
            if (nrhs < 3)
                mexErrMsgTxt("Missing third input.");      
            unsigned int size = mxGetNumberOfElements(prhs[2])*mxGetElementSize(prhs[2]);
            unsigned char* buf = getUint8(prhs[2]);
            plhs[0] = mxCreateLogicalMatrix(1,1);
            *(mxGetLogicals(plhs[0])) = handle->write(buf, size);
            return;
        }
    }
}