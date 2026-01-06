/*
 * Client program that logs in as sender and transmits user commands and messages to server
 * CSF Assignment 5
 * V. Ching
 * vching1@jhu.edu
 * S. Sayyad
 * ssayyad1@jhu.edu
 */

#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  std::string server_hostname;
  int server_port;
  std::string username;

  server_hostname = argv[1];
  server_port = std::stoi(argv[2]);
  username = argv[3];

  // TODO: connect to server
  Connection conn;
  conn.connect(server_hostname, server_port);
  if(!conn.is_open()){
    std::cerr << "Failed to connect to server\n";
    return 1;
  }
  // TODO: send slogin message
Message slogin_msg(TAG_SLOGIN, username);
if(!conn.send(slogin_msg)){
  std::cerr << "Connection error\n";
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
  // TODO: loop reading commands from user, sending messages to
  //       server as appropriate
std::string line;
while(true){
  if(!std::getline(std::cin, line)){
    Message quit_msg(TAG_QUIT, "quitting");
    conn.send(quit_msg);
    conn.receive(reply);
    return 0;
  }
  if(line.size() > 0 && line[0] == '/'){
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;
    if(cmd == "/join"){
      std::string room;
      iss >> room;
      Message join_msg(TAG_JOIN, room);
      conn.send(join_msg);
      conn.receive(reply);
      if(reply.tag == TAG_ERR){
        std::cerr << reply.data << "\n";
      }
    }
    else if(cmd == "/leave"){
      Message leave_msg(TAG_LEAVE, "");
      conn.send(leave_msg);
      conn.receive(reply);
      if(reply.tag == TAG_ERR){
        std::cerr << reply.data << "\n";
      }
    }
    else if(cmd == "/quit"){
      Message quit_msg(TAG_QUIT, "quitting");
      conn.send(quit_msg);
      conn.receive(reply);
      if (reply.tag == TAG_ERR) {
        std::cerr << reply.data << "\n";
        return 1;
      }
      return 0;
    }
    else{
      std::cerr << "Unknown command\n";
    }
  }
  else{
    Message msg(TAG_SENDALL, line);
    conn.send(msg);
    conn.receive(reply);
    if(reply.tag == TAG_ERR){
      std::cerr << reply.data << "\n";
    }}}
 return 0;
}
