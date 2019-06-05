#include <Ice/Ice.h>
#include <Chat.h>
#include <TerminalLog.h>
#include <GroupServerManagerI.h>
#include <GroupServerI.h>
#include <CoreServerI.h>

using namespace std;
using namespace Chat;

class ChatServerApplication : virtual public Ice::Application {
  private:
    CoreServerPtr coreServer;
    GroupServerManagerPrx defaultManager;

    void startCoreServer() {
      logInfo("Starting chat core server.");
      sleep(3);

      CoreServerPtr coreServerInstance = new CoreServerI(communicator());

      Ice::ObjectAdapterPtr coreServerAdapter = communicator()
        -> createObjectAdapterWithEndpoints(
          "CoreServerAdapter",
          "default -p 9197"
        );

      coreServerAdapter
        -> add(
          coreServerInstance,
          communicator() -> stringToIdentity("CoreServer")
        );

      coreServerAdapter -> activate();
      this->coreServer = coreServerInstance;

      logInfo("Core server has successfuly started.");
    };
    void startDefaultServerManager() {
      logInfo("Starting default server manager.");
      sleep(3);

      GroupServerManagerPtr managerPtr = new GroupServerManagerI(communicator(), "DefaultGroupManager");

      Ice::ObjectAdapterPtr adapter = communicator()
        -> createObjectAdapterWithEndpoints(
          managerPtr -> getName() + "Adapter", "default"
        );

      adapter -> add(
        managerPtr,
        communicator() -> stringToIdentity(managerPtr -> getName())
      );
      adapter -> activate();

      Ice::ObjectPrx managerObjectPrx = adapter -> createProxy(
        communicator() -> stringToIdentity(managerPtr -> getName())
      );
      GroupServerManagerPrx managerPrx = GroupServerManagerPrx::checkedCast(managerObjectPrx);

      this->defaultManager = managerPrx;

      try {
        UserPrx userProxy;
        this->coreServer -> registerServer(managerPrx);
        this->coreServer -> createGroup("DefaultGroup", userProxy);
      } catch (const Ice::Exception& ex) {
        cerr << ex << endl;
      }
    };
  public:
    virtual int run(int, char*[]) {
      startCoreServer();
      startDefaultServerManager();

      communicator() -> waitForShutdown();

      if (interrupted())
        cerr << appName() << ": terminating" << endl;
      return 0;
    }
};

int main(int argc, char* argv[]) {
  ChatServerApplication csApp;

  return csApp.main(argc, argv);
};
