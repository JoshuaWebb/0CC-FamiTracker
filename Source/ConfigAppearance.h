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

enum color_items_t {
	COL_BACKGROUND,
	COL_BACKGROUND_HILITE,
	COL_BACKGROUND_HILITE2,
	COL_PATTERN_TEXT,
	COL_PATTERN_TEXT_HILITE,
	COL_PATTERN_TEXT_HILITE2,
	COL_PATTERN_INSTRUMENT,
	COL_PATTERN_VOLUME,
	COL_PATTERN_EFF_NUM,
	COL_SELECTION,
	COL_CURSOR,
	COL_CURRENT_ROW_NORMAL,
	COL_CURRENT_ROW_EDIT,
	COL_CURRENT_ROW_PLAYING,
	COLOR_ITEM_COUNT,
};

#include "ColorScheme.h"
#include "../resource.h"		// // //
#include <string>		// // //
#include <string_view>		// // //

// CConfigAppearance dialog

class CConfigAppearance : public CPropertyPage
{
	DECLARE_DYNAMIC(CConfigAppearance)

public:
	CConfigAppearance();
	virtual ~CConfigAppearance();

	void AddFontName(std::wstring_view Name);		// / //

// Dialog Data
	enum { IDD = IDD_CONFIG_APPEARANCE };

protected:
	static int CALLBACK EnumFontFamExProc(ENUMLOGFONTEXW *lpelfe, NEWTEXTMETRICEXW *lpntme, DWORD FontType, LPARAM lParam);

	static const std::string_view COLOR_ITEMS[];		// // //

	static const COLOR_SCHEME *const COLOR_SCHEMES[];
	static const int FONT_SIZES[];

	static const char SETTING_SEPARATOR[];		// // // 050B
	static const char HEX_PREFIX[];		// // // 050B

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	void SelectColorScheme(const COLOR_SCHEME *pColorScheme);

	void SetColor(int Index, int Color);
	int GetColor(int Index) const;

	void ExportSettings(LPCWSTR Path) const;		// // // 050B
	void ImportSettings(LPCWSTR Path);		// // // 050B

protected:
	std::wstring	m_strFont;		// // //

	int			m_iFontSize;
	int			m_iSelectedItem;
	bool		m_bPatternColors;
	bool		m_bDisplayFlats;

	int			m_iColors[COLOR_ITEM_COUNT];

	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnPaint();
	virtual BOOL OnInitDialog();
	virtual BOOL OnApply();
	afx_msg void OnCbnSelchangeFont();
	virtual BOOL OnSetActive();
	afx_msg void OnBnClickedPickCol();
	afx_msg void OnCbnSelchangeColItem();
	afx_msg void OnCbnSelchangeScheme();
	afx_msg void OnCbnSelchangeFontSize();
	afx_msg void OnBnClickedPatterncolors();
	afx_msg void OnBnClickedDisplayFlats();
	afx_msg void OnCbnEditchangeFontSize();
	afx_msg void OnBnClickedButtonAppearanceSave();		// // // 050B
	afx_msg void OnBnClickedButtonAppearanceLoad();		// // // 050B
};
