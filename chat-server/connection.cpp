/*
 * Provides a buffered connection wrapper for sending and receiving messages
 * CSF Assignment 5
 * V. Ching
 * vching1@jhu.edu
 * S. Sayyad
 * ssayyad1@jhu.edu
 */

#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"

Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  // TODO: call rio_readinitb to initialize the rio_t object
  if(m_fd >= 0){
    rio_readinitb(&m_fdbuf, m_fd);
  }
}

void Connection::connect(const std::string &hostname, int port) {
  // TODO: call open_clientfd to connect to the server
  std::string port_str = std::to_string(port);
  m_fd = open_clientfd((char*)hostname.c_str(), (char*)port_str.c_str());
  if (m_fd < 0) {
    m_last_result = EOF_OR_ERROR;
    return;
  }
  // TODO: call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, m_fd);
  m_last_result = SUCCESS;
}

Connection::~Connection() {
  // TODO: close the socket if it is open
  if(m_fd >= 0){
    Close(m_fd);
  }
}

bool Connection::is_open() const {
  // TODO: return true if the connection is open
  return m_fd >= 0;
}

void Connection::close() {
  // TODO: close the connection if it is open
  if(m_fd >= 0){
    Close(m_fd);
    m_fd = -1;
  }
}

bool Connection::send(const Message &msg) {
  // TODO: send a message
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  if(m_fd < 0){
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  std::string out = msg.to_string();
  ssize_t rc = rio_writen(m_fd, out.c_str(), out.size());
  if(rc != (ssize_t)out.size()){
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  m_last_result = SUCCESS;
  return true;
}

bool Connection::receive(Message &msg) {
  // TODO: receive a message, storing its tag and data in msg
  // return true if successful, false if not
  // make sure that m_last_result is set appropriately
  if(m_fd < 0){
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  char buf[MAXLINE];
  ssize_t rc = rio_readlineb(&m_fdbuf, buf, MAXLINE);
  if(rc <= 0){
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  std::string line(buf);
  if(!msg.parse(line)){
    m_last_result = INVALID_MSG;
    return false;
  }
  m_last_result = SUCCESS;
  return true;
}
