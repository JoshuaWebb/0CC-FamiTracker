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

#include "BookmarkCollection.h"
#include <algorithm>
#include <string>
#include <exception>
#include "Bookmark.h"
#include "FamiTrackerTypes.h" // constants

CBookmarkCollection::CBookmarkCollection()
{
}

unsigned CBookmarkCollection::GetCount() const
{
	return m_pBookmark.size();
}

CBookmark *CBookmarkCollection::GetBookmark(unsigned Index) const
{
	return Index < m_pBookmark.size() ? m_pBookmark[Index].get() : nullptr;
}

bool CBookmarkCollection::AddBookmark(std::unique_ptr<CBookmark> pMark)
{
	try {
		m_pBookmark.push_back(std::move(pMark));
		return true;
	}
	catch (std::exception) {
		return false;
	}
}

bool CBookmarkCollection::SetBookmark(unsigned Index, std::unique_ptr<CBookmark> pMark)
{
	if (m_pBookmark[Index] && m_pBookmark[Index]->IsEqual(*pMark))
		return false;
	m_pBookmark[Index] = std::move(pMark);
	return true;
}

bool CBookmarkCollection::InsertBookmark(unsigned Index, std::unique_ptr<CBookmark> pMark)
{
	if (Index > m_pBookmark.size())
		return false;
	m_pBookmark.emplace(m_pBookmark.begin() + Index, std::move(pMark));
	return true;
}

bool CBookmarkCollection::RemoveBookmark(unsigned Index)
{
	if (Index >= m_pBookmark.size())
		return false;
	m_pBookmark.erase(m_pBookmark.begin() + Index);
	return true;
}

bool CBookmarkCollection::ClearBookmarks()
{
	if (m_pBookmark.empty())
		return false;
	m_pBookmark.clear();
	return true;
}

bool CBookmarkCollection::SwapBookmarks(unsigned A, unsigned B)
{
	if (A == B)
		return false;
	m_pBookmark[A].swap(m_pBookmark[B]);
	return true;
}

void CBookmarkCollection::InsertFrames(unsigned Frame, unsigned Count)
{
	std::for_each(m_pBookmark.begin(), m_pBookmark.end(),
				  [&] (std::unique_ptr<CBookmark> &a) { if (a->m_iFrame >= Frame) a->m_iFrame += Count; });
}

void CBookmarkCollection::RemoveFrames(unsigned Frame, unsigned Count)
{
	m_pBookmark.erase(
		std::remove_if(m_pBookmark.begin(), m_pBookmark.end(),
			[&] (std::unique_ptr<CBookmark> &a) { return a->m_iFrame >= Frame && a->m_iFrame < Frame + Count; }),
		m_pBookmark.end());
	std::for_each(m_pBookmark.begin(), m_pBookmark.end(),
				  [&] (std::unique_ptr<CBookmark> &a) { if (a->m_iFrame >= Frame) a->m_iFrame -= Count; });
}

void CBookmarkCollection::SwapFrames(unsigned A, unsigned B)
{
	std::for_each(m_pBookmark.begin(), m_pBookmark.end(), [&] (std::unique_ptr<CBookmark> &a) {
		if (a->m_iFrame == A) a->m_iFrame = B;
		else if (a->m_iFrame == B) a->m_iFrame = A;
	});
}

int CBookmarkCollection::GetBookmarkIndex(const CBookmark *pMark) const
{
	if (unsigned Count = GetCount())
		for (size_t i = 0; i < Count; ++i)
			if (m_pBookmark[i].get() == pMark)
				return i;
	return -1;
}

CBookmark *CBookmarkCollection::FindAt(unsigned Frame, unsigned Row) const
{
	const CBookmark tmp(Frame, Row);
	auto it = std::find_if(m_pBookmark.begin(), m_pBookmark.end(), [&] (const std::unique_ptr<CBookmark> &a) {
		return *a == tmp;
	});
	return it == m_pBookmark.end() ? nullptr : it->get();
}

void CBookmarkCollection::RemoveAt(unsigned Frame, unsigned Row)
{
	const CBookmark tmp(Frame, Row);
	m_pBookmark.erase(
		std::remove_if(m_pBookmark.begin(), m_pBookmark.end(),
			[&] (const std::unique_ptr<CBookmark> &a) { return *a == tmp; }),
		m_pBookmark.end());
}

CBookmark *CBookmarkCollection::FindNext(unsigned Frame, unsigned Row) const
{
	if (m_pBookmark.empty()) return nullptr;
	CBookmark temp(Frame, Row);
	return std::min_element(m_pBookmark.begin(), m_pBookmark.end(),
					 [&] (const std::unique_ptr<CBookmark> &a, const std::unique_ptr<CBookmark> &b) {
		return static_cast<unsigned>(a->Distance(temp) - 1) < static_cast<unsigned>(b->Distance(temp) - 1);
	})->get();
}

CBookmark *CBookmarkCollection::FindPrevious(unsigned Frame, unsigned Row) const
{
	if (m_pBookmark.empty()) return nullptr;
	CBookmark temp(Frame, Row);
	return std::min_element(m_pBookmark.begin(), m_pBookmark.end(),
					 [&] (const std::unique_ptr<CBookmark> &a, const std::unique_ptr<CBookmark> &b) {
		return static_cast<unsigned>(temp.Distance(*a) - 1) < static_cast<unsigned>(temp.Distance(*b) - 1);
	})->get();
}

bool CBookmarkCollection::SortByName(bool Desc)
{
	const auto sortFunc = [] (const std::unique_ptr<CBookmark> &a, const std::unique_ptr<CBookmark> &b) {
		return a->m_sName < b->m_sName;
	};

	if (Desc) std::reverse(m_pBookmark.begin(), m_pBookmark.end());
	bool Change = !std::is_sorted(m_pBookmark.begin(), m_pBookmark.end(), sortFunc);
	if (Desc) std::reverse(m_pBookmark.begin(), m_pBookmark.end());
	if (!Change) return false;
	std::stable_sort(m_pBookmark.begin(), m_pBookmark.end(), sortFunc);
	if (Desc) std::reverse(m_pBookmark.begin(), m_pBookmark.end());
	return true;
}

bool CBookmarkCollection::SortByPosition(bool Desc)
{
	const auto sortFunc = [] (const std::unique_ptr<CBookmark> &a, const std::unique_ptr<CBookmark> &b) { return *a < *b; };

	if (Desc) std::reverse(m_pBookmark.begin(), m_pBookmark.end());
	bool Change = !std::is_sorted(m_pBookmark.begin(), m_pBookmark.end(), sortFunc);
	if (Desc) std::reverse(m_pBookmark.begin(), m_pBookmark.end());
	if (!Change) return false;
	std::stable_sort(m_pBookmark.begin(), m_pBookmark.end(), sortFunc);
	if (Desc) std::reverse(m_pBookmark.begin(), m_pBookmark.end());
	return true;
}
