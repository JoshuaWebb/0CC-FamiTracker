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

#include "APU/N163.h"
#include "APU/Mixer.h"		// // //
#include "RegisterState.h"		// // //
#include <algorithm>		// // //

/*

 Namco 163 emulation

 I found out that all channels shares the same DAC, and the chip
 switches channel at the rate of 120 kHz. This is why there is
 high pitched noise (15 kHz) when using 8 channels.

*/

//
// Namco 163 (previously called N106)
//

CN163::CN163(CMixer &Mixer) :
	CSoundChip(Mixer),		// // //
	m_Channels {
		{Mixer, *this, chan_id_t::N163_CH1, m_iWaveData},
		{Mixer, *this, chan_id_t::N163_CH2, m_iWaveData},
		{Mixer, *this, chan_id_t::N163_CH3, m_iWaveData},
		{Mixer, *this, chan_id_t::N163_CH4, m_iWaveData},
		{Mixer, *this, chan_id_t::N163_CH5, m_iWaveData},
		{Mixer, *this, chan_id_t::N163_CH6, m_iWaveData},
		{Mixer, *this, chan_id_t::N163_CH7, m_iWaveData},
		{Mixer, *this, chan_id_t::N163_CH8, m_iWaveData},
	}
{
	m_pRegisterLogger->AddRegisterRange(0x00, 0x7F);		// // //
}

sound_chip_t CN163::GetID() const {		// // //
	return sound_chip_t::N163;
}

void CN163::Reset()
{
	for (auto &ch : m_Channels)		// // //
		ch.Reset();

	m_iLastValue = 0;

	m_iGlobalTime = 0;

	m_iChannelCntr = 0;
	m_iLastChan = m_iActiveChan = 7;		// // //
	m_iCycle = 0;
}

void CN163::SetMixingMethod(bool bLinear)		// // //
{
	m_bOldMixing = bLinear;
	for (auto &ch : m_Channels)
		ch.Reset();
}

void CN163::Process(uint32_t Time)
{
	if (m_bOldMixing) {		// // //
		ProcessOld(Time);
		return;
	}

	const uint32_t CHAN_PERIOD = 15;		// 15 cycles/channel

	m_pMixer->SetNamcoVolume((m_iChansInUse == 0) ? 1.3f : (1.5f + float(m_iChansInUse - 1) / 1.5f));

	while (Time > 0) {
		uint32_t TimeToRun = std::min(Time, CHAN_PERIOD - m_iChannelCntr);		// // //

		Mix(0, 0, m_Channels[m_iLastChan].GetChannelType());		// // //
		m_Channels[m_iActiveChan].Process(TimeToRun, m_iChansInUse + 1);		// // //
		m_iLastChan = m_iActiveChan;

		Time -= TimeToRun;
		m_iGlobalTime += TimeToRun;
		m_iChannelCntr += TimeToRun;

		if (m_iChannelCntr >= CHAN_PERIOD) {
			if (m_iActiveChan + m_iChansInUse < MAX_CHANNELS_N163)
				m_iActiveChan = MAX_CHANNELS_N163;
			--m_iActiveChan;
			m_iChannelCntr -= CHAN_PERIOD;
		}
	}
}

void CN163::ProcessOld(uint32_t Time)		// // //
{
	m_pMixer->SetNamcoVolume((m_iChansInUse == 0) ? 1.0f : 0.75f);

	for (int i = 7 - m_iChansInUse; i < MAX_CHANNELS_N163; ++i)
		m_Channels[i].ProcessClean(Time, m_iChansInUse + 1);
}

void CN163::Mix(int32_t Value, uint32_t Time, chan_id_t ChanID)		// // //
{
	// N163 amplitude:
	// One channel: 1.1V P-P
	// Two-eight channels: 800mV P-P
	// 2A03 triangle: 330mV P-P

	if (Value != m_iLastValue) {
		m_pMixer->AddValue(ChanID, Value - m_iLastValue, Time + m_iGlobalTime);
		m_iLastValue = Value;
	}
}

void CN163::EndFrame()
{
	CRegisterLoggerBlock b {*m_pRegisterLogger};
	for (int i = 0; i < MAX_CHANNELS_N163; ++i) {
		m_Channels[i].EndFrame();
		if (i <= m_iChansInUse) for (int j : {1, 3, 5}) {		// // // log phase
			int Address = 0x78 - i * 8 + j;
			m_pRegisterLogger->SetPort(Address);
			m_pRegisterLogger->Write(ReadMem(Address));
		}
		if (i != m_iActiveChan)		// // //
			m_Channels[i].ResetCounter();
	}

	m_iGlobalTime = 0;
}

void CN163::Write(uint16_t Address, uint8_t Value)
{
	int Area = m_iExpandAddr & 0x7F;

	switch (Address) {
		case 0x4800:
			m_iWaveData[Area] = Value;

			if (Area >= 0x40) {
				int Channel = (Area & 0x3F) >> 3;
				m_Channels[Channel].Write(Area & 0x07, Value);

				if (Area == 0x7F)
					m_iChansInUse = (Value >> 4) & 0x07;
			}

			if (m_iExpandAddr & 0x80)
				m_iExpandAddr = ((m_iExpandAddr + 1) & 0x7F) | 0x80;		// // //
			break;

		case 0xF800:
			m_iExpandAddr = Value;
			break;
	}
}

void CN163::Log(uint16_t Address, uint8_t Value)		// // //
{
	switch (Address) {
	case 0xF800:
		m_pRegisterLogger->SetPort(Value & 0x7F);
		m_pRegisterLogger->SetAutoincrement((Value & 0x80) != 0);
		break;
	case 0x4800:
		m_pRegisterLogger->Write(Value);
		break;
	}
}

