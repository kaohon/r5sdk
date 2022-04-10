//===========================================================================//
//
// Purpose: Implementation of the CMemory class.
//
//===========================================================================//
#include "core/stdafx.h"
#include "public/include/utility.h"
#include "public/include/memaddr.h"

//-----------------------------------------------------------------------------
// Purpose: check array of opcodes starting from current address
// Input  : vOpcodeArray - 
// Output : true if equal, false otherwise
//-----------------------------------------------------------------------------
bool CMemory::CheckOpCodes(const vector<uint8_t> vOpcodeArray) const
{
	uintptr_t ref = ptr;

	// Loop forward in the ptr class member.
	for (auto [byteAtCurrentAddress, i] = std::tuple{ uint8_t(), (size_t)0 }; i < vOpcodeArray.size(); i++, ref++)
	{
		byteAtCurrentAddress = *reinterpret_cast<uint8_t*>(ref);

		// If byte at ptr doesn't equal in the byte array return false.
		if (byteAtCurrentAddress != vOpcodeArray[i])
			return false;
	}

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: patch array of opcodes starting from current address
// Input  : vOpcodeArray - 
//-----------------------------------------------------------------------------
void CMemory::Patch(vector<uint8_t> vOpcodes) const
{
	DWORD oldProt = NULL;

	SIZE_T dwSize = vOpcodes.size();
	VirtualProtect(reinterpret_cast<void*>(ptr), dwSize, PAGE_EXECUTE_READWRITE, &oldProt); // Patch page to be able to read and write to it.

	for (int i = 0; i < vOpcodes.size(); i++)
	{
		*reinterpret_cast<uint8_t*>(ptr + i) = vOpcodes[i]; // Write opcodes to Address.
	}

	dwSize = vOpcodes.size();
	VirtualProtect((void*)ptr, dwSize, oldProt, &oldProt); // Restore protection.
}

//-----------------------------------------------------------------------------
// Purpose: patch string constant at current address
// Input  : &svString - 
//-----------------------------------------------------------------------------
void CMemory::PatchString(const string& svString) const
{
	DWORD oldProt = NULL;

	SIZE_T dwSize = svString.size();
	vector<char> bytes(svString.begin(), svString.end());

	VirtualProtect(reinterpret_cast<void*>(ptr), dwSize, PAGE_EXECUTE_READWRITE, &oldProt); // Patch page to be able to read and write to it.

	for (int i = 0; i < svString.size(); i++)
	{
		*reinterpret_cast<uint8_t*>(ptr + i) = bytes[i]; // Write string to Address.
	}

	dwSize = svString.size();
	VirtualProtect(reinterpret_cast<void*>(ptr), dwSize, oldProt, &oldProt); // Restore protection.
}

//-----------------------------------------------------------------------------
// Purpose: find array of bytes in process memory
// Input  : *szPattern - 
//			searchDirect - 
//			nSize - 
//			occurence - 
// Output : CMemory
//-----------------------------------------------------------------------------
CMemory CMemory::FindPattern(const string& svPattern, const Direction searchDirect, const int nSize, const ptrdiff_t occurence) const
{
	uint8_t* ScanBytes = reinterpret_cast<uint8_t*>(ptr); // Get the base of the module.

	const vector<int> PatternBytes = PatternToBytes(svPattern); // Convert our pattern to a byte array.
	const pair BytesInfo = std::make_pair(PatternBytes.size(), PatternBytes.data()); // Get the size and data of our bytes.
	ptrdiff_t occurences = 0;

	for (long i = 01; i < nSize + BytesInfo.first; i++)
	{
		bool bFound = true;

		int memOffset = searchDirect == Direction::DOWN ? i : -i;

		for (DWORD j = 0ul; j < BytesInfo.first; j++)
		{
			// If either the current byte equals to the byte in our pattern or our current byte in the pattern is a wildcard
			// our if clause will be false.
			uint8_t currentByte = *(ScanBytes + memOffset + j);
			if (currentByte != BytesInfo.second[j] && BytesInfo.second[j] != -1)
			{
				bFound = false;
				break;
			}
		}

		if (bFound)
		{
			occurences++;
			if (occurence == occurences)
			{
				return CMemory(&*(ScanBytes + memOffset));
			}
		}
	}

	return CMemory();
}

//-----------------------------------------------------------------------------
// Purpose: find array of bytes in process memory starting from current address
// Input  : *szPattern - 
//			searchDirect - 
//			nSize - 
//			occurence - 
// Output : CMemory
//-----------------------------------------------------------------------------
CMemory CMemory::FindPatternSelf(const string& svPattern, const Direction searchDirect, const int opCodesToScan, const ptrdiff_t occurence)
{
	uint8_t* pScanBytes = reinterpret_cast<uint8_t*>(ptr); // Get the base of the module.

	const vector<int> PatternBytes = PatternToBytes(svPattern); // Convert our pattern to a byte array.
	const pair bytesInfo = std::make_pair(PatternBytes.size(), PatternBytes.data()); // Get the size and data of our bytes.
	ptrdiff_t occurences = 0;

	for (long i = 01; i < opCodesToScan + bytesInfo.first; i++)
	{
		bool bFound = true;
		int nMemOffset = searchDirect == Direction::DOWN ? i : -i;

		for (DWORD j = 0ul; j < bytesInfo.first; j++)
		{
			// If either the current byte equals to the byte in our pattern or our current byte in the pattern is a wildcard
			// our if clause will be false.
			uint8_t currentByte = *(pScanBytes + nMemOffset + j);
			if (currentByte != bytesInfo.second[j] && bytesInfo.second[j] != -1)
			{
				bFound = false;
				break;
			}
		}

		if (bFound)
		{
			occurences++;
			if (occurence == occurences)
			{
				ptr = uintptr_t(&*(pScanBytes + nMemOffset));
				return *this;
			}
		}
	}

	ptr = uintptr_t();
	return *this;
}

//-----------------------------------------------------------------------------
// Purpose: ResolveRelativeAddress wrapper
// Input  : opcodeOffset - 
//			nextInstructionOffset - 
// Output : CMemory
//-----------------------------------------------------------------------------
CMemory CMemory::FollowNearCall(ptrdiff_t opcodeOffset, ptrdiff_t nextInstructionOffset) const
{
	return ResolveRelativeAddress(opcodeOffset, nextInstructionOffset);
}

//-----------------------------------------------------------------------------
// Purpose: ResolveRelativeAddressSelf wrapper
// Input  : opcodeOffset - 
//			nextInstructionOffset - 
// Output : CMemory
//-----------------------------------------------------------------------------
CMemory CMemory::FollowNearCallSelf(ptrdiff_t opcodeOffset, ptrdiff_t nextInstructionOffset)
{
	return ResolveRelativeAddressSelf(opcodeOffset, nextInstructionOffset);
}

//-----------------------------------------------------------------------------
// Purpose: resolves the relative pointer to offset
// Input  : registerOffset - 
//			nextInstructionOffset - 
// Output : CMemory
//-----------------------------------------------------------------------------
CMemory CMemory::ResolveRelativeAddress(ptrdiff_t registerOffset, ptrdiff_t nextInstructionOffset) const
{
	// Skip register.
	uintptr_t skipRegister = ptr + registerOffset;

	// Get 4-byte long relative Address.
	int32_t relativeAddress = *reinterpret_cast<int32_t*>(skipRegister);

	// Get location of next instruction.
	uintptr_t nextInstruction = ptr + nextInstructionOffset;

	// Get function location via adding relative Address to next instruction.
	return CMemory(nextInstruction + relativeAddress);
}

//-----------------------------------------------------------------------------
// Purpose: resolves the relative pointer to offset from current address
// Input  : registerOffset - 
//			nextInstructionOffset - 
// Output : CMemory
//-----------------------------------------------------------------------------
CMemory CMemory::ResolveRelativeAddressSelf(ptrdiff_t registerOffset, ptrdiff_t nextInstructionOffset)
{
	// Skip register.
	uintptr_t skipRegister = ptr + registerOffset;

	// Get 4-byte long relative Address.
	int32_t relativeAddress = *reinterpret_cast<int32_t*>(skipRegister);

	// Get location of next instruction.
	uintptr_t nextInstruction = ptr + nextInstructionOffset;

	// Get function location via adding relative Address to next instruction.
	ptr = nextInstruction + relativeAddress;
	return *this;
}