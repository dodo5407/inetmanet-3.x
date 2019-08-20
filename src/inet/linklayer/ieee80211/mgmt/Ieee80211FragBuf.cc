//
// Copyright (C) 2004 Andras Varga
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

#include <stdlib.h>
#include <string.h>

#include "inet/linklayer/ieee80211/mgmt/Ieee80211FragBuf.h"

#include "inet/common/RawPacket.h"
#include "inet/common/serializer/SerializerBase.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"

namespace inet {

namespace ieee80211 {

//TODO need solution for fragments with encapsulated RawPacket contains bytes of fragment or bytes of total packet

Ieee80211FragBuf::Ieee80211FragBuf()
{

}

Ieee80211FragBuf::~Ieee80211FragBuf()
{
    while(!bufs.empty()){
        if (bufs.begin()->second.datagram)
            delete bufs.begin()->second.datagram;

        bufs.erase(bufs.begin());
    }
}

void Ieee80211FragBuf::init()
{

}

Ieee80211DataFrameWithSNAP *Ieee80211FragBuf::addFragment(Ieee80211DataFrameWithSNAP *datagram, simtime_t now)
{
    // find datagram buffer
    Key key;
    key.id = datagram->getIdentification();
    key.src = datagram->getTransmitterAddress();
    key.dest = datagram->getReceiverAddress();

    auto i = bufs.find(key);

    DatagramBuffer *buf = nullptr;

    if(i == bufs.end()){
        buf = &bufs[key];
        buf->datagram = nullptr;
    }else{
        // use existing buffer
        buf = &(i->second);
    }

    // add fragment into reassembly buffer
    int bytes = datagram->getByteLength() - datagram->getHeaderLength();
    bool isComplete = buf->buf.addFragment(datagram->getFragmentOffset(),
                datagram->getFragmentOffset() + bytes,
                !datagram->getMoreFragments());

    // store datagram. Only one fragment carries the actual modelled
    // content (getEncapsulatedPacket()), other (empty) ones are only
    // preserved so that we can send them in ICMP if reassembly times out.
    if(buf->datagram == nullptr){
        if (dynamic_cast<RawPacket *>(datagram->getEncapsulatedPacket())) {
            RawPacket * rp = static_cast<RawPacket *>(datagram->getEncapsulatedPacket());
            // move raw bytes to its offset in RawPacket
            if (datagram->getFragmentOffset()) {
                rp->getByteArray().expandData(datagram->getFragmentOffset(), 0);
                rp->addByteLength(datagram->getFragmentOffset());
            }
        }

        buf->datagram = datagram;
    }
    else if (buf->datagram->getEncapsulatedPacket() == nullptr && datagram->getEncapsulatedPacket() != nullptr) {
        delete buf->datagram;

        if (dynamic_cast<RawPacket *>(datagram->getEncapsulatedPacket())) {
            RawPacket *rp = static_cast<RawPacket *>(datagram->getEncapsulatedPacket());
            // move raw bytes to its offset in RawPacket
            if (datagram->getFragmentOffset()) {
                rp->getByteArray().expandData(datagram->getFragmentOffset(), 0);
                rp->setByteLength(rp->getByteArray().getDataArraySize());
            }
        }

        buf->datagram = datagram;
    }
    else {
        RawPacket *brp = dynamic_cast<RawPacket *>(buf->datagram->getEncapsulatedPacket());
        RawPacket *rp = dynamic_cast<RawPacket *>(datagram->getEncapsulatedPacket());
        if (brp && rp) {
            // merge encapsulated raw data
            brp->getByteArray().copyDataFromBuffer(datagram->getFragmentOffset(), rp->getByteArray().getDataPtr(), rp->getByteArray().getDataArraySize());
            brp->setByteLength(rp->getByteArray().getDataArraySize());
        }
        delete datagram;
    }

    // do we have the complete datagram?
    if (isComplete) {
        // datagram complete: deallocate buffer and return complete datagram
        Ieee80211DataFrameWithSNAP *ret = buf->datagram;
        ret->setByteLength(ret->getHeaderLength() + buf->buf.getTotalLength());
        ret->setFragmentOffset(0);
        ret->setMoreFragments(false);
        bufs.erase(i);
        if (dynamic_cast<RawPacket *>(ret->getEncapsulatedPacket())) {
            using namespace serializer;
            RawPacket *rp = static_cast<RawPacket *>(ret->getEncapsulatedPacket());
            char ipv4addresses[8];    // 2 * 4 bytes for 2 IPv4 addresses
            Buffer hdr(ipv4addresses, sizeof(ipv4addresses));
            hdr.writeMACAddress(ret->getTransmitterAddress());
            hdr.writeMACAddress(ret->getReceiverAddress());
            Buffer b(rp->getByteArray().getDataPtr(), rp->getByteArray().getDataArraySize());
            Context c;
            c.l3AddressesPtr = ipv4addresses;
            c.l3AddressesLength = sizeof(ipv4addresses);
            cPacket *enc = SerializerBase::lookupAndDeserialize(b, c, ETHERTYPE, 0);
            if (enc) {
                delete ret->decapsulate();
                ret->encapsulate(enc);
            }
        }
        return ret;
    }else{
        // there are still missing fragments
        buf->lastupdate = now;
        return nullptr;
    }
}

void Ieee80211FragBuf::purgeStaleFragments(simtime_t lastupdate)
{
    // this method shouldn't be called too often because iteration on
    // an std::map is *very* slow...

    for(auto i = bufs.begin(); i != bufs.end();) {
        // if too old, remove it
        DatagramBuffer& buf = i->second;
        if(buf.lastupdate < lastupdate){
            if(buf.datagram)
                delete buf.datagram;

            // delete
            auto oldi = i++;
            bufs.erase(oldi);
        }
        else {
            ++i;
        }
    }
}

} // namespace ieee80211

} // namespace inet
