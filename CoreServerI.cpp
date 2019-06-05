#include <Ice/Ice.h>
#include <Chat.h>

#include "TerminalLog.h"
#include "CoreServerI.h"
#include "Userable.h"

using namespace std;
using namespace Chat;

CoreServerI::CoreServerI(const Ice::CommunicatorPtr& communicator) {
  this->communicator = communicator;
};

void CoreServerI::addServer(const GroupServerManagerPrx& serverManager) {
  this->groupManagers.push_back(serverManager);
};

void CoreServerI::removeServer(const GroupServerManagerPrx& serverManager) {
  this->groupManagers.erase(
    remove(
      this->groupManagers.begin(),
      this->groupManagers.end(),
      serverManager
    ),
    this->groupManagers.end()
  );
};

UserPrx CoreServerI::getUserByName(const string& name, const Ice::Current&) throw (UserDoesNotExist) {
  unsigned int usersIterator = 0;
  unsigned int usersSize = this->users.size();
  UserPrx currentUser;

  for (usersIterator; usersIterator < usersSize; usersIterator++) {
    if (this->users[usersIterator] -> getName() == name) {
      currentUser = this->users[usersIterator];
    }
  }

  if (currentUser) {
    logInfo("Requested user " + name + " has been returned.");

    return currentUser;
  } else {
    logError("Requested user " + name + " does not exist.");

    throw UserDoesNotExist();
  }
};

Groups CoreServerI::listGroups(const Ice::Current&) {
  unsigned int groupManagersIterator = 0;
  unsigned int groupManagersSize = this->groupManagers.size();
  Groups groupsList;

  for (groupManagersIterator; groupManagersIterator < groupManagersSize; groupManagersIterator++) {
    Groups groupsManagerGroupsList = this->groupManagers[groupManagersIterator] -> listGroups();

    groupsList.insert(
      groupsList.end(),
      groupsManagerGroupsList.begin(),
      groupsManagerGroupsList.end()
    );
  }

  logInfo("Requested list groups has been returned.");

  return groupsList;
};

GroupServerPrx CoreServerI::getGroupServerByName(const string& groupName, const Ice::Current&) throw (NameDoesNotExist) {
  unsigned int groupManagersIterator = 0;
  unsigned int groupManagersSize = this->groupManagers.size();
  GroupServerPrx groupServer;

  if (groupManagersSize == 0) {
    logError("Requested non-existing group " + groupName + ".");

    throw NameDoesNotExist();
  }

  for (groupManagersIterator; groupManagersIterator < groupManagersSize; groupManagersIterator++) {
    if (!groupServer) {
      groupServer = this->groupManagers[groupManagersIterator] -> getGroupServerByName(groupName);
    }
  }

  if (groupServer) {
    logInfo("Returned " + groupName + " group proxy.");

    return groupServer;
  } else {
    logError("Requested non-existing group " + groupName + ".");

    throw NameDoesNotExist();
  }
};

void CoreServerI::logIn(const UserPrx& user, const Ice::Current&) throw (UserAlreadyRegistered) {
  unsigned int usersIterator = 0;
  unsigned int usersSize = this->users.size();
  const string userName = user -> getName();

  if (usersSize == 0) {
    logInfo("User " + userName + " has successfuly logged in.");

    addUser(user);
  } else {
    for (usersIterator; usersIterator < usersSize; usersIterator++) {
      if (this->users[usersIterator] -> getName() == userName) {
        logError("User " + userName + " has already logged in.");

        throw UserAlreadyRegistered();
      }
    }

    logInfo("User " + userName + " has successfuly logged in.");

    addUser(user);
  }
};

void CoreServerI::logOut(const UserPrx& user, const Ice::Current&) throw (UserDoesNotExist) {
  const string userName = user -> getName();
  unsigned int usersIterator = 0;
  unsigned int usersSize = this->users.size();
  bool isValid = false;

  if (usersSize == 0) {
    logError("User " + userName + " does not exist.");

    throw UserDoesNotExist();
  } else {
    for (usersIterator; usersIterator < usersSize; usersIterator++) {
      if (this->users[usersIterator] -> getName() == userName) {
        logInfo("User " + userName + " has successfuly logged out.");

        isValid = true;
        removeUser(user);
      }
    }

    if (!isValid) {
      logError("User " + userName + " does not exist.");

      throw UserDoesNotExist();
    }
  }
};

