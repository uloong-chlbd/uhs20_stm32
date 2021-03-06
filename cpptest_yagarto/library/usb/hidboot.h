/* Copyright (C) 2011 Circuits At Home, LTD. All rights reserved.

This software may be distributed and modified under the terms of the GNU
General Public License version 2 (GPL2) as published by the Free Software
Foundation and appearing in the file GPL2.TXT included in the packaging of
this file. Please note that GPL2 Section 2[b] requires that all works based
on this software must also be made publicly available under the terms of
the GPL2 ("Copyleft").

Contact information
-------------------

Circuits At Home, LTD
Web      :  http://www.circuitsathome.com
e-mail   :  support@circuitsathome.com
 */
#if !defined(__HIDBOOT_H__)
#define __HIDBOOT_H__

#include <inttypes.h>
//#include <avr/pgmspace.h>
//#include "avrpins.h"
//#include "max3421e.h"
#include "usbhost.h"
#include "usb_ch9.h"
#include "Usb.h"
#include "hid.h"

//#include "printhex.h"
//#include "hexdump.h"
#include "message.h"

#include "confdescparser.h"

#define KEY_SPACE							0x2c
#define KEY_ZERO							0x27
#define KEY_ZERO2							0x62
#define KEY_ENTER							0x28
#define KEY_PERIOD							0x63

struct MOUSEINFO {

        struct {
                uint8_t bmLeftButton : 1;
                uint8_t bmRightButton : 1;
                uint8_t bmMiddleButton : 1;
                uint8_t bmDummy : 1;
        };
        int8_t dX;
        int8_t dY;
};

class MouseReportParser : public HIDReportParser {

        union {
                MOUSEINFO mouseInfo;
                uint8_t bInfo[sizeof(MOUSEINFO)];
        } prevState;

public:
        virtual void Parse(HID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);

protected:

        virtual void OnMouseMove(MOUSEINFO *mi) {
        };

        virtual void OnLeftButtonUp(MOUSEINFO *mi) {
        };

        virtual void OnLeftButtonDown(MOUSEINFO *mi) {
        };

        virtual void OnRightButtonUp(MOUSEINFO *mi) {
        };

        virtual void OnRightButtonDown(MOUSEINFO *mi) {
        };

        virtual void OnMiddleButtonUp(MOUSEINFO *mi) {
        };

        virtual void OnMiddleButtonDown(MOUSEINFO *mi) {
        };
};

struct MODIFIERKEYS {
        uint8_t bmLeftCtrl : 1;
        uint8_t bmLeftShift : 1;
        uint8_t bmLeftAlt : 1;
        uint8_t bmLeftGUI : 1;
        uint8_t bmRightCtrl : 1;
        uint8_t bmRightShift : 1;
        uint8_t bmRightAlt : 1;
        uint8_t bmRightGUI : 1;
};

struct KBDINFO {

        struct {
                uint8_t bmLeftCtrl : 1;
                uint8_t bmLeftShift : 1;
                uint8_t bmLeftAlt : 1;
                uint8_t bmLeftGUI : 1;
                uint8_t bmRightCtrl : 1;
                uint8_t bmRightShift : 1;
                uint8_t bmRightAlt : 1;
                uint8_t bmRightGUI : 1;
        };
        uint8_t bReserved;
        uint8_t Keys[6];
};

struct KBDLEDS {
        uint8_t bmNumLock : 1;
        uint8_t bmCapsLock : 1;
        uint8_t bmScrollLock : 1;
        uint8_t bmCompose : 1;
        uint8_t bmKana : 1;
        uint8_t bmReserved : 3;
};

#define KEY_NUM_LOCK				0x53
#define KEY_CAPS_LOCK				0x39
#define KEY_SCROLL_LOCK				0x47

class KeyboardReportParser : public HIDReportParser {
        static const uint8_t numKeys[];
        static const uint8_t symKeysUp[];
        static const uint8_t symKeysLo[];
        static const uint8_t padKeys[];

protected:

