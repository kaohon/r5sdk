﻿//=============================================================================//
//
// Purpose: Valve Pak utility class.
//
//=============================================================================//
// packedstore.cpp
//
// Note: VPK's are created in pairs of a directory file and pack file(s).
// - <locale><target>_<level>.bsp.pak000_dir.vpk --> directory file.
// - <target>_<level>.bsp.pak000_<patch>.vpk --> pack file.
// 
// - Assets larger than 1MiB are fragmented into chunks of 1MiB or smaller (ENTRY_MAX_LEN).
// - A VPK directory file could be patched up to 512 times before full rebuild is required.
// 
/////////////////////////////////////////////////////////////////////////////////
#include "core/stdafx.h"
#include "tier1/cvar.h"
#include "mathlib/adler32.h"
#include "mathlib/crc32.h"
#include "mathlib/sha1.h"
#include "vpklib/packedstore.h"

//-----------------------------------------------------------------------------
// Static buffers for chunking/compressing the source files and decompressing
//-----------------------------------------------------------------------------
static uint8_t s_EntryBuf[ENTRY_MAX_LEN];
static uint8_t s_DecompBuf[ENTRY_MAX_LEN];

//-----------------------------------------------------------------------------
// Purpose: initialize parameters for compression algorithm
//-----------------------------------------------------------------------------
void CPackedStore::InitLzCompParams(void)
{
	/*| PARAMETERS ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||*/
	m_lzCompParams.m_dict_size_log2     = VPK_DICT_SIZE;
	m_lzCompParams.m_level              = GetCompressionLevel();
	m_lzCompParams.m_compress_flags     = lzham_compress_flags::LZHAM_COMP_FLAG_DETERMINISTIC_PARSING;
	m_lzCompParams.m_max_helper_threads = -1;
}

//-----------------------------------------------------------------------------
// Purpose: initialize parameters for decompression algorithm
//-----------------------------------------------------------------------------
void CPackedStore::InitLzDecompParams(void)
{
	/*| PARAMETERS ||||||||||||||||||||||||||||||||||||||||||||||||||||||||||*/
	m_lzDecompParams.m_dict_size_log2   = VPK_DICT_SIZE;
	m_lzDecompParams.m_decompress_flags = lzham_decompress_flags::LZHAM_DECOMP_FLAG_OUTPUT_UNBUFFERED | lzham_decompress_flags::LZHAM_DECOMP_FLAG_COMPUTE_CRC32;
	m_lzDecompParams.m_struct_size      = sizeof(lzham_decompress_params);
}

//-----------------------------------------------------------------------------
// Purpose: gets a directory structure for specified file.
// Input  : svPackDirFile - 
//          bSanitizeName - retrieve the directory file name from block name
// Output : VPKDir_t
//-----------------------------------------------------------------------------
VPKDir_t CPackedStore::GetDirectoryFile(const string& svPackDirFile, bool bSanitizeName) const
{
	if (!bSanitizeName)
		return VPKDir_t(svPackDirFile);

	std::smatch smRegexMatches;
	std::regex_search(svPackDirFile, smRegexMatches, BLOCK_REGEX);

	if (smRegexMatches.empty())
		return VPKDir_t(svPackDirFile);

	string svSanitizedName = svPackDirFile;
	StringReplace(svSanitizedName, smRegexMatches[0], "pak000_dir");

	bool bHasLocale = false;
	for (const string& svLocale : DIR_LOCALE)
	{
		if (svSanitizedName.find(svLocale) != string::npos)
		{
			bHasLocale = true;
			break;
		}
	}

	if (!bHasLocale) // Only sanitize if no locale was provided.
	{
		string svPackDirPrefix;
		svPackDirPrefix.append(DIR_LOCALE[0]);

		for (const string& svTarget : DIR_TARGET)
		{
			if (svSanitizedName.find(svTarget) != string::npos)
			{
				svPackDirPrefix.append(svTarget);
				StringReplace(svSanitizedName, svTarget, svPackDirPrefix);

				break;
			}
		}
	}

	return VPKDir_t(svSanitizedName);
}

//-----------------------------------------------------------------------------
// Purpose: formats pack file path for specific directory file
// Input  : &svPackDirFile - 
//          iPackFileIndex - 
// output : string
//-----------------------------------------------------------------------------
string CPackedStore::GetPackFile(const string& svPackDirFile, uint16_t iPackFileIndex) const
{
	string svPackChunkFile = StripLocalePrefix(svPackDirFile);
	ostringstream oss;

	oss << std::setw(3) << std::setfill('0') << iPackFileIndex;
	string svPackChunkIndex = "pak000_" + oss.str();

	StringReplace(svPackChunkFile, "pak000_dir", svPackChunkIndex);
	return svPackChunkFile;
}

