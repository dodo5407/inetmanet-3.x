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

#include "inet/common/serializer/routing/OLSRSerializer.h"

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

using namespace inetmanet;

Register_Serializer(OLSR_pkt, UNKNOWN, 0, OLSRSerializer);



void OLSRSerializer::serialize(const cPacket *_pkt, Buffer &b, Context& c)
{
    ASSERT(b.getPos() == 0);

    const OLSR_pkt *OLSRpkt = check_and_cast<const OLSR_pkt *>(_pkt);
    if (!strcmp(OLSRpkt->getName(), "OLSR_ETX pkt"))
        throw cRuntimeError("OLSR_ETX pkt serializer not implement yet");
    uint32_t rf = OLSRpkt->reduceFuncionality() << 31;
    b.writeUint16(OLSRpkt->pkt_seq_num());
    b.writeUint32(rf | OLSRpkt->msgArraySize());

    for (unsigned int i=0; i < OLSRpkt->msgArraySize(); i++ )
    {
        serializeMsg(b, c, OLSRpkt->msg(i));
    }


    //throw cRuntimeError("OLSR serializer in");
}

void OLSRSerializer::serializeMsg(Buffer &b, Context& c, const OLSR_msg& _msg)
{
    OLSR_msg tmpMsg = _msg;
    b.writeByte(tmpMsg.msg_type());
    b.writeByte(tmpMsg.vtime());
    b.writeUint16(tmpMsg.msg_size());
    ASSERT(tmpMsg.orig_addr().getType() == L3Address::IPv4);
    b.writeIPv4Address(tmpMsg.orig_addr().toIPv4());
    b.writeByte(tmpMsg.ttl());
    b.writeByte(tmpMsg.hop_count());
    b.writeUint16(tmpMsg.msg_seq_num());

    if (tmpMsg.msg_type() == OLSR_HELLO_MSG)
    {
        OLSR_hello& tmphello_msg = tmpMsg.hello();
        b.writeUint16(tmphello_msg.reserved());
        b.writeByte(tmphello_msg.htime());
        b.writeByte(tmphello_msg.willingness());
        for(auto i = 0; i < tmphello_msg.count; i++)
        {
            b.writeByte(tmphello_msg.hello_msg(i).link_code());
            b.writeByte(tmphello_msg.hello_msg(i).reserved());
            b.writeUint16(tmphello_msg.hello_msg(i).link_msg_size());
            for(auto j = 0; j < tmphello_msg.hello_msg(i).count; j++)
                b.writeIPv4Address(tmphello_msg.hello_msg(i).nb_iface_addr(j).toIPv4());
        }

    }
    else if (tmpMsg.msg_type() == OLSR_TC_MSG)
    {
        OLSR_tc& tc_msg = tmpMsg.tc();
        b.writeUint16(tc_msg.ansn());
        b.writeUint16(tc_msg.reserved());
        //qos_behaviour not include >> is for OLSR_ETX packet
        for(auto i = 0; i < tc_msg.count; i++) // count = (tmpMsg.msg_size()-OLSR_MSG_HDR_SIZE-OLSR_TC_HDR_SIZE)/OlsrAddressSize::ADDR_SIZE
        {
            b.writeIPv4Address(tc_msg.nb_main_addr(i).toIPv4());
        }

    }
    else if (tmpMsg.msg_type() == OLSR_MID_MSG)
    {
        OLSR_mid& mid_msg = tmpMsg.mid();
        for(auto i = 0; i < mid_msg.count; i++) // count = (tmpMsg.msg_size()-OLSR_MSG_HDR_SIZE)/mid_msg.size()
        {
            b.writeIPv4Address(mid_msg.iface_addr(i).toIPv4());
        }

    }else
    {
        throw cRuntimeError("unknown OLSR msg type!!!");
    }

}

