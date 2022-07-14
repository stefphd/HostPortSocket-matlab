function make()

    target = 'HostPortSocketMex';
    include = './include';
    src = 'src/HostPortSocketMex.cpp src/HostPortSocket.cpp src/sockpp/acceptor.cpp src/sockpp/connector.cpp src/sockpp/datagram_socket.cpp src/sockpp/exception.cpp src/sockpp/inet6_address.cpp src/sockpp/inet_address.cpp src/sockpp/socket.cpp src/sockpp/stream_socket.cpp';
    flags = '-lws2_32 ';

    if ispc
        cflags = ['COMPFLAGS=''$COMPFLAGS /' 'std:c++17' ''''];
     elseif isunix
        cflags = ['CXXFLAGS=''$CXXFLAGS -' 'std=c++17' ''''];
    else
        error('Unsuppoted OS');
    end

    cmd = ['mex ' src ' -I' include ' -output ' target ' ' cflags ' ' flags];
    disp(cmd);
    eval(cmd);
end