        union {
                KBDINFO kbdInfo;
                uint8_t bInfo[sizeof(KBDINFO)];
        } prevState;

        union {
                KBDLEDS kbdLeds;
                uint8_t bLeds;
        } kbdLockingKeys;

        uint8_t OemToAscii(uint8_t mod, uint8_t key);

public:

        KeyboardReportParser() {
                kbdLockingKeys.bLeds = 0;
        };

        virtual void Parse(HID *hid, bool is_rpt_id, uint8_t len, uint8_t *buf);

protected:
        virtual uint8_t HandleLockingKeys(HID* hid, uint8_t key);

		virtual void OnControlKeysChanged(uint8_t before, uint8_t after) {
		};

		virtual void OnKeyDown(uint8_t mod, uint8_t key) {
        };

        virtual void OnKeyUp(uint8_t mod, uint8_t key) {
        };
};

#define totalEndpoints (2 + ((BOOT_PROTOCOL == (HID_PROTOCOL_KEYBOARD | HID_PROTOCOL_MOUSE))? 1 : 0))
#define epMUL (((BOOT_PROTOCOL & HID_PROTOCOL_KEYBOARD)? 1 : 0)+((BOOT_PROTOCOL & HID_PROTOCOL_MOUSE)? 1 : 0))
#define HID_MAX_HID_CLASS_DESCRIPTORS		5

template <const uint8_t BOOT_PROTOCOL>
class HIDBoot : public HID //public USBDeviceConfig, public UsbConfigXtracter
{
        EpInfo epInfo[totalEndpoints];
        HIDReportParser *pRptParser[epMUL];

        uint8_t bConfNum; // configuration number
        uint8_t bIfaceNum; // Interface Number
        uint8_t bNumIface; // number of interfaces in the configuration
        uint8_t bNumEP; // total number of EP in the configuration
        uint32_t qNextPollTime; // next poll time
        bool bPollEnable; // poll enable flag

        void Initialize();

        virtual HIDReportParser* GetReportParser(uint8_t id) {
                return pRptParser[id];
        };

public:
        HIDBoot(USB *p);

        virtual bool SetReportParser(uint8_t id, HIDReportParser *prs) {
                pRptParser[id] = prs;
                return true;
        };

        // USBDeviceConfig implementation
        virtual uint8_t Init(uint8_t parent, uint8_t port, bool lowspeed);
        virtual uint8_t Release();
        virtual uint8_t Poll();

        virtual uint8_t GetAddress() {
                return bAddress;
        };

        // UsbConfigXtracter implementation
        virtual void EndpointXtract(uint8_t conf, uint8_t iface, uint8_t alt, uint8_t proto, const USB_ENDPOINT_DESCRIPTOR *ep);
};

template <const uint8_t BOOT_PROTOCOL>
HIDBoot<BOOT_PROTOCOL>::HIDBoot(USB *p) :
HID(p),
qNextPollTime(0),
bPollEnable(false) {
        Initialize();

        for(uint8_t i = 0; i < epMUL; i++) {
                pRptParser[i] = NULL;
        }
        if(pUsb)
			pUsb->RegisterDeviceClass(this);
}

template <const uint8_t BOOT_PROTOCOL>
void HIDBoot<BOOT_PROTOCOL>::Initialize() {
        for(uint8_t i = 0; i < totalEndpoints; i++) {
                epInfo[i].epAddr = 0;
                epInfo[i].maxPktSize = (i) ? 0 : 8;
                epInfo[i].hcNumber = 0;
                epInfo[i].epAttribs = 0;
                epInfo[i].bmNakPower = (i) ? USB_NAK_NOWAIT : USB_NAK_MAX_POWER;
        }
        bNumEP = 1;
        bNumIface = 0;
        bConfNum = 0;
}

