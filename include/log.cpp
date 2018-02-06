/****************************************************
* log.cpp
*
* Constructors and Methods thorden Class
*
* Node.js Addons C/C++
*
* Deps: v8, libuv ( POSIX-like )
*
* (C) 2016 Erick Rettozi
*
* MIT LICENCE
****************************************************/

#include <node.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cstdlib>
#include <sys/time.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <algorithm>
#include <exception>
#include <vector>
#include "log.hpp"
#include "colors.h"
#include "logtypes.h"
#include "constants.h"

using namespace std;
using namespace v8;

using v8::Exception;
using v8::Function;
using v8::FunctionTemplate;
using v8::HandleScope;
using v8::FunctionCallbackInfo;
using v8::Isolate;
using v8::Local;
using v8::Object;
using v8::String;
using v8::Value;
using v8::Array;

/*
* @Globals
*/
std::vector<const char*> fileFields[SIZE_TRANSPORT_FILE];
static char* buffDateTime = new char[BUFF_SIZE_DATETIME];
static char* buffDate = new char[BUFF_SIZE_DATE];
static char* buffDateLogRotate = new char[BUFF_SIZE_DATE];
static char* fileOutputOrigin[SIZE_TRANSPORT_FILE] = {NULL,NULL,NULL};
static char* fileOutput[SIZE_TRANSPORT_FILE] = {NULL,NULL,NULL};
static bool datePattern[SIZE_TRANSPORT_FILE] = {false,false,false};
static bool textColorizeFile[SIZE_TRANSPORT_FILE] = {false,false,false};
static bool textColorizeConsole[SIZE_TRANSPORT_CONSOLE] = {false,false,false};
static bool csvFile[SIZE_TRANSPORT_FILE] = {false,false,false};
static bool console[SIZE_TRANSPORT_CONSOLE] = {false,false,false};
int fOut[SIZE_TRANSPORT_FILE] = {};

Persistent<Function> Logger::constructor;

/*
* CONSTRUCTOR()
*
* @Public
*/
Logger::Logger(Local<Object> objectArgs,Isolate* isolate) {
	// Pego o Array Transports      
        Local<Array> Transports = Local<Array>::Cast(objectArgs->Get(String::NewFromUtf8(isolate,"Transports")));

	/* ------------------------------
	* Confiiguro o(s) Console(s)
	*------------------------------*/

        // Pego o Objeto Console (Local<Array>Transports(Local<Object>Console))
        Local<Value> consoleElem = Local<Value>::Cast(Transports->Get(String::NewFromUtf8(isolate,"Console")));

	// Se for Array de objetos Local<Array>Console<Objeto>
	if(consoleElem->IsArray()) {
		Local<Array> Console = Local<Array>::Cast(consoleElem);
		configureMultipleConsoleTransport(Console,isolate);
	}
	// Se não, configuro somente um console (Local<Object>Console)
	else
	if(consoleElem->IsObject()) {
        	Local<Object> Console = Local<Object>::Cast(Transports->Get(String::NewFromUtf8(isolate,"Console")));
		configureConsoleTransport(Console,isolate);
	}

	/* ------------------------------
	* Confiiguro o(s) Files(s)
	*------------------------------*/

        // Pego o Objeto File (Local<Array>Transports(Local<Object>File))
        Local<Value> fileElem = Local<Value>::Cast(Transports->Get(String::NewFromUtf8(isolate,"File")));

	// Se for Array de objetos Local<Array>File<Objeto>
	if(fileElem->IsArray()) {
		Local<Array> File = Local<Array>::Cast(fileElem);
		configureMultipleFileTransport(File,isolate);
	}
	// Se não, configuro somente um file (Local<Object>File)
	else
	if(fileElem->IsObject()) {
        	Local<Object> File = Local<Object>::Cast(Transports->Get(String::NewFromUtf8(isolate,"File")));
		configureFileTransport(File,isolate);
	}
}

/*
* DESTRUCTOR()
*
* @Public
*/
Logger::~Logger() {
	//...
}

/*
* Inicializo a lib e seus métodos
*
* @Public
*/
void Logger::Init(Local<Object> exports) {
	Isolate* isolate = exports->GetIsolate();

	Local<FunctionTemplate> tpl = FunctionTemplate::New(isolate, New);
	tpl->SetClassName(String::NewFromUtf8(isolate, "Logger"));
	tpl->InstanceTemplate()->SetInternalFieldCount(4);

	NODE_SET_PROTOTYPE_METHOD(tpl, "info", writeInfo);
	NODE_SET_PROTOTYPE_METHOD(tpl, "error", writeError);
	NODE_SET_PROTOTYPE_METHOD(tpl, "warn", writeWarn);
	NODE_SET_PROTOTYPE_METHOD(tpl, "destroy", destroy);

	constructor.Reset(isolate, tpl->GetFunction());
	exports->Set(String::NewFromUtf8(isolate, "Logger"),
        				 tpl->GetFunction());
}

