
# TCP/IP Chat Application
A simple application that enables multiple clients to exchange messages through a server using socket programming and multithreading.

## Run a server
```
gcc -Wall server.c -o server -pthread
./server 42000
```

## Run a client
In a new terminal
```
gcc -Wall client.c -o client
./client localhost 42000
```