double CN163::GetFreq(int Channel) const		// // //
{
	if (Channel < 0 || Channel > m_iChansInUse)
		return 0.;
	return m_Channels[7 - Channel].GetFrequency() / (m_iChansInUse + 1);
}

uint8_t CN163::Read(uint16_t Address, bool &Mapped)
{
	uint16_t ReadAddr;

	switch (Address) {
		case 0x4800:
			Mapped = true;
			ReadAddr = m_iExpandAddr & 0x7F;
			if (m_iExpandAddr & 0x80)
				m_iExpandAddr = ((m_iExpandAddr + 1) & 0x7F) | 0x80;
			return m_iWaveData[ReadAddr];
	}

	return 0;
}

uint8_t CN163::ReadMem(uint8_t Reg)
{
	int ChanArea = 0x80 - ((m_iChansInUse + 1) << 3);
	int Chan = (Reg & 0x3F) >> 3;

	if (Reg < ChanArea)
		return m_iWaveData[Reg & 0x7F];

	if ((Reg & 7) == 7)
		return m_Channels[Chan].ReadMem(Reg) | (m_iWaveData[Reg] & 0xF0);

	return m_Channels[Chan].ReadMem(Reg);
}

//
// N163 channels
//

CN163Chan::CN163Chan(CMixer &Mixer, CN163 &parent, chan_id_t ID, uint8_t *pWaveData) :		// // //
	CChannel(Mixer, sound_chip_t::N163, ID),
	m_pWaveData(pWaveData), parent_(parent)
{
	Reset();
}

void CN163Chan::Reset()
{
	m_iCounter	  = 0;
	m_iWaveOffset = 0;
	m_iWaveLength = 0x1000000;		// // //
	m_iVolume	  = 0;
	m_iLastSample = 0;
	m_iPhase	  = 0;
	m_iFrequency  = 0;

	Mix(0);		// // //

	EndFrame();
}

void CN163Chan::Write(uint16_t Address, uint8_t Value)
{
	switch (Address) {
		case 0x00:
			m_iFrequency = (m_iFrequency & 0xFFFF00) | Value;
			break;
		case 0x01:
			m_iPhase = (m_iPhase & 0xFFFF00) | Value;
			break;
		case 0x02:
			m_iFrequency = (m_iFrequency & 0xFF00FF) | (Value << 8);
			break;
		case 0x03:
			m_iPhase = (m_iPhase & 0xFF00FF) | (Value << 8);
			break;
		case 0x04:
			m_iFrequency = (m_iFrequency & 0x00FFFF) | ((Value & 3) << 16);
			m_iWaveLength = (256 - (Value & 0xFC)) << 16;
			break;
		case 0x05:
			m_iPhase = (m_iPhase & 0x00FFFF) | (Value << 16);
			break;
		case 0x06:
			m_iWaveOffset = Value;
			break;
		case 0x07:
			m_iVolume = Value & 0x0F;
			break;
	}
}

void CN163Chan::Process(uint32_t Time, uint8_t ChannelsActive)		// // //
{
	uint32_t TimeStamp = 0;

	parent_.Mix(m_iLastSample, TimeStamp, m_iChanId);

	if (!m_iFrequency || !m_iWaveLength) {
		m_iLastSample = 0;
		m_iTime += Time;
		return;
	}

	while (Time >= m_iCounter) {
		Time	-= m_iCounter;
		m_iTime += m_iCounter;
		TimeStamp += m_iCounter;
		m_iCounter = 15;

		m_iPhase = (m_iPhase + m_iFrequency) % m_iWaveLength;

		int WavePtr = m_iPhase >> 16;

		uint8_t Sample = m_pWaveData[((WavePtr + m_iWaveOffset) & 0xFF) >> 1];

		if (WavePtr & 1)
			Sample >>= 4;

		m_iLastSample = (Sample & 0x0F) * m_iVolume;

		parent_.Mix(m_iLastSample, TimeStamp, m_iChanId);
	}

	m_iCounter -= Time;
	m_iTime += Time;
}

void CN163Chan::ProcessClean(uint32_t Time, uint8_t ChannelsActive)
{
	// legacy

	if (!m_iFrequency || !m_iWaveLength) {
		m_iTime += Time;
		return;
	}

	while (Time >= m_iCounter) {
		Time	-= m_iCounter;
		m_iTime += m_iCounter;
		m_iCounter = 15 * ChannelsActive;

		m_iPhase = (m_iPhase + m_iFrequency) % m_iWaveLength;

		int WavePtr = m_iPhase >> 16;

		uint8_t Sample = m_pWaveData[((WavePtr + m_iWaveOffset) & 0xFF) >> 1];

		if (WavePtr & 1)
			Sample >>= 4;

		Mix((Sample & 0x0F) * m_iVolume);
	}

	m_iCounter -= Time;
	m_iTime += Time;
}

uint8_t CN163Chan::ReadMem(uint8_t Reg)
{
	switch (Reg & 7) {
		case 0:
			return m_iFrequency & 0xFF;
		case 1:
			return m_iPhase & 0xFF;
		case 2:
			return (m_iFrequency >> 8) & 0xFF;
		case 3:
			return (m_iPhase >> 8) & 0xFF;
		case 4:
			return (uint8_t)(((m_iFrequency >> 16) & 0x03) | (256 - (m_iWaveLength >> 16)));
		case 5:
			return (m_iPhase >> 16) & 0xFF;
		case 6:
			return m_iWaveOffset;
		case 7:
			return m_iVolume;
	}

	return 0;
}

void CN163Chan::ResetCounter()
{
	m_iCounter = 0;
}

double CN163Chan::GetFrequency() const		// // //
{
	return MASTER_CLOCK_NTSC / 983040. * m_iFrequency / (m_iWaveLength >> 16);
}
