//
// Copyright (C) 2006 Andras Varga and Levente Meszaros (original code)
// Copyright (C) 2007 Sorin    (802.11g)
// Copyright (C) 2009 Lukáš Hlůže (Lukás Hluze) lukas@hluze.cz (802.11e)
// Copyright (C) 2011 Alfonso Ariza  (clean code, fix some errors, new radio model)
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

#ifndef __INET_IEEE80211OLDMAC2_H
#define __INET_IEEE80211OLDMAC2_H

// un-comment this if you do not want to log state machine transitions
//#define FSM_DEBUG

#include "inet/common/INETDefs.h"
#include "inet/common/FSMA.h"
#include "inet/common/INETMath.h"
#include "inet/common/lifecycle/ILifecycle.h"
#include "inet/physicallayer/contract/packetlevel/IRadio.h"
#include "inet/physicallayer/ieee80211/mode/IIeee80211Mode.h"
#include "inet/physicallayer/ieee80211/mode/Ieee80211ModeSet.h"
#include "inet/linklayer/base/MACProtocolBase.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/oldmac/Ieee80211Consts.h"
#include "inet/linklayer/ieee80211/mac/common/AccessCategory.h"

namespace inet {

namespace ieee80211 {

using namespace physicallayer;

/**
 * IEEE 802.11g with e Media Access Control Layer.
 *
 * Various comments in the code refer to the Wireless LAN Medium Access
 * Control (MAC) and Physical Layer(PHY) Specifications
 * ANSI/IEEE Std 802.11, 1999 Edition (R2003)
 * ANSI/IEEE Std 802.11, 2007 Edition
 *
 * For more info, see the NED file.
 *
 * TODO: support fragmentation
 * TODO: PCF mode
 * TODO: CF period
 * TODO: pass radio power to upper layer
 * TODO: transmission complete notification to upper layer
 * TODO: STA TCF timer synchronization, see Chapter 11 pp 123
 *
 * Parts of the implementation have been taken over from the
 * Mobility Framework's Mac80211 module.
 *
 * @ingroup macLayer
 */
class INET_API Ieee80211OldMac2 : public MACProtocolBase
{
    typedef std::list<Ieee80211DataOrMgmtFrame *> Ieee80211DataOrMgmtFrameList;
    /**
     * This is used to populate fragments and identify duplicated messages. See spec 9.2.9.
     */
    struct Ieee80211ASFTuple
    {
        int sequenceNumber;
        int fragmentNumber;
        simtime_t receivedTime;
        Ieee80211ASFTuple& operator=(const Ieee80211ASFTuple& other)
        {
            if (this == &other) return *this;
            this->sequenceNumber = other.sequenceNumber;
            this->fragmentNumber = other.fragmentNumber;
            this->receivedTime = other.receivedTime;
            return *this;
        }
    };

    typedef std::map<MACAddress, Ieee80211ASFTuple> Ieee80211ASFTupleList;


    struct Ieee80211PacketErrorInfo
    {
        uint64_t Size;
        bool hasErrors;
        simtime_t timeRec;
        Ieee80211PacketErrorInfo& operator=(const Ieee80211PacketErrorInfo& other)
        {
            if (this==&other) return *this;
            this->Size = other.Size;
            this->hasErrors = other.hasErrors;
            this->timeRec = other.timeRec;
            return *this;
        }
    };

    bool registerErrors;
    typedef std::map<MACAddress, std::vector<Ieee80211PacketErrorInfo> > Ieee80211ErrorInfo;
    Ieee80211ErrorInfo errorInfo;

    enum RateControlMode
    {
        RATE_ARF,   // Auto Rate Fallback
        RATE_AARF,  // Adaptatice ARF
        RATE_CR,    // Constant Rate
    };

    RateControlMode rateControlMode = (RateControlMode)-1;

    const IIeee80211Mode *recFrameModulation = nullptr;
    bool validRecMode = false;
    bool useModulationParameters = false;
    bool prioritizeMulticast = false;

  protected:
    bool isInHandleWithFSM = false;
    IRadio::TransmissionState transmissionState = IRadio::TRANSMISSION_STATE_UNDEFINED;
    /**
     * @name Configuration parameters
     * These are filled in during the initialization phase and not supposed to change afterwards.
     */
    //@{
    /** MAC address */
    MACAddress address;
    const Ieee80211ModeSet *modeSet = nullptr;
    /** The bitrate is used to send unicast data and mgmt frames; be sure to use a valid 802.11 bitrate */
    const IIeee80211Mode *dataFrameMode = nullptr;


