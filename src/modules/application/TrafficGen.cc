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
#include "TrafficGen.h"

#include <cassert>

#include "NetwControlInfo.h"
#include "Packet.h"
#include "BaseNetwLayer.h"
#include "FindModule.h"
#include "BaseWorldUtility.h"
#include "ApplPkt_m.h"
#include "SimpleAddress.h"

Define_Module(TrafficGen);

void TrafficGen::initialize(int stage)
{
	BaseApplLayer::initialize(stage);

	// Initialize signal with  name "received"
	receivedSignalId = registerSignal("received");

	if(stage == 0) {
		world           = FindModule<BaseWorldUtility*>::findGlobalModule();
		delayTimer      = new cMessage("delay-timer", SEND_PACKET_TIMER);

		packetTime      = par("packetTime");
		pppt            = par("packetsPerPacketTime");
		burstSize       = par("burstSize");

		usePoissonArrival = par("usePoissonArrival");
		mu              = par("mu");

		nbPacketDropped = 0;
		nbPacketReceived = 0;
		nbPacketGenerated = 0;
	} else if (stage == 1) {
		if(burstSize > 0) {
			remainingBurst = burstSize;

            if (usePoissonArrival) {
                scheduleAt(exponential(mu), delayTimer);
            } else {
                scheduleAt(dblrand() * packetTime * burstSize / pppt, delayTimer);
            }
		}
	} else {

	}
}

TrafficGen::~TrafficGen() {
	cancelAndDelete(delayTimer);
}

void TrafficGen::finish()
{
//	recordScalar("dropped", nbPacketDropped);
//    recordScalar("Received", nbPacketReceived);
//    recordScalar("Generaed", nbPacketGenerated);
}

void TrafficGen::handleSelfMsg(cMessage *msg)
{
	switch( msg->getKind() )
	{
	case SEND_PACKET_TIMER:
		assert(msg == delayTimer);

		sendBroadcast();

		remainingBurst--;

		if(remainingBurst == 0) {
			remainingBurst = burstSize;
            if (usePoissonArrival) {
                scheduleAt(simTime() + exponential(par("mu").doubleValue()), msg);
            } else {
                scheduleAt(simTime() + (dblrand()*1.4+0.3)*packetTime * burstSize / pppt, msg);
            }
		} else {
			scheduleAt(simTime() + packetTime * 2, msg);
		}

		break;
	default:
		EV << "Unkown selfmessage! -> delete, kind: "<<msg->getKind() <<endl;
		delete msg;
		break;
	}
}


void TrafficGen::handleLowerMsg(cMessage *msg)
{
	cPacket* pkt = static_cast<cPacket*>(msg);

	Packet p(pkt->getBitLength(), 1, 0);
	p.setPacketSent(false);
	emit(BaseLayer::catPacketSignal, &p);

	emit(receivedSignalId, msg->getControlInfo()->getName());

	nbPacketReceived++;

	delete msg;
	msg = 0;
}


void TrafficGen::sendBroadcast()
{
	ApplPkt *pkt = new ApplPkt("BROADCAST_MESSAGE", TRAFFIC_GEN_PACKET);
	pkt->setDestAddr(LAddress::L3BROADCAST);
	// we use the host modules getIndex() as a appl address
	pkt->setSrcAddr( myApplAddr() );
	pkt->setBitLength(headerLength);

	// set the control info to tell the network layer the layer 3
	// address;
	NetwControlInfo::setControlInfo(pkt, LAddress::L3BROADCAST);

	debugEV << "Sending broadcast packet!" << endl;
	sendDown( pkt );

	nbPacketGenerated++;

	Packet p(pkt->getBitLength(), 0, 1);
	emit(BaseLayer::catPacketSignal, &p);
}

