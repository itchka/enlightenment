#ifndef DBUS_PROXY_H
#define DBUS_PROXY_H

#include "dbus-module.h"
#include "dbus-object.h"

using namespace v8;

namespace dbus {

class DProxy : public ObjectWrap {
public:
  static void Init(Handle<Object> target);
  static Handle<Value> NewInstance(DObject *obj, const Arguments &args);

  EDBus_Proxy *GetProxy() { return proxy; }

private:
  DProxy(DObject *obj, const char *iface);
  ~DProxy();

  static Persistent<Function> constructor;
  static Handle<Value> New(const Arguments& args);

  static Handle<Value> AddSignalHandler(const Arguments &args);
  static Handle<Value> RemoveSignalHandler(const Arguments &args);
  static Handle<Value> Call(const Arguments &args);
  static Handle<Value> Send(const Arguments &args);

  EDBus_Proxy *proxy;
};

}

#endif