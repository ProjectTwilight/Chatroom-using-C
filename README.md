# Chatroom-using-C
Encrypted Chat Application
Overview
This repository contains the code for a simple, yet effective, encrypted chat application built for the Windows platform. The application is designed in C and utilizes the Win32 API for its graphical user interface, along with Winsock2 for network communications. It provides basic chat functionality with an added layer of encryption for secure message exchange.

# Features
Win32 API-based GUI: A user-friendly interface including text boxes for entering messages and displaying chat history.
UDP Socket Communication: Uses Datagram sockets (UDP) for sending and receiving messages.
Basic Encryption: Implements a custom encryption and decryption mechanism to secure messages.
Two-Way Chat: Allows two users to communicate in real-time.
Local Network Communication: Configured to work in a local network environment using predefined IP addresses and port numbers.

# Installation and Setup
Clone the Repository:
git clone https://github.com/your-username/encrypted-chat-application.git

Compile the Code:
Use a C compiler like GCC or an IDE such as Visual Studio. Ensure inclusion of the Winsock2.h header file.

Important Libraries: Link the lgdi32 and lws2_32 libraries to your project. These libraries are essential for graphical interface functionality and network communication, respectively.

Run the Application:
Compile and execute the program on two different machines within the same network.

# Usage
Set Your Username: Enter your desired username in the provided field.
Type and Send Messages: Type your messages in the text box and click 'Send' to communicate.
View Chat History: The conversation will be displayed in the chat area, with encrypted and decrypted messages shown.

Configuration
IP Address and Ports: By default, the application uses 127.0.0.1 as the IP address and predefined port numbers. Modify these in the code to fit your network configuration.
Contributing
Contributions to enhance the features, improve encryption, or fix bugs are welcome. Please fork the repository and create a pull request with your changes.
