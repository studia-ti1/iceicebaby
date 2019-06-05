#ifndef __Userable_h__
#define __Userable_h__

#include <Ice/Ice.h>
#include <Chat.h>

using namespace std;
using namespace Chat;

class Userable {
  protected:
    Users users;
    virtual void addUser(const UserPrx& user);
    virtual void removeUser(const UserPrx& user);
};

#endif
