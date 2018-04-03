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

#include "CreateWaveDlg.h"
#include "FamiTrackerEnv.h"		// // //
#include "Settings.h"		// // //
#include "FamiTrackerDoc.h"
#include "FamiTrackerView.h"
#include "FamiTrackerModule.h"		// // //
#include "FileDialogs.h"		// // //
#include "SongData.h"		// // //
#include "ChannelOrder.h"		// // //
#include "MainFrm.h"
#include "SoundGen.h"
#include "WavProgressDlg.h"
#include "WaveRenderer.h"		// // //
#include "WaveRendererFactory.h"		// // //
#include "ChannelName.h"		// // //
#include "str_conv/str_conv.hpp"		// // //
#include "IndividualWavProgressDlg.h"

const int MAX_LOOP_TIMES = 99;
const int MAX_PLAY_TIME	 = (99 * 60) + 0;

// CCreateWaveDlg dialog

IMPLEMENT_DYNAMIC(CCreateWaveDlg, CDialog)

CCreateWaveDlg::CCreateWaveDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CCreateWaveDlg::IDD, pParent)
{
}

CCreateWaveDlg::~CCreateWaveDlg()
{
}

void CCreateWaveDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CCreateWaveDlg, CDialog)
	ON_BN_CLICKED(IDC_BEGIN, &CCreateWaveDlg::OnBnClickedBegin)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_LOOP, &CCreateWaveDlg::OnDeltaposSpinLoop)
	ON_NOTIFY(UDN_DELTAPOS, IDC_SPIN_TIME, &CCreateWaveDlg::OnDeltaposSpinTime)
END_MESSAGE_MAP()

int CCreateWaveDlg::GetFrameLoopCount() const
{
	int Frames = GetDlgItemInt(IDC_TIMES);

	if (Frames < 1)
		Frames = 1;
	if (Frames > MAX_LOOP_TIMES)
		Frames = MAX_LOOP_TIMES;

	return Frames;
}

int CCreateWaveDlg::GetTimeLimit() const
{
	int Minutes, Seconds;
	CStringW str;

	GetDlgItemTextW(IDC_SECONDS, str);
	swscanf_s(str, L"%u:%u", &Minutes, &Seconds);
	int Time = (Minutes * 60) + (Seconds % 60);

	if (Time < 1)
		Time = 1;
	if (Time > MAX_PLAY_TIME)
		Time = MAX_PLAY_TIME;

	return Time;
}

// CCreateWaveDlg message handlers

void CCreateWaveDlg::OnBnClickedBegin()
{
	CFamiTrackerDoc *pDoc = CFamiTrackerDoc::GetDoc();
	CFamiTrackerView *pView = CFamiTrackerView::GetView();
	const CFamiTrackerModule *pModule = pView->GetModuleData();		// // //

	CStringW FileName = pDoc->GetFileTitle();
	int Track = m_ctlTracks.GetCurSel();

	if (pModule->GetSongCount() > 1) {
		auto sv = conv::to_wide(pModule->GetSong(Track)->GetTitle());
		AppendFormatW(FileName, L" - Track %02i (%.*s)", Track + 1, sv.size(), sv.data());		// // //
	}

	// Close this dialog
	EndDialog(0);

	// Ask for file location
	auto initPath = Env.GetSettings()->GetPath(PATH_WAV);
	auto path = GetSavePath(FileName, initPath.c_str(), IDS_FILTER_WAV, L"*.wav");		// // //
	if (!path)
		return;

	const auto rendererFactory = [&]() -> std::unique_ptr<CWaveRenderer>
	{
		std::unique_ptr<CWaveRenderer> newRenderer = nullptr;
		if (IsDlgButtonChecked(IDC_RADIO_LOOP))
			newRenderer = CWaveRendererFactory::Make(*pModule, Track, render_type_t::Loops, GetFrameLoopCount());
		else if (IsDlgButtonChecked(IDC_RADIO_TIME))
			newRenderer = CWaveRendererFactory::Make(*pModule, Track, render_type_t::Seconds, GetTimeLimit());

		if (newRenderer)
			newRenderer->SetRenderTrack(Track);

		return newRenderer;
	};

	const bool saveIndividual = IsDlgButtonChecked(IDC_SAVE_INDIVIDUAL);
	if (saveIndividual) {
		std::vector<channel_to_render> channelsToRender;

		for (int i = 0; i < m_ctlChannelList.GetCount(); ++i) {
			if (m_ctlChannelList.GetCheck(i) == BST_UNCHECKED)
				continue;

			const auto channelId = pModule->GetChannelOrder().TranslateChannel(i);
			auto channelName = conv::to_wide(GetChannelFullName(channelId));
			for (auto& ch : channelName)
				if (ch == ' ')
					ch = '_';

			auto lastDot = path->ReverseFind('.');
			CString channelPath;
			channelPath.Format(L"%s.%s.wav", path->Left(lastDot).GetString(), channelName.data());

			channelsToRender.emplace_back(channelId, channelPath);
		}

		// Show the render progress dialog, this will also start rendering
		CIndividualWavProgressDlg ProgressDlg;
		ProgressDlg.BeginRender(rendererFactory, channelsToRender);		// // //
	} else {
		// Mute selected channels
		pView->UnmuteAllChannels();
		for (int i = 0; i < m_ctlChannelList.GetCount(); ++i)
			if (m_ctlChannelList.GetCheck(i) == BST_UNCHECKED)
				pView->ToggleChannel(pModule->GetChannelOrder().TranslateChannel(i));

		auto pRenderer = rendererFactory();
		CWavProgressDlg ProgressDlg;
		ProgressDlg.BeginRender(*path, std::move(pRenderer));           // // //
	}

	// Unmute all channels
	pView->UnmuteAllChannels();
}


