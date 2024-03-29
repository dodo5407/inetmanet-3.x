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

#include "inet/common/RawPacket.h"
#include "inet/common/serializer/SerializerBase.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtAdhocforFreqHop.h"
#include "inet/linklayer/common/Ieee802Ctrl.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"

namespace inet {

namespace ieee80211 {

Define_Module(Ieee80211MgmtAdhocforFreqHop);

simsignal_t Ieee80211MgmtAdhocforFreqHop::macTrasmissionFinishedSignal = cComponent::registerSignal("macTrasmissionFinished");

void Ieee80211MgmtAdhocforFreqHop::initialize(int stage)
{
    Ieee80211MgmtBase::initialize(stage);

    if (stage == INITSTAGE_LOCAL) {
        maxMultiOutChannel = par("maxMultiOutChannel");
        fragmentTimeoutTime = par("fragmentTimeoutTime");
        endSIFS = new cMessage("SIFS");
        numFrameInMac = 0;
        numMac = gateSize("macOut");
        numFragDataFrameReceived = 0;
        numNoFragDataFrameReceived = 0;
        numFragDataFrameSend = 0;
        numNoFragDataFrameSend = 0;
        WATCH(numFrameInMac);
        WATCH(numFragDataFrameReceived);
        WATCH(numNoFragDataFrameReceived);
        WATCH(numFragDataFrameSend);
        WATCH(numNoFragDataFrameSend);
        waitToSendQueueVector.setName("waitToSendQueueSize");
        numFrameInMacVector.setName("numFrameInMac");
    }


    if (stage == INITSTAGE_LINK_LAYER)
        channelBusyState = new bool[numMac];

    if ((numMac > 1)&&(stage == INITSTAGE_LINK_LAYER_2)) {
        /*const char *bandName = getModuleByPath("^.radio[0]")->par("bandName");
        int numChannels = Ieee80211CompliantBands::getBand(bandName)->getNumChannels();

        float diff = (float)numChannels / (float)numMac;
        int space = (diff-(int)diff)>0.4 ? (int)diff+1 : (int)diff ;
        for(int index=0; index<numMac; index++){
            changeChannel(space*index, index);
        }
        */
        /*GHz freqencyBase(par("carrierFrequencyBase"));

        Hz space(getModuleByPath("^.radio[0]")->par("bandwidth"));
        space += MHz(5);
        for(int index=0; index<numMac; index++){
            changeFrequency(freqencyBase + space*index, index);
        }*/

        for(int gateindex=0; gateindex<numMac; gateindex++) {
            std::string modulename = std::string("^.radio[") + std::to_string(gateindex) + "]";
            radioModule = check_and_cast<Radio *>(getModuleByPath(modulename.c_str()));
            radioModule->subscribe(macTrasmissionFinishedSignal,this);
        }

    }
}

void Ieee80211MgmtAdhocforFreqHop::finish()
{
    recordScalar("#dataFrameReceived", numDataFramesReceived);
    recordScalar("#dataFragDataFrameReceived", numFragDataFrameReceived);
    recordScalar("#dataNoFragDataFrameReceived", numNoFragDataFrameReceived);
    recordScalar("#dataFragDataFrameSent", numFragDataFrameSend);
    recordScalar("#dataNoFragDataFrameSent", numNoFragDataFrameSend);

    waitToSendQueueStats.recordAs("waitToSend queue size");
}

void Ieee80211MgmtAdhocforFreqHop::handleMessage(cMessage *msg)
{
    if (!isOperational)
        throw cRuntimeError("Message '%s' received when module is OFF", msg->getName());

    if (msg->isSelfMessage()) {
        if (msg == endSIFS) {
            if(waitToSendQueue.empty()) {
                EV_WARN << " Ready send down to free channel, but sendqueue is empty!.\n";
                cancelEvent(endSIFS);
                return ;
            }

            int gateindex = checkFreeChannel();
            std::string modulename = std::string("^.mac[") + std::to_string(gateindex) + "]";
            PacketFragPair *dataframePair = waitToSendQueue.front();
            std::string numFragString = (dataframePair->second == -1) ? std::string(".\n") : std::string("for no.")+std::to_string(dataframePair->second)+" fragment.\n";
            if (gateindex >= 0) {
                numFrameInMac++;
                numFrameInMacVector.record(numFrameInMac);
                EV_DETAIL << "The Channel of " << modulename.substr(2,6) << " is free "
                                        << numFragString;
                send(dataframePair->first, "macOut", gateindex);
                waitToSendQueue.pop_front();
                waitToSendQueueVector.record(waitToSendQueue.size());
                waitToSendQueueStats.collect(waitToSendQueue.size());
            }else if(gateindex == -1){
                EV_DETAIL << "All Channel is busy "
                                        << numFragString;
            }else {
                /*EV_DETAIL << "Number of frames in mac over the max multi-out channels "
                                        << numFragString;*/
            }

            if(!waitToSendQueue.empty() && !endSIFS->isScheduled()){
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

void Ieee80211MgmtAdhocforFreqHop::receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details)
{
    Enter_Method_Silent();
    if (signalID == macTrasmissionFinishedSignal) {
        numFrameInMac--;
        numFrameInMacVector.record(numFrameInMac);
        if(waitToSendQueue.empty()) {
            EV_WARN << " Ready send down to free channel, but sendqueue is empty!.\n";
            cancelEvent(endSIFS);
            return ;
        }

        int gateindex = checkFreeChannel();
        std::string modulename = std::string("^.mac[") + std::to_string(gateindex) + "]";
        PacketFragPair *dataframePair = waitToSendQueue.front();
        std::string numFragString = (dataframePair->second == -1) ? std::string(".\n") : std::string("for no.")+std::to_string(dataframePair->second)+" fragment.\n";
        if (gateindex >= 0) {
            numFrameInMac++;
            numFrameInMacVector.record(numFrameInMac);
            EV_DETAIL << "The Channel of " << modulename.substr(2,6) << " is free "
                        << numFragString;
            send(dataframePair->first, "macOut", gateindex);
            waitToSendQueue.pop_front();
            waitToSendQueueVector.record(waitToSendQueue.size());
            waitToSendQueueStats.collect(waitToSendQueue.size());
        }else if(gateindex == -1){
            EV_DETAIL << "All Channel is busy "
                        << numFragString;
        }else {
            /*EV_DETAIL << "Number of frames in mac over the max multi-out channels "
                        << numFragString;*/
        }
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

    if (radioModule != nullptr) {
        radioModule->unsubscribe(IRadioMedium::transmissionEndedSignal, this);
    }

    while(!waitToSendQueue.empty()) {
        PacketFragPair *temp = dynamic_cast<PacketFragPair *>(waitToSendQueue.front());
        waitToSendQueue.pop_front();
        delete temp->first;
        delete temp;
    }

    delete[] channelBusyState;
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

void Ieee80211MgmtAdhocforFreqHop::changeFrequency(Hz carrierFrequency, int gateindex)
{
    EV << "Tuning to frequency #" << carrierFrequency << "\n";

    Ieee80211ConfigureRadioCommand *configureCommand = new Ieee80211ConfigureRadioCommand();
    configureCommand->setCarrierFrequency(carrierFrequency);
    cMessage *msg = new cMessage("changeFrequency", RADIO_C_CONFIGURE);
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
    frame->setIdentification(frameId++);
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

    if(dataframe->getEncapsulatedPacket())
        dataframe->setTotalPayloadLength(dataframe->getEncapsulatedPacket()->getByteLength());

    // check if datagram does not require fragmentation
    if (frameMTU == 0 || dataframe->getByteLength() <= frameMTU)
    {
        numNoFragDataFrameSend++;
        sendFromFreeChannel(dataframe);
        return;
    }

    cPacket *payload = dataframe->decapsulate();
    int headerLength = dataframe->getByteLength();
    dataframe->setHeaderLength(headerLength);
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
            fragment->setMoreFragment(true);

        fragment->setByteLength(headerLength);
        fragment->encapsulate(payloadFrag);

        fragment->setFragmentOffset(offsetBase + offset);

        sendFromFreeChannel(fragment, fragnum++);//send(msg, "macOut", gateindex);
    }

    numFragDataFrameSend++;

    delete payload;
    delete dataframe;

}

void Ieee80211MgmtAdhocforFreqHop::reassembleAndDeliver(Ieee80211DataFrameWithSNAP *dataframe)
{
    EV_INFO << "Reassemble frame.\n";

    if(dataframe->getReceiverAddress().isUnspecified())
        EV_WARN << "Received datagram '%s' without source address filled in" << dataframe->getName() << "\n";

    // reassemble the packet (if fragmented)
    if(dataframe->getFragmentOffset()!=0 || dataframe->getMoreFragment())
    {
        EV_DETAIL << "Datagram fragment: offset=" << dataframe->getFragmentOffset()
                << ", More=" << (dataframe->getMoreFragment() ? "true" : "false") << ".\n";

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
        numFragDataFrameReceived++;
        EV_DETAIL << "This fragment completes the datagram.\n";
    }else {
        dataframe->setFragmentOffset(0);
        dataframe->setMoreFragment(false);
        if (dynamic_cast<RawPacket *>(dataframe->getEncapsulatedPacket())) {
            using namespace serializer;
            RawPacket *rp = static_cast<RawPacket *>(dataframe->getEncapsulatedPacket());
            char ipv4addresses[8];    // 2 * 4 bytes for 2 IPv4 addresses
            Buffer b(rp->getByteArray().getDataPtr(), rp->getByteArray().getDataArraySize());
            Context c;
            c.l3AddressesPtr = ipv4addresses;
            c.l3AddressesLength = sizeof(ipv4addresses);
            cPacket *enc = SerializerBase::lookupAndDeserialize(b, c, ETHERTYPE, dataframe->getEtherType());
            if (enc) {
                delete dataframe->decapsulate();
                dataframe->encapsulate(enc);
            }
        }
        numNoFragDataFrameReceived++;
        EV_DETAIL << "This datagram is not fragment. Send up!\n";
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
    if(gateindex < 0)
    {
        PacketFragPair *dataframePair = new PacketFragPair(dataframe, -1);
        waitToSendQueue.push_back(dataframePair);
        waitToSendQueueVector.record(waitToSendQueue.size());
        waitToSendQueueStats.collect(waitToSendQueue.size());
        if (gateindex == -1)
        {
            EV_DETAIL << "All Channel is busy.\n";
        }else
        {
            //EV_DETAIL << "Number of frames in mac over the max multi-out channels.\n";
        }
        if(!endSIFS->isScheduled())
            scheduleAt(simTime() + macModule->getSIFS(), endSIFS);
    }else {
        numFrameInMac++;
        numFrameInMacVector.record(numFrameInMac);
        EV_DETAIL << "The Channel of " << modulename.substr(2,6) << " is free.\n";
        send(dataframe, "macOut", gateindex);
    }
}

void Ieee80211MgmtAdhocforFreqHop::sendFromFreeChannel(cPacket *dataframe, int fragnum)
{
    int gateindex = checkFreeChannel();
    std::string modulename = std::string("^.mac[") + std::to_string(gateindex) + "]";
    if(gateindex < 0)
    {
        PacketFragPair *dataframePair = new PacketFragPair(dataframe, fragnum);
        waitToSendQueue.push_back(dataframePair);
        waitToSendQueueVector.record(waitToSendQueue.size());
        waitToSendQueueStats.collect(waitToSendQueue.size());
        if (gateindex == -1)
        {
            EV_DETAIL << "All Channel is busy "
                    << "for no." << fragnum << " fragment.\n";
        }else
        {
            /*EV_DETAIL << "Number of frames in mac over the max multi-out channels"
                                << "for no." << fragnum << " fragment.\n";*/
        }
        if(!endSIFS->isScheduled())
            scheduleAt(simTime() + macModule->getSIFS(), endSIFS);
    }else {
        numFrameInMac++;
        numFrameInMacVector.record(numFrameInMac);
        EV_DETAIL << "The Channel of " << modulename.substr(2,6) << " is free "
                        << "for " << fragnum << " fragment.\n";
        send(dataframe, "macOut", gateindex);
    }
}

int Ieee80211MgmtAdhocforFreqHop::checkFreeChannel(void)
{
    bool isChannelFree = false;
    int gateindex = -1;
    if (lastCheckChannelFreeTime != simTime()) //avoid send many frames to one channel at the same time
    {
        for (int i = 0; i<numMac; i++)  channelBusyState[i] = false ;
    }
    lastCheckChannelFreeTime = simTime();

    while(!isChannelFree)
    {
        bool allChannelBusy = true;
        for (int i=0; i<numMac; i++) allChannelBusy = allChannelBusy && channelBusyState[i];
        if(allChannelBusy)
            return -1;
        else if(numFrameInMac >= maxMultiOutChannel)
            return -2;
        else
        {
            gateindex=(int)intrand(numMac);
            if (channelBusyState[gateindex] == true)
                continue;

            std::string modulename = std::string("^.mac[") + std::to_string(gateindex) + "]";
            macModule = check_and_cast<Ieee80211OldMac2 *>(getModuleByPath(modulename.c_str()));
            isChannelFree = macModule->isMediumFree();
            channelBusyState[gateindex] = !isChannelFree;
        }

    }
    channelBusyState[gateindex] = true;

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

