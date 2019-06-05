#ifndef __GroupServerI_h__
#define __GroupServerI_h__

#include <Ice/Ice.h>
#include <Chat.h>
#include "Userable.h"

using namespace std;
using namespace Chat;

class GroupServerI : public GroupServer, public Userable {
  private:
    string name;
    UserPrx admin;
  public:
    GroupServerI(const Ice::CommunicatorPtr&, const string& name, const UserPrx& admin);
    virtual string getName(const Ice::Current&);
    virtual string getAdminName(const Ice::Current&);
    virtual Users listUsers(const Ice::Current&);
    virtual void join(const UserPrx& user, const Ice::Current&) throw (UserAlreadyRegistered);
    virtual void leave(const UserPrx& user, const Ice::Current&) throw (UserDoesNotExist);
    virtual void sendMessage(const string& msg, const UserPrx& sender, const Ice::Current& c) throw (UserDoesNotExist);
};

#endif
