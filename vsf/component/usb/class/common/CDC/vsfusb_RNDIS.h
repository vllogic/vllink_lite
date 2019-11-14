/***************************************************************************
 *   Copyright (C) 2009 - 2010 by Simon Qian <SimonQian@SimonQian.com>     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#ifndef __VSFUSB_RNDIS_H_INCLUDED__
#define __VSFUSB_RNDIS_H_INCLUDED__

enum rndis_ctrlmsg_t
{
	RNDIS_PACKET_MSG									= 0x00000001,
	RNDIS_INITIALIZE_MSG								= 0x00000002,
	RNDIS_INITIALIZE_CMPLT								= 0x80000002,
	RNDIS_HALT_MSG										= 0x00000003,
	RNDIS_QUERY_MSG										= 0x00000004,
	RNDIS_QUERY_CMPLT									= 0x80000004,
	RNDIS_SET_MSG										= 0x00000005,
	RNDIS_SET_CMPLT										= 0x80000005,
	RNDIS_RESET_MSG										= 0x00000006,
	RNDIS_RESET_CMPLT									= 0x80000006,
	RNDIS_INDICATE_STATUS_MSG							= 0x00000007,
	RNDIS_KEEPALIVE_MSG									= 0x00000008,
	RNDIS_KEEPALIVE_CMPLT								= 0x80000008,
};

// OIDs
enum oid_t
{
	// General OIDs
	OID_GEN_SUPPORTED_LIST								= 0x00010101,
	OID_GEN_HARDWARE_STATUS								= 0x00010102,
	OID_GEN_MEDIA_SUPPORTED								= 0x00010103,
	OID_GEN_MEDIA_IN_USE								= 0x00010104,
	OID_GEN_MAXIMUM_LOOKAHEAD							= 0x00010105,
	OID_GEN_MAXIMUM_FRAME_SIZE							= 0x00010106,
	OID_GEN_LINK_SPEED									= 0x00010107,
	OID_GEN_TRANSMIT_BLOCK_SIZE							= 0x00010108,
	OID_GEN_RECEIVE_BLOCK_SIZE							= 0x00010109,
	OID_GEN_VENDOR_ID									= 0x0001010C,
	OID_GEN_VENDOR_DESCRIPTION							= 0x0001010D,
	OID_GEN_CURRENT_PACKET_FILTER						= 0x0001010E,
	OID_GEN_CURRENT_LOOKAHEAD							= 0x0001010F,
	OID_GEN_DRIVER_VERSION								= 0x00010110,
	OID_GEN_MAXIMUM_TOTAL_SIZE							= 0x00010111,
	OID_GEN_PROTOCOL_OPTIONS							= 0x00010112,
	OID_GEN_MAC_OPTIONS									= 0x00010113,
	OID_GEN_MEDIA_CONNECT_STATUS						= 0x00010114,
	OID_GEN_MAXIMUM_SEND_PACKETS						= 0x00010115,
	OID_GEN_VENDOR_DRIVER_VERSION						= 0x00010116,
	OID_GEN_SUPPORTED_GUIDS								= 0x00010117,
	OID_GEN_NETWORK_LAYER_ADDRESSES						= 0x00010118,
	OID_GEN_TRANSPORT_HEADER_OFFSET						= 0x00010119,
	OID_GEN_MACHINE_NAME								= 0x0001021A,
	OID_GEN_RNDIS_CONFIG_PARAMETER						= 0x0001021B,
	OID_GEN_VLAN_ID										= 0x0001021C,
	// Option OIDs
	OID_GEN_MEDIA_CAPABILITIES							= 0x00010201,
	OID_GEN_PHYSICAL_MEDIUM								= 0x00010202,
	// Required Statistics OIDs
	OID_GEN_XMIT_OK										= 0x00020101,
	OID_GEN_RCV_OK										= 0x00020102,
	OID_GEN_XMIT_ERROR									= 0x00020103,
	OID_GEN_RCV_ERROR									= 0x00020104,
	OID_GEN_RCV_NO_BUFFER								= 0x00020105,
	// Optional Statistics OIDs
	OID_GEN_DIRECTED_BYTES_XMIT							= 0x00020201,
	OID_GEN_DIRECTED_FRAMES_XMIT						= 0x00020202,
	OID_GEN_MULTICAST_BYTES_XMIT						= 0x00020203,
	OID_GEN_MULTICAST_FRAMES_XMIT						= 0x00020204,
	OID_GEN_BROADCAST_BYTES_XMIT						= 0x00020205,
	OID_GEN_BROADCAST_FRAMES_XMIT						= 0x00020206,
	OID_GEN_DIRECTED_BYTES_RCV							= 0x00020207,
	OID_GEN_DIRECTED_FRAMES_RCV							= 0x00020208,
	OID_GEN_MULTICAST_BYTES_RCV							= 0x00020209,
	OID_GEN_MULTICAST_FRAMES_RCV						= 0x0002020A,
	OID_GEN_BROADCAST_BYTES_RCV							= 0x0002020B,
	OID_GEN_BROADCAST_FRAMES_RCV						= 0x0002020C,
	OID_GEN_RCV_CRC_ERROR								= 0x0002020D,
	OID_GEN_TRANSMIT_QUEUE_LENGTH						= 0x0002020E,
	OID_GEN_GET_TIME_CAPS								= 0x0002020F,
	OID_GEN_GET_NETCARD_LOAD							= 0x00020210,
	OID_GEN_NETCARD_LOAD								= 0x00020211,
	OID_GEN_DEVICE_PROFILE								= 0x00020212,
	OID_GEN_INIT_TIME_MS								= 0x00020213,
	OID_GEN_RESET_COUNTS								= 0x00020214,
	OID_GEN_MEDIA_SENSE_COUNTS							= 0x00020215,
	OID_GEN_FRIENDLY_NAME								= 0x00020216,
	OID_GEN_MINIPORT_INFO								= 0x00020217,
	OID_GEN_RESET_VERIFY_PARAMETERS						= 0x00020218,
	
	// 802.3 OIDs
	OID_802_3_PERMANENT_ADDRESS							= 0x01010101,
	OID_802_3_CURRENT_ADDRESS							= 0x01010102,
	OID_802_3_MULTICAST_LIST							= 0x01010103,
	OID_802_3_MAXIMUM_LIST_SIZE							= 0x01010104,
	OID_802_3_MAC_OPTIONS								= 0x01010105,
	// 802.3 Statistic OIDs
	OID_802_3_RCV_ERROR_ALIGNMENT						= 0x01020101,
	OID_802_3_XMIT_ONE_COLLISION						= 0x01020102,
	OID_802_3_XMIT_MORE_COLLISION						= 0x01020103,
	OID_802_3_XMIT_DEFERRED								= 0x01020201,
	OID_802_3_XMIT_MAX_COLLISIONS						= 0x01020202,
	OID_802_3_RCV_OVERRUN								= 0x01020203,
	OID_802_3_XMIT_UNDERRUN								= 0x01020204,
	OID_802_3_XMIT_HEARTBEAT_FAILURE					= 0x01020205,
	OID_802_3_XMIT_TIMES_CRS_LOST						= 0x01020206,
	OID_802_3_XMIT_LATE_COLLISIONS						= 0x01020207,

	// Wireless LAN OIDs																															
	OID_802_11_BSSID									= 0x0D010101,
	OID_802_11_SSID										= 0x0D010102,
	OID_802_11_INFRASTRUCTURE_MODE						= 0x0D010108,
	OID_802_11_ADD_WEP									= 0x0D010113,
	OID_802_11_REMOVE_WEP								= 0x0D010114,
	OID_802_11_DISASSOCIATE								= 0x0D010115,
	OID_802_11_AUTHENTICATION_MODE						= 0x0D010118,
	OID_802_11_BSSID_LIST_SCAN							= 0x0D01011A,
	OID_802_11_WEP_STATUS								= 0x0D01011B,
	OID_802_11_RELOAD_DEFAULTS							= 0x0D01011C,
	OID_802_11_NETWORK_TYPE_IN_USE						= 0x0D010204,
	OID_802_11_RSSI										= 0x0D010206,
	OID_802_11_SUPPORTED_RATES							= 0x0D01020E,
	OID_802_11_CONFIGURATION							= 0x0D010211,
	OID_802_11_BSSID_LIST								= 0x0D010217,

	// NDIS TCP/IP Offload OIDs
/*	OID_TCP_CONNECTION_OFFLOAD_CURRENT_CONFIG			= ,
	OID_TCP_CONNECTION_OFFLOAD_HARDWARE_CAPABILITIES	= ,
	OID_TCP_CONNECTION_OFFLOAD_PARAMETERS				= ,
	OID_TCP_OFFLOAD_CURRENT_CONFIG						= ,
	OID_TCP_OFFLOAD_HARDWARE_CAPABILITIES				= ,
	OID_TCP_OFFLOAD_PARAMETERS							= ,
	OID_OFFLOAD_ENCAPSULATION							= ,

	// Power Management OIDs
	OID_PM_ADD_PROTOCOL_OFFLOAD							= ,
	OID_PM_ADD_WOL_PATTERN								= ,
	OID_PM_CURRENT_CAPABILITIES							= ,
	OID_PM_GET_PROTOCOL_OFFLOAD							= ,
	OID_PM_HARDWARE_CAPABILITIES						= ,
	OID_PM_PARAMETERS									= ,
	OID_PM_PROTOCOL_OFFLOAD_LIST						= ,
	OID_PM_REMOVE_PROTOCOL_OFFLOAD						= ,
	OID_PM_REMOVE_WOL_PATTERN							= ,
	OID_PM_WOL_PATTERN_LIST								= ,
*/	// Optional Power Management OIDs
	OID_PNP_CAPABILITIES								= 0xFD010100,
	OID_PNP_SET_POWER									= 0xFD010101,
	OID_PNP_QUERY_POWER									= 0xFD010102,
	// Option Network Wake Up IODs
	OID_PNP_ADD_WAKE_UP_PATTERN							= 0xFD010103,
	OID_PNP_REMOVE_WAKE_UP_PATTERN						= 0xFD010104,
	OID_PNP_ENABLE_WAKE_UP								= 0xFD010106,
};

