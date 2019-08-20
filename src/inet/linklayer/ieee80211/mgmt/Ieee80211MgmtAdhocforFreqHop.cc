//
// Copyright (C) 2006 Andras Varga
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

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAdhocforFreqHop.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/networklayer/ipv4/IPv4Datagram.h"

namespace inet {

namespace ieee80211 {

using namespace physicallayer;

Define_Module(Ieee80211MgmtAdhocforFreqHop);

void Ieee80211MgmtAdhocforFreqHop::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        fragmentTimeoutTime = par("fragmentTimeoutTime");
        endSIFS = new cMessage("SIFS");
    }

    numMac = gateSize("macOut");
    if ((numMac > 1)&&(stage == INITSTAGE_LINK_LAYER_2)) {
        const char *bandName = getModuleByPath("^.radio[0]")->par("bandName");
        int numChannels = Ieee80211CompliantBands::getBand(bandName)->getNumChannels();

        float diff = (float)numChannels / (float)numMac;
        int space = (diff-(int)diff)>0.4 ? (int)diff+1 : (int)diff ;
        for(int index=0; index<numMac; index++){
            changeChannel(space*index, index);
        }
    }
}

void Ieee80211MgmtAdhocforFreqHop::handleMessage(cMessage *msg)
{
    if (!isOperational)
        throw cRuntimeError("Message '%s' received when module is OFF", msg->getName());

    if (msg->isSelfMessage()) {
        if (msg == endSIFS) {
            if(sendToChannelQueue.empty()) {
                EV_WARN << " Ready send down to free channel, but sendqueue is empty!.\n";
                cancelEvent(endSIFS);
                return ;
            }

            int gateindex = checkFreeChannel();
            std::string modulename = std::string("^.mac[") + std::to_string(gateindex) + "]";
            PacketFragPair *dataframePair = sendToChannelQueue.front();
            std::string numFragString = (dataframePair->second == -1) ? std::string(".\n") : std::string("for ")+std::to_string(dataframePair->second)+" fragment.\n";
            if (gateindex != -1) {
                EV_DETAIL << "The Channel of " << modulename.substr(2,6) << " is free "
                                        << numFragString;
                send(dataframePair->first, "macOut", gateindex);
                sendToChannelQueue.pop_front();
            }else {
                EV_DETAIL << "All Channel is busy "
                                        << numFragString;
            }

            if(!sendToChannelQueue.empty() && !endSIFS->isScheduled()){
                scheduleAt(simTime()+macModule->getSIFS(),endSIFS);
            }
        }
        else {
        // process timers
        EV << "Timer expired: " << msg << "\n";
        handleTimer(msg);
        }
    }
    else if (msg->arrivedOn("macIn")) {
        // process incoming frame
        EV << "Frame arrived from MAC: " << msg << "\n";
        Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *>(msg);
        processFrame(frame);
    }
    else if (msg->arrivedOn("agentIn")) {
        // process command from agent
        EV << "Command arrived from agent: " << msg << "\n";
        int msgkind = msg->getKind();
        cObject *ctrl = msg->removeControlInfo();
        delete msg;

        handleCommand(msgkind, ctrl);
    }
    else {
        // packet from upper layers, to be sent out
        cPacket *pk = PK(msg);
        EV << "Packet arrived from upper layers: " << pk << "\n";
        handleUpperMessage(pk);
    }
}

Ieee80211MgmtAdhocforFreqHop::Ieee80211MgmtAdhocforFreqHop()
{
}

Ieee80211MgmtAdhocforFreqHop::~Ieee80211MgmtAdhocforFreqHop()
{
    if(endSIFS){
        if(endSIFS->getContextPointer())
            delete (cMessage *)endSIFS->getContextPointer();
        endSIFS->setContextPointer(nullptr);
        cancelAndDelete(endSIFS);
    }

    while(!sendToChannelQueue.empty()) {
        PacketFragPair *temp = dynamic_cast<PacketFragPair *>(sendToChannelQueue.front());
        sendToChannelQueue.pop_front();
        delete temp->first;
        delete temp;
    }
}

void Ieee80211MgmtAdhocforFreqHop::changeChannel(int channelNum, int gateindex)
{
    EV << "Tuning to channel #" << channelNum << "\n";

    Ieee80211ConfigureRadioCommand *configureCommand = new Ieee80211ConfigureRadioCommand();
    configureCommand->setChannelNumber(channelNum);
    cMessage *msg = new cMessage("changeChannel", RADIO_C_CONFIGURE);
    msg->setControlInfo(configureCommand);
    send(msg, "macOut", gateindex);
}

void Ieee80211MgmtAdhocforFreqHop::handleTimer(cMessage *msg)
{
    ASSERT(false);
}

void Ieee80211MgmtAdhocforFreqHop::handleUpperMessage(cPacket *msg)
{
    Ieee80211DataFrameWithSNAP *frame = encapsulate(msg);
    fragmentAndSend(frame);
}

