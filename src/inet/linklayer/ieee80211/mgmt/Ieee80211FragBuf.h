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

#ifndef __INET_IEEE80211FRAGBUF_H
#define __INET_IEEE80211FRAGBUF_H

#include <map>

#include "inet/common/INETDefs.h"

#include "inet/linklayer/common/MACAddress.h"
#include "inet/common/ReassemblyBuffer.h"

namespace inet {

namespace ieee80211 {

class Ieee80211DataFrameWithSNAP;

/**
 * Reassembly buffer for fragmented IPv4 datagrams.
 */
class INET_API Ieee80211FragBuf
{
  protected:
    //
    // Key for finding the reassembly buffer for a datagram.
    //
    struct Key
    {
        ushort id;
        MACAddress src;
        MACAddress dest;

        inline bool operator<(const Key& b) const
        {
            return (id != b.id) ? (id < b.id) : (src != b.src) ? (src < b.src) : (dest < b.dest);
        }
    };

    //
    // Reassembly buffer for the datagram
    //
    struct DatagramBuffer
    {
        ReassemblyBuffer buf;    // reassembly buffer
        Ieee80211DataFrameWithSNAP *datagram;    // the actual datagram
        simtime_t lastupdate;    // last time a new fragment arrived
    };

    // we use std::map for fast lookup by datagram Id
    typedef std::map<Key, DatagramBuffer> Buffers;

    // the reassembly buffers
    Buffers bufs;


  public:
    /**
     * Ctor.
     */
    Ieee80211FragBuf();

    /**
     * Dtor.
     */
    ~Ieee80211FragBuf();

    /**
     * Initialize fragmentation buffer. ICMP module is needed for sending
     * TIME_EXCEEDED ICMP message in purgeStaleFragments().
     */
    void init();

    /**
     * Takes a fragment and inserts it into the reassembly buffer.
     * If this fragment completes a datagram, the full reassembled
     * datagram is returned, otherwise nullptr.
     */
    Ieee80211DataFrameWithSNAP *addFragment(Ieee80211DataFrameWithSNAP *datagram, simtime_t now);

    /**
     * Throws out all fragments which are incomplete and their
     * last update (last fragment arrival) was before "lastupdate",
     * and sends ICMP TIME EXCEEDED message about them.
     *
     * Timeout should be between 60 seconds and 120 seconds (RFC1122).
     * This method should be called more frequently, maybe every
     * 10..30 seconds or so.
     */
    void purgeStaleFragments(simtime_t lastupdate);
};

} // namespace ieee80211

} // namespace inet

#endif // ifndef __INET_IPV4FRAGBUF_H
