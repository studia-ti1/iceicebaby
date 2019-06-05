#include <Ice/Ice.h>
#include <Chat.h>
#include <TerminalLog.h>
#include "GroupServerI.h"

using namespace std;
using namespace Chat;

GroupServerI::GroupServerI(const Ice::CommunicatorPtr& communicator, const string& name, const UserPrx& admin) {
  this->name = name;
  this->admin = admin;
};

string GroupServerI::getName(const Ice::Current&) {
  return this->name;
};

string GroupServerI::getAdminName(const Ice::Current&) {
  return this->admin -> getName();
};

Users GroupServerI::listUsers(const Ice::Current&) {
  return this->users;
};

void GroupServerI::join(const UserPrx& user, const Ice::Current&) throw (UserAlreadyRegistered) {
  unsigned int usersIterator = 0;
  unsigned int usersCount = this->users.size();
  bool isValid = true;

  for (usersIterator; usersIterator < usersCount; usersIterator++) {
    if (user -> getName() == this->users[usersIterator] -> getName()) {
      isValid = false;
    }
  }

  if (isValid) {
    addUser(user);
  } else {
    throw UserAlreadyRegistered();
  }
};

void GroupServerI::leave(const UserPrx& user, const Ice::Current&) throw (UserDoesNotExist) {
  unsigned int usersIterator = 0;
  unsigned int usersCount = this->users.size();
  bool isValid = false;

  for (usersIterator; usersIterator < usersCount; usersIterator++) {
    if (user -> getName() == this->users[usersIterator] -> getName()) {
      isValid = true;
    }
  }

  if (isValid) {
    removeUser(user);
  } else {
    throw UserDoesNotExist();
  }
};

void GroupServerI::sendMessage(const string& msg, const UserPrx& sender, const Ice::Current&) throw (UserDoesNotExist) {
  unsigned int usersIterator = 0;
  unsigned int usersCount = this->users.size();
  string name = this->name;

  for (usersIterator; usersIterator < usersCount; usersIterator++) {
    this->users[usersIterator] -> receiveText(msg, sender, name);
  }
};
