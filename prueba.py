import socket

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)

sock.bind(("192.168.1.132", 4444))
try:
    while(1):
        data, add = sock.recvfrom(1024)

        print(data.decode())
except KeyboardInterrupt:
    pass
print("CERRANDO")
sock.close()