void CoreServerI::createGroup(const string& name, const UserPrx& requester, const Ice::Current&) throw (NameAlreadyExists) {
  unsigned int groupManagersIterator = 0;
  unsigned int groupManagersSize = this->groupManagers.size();
  GroupServerManagerPrx groupManagerPrx;
  GroupServerPrx groupPrx;

  for (groupManagersIterator; groupManagersIterator < groupManagersSize; groupManagersIterator++) {
    if (!groupManagerPrx) {
      groupManagerPrx = this->groupManagers[groupManagersIterator];
    }

    if (this->groupManagers[groupManagersIterator] -> getGroupsCount() < groupManagerPrx -> getGroupsCount()) {
      groupManagerPrx = this->groupManagers[groupManagersIterator];
    }

    if (!groupPrx) {
      groupPrx = this->groupManagers[groupManagersIterator] -> getGroupServerByName(name);
      break;
    }
  }

  if (groupPrx) {
    logError("Group " + name + " has already exist.");

    throw NameAlreadyExists();
  } else {
    try {
      groupManagerPrx -> createGroup(name, requester);

      logInfo("Group " + name + " has been created.");
    } catch (const Ice::Exception& ex) {
      cerr << ex << endl;
    }
  }
};

void CoreServerI::deleteGroup(const string& name, const UserPrx& requester, const Ice::Current&) throw (NameDoesNotExist, NoPermission) {
  unsigned int groupManagersIterator = 0;
  unsigned int groupManagersSize = this->groupManagers.size();
  GroupServerManagerPrx groupManagerPrx;

  for (groupManagersIterator; groupManagersIterator < groupManagersSize; groupManagersIterator++) {
    GroupServerPrx groupPrx = this->groupManagers[groupManagersIterator] -> getGroupServerByName(name);

    if (groupPrx) {
      groupManagerPrx = this->groupManagers[groupManagersIterator];
    }
  }

  if (groupManagerPrx && name != "DefaultGroup") {
    try {
      groupManagerPrx -> deleteGroup(name, requester);

      logInfo("Group " + name + " has been deleted");
    } catch (const Ice::Exception& ex) {
      throw NoPermission();
    }
  } else if (name == "DefaultGroup") {
    logError("Unauthorized attempt to delete default group.");

    throw NoPermission();
  } else {
    logError("Group " + name + " does not exists.");

    throw NameDoesNotExist();
  }
};

void CoreServerI::registerServer(const GroupServerManagerPrx& serverManager, const Ice::Current&) throw (ServerAlreadyRegistered) {
  unsigned int groupManagersIterator = 0;
  unsigned int groupManagersSize = this->groupManagers.size();
  const string serverManagerName = serverManager -> getName();
  bool isValid = true;

  if (groupManagersSize == 0) {
    logInfo("Server " + serverManagerName + " has been registered.");

    addServer(serverManager);
  } else {
    for (groupManagersIterator; groupManagersIterator < groupManagersSize; groupManagersIterator++) {
      if (this->groupManagers[groupManagersIterator] -> getName() == serverManagerName) {
        logError("Server " + serverManagerName + " has been already registered.");
        isValid = false;

        throw ServerAlreadyRegistered();
      }
    }

    if (isValid) {
      logInfo("Server " + serverManagerName + " has been registered.");

      addServer(serverManager);
    }
  }
};

void CoreServerI::unregisterServer(const GroupServerManagerPrx& serverManager, const Ice::Current&) throw (ServerDoesNotExist) {
  unsigned int groupManagersIterator = 0;
  unsigned int groupManagersSize = this->groupManagers.size();
  const string serverManagerName = serverManager -> getName();
  bool isValid = false;

  if (groupManagersSize == 0) {
    logError("Server " + serverManagerName + " does not exist.");

    throw ServerDoesNotExist();
  } else {
    for (groupManagersIterator; groupManagersIterator < groupManagersSize; groupManagersIterator++) {
      if (this->groupManagers[groupManagersIterator] -> getName() == serverManagerName) {
        logInfo("Server " + serverManagerName + " has been unregistered.");

        isValid = true;
        removeServer(serverManager);
      }
    }

    if (!isValid) {
      logError("Server " + serverManagerName + " does not exist.");

      throw ServerDoesNotExist();
    }
  }
};
