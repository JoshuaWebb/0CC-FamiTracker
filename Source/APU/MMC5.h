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

#include "APU/SoundChip.h"
#include "APU/Square.h"		// // //

class CMMC5 : public CSoundChip {
public:
	explicit CMMC5(CMixer &Mixer);

	sound_chip_t GetID() const override;		// // //

	void Reset() override;
	void Process(uint32_t Time) override;
	void EndFrame() override;

	void Write(uint16_t Address, uint8_t Value) override;
	uint8_t Read(uint16_t Address, bool &Mapped) override;

	double GetFreq(int Channel) const override;		// // //

	void LengthCounterUpdate();
	void EnvelopeUpdate();
	void ClockSequence();		// // //

private:
	CSquare	m_Square1;		// // //
	CSquare	m_Square2;		// // //
	uint8_t	m_iEXRAM[0x400] = { };		// // //
	uint8_t	m_iMulLow;
	uint8_t	m_iMulHigh;
};
