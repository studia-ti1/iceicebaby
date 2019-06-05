module Chat {
  // Interfaces
  interface CoreServer;
  interface GroupServer;
  interface GroupServerManager;
  interface User;

  // Exceptions
  exception NameDoesNotExist{};
  exception NameAlreadyExists{};
  exception ServerAlreadyRegistered{};
  exception ServerDoesNotExist{};
  exception UserAlreadyRegistered{};
  exception UserDoesNotExist{};
  exception NoPermission{};

  // Sequences, enums, structs etc.
  sequence<GroupServerManager*> GroupManagers;
  sequence<GroupServer*> Groups;
  sequence<User*> Users;

  // Interfaces methods
  interface CoreServer {
  	User* getUserByName(string name) throws UserDoesNotExist;
  	Groups listGroups();
  	GroupServer* getGroupServerByName(string name) throws NameDoesNotExist;
    void logIn(User* user) throws UserAlreadyRegistered;
    void logOut(User* user) throws UserDoesNotExist;
  	void createGroup(string name, User* requester) throws NameAlreadyExists;
  	void deleteGroup(string name, User* requester) throws NameDoesNotExist, NoPermission;
  	void registerServer(GroupServerManager* serverManager) throws ServerAlreadyRegistered;
  	void unregisterServer(GroupServerManager* serverManager) throws ServerDoesNotExist;
  };

  interface GroupServer {
    string getName();
    string getAdminName();
    Users listUsers();
    void join(User* user)  throws UserAlreadyRegistered;
  	void leave(User* user) throws UserDoesNotExist;
  	void sendMessage(string message, User* sender) throws UserDoesNotExist;
  };

  interface GroupServerManager {
    int getGroupsCount();
    string getName();
    Groups listGroups();
  	GroupServer* getGroupServerByName(string name) throws NameDoesNotExist;
    GroupServer* createGroup(string name, User* requester) throws NameAlreadyExists;
  	void deleteGroup(string name, User* requester) throws NameDoesNotExist, NoPermission;
  };

  interface User {
    string getName();
  	void receiveText(string msg, User* sender, string groupName);
  	void receivePrivateText(string msg, User* sender);
  };
};
