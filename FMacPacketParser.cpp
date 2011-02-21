#include "FMacPacketParser.h"
#include "PacketSlots.h"

FMacPacketParser::FMacPacketParser(QObject *parent) : QObject(parent)
{
}

void FMacPacketParser::processPacket(QList<uint8_t> packet)
{
	switch (packet[NodePacket::Cmdform])
	{
	case Cmdform::Return_Max_Tier:
		emit gotMaxTier(packet[NodePacket::Deploy_Max_Tier]);
		break;

	case Cmdform::Return_Path:
		if (packet[NodePacket::Path_Sender_Tier] == 1)
			emit gotFirstTierNode();

		emit gotNodePath(packet[NodePacket::Path_Sender_Id],
						 packet[NodePacket::Path_Sender_Parent_Id]);

		if (packet[NodePacket::Path_Source_Id])
			emit gotNodePath(packet[NodePacket::Path_Source_Id],
							 packet[NodePacket::Path_Source_Parent_Id],
							 true);
		break;

	case Cmdform::Return_Data:
		emit gotData(getDataOfPacket(packet));
		break;

	case Cmdform::Supp_Return_Data:
		emit gotData(getDataOfPacket(packet), true);
		break;

	case Cmdform::Awake:
		emit hadAwakeNode();
		break;

	case Cmdform::Request_Reroute:
		emit shouldReroute();
		break;
	}
}

NodeData FMacPacketParser::getDataOfPacket(QList<uint8_t> packet)
{
	NodeData data;
	data.senderNodeId = packet[NodePacket::Data_Sender_Id];
	data.dataSourceNodeId = packet[NodePacket::Data_Source_Id];
	data.dataSourceTier = packet[NodePacket::Data_Source_Tier];
	data.temperature = packet[NodePacket::Data_Temperature_Hi] * 0x100 +
					   packet[NodePacket::Data_Temperature_Lo];
	data.humidity = packet[NodePacket::Data_Humidity_Hi] * 0x100 +
					packet[NodePacket::Data_Humidity_Lo];
	data.pest = packet[NodePacket::Data_Pest_Hi] * 0x100 +
				packet[NodePacket::Data_Pest_Lo];
	data.par = packet[NodePacket::Data_Par_Hi_Hi] * 0x1000000 +
			   packet[NodePacket::Data_Par_Hi_Lo] * 0x10000 +
			   packet[NodePacket::Data_Par_Lo_Hi] * 0x100 +
			   packet[NodePacket::Data_Par_Lo_Lo];

	return data;
}
