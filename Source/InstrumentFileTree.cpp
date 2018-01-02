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

// The instrument file tree, used in the instrument toolbar to quickly load an instrument

#include "InstrumentFileTree.h"

CString CInstrumentFileTree::GetFile(int Index) const
{
	ASSERT(Index >= MENU_BASE + 2);
	return m_fileList[Index - MENU_BASE - 2];
}

void CInstrumentFileTree::Changed()
{
	m_bShouldRebuild = true;
}

bool CInstrumentFileTree::ShouldRebuild() const
{
	// Check if tree expired, to allow changes in the file system to be visible
	return (GetTickCount() > m_iTimeout) || m_bShouldRebuild;
}

bool CInstrumentFileTree::BuildMenuTree(const CString &instrumentPath)		// // //
{
	CWaitCursor wait;

	TRACE("Clearing instrument file tree...\n");		// // //

	m_fileList.clear();
	m_menuArray.clear();
	m_iTotalMenusAdded = 0;

	TRACE("Building instrument file tree...\n");

	m_RootMenu.CreatePopupMenu();
	m_RootMenu.AppendMenu(MF_STRING, MENU_BASE + 0, _T("Open file..."));
	m_RootMenu.AppendMenu(MF_STRING, MENU_BASE + 1, _T("Select directory..."));
	m_RootMenu.AppendMenu(MF_SEPARATOR);
	m_RootMenu.SetDefaultItem(0, TRUE);

	if (instrumentPath.GetLength() == 0) {
		m_bShouldRebuild = true;
		m_RootMenu.AppendMenu(MF_STRING | MF_DISABLED, MENU_BASE + 2, _T("(select a directory)"));
	}
	else {
		m_iFileIndex = 2;

		if (!ScanDirectory(instrumentPath, m_RootMenu, 0)) {		// // //
			// No files found
			m_RootMenu.AppendMenu(MF_STRING | MF_DISABLED, MENU_BASE + 2, _T("(no files found)"));
			m_bShouldRebuild = true;
		}
		else {
			m_fileList.shrink_to_fit();		// // //
			m_menuArray.shrink_to_fit();

			m_iTimeout = GetTickCount() + CACHE_TIMEOUT;
			m_bShouldRebuild = false;
		}
	}

	TRACE("Done\n");

	return true;
}

bool CInstrumentFileTree::ScanDirectory(const CString &path, CMenu &Menu, int level) {		// // //
	CFileFind fileFinder;
	bool bNoFile = true;

	if (level > RECURSION_LIMIT)
		return false;

	BOOL working = fileFinder.FindFile(path + _T("\\*.*"));

	// First scan directories
	while (working) {
		working = fileFinder.FindNextFile();

		if (fileFinder.IsDirectory() && !fileFinder.IsHidden() && !fileFinder.IsDots() && m_iTotalMenusAdded++ < MAX_MENUS) {
			auto &SubMenu = *m_menuArray.emplace_back(std::make_unique<CMenu>());		// // //
			SubMenu.CreatePopupMenu();
			// Recursive scan
			bool bEnabled = ScanDirectory(path + _T("\\") + fileFinder.GetFileName(), SubMenu, level + 1);
			Menu.AppendMenu(MF_STRING | MF_POPUP | (bEnabled ? MF_ENABLED : MF_DISABLED), (UINT)SubMenu.m_hMenu, fileFinder.GetFileName());
			bNoFile = false;
		}
	}

	working = fileFinder.FindFile(path + _T("\\*.fti"));

	// Then files
	while (working) {
		working = fileFinder.FindNextFile();
		Menu.AppendMenu(MF_STRING | MF_ENABLED, MENU_BASE + m_iFileIndex++, fileFinder.GetFileTitle());
		m_fileList.push_back(path + _T("\\") + fileFinder.GetFileName());		// // //
		bNoFile = false;
	}

	return !bNoFile;
}

CMenu &CInstrumentFileTree::GetMenu() {		// // //
	return m_RootMenu;
}
