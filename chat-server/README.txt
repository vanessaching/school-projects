CONTRIBUTIONS

Vanessa Ching: worked on sender.cpp and receiver.cpp, connection.cpp and connection.h, room.cpp
Sam Sayyad: worked on client_util.cpp, client_util.h and message.h, message_queue.cpp, server.cpp

The server is multithreaded, so mutexes were used to protect shared state in three places:
the Server has a lock around the room map when finding and creating rooms, each Room has
a lock around its member set when adding or removing receivers and broadcasting, and each
MessageQueue uses a lock and semaphore to safely enqueue and dequeue messages between sender
and receiver threads. All locks are local to their objects and held only for short operations,
broadcasts never race with joins or leaves, receivers are cleanly removed on disconnect, 
which also avoids data races and deadlocks.