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

#include "stdafx.h"		// // //
#include <mmsystem.h>

const int MIDI_MSG_NOTE_OFF			= 0x08;
const int MIDI_MSG_NOTE_ON			= 0x09;
const int MIDI_MSG_AFTER_TOUCH		= 0x0A;
const int MIDI_MSG_CONTROL_CHANGE	= 0x0B;
const int MIDI_MSG_PROGRAM_CHANGE	= 0x0C;
const int MIDI_MSG_CHANNEL_PRESSURE = 0x0D;
const int MIDI_MSG_PITCH_WHEEL		= 0x0E;

enum class note_t : unsigned char;		// // //

// CMIDI command target

class CMIDI {
public:
	CMIDI();
	CMIDI(const CMIDI &) = delete;		// // //
	CMIDI(CMIDI &&) = delete;
	~CMIDI();

	bool	Init();
	void	Shutdown();

	bool	OpenDevices();
	bool	CloseDevices();

	bool	ReadMessage(unsigned char &Message, unsigned char &Channel, unsigned char &Data1, unsigned char &Data2);
	void	WriteNote(unsigned char Channel, note_t Note, unsigned char Octave, unsigned char Velocity);		// // //
	void	ResetOutput();
	void	ToggleInput();

	int		GetQuantization() const;

	bool	IsOpened() const;
	bool	IsAvailable() const;

	void	SetInputDevice(int Device, bool MasterSync);
	void	SetOutputDevice(int Device);

	int		GetInputDevice() const;
	int		GetOutputDevice() const;

	// Device enumeration
	int		GetNumInputDevices() const;
	int		GetNumOutputDevices() const;

	void	GetInputDeviceString(int Num, CStringW &Text) const;
	void	GetOutputDeviceString(int Num, CStringW &Text) const;

	// Private methods
private:
	void	Event(unsigned char Status, unsigned char Data1, unsigned char Data2);
	void	Enqueue(unsigned char MsgType, unsigned char MsgChannel, unsigned char Data1, unsigned char Data2);

	// Constants
private:
	static const int MAX_QUEUE = 100;

	// Static functions & variables
private:
	static void CALLBACK MidiInProc(HMIDIIN hMidiIn, UINT wMsg, DWORD dwInstance, DWORD dwParam1, DWORD dwParam2);
	static CMIDI *m_pInstance;

	// Private variables
private:
	// Devices
	int		m_iInDevice;
	int		m_iOutDevice;

	bool	m_bMasterSync;
	bool	m_bInStarted;

	// MIDI queue
	int		m_iQueueHead;
	int		m_iQueueTail;

	char	m_iMsgTypeQueue[MAX_QUEUE];
	char	m_iMsgChanQueue[MAX_QUEUE];
	char	m_iData1Queue[MAX_QUEUE];
	char	m_iData2Queue[MAX_QUEUE];
	char	m_iQuantization[MAX_QUEUE];

	int		m_iQuant;
	int		m_iTimingCounter;

	// Device handles
	HMIDIIN	 m_hMIDIIn;
	HMIDIOUT m_hMIDIOut;

	// Thread sync
	CCriticalSection m_csQueue;
};
