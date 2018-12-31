//
// Copyright (C) 2006 Andras Varga and Levente Meszaros
// Copyright (C) 2009 Lukáš Hlůže   lukas@hluze.cz (802.11e)
// Copyright (C) 2011 Alfonso Ariza  (clean code, fix some errors, new radio model)
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

#include "Ieee80211OldMac2.h"

#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/networklayer/contract/IInterfaceTable.h"
#include "inet/physicallayer/contract/packetlevel/RadioControlInfo_m.h"
#include "inet/physicallayer/ieee80211/packetlevel/Ieee80211ControlInfo_m.h"
#include "inet/common/INETUtils.h"
#include "inet/common/ModuleAccess.h"

namespace inet {

namespace ieee80211 {

// TODO: 9.3.2.1, If there are buffered multicast or broadcast frames, the PC shall transmit these prior to any unicast frames.
// TODO: control frames must send before

Define_Module(Ieee80211OldMac2);

// don't forget to keep synchronized the C++ enum and the runtime enum definition
Register_Enum(inet::ieee80211::Ieee80211OldMac2,
        (Ieee80211OldMac2::IDLE,
         Ieee80211OldMac2::DEFER,
         Ieee80211OldMac2::WAITAIFS,
         Ieee80211OldMac2::BACKOFF,
         Ieee80211OldMac2::WAITACK,
         Ieee80211OldMac2::WAITMULTICAST,
         Ieee80211OldMac2::WAITCTS,
         Ieee80211OldMac2::WAITSIFS,
         Ieee80211OldMac2::RECEIVE,
         Ieee80211OldMac2::TRANSMIT));

/****************************************************************
 * Construction functions.
 */

void Ieee80211OldMac2::initializeCategories()
{
    int numQueues = 1;

    for (int i = 0; i < numQueues; i++)
    {
            Edca catEdca;
            catEdca.backoff = false;
            catEdca.backoffPeriod = -1;
            catEdca.retryCounter = 0;
            edcCAF.push_back(catEdca);
    }









    // statistics
    for (int i=0; i<numCategories(); i++)
    {
        numRetry(i) = 0;
        numSentWithoutRetry(i) = 0;
        numGivenUp(i) = 0;
        numSent(i) = 0;
        bits(i) = 0;
        maxJitter(i) = SIMTIME_ZERO;
        minJitter(i) = SIMTIME_ZERO;
    }
    for (int i=0; i<numCategories(); i++)
    {
        EdcaOutVector outVectors;
        std::stringstream os;
        os<< i;
        std::string th = "throughput AC"+os.str();
        std::string delay = "Mac delay AC"+os.str();
        std::string jit = "jitter AC"+os.str();
        outVectors.jitter = new cOutVector(jit.c_str());
        outVectors.throughput = new cOutVector(th.c_str());
        outVectors.macDelay = new cOutVector(delay.c_str());
        edcCAFOutVector.push_back(outVectors);
    }

}

Ieee80211OldMac2::Ieee80211OldMac2()
{
}

Ieee80211OldMac2::~Ieee80211OldMac2()
{
    if (endSIFS) {
        delete (Ieee80211Frame *)endSIFS->getContextPointer();
        endSIFS->setContextPointer(nullptr);
        cancelAndDelete(endSIFS);
    }
    cancelAndDelete(endTimeout);
    cancelAndDelete(endReserve);
    cancelAndDelete(mediumStateChange);
    for (unsigned int i = 0; i < edcCAF.size(); i++) {
        while (!transmissionQueue(i)->empty()) {
            Ieee80211Frame *temp = dynamic_cast<Ieee80211Frame *>(transmissionQueue(i)->front());
            transmissionQueue(i)->pop_front();
            delete temp;
        }
    }
    edcCAF.clear();
    for (auto & elem : edcCAFOutVector) {
        delete elem.jitter;
        delete elem.macDelay;
        delete elem.throughput;
    }
    edcCAFOutVector.clear();
    if (pendingRadioConfigMsg)
        delete pendingRadioConfigMsg;
}

/****************************************************************
 * Initialization functions.
 */

void Ieee80211OldMac2::initialize(int stage)
{
    EV_DEBUG << "Initializing stage " << stage << endl;

    MACProtocolBase::initialize(stage);

    //TODO: revise it: it's too big; should revise stages, too!!!
    if (stage == INITSTAGE_LOCAL)
    {
        // initialize parameters

        // initialize category
        initializeCategories();

        modeSet = Ieee80211ModeSet::getModeSet(par("opMode").stringValue());

        PHY_HEADER_LENGTH = par("phyHeaderLength");    //26us

        useModulationParameters = par("useModulationParameters");

        prioritizeMulticast = par("prioritizeMulticast");

        EV_DEBUG << "Operating mode: 802.11" << modeSet->getName();
        maxQueueSize = par("maxQueueSize");
        rtsThreshold = par("rtsThresholdBytes");

        // the variable is renamed due to a confusion in the standard
        // the name retry limit would be misleading, see the header file comment
        transmissionLimit = par("retryLimit");
        if (transmissionLimit == -1)
            transmissionLimit = 7;
        ASSERT(transmissionLimit >= 0);

        EV_DEBUG << " retryLimit=" << transmissionLimit;



        duplicateDetect = par("duplicateDetectionFilter");
        purgeOldTuples = par("purgeOldTuples");
        duplicateTimeOut = par("duplicateTimeOut");
        lastTimeDelete = 0;

        double bitrate = par("bitrate");
        if (bitrate == -1)
            dataFrameMode = modeSet->getFastestMode();
        else
            dataFrameMode = modeSet->getMode(bps(bitrate));

        double basicBitrate = par("basicBitrate");
        if (basicBitrate == -1)
            basicFrameMode = modeSet->getFastestMode();
        else
            basicFrameMode = modeSet->getMode(bps(basicBitrate));

        double controlBitRate = par("controlBitrate");
        if (controlBitRate == -1)
            controlFrameMode = modeSet->getSlowestMode();
        else
            controlFrameMode = modeSet->getMode(bps(controlBitRate));


        ST = par("slotTime");
        if (ST == -1)
            ST = dataFrameMode->getSlotTime();

        EV_DEBUG << " slotTime=" << getSlotTime() * 1e6 << "us";


        EV_DEBUG <<" basicBitrate="<<basicBitrate/1e6<<"Mb ";
        EV_DEBUG <<" bitrate="<<bitrate/1e6<<"Mb IDLE="<<IDLE<<" RECEIVE="<<RECEIVE<<endl;

        // configure AutoBit Rate
        configureAutoBitRate();
        //end auto rate code
        EV_DEBUG << " basicBitrate=" << basicBitrate / 1e6 << "Mb ";
        EV_DEBUG << " bitrate=" << bitrate / 1e6 << "Mb IDLE=" << IDLE << " RECEIVE=" << RECEIVE << endl;

        const char *addressString = par("address");
        address = isInterfaceRegistered();
        if (address.isUnspecified()) {
            if (!strcmp(addressString, "auto")) {
                // assign automatic address
                address = MACAddress::generateAutoAddress();
                // change module parameter from "auto" to concrete address
                par("address").setStringValue(address.str().c_str());
            }
            else
                address.setAddress(addressString);
        }


        // initialize self messages
        endSIFS = new cMessage("SIFS");

        endTimeout = new cMessage("Timeout");
        endReserve = new cMessage("Reserve");
        mediumStateChange = new cMessage("MediumStateChange");

        // state variables
        fsm.setName("Ieee80211Mac State Machine");
        mode = CSMA_ONLY;
        sequenceNumber = 0;

        currentAC = 0;
        oldcurrentAC = 0;
        lastReceiveFailed = false;

        nav = false;
        last = 0;

        contI = 0;
        contJ = 0;
        recvdThroughput = 0;
        _snr = 0;
        samplingCoeff = 50;

        numCollision = 0;
        numInternalCollision = 0;
        numReceived = 0;
        numSentMulticast = 0;
        numReceivedMulticast = 0;
        numBits = 0;
        numReceivedOther = 0;
        numAckSend = 0;
        successCounter = 0;
        failedCounter = 0;
        recovery = 0;
        timer = 0;
        timeStampLastMessageReceived = SIMTIME_ZERO;

        stateVector.setName("State");
        stateVector.setEnum("inet::ieee80211::Ieee80211OldMac2");

        // Code to compute the throughput over a period of time
        throughputTimePeriod = par("throughputTimePeriod");
        recBytesOverPeriod = 0;
        throughputLastPeriod = 0;
        throughputTimer = nullptr;
        if (throughputTimePeriod > 0)
            throughputTimer = new cMessage("throughput-timer");
        if (throughputTimer)
            scheduleAt(simTime() + throughputTimePeriod, throughputTimer);
        // end initialize variables throughput over a period of time
        // initialize watches
        validRecMode = false;
        cModule *radioModule = gate("lowerLayerOut")->getNextGate()->getOwnerModule();
        radioModule->subscribe(IRadio::radioModeChangedSignal, this);
        radioModule->subscribe(IRadio::receptionStateChangedSignal, this);
        radioModule->subscribe(IRadio::transmissionStateChangedSignal, this);
        radio = check_and_cast<IRadio *>(radioModule);
    }
    else if (stage == INITSTAGE_LINK_LAYER) {

        if (isOperational)
            radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
        // interface
        if (isInterfaceRegistered().isUnspecified()) //TODO do we need multi-MAC feature? if so, should they share interfaceEntry??  --Andras
            registerInterface();
        initWatches();
    }
}

void Ieee80211OldMac2::initWatches()
{
// initialize watches
     WATCH(fsm);

     WATCH(nav);
     WATCH(numBits);
     WATCH(numReceived);
     WATCH(numSentMulticast);
     WATCH(numReceivedMulticast);


     if (throughputTimer)
         WATCH(throughputLastPeriod);
    for (int i=0; i<numCategories(); i++)
        WATCH(edcCAF[i].retryCounter);

    for (int i=0; i<numCategories(); i++)
        WATCH_LIST(edcCAF[i].transmissionQueue);

    for (int i=0; i<numCategories(); i++)
        WATCH(edcCAF[i].numRetry);
    for (int i=0; i<numCategories(); i++)
        WATCH(edcCAF[i].numSentWithoutRetry);
    for (int i=0; i<numCategories(); i++)
        WATCH(edcCAF[i].numGivenUp);
    WATCH(numCollision);
    for (int i=0; i<numCategories(); i++)
        WATCH(edcCAF[i].numSent);

    for (int i=0; i<numCategories(); i++)
        WATCH(edcCAF[i].numDropped);
    for (int i=0; i<numCategories(); i++)
        WATCH_LIST(edcCAF[i].transmissionQueue);
}

void Ieee80211OldMac2::configureAutoBitRate()
{
    forceBitRate = par("forceBitRate");
    minSuccessThreshold = par("minSuccessThreshold");
    minTimerTimeout = par("minTimerTimeout");
    timerTimeout = par("timerTimeout");
    successThreshold = par("successThreshold");
    autoBitrate = par("autoBitrate");
    switch (autoBitrate) {
        case 0:
            rateControlMode = RATE_CR;
            EV_DEBUG << "MAC Transmission algorithm : Constant Rate" << endl;
            break;

        case 1:
            rateControlMode = RATE_ARF;
            EV_DEBUG << "MAC Transmission algorithm : ARF Rate" << endl;
            break;

        case 2:
            rateControlMode = RATE_AARF;
            successCoeff = par("successCoeff");
            timerCoeff = par("timerCoeff");
            maxSuccessThreshold = par("maxSuccessThreshold");
            EV_DEBUG << "MAC Transmission algorithm : AARF Rate" << endl;
            break;

        default:
            throw cRuntimeError("Invalid autoBitrate parameter: '%d'", autoBitrate);
            break;
    }
}

void Ieee80211OldMac2::finish()
{
    recordScalar("number of received packets", numReceived);
    recordScalar("number of collisions", numCollision);
    recordScalar("number of internal collisions", numInternalCollision);
    for (int i = 0; i < numCategories(); i++) {
        std::stringstream os;
        os << i;
        std::string th = "number of retry for AC " + os.str();
        recordScalar(th.c_str(), numRetry(i));
    }
    recordScalar("sent and received bits", numBits);
    for (int i = 0; i < numCategories(); i++) {
        std::stringstream os;
        os << i;
        std::string th = "sent packet within AC " + os.str();
        recordScalar(th.c_str(), numSent(i));
    }

    for (int i = 0; i < numCategories(); i++) {
        std::stringstream os;
        os << i;
        std::string th = "sentWithoutRetry AC " + os.str();
        recordScalar(th.c_str(), numSentWithoutRetry(i));
    }
    for (int i = 0; i < numCategories(); i++) {
        std::stringstream os;
        os << i;
        std::string th = "numGivenUp AC " + os.str();
        recordScalar(th.c_str(), numGivenUp(i));
    }
    for (int i = 0; i < numCategories(); i++) {
        std::stringstream os;
        os << i;
        std::string th = "numDropped AC " + os.str();
        recordScalar(th.c_str(), numDropped(i));
    }
}

InterfaceEntry *Ieee80211OldMac2::createInterfaceEntry()
{
    InterfaceEntry *e = new InterfaceEntry(this);

    // address
    e->setMACAddress(address);
    e->setInterfaceToken(address.formInterfaceIdentifier());

    e->setMtu(par("mtu").longValue());

    // capabilities
    e->setBroadcast(true);
    e->setMulticast(true);
    e->setPointToPoint(false);

    return e;
}

/****************************************************************
 * Message handling functions.
 */
void Ieee80211OldMac2::handleSelfMessage(cMessage *msg)
{
    if (msg == throughputTimer) {
        throughputLastPeriod = recBytesOverPeriod / SIMTIME_DBL(throughputTimePeriod);
        recBytesOverPeriod = 0;
        scheduleAt(simTime() + throughputTimePeriod, throughputTimer);
        return;
    }

    EV_DEBUG << "received self message: " << msg << "(kind: " << msg->getKind() << ")" << endl;

    if (msg == endReserve)
        nav = false;

    handleWithFSM(msg);
}

void Ieee80211OldMac2::handleUpperPacket(cPacket *msg)
{
    // check if it's a command from the mgmt layer
    if (msg->getBitLength() == 0 && msg->getKind() != 0) {
        handleUpperCommand(msg);
        return;
    }

    // must be a Ieee80211DataOrMgmtFrame, within the max size because we don't support fragmentation
    Ieee80211DataOrMgmtFrame *frame = check_and_cast<Ieee80211DataOrMgmtFrame *>(msg);
    if (frame->getByteLength() > fragmentationThreshold)
        throw cRuntimeError("message from higher layer (%s)%s is too long for 802.11b, %d bytes (fragmentation is not supported yet)",
                msg->getClassName(), msg->getName(), (int)(msg->getByteLength()));
    EV_DEBUG << "frame " << frame << " received from higher layer, receiver = " << frame->getReceiverAddress() << endl;

    // if you get error from this assert check if is client associated to AP
    ASSERT(!frame->getReceiverAddress().isUnspecified());

    // fill in missing fields (receiver address, seq number), and insert into the queue
    frame->setTransmitterAddress(address);
    //frame->setSequenceNumber(sequenceNumber);
    //sequenceNumber = (sequenceNumber+1) % 4096;  //XXX seqNum must be checked upon reception of frames!

    if (mappingAccessCategory(frame) == 200) {
        // if function mappingAccessCategory() returns 200, it means transsmissionQueue is full
        return;
    }
    frame->setMACArrive(simTime());
    handleWithFSM(frame);
}

int Ieee80211OldMac2::classifyFrame(Ieee80211DataOrMgmtFrame *frame)
{
    if (edcCAF.size() <= 1)
        return 0;
    if (frame->getType() == ST_DATA) {
        return defaultAC;
    }
    else if (frame->getType() == ST_DATA_WITH_QOS) {
        Ieee80211DataFrame *dataFrame = check_and_cast<Ieee80211DataFrame*>(frame);
        return mapTidToAc(dataFrame->getTid());  // QoS frames: map TID to AC
    }
    else {
        return AC_VO; // management frames travel in the Voice category
    }
}

AccessCategory Ieee80211OldMac2::mapTidToAc(int tid)
{
    // standard static mapping (see "UP-to-AC mappings" table in the 802.11 spec.)
    switch (tid) {
        case 1: case 2: return AC_BK;
        case 0: case 3: return AC_BE;
        case 4: case 5: return AC_VI;
        case 6: case 7: return AC_VO;
        default: throw cRuntimeError("No mapping from TID=%d to AccessCategory (must be in the range 0..7)", tid);
    }
}



int Ieee80211OldMac2::mappingAccessCategory(Ieee80211DataOrMgmtFrame *frame)
{
    bool isDataFrame = (dynamic_cast<Ieee80211DataFrame *>(frame) != nullptr);

    int tempAC = classifyFrame(frame);;

    // check for queue overflow
    if (isDataFrame && maxQueueSize > 0 && (int)transmissionQueue(tempAC)->size() >= maxQueueSize) {
        EV_WARN << "message " << frame << " received from higher layer but AC queue is full, dropping message\n";
        numDropped()++;
        delete frame;
        return 200;
    }


    // if the frame is not discarded actualize currectAC
    currentAC = tempAC;
    if (isDataFrame) {
        if (!prioritizeMulticast || !frame->getReceiverAddress().isMulticast() || transmissionQueue()->size() < 2)
            transmissionQueue()->push_back(frame);
        else {
            // if the last frame is management insert here
            Ieee80211DataFrame *frameAux = dynamic_cast<Ieee80211DataFrame *>(transmissionQueue()->back());
            if ((frameAux == nullptr) || (frameAux && frameAux->getReceiverAddress().isMulticast()))
                transmissionQueue()->push_back(frame);
            else {
                // in other case search the possition
                auto p = transmissionQueue()->end();
                while ((*p)->getReceiverAddress().isMulticast() && (p != transmissionQueue()->begin())) {    // search the first broadcast frame
                    if (dynamic_cast<Ieee80211DataFrame *>(*p) == nullptr)
                        break;
                    --p;
                }
                ++p;
                transmissionQueue()->insert(p, frame);
            }
        }
    }
    else {
        if (transmissionQueue()->empty() || transmissionQueue()->size() == 1) {
            transmissionQueue()->push_back(frame);
        }
        else {
            //we don't know if first frame in the queue is in middle of transmission
            //so for sure we placed it on second place
            auto p = transmissionQueue()->begin();
            p++;
            while ((dynamic_cast<Ieee80211DataFrame *>(*p) == nullptr) && (p != transmissionQueue()->end())) // search the first not management frame
                p++;
            transmissionQueue()->insert(p, frame);
        }
    }
    EV_DEBUG << "frame classified as access category " << currentAC << " (0 background, 1 best effort, 2 video, 3 voice)\n";
    return true;
}

void Ieee80211OldMac2::handleUpperCommand(cMessage *msg)
{
    if (msg->getKind() == RADIO_C_CONFIGURE) {
        EV_DEBUG << "Passing on command " << msg->getName() << " to physical layer\n";
        if (pendingRadioConfigMsg != nullptr) {
            // merge contents of the old command into the new one, then delete it
            Ieee80211ConfigureRadioCommand *oldConfigureCommand = check_and_cast<Ieee80211ConfigureRadioCommand *>(pendingRadioConfigMsg->getControlInfo());
            Ieee80211ConfigureRadioCommand *newConfigureCommand = check_and_cast<Ieee80211ConfigureRadioCommand *>(msg->getControlInfo());
            if (newConfigureCommand->getChannelNumber() == -1 && oldConfigureCommand->getChannelNumber() != -1)
                newConfigureCommand->setChannelNumber(oldConfigureCommand->getChannelNumber());
            if (std::isnan(newConfigureCommand->getBitrate().get()) && !std::isnan(oldConfigureCommand->getBitrate().get()))
                newConfigureCommand->setBitrate(oldConfigureCommand->getBitrate());
            delete pendingRadioConfigMsg;
            pendingRadioConfigMsg = nullptr;
        }

        if (fsm.getState() == IDLE || fsm.getState() == DEFER || fsm.getState() == BACKOFF) {
            EV_DEBUG << "Sending it down immediately\n";
/*
   // Dynamic power
            PhyControlInfo *phyControlInfo = dynamic_cast<PhyControlInfo *>(msg->getControlInfo());
            if (phyControlInfo)
                phyControlInfo->setAdaptiveSensitivity(true);
   // end dynamic power
 */
            sendDown(msg);
        }
        else {
            EV_DEBUG << "Delaying " << msg->getName() << " until next IDLE or DEFER state\n";
            pendingRadioConfigMsg = msg;
        }
    }
    else {
        throw cRuntimeError("Unrecognized command from mgmt layer: (%s)%s msgkind=%d", msg->getClassName(), msg->getName(), msg->getKind());
    }
}

void Ieee80211OldMac2::handleLowerPacket(cPacket *msg)
{
    EV_TRACE << "->Enter handleLowerMsg...\n";
    EV_DEBUG << "received message from lower layer: " << msg << endl;
    Ieee80211ReceptionIndication *cinfo = dynamic_cast<Ieee80211ReceptionIndication *>(msg->getControlInfo());

    validRecMode = false;
    if (cinfo) {
        recFrameModulation = cinfo->getMode();
        if (!std::isnan(recFrameModulation->getDataMode()->getNetBitrate().get()))
            validRecMode = true;
    }

    Ieee80211Frame *frame = dynamic_cast<Ieee80211Frame *>(msg);

    bool error = msg->hasBitError();

    if (!error)
        sendNotification(NF_LINK_FULL_PROMISCUOUS, msg);

    if (cinfo) {
        if (contJ % 10 == 0) {
            snr = _snr;
            contJ = 0;
            _snr = 0;
        }
        contJ++;
        _snr += cinfo->getSnr() / 10;
        lossRate = cinfo->getLossRate();
    }

    if (msg->getControlInfo())
        delete msg->removeControlInfo();

    if (contI % samplingCoeff == 0) {
        contI = 0;
        recvdThroughput = 0;
    }
    contI++;

    frame = dynamic_cast<Ieee80211Frame *>(msg);
    if (timeStampLastMessageReceived == SIMTIME_ZERO)
        timeStampLastMessageReceived = simTime();
    else {
        if (frame)
            recvdThroughput += ((frame->getBitLength() / (simTime() - timeStampLastMessageReceived)) / 1000000) / samplingCoeff;
        timeStampLastMessageReceived = simTime();
    }
    if (frame && throughputTimer)
        recBytesOverPeriod += frame->getByteLength();

    if (!frame) {
        EV_ERROR << "message from physical layer (%s)%s is not a subclass of Ieee80211Frame" << msg->getClassName() << " " << msg->getName() << endl;
        delete msg;
        return;
        // throw cRuntimeError("message from physical layer (%s)%s is not a subclass of Ieee80211Frame",msg->getClassName(), msg->getName());
    }

    EV_DEBUG << "Self address: " << address
             << ", receiver address: " << frame->getReceiverAddress()
             << ", received frame is for us: " << isForUs(frame)
             << ", received frame was sent by us: " << isSentByUs(frame) << endl;

    Ieee80211TwoAddressFrame *twoAddressFrame = dynamic_cast<Ieee80211TwoAddressFrame *>(msg);
    ASSERT(!twoAddressFrame || twoAddressFrame->getTransmitterAddress() != address);

#ifdef LWMPLS
    int msgKind = msg->getKind();
    if (msgKind != COLLISION && msgKind != BITERROR && twoAddressFrame!=nullptr)
        sendNotification(NF_LINK_REFRESH, twoAddressFrame);
#endif


    if (registerErrors && twoAddressFrame)
    {
        auto it = errorInfo.find(frame->getReceiverAddress());
        if (it == errorInfo.end())
        {
            Ieee80211PacketErrorInfo info;
            std::vector<Ieee80211PacketErrorInfo> infoVector;
            info.Size = frame->getByteLength();
            info.timeRec = simTime();
            info.hasErrors = error;
            infoVector.push_back(info);
            errorInfo.insert(std::pair<MACAddress, std::vector<Ieee80211PacketErrorInfo> >(frame->getReceiverAddress(),infoVector));
        }
        else
        {

            while (!it->second.empty() && (simTime() - it->second.front().timeRec > 10))
                it->second.erase(it->second.begin());
            Ieee80211PacketErrorInfo info;
            info.Size = frame->getByteLength();
            info.timeRec = simTime();
            info.hasErrors = error;
            it->second.push_back(info);
       }
    }

    handleWithFSM(msg);

    // if we are the owner then we did not send this message up
    if (msg->getOwner() == this)
        delete msg;
    EV_TRACE << "Leave handleLowerMsg...\n";
}

void Ieee80211OldMac2::receiveSignal(cComponent *source, simsignal_t signalID, long value, cObject *details)
{
    Enter_Method_Silent();
    if (signalID == IRadio::receptionStateChangedSignal)
        handleWithFSM(mediumStateChange);
    else if (signalID == IRadio::transmissionStateChangedSignal) {
        handleWithFSM(mediumStateChange);
        IRadio::TransmissionState newRadioTransmissionState = (IRadio::TransmissionState)value;
        if (transmissionState == IRadio::TRANSMISSION_STATE_TRANSMITTING && newRadioTransmissionState == IRadio::TRANSMISSION_STATE_IDLE)
            configureRadioMode(IRadio::RADIO_MODE_RECEIVER);
        transmissionState = newRadioTransmissionState;
    }
}

/**
 * Msg can be upper, lower, self or nullptr (when radio state changes)
 */


void Ieee80211OldMac2::finishReception()
{
    if (getCurrentTransmission())
    {

    }
    else
    {
        resetStateVariables();
    }
}


/****************************************************************
 * Timing functions.
 */
simtime_t Ieee80211OldMac2::getSIFS()
{
// TODO:   return aRxRFDelay() + aRxPLCPDelay() + aMACProcessingDelay() + aRxTxTurnaroundTime();
    if (useModulationParameters)
        return dataFrameMode->getSifsTime();

    return SIFS;
}

simtime_t Ieee80211OldMac2::getSlotTime()
{
// TODO:   return aCCATime() + aRxTxTurnaroundTime + aAirPropagationTime() + aMACProcessingDelay();
    if (useModulationParameters)
        return dataFrameMode->getSlotTime();
    return ST;
}




/****************************************************************
 * Timer functions.
 */
void Ieee80211OldMac2::scheduleSIFSPeriod(Ieee80211Frame *frame)
{
    EV_DEBUG << "scheduling SIFS period\n";
    endSIFS->setContextPointer(frame->dup());
    scheduleAt(simTime() + getSIFS(), endSIFS);
}

//XXXvoid Ieee80211OldMac2::checkInternalColision()
//{
//  EV_DEBUG << "We obtain endAIFS, so we have to check if there
//}


void Ieee80211OldMac2::scheduleDataTimeoutPeriod(Ieee80211DataOrMgmtFrame *frameToSend)
{
    double tim;
    double bitRate = dataFrameMode->getDataMode()->getNetBitrate().get();
    TransmissionRequest *trq = dynamic_cast<TransmissionRequest *>(frameToSend->getControlInfo());
    if (trq) {
        bitRate = trq->getBitrate().get();
        if (bitRate == 0)
            bitRate = dataFrameMode->getDataMode()->getNetBitrate().get();
    }
    if (!endTimeout->isScheduled()) {
        EV_DEBUG << "scheduling data timeout period\n";
        if (useModulationParameters) {
            const IIeee80211Mode *modType = modeSet->getMode(bps(bitRate));
            double duration = computeFrameDuration(frameToSend);
            double slot = SIMTIME_DBL(modType->getSlotTime());
            double sifs = SIMTIME_DBL(modType->getSifsTime());
            double PHY_RX_START = SIMTIME_DBL(modType->getPhyRxStartDelay());
            tim = duration + slot + PHY_RX_START;
        }
        else
            tim = computeFrameDuration(frameToSend) + SIMTIME_DBL( getSlotTime())  + MAX_PROPAGATION_DELAY * 2;
        EV_DEBUG << " time out=" << tim*1e6 << "us" << endl;
        scheduleAt(simTime() + tim, endTimeout);
    }
}

void Ieee80211OldMac2::scheduleMulticastTimeoutPeriod(Ieee80211DataOrMgmtFrame *frameToSend)
{
    if (!endTimeout->isScheduled()) {
        EV_DEBUG << "scheduling multicast timeout period\n";
        scheduleAt(simTime() + computeFrameDuration(frameToSend), endTimeout);
    }
}

void Ieee80211OldMac2::cancelTimeoutPeriod()
{
    EV_DEBUG << "canceling timeout period\n";
    if (endTimeout->isScheduled())
        cancelEvent(endTimeout);
}



void Ieee80211OldMac2::scheduleReservePeriod(Ieee80211Frame *frame)
{
    simtime_t reserve = frame->getDuration();

    // see spec. 7.1.3.2
    if (!isForUs(frame) && reserve != 0 && reserve < 32768) {
        if (endReserve->isScheduled()) {
            simtime_t oldReserve = endReserve->getArrivalTime() - simTime();

            if (oldReserve > reserve)
                return;

            reserve = std::max(reserve, oldReserve);
            cancelEvent(endReserve);
        }
        else if (radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE) {
            // NAV: the channel just became virtually busy according to the spec
            scheduleAt(simTime(), mediumStateChange);
        }

        EV_DEBUG << "scheduling reserve period for: " << reserve << endl;

        ASSERT(reserve > 0);

        nav = true;
        scheduleAt(simTime() + reserve, endReserve);
    }
}

void Ieee80211OldMac2::invalidateBackoffPeriod()
{

}

bool Ieee80211OldMac2::isInvalidBackoffPeriod()
{
    return 1;
}
/****************************************************************
 * Frame sender functions.
 */
void Ieee80211OldMac2::sendDataFrameOnEndSIFS(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211Frame *ctsFrame = (Ieee80211Frame *)endSIFS->getContextPointer();
    endSIFS->setContextPointer(nullptr);
    sendDataFrame(frameToSend);
    delete ctsFrame;
}


void Ieee80211OldMac2::sendDataFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    simtime_t t = 0, time = 0;
    int count = 0;
    auto frame = transmissionQueue()->begin();
    ASSERT(*frame == frameToSend);
    if (transmissionQueue()->size() >= 2) {
        //we start packet burst within TXOP time period

        for (frame = transmissionQueue()->begin(); frame != transmissionQueue()->end(); ++frame) {
            count++;
            t = computeFrameDuration(*frame) + 2 * getSIFS();
            EV_DEBUG << "t is " << t << endl;
            /*if (TXOP() > time + t) {
                time += t;
                EV_DEBUG << "adding t\n";
            }*/
            if(1){}
            else {
                break;
            }
        }
        //to be sure we get endTXOP earlier then receive ACK and we have to minus SIFS time from first packet
        time -= getSIFS() / 2 + getSIFS();
        EV_DEBUG << "duration is " << time << ", count is " << count << endl;

    }
    EV_INFO << "sending Data frame\n";
    configureRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    sendDown(buildDataFrame(dynamic_cast<Ieee80211DataOrMgmtFrame *>(setBitrateFrame(frameToSend))));
}




void Ieee80211OldMac2::sendMulticastFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    EV_INFO << "sending Multicast frame\n";
    if (frameToSend->getControlInfo())
        delete frameToSend->removeControlInfo();
    configureRadioMode(IRadio::RADIO_MODE_TRANSMITTER);
    sendDown(buildDataFrame(dynamic_cast<Ieee80211DataOrMgmtFrame *>(setBasicBitrate(frameToSend))));
}



