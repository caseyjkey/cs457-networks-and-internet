mport socket module
from socket import *
from threading import Thread
from socketserver import ThreadingMixIn
import sys # In order to terminate the program

class ClientThread(Thread):
    def __init__(self, ip, port):
        Thread.__init__(self)
        self.ip = ip
        self.port = port
        print(f'Ready to serve on {ip}:{port}...')
    
    def run(self):
        while True:
            connSocket, addr = serverSocket.accept()
            try:
                message = connSocket.recv(1024).decode()
                filename = message.split()[1]
                f = open(filename[1:])
                outputdata = f.read()
                connSocket.send("HTTP/1.1 200 OK\r\n\r\n".encode())
                for i in range(0, len(outputdata)):
                    connSocket.send(outputdata[i].encode())
                connSocket.send("\r\n".encode())
                connSocket.close()
            except IOError:
                connSocket.send("<html><head></head><body><h1>404 Not Found</h1></body></html>\r\n".encode())
                connSocket.send("Some content\r\n".encode())
                connSocket.close()


        

serverSocket = socket(AF_INET, SOCK_STREAM) #Prepare a sever socket

#####Fill in start
port = 6789
serverSocket.bind(("127.0.0.1", port))
serverSocket.listen(1)
threads = []
#####Fill in end

while True:
    serverSocket.listen(4)
    print("Main threading waiting for connections")
    conn, (ip, port) = serverSocket.accept()
    newthread = ClientThread(ip, port)
    newthread.start()
    threads.append(newthread)
    
for t in threads:
    t.join()
    
    """
    print('Ready to serve...')
    connectionSocket, addr = serverSocket.accept()
    try:
        message = connectionSocket.recv(1024).decode() ####Fill in start # do a recv on connection socket, don't forget to append ".decode()" #####Fill in end 
        
        # Extract the path of the requested object from the message
        # The path is the second part of HTTP header, identified by [1]
        filename = message.split()[1]
        
        # Because the extracted path of the HTTP request includes 
        # a character '\', we read the path starting from the second character
        f = open(filename[1:])
        
        # Store the entire content of the requested file in a temporary buffer variable
        outputdata = f.read()
        
        #Send one HTTP header line into socket to indicate all is ok
        connectionSocket.send("HTTP/1.1 200 OK\r\n\r\n".encode())
        
        #Send the content of the requested file to the client 
        
        for i in range(0, len(outputdata)):
            connectionSocket.send(outputdata[i].encode()) 
        connectionSocket.send("\r\n".encode())
        connectionSocket.close() 
    
    except IOError:
        #Send response message for file not found
        ##### Fill in start
        ### you should send a header similar to the ok message above, but with error code 404 NOT FOUND.
        ### then send some content: 
        connectionSocket.send("<html><head></head><body><h1>404 Not Found</h1></body></html>\r\n".encode())
        connectionSocket.send("Some content\r\n".encode())
        ##### Fill in end
        
        #Close client socket
        ##### Fill in start
        connectionSocket.close()
        ##### Fill in end
    """
    
serverSocket.close()
sys.exit()  #Terminate the program after sending the corresponding data
