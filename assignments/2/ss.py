import sys
import socket
from urllib.parse import urlparse
import pickle
from _thread import *
import threading
import os
import random


def ss(conn, addr, port, buffsize):
    # Recieve chain info and URL from connection
    data = conn.recv(buffsize)
    # Array of arrays where each array is a chaingang.txt connection
    data = pickle.loads(data)
    stepping_stones = data[:-1]
    url = data[-1] 
    # Grabbing the filename from the url or resovle to default filename
    if('/' in url):
        a = urlparse(url)
        filename = os.path.basename(a.path)
    else:
        filename = "index.html"

    print("Request:", url + "...")
    if len(stepping_stones) == 0:
        print("chainlist is empty")
        print("issuing wget for file " + str(filename))
        
        command = "wget " + url
        os.system(command)
        print("File recieved")
        print("Relaying file ...")
        # Read the file in small chunks to transmit previous ss
        put_file(conn, filename, buffsize)
    else:
        print("chainlist is ")
        print(*stepping_stones, sep = "\n")

        random_stone = random.choice(stepping_stones)
        print("next SS is " + str(random_stone))
        data.remove(random_stone)
        nextss = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        nextss.connect((random_stone[0],int(random_stone[1])))
        nextss.send(pickle.dumps(data))
        print("waiting for file ...")
        get_file(nextss, filename)
        print("Relaying file...")
        put_file(conn, filename, buffsize)
        
    # Delete local file
    command = "rm " + filename
    os.system(command)
    print("Goodbye!")

def put_file(socket, filename, buffsize) :
    with open(filename,'rb') as file:
        line = file.read(buffsize)
        while line:
            socket.send(line)
            line = file.read(buffsize)
            if not line:
                break

    socket.close()

def get_file(sock, filename):
    with open(filename, 'wb') as file:
        while True:
            data = sock.recv(1024)
            if data == None:
                break
            file.write(data)

def dispatch(port=20000) :
    # Variable Setup
    buffsize = 1024
    hostname = socket.gethostname()
    ip = socket.gethostbyname(hostname)
    print(f"Listening on {hostname}, port {port}.")

    # Create Socket and bind it to given port and ip it's running on
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    print(f"Attempting to bind to port {port}.")
    try:
        s.bind((ip, int(port)))
    except Exception as e:
        print(f"Error with {ip}:{port}. Exception: {e}")
    else:
        print("Binding successful.")
        # Listen for one connection
        s.listen(1)
        print("Listening...")
        while True:
            # Accept connection
            conn, addr = s.accept()
            # Start new thread for each stepping stone
            t = threading.Thread(target=ss, args=(conn, addr, port, buffsize, ), daemon=False)
            t.start()

    finally:
        s.close()


if __name__ == "__main__" :
    if len(sys.argv) == 2 :
        dispatch(sys.argv[1])
    else :
        dispatch()