template <const uint8_t BOOT_PROTOCOL>
uint8_t HIDBoot<BOOT_PROTOCOL>::Init(uint8_t parent, uint8_t port, bool lowspeed) {
        const uint8_t constBufSize = sizeof(USB_DEVICE_DESCRIPTOR);

        uint8_t buf[constBufSize];
        uint8_t rcode;
        UsbDevice *p = NULL;
        EpInfo *oldep_ptr = NULL;
        uint8_t len = 0;
        //uint16_t cd_len = 0;

        uint8_t num_of_conf; // number of configurations
        //uint8_t num_of_intf; // number of interfaces

        AddressPool &addrPool = pUsb->GetAddressPool();

        printf("\n\nHIDBoot Init");

        if(bAddress)
			return USB_ERROR_CLASS_INSTANCE_ALREADY_IN_USE;

        // Get pointer to pseudo device with address 0 assigned
        p = addrPool.GetUsbDevicePtr(0);

        if(!p)
			return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;

        if(!p->epinfo) {
			USBTRACE("epinfo\r\n");
			return USB_ERROR_EPINFO_IS_NULL;
        }

        // Save old pointer to EP_RECORD of address 0
        oldep_ptr = p->epinfo;

        // Temporary assign new pointer to epInfo to p->epinfo in order to avoid toggle inconsistence
        p->epinfo = epInfo;

        // still use mother's host channel
        p->epinfo->hcNumber = oldep_ptr->hcNumber;

        p->lowspeed = lowspeed;

        // Get device descriptor
        rcode = pUsb->getDevDescr(0, 0, 8, (uint8_t*) buf);
        //STM_EVAL_LEDToggle(LED1);
        printf("\nHIDBoot - Get 2nd 8 byte desc.");
        if(!rcode)
			len = (buf[0] > constBufSize) ? constBufSize : buf[0];

        if(rcode) {
			// Restore p->epinfo
			p->epinfo = oldep_ptr;

			goto FailGetDevDescr;
        }

        // Restore p->epinfo
        p->epinfo = oldep_ptr;

        // Allocate new address according to device class
        bAddress = addrPool.AllocAddress(parent, false, port);

        if(!bAddress)
			return USB_ERROR_OUT_OF_ADDRESS_SPACE_IN_POOL;

        // Extract Max Packet Size from the device descriptor
        epInfo[0].maxPktSize = (uint8_t) ((USB_DEVICE_DESCRIPTOR*) buf)->bMaxPacketSize0;

        // Assign new address to the device
        rcode = pUsb->setAddr(0, 0, bAddress);

        if(rcode) {
			p->lowspeed = false;
			addrPool.FreeAddress(bAddress);
			bAddress = 0;
			USBTRACE2("setAddr:", rcode);
			return rcode;
        }

        printf("\nHIDBoot Addr:%d", bAddress);

        p->lowspeed = false;

        p = addrPool.GetUsbDevicePtr(bAddress);
        // let's copy ep0's hc_num to ep1
        p->epinfo->hcNumber = epInfo[0].hcNumber;
        p->epinfo->maxPktSize = epInfo[0].maxPktSize;

        if(!p)
			return USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL;

        p->lowspeed = lowspeed;
        if(len)
			rcode = pUsb->getDevDescr(bAddress, 0, len, (uint8_t*) buf);

        if(rcode)
			goto FailGetDevDescr;

        num_of_conf = ((USB_DEVICE_DESCRIPTOR*) buf)->bNumConfigurations;

        // Assign epInfo to epinfo pointer
        rcode = pUsb->setEpInfoEntry(bAddress, 1, epInfo);

        if(rcode)
			goto FailSetDevTblEntry;

        //USBTRACE2("NC:", num_of_conf);

        // GCC will optimize unused stuff away.
        if(BOOT_PROTOCOL & HID_PROTOCOL_KEYBOARD) {
                for(uint8_t i = 0; i < num_of_conf; i++) {
                        ConfigDescParser<
                                USB_CLASS_HID,
                                HID_BOOT_INTF_SUBCLASS,
                                HID_PROTOCOL_KEYBOARD,
                                CP_MASK_COMPARE_ALL> confDescrParserA(this);

                        if(bNumEP == totalEndpoints)
                                break;
                        pUsb->getConfDescr(bAddress, 0, i, &confDescrParserA);
                }
        }

        // GCC will optimize unused stuff away.
        if(BOOT_PROTOCOL & HID_PROTOCOL_MOUSE) {
                for(uint8_t i = 0; i < num_of_conf; i++) {
                        ConfigDescParser<
                                USB_CLASS_HID,
                                HID_BOOT_INTF_SUBCLASS,
                                HID_PROTOCOL_MOUSE,
                                CP_MASK_COMPARE_ALL> confDescrParserB(this);
                        if(bNumEP == totalEndpoints)
                                break;

                        pUsb->getConfDescr(bAddress, 0, i, &confDescrParserB);
                }
        }

        USBTRACE2("\r\nbAddr:", bAddress);
        USBTRACE2("\r\nbNumEP:", bNumEP);
        USBTRACE2("\r\ntotalEndpoints:", totalEndpoints);
        if(bNumEP != totalEndpoints) {
        	printf("\nHIDBoot Dev not supported, bNumEP = %d", bNumEP);
        	rcode = USB_DEV_CONFIG_ERROR_DEVICE_NOT_SUPPORTED;
        	goto Fail;
        }

        // Assign epInfo to epinfo pointer
        rcode = pUsb->setEpInfoEntry(bAddress, bNumEP, epInfo);

        USBTRACE2("\r\nCnf:", bConfNum);

        // Set Configuration Value
        rcode = pUsb->setConf(bAddress, 0, bConfNum);

        if(rcode)
			goto FailSetConfDescr;

        USBTRACE2("\r\nIf:", bIfaceNum);

        rcode = SetProtocol(bIfaceNum, HID_BOOT_PROTOCOL);

        if(rcode)
			goto FailSetProtocol;

        // GCC will optimize unused stuff away.
        if(BOOT_PROTOCOL & HID_PROTOCOL_KEYBOARD) {
                rcode = SetIdle(0/* bIfaceNum*/, 0, 0);

			if(rcode)
				goto FailSetIdle;
        }
        USBTRACE("HIDBoot BM configured\r\n");

        bPollEnable = true;
        uint8_t hcnum;
        hcnum = USB::USBH_Alloc_Channel(pUsb->coreConfig, epInfo[1].epAddr);
    	for(uint8_t i = 0; i < epMUL; i++) {
    		epInfo[1+i].hcNumIn = hcnum;
    	}
		printf("\nHIDBoot Pipe in = %d (EP_TYPE_INTR)", hcnum);

        // Assign epInfo to epinfo pointer
        rcode = pUsb->setEpInfoEntry(bAddress, bNumEP, epInfo);
        USB::USBH_Open_Channel(pUsb->coreConfig, hcnum, bAddress, (lowspeed)?bmLOWSPEED:bmFULLSPEED, EP_TYPE_INTR, epInfo[1].maxPktSize);
        pUsb->coreConfig->host.hc[hcnum].toggle_in = 0x0;

        return 0;

FailGetDevDescr:
#ifdef DEBUG_USB_HOST
        NotifyFailGetDevDescr();
        goto Fail;
#endif

FailSetDevTblEntry:
#ifdef DEBUG_USB_HOST
        NotifyFailSetDevTblEntry();
        goto Fail;
#endif

FailGetConfDescr:
#ifdef DEBUG_USB_HOST
        NotifyFailGetConfDescr();
        goto Fail;
#endif

FailSetConfDescr:
#ifdef DEBUG_USB_HOST
        NotifyFailSetConfDescr();
        goto Fail;
#endif

FailSetProtocol:
#ifdef DEBUG_USB_HOST
        USBTRACE("SetProto:");
        goto Fail;
#endif

FailSetIdle:
#ifdef DEBUG_USB_HOST
        USBTRACE("SetIdle:");
#endif

Fail:
#ifdef DEBUG_USB_HOST
        NotifyFail(rcode);
#endif
        Release();
        return rcode;
}