//-----------------------------------------------------------------------------
// Purpose: gets the LZHAM compression level
// output : lzham_compress_level
//-----------------------------------------------------------------------------
lzham_compress_level CPackedStore::GetCompressionLevel(void) const
{
	const char* pszLevel = fs_packedstore_compression_level->GetString();

	if(strcmp(pszLevel, "fastest") == NULL)
		return lzham_compress_level::LZHAM_COMP_LEVEL_FASTEST;
	else if (strcmp(pszLevel, "faster") == NULL)
		return lzham_compress_level::LZHAM_COMP_LEVEL_FASTER;
	else if (strcmp(pszLevel, "default") == NULL)
		return lzham_compress_level::LZHAM_COMP_LEVEL_DEFAULT;
	else if (strcmp(pszLevel, "better") == NULL)
		return lzham_compress_level::LZHAM_COMP_LEVEL_BETTER;
	else if (strcmp(pszLevel, "uber") == NULL)
		return lzham_compress_level::LZHAM_COMP_LEVEL_UBER;
	else
		return lzham_compress_level::LZHAM_COMP_LEVEL_DEFAULT;
}

//-----------------------------------------------------------------------------
// Purpose: obtains and returns the entry block to the vector
// Input  : *pReader - 
// output : vector<VPKEntryBlock_t>
//-----------------------------------------------------------------------------
vector<VPKEntryBlock_t> CPackedStore::GetEntryBlocks(CIOStream* pReader) const
{
	string svName, svPath, svExtension;
	vector<VPKEntryBlock_t> vBlocks;
	while (!(svExtension = pReader->ReadString()).empty())
	{
		while (!(svPath = pReader->ReadString()).empty())
		{
			while (!(svName = pReader->ReadString()).empty())
			{
				const string svFilePath = FormatEntryPath(svPath, svName, svExtension);
				vBlocks.push_back(VPKEntryBlock_t(pReader, svFilePath));
			}
		}
	}
	return vBlocks;
}

//-----------------------------------------------------------------------------
// Purpose: scans the input directory and returns the paths to the vector
// Input  : &svPathIn - 
// Output : a string vector of all included entry paths
//-----------------------------------------------------------------------------
vector<string> CPackedStore::GetEntryPaths(const string& svPathIn) const
{
	vector<string> vPaths;
	vector<string> vIgnore = GetIgnoreList(svPathIn);

	fs::recursive_directory_iterator dir(svPathIn), end;
	while (dir != end)
	{
		const vector<string>::iterator it = std::find(vIgnore.begin(), vIgnore.end(),
			GetExtension(dir->path().filename().u8string(), true, true));
		if (it != vIgnore.end())
		{
			dir.disable_recursion_pending(); // Skip all ignored folders and extensions.
		}
		else if (dir->file_size() > 0) // Empty files are not supported.
		{
			const string svPath = dir->path().u8string();
			if (!GetExtension(svPath).empty())
			{
				vPaths.push_back(ConvertToUnixPath(svPath));
			}
		}
		dir++;
	}
	return vPaths;
}

//-----------------------------------------------------------------------------
// Purpose: scans the input directory and returns the paths to the vector if path exists in manifest
// Input  : &svPathIn - 
//          &jManifest - 
// Output : a string vector of all included entry paths
//-----------------------------------------------------------------------------
vector<string> CPackedStore::GetEntryPaths(const string& svPathIn, const nlohmann::json& jManifest) const
{
	vector<string> vPaths;
	vector<string> vIgnore = GetIgnoreList(svPathIn);

	fs::recursive_directory_iterator dir(svPathIn), end;
	while (dir != end)
	{
		const vector<string>::iterator it = std::find(vIgnore.begin(), vIgnore.end(), 
			GetExtension(dir->path().filename().u8string(), true, true));
		if (it != vIgnore.end())
		{
			dir.disable_recursion_pending(); // Skip all ignored folders and extensions.
		}
		else if (dir->file_size() > 0) // Empty files are not supported.
		{
			const string svPath = dir->path().u8string();
			if (!GetExtension(svPath).empty())
			{
				if (!jManifest.is_null())
				{
					try
					{
						const string svEntryPath = ConvertToUnixPath(svPath);
						if (jManifest.contains(StringReplaceC(svEntryPath, svPathIn, "")))
						{
							vPaths.push_back(svEntryPath);
						}
					}
					catch (const std::exception& ex)
					{
						Warning(eDLL_T::FS, "Exception while reading VPK control file: '%s'\n", ex.what());
					}
				}
			}
		}
		dir++;
	}
	return vPaths;
}

