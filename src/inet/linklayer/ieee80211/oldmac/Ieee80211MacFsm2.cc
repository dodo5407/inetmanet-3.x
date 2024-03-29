
#include "Ieee80211OldMac2.h"

namespace inet {

namespace ieee80211 {

bool Ieee80211OldMac2::initFsm(cMessage *msg,bool &receptionError, Ieee80211Frame *& frame)
{

    removeOldTuplesFromDuplicateMap();
    // skip those cases where there's nothing to do, so the switch looks simpler

    logState();
    stateVector.record(fsm.getState());

    receptionError = false;
    frame = dynamic_cast<Ieee80211Frame*>(msg);
    if (frame && isLowerMessage(frame)) {
        lastReceiveFailed = receptionError = frame ? frame->hasBitError() : false;
        scheduleReservePeriod(frame);
    }
    return true;
}

void Ieee80211OldMac2::endFsm(cMessage *msg)
{
    EV_TRACE << "leaving handleWithFSM\n\t";
    logState();
    stateVector.record(fsm.getState());
    if (simTime() - last > 0.1)
    {
        for (int i = 0; i<numCategories(); i++)
        {
            throughput(i)->record(bits(i)/(simTime()-last));
            bits(i) = 0;
        }
        last = simTime();
    }
}

void Ieee80211OldMac2::handleWithFSM(cMessage *msg)
{

    bool receptionError;
    Ieee80211Frame *frame;
    if (!initFsm(msg, receptionError, frame))
        return;

    //int frameType = frame ? frame->getType() : -1;

    // TODO: fix bug according to the message: [omnetpp] A possible bug in the Ieee80211's FSM.
    FSMA_Switch(fsm) {
        FSMA_State(IDLE) {
            FSMA_Enter(sendDownPendingRadioConfigMsg());
            /*
               if (fixFSM)
               {
               FSMA_Event_Transition(Data-Ready,
                                  // isUpperMessage(msg),
                                  isUpperMessage(msg) && backoffPeriod[currentAC] > 0,
                                  DEFER,
                //ASSERT(isInvalidBackoffPeriod() || backoffPeriod == 0);
                //invalidateBackoffPeriod();
               ASSERT(false);

               );
               FSMA_No_Event_Transition(Immediate-Data-Ready,
                                     //!transmissionQueue.empty(),
                !transmissionQueuesEmpty(),
                                     DEFER,
               //  invalidateBackoffPeriod();
                ASSERT(backoff[currentAC]);

               );
               }
             */
            FSMA_Event_Transition(Data - Ready,
                    isUpperMessage(msg),
                    DEFER,
                    );
            FSMA_No_Event_Transition(Immediate - Data - Ready,
                    !transmissionQueuesEmpty(),
                    DEFER,
                    frame = getCurrentTransmission();
                    );
            FSMA_Event_Transition(Receive,
                    isLowerMessage(msg),
                    RECEIVE,
                    );
        }
        FSMA_State(DEFER) {
            FSMA_Enter(sendDownPendingRadioConfigMsg());
            FSMA_Event_Transition(Wait - AIFS,
                    isMediumStateChange(msg) && isMediumFree(),
                    WAITSIFS,
                    frame = getCurrentTransmission();
                    );
            //FSMA_No_Event_Transition(Immediate - Wait - AIFS,
            //        isMediumFree() || (!isBackoffPending()),
            //        TRANSMIT,
            //        ;
            //        );
            FSMA_No_Event_Transition(Immediate - Wait - AIFS,
                    isMediumFree() && lastEndSIFSTime != simTime(),
                    WAITSIFS,
                    ;
                    );
            FSMA_Event_Transition(Receive,
                    isLowerMessage(msg),
                    RECEIVE,
                    ;
                    );
        }
        FSMA_State(TRANSMIT) {
            FSMA_No_Event_Transition(Immediate - Transmit - Multicast,
                    isMulticast(getCurrentTransmission()),
                    IDLE,
                    sendMulticastFrame(getCurrentTransmission());
                    oldcurrentAC = currentAC;
                    numSentMulticast++;
                    fr = getCurrentTransmission();
                    numBits += fr->getBitLength();
                    bits() += fr->getBitLength();
                    finishCurrentTransmission();
                    );
            FSMA_No_Event_Transition(Immediate - Transmit - Data,
                    !isMulticast(getCurrentTransmission()),
                    IDLE,
                    sendDataFrame(getCurrentTransmission());
                    oldcurrentAC = currentAC;
                    if (retryCounter() == 0) numSentWithoutRetry()++;
                    numSent()++;
                    fr = getCurrentTransmission();
                    numBits += fr->getBitLength();
                    bits() += fr->getBitLength();
                    finishCurrentTransmission();
                    );
            // radio state changes before we actually get the message, so this must be here
            FSMA_Event_Transition(Receive,
                    isLowerMessage(msg),
                    RECEIVE,
                    );
        }
        FSMA_State(WAITSIFS)
        {
            FSMA_Enter(scheduleSIFSPeriod(frame));
            FSMA_Event_Transition(Transmit-Data-TXOP,
                    msg == endSIFS && ((Ieee80211TwoAddressFrame *)getFrameReceivedBeforeSIFS())->getTransmitterAddress() == address && getFrameReceivedBeforeSIFS()->getType() == ST_DATA,
                    TRANSMIT,
                    delete getFrameReceivedBeforeSIFS();
                    endSIFS->setContextPointer(nullptr);
                    lastEndSIFSTime = simTime();
                    );
        }
        // this is not a real state
        FSMA_State(RECEIVE)
        {
            FSMA_No_Event_Transition(Immediate-Receive-Error,
                    isLowerMessage(msg) && receptionError,
                    IDLE,
                    EV << "received frame contains bit errors or collision, next wait period is EIFS\n";
                    numCollision++;
                    isMulticast(frame) ? numCollisionMulticastVector.record(1) : numCollisionVector.record(1);
                    finishReception();
                    );
            FSMA_No_Event_Transition(Immediate-Receive-Multicast,
                    isLowerMessage(msg) && isMulticast(frame) && !isSentByUs(frame) && isDataOrMgmtFrame(frame),
                    IDLE,
                    sendUp(frame);
                    numReceivedMulticast++;
                    numReceivedMulticastVector.record(1);
                    finishReception();
                    );
            FSMA_No_Event_Transition(Immediate-Receive-Data,
                    isLowerMessage(msg) && isForUs(frame) && isDataOrMgmtFrame(frame),
                    IDLE,
                    sendUp(frame);
                    numReceived++;
                    numReceivedVector.record(1);
                    );
            FSMA_No_Event_Transition(Immediate-Promiscuous-Data,
                    isLowerMessage(msg) && !isForUs(frame) && isDataOrMgmtFrame(frame),
                    IDLE,
                    promiscousFrame(frame);
                    finishReception();
                    numReceivedOther++;
                    numReceivedOtherVector.record(1);
                    );
            FSMA_No_Event_Transition(Immediate-Receive-Other,
                    isLowerMessage(msg),
                    IDLE,
                    finishReception();
                    numReceivedOther++;
                    numReceivedOtherVector.record(1);
                    );
        }
    }
    endFsm(msg);
}


}

}