template <const uint8_t BOOT_PROTOCOL>
void HIDBoot<BOOT_PROTOCOL>::EndpointXtract(uint8_t conf, uint8_t iface, uint8_t alt, uint8_t proto, const USB_ENDPOINT_DESCRIPTOR *pep) {
	// If the first configuration satisfies, the others are not concidered.
	if(bNumEP > 1 && conf != bConfNum)
		return;

	bConfNum = conf;
	bIfaceNum = iface;

        if((pep->bmAttributes & 0x03) == 3 && (pep->bEndpointAddress & 0x80) == 0x80) {
                uint8_t index = bNumEP;//epInterruptInIndex; //+ bNumEP;

		// Fill in the endpoint info structure
		//st bsp needs full address(0x81 for in channel)
		epInfo[index].epAddr = pep->bEndpointAddress;	// (pep->bEndpointAddress & 0x0F);
		epInfo[index].maxPktSize = (uint8_t) pep->wMaxPacketSize;
		epInfo[index].epAttribs = 0;
		epInfo[index].bmNakPower = USB_NAK_NOWAIT;

		bNumEP++;
	}
}

template <const uint8_t BOOT_PROTOCOL>
uint8_t HIDBoot<BOOT_PROTOCOL>::Release() {
	if(epInfo[1].hcNumber != 0) {	// HC0&HC1 are taken by control pipe.
		USB::USB_OTG_HC_Halt(pUsb->coreConfig, epInfo[1].hcNumIn);
		USB::USBH_Free_Channel(pUsb->coreConfig, epInfo[1].hcNumIn);
	}

	for(uint8_t i = 0; i < epMUL; i++) {
		epInfo[i].hcNumber = 0;
	}
	pUsb->GetAddressPool().FreeAddress(bAddress);

	bConfNum = 0;
	bIfaceNum = 0;
	bNumEP = 1;
	bAddress = 0;
	qNextPollTime = 0;
	bPollEnable = false;
        return 0;
}