/*
* Método para configurar o console transport
*
* @Public
*/
void Logger::configureConsoleTransport(Local<Object> Console,Isolate* isolate) {
	// Habilito o console em todos os levels
	for(unsigned int idx = 0; (unsigned)idx < SIZE_TRANSPORT_CONSOLE; idx++) {
		// Ligo o console
		setConsole(true,idx);

		// Pego a propriedade show do Objeto Console (Local<Array>Transports(Local<Object>Console)Local<Value>show)
		Local<Value> colorizeConsole = Console->Get(String::NewFromUtf8(isolate, "colorize"));

		// Habilito/Desabilito o console colorize
		if(colorizeConsole->IsBoolean())
        		setColorizeConsole(colorizeConsole->BooleanValue(),idx);
	}
}

/*
* Método para configurar o multiple-transport console
*
* @Public
*/
void Logger::configureMultipleConsoleTransport(Local<Array> Console,Isolate* isolate) {
       // Percorro todos os Consoles
       for(unsigned int i = 0; (unsigned)i < Console->Length(); i++) {
	        Local<Object> consoleObject = Local<Object>::Cast(Console->Get(i));

                // Pego a propriedade level do Objeto Console (Local<Array>Transports(Local<Object>Console)Local<Value>level)
                Local<Value> levelArg = consoleObject->Get(String::NewFromUtf8(isolate, "level"));

	        // Se não tiver a propriedade level, emito uma Exception
                if(strcmp(*String::Utf8Value(levelArg),"undefined") == 0) {
                        isolate->ThrowException(Exception::TypeError(
                                                         String::NewFromUtf8(isolate, "Erro: Ao configurar o multiple transport Console, é obrigatório atribuir um level à configuração do Transport!"))
                                               );
                        return;
                }

                int level = -1;

                if(strcmp(*String::Utf8Value(levelArg),INFO) == 0)
                          level = INFO_IDX;
                else
                if(strcmp(*String::Utf8Value(levelArg),ERROR) == 0)
                          level = ERROR_IDX;
                else
                if(strcmp(*String::Utf8Value(levelArg),WARN) == 0)
                          level = WARN_IDX;

	        // Ligo o console
		setConsole(true,level);

       		// Pego a propriedade colorize do Objeto Console (Local<Array>Transports(Local<Object>Console)Local<Value>colorize)
       		Local<Value> colorizeConsole = consoleObject->Get(String::NewFromUtf8(isolate, "colorize"));

       		// Seto se o console.log(). true | false
       		if(colorizeConsole->IsBoolean())
                        setColorizeConsole(colorizeConsole->BooleanValue(),level);
	}
}

/*
* Método para configurar o file transport
*
* @Public
*/
void Logger::configureFileTransport(Local<Object> File,Isolate* isolate) {
       // Pego a propriedade filename do Objeto File (Local<Array>Transports(Local<Object>File)Local<Value>filename)
       Local<Value> filename = File->Get(String::NewFromUtf8(isolate, "filename"));

       // Pego a propriedade datePattern do Objeto File (Local<Array>Transports(Local<Object>File)Local<Value>datePattern)
       Local<Value> datePattern = File->Get(String::NewFromUtf8(isolate, "datePattern"));

       // Se não for Comma-Separated values, então checo a propriedade colorize file
       Local<Value> CSV = File->Get(String::NewFromUtf8(isolate, "csv"));

       if(! CSV->IsBoolean()) {
		// Pego a propriedade colorize do Objeto File (Local<Array>Transports(Local<Object>File)Local<Value>colorize)
                Local<Value> colorizeFile = File->Get(String::NewFromUtf8(isolate, "colorize"));

                // Seto o text colorize
                if(colorizeFile->IsBoolean())
                	setColorizeFile(colorizeFile->BooleanValue(),INFO_IDX);
       }
       // Se for Comma-Separated values, então seto a proriedade csvFile
       else {
		// Pego a proriedade fields
                Local<Array> Fields = Local<Array>::Cast(File->Get(String::NewFromUtf8(isolate, "fields")));

                setCSVFileFields(Fields,INFO_IDX);

                setCSVFile(CSV->BooleanValue(),INFO_IDX);
       }

       // Seto o date pattern file
       if(datePattern->IsBoolean())
       		setDatePatternFile(datePattern->BooleanValue(),0);

       // Seto o arquivo output
       if(strcmp(*String::Utf8Value(filename),"undefined") == 0) {
                isolate->ThrowException(Exception::TypeError(
                                              String::NewFromUtf8(isolate, "Erro: Nenhum arquivo de saída foi definido!"))
                                       );
                return;
       }

       setFileOutput(*String::Utf8Value(filename),INFO_IDX);
}

