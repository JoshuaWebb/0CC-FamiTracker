/*
** FamiTracker - NES/Famicom sound tracker
** Copyright (C) 2005-2014  Jonathan Liss
**
** 0CC-FamiTracker is (C) 2014-2018 HertzDevil
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
** Library General Public License for more details.  To obtain a
** copy of the GNU Library General Public License, write to the Free
** Software Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
**
** Any permitted reproduction of these routines, in whole or in part,
** must bear this legend.
*/


#pragma once

#include <vector>
#include <memory>		// // //
#include "array_view.h"		// // //

//
// Binary chunk renderers
//

namespace ft0cc::doc {
class dpcm_sample;
} // namespace ft0cc::doc
class CChunk;		// // //
class CFile;		// // //

// Base class
class CBinaryFileWriter
{
protected:
	CBinaryFileWriter(CFile *pFile);
	void Store(array_view<std::uint8_t> Data);
	void Fill(unsigned int Size);
	unsigned int GetWritten() const;

private:
	CFile		*m_pFile;
	unsigned int m_iDataWritten;
};

// Binary data render
class CChunkRenderBinary : public CBinaryFileWriter
{
public:
	CChunkRenderBinary(CFile *pFile);
	void StoreChunks(const std::vector<std::shared_ptr<CChunk>> &Chunks);		// // //
	void StoreSamples(const std::vector<std::shared_ptr<const ft0cc::doc::dpcm_sample>> &Samples);		// // //

private:
	void StoreChunk(const CChunk &Chunk);		// // //
	void StoreSample(const ft0cc::doc::dpcm_sample &DSample);		// // //

private:
	int m_iSampleAddress;
};

// NSF render
class CChunkRenderNSF : public CBinaryFileWriter
{
public:
	CChunkRenderNSF(CFile *pFile, unsigned int StartAddr);

	void StoreDriver(array_view<std::uint8_t> Driver);		// // //
	void StoreChunks(const std::vector<std::shared_ptr<CChunk>> &Chunks);		// // //
	void StoreChunksBankswitched(const std::vector<std::shared_ptr<CChunk>> &Chunks);
	void StoreSamples(const std::vector<std::shared_ptr<const ft0cc::doc::dpcm_sample>> &Samples);
	void StoreSamplesBankswitched(const std::vector<std::shared_ptr<const ft0cc::doc::dpcm_sample>> &Samples);
	int  GetBankCount() const;

protected:
	void StoreChunk(const CChunk &Chunk);		// // //
	void StoreChunkBankswitched(const CChunk &Chunk);
	void StoreSample(const ft0cc::doc::dpcm_sample &DSample);
	void StoreSampleBankswitched(const ft0cc::doc::dpcm_sample &DSample);

	int  GetRemainingSize() const;
	void AllocateNewBank();
	int  GetBank() const;
	int	 GetAbsoluteAddr() const;

protected:
	unsigned int m_iStartAddr;
	unsigned int m_iSampleAddr;
};

// NES render
class CChunkRenderNES : public CChunkRenderNSF
{
public:
	CChunkRenderNES(CFile *pFile, unsigned int StartAddr);
	void StoreCaller(array_view<std::uint8_t> Data);		// // //
};