enum ndis_status_t
{
	NDIS_STATUS_SUCCESS									= 0x00000000,
	NDIS_STATUS_FAILURE									= 0xC0000001,
	NDIS_STATUS_MULTICAST_FULL							= 0xC0010009,
	NDIS_STATUS_MULTICAST_EXISTS						= 0xC001000A,
	NDIS_STATUS_MULTICAST_NOT_FOUND						= 0xC001000B,
	NDIS_STATUS_INVALID_DATA							= 0xC0010015,
	NDIS_STATUS_NOT_SUPPORTED							= 0xC00000BB,
	NDIS_STATUS_MEDIA_CONNECT							= 0x4001000B,
	NDIS_STATUS_MEDIA_DISCONNECT						= 0x4001000C,
};

enum ndis_miniport_t
{
	NDIS_MINIPORT_BUS_MASTER							= 0x00000001,
	NDIS_MINIPORT_WDM_DRIVER							= 0x00000002,
	NDIS_MINIPORT_SG_LIST								= 0x00000004,
	NDIS_MINIPORT_SUPPORTS_MEDIA_QUERY					= 0x00000008,
	NDIS_MINIPORT_INDICATES_PACKETS						= 0x00000010,
	NDIS_MINIPORT_IGNORE_PACKET_QUEUE					= 0x00000020,
	NDIS_MINIPORT_IGNORE_REQUEST_QUEUE					= 0x00000040,
	NDIS_MINIPORT_IGNORE_TOKEN_RING_ERRORS				= 0x00000080,
	NDIS_MINIPORT_INTERMEDIATED_DRIVER					= 0x00000100,
	NDIS_MINIPORT_IS_NDIS_5								= 0x00000200,
	NDIS_MINIPORT_IS_CO									= 0x00000400,
	NDIS_MINIPORT_DESERIALIZE							= 0x00000800,
	NDIS_MINIPORT_REQUIRES_MEDIA_POLLING				= 0x00001000,
	NDIS_MINIPORT_SUPPORTS_MEDIA_SENSE					= 0x00002000,
	NDIS_MINIPORT_NETBOOT_CARD							= 0x00004000,
	NDIS_MINIPORT_PM_SUPPORTED							= 0x00008000,
	NDIS_MINIPORT_SUPPORTS_MAC_ADDRESS_OVERWRITE		= 0x00010000,
	NDIS_MINIPORT_USES_SAFE_BUFFER_APIS					= 0x00020000,
	NDIS_MINIPORT_HIDDEN								= 0x00040000,
	NDIS_MINIPORT_SWENUM								= 0x00080000,
	NDIS_MINIPORT_SURPRISE_REMOVE_OK					= 0x00100000,
	NDIS_MINIPORT_NO_HALT_ON_SUSPEND					= 0x00200000,
	NDIS_MINIPORT_HARDWARE_DEVICE						= 0x00400000,
	NDIS_MINIPORT_SUPPORTS_CANCEL_SEND_PACKETS			= 0x00800000,
	NDIS_MINIPORT_64BITS_DMA							= 0x01000000,
};