/****************************************************************
 * Frame builder functions.
 */
Ieee80211DataOrMgmtFrame *Ieee80211OldMac2::buildDataFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    if (retryCounter() == 0)
    {
        frameToSend->setSequenceNumber(sequenceNumber);
        sequenceNumber = (sequenceNumber+1) % 4096;  //XXX seqNum must be checked upon reception of frames!
    }

    Ieee80211DataOrMgmtFrame *frame = (Ieee80211DataOrMgmtFrame *)frameToSend->dup();

    if (frameToSend->getControlInfo() != nullptr) {
        cObject *ctr = frameToSend->getControlInfo();
        TransmissionRequest *ctrl = dynamic_cast<TransmissionRequest *>(ctr);
        if (ctrl == nullptr)
            throw cRuntimeError("control info is not TransmissionRequest but %s", ctr->getClassName());
        frame->setControlInfo(ctrl->dup());
    }
    if (isMulticast(frameToSend))
        frame->setDuration(0);
    else if (!frameToSend->getMoreFragments()) {
        if ( transmissionQueue()->size() > 1) {
            // ++ operation is safe because txop is true
            auto nextframeToSend = transmissionQueue()->begin();
            nextframeToSend++;
            ASSERT(transmissionQueue()->end() != nextframeToSend);
            double bitRate = dataFrameMode->getDataMode()->getNetBitrate().get();
            int size = (*nextframeToSend)->getBitLength();
            TransmissionRequest *trRq = dynamic_cast<TransmissionRequest *>(transmissionQueue()->front()->getControlInfo());
            if (trRq) {
                bitRate = trRq->getBitrate().get();
                if (bitRate == 0)
                    bitRate = dataFrameMode->getDataMode()->getNetBitrate().get();
            }
            frame->setDuration(3 * getSIFS()  + computeFrameDuration(size, bitRate));
        }
        else
            frame->setDuration(getSIFS() );
    }
    else
        // FIXME: shouldn't we use the next frame to be sent?
        frame->setDuration(3 * getSIFS()  + computeFrameDuration(frameToSend));

    return frame;
}