/*
* Método para configurar o multiple-transport file
*
* @Public
*/
void Logger::configureMultipleFileTransport(Local<Array> File,Isolate* isolate) {
       // Percorro todos os File fields
       for(unsigned int i = 0; (unsigned)i < File->Length(); i++) {
	       Local<Object> fileObject = Local<Object>::Cast(File->Get(i));

               // Pego a propriedade level do Objeto File (Local<Array>Transports(Local<Object>File)Local<Value>filename)
               Local<Value> levelArg = fileObject->Get(String::NewFromUtf8(isolate, "level"));

	       // Se não tiver a propriedade level, emito uma Exception
               if(strcmp(*String::Utf8Value(levelArg),"undefined") == 0) {
                        isolate->ThrowException(Exception::TypeError(
                                                         String::NewFromUtf8(isolate, "Erro: Ao configurar o multiple transport File, é obrigatório atribuir um level à configuração do Transport!"))
                                               );
                        return;
                }

                int level = -1;

                if(strcmp(*String::Utf8Value(levelArg),INFO) == 0)
                          level = INFO_IDX;
                else
                if(strcmp(*String::Utf8Value(levelArg),ERROR) == 0)
                          level = ERROR_IDX;
                else
                if(strcmp(*String::Utf8Value(levelArg),WARN) == 0)
                          level = WARN_IDX;

                // Pego a propriedade filename do Objeto File (Local<Array>Transports(Local<Object>File)Local<Value>filename)
                Local<Value> filename = fileObject->Get(String::NewFromUtf8(isolate, "filename"));

                // Pego a propriedade datePattern do Objeto File (Local<Array>Transports(Local<Object>File)Local<Value>datePattern)
                Local<Value> datePattern = fileObject->Get(String::NewFromUtf8(isolate, "datePattern"));

                // Se não for Comma-Separated values, então checo a propriedade colorize file
                Local<Value> CSV = fileObject->Get(String::NewFromUtf8(isolate, "csv"));

                if(! CSV->IsBoolean()) {
                      // Pego a propriedade colorize do Objeto File (Local<Array>Transports(Local<Object>File)Local<Value>colorize)
                      Local<Value> colorizeFile = fileObject->Get(String::NewFromUtf8(isolate, "colorize"));
                      // Seto o text colorize
                      if(colorizeFile->IsBoolean())
                                 setColorizeFile(colorizeFile->BooleanValue(),level);
                }
                // Se for Comma-Separated values, então seto a proriedade csvFile
                else {
                      // Pego a proriedade fields
                      Local<Array> Fields = Local<Array>::Cast(fileObject->Get(String::NewFromUtf8(isolate, "fields")));
                      setCSVFileFields(Fields,level);
                      setCSVFile(CSV->BooleanValue(),level);
                }

                // Seto o date pattern file
                if(datePattern->IsBoolean())
                      setDatePatternFile(datePattern->BooleanValue(),level);

                // Seto o arquivo output
                if(strcmp(*String::Utf8Value(filename),"undefined") == 0) {
                      isolate->ThrowException(Exception::TypeError(
                                                     String::NewFromUtf8(isolate, "Erro: Nenhum arquivo de saída foi definido!"))
                                             );
                      return;
                }

               setFileOutput(*String::Utf8Value(filename),level);
	}
}

/*
* Método para instanciar a classe
*
* @Public
*/
void Logger::New(const FunctionCallbackInfo<Value>& args) {
	Isolate* isolate = args.GetIsolate();

	if (args.IsConstructCall()) {
		if(args.Length() < 1 || !args[0]->IsObject()) {
		    isolate->ThrowException(Exception::TypeError(
		    String::NewFromUtf8(isolate, "Erro: nenhum objeto foi passado")));
		    return;
		}

		// Pego o objeto passado para a classe
		Local<Object> objectArgs = args[0]->ToObject(isolate);

		// Instâncio a classe Log
		Logger* log = new Logger(objectArgs,isolate);
		log->Wrap(args.This());
		args.GetReturnValue().Set(args.This());
	}
	//else {
	//	const int argc = 1;
	//	Local<Value> argv[argc] = { args[0] };
	//	Local<Function> cons = Local<Function>::New(isolate, constructor);
	//	args.GetReturnValue().Set(cons->NewInstance(argc, argv));
	//}
}

