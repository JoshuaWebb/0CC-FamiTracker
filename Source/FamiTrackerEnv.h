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

class CWinApp;
class CMainFrame;
class CAccelerator;
class CSoundGen;
class CMIDI;
class CSettings;
class CInstrumentService;
class CSoundChipService;

// global tracker environment

struct CFamiTrackerEnv {
	static CWinApp		*GetMainApp();
	static CMainFrame	*GetMainFrame();
	static CAccelerator	*GetAccelerator();
	static CSoundGen	*GetSoundGenerator();
	static CMIDI		*GetMIDI();
	static CSettings	*GetSettings();
	static CInstrumentService *GetInstrumentService();		// // //
	static CSoundChipService *GetSoundChipService();		// // //

	static bool IsFileLoaded();
};

extern CFamiTrackerEnv Env; // saves some typing