enum ndis_medium_t
{
	NDIS_MEDIUM_802_3									= 0x00000000,
	NDIS_MEDIUM_802_5									= 0x00000001,
	NDIS_MEDIUM_FDDI									= 0x00000002,
	NDIS_MEDIUM_WAN										= 0x00000003,
	NDIS_MEDIUM_LOCAL_TALK								= 0x00000004,
	NDIS_MEDIUM_DIX										= 0x00000005,
	NDIS_MEDIUM_ARCENT_RAW								= 0x00000006,
	NDIS_MEDIUM_ARCENT_878_2							= 0x00000007,
	NDIS_MEDIUM_ATM										= 0x00000008,
	NDIS_MEDIUM_WIRELESS_LAN							= 0x00000009,
	NDIS_MEDIUM_IRDA									= 0x0000000A,
	NDIS_MEDIUM_BPC										= 0x0000000B,
	NDIS_MEDIUM_CO_WAN									= 0x0000000C,
	NDIS_MEDIUM_1394									= 0x0000000D,
};

enum ndis_packet_type_t
{
	NDIS_PACKET_TYPE_DIRECTED							= 0x00000001,
	NDIS_PACKET_TYPE_MULTICAST							= 0x00000002,
	NDIS_PACKET_TYPE_ALL_MULTICAST						= 0x00000004,
	NDIS_PACKET_TYPE_BROADCAST							= 0x00000008,
	NDIS_PACKET_TYPE_SOURCE_ROUTING						= 0x00000010,
	NDIS_PACKET_TYPE_PROMISCUOUS						= 0x00000020,
	NDIS_PACKET_TYPE_SMT								= 0x00000040,
	NDIS_PACKET_TYPE_ALL_LOCAL							= 0x00000080,
	NDIS_PACKET_TYPE_GROUP								= 0x00000100,
	NDIS_PACKET_TYPE_ALL_FUNCTIONAL						= 0x00000200,
	NDIS_PACKET_TYPE_FUNCTIONAL							= 0x00000400,
	NDIS_PACKET_TYPE_MAC_FRAME							= 0x00000800,
};

