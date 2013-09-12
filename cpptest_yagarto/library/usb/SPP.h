/* Copyright (C) 2012 Kristian Lauszus, TKJ Electronics. All rights reserved.

 This software may be distributed and modified under the terms of the GNU
 General Public License version 2 (GPL2) as published by the Free Software
 Foundation and appearing in the file GPL2.TXT included in the packaging of
 this file. Please note that GPL2 Section 2[b] requires that all works based
 on this software must also be made publicly available under the terms of
 the GPL2 ("Copyleft").

 Contact information
 -------------------

 Kristian Lauszus, TKJ Electronics
 Web      :  http://www.tkjelectronics.com
 e-mail   :  kristianl@tkjelectronics.com
 */

#ifndef _spp_h_
#define _spp_h_

#include "BTD.h"

/* Bluetooth L2CAP states for SDP_task() */
#define L2CAP_SDP_WAIT                  0
#define L2CAP_SDP_REQUEST               1
#define L2CAP_SDP_SUCCESS               2
#define L2CAP_SDP_DONE                  3
#define L2CAP_DISCONNECT_RESPONSE       4

/* Bluetooth L2CAP states for RFCOMM_task() */
#define L2CAP_RFCOMM_WAIT               0
#define L2CAP_RFCOMM_REQUEST            1
#define L2CAP_RFCOMM_SUCCESS            2
#define L2CAP_RFCOMM_DONE               3

/* L2CAP event flags */
#define L2CAP_FLAG_CONNECTION_SDP_REQUEST       0x001
#define L2CAP_FLAG_CONNECTION_RFCOMM_REQUEST    0x002
#define L2CAP_FLAG_CONFIG_SDP_REQUEST           0x004
#define L2CAP_FLAG_CONFIG_RFCOMM_REQUEST        0x008
#define L2CAP_FLAG_CONFIG_SDP_SUCCESS           0x010
#define L2CAP_FLAG_CONFIG_RFCOMM_SUCCESS        0x020
#define L2CAP_FLAG_DISCONNECT_SDP_REQUEST       0x040
#define L2CAP_FLAG_DISCONNECT_RFCOMM_REQUEST    0x080
#define L2CAP_FLAG_DISCONNECT_RESPONSE          0x100

/* Macros for L2CAP event flag tests */
#define l2cap_connection_request_sdp_flag (l2cap_event_flag & L2CAP_FLAG_CONNECTION_SDP_REQUEST)
#define l2cap_connection_request_rfcomm_flag (l2cap_event_flag & L2CAP_FLAG_CONNECTION_RFCOMM_REQUEST)
#define l2cap_config_request_sdp_flag (l2cap_event_flag & L2CAP_FLAG_CONFIG_SDP_REQUEST)
#define l2cap_config_request_rfcomm_flag (l2cap_event_flag & L2CAP_FLAG_CONFIG_RFCOMM_REQUEST)
#define l2cap_config_success_sdp_flag (l2cap_event_flag & L2CAP_FLAG_CONFIG_SDP_SUCCESS)
#define l2cap_config_success_rfcomm_flag (l2cap_event_flag & L2CAP_FLAG_CONFIG_RFCOMM_SUCCESS)
#define l2cap_disconnect_request_sdp_flag (l2cap_event_flag & L2CAP_FLAG_DISCONNECT_SDP_REQUEST)
#define l2cap_disconnect_request_rfcomm_flag (l2cap_event_flag & L2CAP_FLAG_DISCONNECT_RFCOMM_REQUEST)
#define l2cap_disconnect_response_flag (l2cap_event_flag & L2CAP_FLAG_DISCONNECT_RESPONSE)

/* Used for SDP */
#define SDP_SERVICE_SEARCH_ATTRIBUTE_REQUEST_PDU    0x06 // See the RFCOMM specs
#define SDP_SERVICE_SEARCH_ATTRIBUTE_RESPONSE_PDU   0x07 // See the RFCOMM specs
#define SERIALPORT_UUID     0x1101 // See http://www.bluetooth.org/Technical/AssignedNumbers/service_discovery.htm
#define L2CAP_UUID          0x0100

/* Used for RFCOMM */
#define RFCOMM_SABM     0x2F
#define RFCOMM_UA       0x63
#define RFCOMM_UIH      0xEF
//#define RFCOMM_DM       0x0F
#define RFCOMM_DISC     0x43

#define extendAddress   0x01 // Always 1

// Multiplexer message types
#define BT_RFCOMM_PN_CMD     0x83
#define BT_RFCOMM_PN_RSP     0x81
#define BT_RFCOMM_MSC_CMD    0xE3
#define BT_RFCOMM_MSC_RSP    0xE1
#define BT_RFCOMM_RPN_CMD    0x93
#define BT_RFCOMM_RPN_RSP    0x91
/*
#define BT_RFCOMM_TEST_CMD   0x23
#define BT_RFCOMM_TEST_RSP   0x21
#define BT_RFCOMM_FCON_CMD   0xA3
#define BT_RFCOMM_FCON_RSP   0xA1
#define BT_RFCOMM_FCOFF_CMD  0x63
#define BT_RFCOMM_FCOFF_RSP  0x61
#define BT_RFCOMM_RLS_CMD    0x53
#define BT_RFCOMM_RLS_RSP    0x51
#define BT_RFCOMM_NSC_RSP    0x11
 */

