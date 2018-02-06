/****************************************************
* log.hpp
*
* @Class: thorden
*
* Node.js Addons C/C++
*
* Deps: v8, libuv ( POSIX-like )
*
* (C) 2016 Erick Rettozi 
*
* MIT LICENCE
****************************************************/


#ifndef LOG_HPP
#define LOG_HPP

#include <node.h>
#include <node_object_wrap.h>

using v8::Function;
using v8::FunctionCallbackInfo;
using v8::Value;

using namespace node;
using namespace v8;

class Logger : public node::ObjectWrap {

	public:
	    static void Init(v8::Local<v8::Object> exports);

	private:
	    explicit Logger(Local<Object> objectArgs,Isolate* isolate);
	    ~Logger();

            static void New(const FunctionCallbackInfo<Value>& args);
            static v8::Persistent<v8::Function> constructor;

            static void writeInfo(const FunctionCallbackInfo<Value>& args);
            static void writeError(const FunctionCallbackInfo<Value>& args);
            static void writeWarn(const FunctionCallbackInfo<Value>& args);
            static void destroy(const FunctionCallbackInfo<Value>& args);

            static void setFileOutput(char* file,int idx);
            static void setColorizeFile(bool value,int idx);
            static void setColorizeConsole(bool value, int idx);
            static void setConsole(bool value, int idx);
            static void setCSVFile(bool value,int idx);
            static void setCSVFileFields(Local<Array> Fields,int idx);
	    static void createFile(int idx);
	    static void closeFile(int idx);
	    static void setDatePatternFile(bool value,int idx);
	    static void writeTextFile(int fh,char* buff);
	    static char* jsonStringify(Isolate* isolate,Local<Object> object);
	    static char* prepareCSVLine(const FunctionCallbackInfo<Value>& args, int idx);
	    static char* getCSVLogLine(const FunctionCallbackInfo<Value>& args, char* logType, bool textColorize,int idx);
	    static char* createCSVLine(char* logType, char* appName, char* stringInfo, bool textColorize);
	    static char* getLogLine(const FunctionCallbackInfo<Value>& args, char* logType, bool textColorize);
	    static char* createLogLine(char* logType, char* appName, char* stringInfo, bool textColorize);
    	    static char* formatDateTime();
    	    static char* formatDate();
    	    static bool logRotate();
	    static char* getValueVar(const char *str, const char *oldstr, const char *newstr, int *count);
	    static char* getVars(const FunctionCallbackInfo<Value>& args, int offset, char* stringInfo, int idx);
	    static void validParams(const FunctionCallbackInfo<Value>& args, char* logType);
	    static void freeMemory(Isolate* isolate);
	    static void configureConsoleTransport(Local<Object> Console,Isolate* isolate);
	    static void configureFileTransport(Local<Object> File,Isolate* isolate);
	    static void configureMultipleFileTransport(Local<Array> File,Isolate* isolate);
	    static void configureMultipleConsoleTransport(Local<Array> Console,Isolate* isolate);
};

#endif
