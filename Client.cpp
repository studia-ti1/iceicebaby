#include <Ice/Ice.h>
#include <Chat.h>

#include <string>
#include <sstream>
#include <algorithm>
#include <iterator>
#include <vector>
#include <stdio.h>
#include <stdlib.h>

#include "TerminalLog.h"
#include "UserI.h"
#include "GroupServerManagerI.h"
#include "GroupServerI.h"

using namespace std;
using namespace Chat;

template<typename Out>
void split(const string &s, char delim, Out result) {
    stringstream ss;
    ss.str(s);
    string item;
    while (getline(ss, item, delim)) {
        *(result++) = item;
    }
}

vector<string> split(const string &s, char delim) {
    vector<string> elems;
    split(s, delim, back_inserter(elems));
    return elems;
}

class ChatClientApplication : virtual public Ice::Application {
  private:
    CoreServerPrx _coreServerPrx;
    UserPrx _currentUserPrx;
    GroupServerManagerPrx _currentManagerPrx;
    GroupServerPrx _currentGroup;
    vector<string> _createdGroupsNames;

    void setServerProxy(const CoreServerPrx& server) {
      this->_coreServerPrx = server;
    };

    void setUserProxy(const UserPrx& user) {
      this->_currentUserPrx = user;
    };

    void setManagerProxy(const GroupServerManagerPrx& manager) {
      this->_currentManagerPrx = manager;
    };

    void setCurrentGroup(const GroupServerPrx& group) {
      this->_currentGroup = group;
    };

    void pushCreatedGroup(const string& groupName) {
      createdGroups().push_back(groupName);
    };

    void removeCreatedGroup(const string& groupName) {
      createdGroups().erase(
        remove(
          createdGroups().begin(),
          createdGroups().end(),
          groupName
        ),
        createdGroups().end()
      );
    };

    CoreServerPrx serverProxy() {
      return this->_coreServerPrx;
    };

    UserPrx userProxy() {
      return this->_currentUserPrx;
    };

    GroupServerManagerPrx managerProxy() {
      return this->_currentManagerPrx;
    };

    GroupServerPrx groupProxy() {
      return this->_currentGroup;
    };

    vector<string> createdGroups() {
      return this->_createdGroupsNames;
    };

    void clearGroups() {
      for (int iterator = 0; iterator < createdGroups().size(); iterator++) {
        bool groupDeleted = false;
        string currentName = createdGroups()[iterator];
        try {
          serverProxy() -> deleteGroup(currentName, userProxy());
          groupDeleted = true;
        } catch (const NameDoesNotExist) {
          logError("Group named " + currentName + " does not exist.");
          groupDeleted = false;
        } catch (const NoPermission) {
          logError("You have no permission to delete this group.");
          groupDeleted = false;
        } catch (const Ice::Exception& ex) {
          cerr << ex << endl;
          groupDeleted = false;
        }

        if (groupDeleted) {
          logInfo("Group named " + currentName + " has been deleted.");
        }
      }
    };

    void logOut() {
      bool isValid = true;

      try {
        if (managerProxy()) {
          serverProxy() -> unregisterServer(managerProxy());
        }
        if (createdGroups().size() > 0) {
          clearGroups();
        }
        serverProxy() -> logOut(userProxy());
      } catch (const Ice::Exception& e) {
        cerr << e << endl;
        isValid = false;
      }

      if (isValid) {
        logInfo("Background server manager successfuly stoped.");
      }
    };

    void connectToCoreServer() {
      setServerProxy(
        CoreServerPrx::checkedCast(
          communicator() -> stringToProxy("CoreServer:default -p 9197")
        )
      );
      logInfo("Connected to the core server.");
    };

