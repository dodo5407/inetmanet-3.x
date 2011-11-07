//
// Copyright (C) 2005 Andras Varga
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
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//


package inet.examples.wireless.synchronized;

import inet.nodes.inet.AdhocHost;
import inet.world.radio.ChannelControl;


network Synchronized
{
    parameters:
        int nodeCount;
        **.bitrate = 11Mbps;
        **.globalARP = true;
    submodules:
        channelControl: ChannelControl;
        node[nodeCount]: AdhocHost {
            parameters:
                numUdpApps = 1;
                udpApp[0].typename = "UDPBasicApp";
                udpApp[0].destAddresses = "224.0.0.1";
                udpApp[0].localPort = 1;
                udpApp[0].destPort = 1;
                udpApp[0].messageLength = 100B;
                routingFile = "node" + string(index) + ".mrt";
        }
}