template <const uint8_t BOOT_PROTOCOL>
uint8_t HIDBoot<BOOT_PROTOCOL>::Poll() {
	uint8_t rcode = HC_IDLE;

	if(!bPollEnable)
		return 0;

	if(qNextPollTime <= millis()) {
		qNextPollTime = millis() + 10;

		// To-do: optimize manually, getting rid of the loop
		for(uint8_t i = 0; i < epMUL; i++) {
			const uint8_t const_buff_len = 20;	//16;
			uint8_t buf[const_buff_len];
			uint16_t read = (uint16_t) epInfo[epInterruptInIndex + i].maxPktSize;
			STM_EVAL_LEDToggle(LED1);
			if(i==1) {
				delay(1);
				STM_EVAL_LEDToggle(LED1);
				delay(1);
				STM_EVAL_LEDToggle(LED1);
				//delay(1);
			}

			uint8_t rcode = pUsb->inTransfer(bAddress, epInfo[epInterruptInIndex + i].epAddr, &read, buf);
			if(!rcode) {
				if(pRptParser[i])
					pRptParser[i]->Parse((HID*)this, 0, (uint8_t) read, buf);

				//for (uint8_t i=0; i<read; i++)
				//	PrintHex<uint8_t>(buf[i]);
				//if (read)
				//	USB_HOST_SERIAL.println("");
			} else {
				if(rcode != hrNAK) {
					USBTRACE2("Poll:", rcode);
					break;
				}
			}
		}
	}
	return rcode;
}


#endif // __HIDBOOTMOUSE_H__
