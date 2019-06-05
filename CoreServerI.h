#ifndef __CoreServerI_h__
#define __CoreServerI_h__

#include <Ice/Ice.h>
#include <Chat.h>

#include "Userable.h"

using namespace std;
using namespace Chat;

class CoreServerI : public CoreServer, public Userable {
  private:
    GroupManagers groupManagers;
    Ice::CommunicatorPtr communicator;
    virtual void addServer(const GroupServerManagerPrx& serverManager);
    virtual void removeServer(const GroupServerManagerPrx& serverManager);
  public:
    CoreServerI(const Ice::CommunicatorPtr&);
    virtual UserPrx getUserByName(const string& name, const Ice::Current&) throw (UserDoesNotExist);
  	virtual Groups listGroups(const Ice::Current&);
  	virtual GroupServerPrx getGroupServerByName(const string& groupName, const Ice::Current&) throw (NameDoesNotExist);
    virtual void logIn(const UserPrx& user, const Ice::Current&) throw (UserAlreadyRegistered);
    virtual void logOut(const UserPrx& user, const Ice::Current&) throw (UserDoesNotExist);
  	virtual void createGroup(const string& name, const UserPrx& requester, const Ice::Current&) throw (NameAlreadyExists);
  	virtual void deleteGroup(const string& name, const UserPrx& requester, const Ice::Current&) throw (NameDoesNotExist, NoPermission);
  	virtual void registerServer(const GroupServerManagerPrx& serverManager, const Ice::Current&) throw (ServerAlreadyRegistered);
  	virtual void unregisterServer(const GroupServerManagerPrx& serverManager, const Ice::Current&) throw (ServerDoesNotExist);
};

#endif