enum ndis_media_state_t
{
	NDIS_MEDIA_STATE_CONNECTED							= 0x00000000,
	NDIS_MEDIA_STATE_DISCONNECTED						= 0x00000001,
};

enum ndis_mac_option_t
{
	NDIS_MAC_OPTION_COPY_LOOKAHEAD_DATA					= 0x00000001,
	NDIS_MAC_OPTION_RECEIVE_SERIALIZED					= 0x00000002,
	NDIS_MAC_OPTION_TRANSFERS_NOT_PEND					= 0x00000004,
	NDIS_MAC_OPTION_NO_LOOPBACK							= 0x00000008,
	NDIS_MAC_OPTION_FULL_DUPLEX							= 0x00000010,
	NDIS_MAC_OPTION_EOTX_INDICATION						= 0x00000020,
	NDIS_MAC_OPTION_8021P_PRIORITY						= 0x00000040,
	NDIS_MAC_OPTION_OPTION_RESERVED						= 0x80000000,
};

#define RNDIS_MAJOR_VERSION								1
#define RNDIS_MINOR_VERSION								0

struct rndis_config_parameter_t
{
	uint32_t ParameterNameOffset;
	uint32_t ParameterNameLength;
	union
	{
		enum
		{
			RNDIS_CONFIG_PARAM_TYPE_STRING				= 2,
			RNDIS_CONFIG_PARAM_TYPE_NUMERICAL			= 0,
		} type;
		uint32_t value;
	} ParameterType;
	uint32_t ParameterValueOffset;
	uint32_t ParameterValueLength;
};