cPacket *OLSRSerializer::deserialize(const Buffer &b, Context& c)
{
    //OLSRHEADERLENGTH = 4
    ASSERT(b.getPos() == 0);

    OLSR_pkt *pkt = new OLSR_pkt("parsed-OLSR Pkt");
    int64_t pkt_length = 0;

    pkt->setPkt_seq_num(b.readUint16());
    uint32_t rf_msgArraySize = b.readUint32();
    pkt->setReduceFuncionality(rf_msgArraySize >> 31);
    uint32_t MsgArrSize = rf_msgArraySize & ((0x01u<<31) - 1);
    pkt->setMsgArraySize(MsgArrSize);

    for(unsigned int i=0; i < MsgArrSize; i++)
    {
        deserializeMsg(b, c, pkt->msg(i));
        pkt_length += pkt->msg(i).size();
    }

    pkt->setByteLength((OLSR_PKT_HDR_SIZE) + 2 + pkt_length);//calculate packetlength
    return pkt;
}

void OLSRSerializer::deserializeMsg(const Buffer &b, Context& c, OLSR_msg& _msg)
{
    _msg.msg_type() = b.readByte();
    _msg.vtime() = b.readByte();
    _msg.msg_size() = b.readUint16();
    _msg.orig_addr().set(b.readIPv4Address());
    _msg.ttl() = b.readByte();
    _msg.hop_count() = b.readByte();
    _msg.msg_seq_num() = b.readUint16();

    if (_msg.msg_type() == OLSR_HELLO_MSG)
    {
        OLSR_hello& tmphello_msg = _msg.hello();
        tmphello_msg.reserved() = b.readUint16();
        tmphello_msg.htime() = b.readByte();
        tmphello_msg.willingness() = b.readByte();
        uint32_t msg_body_bytes = _msg.msg_size()-(OLSR_MSG_HDR_SIZE)-(OLSR_HELLO_HDR_SIZE);
        int count = 0;
        for(auto i=msg_body_bytes; i>0;)
        {
            tmphello_msg.hello_msg(count).link_code() = b.readByte();
            tmphello_msg.hello_msg(count).reserved() = b.readByte();
            tmphello_msg.hello_msg(count).link_msg_size() = b.readUint16();
            tmphello_msg.hello_msg(count).count = (tmphello_msg.hello_msg(count).link_msg_size() - OLSR_HELLO_MSG_HDR_SIZE)
                                                    /OlsrAddressSize::ADDR_SIZE;
            for(auto j = 0; j < tmphello_msg.hello_msg(count).count; j++)
                tmphello_msg.hello_msg(count).nb_iface_addr(j).set(b.readIPv4Address());
            i -= tmphello_msg.hello_msg(count).size();
            count++;
        }
        tmphello_msg.count = count;

    }
    else if (_msg.msg_type() == OLSR_TC_MSG)
    {
        OLSR_tc& tc_msg = _msg.tc();
        tc_msg.ansn() = b.readUint16();
        tc_msg.reserved() = b.readUint16();
        tc_msg.count = (_msg.msg_size()-(OLSR_MSG_HDR_SIZE)-(OLSR_TC_HDR_SIZE))/OlsrAddressSize::ADDR_SIZE;
        for(auto i = 0; i < tc_msg.count; i++)
        {
            tc_msg.nb_main_addr(i).set(b.readIPv4Address());
        }
    }
    else if (_msg.msg_type() == OLSR_MID_MSG)
    {
        OLSR_mid& mid_msg = _msg.mid();
        mid_msg.count = (_msg.msg_size()-(OLSR_MSG_HDR_SIZE))/OlsrAddressSize::ADDR_SIZE;
        for(auto i = 0; i < mid_msg.count; i++)
        {
            mid_msg.iface_addr(i).set(b.readIPv4Address());
        }

    }else
    {
        throw cRuntimeError("deserialize: unknown OLSR msg type!!!");
    }
}

} // namespace serializer

} // namespace inet

