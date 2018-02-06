#include<cppsharp.h>
#include<string>
#include<Header.h>

namespace Work
{

class Test
{
	enum enum_test_private
	{
		foo = 123,
		bar = 456,
		baz
	};

	TMP __tmp;
	
public:
	enum enum_test_public
	{
		no = 123,
		yes = 456,
		maybe
	};
	
	Test() {
		get_set_a = 1000;
		get_set_b = 1001;
		VarA = 1002;
		VarB = 1003;
		VarD = 1004;
	}

	void charTinput(const char*) {}
	
	// test all the data types sizes
	#define SIZEOFFUNC1(x) int sizeOf ## x () { \
		return sizeof(x); }
	#define SIZEOFFUNC2(x, y) int sizeOf ## x ## y () { \
		return sizeof(x y); }
	#define SIZEOFFUNC3(x, y, z) int sizeOf ## x ## y ## z() { \
		return sizeof(x y z); }
	SIZEOFFUNC1(char);
	SIZEOFFUNC2(unsigned,char);
	SIZEOFFUNC1(short);
	SIZEOFFUNC2(unsigned,short);
	SIZEOFFUNC1(int);
	SIZEOFFUNC2(unsigned,int);
	SIZEOFFUNC2(long,long);
	SIZEOFFUNC3(unsigned,long,long);

	// test the fundamental types as arguements
	#define INPUTFUNC1(x, i) int inputTest(x) { return i; }
	#define INPUTFUNC2(x, y, i) int inputTest(x y) { return i; }
	#define INPUTFUNC3(x, y, z, i) int inputTest(x y z) { return i; }
	INPUTFUNC1(char, 0);
	INPUTFUNC2(unsigned,char, 1);
	INPUTFUNC1(short, 2);
	INPUTFUNC2(unsigned,short, 3);
	INPUTFUNC1(int, 4);
	INPUTFUNC2(unsigned,int, 5);
	INPUTFUNC2(long,long, 6);
	INPUTFUNC3(unsigned,long,long, 7)

	// input class by value, pointer, ref
	#define INPUTFUNCCLASS(x, i) int inputTest##i(x ptr) { return i; }
	INPUTFUNCCLASS(Test, 8);
	#define INPUTFUNCCLASSP(x, i) int inputTest##i(x ptr) { return i; }
	INPUTFUNCCLASSP(Test*, 9);
	INPUTFUNCCLASS(Test&, 10);
	
	

	// test the fundamental types as arguements, by ref
	#define INPUTFUNC1REF(x, i) int inputTestRef(x& var) { var=i; return i; }
	#define INPUTFUNC2REF(x, y, i) int inputTestRef(x y& var) { var=i; return i; }
	#define INPUTFUNC3REF(x, y, z, i) int inputTestRef(x y z& var) { var=i; return i; }
	INPUTFUNC1REF(char, 11);
	INPUTFUNC2REF(unsigned,char, 12);
	INPUTFUNC1REF(short, 13);
	INPUTFUNC2REF(unsigned,short, 14);
	INPUTFUNC1REF(int, 15);
	INPUTFUNC2REF(unsigned,int, 16);
	INPUTFUNC2REF(long,long, 17);
	INPUTFUNC3REF(unsigned,long,long, 18);

	// test the fundamental types as arguements, by pointer
	#define INPUTFUNC1PTR(x, i) int inputTestPtr(x* var) { *var=i; return i; }
	#define INPUTFUNC2PTR(x, y, i) int inputTestPtr(x y* var) { *var=i; return i; }
	#define INPUTFUNC3PTR(x, y, z, i) int inputTestPtr(x y z* var) { *var=i; return i; }
	INPUTFUNC1PTR(char, 19);
	INPUTFUNC2PTR(unsigned,char, 20);
	INPUTFUNC1PTR(short, 21);
	INPUTFUNC2PTR(unsigned,short, 22);
	INPUTFUNC1PTR(int, 23);
	INPUTFUNC2PTR(unsigned,int, 24);
	INPUTFUNC2PTR(long,long, 25);
	INPUTFUNC3PTR(unsigned,long,long, 26);

	// test an enumeration as arguement by value, reference, pointer
	enum_test_public test_enum;
	enum_test_public enumValue(enum_test_public e) 
		{ test_enum=yes; return yes; }
	
	
	// test get/set generation
	int get_set_a;
	int get_set_b;
	
	__setter(Var)	void setter(int a) { get_set_a = a; };
	__getter(Var)	int getter() { return get_set_a; }
	__getter(A)		int getA() { return get_set_b; }
	__setter(B)		void setB(int b) { get_set_b = b; }
	
	int VarA __setter(VarA) __getter(VarA);
	int VarB __setter(VarC) __getter(VarC);
	int VarD __setter(VarD) __getter(VarE);

	// returning class
	class RetTest {} __export;
	RetTest _internal;
	RetTest retTestValue() { return _internal; }
	RetTest& retTestReference() { return _internal; }
	RetTest* retTestPointer_dtor() { return new RetTest(_internal); }
	__nodtor
	RetTest* retTestPointer_nodtor() { return &_internal; }
	
	// test member operators
	bool operator==(int i) { return i == 27; }
	bool operator!=(int i) { return i != 28; }

	int _indexRet;
	int operator[](int i) { return i+1; }
	
	
	// test global operators
	friend int operator +(Test&, int i) { return i + 29; }
	friend int operator -(Test&, int i) { return i - 30; }
	
	// check if function arguements are distinct
	void distinctArgs(int a, int b, int c) {}
	void notDistinctArgs(int, int, int) {}
	

	// test special cases
	std::string stdStringPassThrough(std::string str) { return str; }
	private:
	std::string _strTemp;
	public:
	std::string stdStringRet() { return _strTemp; }
	void stdStringInput(std::string str) { _strTemp = str; }
	void charInput(const char*) {}

	void testCallbacks()
	{
		cppsharp::Object* obj = new cppsharp::Object("Work.CallbackTest");
		obj->OnCreate();
		obj->OnUpdate();
		
	}
	
	// performance tests
	int callMe() { static int i=0; return i++; }
	void cppFunctionPerformance()
	{
		for(int i=0; i<100000000; i++)
		{
			callMe();
		}
	}
	
	
} __export;

	// test operators
//	bool operator==(Test& a, Test& b) { return false; }
//	bool operator!=(Test& a, Test& b) { return false; }

}
