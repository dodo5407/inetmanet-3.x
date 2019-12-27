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

#ifndef __INET_IEEE80211MGMTADHOCFORFREQHOP_H
#define __INET_IEEE80211MGMTADHOCFORFREQHOP_H

#include "inet/common/INETDefs.h"

#include "inet/linklayer/ieee80211/mgmt/Ieee80211MgmtBase.h"
#include "inet/linklayer/ieee80211/mac/Ieee80211Frame_m.h"
#include "inet/linklayer/ieee80211/mgmt/Ieee80211FragBuf.h"
#include "inet/linklayer/ieee80211/oldmac/Ieee80211OldMac2.h"
#include "inet/physicallayer/common/packetlevel/Radio.h"

namespace inet {

namespace ieee80211 {

/**
 * Used in 802.11 ad-hoc mode. See corresponding NED file for a detailed description.
 * This implementation ignores many details.
 *
 * @author Andras Varga
 */
class INET_API Ieee80211MgmtAdhocforFreqHop : public Ieee80211MgmtBase, public cListener
{

  public:
    static simsignal_t macTrasmissionFinishedSignal;

  protected:

    typedef std::pair<cPacket *, int> PacketFragPair;
    typedef std::list<PacketFragPair *> Ieee80211PacketList;

    int numMac;
    int frameId = 0;

    simtime_t fragmentTimeoutTime;
    simtime_t lastCheckTime;
    Ieee80211FragBuf fragbuf;

    bool *channelBusyState;
    simtime_t lastCheckChannelFreeTime = SIMTIME_ZERO;
    Ieee80211PacketList waitToSendQueue;
    int numFrameInMac;
    int maxMultiOutChannel;

    Ieee80211OldMac2 *macModule = nullptr;
    Radio *radioModule = nullptr;
    cMessage *endSIFS;

  public:
    Ieee80211MgmtAdhocforFreqHop();
    ~Ieee80211MgmtAdhocforFreqHop();

  protected:
    virtual int numInitStages() const override { return NUM_INIT_STAGES; }
    virtual void initialize(int) override;
    virtual void handleMessage(cMessage *msg) override;

    int  checkFreeChannel(void);
    void changeChannel(int channelNum, int gateindex);
    void changeFrequency(Hz carrierFrequency, int gateindex);

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleTimer(cMessage *msg) override;

    /** Implements abstract Ieee80211MgmtBase method */
    virtual void handleUpperMessage(cPacket *msg) override;

    virtual void receiveSignal(cComponent *source, simsignal_t signalID, cObject *obj, cObject *details) override;

    /** Implements abstract Ieee80211MgmtBase method -- throws an error (no commands supported) */
    virtual void handleCommand(int msgkind, cObject *ctrl) override;

    /** Utility function for handleUpperMessage() */
    virtual Ieee80211DataFrameWithSNAP *encapsulate(cPacket *msg);

    /** Utility method to decapsulate a data frame */
    virtual cPacket *decapsulate(Ieee80211DataFrame *frame);

    void fragmentAndSend(Ieee80211DataFrameWithSNAP *dataframe);

    void sendFromFreeChannel(cPacket *dataframe);
    void sendFromFreeChannel(cPacket *dataframe, int fragnum);

    void reassembleAndDeliver(Ieee80211DataFrameWithSNAP *frame);

    void reassembleAndDeliverFinish(Ieee80211DataFrame *frame);

    /** @name Processing of different frame types */
    //@{
    virtual void handleDataFrame(Ieee80211DataFrame *frame) override;
    virtual void handleAuthenticationFrame(Ieee80211AuthenticationFrame *frame) override;
    virtual void handleDeauthenticationFrame(Ieee80211DeauthenticationFrame *frame) override;
    virtual void handleAssociationRequestFrame(Ieee80211AssociationRequestFrame *frame) override;
    virtual void handleAssociationResponseFrame(Ieee80211AssociationResponseFrame *frame) override;
    virtual void handleReassociationRequestFrame(Ieee80211ReassociationRequestFrame *frame) override;
    virtual void handleReassociationResponseFrame(Ieee80211ReassociationResponseFrame *frame) override;
    virtual void handleDisassociationFrame(Ieee80211DisassociationFrame *frame) override;
    virtual void handleBeaconFrame(Ieee80211BeaconFrame *frame) override;
    virtual void handleProbeRequestFrame(Ieee80211ProbeRequestFrame *frame) override;
    virtual void handleProbeResponseFrame(Ieee80211ProbeResponseFrame *frame) override;
    //@}
};

} // namespace ieee80211

} // namespace inet

#endif // ifndef __INET_IEEE80211MGMTADHOC_H

