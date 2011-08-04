#ifndef PACKETSLOTS_H
#define PACKETSLOTS_H

namespace GatewayPacket
{
	enum GatewayPacket
	{
		Group_Id           = 11,
		Cmdform            ,
		Receiver           = 14,
		Depth              ,
		Sleep_Time_Lo      = 22,
		Sleep_Time_Hi      = 23,
	};
}

namespace NodePacket
{
	enum NodePacket
	{
		Group_Id                  = 10,
                Cmdform                   ,
		Data_Sender_Id            ,
		Data_Source_Id            ,
		Data_Source_Tier          ,
		Data_Par_Hi_Lo            ,
		Data_Par_Hi_Hi            ,
		Data_Temperature_Lo       ,
		Data_Temperature_Hi       ,
		Data_Humidity_Lo          ,
		Data_Humidity_Hi          ,
		Data_Pest_Lo              ,
		Data_Pest_Hi              ,
		Data_Par_Lo_Lo            ,
                Data_Par_Lo_Hi            ,
		/*                        */
		Deploy_Max_Tier           = 15,
		/*                        */
		Path_Sender_Tier          = 13,
		Path_Sender_Id            ,
		Path_Sender_Parent_Id     ,
		Path_Source_Id            = 19,
		Path_Source_Parent_Id     = 21,
	};
}

namespace Cmdform
{
	enum Cmdform
	{
		Return_Max_Tier		= 0x01,
		Return_Path			= 0x03,
		Return_Data			= 0x21,
		Supp_Return_Data		= 0x49,
		Awake				= 0x90,
		Request_Reroute		= 0x91,
	};
}

#endif // PACKETSLOTS_H
