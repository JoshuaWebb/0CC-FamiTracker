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

#include "SoundChipService.h"
#include "SoundChipTypeImpl.h"
#include "APU/Types.h"
#include "APU/SoundChip.h"
#include "ChipHandler.h"

void CSoundChipService::AddType(std::unique_ptr<CSoundChipType> stype) {
	sound_chip_t id = stype->GetID();

	if (id == sound_chip_t::NONE)
		throw std::invalid_argument {"Cannot add sound chip with ID sound_chip_t::NONE"};
	if (types_.count(id))
		throw std::invalid_argument {"Cannot add sound chips with same ID"};

	types_.try_emplace(id, std::move(stype));
}

void CSoundChipService::AddDefaultTypes() {
	AddType(std::make_unique<CSoundChipType2A03>());
	AddType(std::make_unique<CSoundChipTypeVRC6>());
	AddType(std::make_unique<CSoundChipTypeVRC7>());
	AddType(std::make_unique<CSoundChipTypeFDS>());
	AddType(std::make_unique<CSoundChipTypeMMC5>());
	AddType(std::make_unique<CSoundChipTypeN163>());
	AddType(std::make_unique<CSoundChipTypeS5B>());
}

std::size_t CSoundChipService::GetSupportedChannelCount(sound_chip_t chip) const {
	auto *pChip = GetTypePtr(chip);
	return pChip ? pChip->GetSupportedChannelCount() : static_cast<std::size_t>(0u);
}

std::size_t CSoundChipService::GetChannelSubindex(chan_id_t ch) const {
	auto id = value_cast(ch);
	for (auto &x : types_) {
		auto f = value_cast(x.second->GetFirstChannelID());
		if (id >= f && id < f + x.second->GetSupportedChannelCount())
			return static_cast<std::size_t>(id - f);
	}
	return static_cast<std::size_t>(-1);
}

chan_id_t CSoundChipService::MakeChannelIndex(sound_chip_t chip, std::size_t subindex) const {
	if (auto *pChip = GetTypePtr(chip))
		if (subindex < pChip->GetSupportedChannelCount())
			return enum_cast<chan_id_t>(value_cast(pChip->GetFirstChannelID()) + subindex);
	return chan_id_t::NONE;
}

std::string_view CSoundChipService::GetShortChipName(sound_chip_t chip) const {
	return GetType(chip).GetShortName();
}

std::string_view CSoundChipService::GetFullChipName(sound_chip_t chip) const {
	return GetType(chip).GetFullName();
}

std::string_view CSoundChipService::GetShortChannelName(chan_id_t ch) const {
	auto id = value_cast(ch);
	for (auto &x : types_) {
		auto f = value_cast(x.second->GetFirstChannelID());
		if (id >= f && id < f + x.second->GetSupportedChannelCount())
			return x.second->GetShortChannelName(id - f);
	}
	throw std::invalid_argument {"Channel with given ID does not exist"};
}

std::string_view CSoundChipService::GetFullChannelName(chan_id_t ch) const {
	auto id = value_cast(ch);
	for (auto &x : types_) {
		auto f = value_cast(x.second->GetFirstChannelID());
		if (id >= f && id < f + x.second->GetSupportedChannelCount())
			return x.second->GetFullChannelName(id - f);
	}
	throw std::invalid_argument {"Channel with given ID does not exist"};
}

sound_chip_t CSoundChipService::GetChipFromChannel(chan_id_t ch) const {
	auto id = value_cast(ch);
	for (auto &[c, ty] : types_) {
		auto f = value_cast(ty->GetFirstChannelID());
		if (id >= f && id < f + ty->GetSupportedChannelCount())
			return c;
	}
	return sound_chip_t::NONE;
}

sound_chip_t CSoundChipService::GetChipFromString(std::string_view sv) const {
	for (const auto &[c, ty] : types_)
		if (ty->GetShortName() == sv)
			return c;
	return sound_chip_t::NONE;
}

std::unique_ptr<CSoundChip> CSoundChipService::MakeSoundChipDriver(sound_chip_t chip, CMixer &mixer) const {
	return GetType(chip).MakeSoundDriver(mixer);
}

std::unique_ptr<CChipHandler> CSoundChipService::MakeChipHandler(sound_chip_t chip) const {
	return GetType(chip).MakeChipHandler();
}

const CSoundChipType &CSoundChipService::GetType(sound_chip_t chip) const {
	auto it = types_.find(chip);
	if (it == types_.end())
		throw std::invalid_argument {"Sound chip type with given ID does not exist"};
	return *it->second;
}

const CSoundChipType *CSoundChipService::GetTypePtr(sound_chip_t chip) const {
	auto it = types_.find(chip);
	return it == types_.end() ? nullptr : std::addressof(*it->second);
}
