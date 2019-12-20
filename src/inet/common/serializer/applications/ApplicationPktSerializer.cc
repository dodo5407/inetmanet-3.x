//
// Copyright (C) 2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen, Andras Varga
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//

#include "inet/common/serializer/SerializerUtil.h"

#include "inet/common/serializer/applications/ApplicationPktSerializer.h"

#include "inet/common/serializer/headers/defs.h"
#include "inet/common/serializer/headers/bsdint.h"
#include "inet/common/serializer/headers/in.h"
#include "inet/common/serializer/headers/in_systm.h"

#include "inet/common/serializer/TCPIPchecksum.h"
#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/networklayer/common/L3Address.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

namespace inet {

namespace serializer {


Register_Serializer(ApplicationPacket, UNKNOWN, 0, ApplicationPktSerializer);



void ApplicationPktSerializer::serialize(const cPacket *_pkt, Buffer &b, Context& c)
{
    ASSERT(b.getPos() == 0);

    const ApplicationPacket *AppPkt = check_and_cast<const ApplicationPacket *>(_pkt);
    b.writeUint64(AppPkt->getSequenceNumber());
    if(_pkt->getByteLength() > 64)
        b.fillNBytes(AppPkt->getByteLength() - 64, 0);

}



cPacket *ApplicationPktSerializer::deserialize(const Buffer &b, Context& c)
{
    ASSERT(b.getPos() == 0);

    return nullptr;
}


} // namespace serializer

} // namespace inet

