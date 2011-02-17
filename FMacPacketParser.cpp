#include "FMacPacketParser.h"
#include "PacketSlots.h"

FMacPacketParser::FMacPacketParser(QObject *parent) : QObject(parent)
{
}

void FMacPacketParser::processPacket(QList<uint8_t> packet)
{
	switch (packet[NODE_PACKET_CMDFORM])
	{
	case CMD_RETURN_MAX_TIER:
		emit gotMaxTier(packet[NODE_PACKET_DEPLOY_MAX_TIER]);
		break;

	case CMD_RETURN_PATH:
		if (packet[NODE_PACKET_PATH_SENDER_TIER] == 1)
			emit gotFirstTierNode();

		emit gotNodePath(packet[NODE_PACKET_PATH_SENDER_ID],
						 packet[NODE_PACKET_PATH_SENDER_PARENT_ID]);

		if (packet[NODE_PACKET_PATH_SOURCE_ID])
			emit gotNodePath(packet[NODE_PACKET_PATH_SOURCE_ID],
							 packet[NODE_PACKET_PATH_SOURCE_PARENT_ID],
							 true);
		break;

	case CMD_RETURN_DATA:
		emit gotData(getDataOfPacket(packet));
		break;

	case CMD_SUPP_RETURN_DATA:
		emit gotData(getDataOfPacket(packet), true);
		break;

	case CMD_NODE_AWAKE:
		emit hadAwakeNode();
		break;

	case CMD_REQUEST_REROUTE:
		emit shouldReroute();
		break;
	}
}

NodeData FMacPacketParser::getDataOfPacket(QList<uint8_t> packet)
{
	NodeData data;
	data.senderNodeId = packet[NODE_PACKET_DATA_SENDER_ID];
	data.dataSourceNodeId = packet[NODE_PACKET_DATA_SOURCE_ID];
	data.dataSourceTier = packet[NODE_PACKET_DATA_SOURCE_TIER];
	data.temperature = packet[NODE_PACKET_DATA_TEMPERATURE_HI] * 0x100 +
					   packet[NODE_PACKET_DATA_TEMPERATURE_LO];
	data.humidity = packet[NODE_PACKET_DATA_HUMIDITY_HI] * 0x100 +
					packet[NODE_PACKET_DATA_HUMIDITY_LO];
	data.pest = packet[NODE_PACKET_DATA_PEST_HI] * 0x100 +
				packet[NODE_PACKET_DATA_PEST_LO];
	data.par = packet[NODE_PACKET_DATA_PAR_HI_HI] * 0x1000000 +
			   packet[NODE_PACKET_DATA_PAR_HI_LO] * 0x10000 +
			   packet[NODE_PACKET_DATA_PAR_LO_HI] * 0x100 +
			   packet[NODE_PACKET_DATA_PAR_LO_LO];

	return data;
}
