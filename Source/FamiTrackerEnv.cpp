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

#include "FamiTrackerEnv.h"
#include "stdafx.h"
#include "FamiTracker.h"
#include "InstrumentService.h"		// // //
#include "SoundChipService.h"		// // //
#include "FamiTrackerDoc.h"

CFamiTrackerEnv Env;

CWinApp *CFamiTrackerEnv::GetMainApp() {
	return &theApp;
}

CMainFrame *CFamiTrackerEnv::GetMainFrame() {
	return theApp.GetMainFrame();
}

CAccelerator *CFamiTrackerEnv::GetAccelerator() {
	return theApp.GetAccelerator();
}

CSoundGen *CFamiTrackerEnv::GetSoundGenerator() {
	return theApp.GetSoundGenerator();
}

CMIDI *CFamiTrackerEnv::GetMIDI() {
	return theApp.GetMIDI();
}

CSettings *CFamiTrackerEnv::GetSettings() {
	return theApp.GetSettings();
}

CInstrumentService *CFamiTrackerEnv::GetInstrumentService() {
	static auto factory = [] {
		CInstrumentService f;
		f.AddDefaultTypes();
		return f;
	}();
	return &factory;
}

CSoundChipService *CFamiTrackerEnv::GetSoundChipService() {
	static auto factory = [] {
		CSoundChipService f;
		f.AddDefaultTypes();
		return f;
	}();
	return &factory;
}

bool CFamiTrackerEnv::IsFileLoaded() {
	return CFamiTrackerDoc::GetDoc()->IsFileLoaded();
}