/*
* Seto se as mensagens (info, error e warn) devem ser coloridos no arquivo de log
*
* @Public
*/
void Logger::setColorizeFile(bool value,int idx) {
	textColorizeFile[idx] = value;
}

/*
* Seto o formato do log para CSV
*
* @Public
*/
void Logger::setCSVFile(bool value,int idx) {
	csvFile[idx] = value;
}

/*
* Carrego o vetor com os campos ordenados
*
* @Public
*/
void Logger::setCSVFileFields(Local<Array> Fields,int idx) {
	for (unsigned int i = 0; (unsigned)i < Fields->Length(); i++) {
        	char *field = (char* )malloc(strlen(*String::Utf8Value(Fields->Get(i)))+PAD_BYTES);
		memset(field, 0, sizeof(*field));
		strcpy(field,*String::Utf8Value(Fields->Get(i)));
		fileFields[idx].push_back(field);
	}
}

/*
* Seto se as mensagens (info, error e warn) devem ser exibidas coloridas no console
*
* @Public
*/
void Logger::setColorizeConsole(bool value, int idx) {
	textColorizeConsole[idx] = value;
}

/*
* Seto se as mensagens (info, error e warn) devem ser exibidas coloridas no console
*
* @Public
*/
void Logger::setConsole(bool value, int idx) {
	console[idx] = value;
}

/*
* Seto o arquivo texto onde serão gravados os logs
*
* @Public
*/
void Logger::setFileOutput(char* file,int idx) {
        fileOutputOrigin[idx] = (char* )malloc((strlen(file) * SIZE_TRANSPORT_FILE)+PAD_BYTES);
	memset(fileOutputOrigin[idx], 0, sizeof(*fileOutputOrigin[idx]));
        strcpy(fileOutputOrigin[idx],file);

        fileOutput[idx] = (char* )malloc((strlen(file) * SIZE_TRANSPORT_FILE)+PAD_BYTES);
	memset(fileOutput[idx], 0, sizeof(*fileOutput[idx]));
        strcpy(fileOutput[idx],file);

        // Crio/Abro o arquivo de log em mondo de append
	createFile(idx);
}

/*
* Imprimo a linha de log. INFO TYPE
*
* @Public
*/
void Logger::writeInfo(const FunctionCallbackInfo<Value>& args) {
	 // Seto o FH ID
	 int fhIdx = INFO_IDX;

	// Verifico se faço o rotate do arquivo de log
	if(datePattern[fhIdx] && logRotate())
		createFile(fhIdx);

	//Se não for Comma-Separated values
        if(fileOutput[fhIdx] && !csvFile[fhIdx])
                writeTextFile(fOut[fhIdx],getLogLine(args,(char*)INFO,textColorizeFile[fhIdx]));
	else
        if(fileOutput[fhIdx] && csvFile[fhIdx])
                writeTextFile(fOut[fhIdx],getCSVLogLine(args,(char*)INFO,false,fhIdx));

	// Console.log()
	// Se não for Comma-Separated values
	if(!csvFile[fhIdx] && console[fhIdx])
	       	printf("%s\n",getLogLine(args,(char*)INFO,textColorizeConsole[fhIdx]));
	else
	if(csvFile[fhIdx] && console[fhIdx])
	       	printf("%s\n",getCSVLogLine(args,(char*)INFO,textColorizeConsole[fhIdx],fhIdx));
}

/*
* Imprimo a linha de log. ERROR TYPE
*
* @Public
*/
void Logger::writeError(const FunctionCallbackInfo<Value>& args) {
	 // Seto o FH ID
	 int fhIdx = INFO_IDX;

         if(fileOutput[(int)ERROR_IDX] != NULL)
		fhIdx = ERROR_IDX;

	// Verifico se faço o rotate do arquivo de log
	if(datePattern[fhIdx] && logRotate())
		createFile(fhIdx);

	// Se não for Comma-Separated values
        if(fileOutput[fhIdx] && !csvFile[fhIdx])
                writeTextFile(fOut[fhIdx],getLogLine(args,(char*)ERROR,textColorizeFile[fhIdx]));
	else
        if(fileOutput[fhIdx] && csvFile[fhIdx])
                writeTextFile(fOut[fhIdx],getCSVLogLine(args,(char*)ERROR,false,fhIdx));

	// Console.log()
	// Se não for Comma-Separated values
	if(!csvFile[fhIdx] && console[fhIdx])
	        printf("%s\n",getLogLine(args,(char*)ERROR,textColorizeConsole[fhIdx]));
	else
	if(csvFile[fhIdx] && console[fhIdx])
	       	printf("%s\n",getCSVLogLine(args,(char*)ERROR,textColorizeConsole[fhIdx],fhIdx));
}

