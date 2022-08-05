import argparse
import cv2
import numpy as np
import os
import time
import usb1
from PIL import Image
from io import BytesIO

WEBUSB_JPEG_MAGIC = 0x2B2D2B2D
WEBUSB_TEXT_MAGIC = 0x0F100E12

VendorId = 0x2886  # seeed studio
ProductId = 0x8060

total_num = 0
now_num = 0
breaken = False


class Receive_Mess():
    def __init__(self, arg,device):
        self.showimg = arg.showimg
        self.saveimg = arg.saveimg
        self.deid = []

        self.expect_size = 0
        self.buff = bytearray()

        os.makedirs("./save_img", exist_ok=True)

        self.device = device
        self.context = usb1.USBContext()

        self.rleasedevice(device)


    def connect(self):
        self.handle = self.device_info(self.device)

        # self.handle = self.deid[self.device].open()
        # self.handle = self.context.openByVendorIDAndProductID(VendorId, ProductId, skip_on_error=False)
        # for i in self.context.getDeviceIterator():
        print('Device is connected!')

    def read_data(self):
        # Device not present, or user is not allowed to access device.

        with self.handle.claimInterface(2):
            # Do stuff with endpoints on claimed interface.
            self.handle.setInterfaceAltSetting(2, 0)
            self.handle.controlRead(0x01 << 5, request=0x22, value=0x01, index=2, length=2048, timeout=1000)
            # Build a list of transfer objects and submit them to prime the pump.
            transfer_list = []
            for _ in range(1):
                transfer = self.handle.getTransfer()

                transfer.setBulk(usb1.ENDPOINT_IN | 2, 2048, callback=self.processReceivedData, timeout=1000)
                transfer.submit()
                transfer_list.append(transfer)

            # Loop as long as there is at least one submitted transfer.
            while any(x.isSubmitted() for x in transfer_list):
                # try:
                    self.context.handleEvents()
                    print(self.device)
                # except usb1.USBErrorInterrupted:
                #     return 0

    def disconnect(self):
        try:
            if hasattr(self, 'handle'):
                if self.handle is None: return
                self.handle.controlRead(0x01 << 5, request=0x22, value=0x00, index=2, length=2048, timeout=1000)
                self.handle.close()
                print('Device is disconnected!')
            else:
                with usb1.USBContext() as context:
                    handle = context.getByVendorIDAndProductID(0x2886, 0x8060, skip_on_error=True).open()
                    handle.controlRead(0x01 << 5, request=0x22, value=0x00, index=2, length=2048, timeout=1000)
                    handle.close()
                    print('Device is disconnected!')
        except:
            pass

        time.sleep(0.5)

    def pare_data(self, data: bytearray):
        if len(data) == 8 and int.from_bytes(bytes(data[:4]), 'big') == WEBUSB_JPEG_MAGIC:
            self.expect_size = int.from_bytes(bytes(data[4:]), 'big')
            self.buff = bytearray()
        elif len(data) == 8 and int.from_bytes(bytes(data[:4]), 'big') == WEBUSB_TEXT_MAGIC:
            self.expect_size = int.from_bytes(bytes(data[4:]), 'big')
            self.buff = bytearray()
        else:
            self.buff = self.buff + data

        if self.expect_size == len(self.buff):
            print(self.expect_size, 'B')
            if self.saveimg:
                with open(f'./save_img/{time.time()}.jpg', 'wb')as f:
                    f.write(bytes(self.buff))

            if self.showimg:
                self.show_byte()

            self.buff = bytearray()

    def show_byte(self):
        try:
            img = Image.open(BytesIO(self.buff))
            img = np.array(img)
            cv2.imshow('img', cv2.cvtColor(img, cv2.COLOR_BGR2RGB))
            cv2.waitKey(10)
        except:
            return False

    def processReceivedData(self, transfer):
        if transfer.getStatus() != usb1.TRANSFER_COMPLETED:
            self.rleasedevice(self.device)
            # transfer.close()
            return
            # return

        data = transfer.getBuffer()[:transfer.getActualLength()]
        # Process data...
        self.pare_data(data)
        # Resubmit transfer once data is processed.
        transfer.submit()
        # time.sleep(0.00001)

    def device_info(self,did):
        tmp=0
        # with usb1.USBContext() as context:
        for device in self.context.getDeviceIterator(skip_on_error=True):
            print('ID %04x:%04x' % (device.getVendorID(), device.getProductID()),
                  '->'.join(str(x) for x in ['Bus %03i' % (device.getBusNumber(),)] + device.getPortNumberList()),
                  'Device', device.getDeviceAddress())
            if device.getVendorID() == VendorId and device.getProductID() == ProductId and tmp == did:
                return device.open()
            elif device.getVendorID() == VendorId and device.getProductID() == ProductId:
                tmp = tmp + 1

    def rleasedevice(self,did):
        tmp = 0
        for device in self.context.getDeviceIterator(skip_on_error=True):
            print('ID %04x:%04x' % (device.getVendorID(), device.getProductID()),
                  '->'.join(str(x) for x in ['Bus %03i' % (device.getBusNumber(),)] + device.getPortNumberList()),
                  'Device', device.getDeviceAddress())
            if device.getVendorID() == VendorId and device.getProductID() == ProductId and tmp == did:
                print('++++++++++++++++')
                device.close()
            elif device.getVendorID() == VendorId and device.getProductID() == ProductId:
                tmp = tmp + 1


def rede(did):
    tmp = 0
    # with usb1.USBContext() as context:
    for device in usb1.USBContext().getDeviceIterator(skip_on_error=True):
        print('ID %04x:%04x' % (device.getVendorID(), device.getProductID()),
              '->'.join(str(x) for x in ['Bus %03i' % (device.getBusNumber(),)] + device.getPortNumberList()),
              'Device', device.getDeviceAddress())
        if device.getVendorID() == VendorId and device.getProductID() == ProductId and tmp == did:
            return device.open()
        elif device.getVendorID() == VendorId and device.getProductID() == ProductId:
            tmp = tmp + 1

def disconnect(did):
    hand=rede(did)
    hand.controlRead(0x01 << 5, request=0x22, value=0x00, index=2, length=2048, timeout=1000)
    # hand.close()


def implement(arg,device):
    while True:

        rr = Receive_Mess(arg,device)
        try:
            disconnect(device)
            rr.rleasedevice(device)
            # rr.disconnect()
            rr.connect()
            rr.read_data()
        except:
            # disconnect(device)

            pass

def redevice_number():
    res=0
    with usb1.USBContext() as context:
        for device in context.getDeviceIterator(skip_on_error=False):
            print('ID %04x:%04x' % (device.getVendorID(), device.getProductID()),
                  '->'.join(str(x) for x in ['Bus %03i' % (device.getBusNumber(),)] + device.getPortNumberList()),
                  'Device', device.getDeviceAddress())
            if device.getVendorID() == VendorId and device.getProductID() == ProductId:
                res +=1
    return res

if __name__ == '__main__':
    from multiprocessing import Process
    opt = argparse.ArgumentParser()
    opt.add_argument('--saveimg', action='store_true', default=True, help='wether save img')
    opt.add_argument('--showimg', action='store_true', default=True, help='wether save img')
    num = redevice_number()
    arg = opt.parse_args()
    # implement(arg,0)
    num=2
    pro_ls = []
    for i in range(num):
        pro_ls.append(Process(target=implement,args=(arg,i,)))
    for i in pro_ls:
        i.start()

