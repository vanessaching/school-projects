/*
 * Utility helpers used by both sender and receiver for common client-side parsing tasks
 * CSF Assignment 5
 * V. Ching
 * vching1@jhu.edu
 * S. Sayyad
 * ssayyad1@jhu.edu
 */

#ifndef CLIENT_UTIL_H
#define CLIENT_UTIL_H

#include <string>
class Connection;
struct Message;

// this header file is useful for any declarations for functions
// or classes that are used by both clients (the sender and receiver)

std::string ltrim(const std::string &s);
std::string rtrim(const std::string &s);
std::string trim(const std::string &s);

// you can add additional declarations here...
bool parse_delivery(const std::string &data, std::string &sender, std::string &text);

#endif // CLIENT_UTIL_H
