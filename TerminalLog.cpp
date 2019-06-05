#include <TerminalLog.h>

#include <iostream>
#include <string>
#include <ctime>

void logClean(const std::string& msg) {
  std::cout << msg;
};

void log(const std::string& msg) {
  char buffer[80];
  time_t t = std::time(NULL);
  struct tm *timeinfo = std::localtime(&t);
  std::strftime(buffer, 80, "%d-%m-%Y %H:%M:%S ", timeinfo);

  std::cout << buffer << msg << std::endl;
};

void logPrivate(const std::string& senderName, const std::string& msg) {
  char buffer[80];
  time_t t = std::time(NULL);
  struct tm *timeinfo = std::localtime(&t);
  std::strftime(buffer, 80, "%d-%m-%Y %H:%M:%S ", timeinfo);

  std::cout << "\033[1;35m" << buffer << "Private msg from " << senderName << " ### " << msg << "\033[0m\n";
}

void logGroup(const std::string& senderName, const std::string& groupName, const std::string& msg) {
  char buffer[80];
  time_t t = std::time(NULL);
  struct tm *timeinfo = std::localtime(&t);
  std::strftime(buffer, 80, "%d-%m-%Y %H:%M:%S ", timeinfo);

  std::cout << "\033[1;32m" << buffer << "Group: " << groupName << ", User: " << senderName << " ### " << msg << "\033[0m\n";
};

void logInfo(const std::string& msg) {
  char buffer[80];
  time_t t = std::time(NULL);
  struct tm *timeinfo = std::localtime(&t);\
  std::strftime(buffer, 80, "%d-%m-%Y %H:%M:%S ", timeinfo);

  std::cout << "\033[1;33m" << buffer << msg << "\033[0m\n";
};

void logError(const std::string& msg) {
  char buffer[80];
  time_t t = std::time(NULL);
  struct tm *timeinfo = std::localtime(&t);\
  std::strftime(buffer, 80, "%d-%m-%Y %H:%M:%S ", timeinfo);

  std::cout << "\033[1;31m" << buffer << msg << "\033[0m\n";
};
