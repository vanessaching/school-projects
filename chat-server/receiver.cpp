/*
 * Client program that logs in as receiver and prints delivered messages from server
 * CSF Assignment 5
 * V. Ching
 * vching1@jhu.edu
 * S. Sayyad
 * ssayyad1@jhu.edu
 */

#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  std::string server_hostname = argv[1];
  int server_port = std::stoi(argv[2]);
  std::string username = argv[3];
  std::string room_name = argv[4];

  Connection conn;

  // TODO: connect to server
  conn.connect(server_hostname, server_port);
  if(!conn.is_open()){
    std::cerr << "Failed to connect to server\n";
    return 1;
  }
  // TODO: send rlogin and join messages (expect a response from
  //       the server for each one)
  Message rlogin_msg(TAG_RLOGIN, username);
  if(!conn.send(rlogin_msg)){
    std::cerr <<"Connection error\n";
    return 1;
  }
  Message reply;
  if(!conn.receive(reply)){
    std::cerr << "Connection error\n";
    return 1;
  }
  if(reply.tag == TAG_ERR){
    std::cerr << reply.data << "\n";
    return 1;
  }
  Message join_msg(TAG_JOIN, room_name);
  if(!conn.send(join_msg)){
    std::cerr << "Connection error\n";
    return 1;
  }
  if(!conn.receive(reply)){
    std::cerr << "Connection error\n";
    return 1;
  }
  if(reply.tag == TAG_ERR){
    std::cerr << reply.data << "\n";
    return 1;
  }

  // TODO: loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)
  while(true){
    Message incoming;
    if(!conn.receive(incoming)){
      return 1;
    }
    if(incoming.tag == TAG_DELIVERY){
      std::string sender, text;
      if(parse_delivery(incoming.data, sender, text)){
        std::cout << sender << ": " << text << "\n";
      }
    } 
    else if(incoming.tag == TAG_ERR){
      std::cerr << incoming.data << "\n";
    }
  }

  return 0;
}
