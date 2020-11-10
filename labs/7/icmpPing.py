
from socket import * 
import os
import sys
import struct
import time 
import select 
import binascii

ICMP_ECHO_REQUEST = 8     # the ICMP code number for PING (ECHO REQUEST)

time_min = 10000000
time_max = time_sum = time_count = 0
#
#  Yay!  you don't have to write your own checksum calculation.  Just use this routine as is.
#

def checksum(checksum_packet):
    """Calculate checksum"""
    total = 0

    # Add up 16-bit words

    num_words = len(checksum_packet) // 2
    for chunk in struct.unpack("!%sH" % num_words, checksum_packet[0:num_words*2]):
        total += chunk

    # Add any left over byte
    if len(checksum_packet) % 2:
        total += ord(checksum_packet[-1]) << 8

    # Fold 32-bits into 16-bits
    # Note the offset: in C this would return as a uint16_t type, but Python
    # returns it as signed which puts it in the wrong range for struct.pack's H cast
    # Adding the 0xffff offset moves it into the correct range, while the mask removes any overflow.
    total = (total >> 16) + (total & 0xffff)
    total += total >> 16
    return (~total + 0x10000 & 0xffff)



#
# Receive and process an echo reply
# You will have to add code to this skeleton 
#

def receiveOnePing(mySocket, ID, timeout, destAddr): 

    global time_min, time_max, time_sum, time_count
    timeLeft = timeout
    
    while 1:
        startedSelect = time.time()
        whatReady = select.select([mySocket], [], [], timeLeft) 
        howLongInSelect = (time.time() - startedSelect)
        if whatReady[0] == []: # Timeout
            return "Request timed out."
        
        timeReceived = time.time()
        recPacket, addr = mySocket.recvfrom(1024)
        
        ####
        #### You Fill in the code to fetch the ICMP header from the IP packet
        ####
        ####   - extract the header from the packet 
        ####   - get the ttl from the IP portion of the received packet and convert it to a printable number
        ####         the binascii library can help here
        ####   - unpack the header, get the number of bytes and the timestamp, calculate the delay
        ####   - return the following line using appropriate variables: 
        ####           return "Reply from %s: bytes=%d time=%f5ms TTL=%d" % this, that and other stuff
        ####

        header = struct.unpack('bbHHh', recPacket[20:28])
        _type = header[0]
        code = header[1]
        checksum = header[2]
        _id = header[3]
        seq = header[4]
        
        if timeLeft <= 0:
            return "Request timed out."

        header = struct.unpack('!BBHHHBBH4s4s', recPacket[:20])
        ttl = header[5]
        length = len(recPacket) - 20
        ipAddr = inet_ntoa(header[8])
        sent_time, = struct.unpack('d', recPacket[28:])
        _time = (timeReceived - sent_time) * 1000
        time_count += 1
        time_sum += _time 
        time_min = min(time_min, _time)
        time_max = max(time_max, _time)
        timeLeft = timeLeft - howLongInSelect 
        
        return f'Reply from {ipAddr}: bytes={length} time={_time:0.6f}ms TTL={ttl}'

#
# send an echo request
# you do not have to add code to this routine
#

def sendOnePing(mySocket, destAddr, ID):
    
    # Header is defined as:  type (8), code (8), checksum (16), id (16), sequence (16)
    
    # Make a dummy header with a 0 checksum
    myChecksum = 0
    
    # struct -- Interpret strings as packed binary data that complies with the header format
    #           and uses a data payload containing the time in packed binary format as well.
    header = struct.pack("bbHHh", ICMP_ECHO_REQUEST, 0, myChecksum, ID, 1) 
    data = struct.pack("d", time.time())
    
    # Calculate the checksum on the data and the dummy header.
    myChecksum = checksum(header + data)
    
    # Get the right checksum, and put in the header 
    if sys.platform == 'darwin':
        # Convert 16-bit integers from host to network byte order
        myChecksum = htons(myChecksum) & 0xffff 
    else:
        myChecksum = htons(myChecksum)
        
    header = struct.pack("bbHHh", ICMP_ECHO_REQUEST, 0, myChecksum, ID, 1) 
    packet = header + data
    mySocket.sendto(packet, (destAddr, 1)) 
    
    # AF_INET address must be tuple, not str 
    # Both LISTS and TUPLES consist of a number of objects
    # which can be referenced by their position number within the object.


#
# perform a single ping and receive a pong
# you do not have to change any code in this routine
#
    
def doOnePing(destAddr, timeout):
    icmp = getprotobyname("icmp")
    # SOCK_RAW is a powerful socket type. For more details: http://sock- raw.org/papers/sock_raw
    mySocket = socket(AF_INET, SOCK_RAW, icmp)
    myID = os.getpid() & 0xFFFF # Return the current process 
    sendOnePing(mySocket, destAddr, myID)
    delay = receiveOnePing(mySocket, myID, timeout, destAddr)
    mySocket.close() 
    return delay


#
# this routine performs multiple pings to a site, again, no changes to this code
#

def ping(host, timeout=1):
    
    # timeout=1 means: If one second goes by without a reply from the server,
    # the client assumes that either the client's ping or the server's pong is lost 
    
    dest = gethostbyname(host)
    print("Pinging " + dest + " using Python:")
    print("")
    
    # Send ping requests to a server separated by approximately one second
    
    while 1 :
        delay = doOnePing(dest, timeout) 
        print(delay)
        time.sleep(1)    # one second
    return delay


# Finally !  ping google.  Try out other sites as well.

url = 'google.com'

try:
    ping(url)
except KeyboardInterrupt:
    print(f"\n --- {url} statistics --- ")
    print(f"rtt min/avg/max = {time_min:.03f}/{time_sum/time_count:.03f}/{time_max:.03f} ms")
