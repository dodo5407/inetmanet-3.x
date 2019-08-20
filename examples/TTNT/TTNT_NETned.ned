//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License
// along with this program.  If not, see http://www.gnu.org/licenses/.
// 

package inet.examples.TTNT;

// numOfHosts: 5
import inet.networklayer.configurator.ipv4.IPv4NetworkConfigurator;
import inet.node.inet.INetworkNode;
//import inet.physicallayer.ieee80211.packetlevel.Ieee80211ScalarRadioMedium;
import inet.physicallayer.contract.packetlevel.IRadioMedium;
import inet.visualizer.integrated.IntegratedVisualizer;


network TTNT_net
{
    parameters:
        int numHosts;
        int numFixHosts;
        string radioMediumType;
        string hosttype = default("AdhocHost");
    submodules:
        fixhost[numFixHosts]: <hosttype> like INetworkNode {
            parameters:
                @display("i=device/plane;r=,,#707070");
        }
        host[numHosts]: <hosttype> like INetworkNode {
            parameters:
                @display("i=device/plane;r=,,#707070");
        }
        radioMedium: <radioMediumType> like IRadioMedium {
            parameters:
                @display("p=60,50;i=misc/sun");
        }
        configurator: IPv4NetworkConfigurator {
            parameters:
                config = xml("<config><interface hosts='*' address='192.168.x.x' netmask='255.255.0.0'/></config>");
                @display("p=140,50;i=block/cogwheel_s");
        }
        visualizer: IntegratedVisualizer {
            parameters:
                @display("p=100,50");
        }
    connections allowunconnected:
}