BOOL CCreateWaveDlg::OnInitDialog()
{
	CheckDlgButton(IDC_RADIO_LOOP, BST_CHECKED);
	CheckDlgButton(IDC_RADIO_TIME, BST_UNCHECKED);

	SetDlgItemTextW(IDC_TIMES, L"1");
	SetDlgItemTextW(IDC_SECONDS, L"01:00");

	m_ctlChannelList.SubclassDlgItem(IDC_CHANNELS, this);

	m_ctlChannelList.ResetContent();
	m_ctlChannelList.SetCheckStyle(BS_AUTOCHECKBOX);

	m_ctlTracks.SubclassDlgItem(IDC_TRACKS, this);

	const CFamiTrackerModule *pModule = CFamiTrackerView::GetView()->GetModuleData();		// // //
	const CChannelOrder &order = pModule->GetChannelOrder(); // CFamiTrackerView::GetView()->GetSongView()->

	order.ForeachChannel([&] (chan_id_t i) {
		m_ctlChannelList.AddString(conv::to_wide(GetChannelFullName(i)).data());		// // //
		m_ctlChannelList.SetCheck(order.GetChannelIndex(i), 1);
	});

	pModule->VisitSongs([&] (const CSongData &song, unsigned i) {
		auto sv = conv::to_wide(song.GetTitle());
		m_ctlTracks.AddString(FormattedW(L"#%02i - %.*s", i + 1, sv.size(), sv.data()));		// // //
	});

	CMainFrame *pMainFrm = static_cast<CMainFrame*>(AfxGetMainWnd());		// // //
	m_ctlTracks.SetCurSel(pMainFrm->GetSelectedTrack());

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CCreateWaveDlg::ShowDialog()
{
	CDialog::DoModal();
}

void CCreateWaveDlg::OnDeltaposSpinLoop(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	int Times = GetFrameLoopCount() - pNMUpDown->iDelta;

	if (Times < 1)
		Times = 1;
	if (Times > MAX_LOOP_TIMES)
		Times = MAX_LOOP_TIMES;

	SetDlgItemInt(IDC_TIMES, Times);
	CheckDlgButton(IDC_RADIO_LOOP, BST_CHECKED);
	CheckDlgButton(IDC_RADIO_TIME, BST_UNCHECKED);
	*pResult = 0;
}

void CCreateWaveDlg::OnDeltaposSpinTime(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMUPDOWN pNMUpDown = reinterpret_cast<LPNMUPDOWN>(pNMHDR);
	int Minutes, Seconds;
	int Time = GetTimeLimit() - pNMUpDown->iDelta;

	if (Time < 1)
		Time = 1;
	if (Time > MAX_PLAY_TIME)
		Time = MAX_PLAY_TIME;

	Seconds = Time % 60;
	Minutes = Time / 60;

	SetDlgItemTextW(IDC_SECONDS, FormattedW(L"%02i:%02i", Minutes, Seconds));
	CheckDlgButton(IDC_RADIO_LOOP, BST_UNCHECKED);
	CheckDlgButton(IDC_RADIO_TIME, BST_CHECKED);
	*pResult = 0;
}