    /** The basic bitrate (1 or 2 Mbps) is used to transmit control frames and multicast/broadcast frames */
    const IIeee80211Mode *basicFrameMode = nullptr;
    const IIeee80211Mode *controlFrameMode = nullptr;

    // Variables used by the auto bit rate
    bool forceBitRate = false;    //if true the
    unsigned int intrateIndex = 0;
    int contI = 0;
    int contJ = 0;
    int samplingCoeff = 0;
    double recvdThroughput = NaN;
    int autoBitrate = 0;
    int rateIndex = 0;
    int successCounter = 0;
    int failedCounter = 0;
    bool recovery = false;
    int timer = 0;
    int successThreshold = 0;
    int maxSuccessThreshold = 0;
    int timerTimeout = 0;
    int minSuccessThreshold = 0;
    int minTimerTimeout = 0;
    double successCoeff = NaN;
    double timerCoeff = NaN;
    double _snr = NaN;
    double snr = NaN;
    double lossRate = NaN;
    simtime_t timeStampLastMessageReceived;
    // used to measure the throughput over a period
    uint64_t recBytesOverPeriod = 0;
    simtime_t throughputTimePeriod = 0;
    cMessage *throughputTimer = nullptr;
    double throughputLastPeriod = NaN;

    /** Maximum number of frames in the queue; should be set in the omnetpp.ini */
    int maxQueueSize = 0;

    /**
     * The minimum length of MPDU to use RTS/CTS mechanism. 0 means always, extremely
     * large value means never. See spec 9.2.6 and 361.
     */
    int rtsThreshold = 0;

    /**
     * Maximum number of transmissions for a message.
     * This includes the initial transmission and all subsequent retransmissions.
     * Thus a value 0 is invalid and a value 1 means no retransmissions.
     * See: dot11ShortRetryLimit on page 484.
     *   'This attribute shall indicate the maximum number of
     *    transmission attempts of a frame, the length of which is less
     *    than or equal to dot11RTSThreshold, that shall be made before a
     *    failure condition is indicated. The default value of this
     *    attribute shall be 7'
     */
    int transmissionLimit = 0;

    /** Default access catagory */
    int defaultAC = 0;

    /** Slot time 9us(fast slot time 802.11g only) 20us(802.11b / 802.11g backward compatible)*/
    simtime_t ST;

    double PHY_HEADER_LENGTH = NaN;
    /** Minimum contention window. */
    int cwMinData = 0;
    //int cwMin[4];

    /** Maximum contention window. */
    int cwMaxData = 0;
    // int cwMax[4];

    /** Contention window size for multicast messages. */
    int cwMinMulticast = 0;

    /** Messages longer than this threshold will be sent in multiple fragments. see spec 361 */
    static const int fragmentationThreshold = 2346;
    //@}

  public:
    /**
     * @name Ieee80211OldMac2 state variables
     * Various state information checked and modified according to the state machine.
     */
    //@{
    // don't forget to keep synchronized the C++ enum and the runtime enum definition
    /** the 80211 MAC state machine */
    enum State {
        IDLE,
        DEFER,
        WAITAIFS,
        BACKOFF,
        WAITACK,
        WAITMULTICAST,
        WAITCTS,
        WAITSIFS,
        RECEIVE,
        TRANSMIT,
    };

  protected:
    cFSM fsm;

    struct Edca
    {
        simtime_t TXOP;
        bool backoff;
        simtime_t backoffPeriod;
        int retryCounter;
        int AIFSN;    // Arbitration interframe space number. The duration edcCAF[AC].AIFSis a duration derived from the value AIFSN[AC] by the relation
        int cwMax;
        int cwMin;
        // queue
        Ieee80211DataOrMgmtFrameList transmissionQueue;
        // per class timers
        cMessage *endAIFS;
        cMessage *endBackoff;
        /** @name Statistics per Access Class*/
        //@{
        long numRetry;
        long numSentWithoutRetry;
        long numGivenUp;
        long numSent;
        long numDropped;
        long bits;
        simtime_t minjitter;
        simtime_t maxjitter;
    };

    struct EdcaOutVector
    {
        cOutVector *jitter;
        cOutVector *macDelay;
        cOutVector *throughput;
    };

    int initialBackoffExponent; // initial exponential of backoff value
    int difsSlot; // slots used to compute the difs value

    std::vector<Edca> edcCAF;
    std::vector<EdcaOutVector> edcCAFOutVector;
    //
    // methods for access to the current AC data
    //
    // methods for access to specific AC data
    virtual int& retryCounter(int i = -1);
    virtual Ieee80211DataOrMgmtFrameList *transmissionQueue(int i = -1);