/** This BluetoothService class implements the Serial Port Protocol (SPP). */
class SPP : public BluetoothService { //, public Stream {
public:
        /**
         * Constructor for the SPP class.
         * @param  p   Pointer to BTD class instance.
         * @param  name   Set the name to BTD#btdName. If argument is omitted, then "Arduino" will be used.
         * @param  pin   Write the pin to BTD#btdPin. If argument is omitted, then "1234" will be used.
         */
        SPP(BTD *p, const char* name = "USB Host", const char* pin = "0000");

        /**
         * Used to provide Boolean tests for the class.
         * @return Return true if SPP communication is connected.
         */
        operator bool() {
                return connected;
        }
        /** Variable used to indicate if the connection is established. */
        bool connected;

        /** @name BluetoothService implementation */
        /**
         * Used to pass acldata to the services.
         * @param ACLData Incoming acldata.
         */
        virtual void ACLData(uint8_t* ACLData);
        /** Used to establish the connection automatically. */
        virtual void Run();
        /** Use this to reset the service. */
        virtual void Reset();
        /** Used this to disconnect the virtual serial port. */
        virtual void disconnect();
        /**@}*/

        /** @name Serial port profile (SPP) Print functions */
        /**
         * Get number of bytes waiting to be read.
         * @return Return the number of bytes ready to be read.
         */
        virtual int available(void);
        /** Discard all the bytes in the buffer. */
        virtual void flush(void);
        /**
         * Used to read the next value in the buffer without advancing to the next one.
         * @return Return the byte. Will return -1 if no bytes are available.
         */
        virtual int peek(void);
        /**
         * Used to read the buffer.
         * @return Return the byte. Will return -1 if no bytes are available.
         */
        virtual int read(void);
        /**
         * Writes the byte to send to a buffer. The message is send when either send() or after Usb.Task() is called.
         * @param  data The byte to write.
         * @return      Return the number of bytes written.
         */
        virtual size_t write(uint8_t data);
        /**
         * Writes the bytes to send to a buffer. The message is send when either send() or after Usb.Task() is called.
         * @param  data The data array to send.
         * @param  size Size of the data.
         * @return      Return the number of bytes written.
         */
        virtual size_t write(const uint8_t* data, size_t size);
        /** Pull in write(const char *str) from Print */
        //using Print::write;
        /**
         * This will send all the bytes in the buffer.
         * This is called whenever Usb.Task() is called,
         * but can also be called via this function.
         */
        void send(void);
        /**@}*/

private:
        /* Bluetooth dongle library pointer */
        BTD *pBtd;

        /* Set true when a channel is created */
        bool SDPConnected;
        bool RFCOMMConnected;

        uint16_t hci_handle; // The HCI Handle for the connection

        /* Variables used by L2CAP state machines */
        uint8_t l2cap_sdp_state;
        uint8_t l2cap_rfcomm_state;
        uint16_t l2cap_event_flag; // l2cap flags of received Bluetooth events

        uint8_t l2capoutbuf[BULK_MAXPKTSIZE]; // General purpose buffer for l2cap out data
        uint8_t rfcommbuf[10]; // Buffer for RFCOMM Commands

        /* L2CAP Channels */
        uint8_t sdp_scid[2]; // L2CAP source CID for SDP
        uint8_t sdp_dcid[2]; // 0x0050
        uint8_t rfcomm_scid[2]; // L2CAP source CID for RFCOMM
        uint8_t rfcomm_dcid[2]; // 0x0051
        uint8_t identifier; // Identifier for command

        /* RFCOMM Variables */
        uint8_t rfcommChannel;
        uint8_t rfcommChannelConnection; // This is the channel the SPP channel will be running at
        uint8_t rfcommDirection;
        uint8_t rfcommCommandResponse;
        uint8_t rfcommChannelType;
        uint8_t rfcommPfBit;

        unsigned long timer;
        bool waitForLastCommand;
        bool creditSent;

        uint8_t rfcommDataBuffer[100]; // Create a 100 sized buffer for incoming data
        uint8_t sppOutputBuffer[100]; // Create a 100 sized buffer for outgoing SPP data
        uint8_t sppIndex;
        uint8_t rfcommAvailable;

        bool firstMessage; // Used to see if it's the first SDP request received
        uint8_t bytesRead; // Counter to see when it's time to send more credit

        /* State machines */
        void SDP_task(); // SDP state machine
        void RFCOMM_task(); // RFCOMM state machine

        /* SDP Commands */
        void SDP_Command(uint8_t* data, uint8_t nbytes);
        void serviceNotSupported(uint8_t transactionIDHigh, uint8_t transactionIDLow);
        void serialPortResponse1(uint8_t transactionIDHigh, uint8_t transactionIDLow);
        void serialPortResponse2(uint8_t transactionIDHigh, uint8_t transactionIDLow);
        void l2capResponse1(uint8_t transactionIDHigh, uint8_t transactionIDLow);
        void l2capResponse2(uint8_t transactionIDHigh, uint8_t transactionIDLow);

        /* RFCOMM Commands */
        void RFCOMM_Command(uint8_t* data, uint8_t nbytes);
        void sendRfcomm(uint8_t channel, uint8_t direction, uint8_t CR, uint8_t channelType, uint8_t pfBit, uint8_t* data, uint8_t length);
        void sendRfcommCredit(uint8_t channel, uint8_t direction, uint8_t CR, uint8_t channelType, uint8_t pfBit, uint8_t credit);
        uint8_t calcFcs(uint8_t *data);
        uint8_t __crc(uint8_t* data);
};
#endif
