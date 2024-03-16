//=============================================================================//
//
// Purpose: Squirrel API interface to engine
//
//=============================================================================//

#include "core/stdafx.h"
#include "squirrel.h"
#include "sqvm.h"
#include "sqstring.h"

//---------------------------------------------------------------------------------
SQChar* sq_getstring(HSQUIRRELVM v, SQInteger i)
{
	return v->_stackbase[i]._unVal.pString->_val;
}

//---------------------------------------------------------------------------------
SQInteger sq_getinteger(HSQUIRRELVM v, SQInteger i)
{
	return v->_stackbase[i]._unVal.nInteger;
}

//---------------------------------------------------------------------------------
SQRESULT sq_pushroottable(HSQUIRRELVM v)
{
	v->Push(v->_roottable);

	return SQ_OK;
}

//---------------------------------------------------------------------------------
void sq_pushbool(HSQUIRRELVM v, SQBool b)
{
	v->Push(b?true:false);
}

//---------------------------------------------------------------------------------
void sq_pushstring(HSQUIRRELVM v, const SQChar* s, SQInteger len)
{
	if (s)
	{
		SQString* pString = SQString::Create(v->_sharedstate, s, len);
		v->Push(pString);
	}
	else
		v->Push(_null_);
}

//---------------------------------------------------------------------------------
void sq_pushinteger(HSQUIRRELVM v, SQInteger val)
{
	v->Push(val);
}

//---------------------------------------------------------------------------------
void sq_pushfloat(HSQUIRRELVM v, SQFloat n)
{
	v->Push(n);
}

//---------------------------------------------------------------------------------
void sq_newarray(HSQUIRRELVM v, SQInteger size)
{
	v_sq_newarray(v, size);
}

//---------------------------------------------------------------------------------
void sq_newtable(HSQUIRRELVM v)
{
	v_sq_newtable(v);
}

//---------------------------------------------------------------------------------
SQRESULT sq_newslot(HSQUIRRELVM v, SQInteger idx)
{
	return v_sq_newslot(v, idx);
}

//---------------------------------------------------------------------------------
SQRESULT sq_arrayappend(HSQUIRRELVM v, SQInteger idx)
{
	return v_sq_arrayappend(v, idx);
}

//---------------------------------------------------------------------------------
SQRESULT sq_pushstructure(HSQUIRRELVM v, const SQChar* name, const SQChar* member, const SQChar* codeclass1, const SQChar* codeclass2)
{
	return v_sq_pushstructure(v, name, member, codeclass1, codeclass2);
}

//---------------------------------------------------------------------------------
SQRESULT sq_compilebuffer(HSQUIRRELVM v, SQBufState* bufferState, const SQChar* buffer, SQInteger level)
{
	return v_sq_compilebuffer(v, bufferState, buffer, level);
}

//---------------------------------------------------------------------------------
SQRESULT sq_call(HSQUIRRELVM v, SQInteger params, SQBool retval, SQBool raiseerror)
{
	return v_sq_call(v, params, retval, raiseerror);
}

SQRESULT sq_startconsttable(HSQUIRRELVM v)
{
	return v_sq_startconsttable(v);
}

SQRESULT sq_endconsttable(HSQUIRRELVM v)
{
	return v_sq_endconsttable(v);
}

void VSquirrelAPI::Detour(const bool bAttach) const
{
	DetourSetup(&v_sq_pushroottable, &sq_pushroottable, bAttach);
	//DetourSetup(&v_sq_pushbool, &sq_pushbool, bAttach);
	//DetourSetup(&v_sq_pushstring, &sq_pushstring, bAttach);
	//DetourSetup(&v_sq_pushinteger, &sq_pushinteger, bAttach);
	//DetourSetup(&v_sq_pushfloat, &sq_pushfloat, bAttach);
	DetourSetup(&v_sq_newarray, &sq_newarray, bAttach);
	DetourSetup(&v_sq_newtable, &sq_newtable, bAttach);
	DetourSetup(&v_sq_newslot, &sq_newslot, bAttach);
	DetourSetup(&v_sq_arrayappend, &sq_arrayappend, bAttach);
	DetourSetup(&v_sq_pushstructure, &sq_pushstructure, bAttach);
	DetourSetup(&v_sq_compilebuffer, &sq_compilebuffer, bAttach);
	DetourSetup(&v_sq_call, &sq_call, bAttach);

	DetourSetup(&v_sq_startconsttable, &sq_startconsttable, bAttach);
	DetourSetup(&v_sq_endconsttable, &sq_endconsttable, bAttach);
}
