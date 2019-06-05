#ifndef __UserI_h__
#define __UserI_h__

#include <Ice/Ice.h>
#include <Chat.h>

using namespace std;
using namespace Chat;

class UserI : public User {
  private:
    string name;
  public:
    UserI(const Ice::CommunicatorPtr&, const string& name);
    virtual string getName(const Ice::Current&);
    virtual void receiveText(const string& msg, const UserPrx& sender, const string& groupName, const Ice::Current&);
  	virtual void receivePrivateText(const string& msg, const UserPrx& sender, const Ice::Current&);
};

#endif