    // Statistics
    virtual long& numRetry(int i = -1);
    virtual long& numSentWithoutRetry(int i = -1);
    virtual long& numGivenUp(int i = -1);
    virtual long& numSent(int i = -1);
    virtual long& numDropped(int i = -1);
    virtual long& bits(int i = -1);
    virtual simtime_t& minJitter(int i = -1);
    virtual simtime_t& maxJitter(int i = -1);
// out vectors
    virtual cOutVector *jitter(int i = -1);
    virtual cOutVector *macDelay(int i = -1);
    virtual cOutVector *throughput(int i = -1);
    inline int numCategories() const { return edcCAF.size(); }
    virtual const bool isBackoffMsg(cMessage *msg);

    const char *modeName(int mode);

    /**
     * Arbitration interframe space number.
     * The duration AIFS[AC] is a duration derived from the value AIFSN[AC] by the relation
     * AIFS[AC] = AIFSN[AC] × aSlotTime + aSIFSTime.
     * See spec 7.3.2.29
     *
     */
    //int AIFSN[4];

    /** Remaining backoff period in seconds */
    //simtime_t backoffPeriod[4];
    /** True if backoff is enabled */
    // bool backoff[4];
    /** TXOP parametr */
    //simtime_t TXOP[4];

    /**
     * Number of frame retransmission attempts, this is a simpification of
     * SLRC and SSRC, see 9.2.4 in the spec
     */
    //int retryCounter[4];

  public:
    /** 80211 MAC operation modes */
    enum Mode {
        DCF,    ///< Distributed Coordination Function
        PCF,    ///< Point Coordination Function
        EDCA,
        CSMA_ONLY
    };

  protected:
    Mode mode = (Mode)-1;

    /** Sequence number to be assigned to the next frame */
    int sequenceNumber = 0;

    /**
     * Indicates that the last frame received had bit errors in it or there was a
     * collision during receiving the frame. If this flag is set, then the MAC
     * will wait EIFS - DIFS + AIFS  instead of AIFS period of time in WAITAIFS state.
     */
    bool lastReceiveFailed = false;

    simtime_t lastEndSIFSTime = 0;

    /** True if we are in txop bursting packets. */
    bool txop = false;

    /** Indicates which queue is acite. Depends on access category. */
    int currentAC = 0;

    /** Remember currentAC. We need this to figure out internal colision. */
    int oldcurrentAC = 0;

    /** XXX Remember for which AC we wait for ACK. */
    //int ACKcurrentAC;

    IRadio *radio = nullptr;

    Ieee80211DataOrMgmtFrame *fr = nullptr;

    /**
     * A list of last sender, sequence and fragment number tuples to identify
     * duplicates, see spec 9.2.9.
     */
    bool duplicateDetect = false;
    bool purgeOldTuples = false;
    simtime_t duplicateTimeOut;
    simtime_t lastTimeDelete;
    Ieee80211ASFTupleList asfTuplesList;

    /**
     * The last change channel message received and not yet sent to the physical layer, or nullptr.
     * The message will be sent down when the state goes to IDLE or DEFER next time.
     */
    cMessage *pendingRadioConfigMsg = nullptr;
    //@}

  protected:
    /** @name Timer messages */
    //@{
    /** End of the Short Inter-Frame Time period */
    cMessage *endSIFS = nullptr;

    /** End of the Data Inter-Frame Time period */
    cMessage *endDIFS = nullptr;

    /** End of the Arbitration Inter-Frame Time period */
//    cMessage *endAIFS[4];

    /** End of the TXOP time limit period */
    cMessage *endTXOP = nullptr;

    /** End of the backoff period */
    //cMessage *endBackoff[4];

    /** Timeout after the transmission of an RTS, a CTS, or a DATA frame */
    cMessage *endTimeout = nullptr;

    /** Radio state change self message. Currently this is optimized away and sent directly */
    cMessage *mediumStateChange = nullptr;
    //@}