//-----------------------------------------------------------------------------
// Purpose: gets the parts of the directory file name
// Input  : &svDirectoryName - 
//          nCaptureGroup - (1 = locale + target, 2 = level)
// Output : part of directory file name as string
//-----------------------------------------------------------------------------
string CPackedStore::GetNameParts(const string& svDirectoryName, int nCaptureGroup) const
{
	std::smatch smRegexMatches;
	std::regex_search(svDirectoryName, smRegexMatches, DIR_REGEX);

	return smRegexMatches[nCaptureGroup].str();
}

//-----------------------------------------------------------------------------
// Purpose: gets the level name from the directory file name
// Input  : &svDirectoryName - 
// Output : level name as string (e.g. "mp_rr_box")
//-----------------------------------------------------------------------------
string CPackedStore::GetLevelName(const string& svDirectoryName) const
{
	std::smatch smRegexMatches;
	std::regex_search(svDirectoryName, smRegexMatches, DIR_REGEX);

	return smRegexMatches[1].str() + smRegexMatches[2].str();
}

//-----------------------------------------------------------------------------
// Purpose: gets the manifest file associated with the VPK name
// Input  : &svWorkspace - 
//          &svManifestName - 
// Output : parsed manifest as json
//-----------------------------------------------------------------------------
nlohmann::json CPackedStore::GetManifest(const string& svWorkspace, const string& svManifestName) const
{
	ostringstream ostream;
	ostream << svWorkspace << "manifest/" << svManifestName << ".json";
	fs::path fsPath = fs::current_path() /= ostream.str();
	nlohmann::json jsOut;

	if (fs::exists(fsPath))
	{
		try
		{
			ifstream iManifest(fsPath.u8string(), std::ios::binary);
			jsOut = nlohmann::json::parse(iManifest);

			return jsOut;
		}
		catch (const std::exception& ex)
		{
			Warning(eDLL_T::FS, "Exception while parsing VPK control file: '%s'\n", ex.what());
			return jsOut;
		}
	}
	return jsOut;
}

//-----------------------------------------------------------------------------
// Purpose: gets the contents from the global ignore list (.vpkignore)
// Input  : &svWorkspace - 
// Output : a string vector of ignored directories/files and extensions
//-----------------------------------------------------------------------------
vector<string> CPackedStore::GetIgnoreList(const string& svWorkspace) const
{
	fs::path fsIgnore = svWorkspace + ".vpkignore";
	ifstream iStream(fsIgnore);

	vector<string> vIgnore;
	if (iStream)
	{
		string svIgnore;
		while (std::getline(iStream, svIgnore))
		{
			string::size_type nPos = svIgnore.find("//");
			if (nPos == string::npos)
			{
				if (!svIgnore.empty() && 
					std::find(vIgnore.begin(), vIgnore.end(), svIgnore) == vIgnore.end())
				{
					vIgnore.push_back(svIgnore);
				}
			}
		}
	}
	return vIgnore;
}

//-----------------------------------------------------------------------------
// Purpose: formats the file entry path
// Input  : svPath - 
//          &svName - 
//          &svExtension - 
// Output : formatted entry path
//-----------------------------------------------------------------------------
string CPackedStore::FormatEntryPath(string svPath, const string& svName, const string& svExtension) const
{
	if (!svPath.empty())
	{
		svPath += '/';
	}
	return svPath + svName + '.' + svExtension;
}

//-----------------------------------------------------------------------------
// Purpose: strips locale prefix from file path
// Input  : &svDirectoryFile - 
// Output : directory filename without locale prefix
//-----------------------------------------------------------------------------
string CPackedStore::StripLocalePrefix(const string& svDirectoryFile) const
{
	fs::path fsDirectoryFile(svDirectoryFile);
	string svFileName = fsDirectoryFile.filename().u8string();

	for (const string& svLocale : DIR_LOCALE)
	{
		if (svFileName.find(svLocale) != string::npos)
		{
			StringReplace(svFileName, svLocale, "");
			break;
		}
	}
	return svFileName;
}

