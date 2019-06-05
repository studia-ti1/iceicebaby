#ifndef __TerminalLog_h__
#define __TerminalLog_h__

#include <iostream>
#include <string>
#include <iomanip>
#include <ctime>

void logClean(const std::string& msg);
void log(const std::string& msg);
void logPrivate(const std::string& senderName, const std::string& msg);
void logGroup(const std::string& senderName, const std::string& groupName, const std::string& msg);
void logInfo(const std::string& msg);
void logError(const std::string& msg);

#endif