/*
* Imprimo a linha de log. WARN TYPE
*
* @Public
*/
void Logger::writeWarn(const FunctionCallbackInfo<Value>& args) {
	 // Seto o FH ID
	 int fhIdx = INFO_IDX;

         if(fileOutput[(int)WARN_IDX] != NULL)
		fhIdx = WARN_IDX;

	// Verifico se faço o rotate do arquivo de log
	if(datePattern[fhIdx] && logRotate())
		createFile(fhIdx);

	// Se não for Comma-Separated values
        if(fileOutput[fhIdx] && !csvFile[fhIdx])
                writeTextFile(fOut[fhIdx],getLogLine(args,(char*)WARN,textColorizeFile[fhIdx]));
	else
        if(fileOutput[fhIdx] && csvFile[fhIdx])
                writeTextFile(fOut[fhIdx],getCSVLogLine(args,(char*)WARN,false,fhIdx));

	// Console.log()
	// Se não for Comma-Separated values
	if(!csvFile[fhIdx] && console[fhIdx])
	        printf("%s\n",getLogLine(args,(char*)WARN,textColorizeConsole[fhIdx]));
	else
	if(csvFile[fhIdx] && console[fhIdx])
	       	printf("%s\n",getCSVLogLine(args,(char*)WARN,textColorizeConsole[fhIdx],fhIdx));
}

/*
* Chamo o destructor da classe
*
* @Public
*/
void Logger::destroy(const FunctionCallbackInfo<Value>& args) {
        Isolate* isolate = args.GetIsolate();
	freeMemory(isolate);
}

/*
* Crio o arquivo de log
*
* @Private
*/
void Logger::createFile(int idx) {
	// Coloco a data corrente no nome do arquivo de log caso o datePattern = TRUE
	if(datePattern[idx]) {
		strcpy(fileOutput[idx],fileOutputOrigin[idx]);
		strcat(fileOutput[idx],(char*)"-");
	        strcat(fileOutput[idx],formatDate());
	}

	// Cria o arquivo em mondo de append
	umask(0);
	fOut[idx] = open (fileOutput[idx], O_RDWR|(O_APPEND|O_CREAT)|O_SYNC,0644);
	if (fOut[idx] == -1)
	        printf("Falha ao criar arquivo.\n");
}

/*
* Libero o ponteiro FILE* da memória
*
* @Private
*/
void Logger::closeFile(int idx) {
	if(idx == -1)
		for(unsigned int i = 0; (unsigned)i < SIZE_TRANSPORT_FILE; i++) {
			close(fOut[i]);
		}
	else
		close(fOut[idx]);
}

/*
* Coloco a data corrente no nome do arquivo de log.
* Formato: file.log-YYYY-MM-DD
*
* @Private
*/
void Logger::setDatePatternFile(bool value,int idx) {
	datePattern[idx] = value;
}

/*
* Libero ponteiros da memória
*
* @Private
*/
void Logger::freeMemory(Isolate* isolate) {
	try {
		closeFile(-1);

		for(unsigned int i = 0; (unsigned)i < SIZE_TRANSPORT_FILE; i++) {
			if(fileOutput[i] != NULL) {
				free(fileOutput[i]);
				fileOutput[i] = NULL;
			}
		}

		for(unsigned int i = 0; (unsigned)i < SIZE_TRANSPORT_FILE; i++) {
			if(fileOutputOrigin[i] != NULL) {
				free(fileOutputOrigin[i]);
				fileOutputOrigin[i] = NULL;
			}
		}

		if(buffDateTime != NULL) {
			free(buffDateTime);
			buffDateTime = NULL;
		}

		if(buffDate != NULL) {
			free(buffDate);
			buffDate = NULL;
		}

		if(buffDateLogRotate != NULL) {
			free(buffDateLogRotate);
			buffDateLogRotate = NULL;
		}
	} catch (exception& e) {
		isolate->ThrowException(Exception::TypeError(
                        String::NewFromUtf8(isolate, e.what()))
                );
	}
}

/*
* Verifico se entro no rotate date & time log
*
* @Private
*/
bool Logger::logRotate() {
  	time_t now = time (0);
  	strftime (buffDateLogRotate, BUFF_SIZE_DATE, "%Y-%m-%d", localtime (&now));
	return (strcmp(buffDateLogRotate,buffDate) != 0 ) ? true : false;
}


/*
* Pego a data do sistema
*
* @Private
*/
char* Logger::formatDate() {
  	time_t now = time (0);
  	strftime (buffDate, BUFF_SIZE_DATE, "%Y-%m-%d", localtime (&now));
	return buffDate;
}