//-----------------------------------------------------------------------------
// Purpose: builds a valid file name for the VPK
// Input  : svLanguage - 
//          svTarget - 
//          &svLevel - 
//          nPatch - 
// Output : a vpk file pair (block and directory file names)
//-----------------------------------------------------------------------------
VPKPair_t CPackedStore::BuildFileName(string svLanguage, string svTarget, const string& svLevel, int nPatch) const
{
	if (std::find(DIR_LOCALE.begin(), DIR_LOCALE.end(), svLanguage) == DIR_LOCALE.end())
	{
		svLanguage = DIR_LOCALE[0];
	}
	if (std::find(DIR_TARGET.begin(), DIR_TARGET.end(), svTarget) == DIR_TARGET.end())
	{
		svTarget = DIR_TARGET[0];
	}

	VPKPair_t vPair;
	vPair.m_svBlockName = fmt::format("{:s}_{:s}.bsp.pak000_{:03d}{:s}", svTarget, svLevel, nPatch, ".vpk");
	vPair.m_svDirectoryName = fmt::format("{:s}{:s}_{:s}.bsp.pak000_{:s}", svLanguage, svTarget, svLevel, "dir.vpk");

	return vPair;
}

//-----------------------------------------------------------------------------
// Purpose: builds the VPK manifest file
// Input  : &vBlock - 
//          &svWorkSpace - 
//          &svManifestName - 
//-----------------------------------------------------------------------------
void CPackedStore::BuildManifest(const vector<VPKEntryBlock_t>& vBlock, const string& svWorkSpace, const string& svManifestName) const
{
	nlohmann::json jEntry;

	for (const VPKEntryBlock_t& vEntry : vBlock)
	{
		const VPKChunkDescriptor_t& vDescriptor = vEntry.m_vFragments[0];
		jEntry[vEntry.m_svEntryPath] =
		{
			{ "preloadSize", vEntry.m_iPreloadSize },
			{ "loadFlags", vDescriptor.m_nLoadFlags },
			{ "textureFlags", vDescriptor.m_nTextureFlags },
			{ "useCompression", vDescriptor.m_nCompressedSize != vDescriptor.m_nUncompressedSize },
			{ "useDataSharing", true }
		};
	}

	string svPathOut = svWorkSpace + "manifest/";
	fs::create_directories(svPathOut);

	ofstream oManifest(svPathOut + svManifestName + ".json");
	oManifest << jEntry.dump(4);
}

