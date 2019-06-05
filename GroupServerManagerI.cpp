#include <Ice/Ice.h>
#include <Chat.h>
#include <TerminalLog.h>
#include "GroupServerManagerI.h"
#include "GroupServerI.h"

using namespace std;
using namespace Chat;

GroupServerManagerI::GroupServerManagerI(const Ice::CommunicatorPtr& communicator, const string& managerName) {
  this->managerName = managerName;
  this->_communicator = communicator;
  this->groupsCount = 0;
  this->_adapter = communicator ->
    createObjectAdapterWithEndpoints(
      managerName + "GSMendpoint", "default"
    );
};

void GroupServerManagerI::registerGroup(const GroupServerPrx& group) {
  this->registeredGroups.push_back(group);
  incCounter();
};

void GroupServerManagerI::unregisterGroup(const GroupServerPrx& group) {
  this->registeredGroups.erase(
    remove(
      this->registeredGroups.begin(),
      this->registeredGroups.end(),
      group
    ),
    this->registeredGroups.end()
  );
  decCounter();
};

void GroupServerManagerI::incCounter() {
  this->groupsCount++;
};

void GroupServerManagerI::decCounter() {
  this->groupsCount--;
};

Ice::ObjectAdapterPtr GroupServerManagerI::adapter() {
  return this->_adapter;
};

int GroupServerManagerI::getGroupsCount(const Ice::Current&) {
  return this->groupsCount;
};

Ice::CommunicatorPtr GroupServerManagerI::communicator() {
  return this->_communicator;
};

GroupServerPrx GroupServerManagerI::createGroupProxy(const string& groupName, const UserPrx& requester) {
  try {
    GroupServerPtr groupPtr = new GroupServerI(communicator(), groupName, requester);

    adapter() -> add(
      groupPtr,
      communicator() -> stringToIdentity(groupPtr -> getName() + "GSM")
    );
    adapter() -> activate();

    Ice::ObjectPrx groupObjectProxy = adapter() ->
      createProxy(
        communicator() -> stringToIdentity(groupPtr -> getName() + "GSM")
      );

    GroupServerPrx groupServer = GroupServerPrx::checkedCast(groupObjectProxy);

    registerGroup(groupServer);

    return groupServer;
  } catch(const Ice::AlreadyRegisteredException) {
    logError("Adapter already exist");
  } catch(const Ice::Exception& er) {
    cerr << er << endl;
  }
};

string GroupServerManagerI::getName(const Ice::Current&) {
  return this->managerName;
};

Groups GroupServerManagerI::listGroups(const Ice::Current&) {
  return this->registeredGroups;
};

GroupServerPrx GroupServerManagerI::getGroupServerByName(const string& groupName, const Ice::Current&) {
  unsigned int groupsIterator = 0;
  unsigned int groupsSize = this->registeredGroups.size();
  GroupServerPrx groupServer;

  for (groupsIterator; groupsIterator < groupsSize; groupsIterator++) {
    if (this->registeredGroups[groupsIterator] -> getName() == groupName) {
      groupServer = this->registeredGroups[groupsIterator];
    }
  }

  return groupServer;
};

GroupServerPrx GroupServerManagerI::createGroup(const string& groupName, const UserPrx& requester, const Ice::Current&) throw (NameAlreadyExists) {
  unsigned int groupsIterator = 0;
  unsigned int groupsSize = this->registeredGroups.size();
  bool isValid = true;

  for (groupsIterator; groupsIterator < groupsSize; groupsIterator++) {
    if (this->registeredGroups[groupsIterator] -> getName() == groupName) {
      isValid = false;
    }
  }

  if (isValid) {
    return createGroupProxy(groupName, requester);
  } else {
    throw NameAlreadyExists();
  }
};

void GroupServerManagerI::deleteGroup(const string& groupName, const UserPrx& requester, const Ice::Current&) throw (NameDoesNotExist, NoPermission) {
  unsigned int groupsIterator = 0;
  unsigned int groupsSize = this->registeredGroups.size();
  GroupServerPrx groupProxy;
  bool isValid = false;

  for (groupsIterator; groupsIterator < groupsSize; groupsIterator++) {
    if (this->registeredGroups[groupsIterator] -> getName() == groupName) {
      isValid = true;
      groupProxy = this->registeredGroups[groupsIterator];
    }
  }

  if (isValid) {
    if (groupProxy -> getAdminName() == requester -> getName()) {
      try {
        unregisterGroup(groupProxy);
        adapter() -> remove(groupProxy->ice_getIdentity());
      } catch (const Ice::Exception& er) {
        cerr << er << endl;
      }
    } else {
      throw NoPermission();
    }
  } else {
    throw NameDoesNotExist();
  }
};