    void userCreator() {
      UserPrx userCandidate;
      string nameCandidate;
      string repeatAgain = "yes";
      bool isValid = true;

      while (repeatAgain == "yes") {
        repeatAgain = "no";

        log("Enter your username:");
        cout << "# ";
        cin >> nameCandidate;

        UserPtr user = new UserI(communicator(), nameCandidate);
        Ice::ObjectAdapterPtr adapter = communicator()
          -> createObjectAdapterWithEndpoints(
            user -> getName() + "Adapter", "default"
          );
        adapter -> add(
          user,
          communicator() -> stringToIdentity(user -> getName() + "CL")
        );
        adapter -> activate();
        Ice::ObjectPrx userObjectPrx = adapter -> createProxy(
          communicator() -> stringToIdentity(user -> getName() + "CL")
        );

        userCandidate = UserPrx::checkedCast(userObjectPrx);

        try {
          serverProxy() -> logIn(userCandidate);
          isValid = true;
        } catch (const Ice::Exception& ex) {
          logError("User with " + nameCandidate + " username has already logged in.");
          log("Do you want to enter new username? type: yes/no");
          cout << "# ";
          cin >> repeatAgain;
          isValid = false;
          if (repeatAgain == "yes") {
            adapter -> destroy();
          }
        }
      }

      if (isValid) {
        logInfo("Successfuly logged in.");

        setUserProxy(userCandidate);
        callbackOnInterrupt();
      } else {
        logError("Press ctrl+c to terminate.");
      }
    };

    void startServerManager() {
      logInfo("Starting background server manager.");
      sleep(3);

      GroupServerManagerPtr managerPtr = new GroupServerManagerI(communicator(), userProxy() -> getName() + "CGSM");

      Ice::ObjectAdapterPtr adapter = communicator()
        -> createObjectAdapterWithEndpoints(
          managerPtr -> getName() + "Adapter", "default"
        );

      adapter -> add(
        managerPtr,
        communicator() -> stringToIdentity(managerPtr -> getName() + "CGSM")
      );
      adapter -> activate();

      Ice::ObjectPrx managerObjectPrx = adapter -> createProxy(
        communicator() -> stringToIdentity(managerPtr -> getName() + "CGSM")
      );
      GroupServerManagerPrx managerPrx = GroupServerManagerPrx::checkedCast(managerObjectPrx);

      bool isValid = true;

      try {
        serverProxy() -> registerServer(managerPrx);
        setManagerProxy(managerPrx);
      } catch (const ServerAlreadyRegistered) {
        logError("Server has been already registered.");
        isValid = false;
      } catch (const Ice::Exception& ex) {
        cerr << ex << endl;
        isValid = false;
      }

      if (isValid) {
        logInfo("Background server manager successfuly started.");
      }
    };

    void commandsHelper() {
      log("Sending messages as " + userProxy() -> getName());
      log("ID - description               # command");
      log(" 1 - display commands helper   # type: help");
      log(" 2 - list available groups     # type: listgroups");
      log(" 3 - join group                # type: joingroup groupname");
      log(" 4 - leave group               # type: leavegroup");
      log(" 5 - display group users       # type: groupusers");
      log(" 6 - send group chat message   # type: groupchat message");
      log(" 7 - send private chat message # type: privatechat recipient message");
      log(" 8 - create group              # type: creategroup groupname");
      log(" 9 - delete group              # type: deletegroup groupname");
      log("10 - close chat application    # type: exit");
    };

