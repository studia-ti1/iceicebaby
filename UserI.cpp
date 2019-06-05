#include <Ice/Ice.h>
#include <Chat.h>
#include <TerminalLog.h>
#include "UserI.h"

using namespace std;
using namespace Chat;

UserI::UserI(const Ice::CommunicatorPtr&, const string& name) {
  this->name = name;
};

string UserI::getName(const Ice::Current&) {
  return this->name;
};

void UserI::receiveText(const string& msg, const UserPrx& sender, const string& groupName, const Ice::Current&) {
  const string senderName = sender -> getName();

  logGroup(senderName, groupName, msg);
};

void UserI::receivePrivateText(const string& msg, const UserPrx& sender, const Ice::Current&) {
  const string senderName = sender -> getName();

  logPrivate(senderName, msg);
};
