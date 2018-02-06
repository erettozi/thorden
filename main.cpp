/**********************************************************************
* main.cpp
*
* Async Logging Library de alta performance para gerar arquivos de log
*
* Node.js Addons C/C++
*
* Deps: v8, libuv ( POSIX-like )
*
* (C) 2016 Erick Rettozi 
*
* MIT LICENCE
**********************************************************************/

#include <node.h>
#include "include/log.hpp"

using v8::Local;
using v8::Object;

void initAll (Local<Object> exports) {
	Logger::Init(exports);
}

NODE_MODULE(addon, initAll);
