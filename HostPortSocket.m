classdef HostPortSocket < handle
    %HOSTPORT HostPort class for communication to a microcontroller
    
    properties (SetAccess = private, Hidden)
        ptr_ = uint64([]);
    end

    properties (Constant)
        HEADER = HostPortSocketMex('HEADER');
        TERMINATOR = HostPortSocketMex('TERMINATOR');
        TIMEOUT = HostPortSocketMex('TIMEOUT');
    end
    
    properties (Dependent)
        Port (1,1) {mustBeInteger, mustBeNonnegative, mustBeLessThan(Port,65535)}
        IP char
        Header (1,1) {mustBeInteger, mustBeNonnegative, mustBeLessThan(Header,4294967296)}
        Terminator (1,1) {mustBeInteger, mustBeNonnegative, mustBeLessThan(Terminator,4294967296)}
        Timeout (1,1) {mustBeInteger, mustBeNonnegative, mustBeLessThan(Timeout,4294967296)}
        IsInit 
    end
    
    methods (Static)
        
        function clear()
            HostPortSocketMex('delete');
        end

    end
    
    methods (Access = public)
        
        %constructor
        function obj = HostPortSocket()
            %HOSTPORTSOCKET Construct an instance of this class
            obj.ptr_ = HostPortSocketMex(); %assign pointer of new object
        end

        %destructor
        function delete(obj)
            if any(obj.ptr_ == HostPortSocketMex('getHandles'))
                 HostPortSocketMex('delete',obj.ptr_); %delete object
            end
        end

        %begin, restart and close mathods
        function exit = begin(obj, ipaddr, port, varargin)
            %check input
            arguments
                obj
                ipaddr char
                port (1,1) {mustBeInteger, mustBeNonnegative, mustBeLessThan(port,65535)}
            end
            arguments (Repeating)
                varargin  (1,1) {mustBeInteger, mustBeNonnegative, mustBeLessThan(varargin,4294967296)}
            end
            if (nargin-1)<3
                exit = HostPortSocketMex('begin',obj.ptr_,ipaddr,uint32(port));
            elseif (nargin-1)<5
                exit = HostPortSocketMex('begin',obj.ptr_,ipaddr,uint32(port),uint32(varargin{1}),uint32(varargin{2}));    
            else   
                exit = HostPortSocketMex('begin',obj.ptr_,ipaddr,uint32(port),uint32(varargin{1}),uint32(varargin{2}),uint32(varargin{3}));
            end
        end

        function exit = restart(obj) 
            exit = HostPortSocketMex('restart',obj.ptr_);
        end

        function exit = close(obj)
           exit = HostPortSocketMex('close',obj.ptr_); 
        end
        
        function exit = flush(obj)
           exit = HostPortMex('flush',obj.ptr_); 
        end
        
        %read and write
        function [data, exit] = read(obj, len, type)
            arguments
                obj
                len {mustBeInteger, mustBeNonnegative, mustBeLessThan(len,4294967296)}
                type char {mustBeMember(type,{'uint8','uint16','uint32','uint64','int8','int16','int64','single','double'})} = 'uint8'
            end
           [data, exit] = HostPortSocketMex('read',obj.ptr_,len,type); 
        end
        
        function exit = write(obj, data)
            exit = HostPortSocketMex('write',obj.ptr_,data);            
        end
        
        %logical operators
        function exit = logical(obj)
            exit = HostPortSocketMex('isInit',obj.ptr_);
        end

        function exit = not(obj)
            exit = ~HostPortSocketMex('isInit',obj.ptr_);
        end
        
    end

    methods %get & set methods
        
        function port = get.Port(obj)
            if HostPortSocketMex('isInit',obj.ptr_)
                port = HostPortSocketMex('getPort',obj.ptr_);
            else
                port = [];
            end
        end

        function ipaddr = get.IP(obj)
            if HostPortSocketMex('isInit',obj.ptr_)
                ipaddr = HostPortSocketMex('getIP',obj.ptr_);
            else
                ipaddr = '';
            end
        end

        function header = get.Header(obj)
            if HostPortSocketMex('isInit',obj.ptr_)
                header = HostPortSocketMex('getHeader',obj.ptr_);
            else
                header = [];
            end
        end

        function terminator = get.Terminator(obj)
            if HostPortSocketMex('isInit',obj.ptr_)
                terminator = HostPortSocketMex('getTerminator',obj.ptr_);
            else
                terminator = [];
            end
        end

        function timeout = get.Timeout(obj)
            if HostPortSocketMex('isInit',obj.ptr_)
                timeout = HostPortSocketMex('getTimeout',obj.ptr_);
            else
                timeout = [];
            end
        end

        function isInit = get.IsInit(obj)
            isInit = HostPortSocketMex('isInit',obj.ptr_);
        end

        function set.Port(obj, port)
           HostPortSocketMex('setPort',obj.ptr_,uint32(port));
        end

        function set.IP(obj, baud)
           HostPortSocketMex('setIP',obj.ptr_,uint32(baud));
        end

        function set.Header(obj, header)
           HostPortSocketMex('setHeader',obj.ptr_,uint32(header)); 
        end

        function set.Terminator(obj, terminator)
           HostPortSocketMex('setTerminator',obj.ptr_,uint32(terminator)); 
        end
        
        function set.Timeout(obj, timeout)
           HostPortSocketMex('setTimeout',obj.ptr_,uint32(timeout)); 
        end

    end
end

