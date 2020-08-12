# Chat

This chat has two programs: a server and a client. You can run the client program so that the server will have multiple clients.

The functionality includes:
Register to the chat with your username.
Sending a message broadcast to all users.
Exit from the chat.

### Prerequisites

You must use Linux OS, because of the implementation and the libraries were used.

### Creating object files

Create pbject files using gcc command:

```
gcc server.c -o server
gcc client.c -o client
```

### Linch the programs

```
./server
./client <ip port user_name>
```
note: The server program should get as an argument <port> to llisten to. If doesn’t get, use a default port

you can find an ilustration in this folder