struct rndis_msghead_t
{
	union
	{
		enum rndis_ctrlmsg_t msg;
		uint32_t value;
	} MessageType;
	uint32_t MessageLength;
};

struct rndis_requesthead_t
{
	struct rndis_msghead_t head;
	uint32_t RequestId;
};

struct rndis_replyhead_t
{
	struct rndis_requesthead_t request;
	union
	{
		enum ndis_status_t status;
		uint32_t value;
	} status;
};

struct rndis_initialize_msg_t
{
	struct rndis_requesthead_t request;
	uint32_t MajorVersion;
	uint32_t MinorVersion;
	uint32_t MaxTransferSize;
};

struct rndis_initialize_cmplt_t
{
	struct rndis_replyhead_t reply;
	uint32_t MajorVersion;
	uint32_t MinorVersion;
	union
	{
		enum
		{
			RNDIS_DF_CONNECTIONLESS						= 0x00000001,
			RNDIS_DF_CONNECTION_ORIENTED				= 0x00000002,
		} flags;
		uint32_t value;
	} DeviceFlags;
	union
	{
		enum ndis_medium_t medium;
		uint32_t value;
	} Medium;
	uint32_t MaxPacketsPerMessage;
	uint32_t MaxTransferSize;
	uint32_t PacketAlignmentFactor;
	uint32_t AFListOffset;
	uint32_t AFListSize;
};

struct rndis_halt_msg_t
{
	struct rndis_requesthead_t request;
};

struct rndis_query_msg_t
{
	struct rndis_requesthead_t request;
	union
	{
		enum oid_t oid;
		uint32_t value;
	} Oid;
	uint32_t InformationBufferLength;
	uint32_t InformationBufferOffset;
	uint32_t DeviceVcHandle;
};

struct rndis_query_cmplt_t
{
	struct rndis_replyhead_t reply;
	uint32_t InformationBufferLength;
	uint32_t InformationBufferOffset;
};

struct rndis_set_msg_t
{
	struct rndis_requesthead_t request;
	union
	{
		enum oid_t oid;
		uint32_t value;
	} Oid;
	uint32_t InformationBufferLength;
	uint32_t InformationBufferOffset;
	uint32_t DeviceVcHandle;
};

struct rndis_set_cmplt_t
{
	struct rndis_replyhead_t reply;
};

struct rndis_reset_msg_t
{
	struct rndis_msghead_t head;
	uint32_t Reserved;
};

struct rndis_reset_cmplt_t
{
	struct rndis_msghead_t head;
	union
	{
		enum ndis_status_t status;
		uint32_t value;
	} status;
	uint32_t AddressingReset;
};

struct rndis_indicate_status_msg_t
{
	struct rndis_msghead_t head;
	union
	{
		enum ndis_status_t status;
		uint32_t value;
	} status;
	uint32_t StatusBufferLength;
	uint32_t StatusBufferOffset;
};

struct rndis_keepalive_msg_t
{
	struct rndis_requesthead_t request;
};

struct rndis_keepalive_cmplt_t
{
	struct rndis_replyhead_t reply;
};

struct rndis_oobhead_t
{
	uint32_t Size;
	uint32_t Type;
	uint32_t ClassInformationOffset;
};

// per packet information head
struct rndis_ppihead_t
{
	uint32_t Size;
	uint32_t Type;
	uint32_t PerPacketInformationOffset;
};

struct rndis_data_packet_t
{
	struct rndis_msghead_t head;
	uint32_t DataOffset;
	uint32_t DataLength;
	uint32_t OOBDataOffset;
	uint32_t OOBDataLength;
	uint32_t NumOOBDataElements;
	uint32_t PerPacketInfoOffset;
	uint32_t PerPacketInfoLength;
	uint32_t VcHandle;
	uint32_t Reserved;
};

#endif	// __VSFUSB_RNDIS_H_INCLUDED__
