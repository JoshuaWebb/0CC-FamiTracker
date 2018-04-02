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

#include "IndividualWavProgressDlg.h"
#include "FamiTrackerEnv.h"		// // //
#include "FamiTrackerTypes.h"
#include "APU\Types.h"
#include "SoundGen.h"
#include "WaveRenderer.h"		// // //
#include "str_conv/str_conv.hpp"		// // //
#include <functional>
#include "FamiTrackerView.h"

// CIndividualWavProgressDlg dialog

IMPLEMENT_DYNAMIC(CIndividualWavProgressDlg, CDialog)

CIndividualWavProgressDlg::CIndividualWavProgressDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CIndividualWavProgressDlg::IDD, pParent), m_dwStartTime(0)		// // //
{
}

CIndividualWavProgressDlg::~CIndividualWavProgressDlg()
{
}

void CIndividualWavProgressDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
}


BEGIN_MESSAGE_MAP(CIndividualWavProgressDlg, CDialog)
	ON_BN_CLICKED(IDC_CANCEL, &CIndividualWavProgressDlg::OnBnClickedCancel)
	ON_WM_TIMER()
END_MESSAGE_MAP()

// CIndividualWavProgressDlg message handlers

void CIndividualWavProgressDlg::OnBnClickedCancel()
{
	CSoundGen *pSoundGen = Env.GetSoundGenerator();

	if (pSoundGen->IsRendering()) {
		//pSoundGen->StopRendering();
		pSoundGen->PostThreadMessageW(WM_USER_STOP_RENDER, 0, 0);
	}

	EndDialog(0);
}

void CIndividualWavProgressDlg::BeginRender(std::function<std::unique_ptr<CWaveRenderer>()> waveRendererFactory, std::vector<channel_to_render> channelsToRender)		// // //
{
	m_waveRendererFactory = waveRendererFactory;		// // //
	m_channelsToRender = channelsToRender;

	if (!m_channelsToRender.empty())
		DoModal();
}

BOOL CIndividualWavProgressDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	static_cast<CProgressCtrl*>(GetDlgItem(IDC_PROGRESS_BAR))->SetRange(0, 100);
	static_cast<CProgressCtrl*>(GetDlgItem(IDC_PROGRESS_BAR2))->SetRange(0, short(100 * m_channelsToRender.size()));

	m_currentChannelIdx = 0;
	RenderChannel();

	m_dwStartTime = GetTickCount();
	SetTimer(0, 200, NULL);

	return TRUE;  // return TRUE unless you set the focus to a control
	// EXCEPTION: OCX Property Pages should return FALSE
}

void CIndividualWavProgressDlg::RenderChannel()
{
	CFamiTrackerView *famiTrackerView = CFamiTrackerView::GetView();
	CSoundGen *pSoundGen = Env.GetSoundGenerator();

	const auto currentChannel = m_channelsToRender[m_currentChannelIdx];
	const auto file = currentChannel.renderPath;
	m_pWaveRenderer = m_waveRendererFactory();

	if (!m_pWaveRenderer) {
		AfxMessageBox(L"Unable to create wave renderer!", MB_ICONERROR);
		EndDialog(0);
	}

	famiTrackerView->SoloChannel(currentChannel.channelId);

	famiTrackerView->Invalidate();
	famiTrackerView->RedrawWindow();

	// Start rendering
	SetDlgItemTextW(IDC_PROGRESS_FILE, AfxFormattedW(IDS_WAVE_PROGRESS_FILE_FORMAT, file));

	if (!pSoundGen->RenderToFile(file, m_pWaveRenderer))
		EndDialog(0);
}

void CIndividualWavProgressDlg::OnTimer(UINT_PTR nIDEvent)
{
	// Update progress status
	auto pProgressBar = static_cast<CProgressCtrl*>(GetDlgItem(IDC_PROGRESS_BAR));
	auto pTotalProgressBar = static_cast<CProgressCtrl*>(GetDlgItem(IDC_PROGRESS_BAR2));
	const auto pSoundGen = CFamiTrackerEnv::GetSoundGenerator();

	SetDlgItemTextW(IDC_PROGRESS_LBL, conv::to_wide(m_pWaveRenderer->GetProgressString()).data());
	const auto currentRenderProgress = m_pWaveRenderer->GetProgressPercent();
	pProgressBar->SetPos(currentRenderProgress);		// // //
	pTotalProgressBar->SetPos(m_currentChannelIdx * 100 + currentRenderProgress);

	const DWORD Time = (GetTickCount() - m_dwStartTime) / 1000;		// // //
	SetDlgItemTextW(IDC_TIME, AfxFormattedW(IDS_WAVE_PROGRESS_ELAPSED_FORMAT, FormattedW(L"%02i:%02i", Time / 60, Time % 60)));

	if (!pSoundGen->IsRendering()) {
		m_pWaveRenderer->CloseOutputFile();		// // //

		if (m_currentChannelIdx != m_channelsToRender.size() - 1) {
			m_currentChannelIdx++;
			RenderChannel();
		} else {
			SetDlgItemTextW(IDC_CANCEL, CStringW(MAKEINTRESOURCE(IDS_WAVE_EXPORT_DONE)));
			CStringW title;
			GetWindowTextW(title);
			title.Append(L" ");
			title.Append(CStringW(MAKEINTRESOURCE(IDS_WAVE_EXPORT_FINISHED)));
			SetWindowTextW(title);
			pProgressBar->SetPos(100);
			pTotalProgressBar->SetPos(m_channelsToRender.size() * 100);
			KillTimer(0);
		}
	}

	CDialog::OnTimer(nIDEvent);
}
