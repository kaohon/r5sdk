#ifndef SQTYPE_H
#define SQTYPE_H

#define SQ_OK (1)
#define SQ_ERROR (-1)
#define SQ_FAILED(res) (res<0)
#define SQ_SUCCEEDED(res) (res>=0)

#define SQ_SUSPEND_FLAG -666
#define SQ_TAILCALL_FLAG -777
#define DONT_FALL_BACK 666
//#define EXISTS_FALL_BACK -1

#define GET_FLAG_RAW                0x00000001
#define GET_FLAG_DO_NOT_RAISE_ERROR 0x00000002

typedef char SQChar;
typedef float SQFloat;
typedef long SQInteger;
typedef unsigned long SQUnsignedInteger;
typedef void* SQFunctor;

typedef SQUnsignedInteger SQBool;
typedef SQInteger SQRESULT;

typedef int ScriptDataType_t;

typedef struct SQVM* HSQUIRRELVM;
struct SQBufState;

struct SQString;

#define SQOBJECT_REF_COUNTED 0x08000000

typedef enum tagSQObjectType
{
	OT_VECTOR = 0x40000,
	OT_NULL = 0x1000001,
	OT_INTEGER = 0x5000002,
	OT_FLOAT = 0x5000004,
	OT_BOOL = 0x1000008,
	OT_STRING = 0x8000010,
	OT_TABLE = 0xA000020,
	OT_ARRAY = 0x8000040,
	OT_USERDATA = 0xA000080,
	OT_CLOSURE = 0x8000100,
	OT_NATIVECLOSURE = 0x8000200,
	OT_ASSET = 0x8000400,
	OT_USERPOINTER = 0x800,
	OT_THREAD = 0x8001000,
	OT_FUNCPROTO = 0x8002000,
	OT_CLASS = 0x8004000,
	OT_STRUCT = 0x8200000,
	OT_INSTANCE = 0xA008000,
	OT_ENTITY = 0xA400000,
	OT_WEAKREF = 0x8010000,
} SQObjectType;

// does the type keep track of references?
#define ISREFCOUNTED(t) (t & SQOBJECT_REF_COUNTED)

typedef union tagSQObjectValue
{
	struct SQTable* pTable;
	struct SQArray* pArray;
	struct SQClosure* pClosure;
	struct SQGenerator* pGenerator;
	struct SQNativeClosure* pNativeClosure;
	struct SQString* pString;
	struct SQUserData* pUserData;
	int nInteger;
	float fFloat;
	void* pUserPointer;
	struct SQFunctionProto* pFunctionProto;
	struct SQRefCounted* pRefCounted;
	struct SQDelegable* pDelegable;
	struct SQVM* pThread;
	struct SQClass* pClass;
	struct SQInstance* pInstance;
	struct SQWeakRef* pWeakRef;
	unsigned int raw;
} SQObjectValue;

typedef struct tagSQObject
{
	SQObjectType _type;
	SQObjectValue _unVal;
} SQObject;

template<typename T> class sqvector
{
public:
	T* _vals;
	SQUnsignedInteger _size;
	SQUnsignedInteger _allocated;
};

///////////////////////////////////////////////////////////////////////////////
SQRESULT sq_pushroottable(HSQUIRRELVM v);
SQChar* sq_getstring(HSQUIRRELVM v, SQInteger i);
SQInteger sq_getinteger(HSQUIRRELVM v, SQInteger i);
SQRESULT sq_pushroottable(HSQUIRRELVM v);
void sq_pushbool(HSQUIRRELVM v, SQBool b);
void sq_pushstring(HSQUIRRELVM v, const SQChar* string, SQInteger len);
void sq_pushinteger(HSQUIRRELVM v, SQInteger val);
void sq_pushfloat(HSQUIRRELVM v, SQFloat n);
void sq_newarray(HSQUIRRELVM v, SQInteger size);
void sq_newtable(HSQUIRRELVM v);
SQRESULT sq_newslot(HSQUIRRELVM v, SQInteger idx);
SQRESULT sq_arrayappend(HSQUIRRELVM v, SQInteger idx);
SQRESULT sq_pushstructure(HSQUIRRELVM v, const SQChar* name, const SQChar* member, const SQChar* codeclass1, const SQChar* codeclass2);
SQRESULT sq_compilebuffer(HSQUIRRELVM v, SQBufState* bufferState, const SQChar* buffer, SQInteger context);
SQRESULT sq_call(HSQUIRRELVM v, SQInteger params, SQBool retval, SQBool raiseerror);

SQRESULT sq_startconsttable(HSQUIRRELVM v);
SQRESULT sq_endconsttable(HSQUIRRELVM v);

/* ==== SQUIRREL ======================================================================================================================================================== */
inline SQRESULT(*v_sq_pushroottable)(HSQUIRRELVM v);
inline void(*v_sq_pushbool)(HSQUIRRELVM v, SQBool b);
inline void(*v_sq_pushstring)(HSQUIRRELVM v, const SQChar* string, SQInteger len);
inline void(*v_sq_pushinteger)(HSQUIRRELVM v, SQInteger val);
inline void(*v_sq_pushfloat)(HSQUIRRELVM v, SQFloat val);
inline void(*v_sq_newarray)(HSQUIRRELVM v, SQInteger size);
inline void(*v_sq_newtable)(HSQUIRRELVM v);
inline SQRESULT(*v_sq_newslot)(HSQUIRRELVM v, SQInteger idx);
inline SQRESULT(*v_sq_arrayappend)(HSQUIRRELVM v, SQInteger idx);
inline SQRESULT(*v_sq_pushstructure)(HSQUIRRELVM v, const SQChar* name, const SQChar* member, const SQChar* codeclass1, const SQChar* codeclass2);
inline SQRESULT(*v_sq_compilebuffer)(HSQUIRRELVM v, SQBufState* bufferstate, const SQChar* buffer, SQInteger level);
inline SQRESULT(*v_sq_call)(HSQUIRRELVM v, SQInteger params, SQBool retval, SQBool raiseerror);

