// Copyright (C) 2012 OpenSim Ltd
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU Lesser General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211FHSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211IRMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211DSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HRDSSSMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211OFDMMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ERPOFDMMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211HTMode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211VHTMode.h"
#include <algorithm>

namespace inet {

namespace physicallayer {

Register_Abstract_Class(Ieee80211ModeSet);

const DelayedInitializer<std::vector<Ieee80211ModeSet>> Ieee80211ModeSet::modeSets([]() { return new std::vector<Ieee80211ModeSet> {
    Ieee80211ModeSet("a", {
        { true, &Ieee80211OFDMCompliantModes::ofdmMode6MbpsCS20MHz },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode9MbpsCS20MHz },
        { true, &Ieee80211OFDMCompliantModes::ofdmMode12MbpsCS20MHz },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode18MbpsCS20MHz },
        { true, &Ieee80211OFDMCompliantModes::ofdmMode24MbpsCS20MHz },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode36Mbps },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode48Mbps },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode54Mbps },
    }),
    Ieee80211ModeSet("b", {
        { true, &Ieee80211DsssCompliantModes::dsssMode1Mbps },
        { true, &Ieee80211DsssCompliantModes::dsssMode2Mbps },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode5_5MbpsCckLongPreamble },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode11MbpsCckLongPreamble },
    }),
    // TODO: slotTime, cwMin, cwMax must be identical in all modes
    Ieee80211ModeSet("g(mixed)", {
        { true, &Ieee80211DsssCompliantModes::dsssMode1Mbps },
        { true, &Ieee80211DsssCompliantModes::dsssMode2Mbps },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode5_5MbpsCckLongPreamble },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode6Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode9Mbps },
        { true, &Ieee80211HrDsssCompliantModes::hrDsssMode11MbpsCckLongPreamble },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode12Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode18Mbps },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode24Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode36Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode48Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOfdmMode54Mbps }, // TODO: ERP-CCK, ERP-PBCC, DSSS-OFDM
    }),
    Ieee80211ModeSet("g(erp)", {
        { true, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode6Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode9Mbps },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode12Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode18Mbps },
        { true, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode24Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode36Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode48Mbps },
        { false, &Ieee80211ErpOfdmCompliantModes::erpOnlyOfdmMode54Mbps },
    }),
    Ieee80211ModeSet("p", {
        { true, &Ieee80211OFDMCompliantModes::ofdmMode3MbpsCS10MHz },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode4_5MbpsCS10MHz },
        { true, &Ieee80211OFDMCompliantModes::ofdmMode6MbpsCS10MHz },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode9MbpsCS10MHz },
        { true, &Ieee80211OFDMCompliantModes::ofdmMode12MbpsCS10MHz },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode18MbpsCS10MHz },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode24MbpsCS10MHz },
        { false, &Ieee80211OFDMCompliantModes::ofdmMode27Mbps },
        }),
    Ieee80211ModeSet("n(mixed-2.4Ghz)", { // This table is not complete; it only contains 2.4GHz homogeneous spatial streams, all mandatory and optional modes
        { true, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs0BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs1BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs2BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs3BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs4BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs5BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs6BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs7BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs8BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs9BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs10BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs11BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs12BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs13BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs14BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs15BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs16BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs17BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs18BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs19BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs20BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs21BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs22BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs23BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs24BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs25BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs26BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs27BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs28BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs29BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs30BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs31BW20MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs0BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs1BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs2BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs3BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs4BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs5BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs6BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs7BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs8BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs9BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs10BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs11BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs12BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs13BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs14BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs15BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs16BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs17BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs18BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs19BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs20BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs21BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs22BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs23BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs24BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs25BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs26BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs27BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs28BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs29BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs30BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211HTCompliantModes::getCompliantMode(&Ieee80211HTMCSTable::htMcs31BW40MHz, Ieee80211HTMode::BAND_2_4GHZ, Ieee80211HTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211HTModeBase::HT_GUARD_INTERVAL_SHORT) }
    }),
    Ieee80211ModeSet("ac", {
        { true, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW20MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW20MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW20MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW20MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW20MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW20MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW20MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW20MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_LONG) },
        { true, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW20MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_LONG) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW20MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW20MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW20MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW20MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW20MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW20MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW20MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW20MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW20MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW20MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW20MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW20MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW20MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW20MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW20MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW20MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW20MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW20MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW20MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW20MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW20MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW20MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW20MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW20MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW20MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW20MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW20MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW20MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW20MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW20MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW20MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW20MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW20MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW20MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW20MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW20MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW20MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW20MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW20MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW20MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW20MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW20MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW20MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW20MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW20MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW20MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW20MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW20MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW20MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW20MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW20MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW20MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW20MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW20MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW20MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW20MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW20MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW20MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW20MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW20MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW20MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW20MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW20MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW20MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW20MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW40MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW40MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW40MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW40MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW40MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW40MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW40MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW40MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW40MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW40MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW40MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW40MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW40MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW40MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW40MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW40MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW40MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW40MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW40MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW40MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW40MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW40MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW40MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW40MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW40MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW40MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW40MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW40MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW40MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW40MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW40MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW40MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW40MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW40MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW40MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW40MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW40MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW40MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW40MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW40MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW40MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW40MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW40MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW40MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW40MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW40MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW40MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW40MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW40MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW40MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW40MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW40MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW40MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW40MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW40MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW40MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW40MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW40MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW40MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW40MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW40MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW40MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW40MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW40MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW40MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW40MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW40MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW40MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW40MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW40MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW40MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW40MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW40MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW40MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW40MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW40MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW40MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW40MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW40MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW40MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW80MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW80MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW80MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW80MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW80MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW80MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW80MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW80MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW80MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW80MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW80MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW80MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW80MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW80MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW80MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW80MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW80MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW80MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW80MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW80MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW80MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW80MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW80MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW80MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW80MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW80MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW80MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW80MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW80MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW80MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW80MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW80MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW80MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW80MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW80MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW80MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW80MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW80MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW80MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW80MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW80MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW80MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW80MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW80MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW80MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW80MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW80MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW80MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW80MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW80MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW80MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW80MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW80MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW80MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW80MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW80MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW80MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW80MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW80MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW80MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW80MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW80MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW80MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW80MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW80MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW80MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW80MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW80MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW80MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW80MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW80MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW80MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW80MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW80MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW80MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW80MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW80MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW160MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW160MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW160MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW160MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW160MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW160MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW160MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW160MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW160MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW160MHzNss1, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW160MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW160MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW160MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW160MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW160MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW160MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW160MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW160MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW160MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW160MHzNss2, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW160MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW160MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW160MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW160MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW160MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW160MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW160MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW160MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW160MHzNss3, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW160MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW160MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW160MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW160MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW160MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW160MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW160MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW160MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW160MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW160MHzNss4, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW160MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW160MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW160MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW160MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW160MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW160MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW160MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW160MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW160MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW160MHzNss5, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW160MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW160MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW160MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW160MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW160MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW160MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW160MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW160MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW160MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW160MHzNss6, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW160MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW160MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW160MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW160MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW160MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW160MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW160MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW160MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW160MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW160MHzNss7, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs0BW160MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs1BW160MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs2BW160MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs3BW160MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs4BW160MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs5BW160MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs6BW160MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs7BW160MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs8BW160MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
        { false, Ieee80211VHTCompliantModes::getCompliantMode(&Ieee80211VHTMCSTable::vhtMcs9BW160MHzNss8, Ieee80211VHTMode::BAND_5GHZ, Ieee80211VHTPreambleMode::HT_PREAMBLE_MIXED, Ieee80211VHTModeBase::HT_GUARD_INTERVAL_SHORT) },
}),}; });

