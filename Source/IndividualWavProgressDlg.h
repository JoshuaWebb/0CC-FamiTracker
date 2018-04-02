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
#include "../resource.h"		// // //
#include <memory>		// // //
#include <functional>
#include "APU/2A03Chan.h"

class CWaveRenderer;		// // //

struct channel_to_render
{
	channel_to_render(chan_id_t channel_id, CString render_path)
		: channelId(channel_id),
		  renderPath(render_path)
	{
	}

	chan_id_t channelId;
	CString renderPath;
};

// CIndividualWavProgressDlg dialog

class CIndividualWavProgressDlg : public CDialog
{
	DECLARE_DYNAMIC(CIndividualWavProgressDlg)

public:
	CIndividualWavProgressDlg(CWnd* pParent = NULL);   // standard constructor
	virtual ~CIndividualWavProgressDlg();

	void BeginRender(std::function<std::unique_ptr<CWaveRenderer>()> waveRendererFactory, std::vector<channel_to_render> channelsToRender);		// // //

// Dialog Data
	enum { IDD = IDD_INDIVIDUAL_WAVE_PROGRESS };

protected:
	DWORD m_dwStartTime;
	std::shared_ptr<CWaveRenderer> m_pWaveRenderer;		// // //
	std::function<std::unique_ptr<CWaveRenderer>()> m_waveRendererFactory;
	std::vector<channel_to_render> m_channelsToRender;

	size_t m_currentChannelIdx;

protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support

	DECLARE_MESSAGE_MAP()

private:
	void RenderChannel();

public:
	afx_msg void OnBnClickedCancel();
	virtual BOOL OnInitDialog();
	afx_msg void OnTimer(UINT_PTR nIDEvent);
};