  protected:
    /** @name Statistics */
    //@{
    // long numRetry[4];
    // long numSentWithoutRetry[4];
    // long numGivenUp[4];
    long numCollision = 0;
    long numInternalCollision = 0;
    // long numSent[4];
    long numBits = 0;
    long numSentTXOP = 0;
    long numReceived = 0;
    long numSentMulticast = 0;
    long numReceivedMulticast = 0;
    // long numDropped[4];
    long numReceivedOther = 0;
    long numAckSend = 0;
    cOutVector stateVector;
    cOutVector numReceivedVector;
    cOutVector numReceivedMulticastVector;
    cOutVector numReceivedOtherVector;
    cOutVector numCollisionVector;
    cOutVector numCollisionMulticastVector;
    simtime_t last;
    // long bits[4];
    // simtime_t minjitter[4];
    // simtime_t maxjitter[4];
    // cOutVector jitter[4];
    // cOutVector macDelay[4];
    // cOutVector throughput[4];
    //@}

  public:
    /**
     * @name Construction functions
     */
    //@{
    Ieee80211OldMac2();
    virtual ~Ieee80211OldMac2();
    //@}

  protected:
    /**
     * @name Initialization functions
     */
    //@{
    /** @brief Initialization of the module and its variables */
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initializeCategories();
    virtual void initialize(int) override;
    virtual InterfaceEntry *createInterfaceEntry() override;
    virtual void finish() override;
    virtual void configureAutoBitRate();
    virtual void initWatches();
    virtual const MACAddress& isInterfaceRegistered();
    //@}

  protected:
    /**
     * @name Message handing functions
     * @brief Functions called from other classes to notify about state changes and to handle messages.
     */
    //@{
    /** @brief Called by the signal handler whenever a change occurs we're interested in */
    virtual void receiveSignal(cComponent *source, simsignal_t signalID, long value, cObject *details) override;

    /** @brief Handle commands (msg kind+control info) coming from upper layers */
    virtual void handleUpperCommand(cMessage *msg) override;

    /** @brief Handle timer self messages */
    virtual void handleSelfMessage(cMessage *msg) override;

    /** @brief Handle messages from upper layer */
    virtual void handleUpperPacket(cPacket *msg) override;

    /** @brief Handle messages from lower (physical) layer */
    virtual void handleLowerPacket(cPacket *msg) override;

    /** @brief Handle all kinds of messages and notifications with the state machine */
    virtual void handleWithFSM(cMessage *msg);
    //@}
    virtual bool  initFsm(cMessage *msg, bool &, Ieee80211Frame *&);
    virtual void  endFsm(cMessage *msg);
  protected:
    /**
     * @name Timing functions
     * @brief Calculate various timings based on transmission rate and physical layer charactersitics.
     */
    //@{
    //virtual simtime_t getSIFS();
    virtual simtime_t getSlotTime();
    virtual double controlFrameTxTime(int bits);
    //@}

  protected:
    /**
     * @name Timer functions
     * @brief These functions have the side effect of starting the corresponding timers.
     */
    //@{
    virtual void scheduleSIFSPeriod(Ieee80211Frame *frame);

    virtual void scheduleDataTimeoutPeriod(Ieee80211DataOrMgmtFrame *frame);
    virtual void scheduleMulticastTimeoutPeriod(Ieee80211DataOrMgmtFrame *frame);
    virtual void cancelTimeoutPeriod();


    /** @brief Schedule network allocation period according to 9.2.5.4. */
    virtual void scheduleReservePeriod(Ieee80211Frame *frame);

    /** @brief Generates a new backoff period based on the contention window. */
    virtual void invalidateBackoffPeriod();
    virtual bool isInvalidBackoffPeriod();
    virtual void finishReception();
    //@}

  protected:
    /**
     * @name Frame transmission functions
     */
    //@{
    virtual void sendDataFrameOnEndSIFS(Ieee80211DataOrMgmtFrame *frameToSend);
    virtual void sendDataFrame(Ieee80211DataOrMgmtFrame *frameToSend);
    virtual void sendMulticastFrame(Ieee80211DataOrMgmtFrame *frameToSend);
    //@}

  protected:
    /**
     * @name Frame builder functions
     */
    //@{
    virtual Ieee80211DataOrMgmtFrame *buildDataFrame(Ieee80211DataOrMgmtFrame *frameToSend);
    virtual Ieee80211DataOrMgmtFrame *buildMulticastFrame(Ieee80211DataOrMgmtFrame *frameToSend);
    //@}

    /**
     * @brief Attaches a PhyControlInfo to the frame which will cause it to be sent at
     * basicBitrate not bitrate (e.g. 2Mbps instead of 11Mbps). Used with ACK, CTS, RTS.
     */
    virtual Ieee80211Frame *setBasicBitrate(Ieee80211Frame *frame);
    virtual Ieee80211Frame *setControlBitrate(Ieee80211Frame *frame);
    virtual Ieee80211Frame *setBitrateFrame(Ieee80211Frame *frame);

