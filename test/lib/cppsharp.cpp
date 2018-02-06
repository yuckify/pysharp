#include "cppsharp.h"

#ifndef _WIN32
#include <unistd.h>
#endif

#include <fstream>
#include <string.h>
#include <iostream>
#include <sstream>

namespace cppsharp {

void __throw_error(bool cond, const char *cond_str, const char *file, int line) {
	if (!cond) {
		ostringstream str;
		str<<"Error: failed condition \"" <<cond_str <<"\" at " <<file <<":" <<line <<std::endl;
		cerr<<str.str();
		throw InvalidArgument(str.str());
//		std::cerr<<"Error: failed condition \"" <<cond_str <<"\" at " <<file <<":" <<line <<std::endl;
	}
}

uint64_t sdbm(std::string str) {
	const unsigned char* tmp = (const unsigned char*)str.c_str();
	uint64_t hash = 0;
	int c = 0;
	while(c = *tmp++)
		hash = c + (hash << 6) + (hash << 16) - hash;
	return hash;
}

bool endsWith(std::string& first, std::string& end) {
	if(end.length() > first.length()) return false;
	std::string::reverse_iterator ita = first.rbegin();
	std::string::reverse_iterator itb = end.rbegin();
	for(ita, itb; itb != end.rend(); ita++, itb++)
		if(*ita != *itb) return false;
	return true;
}

bool endsWith(std::string& first, const char* end) {
	std::string second(end);
	return endsWith(first, second);
}

bool FileExists(std::string filename) {
	std::ifstream file(filename.c_str());
	if(!file.is_open()) return false;
	return true;
}

std::string str(const char* str) {
	return str;
}

#ifdef _WIN32
#define stat _stat
#endif

uint64_t GetFileTime(string file)
{
    uint64_t ret;
    struct stat buf;
    memset(&buf, 0, sizeof(struct stat));
    stat(file.c_str(), &buf);
    ret = buf.st_mtime;
    return ret;
}

atomic<unsigned> Runtime::_init(0);
atomic<unsigned> Runtime::_free(0);

Runtime::Runtime(string dllName, string domainName)
{
	unsigned expected = 0;
    // setup the environment
    if (_init.compare_exchange_strong(expected, 1)) {
        _domain = mono_jit_init (domainName.c_str());
	} else {
        _domain = mono_domain_create();
        mono_domain_set(_domain, true);
    }
    assert(_domain != NULL);
    
    // load the assembly
    _assembly = mono_domain_assembly_open (_domain, dllName.c_str());
    assert(_assembly != NULL);
    _image = mono_assembly_get_image(_assembly);
    assert(_assembly != NULL);
}

Runtime::~Runtime()
{
	unsigned expected = 0;
    if (_free.compare_exchange_strong(expected, 1))
    {
        mono_assembly_close(_assembly);
        mono_domain_free(_domain, true);
    }
}

void Runtime::JitExec(int argc, char **argv) {
	mono_jit_exec(_domain, _assembly, argc, argv);
}

Method::Method(MonoObject* obj, Runtime* runtime, string cl, string func) :
    _obj(obj)
{
    string onCreateName = cl + ":" + func;
    _desc = mono_method_desc_new(onCreateName.c_str(), true);
    assert(_desc != NULL);
    _method = mono_method_desc_search_in_image(_desc, runtime->GetImage());
}

Method::~Method()
{
    mono_method_desc_free(_desc);
}

void Method::call(MonoObject* obj)
{
    if (obj != NULL && _method != NULL)
        mono_runtime_invoke(_method, obj, NULL, NULL);
}

AssemblyManager* AssemblyManager::_inst = NULL;
AssemblyManager::Mode AssemblyManager::_mode = AssemblyManager::Release;

AssemblyManager::~AssemblyManager()
{
    stop();
    while (IsRunning());
}

AssemblyManager::AssemblyManager()
{
//    _engineScriptsDir = "./";
//    _scripts.push_back(_engineScriptsDir + "Vector2.cs");
//    _scripts.push_back(_engineScriptsDir + "Vector3.cs");
//    _scripts.push_back(_engineScriptsDir + "Vector4.cs");
//    _scripts.push_back(_engineScriptsDir + "Quaternion.cs");
//    _scripts.push_back(_engineScriptsDir + "Plane.cs");
//    _scripts.push_back(_engineScriptsDir + "Matrix4x4.cs");
//    _scripts.push_back(_engineScriptsDir + "Mathf.cs");
    
    // setup the mono objects
//    _libList.push_back("Scripts.dll");
//    _libList.push_back("Scripts2.dll");
    _masterLib = "Test.dll";
    // start the thread
    _stopThread = false;
    _threadStopped = false;
    if (_mode == Debug) {
//        int status = pthread_create(&_thread, NULL, ThreadFun, this);
//        assert(status == 0);
    } else {
        _runtime = new Runtime(_masterLib);
	}
}

void AssemblyManager::setMode(Mode mode)
{
    _mode = mode;
}

AssemblyManager& AssemblyManager::instance() {
    if (_inst == NULL) _inst = new AssemblyManager();
    return *_inst;
}

Runtime &AssemblyManager::GetRuntime() {
	return *_runtime;
}

void* AssemblyManager::ThreadFun(void* data)
{
    AssemblyManager* obj = (AssemblyManager*)data;
    obj->Run();
    return NULL;
}

bool AssemblyManager::CheckRebuild()
{
    bool rebuild = false;
    for (unsigned i=0; i<_scripts.size(); i++)
    {
        string file = _scripts[i]._file;
        if (FileExists(file))
        {
            uint64_t oldtime = _scripts[i]._time;
            uint64_t newtime = GetFileTime(file);
            if (newtime > oldtime)
            {
                rebuild = true;
                _scripts[i]._time = GetFileTime(file);
            }
        }
        else
        {
            _scripts[i]._exists = false;
        }
    }
    
    return rebuild;
}

void AssemblyManager::Rebuild()
{
    string _scriptFiles;
    for (unsigned i=0; i<_scripts.size(); i++)
    {
        // check to make sure the file exists
        string file = _scripts[i]._file;
        if (_scripts[i]._exists)
            _scriptFiles += " " + file;
    }
    
    string buildCmd = str("mcs ") + _scriptFiles + " -target:library -out:" + _libList[0];
    
    // rotate the lib files
    _libList.push_back(_libList[0]);
    _libList.erase(_libList.begin());
    
    int status = ::system(buildCmd.c_str());
    if (status != 0) {
        std::cout<<"error building" <<std::endl;
    } else {
        // rebuild all the objects
    }
}

void AssemblyManager::Run()
{
    while (true)
    {
        if (_stopThread) break;
        
        {
            // make sure no one else is operating on the assembly
            unique_lock<mutex> locker(_libLock);
            
            bool rebuild = CheckRebuild();
            if (rebuild) Rebuild();
        } // locker
        
//        sleep(10);
    }
    _threadStopped = true;
}

void AssemblyManager::CreateObject(Object* obj, string class_name)
{
	unique_lock<mutex> locker(_libLock);
    
    // input the a script and its path and the ".cs" extension since its a 
    // c# script, this will trim the extension and the path
//    string classname = script;
//    if (endsWith(script, ".cs"))
//        classname = script.substr(0, script.length() - 3);
//    size_t index;
//    if ((index = classname.find_last_of("/")) != string::npos)
//        classname = classname.substr(index);
	size_t period = class_name.find_last_of(".");
	string cname = class_name.substr(period + 1);
	string cnamespace = class_name.substr(0, period);
    MonoClass* monoclass = mono_class_from_name_case(_runtime->GetImage(), cnamespace.c_str(), cname.c_str());
	throw_error(monoclass);
    MonoObject* instance = mono_object_new (_runtime->GetDomain(), monoclass);
    
    // create the object
    ObjectData* data = new ObjectData;
    data->_class = class_name;
    data->_obj = instance;
    data->_handle = mono_gchandle_new(data->_obj, true);
    data->_onCreateMethod = new Method(data->_obj, _runtime, class_name, "OnCreate");
    data->_onUpdateMethod = new Method(data->_obj, _runtime, class_name, "OnUpdate");
    
    // register the object
    obj->_data = data;
    _objList.push_back(obj);
}

void AssemblyManager::FreeObject(Object *obj)
{
	unique_lock<mutex> locker(_libLock);
    
    vector<Object*>::iterator it = _objList.begin();
    for(it; it != _objList.end(); it++)
    {
        if (*it == obj)
        {
            _objList.erase(it);
            break;
        }
    }
    
    if (obj->_data == NULL) return;
    
    if (obj->_data->_onCreateMethod != NULL)
        delete obj->_data->_onCreateMethod;
    if (obj->_data->_onUpdateMethod != NULL)
        delete obj->_data->_onUpdateMethod;
    
    if (obj->_data->_handle)
        mono_gchandle_free(obj->_data->_handle);
    
    delete obj->_data;
    obj->_data = NULL;
}

Object::Object(string script)
{
    AssemblyManager::instance().CreateObject(this, script);
}

Object::~Object()
{
    AssemblyManager::instance().FreeObject(this);
}

void Object::OnCreate() {
    unique_lock<mutex> locker(_lock);
    _data->_onCreateMethod->call(_data->_obj);
}

void Object::OnUpdate() {
    unique_lock<mutex> locker(_lock);
    _data->_onUpdateMethod->call(_data->_obj);
}

} // namespace cppsharp