Ieee80211DataOrMgmtFrame *Ieee80211OldMac2::buildMulticastFrame(Ieee80211DataOrMgmtFrame *frameToSend)
{
    Ieee80211DataOrMgmtFrame *frame = (Ieee80211DataOrMgmtFrame *)frameToSend->dup();

    TransmissionRequest *oldTransmissionRequest = dynamic_cast<TransmissionRequest *>(frameToSend->getControlInfo());
    if (oldTransmissionRequest) {
        EV_DEBUG << "Per frame1 params" << endl;
        TransmissionRequest *newTransmissionRequest = new TransmissionRequest();
        *newTransmissionRequest = *oldTransmissionRequest;
        frame->setControlInfo(newTransmissionRequest);
    }

    frame->setDuration(0);
    return frame;
}

Ieee80211Frame *Ieee80211OldMac2::setBasicBitrate(Ieee80211Frame *frame)
{
    ASSERT(frame->getControlInfo() == nullptr);
    TransmissionRequest *ctrl = new TransmissionRequest();
    ctrl->setBitrate(bps(basicFrameMode->getDataMode()->getNetBitrate()));
    frame->setControlInfo(ctrl);
    return frame;
}

Ieee80211Frame *Ieee80211OldMac2::setControlBitrate(Ieee80211Frame *frame)
{
    ASSERT(frame->getControlInfo()==nullptr);
    TransmissionRequest *ctrl = new TransmissionRequest();
    ctrl->setBitrate((bps)controlFrameMode->getDataMode()->getNetBitrate());
    frame->setControlInfo(ctrl);
    return frame;
}

