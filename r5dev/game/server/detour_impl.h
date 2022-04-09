#pragma once
#include "thirdparty/recast/detour/include/detourstatus.h"
#include "thirdparty/recast/detour/include/detournavmesh.h"

//-------------------------------------------------------------------------
// RUNTIME: DETOUR
//-------------------------------------------------------------------------
inline ADDRESS p_dtNavMesh__Init = g_mGameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x4C\x89\x44\x24\x00\x53\x41\x56\x48\x81\xEC\x00\x00\x00\x00\x0F\x10\x11"), "xxxx?xxxxxx????xxx");
inline dtStatus (*dtNavMesh__Init)(dtNavMesh* thisptr, unsigned char* data, int flags) = (dtStatus(*)(dtNavMesh*, unsigned char*, int))p_dtNavMesh__Init.GetPtr(); /*4C 89 44 24 ? 53 41 56 48 81 EC ? ? ? ? 0F 10 11*/

inline ADDRESS p_dtNavMesh__addTile = g_mGameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x44\x89\x4C\x24\x00\x41\x55"), "xxxx?xx");/*44 89 4C 24 ? 41 55*/
inline dtStatus(*dtNavMesh__addTile)(dtNavMesh* thisptr, unsigned char* data, dtMeshHeader* header, int datasize, int flags, dtTileRef lastRef) = (dtStatus(*)(dtNavMesh*, unsigned char*, dtMeshHeader*, int, int, dtTileRef))p_dtNavMesh__addTile.GetPtr();

inline ADDRESS p_dtNavMesh__isPolyReachable = g_mGameDll.FindPatternSIMD(reinterpret_cast<rsig_t>("\x48\x89\x6C\x24\x00\x48\x89\x74\x24\x00\x48\x89\x7C\x24\x00\x41\x56\x49\x63\xF1"), "xxxx?xxxx?xxxx?xxxxx"); /*48 89 6C 24 ? 48 89 74 24 ? 48 89 7C 24 ? 41 56 49 63 F1*/
inline bool(*dtNavMesh__isPolyReachable)(dtNavMesh* thisptr, dtPolyRef poly_1, dtPolyRef poly_2, int hull_type) = (bool(*)(dtNavMesh*, dtPolyRef, dtPolyRef, int))p_dtNavMesh__isPolyReachable.GetPtr();

///////////////////////////////////////////////////////////////////////////////
class HRecast : public IDetour
{
	virtual void debugp()
	{
		std::cout << "| FUN: dtNavMesh::Init                      : 0x" << std::hex << std::uppercase << p_dtNavMesh__Init.GetPtr()            << std::setw(npad) << " |" << std::endl;
		std::cout << "| FUN: dtNavMesh::addTile                   : 0x" << std::hex << std::uppercase << p_dtNavMesh__addTile.GetPtr()         << std::setw(npad) << " |" << std::endl;
		std::cout << "| FUN: dtNavMesh::isPolyReachable           : 0x" << std::hex << std::uppercase << p_dtNavMesh__isPolyReachable.GetPtr() << std::setw(npad) << " |" << std::endl;
		std::cout << "+----------------------------------------------------------------+" << std::endl;
	}
};
///////////////////////////////////////////////////////////////////////////////

REGISTER(HRecast);
