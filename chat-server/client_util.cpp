/*
 * Utility helpers used by both sender and receiver for common client-side parsing tasks
 * CSF Assignment 5
 * V. Ching
 * vching1@jhu.edu
 * S. Sayyad
 * ssayyad1@jhu.edu
 */

#include <iostream>
#include <string>
#include "connection.h"
#include "message.h"
#include "client_util.h"

// string trim functions shamelessly stolen from
// https://www.techiedelight.com/trim-string-cpp-remove-leading-trailing-spaces/

const std::string WHITESPACE = " \n\r\t\f\v";

std::string ltrim(const std::string &s) {
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}
 
std::string rtrim(const std::string &s) {
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}
 
std::string trim(const std::string &s) {
  return rtrim(ltrim(s));
}

bool parse_delivery(const std::string &data, std::string &sender, std::string &text) {
    std::size_t first = data.find(':');
    if (first == std::string::npos) {
        return false;
    }

    std::size_t second = data.find(':', first + 1);
    if (second == std::string::npos) {
        return false;
    }

    sender = data.substr(first + 1, second - first - 1);
    text   = data.substr(second + 1);
    return true;
}

