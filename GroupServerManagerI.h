#ifndef __GroupServerManagerI_h__
#define __GroupServerManagerI_h__

#include <Ice/Ice.h>
#include <Chat.h>

using namespace std;
using namespace Chat;

class GroupServerManagerI : public GroupServerManager {
  private:
    int groupsCount;
    string managerName;
    Groups registeredGroups;
    Ice::CommunicatorPtr _communicator;
    Ice::ObjectAdapterPtr _adapter;
    void registerGroup(const GroupServerPrx& group);
    void unregisterGroup(const GroupServerPrx& group);
    void incCounter();
    void decCounter();
    GroupServerPrx createGroupProxy(const string& groupName, const UserPrx& requester);
    Ice::CommunicatorPtr communicator();
    Ice::ObjectAdapterPtr adapter();
  public:
    GroupServerManagerI(const Ice::CommunicatorPtr&, const string& managerName);
    virtual int getGroupsCount(const Ice::Current&);
    virtual string getName(const Ice::Current&);
    virtual Groups listGroups(const Ice::Current&);
    virtual GroupServerPrx getGroupServerByName(const string& groupName, const Ice::Current&);
    virtual GroupServerPrx createGroup(const string& groupName, const UserPrx& requester, const Ice::Current&) throw (NameAlreadyExists);
    virtual void deleteGroup(const string& groupName, const UserPrx& requester, const Ice::Current&) throw (NameDoesNotExist, NoPermission);
};

#endif