Ieee80211ModeSet::Ieee80211ModeSet(const char *name, const std::vector<Entry> entries) :
    name(name),
    entries(entries)
{
    std::vector<Entry> *nonConstEntries = const_cast<std::vector<Entry> *>(&this->entries);
    std::stable_sort(nonConstEntries->begin(), nonConstEntries->end(), EntryNetBitrateComparator());
    auto referenceMode = entries[0].mode;
    for (auto entry : entries) {
        auto mode = entry.mode;
        if (mode->getSifsTime() != referenceMode->getSifsTime() ||
            mode->getSlotTime() != referenceMode->getSlotTime() ||
            mode->getPhyRxStartDelay() != referenceMode->getPhyRxStartDelay())
        {
            // FIXME: throw cRuntimeError("Sifs, slot and phyRxStartDelay time must be identical within a ModeSet");
        }
    }
}

int Ieee80211ModeSet::findModeIndex(const IIeee80211Mode *mode) const
{
    for (int index = 0; index < (int)entries.size(); index++)
        if (entries[index].mode == mode)
            return index;
    return -1;
}

int Ieee80211ModeSet::getModeIndex(const IIeee80211Mode *mode) const
{
    int index = findModeIndex(mode);
    if (index < 0)
        throw cRuntimeError("Unknown mode");
    else
        return index;
}

