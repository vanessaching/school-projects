/*
 * Manages room membership and broadcasts, ensures thread-safe access to receivers
 * CSF Assignment 5
 * V. Ching
 * vching1@jhu.edu
 * S. Sayyad
 * ssayyad1@jhu.edu
 */

#include "guard.h"
#include "message.h"
#include "message_queue.h"
#include "user.h"
#include "room.h"

Room::Room(const std::string &room_name)
  : room_name(room_name) {
  //initialize the mutex
  pthread_mutex_init(&lock, nullptr);
}

Room::~Room() {
  //destroy the mutex
  pthread_mutex_destroy(&lock);
}

void Room::add_member(User *user) {
  //add User to the room
  Guard g(lock);
  members.insert(user);
}

void Room::remove_member(User *user) {
  //remove User from the room
  Guard guard(lock);
  members.erase(user);
}

void Room::broadcast_message(const std::string &sender_username, const std::string &message_text) {
  //send a message to every (receiver) User in the room
  std::string payload = room_name + ":" + sender_username + ":" + message_text;
  Guard guard(lock);
  for (User *u : members) {
    // Each receiver gets its own heap-allocated delivery Message
    Message *msg = new Message(TAG_DELIVERY, payload);
    u->mqueue.enqueue(msg);
  }
}