Ieee80211Frame *Ieee80211OldMac2::setBitrateFrame(Ieee80211Frame *frame)
{
    if (rateControlMode == RATE_CR && forceBitRate == false) {
        if (frame->getControlInfo())
            delete frame->removeControlInfo();
        return frame;
    }
    TransmissionRequest *ctrl = nullptr;
    if (frame->getControlInfo() == nullptr) {
        ctrl = new TransmissionRequest();
        frame->setControlInfo(ctrl);
    }
    else
        ctrl = dynamic_cast<TransmissionRequest *>(frame->getControlInfo());
    if (ctrl)
        ctrl->setBitrate(bps(getBitrate()));
    return frame;
}

/****************************************************************
 * Helper functions.
 */
void Ieee80211OldMac2::finishCurrentTransmission()
{
    popTransmissionQueue();
    resetStateVariables();
}

void Ieee80211OldMac2::giveUpCurrentTransmission()
{
    Ieee80211DataOrMgmtFrame *temp = (Ieee80211DataOrMgmtFrame *)transmissionQueue()->front();
    sendNotification(NF_LINK_BREAK, temp);
    popTransmissionQueue();
    resetStateVariables();
    numGivenUp()++;
}


Ieee80211DataOrMgmtFrame *Ieee80211OldMac2::getCurrentTransmission()
{
    return transmissionQueue()->empty() ? nullptr : (Ieee80211DataOrMgmtFrame *)transmissionQueue()->front();
}

