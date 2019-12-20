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

#include "inet/common/serializer/routing/ManetGeneralSerializer.h"

#include "inet/networklayer/common/IPProtocolId_m.h"
#include "inet/transportlayer/udp/UDPPacket.h"
#include "inet/routing/extras/base/ControlManetRouting_m.h"

#if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)
#include <netinet/in.h>    // htonl, ntohl, ...
#endif // if !defined(_WIN32) && !defined(__WIN32__) && !defined(WIN32) && !defined(__CYGWIN__) && !defined(_WIN64)

namespace inet {

namespace serializer {

Register_Serializer(inet::inetmanet::ControlManetRouting, IP_PROT, IP_PROT_MANET, ManetGeneralSerializer);


void ManetGeneralSerializer::serialize(const cPacket *_pkt, Buffer &b, Context& c)
{
    if(typeid(*_pkt) == typeid(UDPPacket)) {
        SerializerBase::lookupAndSerialize(_pkt, b, c, IP_PROT, IP_PROT_UDP);
    }else {
        throw cRuntimeError("Packet encapsulate other than UDP not implement yet!");
    }

    return;
}

cPacket *ManetGeneralSerializer::deserialize(const Buffer &b, Context& c)
{
    cPacket *encapPacket = nullptr;
    SerializerBase& serializer = lookupDeserializer(c, IP_PROT, IP_PROT_UDP);
    encapPacket = serializer.deserializePacket(b, c);
    return encapPacket;
}

} // namespace serializer

} // namespace inet

