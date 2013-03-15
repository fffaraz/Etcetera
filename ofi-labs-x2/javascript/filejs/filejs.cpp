/*
  This file is part of the Ofi Labs X2 project.

  Copyright (C) 2010 Ariya Hidayat <ariya.hidayat@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <v8.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <dirent.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <fstream>
#include <iostream>

using namespace v8;

static Handle<Value> fs_exists(const Arguments& args);
static Handle<Value> fs_isDirectory(const Arguments& args);
static Handle<Value> fs_isFile(const Arguments& args);
static Handle<Value> fs_makeDirectory(const Arguments& args);
static Handle<Value> fs_list(const Arguments& args);
static Handle<Value> fs_open(const Arguments& args);
static Handle<Value> fs_workingDirectory(const Arguments& args);

static Handle<Value> stream_constructor(const Arguments& args);
static Handle<Value> stream_close(const Arguments& args);
static Handle<Value> stream_flush(const Arguments& args);
static Handle<Value> stream_next(const Arguments& args);
static Handle<Value> stream_readLine(const Arguments& args);
static Handle<Value> stream_writeLine(const Arguments& args);

static Handle<Value> system_print(const Arguments& args);
static Handle<Value> system_exit(const Arguments& args);

static void CleanupStream(Persistent<Value>, void *data)
{
    delete reinterpret_cast<std::fstream*>(data);
}

int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Usage: filejs inputfile.js" << std::endl;
        return 0;
    }

    FILE* f = fopen(argv[1], "r");
    if (!f) {
        std::cerr << "Error: unable to open file " << argv[1] << std::endl;
        return 0;
    }
    fseek(f, 0, SEEK_END);
    int len = ftell(f);
    rewind(f);
    char* buf = new char[len + 1];
    fread(buf, 1, len, f);
    buf[len - 1] = '\0';
    fclose(f);

    HandleScope handle_scope;
    Handle<ObjectTemplate> global = ObjectTemplate::New();
    Handle<Context> context = Context::New(NULL, global);

    Context::Scope context_scope(context);

    // 'fs' object
    Handle<FunctionTemplate> fsObject = FunctionTemplate::New();
    fsObject->Set(String::New("exists"), FunctionTemplate::New(fs_exists)->GetFunction());
    fsObject->Set(String::New("makeDirectory"), FunctionTemplate::New(fs_makeDirectory)->GetFunction());
    fsObject->Set(String::New("isDirectory"), FunctionTemplate::New(fs_isDirectory)->GetFunction());
    fsObject->Set(String::New("isFile"), FunctionTemplate::New(fs_isFile)->GetFunction());
    fsObject->Set(String::New("list"), FunctionTemplate::New(fs_list)->GetFunction());
    fsObject->Set(String::New("open"), FunctionTemplate::New(fs_open)->GetFunction());
    fsObject->Set(String::New("workingDirectory"), FunctionTemplate::New(fs_workingDirectory)->GetFunction());
    context->Global()->Set(String::New("fs"), fsObject->GetFunction());

    // 'system' object
    Handle<FunctionTemplate> systemObject = FunctionTemplate::New();
    Handle<Array> systemArgs = Array::New(argc);
    for (int i = 1; i < argc; ++i)
        systemArgs->Set(i - 1, String::New(argv[i]));
    systemObject->Set(String::New("args"), systemArgs);
    systemObject->Set(String::New("print"), FunctionTemplate::New(system_print)->GetFunction());
    systemObject->Set(String::New("exit"), FunctionTemplate::New(system_exit)->GetFunction());
    context->Global()->Set(String::New("system"), systemObject->GetFunction());

    // 'Stream' class
    Handle<FunctionTemplate> streamClass = FunctionTemplate::New(stream_constructor);
    streamClass->SetClassName(String::New("Stream"));
    streamClass->InstanceTemplate()->SetInternalFieldCount(1);
    streamClass->InstanceTemplate()->Set(String::New("close"), FunctionTemplate::New(stream_close)->GetFunction());
    streamClass->InstanceTemplate()->Set(String::New("flush"), FunctionTemplate::New(stream_flush)->GetFunction());
    streamClass->InstanceTemplate()->Set(String::New("next"), FunctionTemplate::New(stream_next)->GetFunction());
    streamClass->InstanceTemplate()->Set(String::New("readLine"), FunctionTemplate::New(stream_readLine)->GetFunction());
    streamClass->InstanceTemplate()->Set(String::New("writeLine"), FunctionTemplate::New(stream_writeLine)->GetFunction());
    context->Global()->Set(String::New("Stream"), streamClass->GetFunction(), PropertyAttribute(ReadOnly | DontDelete));

    Handle<Script> script = Script::Compile(String::New(buf));
    script->Run();

    delete [] buf;
    return 0;
}

static Handle<Value> fs_exists(const Arguments& args)
{
    HandleScope handle_scope;

    if (args.Length() != 1)
        return ThrowException(String::New("Exception: function fs.exists() accepts 1 argument"));

    String::Utf8Value fileName(args[0]);

    struct stat statbuf;
    bool canStat = ::stat(*fileName, &statbuf) == 0;
    return Boolean::New(canStat);
}

static Handle<Value> fs_isDirectory(const Arguments& args)
{
    HandleScope handle_scope;

    if (args.Length() != 1)
        return ThrowException(String::New("Exception: fs.isDirectory() accepts 1 argument"));

    String::Utf8Value name(args[0]);

    struct stat statbuf;
    if (::stat(*name, &statbuf))
        return ThrowException(String::New("Exception: fs.isDirectory() can't access the directory"));

    return Boolean::New(S_ISDIR(statbuf.st_mode));
}

static Handle<Value> fs_isFile(const Arguments& args)
{
    HandleScope handle_scope;

    if (args.Length() != 1)
        return ThrowException(String::New("Exception: fs.isFile() accepts 1 argument"));

    String::Utf8Value name(args[0]);

    struct stat statbuf;
    if (::stat(*name, &statbuf))
        return ThrowException(String::New("Exception: fs.isFile() can't access the file"));

    return Boolean::New(S_ISREG(statbuf.st_mode));
}

static Handle<Value> fs_makeDirectory(const Arguments& args)
{
    HandleScope handle_scope;

    if (args.Length() != 1)
        return ThrowException(String::New("Exception: function fs.makeDirectory() accepts 1 argument"));

    String::Utf8Value directoryName(args[0]);

    if (::mkdir(*directoryName, 0777) == 0)
        return Undefined();

    return ThrowException(String::New("Exception: fs.makeDirectory() can't create the directory"));
}

static Handle<Value> fs_list(const Arguments& args)
{
    HandleScope handle_scope;

    if (args.Length() != 1)
        return ThrowException(String::New("Exception: fs.list() accepts 1 argument"));

    String::Utf8Value dirname(args[0]);

    DIR *dir = opendir(*dirname);
    if (!dir)
        return ThrowException(String::New("Exception: fs.list() can't access the directory"));

    Handle<Array> entries = Array::New();
    int count = 0;
    struct dirent entry;
    struct dirent *ptr = NULL;
    ::readdir_r(dir, &entry, &ptr);
    while (ptr) {
        if (strcmp(entry.d_name, ".") && strcmp(entry.d_name, ".."))
            entries->Set(count++, String::New(entry.d_name));
        ::readdir_r(dir, &entry, &ptr);
    }
    ::closedir(dir);

    return entries;
}

static Handle<Value> fs_open(const Arguments& args)
{
    HandleScope handle_scope;

    if (args.Length() != 1 && args.Length() != 2)
        return ThrowException(String::New("Exception: function fs.open() accepts 1 or 2 arguments"));

    Handle<Context> context = Context::GetCurrent();
    Handle<Value> streamClass = context->Global()->Get(String::New("Stream"));
    Function *streamFunction = Function::Cast(*streamClass);

    Handle<Value> argv[2];
    argv[0] = args[0];
    if (args.Length() == 2)
        argv[1] = args[1];

    Handle<Object> result = streamFunction->NewInstance(args.Length(), argv);
    return result;
}

static Handle<Value> fs_workingDirectory(const Arguments& args)
{
    if (args.Length() != 0)
        return ThrowException(String::New("Exception: function fs.workingDirectory() accepts no argument"));

    char currentName[PATH_MAX + 1];
    if (::getcwd(currentName, PATH_MAX))
        return String::New(currentName);

    return ThrowException(String::New("Exception: fs.workingDirectory() can't get current working directory"));
}

static Handle<Value> stream_constructor(const Arguments& args)
{
    HandleScope handle_scope;

    if (args.Length() != 1 && args.Length() != 2)
        return ThrowException(String::New("Exception: Stream constructor accepts 1 or 2 arguments"));

    String::Utf8Value name(args[0]);
    String::Utf8Value modes(args[1]);

    std::fstream::openmode mode = std::fstream::in;
    if (args.Length() == 2) {
        String::Utf8Value m(args[0]);
        const char* options = *modes;
        bool read = strchr(options, 'r');
        bool write = strchr(options, 'w');
        if (!read && !write)
            return ThrowException(String::New("Exception: Invalid open mode for Stream"));
        if (!read)
            mode = std::fstream::out;
        if (write)
            mode |= std::fstream::out;
    }

    std::fstream *data = new std::fstream;
    data->open(*name, mode);
    if (data->bad()) {
        delete data;
        return ThrowException(String::New("Exception: Can't open the file"));
    }

    args.This()->SetPointerInInternalField(0, data);

    Persistent<Object> persistent = Persistent<Object>::New(args.Holder());
    persistent.MakeWeak(data, CleanupStream);

    persistent->Set(String::New("name"), args[0]);

    return handle_scope.Close(persistent);
}

static Handle<Value> stream_close(const Arguments& args)
{
    if (args.Length() != 0)
        return ThrowException(String::New("Exception: Stream.close() accepts no argument"));

    void *data = args.This()->GetPointerFromInternalField(0);
    std::fstream *fs = reinterpret_cast<std::fstream*>(data);
    fs->close();

    return Undefined();
}

static Handle<Value> stream_flush(const Arguments& args)
{
    if (args.Length() != 0)
        return ThrowException(String::New("Exception: Stream.flush() accepts no argument"));

    void *data = args.This()->GetPointerFromInternalField(0);
    std::fstream *fs = reinterpret_cast<std::fstream*>(data);
    fs->flush();

    return args.This();
}

static Handle<Value> stream_next(const Arguments& args)
{
    if (args.Length() != 0)
        return ThrowException(String::New("Exception: Stream.next() accepts no argument"));

    void *data = args.This()->GetPointerFromInternalField(0);
    std::fstream *fs = reinterpret_cast<std::fstream*>(data);

    if (fs->eof())
        return ThrowException(String::New("Exception: Stream.next() reaches end of file"));

    std::string buffer;
    std::getline(*fs, buffer);

    return String::New(buffer.c_str());
}

static Handle<Value> stream_readLine(const Arguments& args)
{
    if (args.Length() != 0)
        return ThrowException(String::New("Exception: Stream.readLine() accepts no argument"));

    void *data = args.This()->GetPointerFromInternalField(0);
    std::fstream *fs = reinterpret_cast<std::fstream*>(data);

    if (fs->eof())
        return String::NewSymbol("");

    std::string buffer;
    std::getline(*fs, buffer);
    if (!fs->eof())
        buffer.append("\n");

    return String::New(buffer.c_str());
}

static Handle<Value> stream_writeLine(const Arguments& args)
{
    if (args.Length() != 1)
        return ThrowException(String::New("Exception: Stream.writeLine() accepts 1 argument"));

    void *data = args.This()->GetPointerFromInternalField(0);
    std::fstream *fs = reinterpret_cast<std::fstream*>(data);

    String::Utf8Value line(args[0]);
    fs->write(*line, line.length());
    fs->put('\n');

    return args.This();
}

static Handle<Value> system_print(const Arguments& args)
{
    HandleScope handle_scope;

    for (int i = 0; i < args.Length(); i++) {
        String::Utf8Value value(args[i]);
        std::cout << *value;
        if (i < args.Length() - 1)
            std::cout << ' ';
    }
    std::cout << std::endl;

    return Undefined();
}

static Handle<Value> system_exit(const Arguments& args)
{
    HandleScope handle_scope;

    if (args.Length() != 0 && args.Length() != 1)
        return ThrowException(String::New("Exception: function system.exit() accepts 1 argument"));

    int status = (args.Length() == 1) ? args[0]->Int32Value() : 0;
    exit(status);

    return Undefined();
}

