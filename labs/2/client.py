import sys, time
from socket import *

# Initialization Section
#    Set host, port and timeout
host = 'google.com'
port = 7
timeout = 1 # in second
seq = 1
# create dgram udp socket
try:
    s = socket(AF_INET, SOCK_DGRAM)
    s.settimeout(1)
except socket.error:
    print('Failed to create socket')
    sys.exit()
    
# Ping for 10 times using a while loop
min_rtt = 1
max_rtt = 0
losses = 0
total_rtt = 0
while seq <= 10:
    t = time.localtime()
    message = f"Ping {seq} {time.asctime(t)}"
    seq += 1
    
    # Put the rest in a try/except block to handle any possible timeouts
    try:
        # YOU FILL IN: Set a variable for the time sent (hint: time.time() is nice)
        start = time.time()
        # YOU FILL IN:Send the UDP packet with the ping message
        s.sendto(message.encode(), (host, port))
        
        # YOU FILL IN: Receive the server response and address; user recvfrom(1024).   Read the documentation!
        #              Pay attention to the format of the returned parameters.
        data = s.recvfrom(1024)
        reply = data[0]
        addr = data[1]
        
        # YOU FILL IN: Set a variable for the time received
        finish = time.time()
        
        # YOU FILL IN:Print the server response as an output as described earlier: PING FROM ADDRESS: MESSAGE
        #             Notice the message that is returned is a byte string, not a normal UTF-8 string
        #             To get a normal string use the mystring.decode() standard function
        #             Also, print only the IPv4 address.  I'm not interested in the port number.
        print(f"Reply from {addr[0]}: {reply.decode()}")
        # YOU FILL IN: calculate and print the round trip time
        rtt = finish-start
        print(f"RTT: {rtt}")
        if rtt < min_rtt:
            min_rtt = rtt
        if rtt > max_rtt:
            max_rtt = rtt
        total_rtt += rtt 
        
    except Exception as e:
        losses += 1
        # Server does not respond.  Assume the packet is lost
        # Assume the packet is lost
        print ("Request timed out.")
        continue

print(f'\n--- {addr[0]} ping statistics ---')
print(f'10 packets transmitted, {10-losses} received, {losses/10 * 100}% packet loss, time {total_rtt*1000:.0f}ms')
print(f'rtt min/avg/max = {min_rtt*1000:.3f}/{(total_rtt/(10-losses))*1000:.3f}/{max_rtt*1000:.3f} ms')   

# Close the client socket
s.close()
