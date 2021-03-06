/*
** FamiTracker - NES/Famicom sound tracker
** Copyright (C) 2005-2015  Jonathan Liss
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
#include "APU/Channel.h"

// // // 050B

class CS5BChannel : public CChannel
{
public:
	friend class CS5B;

	CS5BChannel(CMixer &Mixer, chan_id_t ID);		// // //

	void Process(uint32_t Time);
	void Reset();

	uint32_t GetTime() const;
	void Output(uint32_t Noise, uint32_t Envelope);

	double GetFrequency() const;

private:
	uint8_t m_iVolume;
	uint32_t m_iPeriod;
	uint32_t m_iPeriodClock;

	bool m_bSquareHigh;
	bool m_bSquareDisable;
	bool m_bNoiseDisable;
};

class CS5B : public CSoundChip
{
public:
	explicit CS5B(CMixer &Mixer);

	sound_chip_t GetID() const override;		// // //

	void	Reset() override;
	void	Process(uint32_t Time) override;
	void	EndFrame() override;

	void	Write(uint16_t Address, uint8_t Value) override;
	uint8_t	Read(uint16_t Address, bool &Mapped) override;

	void	Log(uint16_t Address, uint8_t Value) override;		// // //

	double	GetFreq(int Channel) const override;		// // //

private:
	void	WriteReg(uint8_t Port, uint8_t Value);
	void	RunEnvelope(uint32_t Time);
	void	RunNoise(uint32_t Time);

private:
	CS5BChannel m_Channel[3];

	uint8_t m_cPort;

	int m_iCounter;

	uint32_t m_iNoisePeriod;
	uint32_t m_iNoiseClock;
	uint32_t m_iNoiseState;

	uint32_t m_iEnvelopePeriod;
	uint32_t m_iEnvelopeClock;
	char m_iEnvelopeLevel;
	char m_iEnvelopeShape;
	bool m_bEnvelopeHold;
};
