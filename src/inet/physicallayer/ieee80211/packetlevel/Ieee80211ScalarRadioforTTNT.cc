//
// Copyright (C) 2013 OpenSim Ltd.
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

#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ScalarRadioforTTNT.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAdhocforFreqHop.h"

namespace inet {

namespace physicallayer {

Define_Module(Ieee80211ScalarRadioforTTNT);

void Ieee80211ScalarRadioforTTNT::endTransmission()
{
    Radio::endTransmission();
    emit(ieee80211::Ieee80211MgmtAdhocforFreqHop::macTrasmissionFinishedSignal, (cObject *)0);
}

} // namespace physicallayer

} // namespace inet
