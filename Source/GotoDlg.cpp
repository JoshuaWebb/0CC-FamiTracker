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

#include "GotoDlg.h"
#include "FamiTrackerModule.h"
#include "FamiTrackerView.h"
#include "FamiTrackerEnv.h"
#include "SoundChipService.h"
#include "SoundChipSet.h"
#include "SongData.h"
#include "SongView.h"
#include "APU/Types.h"
#include "str_conv/str_conv.hpp"

// CGotoDlg dialog

IMPLEMENT_DYNAMIC(CGotoDlg, CDialog)

CGotoDlg::CGotoDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CGotoDlg::IDD, pParent)
{
}

CGotoDlg::~CGotoDlg()
{
}

void CGotoDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CGotoDlg, CDialog)
	ON_EN_CHANGE(IDC_EDIT_GOTO_FRAME, OnEnChangeEditGotoFrame)
	ON_EN_CHANGE(IDC_EDIT_GOTO_ROW, OnEnChangeEditGotoRow)
	ON_EN_CHANGE(IDC_EDIT_GOTO_CHANNEL, OnEnChangeEditGotoChannel)
	ON_CBN_SELCHANGE(IDC_COMBO_GOTO_CHIP, OnCbnSelchangeComboGotoChip)
	ON_BN_CLICKED(IDOK, &CGotoDlg::OnBnClickedOk)
END_MESSAGE_MAP()


// CGotoDlg message handlers

BOOL CGotoDlg::OnInitDialog()
{
	m_cChipEdit.SubclassDlgItem(IDC_COMBO_GOTO_CHIP, this);

	const CFamiTrackerView *pView = CFamiTrackerView::GetView();
	const CFamiTrackerModule *pModule = pView->GetModuleData();

	CSoundChipSet chips = pModule->GetSoundChipSet();
	for (sound_chip_t c : SOUND_CHIPS)
		if (chips.ContainsChip(c))
			m_cChipEdit.AddString(conv::to_wide(Env.GetSoundChipService()->GetShortChipName(c)).data());

	chan_id_t Channel = pView->GetSelectedChannelID();
	if (auto c = GetChipFromChannel(Channel); c != sound_chip_t::NONE)
		m_cChipEdit.SelectString(-1, conv::to_wide(Env.GetSoundChipService()->GetShortChipName(c)).data());

	SetDlgItemInt(IDC_EDIT_GOTO_FRAME, pView->GetSelectedFrame());
	SetDlgItemInt(IDC_EDIT_GOTO_ROW, pView->GetSelectedRow());
	SetDlgItemInt(IDC_EDIT_GOTO_CHANNEL, GetChannelSubIndex(Channel) + 1);

	CEdit *pEdit = static_cast<CEdit*>(GetDlgItem(IDC_EDIT_GOTO_CHANNEL));
	pEdit->SetLimitText(1);
	pEdit = static_cast<CEdit*>(GetDlgItem(IDC_EDIT_GOTO_ROW));
	pEdit->SetLimitText(3);
	pEdit = static_cast<CEdit*>(GetDlgItem(IDC_EDIT_GOTO_FRAME));
	pEdit->SetLimitText(3);
	pEdit->SetFocus();

	return CDialog::OnInitDialog();
}

void CGotoDlg::CheckDestination() const
{
	const CFamiTrackerView *pView = CFamiTrackerView::GetView();
	const CConstSongView *pSongView = pView->GetSongView();

	bool Valid = true;
	if (m_iDestFrame >= pSongView->GetSong().GetFrameCount())
		Valid = false;
	else if (m_iDestRow >= static_cast<unsigned>(pSongView->GetFrameLength(m_iDestFrame)))
		Valid = false;
	else if (GetFinalChannel() == -1)
		Valid = false;

	GetDlgItem(IDOK)->EnableWindow(Valid);
}

int CGotoDlg::GetFinalChannel() const
{
	const CFamiTrackerModule *pModule = CFamiTrackerView::GetView()->GetModuleData();
	return pModule->GetChannelOrder().GetChannelIndex(MakeChannelIndex(m_iDestChip, m_iDestSubIndex));
}

void CGotoDlg::OnEnChangeEditGotoFrame()
{
	m_iDestFrame = GetDlgItemInt(IDC_EDIT_GOTO_FRAME);
	CheckDestination();
}

void CGotoDlg::OnEnChangeEditGotoRow()
{
	m_iDestRow = GetDlgItemInt(IDC_EDIT_GOTO_ROW);
	CheckDestination();
}

void CGotoDlg::OnEnChangeEditGotoChannel()
{
	m_iDestSubIndex = GetDlgItemInt(IDC_EDIT_GOTO_CHANNEL) - 1;
	CheckDestination();
}

void CGotoDlg::OnCbnSelchangeComboGotoChip()
{
	CStringW str;
	m_cChipEdit.GetWindowTextW(str);
	m_iDestChip = Env.GetSoundChipService()->GetChipFromString(conv::to_utf8(str));
	CheckDestination();
}

void CGotoDlg::OnBnClickedOk()
{
	CFamiTrackerView *pView = CFamiTrackerView::GetView();
	pView->SelectFrame(m_iDestFrame);
	pView->SelectRow(m_iDestRow);
	pView->SelectChannel(GetFinalChannel());

	CDialog::OnOK();
}
