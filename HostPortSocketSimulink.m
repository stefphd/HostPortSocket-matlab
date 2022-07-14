classdef HostPortSocketSimulink < matlab.System
    % HostPortSocketSimulink 
    properties
        port (1,1) {mustBeInteger, mustBeNonnegative, mustBeLessThan(port,65535)} = 9876
        header (1,1) {mustBeInteger, mustBeNonnegative, mustBeLessThan(header,4294967296)} = double(HostPortSocketMex('HEADER'));
        terminator (1,1) {mustBeInteger, mustBeNonnegative, mustBeLessThan(terminator,4294967296)} = double(HostPortSocketMex('TERMINATOR'));
        timeout (1,1) {mustBeInteger, mustBeNonnegative, mustBeLessThan(timeout,4294967296)} = double(HostPortSocketMex('TIMEOUT'));
        len (1,1) {mustBeInteger, mustBeNonnegative} = 32
    end

    properties(SetAccess=private, Hidden)
        data
    end

    properties (Nontunable)
        ipaddr char = '192.168.0.1'
        datatype char {mustBeMember(datatype,{'uint8','uint16','uint32','uint64','int8','int16','int64','single','double'})} = 'single'
    end

    properties(DiscreteState)
    end

    % Pre-computed constants
    properties(Access = private)
        ptr_ = uint64([]);
    end

    methods(Access = protected)
        function setupImpl(obj)
        end

        function y = stepImpl(obj)
            % Implement algorithm. 
            coder.extrinsic('HostPortSocketMex');
            exit = false;
            c = 0;
            while ~exit && (c < 1000)
                [x, exit] = HostPortSocketMex('read',obj.ptr_,obj.len,obj.datatype);
                c = c + 1;
            end
            if exit
                obj.data = x;
            end
            y = obj.data;
        end

        function resetImpl(obj)
            % Initialize / reset discrete-state properties
            coder.extrinsic('HostPortSocketMex');
            obj.ptr_ = HostPortSocketMex();
            exit = HostPortSocketMex('begin',obj.ptr_,obj.ipaddr,uint32(obj.port),uint32(obj.header),uint32(obj.terminator),uint32(obj.timeout));
            if ~exit
                error('Unable to init socket')
            end
            obj.data = coder.nullcopy(zeros([obj.len 1], obj.datatype));
            obj.data = zeros([obj.len 1], obj.datatype);
        end

        function releaseImpl(obj)
            coder.extrinsic('HostPortSocketMex');
            HostPortSocketMex('close',obj.ptr_);
            HostPortSocketMex('delete',obj.ptr_);
        end

        function sz = getOutputSizeImpl(obj)
            sz = [obj.len 1];
        end
        function dt = getOutputDataTypeImpl(obj)
            dt = obj.datatype;
        end
        function cp = isOutputComplexImpl(obj)
            cp = false;
        end
        function c = isOutputFixedSizeImpl(obj)
            c = true;
        end
    end
end
