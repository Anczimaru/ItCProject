Decription



Necessary libs to compile server and client:
1. Before compilation install boost library:
  sudo apt-get install libboost-all-dev 
2. To compile server: g++ server_mine.cpp -o server -lboost_system -lboost_thread
3. To compile client: g++ client_mine.cpp -o client -lboost_system -lpthread

Compile unit tests:
g++ -Wall -Wextra curve_points_tests.cpp
Make sure you have Boost framework installed

How it works?:
1.  Client sends ACK to server
2.  Server sends chain of messages to client
3.  Client receives them, and if received end of sending from server, send his messages and then ACK
4.  Server waits for messages from Cli and if FIN_ACK received sends FIN_ACK
5.  exchange succesfull
