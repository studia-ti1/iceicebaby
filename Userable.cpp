#include <Ice/Ice.h>
#include <Chat.h>
#include "Userable.h"

using namespace std;
using namespace Chat;

void Userable::addUser(const UserPrx& user) {
  this->users.push_back(user);
};

void Userable::removeUser(const UserPrx& user) {
  this->users.erase(
    remove(
      this->users.begin(),
      this->users.end(),
      user
    ),
    this->users.end()
  );
};
