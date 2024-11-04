# ft_irc - Custom IRC Server 🖧

Welcome to **ft_irc**! This project involves building a custom IRC (Internet Relay Chat) server in C++. It’s an exciting challenge that combines networking, protocol design, and multi-client management, following the basic IRC RFC (Request for Comments) standards.

## 📖 Project Overview
The ft_irc project is a simplified IRC server that allows multiple clients to connect, communicate in channels, and exchange messages. The server adheres to a subset of the **IRC protocol** specifications, managing user connections, channels, and message broadcasting.

### Key Features
- **Multi-client Management**: Supports multiple clients connecting and interacting simultaneously.
- **Channel Creation and Management**: Users can create, join, and leave channels.
- **Message Broadcasting**: Allows users to send messages to channels or private messages to other users.
- **Command Parsing**: Supports essential IRC commands like `/nick`, `/join`, `/part`, `/msg`, and `/quit`.
- **Weather Bot**: A bot that provides real-time weather information using the `!weather [location]` command (e.g., `!weather london`).

## 🔧 How It Works
1. **Client Connections**: Clients connect to the server using sockets, and each client connection is handled using non-blocking I/O to efficiently manage multiple clients.
2. **Command Handling**: The server listens for commands from clients, parses the input, and executes the corresponding actions (e.g., joining channels, sending messages, getting weather updates).
3. **Channel Management**: The server keeps track of active channels and the users in each channel, ensuring message delivery only to relevant users.
4. **Weather Bot Functionality**: The bot responds to commands like `!weather [location]` by fetching and sending weather data for the specified location.
5. **Concurrency**: The server handles multiple clients concurrently, ensuring smooth operation and responsiveness for all connected users.

## 📝 Compilation & Usage
To compile the project, run:
```bash
make && make bot
```

### Running the Server
Once compiled, you can start the server by specifying a port and password:
```bash
./irc_server <port> <password>
```
In another terminal:
```bash
./bot <server> <port> <nickname> <password> <channel>
```

### Connecting to the Server
You can connect to the server using any IRC client (like `ircII`, `WeeChat`, or a custom client). Here’s an example using `netcat` for testing:
```bash
nc localhost [port]
```

Once connected, you can start issuing IRC commands to interact with the server.

### Supported Commands
- **/nick** `<nickname>`: Set your nickname.
- **/join** `<#channel>`: Join a channel or create it if it doesn’t exist.
- **/part** `<#channel>`: Leave a channel.
- **/msg** `<nickname> <message>`: Send a private message to a user.
- **/quit**: Disconnect from the server.
- **!weather** `<location>`: Fetches the weather for the specified location (e.g., `!weather london`).

## 🌱 Learning Outcomes
Building ft_irc has been an incredible experience in understanding network programming, the IRC protocol, and efficient client-server communication. Managing multiple clients, implementing robust command parsing, and adding custom bot functionality were highlights of this project.

## 🔗 Connect with Me
If you're interested in networking or would like to collaborate on similar projects, feel free to reach out on [LinkedIn](https://www.linkedin.com/in/sonam-crumiere/).

Happy chatting!
