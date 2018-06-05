// omapi-node.cpp
#include <node.h>

namespace libomapi {

using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;

void test(const FunctionCallbackInfo<Value>& args) {
  Isolate* isolate = args.GetIsolate();
  args.GetReturnValue().Set(String::NewFromUtf8(isolate, "Test!"));
}

void init(Local<Object> exports) {
  NODE_SET_METHOD(exports, "test", test);
}

NODE_MODULE(libomapi, init)

}