bool Ieee80211ModeSet::getIsMandatory(const IIeee80211Mode *mode) const
{
    return entries[getModeIndex(mode)].isMandatory;
}

const IIeee80211Mode *Ieee80211ModeSet::findMode(bps bitrate) const
{
    for (int index = 0; index < (int)entries.size(); index++) {
        const IIeee80211Mode *mode = entries[index].mode;
        EV_DEBUG << bps(std::round(mode->getDataMode()->getNetBitrate().get() / 1e5) * 1e5) << "\n";
        if (bps(std::round(mode->getDataMode()->getNetBitrate().get() / 1e5) * 1e5) == bitrate) // round to 1 decimal in Mbits
            return entries[index].mode;
    }
    return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getMode(bps bitrate) const
{
    const IIeee80211Mode *mode = findMode(bitrate);
    if (mode == nullptr)
        throw cRuntimeError("Unknown bitrate: %g in operation mode: '%s'", bitrate.get(), getName());
    else
        return mode;
}

const IIeee80211Mode *Ieee80211ModeSet::getSlowestMode() const
{
    return entries.front().mode;
}

const IIeee80211Mode *Ieee80211ModeSet::getFastestMode() const
{
    return entries.back().mode;
}

const IIeee80211Mode *Ieee80211ModeSet::getSlowerMode(const IIeee80211Mode *mode) const
{
    int index = findModeIndex(mode);
    if (index > 0)
        return entries[index - 1].mode;
    else
        return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getFasterMode(const IIeee80211Mode *mode) const
{
    int index = findModeIndex(mode);
    if (index >= 0 && index < (int)entries.size()-1)
        return entries[index + 1].mode;
    else
        return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getSlowestMandatoryMode() const
{
    for (int i = 0; i < (int)entries.size(); i++)
        if (entries[i].isMandatory)
            return entries[i].mode;
    return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getFastestMandatoryMode() const
{
    for (int i = (int)entries.size()-1; i >= 0; i--)
        if (entries[i].isMandatory)
            return entries[i].mode;
    return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getSlowerMandatoryMode(const IIeee80211Mode *mode) const
{
    int index = findModeIndex(mode);
    if (index > 0)
        for (int i = index-1; i >= 0; i--)
            if (entries[i].isMandatory)
                return entries[i].mode;
    return nullptr;
}

const IIeee80211Mode *Ieee80211ModeSet::getFasterMandatoryMode(const IIeee80211Mode *mode) const
{
    int index = findModeIndex(mode);
    if (index >= 0)
        for (int i = index+1; i < (int)entries.size(); i++)
            if (entries[i].isMandatory)
                return entries[i].mode;
    return nullptr;
}

const Ieee80211ModeSet *Ieee80211ModeSet::findModeSet(const char *mode)
{
    for (int index = 0; index < (int)(&modeSets)->size(); index++) {
        const Ieee80211ModeSet *modeSet = &(&modeSets)->at(index);
        if (strcmp(modeSet->getName(), mode) == 0)
            return modeSet;
    }
    return nullptr;
}

const Ieee80211ModeSet *Ieee80211ModeSet::getModeSet(const char *mode)
{
    const Ieee80211ModeSet *modeSet = findModeSet(mode);
    if (modeSet == nullptr) {
        std::string validModeSets;
        for (int index = 0; index < (int)(&modeSets)->size(); index++) {
            const Ieee80211ModeSet *modeSet = &(&modeSets)->at(index);
            validModeSets += std::string("'") + modeSet->getName() + "' ";
        }
        throw cRuntimeError("Unknown 802.11 operational mode: '%s', valid modes are: %s", mode, validModeSets.c_str());
    }
    else
        return modeSet;
}

} // namespace physicallayer

} // namespace inet