  protected:
    /**
     * @name Utility functions
     */
    //@{
    virtual void finishCurrentTransmission();
    virtual void giveUpCurrentTransmission();
    virtual bool transmissionQueuesEmpty();
    virtual unsigned int getTotalQueueLength();
    virtual void flushQueue();
    virtual void clearQueue();

    int classifyFrame(Ieee80211DataOrMgmtFrame *frame);
    AccessCategory mapTidToAc(int tid);

    /** @brief Mapping to access categories. */
    virtual int mappingAccessCategory(Ieee80211DataOrMgmtFrame *frame);

    /** @brief Send down the change channel message to the physical layer if there is any. */
    virtual void sendDownPendingRadioConfigMsg();

    /** @brief Change the current MAC operation mode. */
    virtual void setMode(Mode mode);

    /** @brief Returns the current frame being transmitted */
    virtual Ieee80211DataOrMgmtFrame *getCurrentTransmission();

    /** @brief Reset backoff, backoffPeriod and retryCounter for IDLE state */
    virtual void resetStateVariables();

    /** @brief Used by the state machine to identify medium state change events.
        This message is currently optimized away and not sent through the kernel. */
    virtual bool isMediumStateChange(cMessage *msg);

    /** @brief Tells if the medium is receiving something. */
    virtual bool isMediumRecv();

    /** @brief Returns true if message is a multicast message */
    virtual bool isMulticast(Ieee80211Frame *msg);

    /** @brief Returns true if message destination address is ours */
    virtual bool isForUs(Ieee80211Frame *msg);

    /** @brief Returns true if message source address is ours */
    virtual bool isSentByUs(Ieee80211Frame *msg);

    /** @brief Checks if the frame is a data or management frame */
    virtual bool isDataOrMgmtFrame(Ieee80211Frame *frame);


    /** @brief Returns the last frame received before the SIFS period. */
    virtual Ieee80211Frame *getFrameReceivedBeforeSIFS();

    /** @brief Deletes frame at the front of queue. */
    virtual void popTransmissionQueue();

    /**
     * @brief Computes the duration (in seconds) of the transmission of a frame
     * over the physical channel. 'bits' should be the total length of the MAC frame
     * in bits, but excluding the physical layer framing (preamble etc.)
     */
    virtual double computeFrameDuration(Ieee80211Frame *msg);
    virtual double computeFrameDuration(int bits, double bitrate);

    /** @brief Logs all state information */
    virtual void logState();

    virtual void reportDataOk(void);
    virtual void reportDataFailed(void);

    virtual int getMinTimerTimeout(void);
    virtual int getMinSuccessThreshold(void);

    virtual int getTimerTimeout(void);
    virtual int getSuccessThreshold(void);

    virtual void setTimerTimeout(int timer_timeout);
    virtual void setSuccessThreshold(int success_threshold);

    virtual void reportRecoveryFailure(void);
    virtual void reportFailure(void);

    virtual bool needRecoveryFallback(void);
    virtual bool needNormalFallback(void);

    virtual double getBitrate();
    virtual void setBitrate(double b_rate);

    virtual bool isBackoffPending();

    const IIeee80211Mode *getControlAnswerMode(const IIeee80211Mode *reqMode);
    //@}

    virtual void sendUp(cMessage *msg) override;

    virtual void removeOldTuplesFromDuplicateMap();

    virtual void promiscousFrame(cMessage *msg);

    virtual bool isDuplicated(cMessage *msg);

    virtual bool handleNodeStart(IDoneCallback *doneCallback) override;

    virtual bool handleNodeShutdown(IDoneCallback *doneCallback) override;

    virtual void handleNodeCrash() override;

    virtual void sendNotification(simsignal_t category, cMessage *pkt = nullptr)
    {
        if (pkt)
        {
            int tempKind = pkt->getKind();
            pkt->setKind(this->getIndex());
            emit(category, pkt);
            pkt->setKind(tempKind);
        }
        else
            emit(category, (cObject *)nullptr);
    }

    virtual void configureRadioMode(IRadio::RadioMode radioMode);

  public:
    virtual State getState() { return static_cast<State>(fsm.getState()); }
    virtual unsigned int getQueueSize() { return getTotalQueueLength(); }
    /** @brief Tells if the medium is free according to the physical and virtual carrier sense algorithm. */
    virtual bool isMediumFree();
    virtual simtime_t getSIFS();
};

} // namespace ieee80211

} // namespace inet

#endif // ifndef __INET_IEEE80211OLDMAC_H