void Ieee80211MgmtAdhocforFreqHop::handleCommand(int msgkind, cObject *ctrl)
{
    throw cRuntimeError("handleCommand(): no commands supported");
}

Ieee80211DataFrameWithSNAP *Ieee80211MgmtAdhocforFreqHop::encapsulate(cPacket *msg)
{
    Ieee80211DataFrameWithSNAP *frame = new Ieee80211DataFrameWithSNAP(msg->getName());

    // copy receiver address from the control info (sender address will be set in MAC)
    Ieee802Ctrl *ctrl = check_and_cast<Ieee802Ctrl *>(msg->removeControlInfo());
    frame->setReceiverAddress(ctrl->getDest());
    frame->setEtherType(ctrl->getEtherType());
    if (dynamic_cast<IPv4Datagram *>(msg))
        frame->setIdentification((dynamic_cast<IPv4Datagram *>(msg))-> getIdentification());
    else
        frame->setIdentification((dynamic_cast<cMessage *>(msg))->getCreationTime().raw());
    int up = ctrl->getUserPriority();
    if (up >= 0) {
        // make it a QoS frame, and set TID
        frame->setType(ST_DATA_WITH_QOS);
        frame->addBitLength(QOSCONTROL_BITS);
        frame->setTid(up);
    }
    delete ctrl;

    frame->encapsulate(msg);
    return frame;
}

cPacket *Ieee80211MgmtAdhocforFreqHop::decapsulate(Ieee80211DataFrame *frame)
{
    cPacket *payload = frame->decapsulate();

    Ieee802Ctrl *ctrl = new Ieee802Ctrl();
    ctrl->setSrc(frame->getTransmitterAddress());
    ctrl->setDest(frame->getReceiverAddress());
    int tid = frame->getTid();
    if (tid < 8)
        ctrl->setUserPriority(tid); // TID values 0..7 are UP
    Ieee80211DataFrameWithSNAP *frameWithSNAP = dynamic_cast<Ieee80211DataFrameWithSNAP *>(frame);
    if (frameWithSNAP)
        ctrl->setEtherType(frameWithSNAP->getEtherType());
    payload->setControlInfo(ctrl);

    delete frame;
    return payload;
}

void Ieee80211MgmtAdhocforFreqHop::fragmentAndSend(Ieee80211DataFrameWithSNAP *dataframe)
{
    int frameMTU = par("frameMTU");

    // check if datagram does not require fragmentation
    if (frameMTU == 0 || dataframe->getByteLength() <= frameMTU)
    {
        sendFromFreeChannel(dataframe);
        return;
    }

    if(dataframe->getEncapsulatedPacket())
        dataframe->setTotalPayloadLength(dataframe->getEncapsulatedPacket()->getByteLength());

    cPacket *payload = dataframe->decapsulate();
    int headerLength = dataframe->getByteLength();
    int payloadLength = payload->getByteLength();

    int fragmentLength = ((frameMTU - headerLength) / 8) * 8;// payload only (without header)
    int offsetBase = dataframe->getFragmentOffset();
    if (fragmentLength <= 0)
        throw cRuntimeError("Cannot fragment datagram: MTU=%d too small for header size (%d bytes)", frameMTU, headerLength); // exception and not ICMP because this is likely a simulation configuration error, not something one wants to simulate

    int noOfFragments = (payloadLength + fragmentLength - 1) / fragmentLength;
    EV << "Breaking datagram into " << noOfFragments << " fragments\n";

    // create and send fragments
    EV_DETAIL << "Breaking datagram into " << noOfFragments << " fragments\n";
    std::string fragMsgName = dataframe->getName();
    fragMsgName += "-macfrag";

    int fragnum = 0;
    for(int offset=0; offset < payloadLength; offset+=fragmentLength)
    {
        bool lastFragment = (offset+fragmentLength >= payloadLength);
        // length equal to fragmentLength, except for last fragment;
        int thisFragmentLength = lastFragment ? payloadLength - offset : fragmentLength;
        cPacket *payloadFrag = payload->dup();
        payloadFrag->setByteLength(thisFragmentLength);

        Ieee80211DataFrameWithSNAP *fragment = (Ieee80211DataFrameWithSNAP *)dataframe->dup();
        fragment->setName(fragMsgName.c_str());

        // "more fragments" bit is unchanged in the last fragment, otherwise true
        if(!lastFragment)
            fragment->setMoreFragments(true);

        fragment->setByteLength(headerLength);
        fragment->encapsulate(payloadFrag);

        fragment->setFragmentOffset(offsetBase + offset);

        sendFromFreeChannel(fragment, fragnum++);//send(msg, "macOut", gateindex);
    }

    delete payload;
    delete dataframe;

}

