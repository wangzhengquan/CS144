./apps/tcp_ipv4 -l 169.254.144.9 9090

sudo tshark -Pw /tmp/debug.raw -i tun144

./apps/tcp_ipv4 -d tun145 -a 169.254.145.9 169.254.144.9 9090



cmake .. -DCMAKE_BUILD_TYPE=Debug