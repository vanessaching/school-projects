/*
 * Implements the multithreaded chat server
 * CSF Assignment 5
 * V. Ching
 * vching1@jhu.edu
 * S. Sayyad
 * ssayyad1@jhu.edu
 */
 
#include <pthread.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <set>
#include <vector>
#include <cctype>
#include <cassert>
#include "message.h"
#include "connection.h"
#include "user.h"
#include "room.h"
#include "guard.h"
#include "server.h"

////////////////////////////////////////////////////////////////////////
// Server implementation data types
////////////////////////////////////////////////////////////////////////

// TODO: add any additional data types that might be helpful
//       for implementing the Server member functions

// helper struct passed to worker thread
struct ClientInfo {
  Server *server;
  Connection *conn;
};

// helper to validate usernames and room names
static bool is_valid_name(const std::string &name) {
  if (name.empty()) {
    return false;
  }
  for (char c : name) {
    if (!std::isalnum(static_cast<unsigned char>(c))) {
      return false;
    }
  }
  return true;
}

// forward declarations for per-client chat logic
static void chat_with_sender(Server *server,
                             Connection *conn,
                             const std::string &username);
static void chat_with_receiver(Server *server,
                               Connection *conn,
                               const std::string &username);

////////////////////////////////////////////////////////////////////////
// Client thread functions
////////////////////////////////////////////////////////////////////////

namespace {

void *worker(void *arg) {
  pthread_detach(pthread_self());

  // use a static cast to convert arg from a void* to
  // whatever pointer type describes the object(s) needed
  // to communicate with a client (sender or receiver)
  ClientInfo *info = static_cast<ClientInfo *>(arg);
  Server *server = info->server;
  Connection *conn = info->conn;
  delete info;

  // read login message (should be tagged either with
  // TAG_SLOGIN or TAG_RLOGIN), send response

  Message login;
  if (!conn->receive(login)) {
    if (conn->get_last_result() == Connection::INVALID_MSG) {
    // invalid first line from client
    conn->send(Message(TAG_ERR, "invalid message"));
  }
    delete conn;
    return nullptr;
  }

  //depending on whether the client logged in as a sender or
  //receiver, communicate with the client 

  if (login.tag == TAG_SLOGIN) {
    // sender login path
    if (!is_valid_name(login.data)) {
      conn->send(Message(TAG_ERR, "invalid username"));
      delete conn;
      return nullptr;
    }
    conn->send(Message(TAG_OK, "slogin ok"));
    chat_with_sender(server, conn, login.data);
  } else if (login.tag == TAG_RLOGIN) {
    // receiver login path
    if (!is_valid_name(login.data)) {
      conn->send(Message(TAG_ERR, "invalid username"));
      delete conn;
      return nullptr;
    }
    conn->send(Message(TAG_OK, "rlogin ok"));
    chat_with_receiver(server, conn, login.data);
  } else {
    // invalid initial message
    conn->send(Message(TAG_ERR, "expected slogin or rlogin"));
  }

  delete conn;
  return nullptr;
}

}

////////////////////////////////////////////////////////////////////////
// Per-client chat helpers
////////////////////////////////////////////////////////////////////////

static void chat_with_sender(Server *server,
                             Connection *conn,
                             const std::string &username) {
  Room *current_room = nullptr;

  while (true) {
    Message msg;
    if (!conn->receive(msg)) {
    if (conn->get_last_result() == Connection::INVALID_MSG) {
      // syntax error handling
      conn->send(Message(TAG_ERR, "invalid message"));
      continue;  //waiting for the next command
    }
    // disconnect
    return;
  }

    if (msg.tag == TAG_JOIN) {
      // join a room
      const std::string &room_name = msg.data;
      if (!is_valid_name(room_name)) {
        conn->send(Message(TAG_ERR, "invalid room name"));
        continue;
      }
      current_room = server->find_or_create_room(room_name);
      conn->send(Message(TAG_OK, "joined " + room_name));
    } else if (msg.tag == TAG_LEAVE) {
      // leave current room
      if (!current_room) {
        conn->send(Message(TAG_ERR, "not in a room"));
      } else {
        current_room = nullptr;
        conn->send(Message(TAG_OK, "left room"));
      }
    } else if (msg.tag == TAG_SENDALL) {
      // broadcast message to all receivers in the current room
      if (!current_room) {
        conn->send(Message(TAG_ERR, "not in a room"));
      } else {
        current_room->broadcast_message(username, msg.data);
        conn->send(Message(TAG_OK, "message delivered"));
      }
    } else if (msg.tag == TAG_QUIT) {
      // disconnect
      conn->send(Message(TAG_OK, "goodbye"));
      return;
    } else {
      // unsupported or invalid command
      conn->send(Message(TAG_ERR, "invalid command"));
    }
  }
}