inline SQRESULT (*v_sq_startconsttable)(HSQUIRRELVM v);
inline SQRESULT (*v_sq_endconsttable)(HSQUIRRELVM v);

inline SQString* (*v_StringTable__Add)(void* a1, const SQChar* str, SQInteger len);

///////////////////////////////////////////////////////////////////////////////
class VSquirrelAPI : public IDetour
{
	virtual void GetAdr(void) const
	{
		LogFunAdr("sq_pushroottable", v_sq_pushroottable);
		LogFunAdr("sq_pushbool", v_sq_pushbool);
		LogFunAdr("sq_pushstring", v_sq_pushstring);
		LogFunAdr("sq_pushinteger", v_sq_pushinteger);
		LogFunAdr("sq_pushfloat", v_sq_pushfloat);
		LogFunAdr("sq_newarray", v_sq_newarray);
		LogFunAdr("sq_arrayappend", v_sq_arrayappend);
		LogFunAdr("sq_newtable", v_sq_newtable);
		LogFunAdr("sq_newslot", v_sq_newslot);
		LogFunAdr("sq_pushstructure", v_sq_pushstructure);
		LogFunAdr("sq_compilebuffer", v_sq_compilebuffer);
		LogFunAdr("sq_call", v_sq_call);

		LogFunAdr("sq_startconsttable", v_sq_startconsttable);
		LogFunAdr("sq_endconsttable", v_sq_endconsttable);

		LogFunAdr("StringTable::Add", v_StringTable__Add);
	}
	virtual void GetFun(void) const
	{
		g_GameDll.FindPatternSIMD("48 83 EC 28 8B 51 ?? 44 8B C2").GetPtr(v_sq_pushroottable);
		g_GameDll.FindPatternSIMD("48 83 EC 38 33 C0 48 C7 44 24 20 08 ?? ?? 01 48").GetPtr(v_sq_pushbool);
		g_GameDll.FindPatternSIMD("40 56 48 83 EC 30 48 8B F1 48 85 D2 0F 84 8F ??").GetPtr(v_sq_pushstring);
		g_GameDll.FindPatternSIMD("48 83 EC 38 33 C0 48 C7 44 24 20 02 ?? ?? 05 48").GetPtr(v_sq_pushinteger);
		g_GameDll.FindPatternSIMD("48 83 EC 38 8B 51 78 33 C0").GetPtr(v_sq_pushfloat);
		g_GameDll.FindPatternSIMD("48 89 5C 24 08 57 48 83 EC 30 48 8B D9 48 C7 44 24 20 40").GetPtr(v_sq_newarray);
		g_GameDll.FindPatternSIMD("48 89 5C 24 08 57 48 83 EC 30 48 8B D9 48 C7 44 24 20 20").GetPtr(v_sq_newtable);
		g_GameDll.FindPatternSIMD("40 53 48 83 EC 30 44 8B 49 ?? 48 8B D9 41 8B C1").GetPtr(v_sq_newslot);
		g_GameDll.FindPatternSIMD("40 53 48 83 EC 20 8B 41 ?? 48 8B D9 2B 41 ?? 83 F8 02 7D").GetPtr(v_sq_arrayappend);
		g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 74 24 ?? 48 89 7C 24 ?? 55 41 54 41 55 41 56 41 57 48 8B EC 48 83 EC 60 48 8B 59 60").GetPtr(v_sq_pushstructure);
		g_GameDll.FindPatternSIMD("48 89 5C 24 ?? 48 89 6C 24 ?? 48 89 74 24 ?? 57 41 56 41 57 48 83 EC 50 41 8B E9 49 8B F8").GetPtr(v_sq_compilebuffer);
		g_GameDll.FindPatternSIMD("4C 8B DC 49 89 5B 08 49 89 6B 10 49 89 73 18 57 48 83 EC 50 8B F2").GetPtr(v_sq_call);

		g_GameDll.FindPatternSIMD("8B 51 78 4C 8B 49 60 44 8B C2 49 C1 E0 04 4C 03 81 ?? ?? ?? ?? 8D 42 01 89 41 78 41 F7 81 ?? ?? ?? ?? ?? ?? ?? ?? 74 0A 49 8B 81 ?? ?? ?? ?? FF 40 08 41 F7 00 ?? ?? ?? ?? 41 0F 10 81 ?? ?? ?? ?? 74 15").GetPtr(v_sq_startconsttable);
		g_GameDll.FindPatternSIMD("8B 41 78 45 33 C0 FF C8 8B D0 89 41 78 48 C1 E2 04 48 03 91 ?? ?? ?? ?? 8B 02 48 C7 02 ?? ?? ?? ?? 25 ?? ?? ?? ?? 74 15").GetPtr(v_sq_endconsttable);

		g_GameDll.FindPatternSIMD("E8 ?? ?? ?? ?? 41 8D 4D FF").FollowNearCallSelf().GetPtr(v_StringTable__Add);
	}
	virtual void GetVar(void) const { }
	virtual void GetCon(void) const { }
	virtual void Detour(const bool bAttach) const;
};
///////////////////////////////////////////////////////////////////////////////
#endif // SQTYPE_H
