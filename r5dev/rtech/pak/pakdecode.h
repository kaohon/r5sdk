#ifndef RTECH_PAKDECODE_H
#define RTECH_PAKDECODE_H
#include "rtech/ipakfile.h"

extern size_t Pak_InitDefaultDecoder(PakDecoder_t* const decoder, const uint8_t* const fileBuffer,
	const uint64_t inputMask, const size_t dataSize, const size_t dataOffset, const size_t headerSize);

extern size_t Pak_InitDecoder(PakDecoder_t* const decoder, const uint8_t* const fileBuffer,
	const uint64_t inputMask, const size_t dataSize, const size_t dataOffset, const size_t headerSize, const bool useCustom);

extern bool Pak_StreamToBufferDecode(PakDecoder_t* const decoder, const size_t inLen, const size_t outLen, const bool useCustom);

#endif // RTECH_PAKDECODE_H