/*
* Pego a data e hora do sistema 
*
* @Private
*/
char* Logger::formatDateTime() {
	struct timeval tv;
	struct tm* ptm;
	char timeString[BUFF_SIZE_DATETIME];
	long milliseconds;

	gettimeofday (&tv, NULL);
	ptm = localtime (&tv.tv_sec);
	strftime (timeString, BUFF_SIZE_DATETIME, "%Y-%m-%d %H:%M:%S", ptm);
	milliseconds = tv.tv_usec / 1000;
	sprintf (buffDateTime,"%s.%03ld", timeString, milliseconds);

	return buffDateTime;
}

/*
* Valido os parâmetros passados pela aplicação
*
* @Private
*/
void Logger::validParams(const FunctionCallbackInfo<Value>& args, char* logType) {
  	Isolate* isolate = args.GetIsolate();

	// Verifico o número de parâmentros
        if (args.Length() < 2) {
                isolate->ThrowException(Exception::TypeError(
                        String::NewFromUtf8(isolate, "Número de argumentos inválido!"))
                );
        }

        // Verifico se os parâmetros logType é válido
        if (strcmp(logType,INFO) != 0 && strcmp(logType,ERROR) != 0 && strcmp(logType,WARN) != 0) {
                isolate->ThrowException(Exception::TypeError(
                        String::NewFromUtf8(isolate, "Argumento inválido!"))
                );
        }
}

/*
* Pego o valor atribuido às variáveis
*
* @Private
*/
char* Logger::getValueVar(const char *str, const char *oldstr, const char *newstr, int *count) {
	const char *tmp = str;
	char *result;
	int found = 0;
	int length, reslen;
	int oldlen = strlen(oldstr);
	int newlen = strlen(newstr);
	int limit = (count != NULL && *count > 0) ? *count : -1; 

	tmp = str;
	if ((tmp = strstr(tmp, oldstr)) != NULL && found != limit)
		found++, tmp += oldlen;

	length = strlen(str) + found * (newlen - oldlen);
	if ( (result = (char *)malloc(length+PAD_BYTES)) == NULL) {
		fprintf(stderr, "Memória insuficiente\n");
		found = -1;
	} else {
		tmp = str;
		limit = found;
		reslen = 0; 

		if ((limit-- > 0) && (tmp = strstr(tmp, oldstr)) != NULL) {
			length = (tmp - str);
			strncpy(result + reslen, str, length);
			strcpy(result + (reslen += length), newstr);
			reslen += newlen;
			tmp += oldlen;
			str = tmp;
		}

		strcpy(result + reslen, str);
	}

	if (count != NULL) *count = found;

	return result;
}

/*
* JSON Stringify
*
* @Private
*/
char* Logger::jsonStringify(Isolate* isolate,Local<Object> object) {
	Local<Context> context = isolate->GetCurrentContext();
        Local<Object> global = context->Global();

        Local<Object> JSON = Local<Object>::Cast(
        	global->Get(String::NewFromUtf8(isolate,"JSON"))
        );

        Local<Function> stringify = Local<Function>::Cast(
        	JSON->Get(String::NewFromUtf8(isolate,"stringify"))
        );

        Local<Value> jsonHash[] = { object };
        v8::String::Utf8Value var(Local<String>::Cast(stringify->Call(JSON, 1, jsonHash)));

	char* string = (char* )malloc(strlen(*var)+PAD_BYTES);
	memset(string, 0, sizeof(*string));
	strcpy(string,*var);
	return string;
}

/*
* Pego as variáveis String(%s) e JSON HASH(%j)
*
* @Private
*/
char* Logger::getVars(const FunctionCallbackInfo<Value>& args, int offset, char* stringInfo, int idx) {

  	Isolate* isolate = args.GetIsolate();

	int rpl = 0;
	char* newStringInfo = (char* )malloc(strlen(stringInfo)+PAD_BYTES);
	memset(newStringInfo, 0, sizeof(*newStringInfo));
	strcpy(newStringInfo,stringInfo);

	if(args[1]->IsObject() && !fileFields[idx].empty()) {
                Local<Object> objectArg = args[1]->ToObject(isolate);

                for (unsigned int i = 0; (unsigned)i < fileFields[idx].size(); i++) {
                        Local<Value> field = objectArg->Get(String::NewFromUtf8(isolate,fileFields[idx][i]));

                        if(field->IsObject()) {
				Local<Object> bufferObj = field->ToObject();
				char *var = jsonStringify(isolate,bufferObj);
                	        newStringInfo = getValueVar(newStringInfo,"%j",var,&rpl);
                        }
			else {
                        	v8::String::Utf8Value var(field->ToString());
	                        newStringInfo = getValueVar(newStringInfo,"%s",(strcmp(*var,"undefined") == 0) ? "" : *var,&rpl);
			}
                }
        }
	else {
	        for (int i = offset; i < args.Length(); i++) {
        	        if(args[i]->IsObject()) {
                	        Local<Object> bufferObj = args[i]->ToObject();
				char *var = jsonStringify(isolate,bufferObj);
	                        newStringInfo = getValueVar(newStringInfo,"%j",var,&rpl);
	                }
        	        else {
                	        v8::String::Utf8Value var(args[i]->ToString());
                        	newStringInfo = getValueVar(newStringInfo,"%s",(strcmp(*var,"undefined") == 0) ? "" : *var,&rpl);
	                }
	        }
	}

	return newStringInfo;
}

