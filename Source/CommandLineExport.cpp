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

#include "CommandLineExport.h"
#include "FamiTrackerDoc.h"
#include "FamiTrackerModule.h"		// // //
#include "Compiler.h"
#include "SoundGen.h"
#include "TextExporter.h"
#include "str_conv/str_conv.hpp"		// // //

// Command line export logger
class CCommandLineLog : public CCompilerLog {
public:
	explicit CCommandLineLog(CStdioFile *pFile) : m_pFile(pFile) { }
	void WriteLog(std::string_view text) override {
		m_pFile->Write(text.data(), text.size());
	};
	void Clear() override { }

private:
	CStdioFile *m_pFile;
};

// Command line export function
void CCommandLineExport::CommandLineExport(const CStringW &fileIn, const CStringW &fileOut, const CStringW &fileLog, const CStringW &fileDPCM) {
	// open log
	bool bLog = false;
	CStdioFile fLog;
	if (fileLog.GetLength() > 0) {
		if (fLog.Open(fileLog, CFile::modeCreate | CFile::modeWrite | CFile::typeText, NULL))
			bLog = true;
	}

	// create CFamiTrackerDoc for export
	CRuntimeClass *pRuntimeClass = RUNTIME_CLASS(CFamiTrackerDoc);
	CObject *pObject = pRuntimeClass->CreateObject();
	if (pObject == NULL || !pObject->IsKindOf(RUNTIME_CLASS(CFamiTrackerDoc))) {
		if (bLog)
			fLog.WriteString(L"Error: unable to create CFamiTrackerDoc\n");
		return;
	}
	auto pExportDoc = static_cast<CFamiTrackerDoc *>(pObject);

	// open file
	if (!pExportDoc->OnOpenDocument(fileIn)) {
		if (bLog) {
			fLog.WriteString(L"Error: unable to open document: ");
			fLog.WriteString(fileIn);
			fLog.WriteString(L"\n");
		}
		return;
	}
	if (bLog) {
		fLog.WriteString(L"Opened: ");
		fLog.WriteString(fileIn);
		fLog.WriteString(L"\n");
	}

	// find extension
	int nPos = fileOut.ReverseFind(WCHAR('.'));
	if (nPos < 0) {
		if (bLog) {
			fLog.WriteString(L"Error: export filename has no extension: ");
			fLog.WriteString(fileOut);
			fLog.WriteString(L"\n");
		}
		return;
	}
	CStringW ext = fileOut.Mid(nPos);

	const CFamiTrackerModule *pModule = pExportDoc->GetModule();		// // //

	// export
	if (0 == ext.CompareNoCase(L".nsf")) {
		CCompiler compiler(*pModule, bLog ? std::make_shared<CCommandLineLog>(&fLog) : nullptr);		// // //
		compiler.ExportNSF(fileOut, pModule->GetMachine());
		if (bLog) {
			fLog.WriteString(L"\nNSF export complete.\n");
		}
		return;
	}
	else if (0 == ext.CompareNoCase(L".nes")) {
		CCompiler compiler(*pModule, bLog ? std::make_shared<CCommandLineLog>(&fLog) : nullptr);		// // //
		compiler.ExportNES(fileOut, pModule->GetMachine() == machine_t::PAL);
		if (bLog) {
			fLog.WriteString(L"\nNES export complete.\n");
		}
		return;
	}
	// BIN export requires two files
	else if (0 == ext.CompareNoCase(L".bin")) {
		CCompiler compiler(*pModule, bLog ? std::make_shared<CCommandLineLog>(&fLog) : nullptr);		// // //
		compiler.ExportBIN(fileOut, fileDPCM);
		if (bLog) {
			fLog.WriteString(L"\nBIN export complete.\n");
		}
		return;
	}
	else if (0 == ext.CompareNoCase(L".prg")) {
		CCompiler compiler(*pModule, bLog ? std::make_shared<CCommandLineLog>(&fLog) : nullptr);		// // //
		compiler.ExportPRG(fileOut, pModule->GetMachine() == machine_t::PAL);
		if (bLog) {
			fLog.WriteString(L"\nPRG export complete.\n");
		}
		return;
	}
	else if (0 == ext.CompareNoCase(L".asm")) {
		CCompiler compiler(*pModule, bLog ? std::make_shared<CCommandLineLog>(&fLog) : nullptr);		// // //
		compiler.ExportASM(fileOut);
		if (bLog) {
			fLog.WriteString(L"\nASM export complete.\n");
		}
		return;
	}
	else if (0 == ext.CompareNoCase(L".nsfe"))		// // //
	{
		CCompiler compiler(*pModule, bLog ? std::make_shared<CCommandLineLog>(&fLog) : nullptr);		// // //
		compiler.ExportNSFE(fileOut, pModule->GetMachine());
		if (bLog) {
			fLog.WriteString(L"\nNSFe export complete.\n");
		}
		return;
	}
	else if (0 == ext.CompareNoCase(L".txt")) {
		CTextExport textExport;
		CStringA result = textExport.ExportFile(fileOut, *pExportDoc);		// // //
		if (result.GetLength() > 0) {
			if (bLog) {
				fLog.WriteString(L"Error: ");
				fLog.WriteString(conv::to_wide(result).data());
				fLog.WriteString(L"\n");
			}
		}
		else if (bLog) {
			fLog.WriteString(L"Exported: ");
			fLog.WriteString(fileOut);
			fLog.WriteString(L"\n");
		}
		return;
	}

	// // //

	if (bLog) {
		fLog.WriteString(L"Error: unable to find matching export extension for: ");
		fLog.WriteString(fileOut);
		fLog.WriteString(L"\n");
	}
	return;
}
