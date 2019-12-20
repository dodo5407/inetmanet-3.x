//
// Copyright (C) 2004 Andras Varga
//               2005 Christian Dankbar, Irene Ruengeler, Michael Tuexen
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

#ifndef __INET_OLSRSERIALIZER_H
#define __INET_OLSRSERIALIZER_H

#include "inet/common/serializer/SerializerBase.h"
#include "inet/routing/extras/olsr/OLSRpkt_m.h"

namespace inet {

namespace serializer {

using namespace inetmanet;

/**
 * Converts between OLSRPacket and binary (network byte order) OLSR header.
 */
class INET_API OLSRSerializer : public SerializerBase
{
  protected:
    virtual void serialize(const cPacket *pkt, Buffer &b, Context& context) override;
    virtual cPacket *deserialize(const Buffer &b, Context& context) override;
    virtual void serializeMsg(Buffer &b, Context& c, const OLSR_msg& _msg);
    virtual void deserializeMsg(const Buffer &b, Context& c, OLSR_msg& _msg);


  public:
    OLSRSerializer(const char *name = nullptr) : SerializerBase(name) {}
};

} // namespace serializer

} // namespace inet

#endif // ifndef __INET_OLSRSERIALIZER_H