/*
* Gero a linha que será escrita no arquivo de log
*
* @Private
*/
char* Logger::createLogLine(char* logType, char* appName, char* stringInfo, bool textColorize) {

        char* delim1 = (char*)"-";
        char* delim2 = (char*)":";
        char* space = (char*)" ";
	char* dateTime = formatDateTime();
        char* logLine;

	if(textColorize)
	        logLine = (char* )malloc( strlen(dateTime)         +
                                          strlen(space)            +
                                          strlen(delim1)           +
                                          strlen(space)            +
					  strlen(ANSI_COLOR_GREEN) +
                                          strlen(logType)          +
					  strlen(ANSI_COLOR_RESET) +
                                          strlen(delim2)           +
                                          strlen(space)            +
                                          strlen(appName)          +
                                          strlen(space)            +
                                          strlen(delim1)           +
                                          strlen(space)            +
                                          strlen(stringInfo)       +
                                          PAD_BYTES );
	else
                logLine = (char* )malloc( strlen(dateTime)         +
                                          strlen(space)            +
                                          strlen(delim1)           +
                                          strlen(space)            +
                                          strlen(logType)          +
                                          strlen(delim2)           +
                                          strlen(space)            +
                                          strlen(appName)          +
                                          strlen(space)            +
                                          strlen(delim1)           +
                                          strlen(space)            +
                                          strlen(stringInfo)       +
                                          PAD_BYTES );

	memset(logLine, 0, sizeof(*logLine));

        strcpy(logLine, dateTime);
        strcat(logLine, space);
        strcat(logLine, delim1);
        strcat(logLine, space);

	if(textColorize) {
	        if(strcmp(logType,INFO) == 0) {
        	        strcat(logLine, ANSI_COLOR_GREEN);
	        }
        	else if(strcmp(logType,ERROR) == 0) {
                	strcat(logLine, ANSI_COLOR_RED);
	        }
        	else if(strcmp(logType,WARN) == 0) {
                	strcat(logLine, ANSI_COLOR_YELLOW);
	        }

	        strcat(logLine, logType);
		strcat(logLine,ANSI_COLOR_RESET);
	}
	else
	        strcat(logLine, logType);

        strcat(logLine, delim2);
        strcat(logLine, space);
        strcat(logLine, appName);
        strcat(logLine, space);
        strcat(logLine, delim1);
        strcat(logLine, space);
        strcat(logLine, stringInfo);

        // Retorno a linha já formatada
        return logLine;
}

/*
* Gero a linha que será escrita no arquivo de log no formato CSV
*
* @Private
*/
char* Logger::createCSVLine(char* logType, char* appName, char* stringInfo, bool textColorize) {

	char* delim = (char*)";";
        char* dateTime = formatDateTime();
        char* logLine;

	if(textColorize)
	        logLine = (char* )malloc( strlen(dateTime)         +
                                          strlen(delim)            +
                                          strlen(appName)          +
                                          strlen(delim)            +
	  				  strlen(ANSI_COLOR_GREEN) +
                                          strlen(logType)          +
					  strlen(ANSI_COLOR_RESET) +
                                          strlen(delim)            +
                                          strlen(stringInfo)       +
                                          PAD_BYTES );
	else
                logLine = (char* )malloc( strlen(dateTime)         +
                                          strlen(delim)            +
                                          strlen(appName)          +
                                          strlen(delim)            +
                                          strlen(logType)          +
                                          strlen(delim)            +
                                          strlen(stringInfo)       +
                                          PAD_BYTES );

        memset(logLine, 0, sizeof(*logLine));

        strcpy(logLine, dateTime);
        strcat(logLine, delim);
        strcat(logLine, appName);
        strcat(logLine, delim);

	if(textColorize) {
                if(strcmp(logType,INFO) == 0) {
                        strcat(logLine, ANSI_COLOR_GREEN);
                }
                else if(strcmp(logType,ERROR) == 0) {
                        strcat(logLine, ANSI_COLOR_RED);
                }
                else if(strcmp(logType,WARN) == 0) {
                        strcat(logLine, ANSI_COLOR_YELLOW);
                }

        	strcat(logLine, logType);
		strcat(logLine, ANSI_COLOR_RESET);
        }
	else
        	strcat(logLine, logType);

        strcat(logLine, delim);
        strcat(logLine, stringInfo);

        // Retorno a linha já formatada
        return logLine;
}