void Ieee80211OldMac2::sendDownPendingRadioConfigMsg()
{
    if (pendingRadioConfigMsg != nullptr) {
        sendDown(pendingRadioConfigMsg);
        pendingRadioConfigMsg = nullptr;
    }
}

void Ieee80211OldMac2::setMode(Mode mode)
{
    if (mode == PCF || mode == DCF || mode == EDCA)
        throw cRuntimeError("Mde not yet supported");

    this->mode = mode;
}

void Ieee80211OldMac2::resetStateVariables()
{

    if (rateControlMode == RATE_AARF || rateControlMode == RATE_ARF)
        reportDataOk();
    else
        retryCounter() = 0;

    if (!transmissionQueue()->empty()) {
        getCurrentTransmission()->setRetry(false);
    }
    else {
    }
}

bool Ieee80211OldMac2::isMediumStateChange(cMessage *msg)
{
    return msg == mediumStateChange || (msg == endReserve && radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE);
}

bool Ieee80211OldMac2::isMediumFree()
{
    return !endReserve->isScheduled() && radio->getReceptionState() == IRadio::RECEPTION_STATE_IDLE;
}

bool Ieee80211OldMac2::isMediumRecv()
{
    return !endReserve->isScheduled() && radio->getReceptionState() == IRadio::RECEPTION_STATE_RECEIVING;
}