void Ieee80211MgmtAdhocforFreqHop::reassembleAndDeliver(Ieee80211DataFrameWithSNAP *dataframe)
{
    EV_INFO << "Reassemble frame.\n";

    if(dataframe->getReceiverAddress().isUnspecified())
        EV_WARN << "Received datagram '%s' without source address filled in" << dataframe->getName() << "\n";

    // reassemble the packet (if fragmented)
    if(dataframe->getFragmentOffset()!=0 || dataframe->getMoreFragments())
    {
        EV_DETAIL << "Datagram fragment: offset=" << dataframe->getFragmentOffset()
                << ", More=" << (dataframe->getMoreFragments() ? "true" : "false") << ".\n";

        // erase timed out fragments in fragmentation buffer; check every 10 seconds max
        if(simTime() >= lastCheckTime + 10)
        {
            lastCheckTime = simTime();
            fragbuf.purgeStaleFragments(simTime()-fragmentTimeoutTime);
        }

        if((dataframe->getTotalPayloadLength()>0) && (dataframe->getTotalPayloadLength() != dataframe->getEncapsulatedPacket()->getByteLength()))
        {
            int totalLength = dataframe->getByteLength();
            cPacket *payload = dataframe->decapsulate();
            dataframe->setHeaderLength(dataframe->getByteLength());
            payload->setByteLength(dataframe->getTotalPayloadLength());
            dataframe->encapsulate(payload);
            dataframe->setByteLength(totalLength);
        }

        dataframe = fragbuf.addFragment(dataframe, simTime());
        if(!dataframe)
        {
            EV << "No complete datagram yet.\n";
            return;
        }
        EV_DETAIL << "This fragment completes the datagram.\n";
    }

    reassembleAndDeliverFinish(check_and_cast<Ieee80211DataFrame *>(dataframe));
}

void Ieee80211MgmtAdhocforFreqHop::reassembleAndDeliverFinish(Ieee80211DataFrame *frame)
{
    sendUp(decapsulate(frame));
}

void Ieee80211MgmtAdhocforFreqHop::sendFromFreeChannel(cPacket *dataframe)
{
    int gateindex = checkFreeChannel();
    std::string modulename = std::string("^.mac[") + std::to_string(gateindex) + "]";
    if(gateindex == -1)
    {
        PacketFragPair *dataframePair = new PacketFragPair(dataframe, -1);
        sendToChannelQueue.push_back(dataframePair);
        if(!endSIFS->isScheduled())
            scheduleAt(simTime() + macModule->getSIFS(), endSIFS);
    }else {
        EV_DETAIL << "The Channel of " << modulename.substr(2,6) << " is free.\n";
        send(dataframe, "macOut", gateindex);
    }
}

void Ieee80211MgmtAdhocforFreqHop::sendFromFreeChannel(cPacket *dataframe, int fragnum)
{
    int gateindex = checkFreeChannel();
    std::string modulename = std::string("^.mac[") + std::to_string(gateindex) + "]";
    if(gateindex == -1)
    {
        PacketFragPair *dataframePair = new PacketFragPair(dataframe, fragnum);
        sendToChannelQueue.push_back(dataframePair);
        EV_DETAIL << "All Channel is busy "
                                << "for " << fragnum << " fragment.\n";
        if(!endSIFS->isScheduled())
            scheduleAt(simTime() + macModule->getSIFS(), endSIFS);
    }else {
        EV_DETAIL << "The Channel of " << modulename.substr(2,6) << " is free "
                        << "for " << fragnum << " fragment.\n";
        send(dataframe, "macOut", gateindex);
    }
}

int Ieee80211MgmtAdhocforFreqHop::checkFreeChannel(void)
{
    bool isChannelFree = false;
    bool channelBusyState[numMac] = {false};
    int gateindex = -1;
    while(!isChannelFree)
    {
        gateindex=(int)intrand(numMac);
        std::string modulename = std::string("^.mac[") + std::to_string(gateindex) + "]";
        macModule = check_and_cast<Ieee80211OldMac2 *>(getModuleByPath(modulename.c_str()));
        isChannelFree = macModule->isMediumFree();
        channelBusyState[gateindex] = isChannelFree ? false : true;

        bool allChannelBusy = true;
        for(int i=0; i<numMac; i++) allChannelBusy = allChannelBusy && channelBusyState[i];
        if(allChannelBusy)
            return -1;
    }

    return gateindex;
}

void Ieee80211MgmtAdhocforFreqHop::handleDataFrame(Ieee80211DataFrame *frame)
{
    reassembleAndDeliver(check_and_cast<Ieee80211DataFrameWithSNAP *>(frame));
}

void Ieee80211MgmtAdhocforFreqHop::handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocforFreqHop::handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocforFreqHop::handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocforFreqHop::handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocforFreqHop::handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocforFreqHop::handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocforFreqHop::handleDisassociationFrame(Ieee80211DisassociationFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocforFreqHop::handleBeaconFrame(Ieee80211BeaconFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocforFreqHop::handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame)
{
    dropManagementFrame(frame);
}

void Ieee80211MgmtAdhocforFreqHop::handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame)
{
    dropManagementFrame(frame);
}

} // namespace ieee80211

} // namespace inet