/*
* Retorno a linha que será escrita no arquivo de log
*
* @Private
*/
char* Logger::getLogLine(const FunctionCallbackInfo<Value>& args, char* logType, bool textColorize) {

	// Válido os parâmentros passados
	validParams(args,logType);

	// Pego os parâmetros obrigatórios
	v8::String::Utf8Value appName(args[0]->ToString());
	v8::String::Utf8Value stringInfo(args[1]->ToString());

	// Pego os valores das variáveis que foram passadas por parâmetro
	char* newStringInfo = getVars(args,2,*stringInfo,INFO_IDX);

	// Monto a linha do log
	char* logLine = createLogLine(logType,*appName,newStringInfo,textColorize);

	return logLine;
}

/*
* Retorno a linha que será escrita no arquivo de log no formato CSV
*
* @Private
*/
char* Logger::getCSVLogLine(const FunctionCallbackInfo<Value>& args, char* logType, bool textColorize,int idx) {

        // Válido os parâmentros passados
        validParams(args,logType);

        // Pego os parâmetros obrigatórios
        v8::String::Utf8Value appName(args[0]->ToString());

	// Crio o layout line CSV
	char* stringInfo = prepareCSVLine(args,idx);

        // Pego os valores das variáveis que foram passadas por parâmetro
        char* newStringInfo = getVars(args,1,stringInfo,idx);

        // Monto a linha do log
        char* logCSVLine = createCSVLine(logType,*appName,newStringInfo,textColorize);

        return logCSVLine;
}

/*
* Crio o layout line CSV
*
* @Private
*/
char* Logger::prepareCSVLine(const FunctionCallbackInfo<Value>& args, int idx) {

        Isolate* isolate = args.GetIsolate();

	char *logLine = NULL;

	if(args[1]->IsObject() && !fileFields[idx].empty()) {
                Local<Object> objectArg = args[1]->ToObject(isolate);

		//char* value = (char* )malloc(strlen(*String::Utf8Value(objectArg->Get(String::NewFromUtf8(isolate,fileFields[idx][0])))) + PAD_BYTES);
		//memset(value, 0, sizeof(*value));
		//strcpy(value, *String::Utf8Value(objectArg->Get(String::NewFromUtf8(isolate,fileFields[idx][0]))));

		//if(strcmp(value,"undefined") != 0) {
			Local<Value> field = objectArg->Get(String::NewFromUtf8(isolate,fileFields[idx][0]));
			logLine = (char* )malloc(fileFields[idx].size() + PAD_BYTES);
			memset(logLine, 0, sizeof(*logLine));
			strcpy(logLine, (char*)(field->IsObject()) ? "%j" : "%s" );
		//}

		for (unsigned int i = 1; (unsigned)i < fileFields[idx].size(); i++) {
		        //char* value = (char* )malloc(strlen(*String::Utf8Value(objectArg->Get(String::NewFromUtf8(isolate,fileFields[idx][i])))) + PAD_BYTES);
			//memset(value, 0, sizeof(*value));
			//strcpy(value, *String::Utf8Value(objectArg->Get(String::NewFromUtf8(isolate,fileFields[idx][i]))));

			//if(strcmp(value,"undefined") != 0) {
				Local<Value> field = objectArg->Get(String::NewFromUtf8(isolate,fileFields[idx][i]));
		        	strcat(logLine, (char*)";");
			        strcat(logLine, (char*)(field->IsObject()) ? "%j" : "%s" );
			//}
		}
	}
	else {
	        logLine = (char* )malloc(args.Length() + (args.Length() - 1) + PAD_BYTES);
	        memset(logLine, 0, sizeof(*logLine));
	        strcpy(logLine, (char*)(args[1]->IsObject()) ? "%j" : "%s" );

	        for (int i = 2; i < args.Length(); i++) {
	        	strcat(logLine, (char*)";");
		        strcat(logLine, (char*)(args[i]->IsObject()) ? "%j" : "%s" );
	        }
	}

        return logLine;
}

/*
* Escrevo no arquivo de log
*
* @Private
*/
void Logger::writeTextFile(int fh, char* buff) {
	strcat(buff,"\n");
	int res = write (fh, buff, strlen(buff));
	if(res == -1)
		printf("Erro ao escrever no arquivo.\n");
}