bool Ieee80211OldMac2::isMulticast(Ieee80211Frame *frame)
{
    return frame && frame->getReceiverAddress().isMulticast();
}

bool Ieee80211OldMac2::isForUs(Ieee80211Frame *frame)
{
    return frame && frame->getReceiverAddress() == address;
}

bool Ieee80211OldMac2::isSentByUs(Ieee80211Frame *frame)
{
    if (dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame)) {
        //EV_DEBUG << "ad3 "<<((Ieee80211DataOrMgmtFrame *)frame)->getAddress3();
        //EV_DEBUG << "myad "<<address<<endl;
        if (((Ieee80211DataOrMgmtFrame *)frame)->getAddress3() == address) //received frame sent by us
            return 1;
    }
    else
        EV_ERROR << "Cast failed" << endl; // WTF? (levy)

    return 0;
}

bool Ieee80211OldMac2::isDataOrMgmtFrame(Ieee80211Frame *frame)
{
    return dynamic_cast<Ieee80211DataOrMgmtFrame *>(frame);
}


Ieee80211Frame *Ieee80211OldMac2::getFrameReceivedBeforeSIFS()
{
    return (Ieee80211Frame *)endSIFS->getContextPointer();
}

void Ieee80211OldMac2::popTransmissionQueue()
{
    EV_DEBUG << "dropping frame from transmission queue\n";
    Ieee80211Frame *temp = dynamic_cast<Ieee80211Frame *>(transmissionQueue()->front());
    ASSERT(!transmissionQueue()->empty());
    transmissionQueue()->pop_front();
    delete temp;
}

double Ieee80211OldMac2::computeFrameDuration(Ieee80211Frame *msg)
{
    TransmissionRequest *ctrl;
    double duration;
    EV_DEBUG << *msg;
    ctrl = dynamic_cast<TransmissionRequest *>(msg->removeControlInfo());
    if (ctrl) {
        EV_DEBUG << "Per frame2 params bitrate " << ctrl->getBitrate().get() / 1e6 << "Mb" << endl;
        duration = computeFrameDuration(msg->getBitLength(), ctrl->getBitrate().get());
        delete ctrl;
        return duration;
    }
    else
        return computeFrameDuration(msg->getBitLength(), dataFrameMode->getDataMode()->getNetBitrate().get());
}

double Ieee80211OldMac2::computeFrameDuration(int bits, double bitrate)
{
    double duration;
    const IIeee80211Mode *modType = modeSet->getMode(bps(bitrate));
    if (PHY_HEADER_LENGTH < 0)
        duration = SIMTIME_DBL(modType->getDuration(bits));
    else
        duration = SIMTIME_DBL(modType->getDataMode()->getDuration(bits)) + PHY_HEADER_LENGTH;

    EV_DEBUG << " duration=" << duration * 1e6 << "us(" << bits << "bits " << bitrate / 1e6 << "Mbps)" << endl;
    return duration;
}

void Ieee80211OldMac2::logState()
{
    int numCategs = numCategories();
    EV_TRACE << "# state information: mode = " << modeName(mode) << ", state = " << fsm.getStateName();
    EV_TRACE << ", backoff 0.." << numCategs << " =";
    for (int i = 0; i < numCategs; i++)
        EV_TRACE << " " << edcCAF[i].backoff;
    EV_TRACE << "\n# backoffPeriod 0.." << numCategs << " =";
    for (int i = 0; i < numCategs; i++)
        EV_TRACE << " " << edcCAF[i].backoffPeriod;
    EV_TRACE << "\n# retryCounter 0.." << numCategs << " =";
    for (int i = 0; i < numCategs; i++)
        EV_TRACE << " " << edcCAF[i].retryCounter;
    EV_TRACE << ", radioMode = " << radio->getRadioMode()
             << ", receptionState = " << radio->getReceptionState()
             << ", transmissionState = " << radio->getTransmissionState()
             << ", nav = " << nav << ", txop is " << txop << "\n";
    EV_TRACE << "#queue size 0.." << numCategs << " =";
    for (int i = 0; i < numCategs; i++)
        EV_TRACE << " " << transmissionQueue(i)->size();
    EV_TRACE << ", medium is " << (isMediumFree() ? "free" : "busy");
    EV_TRACE << ", ";
    EV_TRACE << "\n# currentAC: " << currentAC << ", oldcurrentAC: " << oldcurrentAC;
    if (getCurrentTransmission() != nullptr)
        EV_TRACE << "\n# current transmission: " << getCurrentTransmission()->getId();
    else
        EV_TRACE << "\n# current transmission: none";
    EV_TRACE << endl;
}

const char *Ieee80211OldMac2::modeName(int mode)
{
#define CASE(x)    case x: \
        s = #x; break
    const char *s = "???";
    switch (mode) {
        CASE(DCF);
        CASE(PCF);
        CASE(CSMA_ONLY);
    }
    return s;
#undef CASE
}

bool Ieee80211OldMac2::transmissionQueuesEmpty()
{
    for (int i = 0; i < numCategories(); i++)
        if (!transmissionQueue(i)->empty())
            return false;

    return true;
}

unsigned int Ieee80211OldMac2::getTotalQueueLength()
{
    unsigned int totalSize = 0;
    for (int i = 0; i < numCategories(); i++)
        totalSize += transmissionQueue(i)->size();
    return totalSize;
}

void Ieee80211OldMac2::flushQueue()
{
    for (int i = 0; i < numCategories(); i++) {
        while (!transmissionQueue(i)->empty()) {
            cMessage *msg = transmissionQueue(i)->front();
            transmissionQueue(i)->pop_front();
            //TODO emit(dropPkIfaceDownSignal, msg); -- 'pkDropped' signals are missing in this module!
            delete msg;
        }
    }
}