//-----------------------------------------------------------------------------
// Purpose: validates extraction result with precomputed ADLER32 hash
// Input  : &svAssetFile - 
//-----------------------------------------------------------------------------
void CPackedStore::ValidateAdler32PostDecomp(const string& svAssetFile)
{
	CIOStream reader(svAssetFile, CIOStream::Mode_t::READ);
	m_nAdler32 = adler32::update(NULL, reader.GetData(), reader.GetSize());

	if (m_nAdler32 != m_nAdler32_Internal)
	{
		Warning(eDLL_T::FS, "Computed checksum '0x%lX' doesn't match expected checksum '0x%lX'. File may be corrupt!\n", m_nAdler32, m_nAdler32_Internal);
		m_nAdler32          = NULL;
		m_nAdler32_Internal = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: validates extraction result with precomputed CRC32 hash
// Input  : &svAssetFile - 
//-----------------------------------------------------------------------------
void CPackedStore::ValidateCRC32PostDecomp(const string& svAssetFile)
{
	CIOStream reader(svAssetFile, CIOStream::Mode_t::READ);
	m_nCrc32 = crc32::update(NULL, reader.GetData(), reader.GetSize());

	if (m_nCrc32 != m_nCrc32_Internal)
	{
		Warning(eDLL_T::FS, "Computed checksum '0x%lX' doesn't match expected checksum '0x%lX'. File may be corrupt!\n", m_nCrc32, m_nCrc32_Internal);
		m_nCrc32          = NULL;
		m_nCrc32_Internal = NULL;
	}
}

//-----------------------------------------------------------------------------
// Purpose: packs all files from workspace path into VPK file
// Input  : &vPair - 
//          &svWorkspace - 
//          &svBuildPath - 
//          bManifestOnly - 
//-----------------------------------------------------------------------------
void CPackedStore::PackWorkspace(const VPKPair_t& vPair, const string& svWorkspace, const string& svBuildPath, bool bManifestOnly)
{
	CIOStream writer(svBuildPath + vPair.m_svBlockName, CIOStream::Mode_t::WRITE);

	vector<string> vPaths;
	vector<VPKEntryBlock_t> vEntryBlocks;
	const nlohmann::json jManifest = GetManifest(svWorkspace, GetLevelName(vPair.m_svDirectoryName));

	GetIgnoreList(svWorkspace);

	if (bManifestOnly)
	{
		vPaths = GetEntryPaths(svWorkspace, jManifest);
	}
	else // Pack all files in workspace.
	{
		vPaths = GetEntryPaths(svWorkspace);
	}

	uint64_t nSharedTotal = 0;
	uint32_t nSharedCount = 0;

	for (size_t i = 0, ps = vPaths.size(); i < ps; i++)
	{
		const string& svPath = vPaths[i];
		CIOStream reader(svPath, CIOStream::Mode_t::READ);
		if (reader.IsReadable())
		{
			const string svDestPath = StringReplaceC(svPath, svWorkspace, "");
			uint16_t iPreloadSize  = 0;
			uint32_t nLoadFlags    = static_cast<uint32_t>(EPackedLoadFlags::LOAD_VISIBLE) | static_cast<uint32_t>(EPackedLoadFlags::LOAD_CACHE);
			uint16_t nTextureFlags = static_cast<uint16_t>(EPackedTextureFlags::TEXTURE_DEFAULT);
			bool bUseCompression   = true;
			bool bUseDataSharing   = true;

			if (!jManifest.is_null())
			{
				try
				{
					nlohmann::json jEntry = jManifest[svDestPath];
					if (!jEntry.is_null())
					{
						iPreloadSize    = jEntry.at("preloadSize").get<uint32_t>();
						nLoadFlags      = jEntry.at("loadFlags").get<uint32_t>();
						nTextureFlags   = jEntry.at("textureFlags").get<uint16_t>();
						bUseCompression = jEntry.at("useCompression").get<bool>();
						bUseDataSharing = jEntry.at("useDataSharing").get<bool>();
					}
				}
				catch (const std::exception& ex)
				{
					Warning(eDLL_T::FS, "Exception while reading VPK control file: '%s'\n", ex.what());
				}
			}

			DevMsg(eDLL_T::FS, "Packing entry '%zu' ('%s')\n", i, svDestPath.c_str());
			vEntryBlocks.push_back(VPKEntryBlock_t(reader.GetVector(), writer.GetPosition(), iPreloadSize, 0, nLoadFlags, nTextureFlags, svDestPath));

			VPKEntryBlock_t& vEntry = vEntryBlocks[i];
			for (size_t j = 0, es = vEntry.m_vFragments.size(); j < es; j++)
			{
				VPKChunkDescriptor_t& vDescriptor = vEntry.m_vFragments[j];

				reader.Read(s_EntryBuf, vDescriptor.m_nUncompressedSize);
				vDescriptor.m_nPackFileOffset = writer.GetPosition();

				if (bUseDataSharing)
				{
					string svEntryHash = sha1(string(reinterpret_cast<char*>(s_EntryBuf), vDescriptor.m_nUncompressedSize));
					auto p = m_mChunkHashMap.insert({ svEntryHash, vDescriptor });

					if (!p.second) // Map to existing chunk to avoid having copies of the same data.
					{
						DevMsg(eDLL_T::FS, "Mapping chunk '%zu' ('%s') to existing chunk at '0x%llx'\n", j, svEntryHash.c_str(), p.first->second.m_nPackFileOffset);

						vDescriptor = p.first->second;
						nSharedTotal += vDescriptor.m_nCompressedSize;
						nSharedCount++;

						continue;
					}
				}

				if (bUseCompression)
				{
					m_lzCompStatus = lzham_compress_memory(&m_lzCompParams, s_EntryBuf,
						&vDescriptor.m_nCompressedSize, s_EntryBuf,
						vDescriptor.m_nUncompressedSize, &m_nAdler32_Internal, &m_nCrc32_Internal);

					if (m_lzCompStatus != lzham_compress_status_t::LZHAM_COMP_STATUS_SUCCESS)
					{
						Warning(eDLL_T::FS, "Status '%d' for chunk '%zu' within entry '%zu' in block '%hu' (chunk packed without compression)\n",
							m_lzCompStatus, j, i, vEntryBlocks[i].m_iPackFileIndex);

						vDescriptor.m_nCompressedSize = vDescriptor.m_nUncompressedSize;
					}
				}
				else // Write data uncompressed.
				{
					vDescriptor.m_nCompressedSize = vDescriptor.m_nUncompressedSize;
				}

				vDescriptor.m_bIsCompressed = vDescriptor.m_nCompressedSize != vDescriptor.m_nUncompressedSize;
				writer.Write(s_EntryBuf, vDescriptor.m_nCompressedSize);
			}
		}
	}
	DevMsg(eDLL_T::FS, "*** Build block totaling '%zu' bytes with '%zu' shared bytes among '%lu' chunks\n", writer.GetPosition(), nSharedTotal, nSharedCount);

	m_mChunkHashMap.clear();
	memset(s_EntryBuf, '\0', sizeof(s_EntryBuf));

	VPKDir_t vDir = VPKDir_t();
	vDir.Build(svBuildPath + vPair.m_svDirectoryName, vEntryBlocks);
}

//-----------------------------------------------------------------------------
// Purpose: rebuilds manifest and extracts all files from specified VPK file
// Input  : &vDir - 
//          &svWorkspace - 
//-----------------------------------------------------------------------------
void CPackedStore::UnpackWorkspace(const VPKDir_t& vDir, const string& svWorkspace)
{
	if (vDir.m_vHeader.m_nHeaderMarker != VPK_HEADER_MARKER ||
		vDir.m_vHeader.m_nMajorVersion != VPK_MAJOR_VERSION ||
		vDir.m_vHeader.m_nMinorVersion != VPK_MINOR_VERSION)
	{
		Error(eDLL_T::FS, NO_ERROR, "Unsupported VPK directory file (invalid header criteria)\n");
		return;
	}
	BuildManifest(vDir.m_vEntryBlocks, svWorkspace, GetLevelName(vDir.m_svDirPath));

	for (size_t i = 0, fs = vDir.m_vPackFile.size(); i < fs; i++)
	{
		const fs::path fspVpkPath(vDir.m_svDirPath);
		const string svPath = fspVpkPath.parent_path().u8string() + '\\' + vDir.m_vPackFile[i];
		CIOStream iStream(svPath, CIOStream::Mode_t::READ); // Create stream to read from each pack file.

		for (size_t j = 0, es = vDir.m_vEntryBlocks.size(); j < es; j++)
		{
			const VPKEntryBlock_t& vBlock = vDir.m_vEntryBlocks[j];
			if (vBlock.m_iPackFileIndex != static_cast<uint16_t>(i))
			{
				continue;
			}
			else // Chunk belongs to this block.
			{
				const string svFilePath = CreateDirectories(svWorkspace + vBlock.m_svEntryPath);
				CIOStream oStream(svFilePath, CIOStream::Mode_t::WRITE);

				if (!oStream.IsWritable())
				{
					Error(eDLL_T::FS, NO_ERROR, "Unable to write file '%s'\n", svFilePath.c_str());
					continue;
				}

				DevMsg(eDLL_T::FS, "Unpacking entry '%zu' from block '%zu' ('%s')\n", j, i, vBlock.m_svEntryPath.c_str());
				for (size_t k = 0, cs = vBlock.m_vFragments.size(); k < cs; k++)
				{
					const VPKChunkDescriptor_t& vChunk = vBlock.m_vFragments[k];
					m_nChunkCount++;

					iStream.SetPosition(vChunk.m_nPackFileOffset);
					iStream.Read(s_EntryBuf, vChunk.m_nCompressedSize);

					if (vChunk.m_bIsCompressed)
					{
						size_t nDstLen = sizeof(s_DecompBuf);
						assert(vChunk.m_nCompressedSize <= nDstLen);

						if (vChunk.m_nCompressedSize > nDstLen)
							break; // Corrupt or invalid chunk descriptor.

						m_lzDecompStatus = lzham_decompress_memory(&m_lzDecompParams, s_DecompBuf,
							&nDstLen, s_EntryBuf, vChunk.m_nCompressedSize, &m_nAdler32_Internal, &m_nCrc32_Internal);

						if (m_lzDecompStatus != lzham_decompress_status_t::LZHAM_DECOMP_STATUS_SUCCESS)
						{
							Error(eDLL_T::FS, NO_ERROR, "Status '%d' for chunk '%zu' within entry '%zu' in block '%hu' (chunk not decompressed)\n",
								m_lzDecompStatus, m_nChunkCount, i, vBlock.m_iPackFileIndex);
						}
						else // If successfully decompressed, write to file.
						{
							oStream.Write(s_DecompBuf, nDstLen);
						}
					}
					else // If not compressed, write source data into output file.
					{
						oStream.Write(s_EntryBuf, vChunk.m_nUncompressedSize);
					}
				}

				if (m_nChunkCount == vBlock.m_vFragments.size()) // Only validate after last entry in block had been written.
				{
					m_nChunkCount = 0;
					m_nCrc32_Internal = vBlock.m_nFileCRC;

					oStream.Flush();
					ValidateCRC32PostDecomp(svFilePath);
				}
			}
		}
	}
	memset(s_EntryBuf, '\0', sizeof(s_EntryBuf));
	memset(s_DecompBuf, '\0', sizeof(s_DecompBuf));
}

//-----------------------------------------------------------------------------
// Purpose: 'VPKEntryBlock_t' file constructor
// Input  : *pReader - 
//          svEntryPath - 
//-----------------------------------------------------------------------------
VPKEntryBlock_t::VPKEntryBlock_t(CIOStream* pReader, string svEntryPath)
{
	StringReplace(svEntryPath, "\\", "/"); // Flip windows-style backslash to forward slash.
	StringReplace(svEntryPath, " /", "" ); // Remove space character representing VPK root.

	m_svEntryPath = svEntryPath; // Set the entry path.
	pReader->Read<uint32_t>(m_nFileCRC);       //
	pReader->Read<uint16_t>(m_iPreloadSize);   //
	pReader->Read<uint16_t>(m_iPackFileIndex); //

	do // Loop through all chunks in the entry and add to list.
	{
		VPKChunkDescriptor_t entry(pReader);
		m_vFragments.push_back(entry);
	} while (pReader->Read<uint16_t>() != PACKFILEINDEX_END);
}

//-----------------------------------------------------------------------------
// Purpose: 'VPKEntryBlock_t' memory constructor
// Input  : &vData - 
//          nOffset - 
//          iPreloadSize - 
//          iPackFileIndex - 
//          nLoadFlags - 
//          nTextureFlags - 
//          &svEntryPath - 
//-----------------------------------------------------------------------------
VPKEntryBlock_t::VPKEntryBlock_t(const vector<uint8_t> &vData, int64_t nOffset, uint16_t iPreloadSize, 
	uint16_t iPackFileIndex, uint32_t nLoadFlags, uint16_t nTextureFlags, const string& svEntryPath)
{
	m_nFileCRC = crc32::update(NULL, vData.data(), vData.size());
	m_iPreloadSize = iPreloadSize;
	m_iPackFileIndex = iPackFileIndex;
	m_svEntryPath = svEntryPath;

	size_t nFragmentCount = (vData.size() + ENTRY_MAX_LEN - 1) / ENTRY_MAX_LEN;
	size_t nFileSize = vData.size();
	int64_t nCurrentOffset = nOffset;

	for (size_t i = 0; i < nFragmentCount; i++) // Fragment data into 1 MiB chunks.
	{
		size_t nSize = std::min<uint64_t>(ENTRY_MAX_LEN, nFileSize);
		nFileSize -= nSize;
		m_vFragments.push_back(VPKChunkDescriptor_t(nLoadFlags, nTextureFlags, nCurrentOffset, nSize, nSize));
		nCurrentOffset += nSize;
	}
}

//-----------------------------------------------------------------------------
// Purpose: 'VPKChunkDescriptor_t' file constructor
// Input  : *pReader - 
//-----------------------------------------------------------------------------
VPKChunkDescriptor_t::VPKChunkDescriptor_t(CIOStream* pReader)
{
	pReader->Read<uint32_t>(m_nLoadFlags);        //
	pReader->Read<uint16_t>(m_nTextureFlags);     //
	pReader->Read<uint64_t>(m_nPackFileOffset);   //
	pReader->Read<uint64_t>(m_nCompressedSize);   //
	pReader->Read<uint64_t>(m_nUncompressedSize); //
	m_bIsCompressed = (m_nCompressedSize != m_nUncompressedSize);
}

//-----------------------------------------------------------------------------
// Purpose: 'VPKChunkDescriptor_t' memory constructor
// Input  : nLoadFlags - 
//          nTextureFlags - 
//          nArchiveOffset - 
//          nCompressedSize - 
//          nUncompressedSize - 
//-----------------------------------------------------------------------------
VPKChunkDescriptor_t::VPKChunkDescriptor_t(uint32_t nLoadFlags, uint16_t nTextureFlags, 
	uint64_t nArchiveOffset, uint64_t nCompressedSize, uint64_t nUncompressedSize)
{
	m_nLoadFlags = nLoadFlags;
	m_nTextureFlags = nTextureFlags;
	m_nPackFileOffset = nArchiveOffset;

	m_nCompressedSize = nCompressedSize;
	m_nUncompressedSize = nUncompressedSize;
}

//-----------------------------------------------------------------------------
// Purpose: 'VPKDir_t' file constructor
// Input  : &svPath - 
//-----------------------------------------------------------------------------
VPKDir_t::VPKDir_t(const string& svPath)
{
	CIOStream reader(svPath, CIOStream::Mode_t::READ);

	reader.Read<uint32_t>(m_vHeader.m_nHeaderMarker);
	reader.Read<uint16_t>(m_vHeader.m_nMajorVersion);  //
	reader.Read<uint16_t>(m_vHeader.m_nMinorVersion);  //
	reader.Read<uint32_t>(m_vHeader.m_nDirectorySize); //
	reader.Read<uint32_t>(m_nFileDataSize);            //

	m_vEntryBlocks = g_pPackedStore->GetEntryBlocks(&reader);
	m_svDirPath = svPath; // Set path to vpk directory file.

	for (VPKEntryBlock_t vEntry : m_vEntryBlocks)
	{
		if (vEntry.m_iPackFileIndex > m_nPackFileCount)
		{
			m_nPackFileCount = vEntry.m_iPackFileIndex;
		}
	}

	for (uint16_t i = 0; i < m_nPackFileCount + 1; i++)
	{
		string svArchivePath = g_pPackedStore->GetPackFile(svPath, i);
		m_vPackFile.push_back(svArchivePath);
	}
}

//-----------------------------------------------------------------------------
// Purpose: builds the vpk directory file
// Input  : &svDirectoryFile - 
//          &vEntryBlocks - 
//-----------------------------------------------------------------------------
void VPKDir_t::Build(const string& svDirectoryFile, const vector<VPKEntryBlock_t>& vEntryBlocks)
{
	CIOStream writer(svDirectoryFile, CIOStream::Mode_t::WRITE);
	auto vMap = std::map<string, std::map<string, std::list<VPKEntryBlock_t>>>();
	uint64_t nDescriptors = 0;

	writer.Write<uint32_t>(m_vHeader.m_nHeaderMarker);
	writer.Write<uint16_t>(m_vHeader.m_nMajorVersion);
	writer.Write<uint16_t>(m_vHeader.m_nMinorVersion);
	writer.Write<uint32_t>(m_vHeader.m_nDirectorySize);
	writer.Write<uint32_t>(m_vHeader.m_nSignatureSize);

	for (const VPKEntryBlock_t& vBlock : vEntryBlocks)
	{
		string svExtension = GetExtension(vBlock.m_svEntryPath);
		string svFilePath = RemoveFileName(vBlock.m_svEntryPath);

		if (svFilePath.empty())
		{
			svFilePath = ' '; // Has to be padded with a space character if empty [root].
		}
		if (!vMap.count(svExtension))
		{
			vMap.insert({ svExtension, std::map<string, std::list<VPKEntryBlock_t>>() });
		}
		if (!vMap[svExtension].count(svFilePath))
		{
			vMap[svExtension].insert({ svFilePath, std::list<VPKEntryBlock_t>() });
		}
		vMap[svExtension][svFilePath].push_back(vBlock);
	}

	for (auto& iKeyValue : vMap)
	{
		writer.WriteString(iKeyValue.first);
		for (auto& jKeyValue : iKeyValue.second)
		{
			writer.WriteString(jKeyValue.first);
			for (auto& vEntry : jKeyValue.second)
			{
				/*Write entry block*/
				writer.WriteString(GetFileName(vEntry.m_svEntryPath, true));
				writer.Write(vEntry.m_nFileCRC);
				writer.Write(vEntry.m_iPreloadSize);
				writer.Write(vEntry.m_iPackFileIndex);

				for (size_t i = 0, nc = vEntry.m_vFragments.size(); i < nc; i++)
				{
					/*Write chunk descriptor*/
					const VPKChunkDescriptor_t* pDescriptor = &vEntry.m_vFragments[i];

					writer.Write(pDescriptor->m_nLoadFlags);
					writer.Write(pDescriptor->m_nTextureFlags);
					writer.Write(pDescriptor->m_nPackFileOffset);
					writer.Write(pDescriptor->m_nCompressedSize);
					writer.Write(pDescriptor->m_nUncompressedSize);

					if (i != (nc - 1))
					{
						const ushort s = 0;
						writer.Write(s);
					}
					else // Mark end of entry.
					{
						const ushort s = PACKFILEINDEX_END;
						writer.Write(s);
					}
					nDescriptors++;
				}
			}
			writer.Write<uint8_t>('\0');
		}
		writer.Write<uint8_t>('\0');
	}
	writer.Write<uint8_t>('\0');
	m_vHeader.m_nDirectorySize = static_cast<uint32_t>(writer.GetPosition() - sizeof(VPKDirHeader_t));

	writer.SetPosition(offsetof(VPKDir_t, m_vHeader.m_nDirectorySize));
	writer.Write(m_vHeader.m_nDirectorySize);
	writer.Write(0);

	DevMsg(eDLL_T::FS, "*** Build directory totaling '%zu' bytes with '%zu' entries and '%zu' descriptors\n", 
		size_t(sizeof(VPKDirHeader_t) + m_vHeader.m_nDirectorySize), vEntryBlocks.size(), nDescriptors);
}

//-----------------------------------------------------------------------------
// Singleton
//-----------------------------------------------------------------------------
CPackedStore* g_pPackedStore = new CPackedStore();
