clear
clc

h = HostPortSocket();
h.begin('192.168.0.1',9876,HostPortSocket.HEADER,HostPortSocket.TERMINATOR,HostPortSocket.TIMEOUT);
len = 32;
type = 'single';

if ~h.IsInit
    error('Unable to connect');
end

figure
an = animatedline;
hold on
c = 0;
while true
    [data, exit] = h.read(len, type);
    if exit
        addpoints(an,c,double(data(10)));
        drawnow;
        c = c+1;
    end

end