void Ieee80211OldMac2::clearQueue()
{
    for (int i = 0; i < numCategories(); i++) {
        while (!transmissionQueue(i)->empty()) {
            cMessage *msg = transmissionQueue(i)->front();
            transmissionQueue(i)->pop_front();
            delete msg;
        }
    }
}

void Ieee80211OldMac2::reportDataOk()
{
    retryCounter() = 0;
    if (rateControlMode == RATE_CR)
        return;
    successCounter++;
    failedCounter = 0;
    recovery = false;
    if ((successCounter == getSuccessThreshold() || timer == getTimerTimeout())
        && modeSet->getFasterMode(dataFrameMode))
    {
        dataFrameMode = modeSet->getFasterMode(dataFrameMode);
        timer = 0;
        successCounter = 0;
        recovery = true;
    }
}

void Ieee80211OldMac2::reportDataFailed(void)
{
    retryCounter()++;
    if (rateControlMode == RATE_CR)
        return;
    timer++;
    failedCounter++;
    successCounter = 0;
    if (recovery) {
        if (retryCounter() == 1) {
            reportRecoveryFailure();
            const IIeee80211Mode *slowerMode = modeSet->getSlowerMode(dataFrameMode);
            if (slowerMode != nullptr)
                dataFrameMode = slowerMode;
        }
        timer = 0;
    }
    else {
        if (needNormalFallback()) {
            reportFailure();
            const IIeee80211Mode *slowerMode = modeSet->getSlowerMode(dataFrameMode);
            if (slowerMode != nullptr)
                dataFrameMode = slowerMode;
        }
        if (retryCounter() >= 2) {
            timer = 0;
        }
    }
}

int Ieee80211OldMac2::getMinTimerTimeout(void)
{
    return minTimerTimeout;
}

int Ieee80211OldMac2::getMinSuccessThreshold(void)
{
    return minSuccessThreshold;
}

int Ieee80211OldMac2::getTimerTimeout(void)
{
    return timerTimeout;
}

int Ieee80211OldMac2::getSuccessThreshold(void)
{
    return successThreshold;
}

void Ieee80211OldMac2::setTimerTimeout(int timer_timeout)
{
    if (timer_timeout >= minTimerTimeout)
        timerTimeout = timer_timeout;
    else
        throw cRuntimeError("timer_timeout is less than minTimerTimeout");
}
void Ieee80211OldMac2::setSuccessThreshold(int success_threshold)
{
    if (success_threshold >= minSuccessThreshold)
        successThreshold = success_threshold;
    else
        throw cRuntimeError("success_threshold is less than minSuccessThreshold");
}

void Ieee80211OldMac2::reportRecoveryFailure(void)
{
    if (rateControlMode == RATE_AARF) {
        setSuccessThreshold((int)(std::min((double)getSuccessThreshold() * successCoeff, (double)maxSuccessThreshold)));
        setTimerTimeout((int)(std::max((double)getMinTimerTimeout(), (double)(getSuccessThreshold() * timerCoeff))));
    }
}

void Ieee80211OldMac2::reportFailure(void)
{
    if (rateControlMode == RATE_AARF) {
        setTimerTimeout(getMinTimerTimeout());
        setSuccessThreshold(getMinSuccessThreshold());
    }
}

bool Ieee80211OldMac2::needRecoveryFallback(void)
{
    if (retryCounter() == 1) {
        return true;
    }
    else {
        return false;
    }
}

bool Ieee80211OldMac2::needNormalFallback(void)
{
    int retryMod = (retryCounter() - 1) % 2;
    if (retryMod == 1) {
        return true;
    }
    else {
        return false;
    }
}

double Ieee80211OldMac2::getBitrate()
{
    return dataFrameMode->getDataMode()->getNetBitrate().get();
}

void Ieee80211OldMac2::setBitrate(double rate)
{
    dataFrameMode = modeSet->getMode(bps(rate));
}

// method for access to the EDCA data

// methods for access to specific AC data

int& Ieee80211OldMac2::retryCounter(int i)
{
    if (i == -1)
        i = currentAC;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].retryCounter;
}

const bool Ieee80211OldMac2::isBackoffMsg(cMessage *msg)
{
    for (auto & elem : edcCAF) {
        if (msg == elem.endBackoff)
            return true;
    }
    return false;
}

// Statistics
long& Ieee80211OldMac2::numRetry(int i)
{
    if (i == -1)
        i = 0;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].numRetry;
}

long& Ieee80211OldMac2::numSentWithoutRetry(int i)
{
    if (i == -1)
        i = 0;
    if (i >= (int)numCategories())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].numSentWithoutRetry;
}

long& Ieee80211OldMac2::numGivenUp(int i)
{
    if (i == -1)
        i = 0;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].numGivenUp;
}

long& Ieee80211OldMac2::numSent(int i)
{
    if (i == -1)
        i = 0;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].numSent;
}

long& Ieee80211OldMac2::numDropped(int i)
{
    if (i == -1)
        i = 0;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].numDropped;
}

long& Ieee80211OldMac2::bits(int i)
{
    if (i == -1)
        i = 0;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].bits;
}

simtime_t& Ieee80211OldMac2::minJitter(int i)
{
    if (i == -1)
        i = 0;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].minjitter;
}

simtime_t& Ieee80211OldMac2::maxJitter(int i)
{
    if (i == -1)
        i = 0;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAF[i].maxjitter;
}

// out vectors

cOutVector *Ieee80211OldMac2::jitter(int i)
{
    if (i == -1)
        i = 0;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAFOutVector[i].jitter;
}

cOutVector *Ieee80211OldMac2::macDelay(int i)
{
    if (i == -1)
        i = 0;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAFOutVector[i].macDelay;
}

cOutVector *Ieee80211OldMac2::throughput(int i)
{
    if (i == -1)
        i = 0;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return edcCAFOutVector[i].throughput;
}

Ieee80211OldMac2::Ieee80211DataOrMgmtFrameList *Ieee80211OldMac2::transmissionQueue(int i)
{
    if (i == -1)
        i = 0;
    if (i >= (int)edcCAF.size())
        throw cRuntimeError("AC doesn't exist");
    return &(edcCAF[i].transmissionQueue);
}