    void displayUI() {
      string userConsoleInput;

      commandsHelper();

      while (userConsoleInput != "exit") {
        getline(cin, userConsoleInput);

        vector<string> parsedConsoleInput = split(userConsoleInput, ' ');
        unsigned int parsedSize = parsedConsoleInput.size();
        string command, target, msg;

        if (parsedSize == 1) {
          command = parsedConsoleInput[0];

          if (command == "listgroups") {
            Groups groups = serverProxy() -> listGroups();

            for (int iterator = 0; iterator < groups.size(); iterator++) {
              const string groupName = groups[iterator] -> getName();

              if (iterator == groups.size() - 1) {
                logClean(groupName);
              } else {
                logClean(groupName + ", ");
              }
            }
            cout << endl << endl;
          } else if (command == "leavegroup") {
            bool groupLeaved = false;

            if (groupProxy()) {
              try {
                GroupServerPrx empty;
                groupProxy() -> leave(userProxy());
                setCurrentGroup(empty);
                groupLeaved = true;
              } catch (const Ice::Exception& ex) {
                cerr << ex << endl;
                groupLeaved = false;
              }

              if (groupLeaved) {
                logInfo("You have left the group.");
              }
            } else {
              logError("You have to join a group to leave it.");
            }
          } else if (command == "help") {
            commandsHelper();
          } else if (command == "exit") {
            logInfo("Press ctrl+c to shutdown chat application.");
          } else if (command == "groupusers") {
            if (groupProxy()) {
              Users groupUsers = groupProxy() -> listUsers();

              for (int iterator = 0; iterator < groupUsers.size(); iterator++) {
                const string userName = groupUsers[iterator] -> getName();

                if (iterator == groupUsers.size() - 1) {
                  logClean(userName);
                } else {
                  logClean(userName + ", ");
                }
              }
              cout << endl << endl;
            }
          }
        } else if (parsedSize > 1) {
          command = parsedConsoleInput[0];
          target = parsedConsoleInput[1];

          if (command == "joingroup") {
            bool groupJoined = false;

            if (groupProxy()) {
              try {
                GroupServerPrx empty;
                groupProxy() -> leave(userProxy());
                setCurrentGroup(empty);
              } catch (const Ice::Exception& ex) {
                cerr << ex << endl;
              }
            }

            try {
              GroupServerPrx pickedGroup = serverProxy() -> getGroupServerByName(target);
              pickedGroup -> join(userProxy());
              setCurrentGroup(pickedGroup);
              groupJoined = true;
            } catch (const NameDoesNotExist) {
              logError("Group named " + target + " does not exist.");
              groupJoined = false;
            } catch (const UserAlreadyRegistered) {
              logError("You have already joined that group.");
              groupJoined = false;
            } catch (const Ice::Exception& ex) {
              cerr << ex << endl;
              groupJoined = false;
            }

            if (groupJoined) {
              logInfo("You have joined " + target + ".");
            }
          } else if (command == "groupchat") {
            for (int iterator = 1; iterator < parsedSize; iterator++) {
              msg = msg + " " + parsedConsoleInput[iterator];
            }

            if (groupProxy()) {
              try {
                groupProxy() -> sendMessage(msg, userProxy());
              } catch (const Ice::Exception& ex) {
                cerr << ex << endl;
              }
            } else {
              logError("Join a group at first.");
            }
          } else if (command == "creategroup") {
            bool groupCreated = false;

            try {
              serverProxy() -> createGroup(target, userProxy());
              groupCreated = true;
            } catch (const NameAlreadyExists) {
              logError("Group named " + target + " already exists.");
              groupCreated = false;
            } catch (const Ice::Exception& ex) {
              cerr << ex << endl;
              groupCreated = false;
            }

            if (groupCreated) {
              logInfo("Group named " + target + " has been created.");

              pushCreatedGroup(target);
            }
          } else if (command == "deletegroup") {
            bool groupDeleted = false;

            try {
              serverProxy() -> deleteGroup(target, userProxy());
              groupDeleted = true;
            } catch (const NameDoesNotExist) {
              logError("Group named " + target + " does not exist.");
              groupDeleted = false;
            } catch (const NoPermission) {
              logError("You have no permission to delete this group.");
              groupDeleted = false;
            } catch (const Ice::Exception& ex) {
              cerr << ex << endl;
              groupDeleted = false;
            }

            if (groupDeleted) {
              logInfo("Group named " + target + " has been deleted.");

              removeCreatedGroup(target);
            }
          } else if (command == "privatechat") {
            for (int iterator = 2; iterator < parsedSize; iterator++) {
              msg = msg + " " + parsedConsoleInput[iterator];
            }

            UserPrx privateUser = serverProxy() -> getUserByName(target);

            privateUser -> receivePrivateText(msg, userProxy());
          }
        }
      }
    };

  public:
    virtual void interruptCallback(int signal) {
      bool isValid = true;

      try {
        if (userProxy()) {
          logOut();
        }
        communicator() -> destroy();
      } catch (const Ice::Exception& e) {
        cerr << e << endl;
        isValid = false;
      }

      if (isValid) {
        logInfo("Successfuly logged out.");
      }

      logInfo("Shutting down.");
    };

    virtual int run(int argc, char* argv[]) {
      string option;

      if (argc > 1) {
        option = argv[argc - 1];
      }

      try {
        connectToCoreServer();
        userCreator();
        if (userProxy() && option != "nomanager") {
          startServerManager();
        }
        displayUI();
      } catch (const Ice::Exception& ex) {
        cerr << ex << endl;
      }

      communicator() -> waitForShutdown();

      return 0;
    }
};

int main(int argc, char* argv[]) {
  ChatClientApplication ccApp;

  return ccApp.main(argc, argv);
};