static void chat_with_receiver(Server *server,
                               Connection *conn,
                               const std::string &username) {
  // create a User object for the receiver
  std::unique_ptr<User> user(new User(username));

  // expect a join message next
  Message join_msg;
  if (!conn->receive(join_msg)) {
    if (conn->get_last_result() == Connection::INVALID_MSG) {
      conn->send(Message(TAG_ERR, "invalid message"));
    }
    return;
  }
  if (join_msg.tag != TAG_JOIN) {
    conn->send(Message(TAG_ERR, "expected join after rlogin"));
    return;
  }

  const std::string &room_name = join_msg.data;
  if (!is_valid_name(room_name)) {
    conn->send(Message(TAG_ERR, "invalid room name"));
    return;
  }

  // join the room as a receiver
  Room *room = server->find_or_create_room(room_name);
  room->add_member(user.get());
  conn->send(Message(TAG_OK, "joined " + room_name));

  // main delivery loop
  while (true) {
    Message *pending = user->mqueue.dequeue();
    if (!pending) {
      // timeout waiting for a message, then loop again
      continue;
    }

    if (!conn->send(*pending)) {
      delete pending;
      break;
    }
    delete pending;
  }

  // clean up membership when connection dies
  room->remove_member(user.get());
}

////////////////////////////////////////////////////////////////////////
// Server member function implementation
////////////////////////////////////////////////////////////////////////

Server::Server(int port)
  : m_port(port)
  , m_ssock(-1) {
  //initialize mutex
  pthread_mutex_init(&m_lock, nullptr);
}

Server::~Server() {
  //destroy mutex
  pthread_mutex_lock(&m_lock);
  for (RoomMap::iterator it = m_rooms.begin(); it != m_rooms.end(); ++it) {
    delete it->second;
  }
  m_rooms.clear();
  pthread_mutex_unlock(&m_lock);
  pthread_mutex_destroy(&m_lock);
}

bool Server::listen() {
  // TODO: use open_listenfd to create the server socket, return true
  //       if successful, false if not
  std::ostringstream port_str;
  port_str << m_port;
  m_ssock = open_listenfd(port_str.str().c_str());
  if(m_ssock < 0){
    std::cerr << "Error opening listening socket on port " << m_port << "\n";
    return false;
  }
  return true;
}

void Server::handle_client_requests() {
  //  infinite loop calling accept or Accept, starting a new
  //   pthread for each connected client
   while (true) {
    sockaddr_storage client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_fd = Accept(m_ssock, (SA *)&client_addr, &client_len);
    if (client_fd < 0) {
      continue;
    }

    Connection *conn = new Connection(client_fd);
    ClientInfo *info = new ClientInfo;
    info->server = this;
    info->conn = conn;

    pthread_t tid;
    int rc = pthread_create(&tid, nullptr, worker, info);
    if (rc != 0) {
      delete conn;
      delete info;
    }
  }
}

Room *Server::find_or_create_room(const std::string &room_name) {
  // TODO: return a pointer to the unique Room object representing
  //       the named chat room, creating a new one if necessary
  Guard guard(m_lock);  

  RoomMap::iterator it = m_rooms.find(room_name);
  if (it != m_rooms.end()) {
    return it->second;
  }

  Room *room = new Room(room_name);
  m_rooms[room_name] = room;
  return room;
}