const IIeee80211Mode *Ieee80211OldMac2::getControlAnswerMode(const IIeee80211Mode *reqMode)
{
    /**
     * The standard has relatively unambiguous rules for selecting a
     * control response rate (the below is quoted from IEEE 802.11-2007,
     * Section 9.6):
     *
     *   To allow the transmitting STA to calculate the contents of the
     *   Duration/ID field, a STA responding to a received frame shall
     *   transmit its Control Response frame (either CTS or ACK), other
     *   than the BlockAck control frame, at the highest rate in the
     *   BSSBasicRateSet parameter that is less than or equal to the
     *   rate of the immediately previous frame in the frame exchange
     *   sequence (as defined in 9.12) and that is of the same
     *   modulation class (see 9.6.1) as the received frame...
     */

    /**
     * If no suitable basic rate was found, we search the mandatory
     * rates. The standard (IEEE 802.11-2007, Section 9.6) says:
     *
     *   ...If no rate contained in the BSSBasicRateSet parameter meets
     *   these conditions, then the control frame sent in response to a
     *   received frame shall be transmitted at the highest mandatory
     *   rate of the PHY that is less than or equal to the rate of the
     *   received frame, and that is of the same modulation class as the
     *   received frame. In addition, the Control Response frame shall
     *   be sent using the same PHY options as the received frame,
     *   unless they conflict with the requirement to use the
     *   BSSBasicRateSet parameter.
     *
     * TODO: Note that we're ignoring the last sentence for now, because
     * there is not yet any manipulation here of PHY options.
     */
    const IIeee80211Mode *bestMode = nullptr;
    const IIeee80211Mode *mode = modeSet->getSlowestMode();
    while (mode != nullptr) {
        /* If the rate:
         *
         *  - is a mandatory rate for the PHY, and
         *  - is equal to or faster than our current best choice, and
         *  - is less than or equal to the rate of the received frame, and
         *  - is of the same modulation class as the received frame
         *
         * ...then it's our best choice so far.
         */
        if (modeSet->getIsMandatory(mode) &&
            (!bestMode || mode->getDataMode()->getGrossBitrate() > bestMode->getDataMode()->getGrossBitrate()) &&
            mode->getDataMode()->getGrossBitrate() <= reqMode->getDataMode()->getGrossBitrate() &&
            // TODO: same modulation class
            typeid(*mode) == typeid(*bestMode))
        {
            bestMode = mode;
            // As above; we've found a potentially-suitable transmit
            // rate, but we need to continue and consider all the
            // mandatory rates before we can be sure we've got the right
            // one.
        }
    }

    /**
     * If we still haven't found a suitable rate for the response then
     * someone has messed up the simulation config. This probably means
     * that the WifiPhyStandard is not set correctly, or that a rate that
     * is not supported by the PHY has been explicitly requested in a
     * WifiRemoteStationManager (or descendant) configuration.
     *
     * Either way, it is serious - we can either disobey the standard or
     * fail, and I have chosen to do the latter...
     */
    if (!bestMode) {
        throw cRuntimeError("Can't find response rate for reqMode. Check standard and selected rates match.");
    }

    return bestMode;
}

// This methods implemet the duplicate filter
void Ieee80211OldMac2::sendUp(cMessage *msg)
{
    if (isDuplicated(msg))
        EV_INFO << "Discarding duplicate message\n";
    else {
        EV_INFO << "Sending up " << msg << "\n";
        if (msg->isPacket())
            emit(packetSentToUpperSignal, msg);
        send(msg, upperLayerOutGateId);
    }
}

void Ieee80211OldMac2::removeOldTuplesFromDuplicateMap()
{
    if (duplicateDetect && lastTimeDelete + duplicateTimeOut >= simTime()) {
        lastTimeDelete = simTime();
        for (auto it = asfTuplesList.begin(); it != asfTuplesList.end(); ) {
            if (it->second.receivedTime + duplicateTimeOut < simTime()) {
                auto itAux = it;
                it++;
                asfTuplesList.erase(itAux);
            }
            else
                it++;
        }
    }
}

const MACAddress& Ieee80211OldMac2::isInterfaceRegistered()
{
    if (!par("multiMac").boolValue())
        return MACAddress::UNSPECIFIED_ADDRESS;

    IInterfaceTable *ift = findModuleFromPar<IInterfaceTable>(par("interfaceTableModule"), this);
    if (!ift)
        return MACAddress::UNSPECIFIED_ADDRESS;
    cModule *interfaceModule = findModuleUnderContainingNode(this);
    if (!interfaceModule)
        throw cRuntimeError("NIC module not found in the host");
    std::string interfaceName = utils::stripnonalnum(interfaceModule->getFullName());
    InterfaceEntry *e = ift->getInterfaceByName(interfaceName.c_str());
    if (e)
        return e->getMacAddress();
    return MACAddress::UNSPECIFIED_ADDRESS;
}

bool Ieee80211OldMac2::isDuplicated(cMessage *msg)
{
    if (duplicateDetect) {    // duplicate detection filter
        Ieee80211DataOrMgmtFrame *frame = dynamic_cast<Ieee80211DataOrMgmtFrame *>(msg);
        if (frame) {
            auto it = asfTuplesList.find(frame->getTransmitterAddress());
            if (it == asfTuplesList.end()) {
                Ieee80211ASFTuple tuple;
                tuple.receivedTime = simTime();
                tuple.sequenceNumber = frame->getSequenceNumber();
                tuple.fragmentNumber = frame->getFragmentNumber();
                asfTuplesList.insert(std::pair<MACAddress, Ieee80211ASFTuple>(frame->getTransmitterAddress(), tuple));
            }
            else {
                // check if duplicate
                if (it->second.sequenceNumber == frame->getSequenceNumber()
                    && it->second.fragmentNumber == frame->getFragmentNumber())
                {
                    return true;
                }
                else {
                    // actualize
                    it->second.sequenceNumber = frame->getSequenceNumber();
                    it->second.fragmentNumber = frame->getFragmentNumber();
                    it->second.receivedTime = simTime();
                }
            }
        }
    }
    return false;
}

void Ieee80211OldMac2::promiscousFrame(cMessage *msg)
{
    if (!isDuplicated(msg)) // duplicate detection filter
        emit(NF_LINK_PROMISCUOUS, msg);
}

bool Ieee80211OldMac2::isBackoffPending()
{
    for (auto & elem : edcCAF) {
        if (elem.backoff)
            return true;
    }
    return false;
}

double Ieee80211OldMac2::controlFrameTxTime(int bits)
{
     double duration;
     if (PHY_HEADER_LENGTH < 0)
         duration = SIMTIME_DBL(controlFrameMode->getDuration(bits));
     else
         duration = SIMTIME_DBL(controlFrameMode->getDataMode()->getDuration(bits)) + PHY_HEADER_LENGTH;

     EV_DEBUG << " duration=" << duration*1e6 << "us(" << bits << "bits " << controlFrameMode->getDataMode()->getNetBitrate() << ")" << endl;
     return duration;
}

bool Ieee80211OldMac2::handleNodeStart(IDoneCallback *doneCallback)
{
    if (!doneCallback)
        return true; // do nothing when called from initialize()

    bool ret = MACProtocolBase::handleNodeStart(doneCallback);
    radio->setRadioMode(IRadio::RADIO_MODE_RECEIVER);
    return ret;
}

bool Ieee80211OldMac2::handleNodeShutdown(IDoneCallback *doneCallback)
{
    bool ret = MACProtocolBase::handleNodeStart(doneCallback);
    handleNodeCrash();
    return ret;
}

void Ieee80211OldMac2::handleNodeCrash()
{
    cancelEvent(endSIFS);
    cancelEvent(endTimeout);
    cancelEvent(endReserve);
    cancelEvent(mediumStateChange);
    for (unsigned int i = 0; i < edcCAF.size(); i++) {
        while (!transmissionQueue(i)->empty()) {
            Ieee80211Frame *temp = dynamic_cast<Ieee80211Frame *>(transmissionQueue(i)->front());
            transmissionQueue(i)->pop_front();
            delete temp;
        }
    }
}

void Ieee80211OldMac2::configureRadioMode(IRadio::RadioMode radioMode)
{
    if (radio->getRadioMode() != radioMode) {
        ConfigureRadioCommand *configureCommand = new ConfigureRadioCommand();
        configureCommand->setRadioMode(radioMode);
        cMessage *message = new cMessage("configureRadioMode", RADIO_C_CONFIGURE);
        message->setControlInfo(configureCommand);
        sendDown(message);
    }
}

} // namespace ieee80211

} // namespace inet

