#ifndef CPPSHARP_H
#define CPPSHARP_H

#if defined(__castxml__)
	#define __nodtor		__attribute__((annotate("nodtor")))
	#define __import		__attribute__((annotate("import")))
	#define __export		__attribute__((annotate("export")))
	#define __setter(x)		__attribute__((annotate("setter," #x )))
	#define __getter(x)		__attribute__((annotate("getter," #x )))
#elif defined(__clang__)
	#define __nodtor		__attribute__((annotate("nodtor")))
	#define __import		__attribute__((annotate("import")))
	#define __export		__attribute__((annotate("export")))
	#define __setter(x)		__attribute__((annotate("setter," #x )))
	#define __getter(x)		__attribute__((annotate("getter," #x )))
#else
	#define __nodtor
	#define __import
	#define __export
	#define __setter(x)
	#define __getter(x)
#endif

#ifdef _MSC_VER
#include <Windows.h>
#define _GLIBCXX_USE_NOEXCEPT
#endif

#include <string>
#include <vector>
#include <assert.h>
#include <string>
#include <stdint.h>
#include <stdexcept>
#include <stdlib.h>


#include <mono/metadata/object.h>
#include <mono/metadata/class.h>
#include <mono/metadata/assembly.h>
#include <mono/metadata/mono-config.h>
#include <mono/jit/jit.h>
#include <mono/metadata/debug-helpers.h>
#include <mono/metadata/appdomain.h>
#include <sys/stat.h>

#include <thread>
#include <mutex>
#include <atomic>

using namespace std;

namespace cppsharp {

	class InvalidArgument : std::exception {
	public:
		InvalidArgument(const char* str) : _str(str) {}
		InvalidArgument(std::string str) : _str(str) {}
		~InvalidArgument() throw() {}
		const char* what() const _GLIBCXX_USE_NOEXCEPT { return _str.c_str(); }
	private:
		std::string _str;
	};

void __throw_error(bool cond, const char *cond_str, const char *file, int line);
#define throw_error(cond) \
	__throw_error((cond), #cond, __FILE__, __LINE__)

	uint64_t sdbm(std::string str);
	bool endsWith(std::string& first, std::string& end);
	bool endsWith(std::string& first, const char* end);
	bool FileExists(std::string filename);
	std::string str(const char* str);
    uint64_t GetFileTime(string file);
    
    class Object;
    
    class Runtime {
    public:
        Runtime(string dllName, string domainName = "default");
        ~Runtime();
        
        MonoImage* GetImage() { return _image; }
        MonoDomain* GetDomain() { return _domain; }
        void JitExec(int argc, char **argv);
        
    private:
        MonoDomain* _domain;
		MonoAssembly* _assembly;
		MonoImage* _image;
        static std::atomic<unsigned> _init;
        static std::atomic<unsigned> _free;
    }; // class Runtime
    
    class Method {
    public:
        ~Method();
        Method(MonoObject* obj, Runtime* runtime, string cl, string func);
        void call(MonoObject* obj);
        
    private:
        MonoObject* _obj;
        MonoMethod* _method;
		MonoMethodDesc* _desc;
    }; // class Method
    
	class AssemblyManager {
        friend class Object;
	public:
        ~AssemblyManager();

        AssemblyManager();
        
        enum Mode {
            Release,
            Debug
        };
        
        void stop() { _stopThread = true; }
        bool IsRunning() { return !_threadStopped; }
        
        static void setMode(Mode mode);
        static AssemblyManager& instance();
        Runtime &GetRuntime();
        
	private:
        struct ObjectData {
            ObjectData() 
            	: _handle(0), _obj(NULL), _onCreateMethod(NULL), _onUpdateMethod(NULL)
            { }
            
            uint32_t _handle;
            MonoObject* _obj;
            std::string _class;
    
            Method* _onCreateMethod;
            Method* _onUpdateMethod;
        };
        
        struct ScriptInfo {
            ScriptInfo(string file) : _file(file), _time(0), _exists(true) {}
            string _file;
            uint64_t _time;
            bool _exists;
        };
        
        // ctor/dtor for making objects
        void CreateObject(Object* obj, string class_name);
        void FreeObject(Object* obj);
        
        // thread functions for building/rebuilding the assembly
        static void* ThreadFun(void* data);
        void Run();
        bool CheckRebuild();
        void Rebuild();
        
        static AssemblyManager* _inst;
        static Mode _mode;
        
        string _engineScriptsDir;
        vector<ScriptInfo> _scripts;
        vector<string> _libList;
        string _masterLib;
        
        // list of actively use objects, this is needed so when the scripts
        // are recompiled the ObjectData and be swapped out
        vector<Object*> _objList;
        
        // object for generating c# objects, this will be regenerated 
        // periodically when the scripts are edited
        Runtime* _runtime;
        
        // variables for the thread that handles loading/compiling the assembly
        std::atomic<int> _stopThread;
        std::atomic<int> _threadStopped;
        
        // used to control access to the assembly, Object will increment the
        // use count in the Semaphore the worker thread will lock the assembly
        // using the mutex
		mutex _libLock;
	}; // class AssemblyManager


	class Object {
        friend class AssemblyManager;
	public:
        Object(string script);
        ~Object();

		void OnCreate();
		void OnUpdate();
        
        string ClassName() const { return _data->_class; }
        
	private:
        AssemblyManager::ObjectData* _data;
        mutex _lock;
	}; // class Object
	
} // namespace cppsharp

#endif

