import os
import usb1
from PIL import Image
from io import BytesIO
import argparse
import time
import cv2
import numpy as np
from threading import Thread

WEBUSB_JPEG_MAGIC = 0x2B2D2B2D
WEBUSB_TEXT_MAGIC = 0x0F100E12

VendorId = 0x2886  # seeed studio
ProductId = [0x8060, 0x8061]


class Receive_Mess():
    def __init__(self, arg, device_id):
        self.showimg = not arg.unshow
        self.saveimg = not arg.unsave
        self.interval = arg.interval
        self.img_number = 0
        self.ProductId = []
        os.makedirs("./save_img", exist_ok=True)

        self.expect_size = 0
        self.buff = bytearray()

        self.device_id = device_id
        self.context = usb1.USBContext()

        self.get_rlease_device(device_id, False)

        self.disconnect()
        self.pre_time = time.time() * 1000
        time.time_ns()

    def start(self):
        while True:
            if not self.connect():
                continue
            self.read_data()
            del self.handle
            self.disconnect()

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
                # reading data
                self.context.handleEvents()

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
            try:
                Image.open(BytesIO(self.buff))
            except:
                self.buff = bytearray()
                return
            if self.saveimg and ((time.time() * 1000 - self.pre_time) > self.interval):
                with open(f'./save_img/{time.time()}.jpg', 'wb') as f:
                    f.write(bytes(self.buff))
                self.img_number += 1
                print(f'\rNumber of saved pictures on device {self.device_id}：{self.img_number}', end='')
                self.pre_time = time.time() * 1000

            if self.showimg:
                self.show_byte()
            self.buff = bytearray()

    def show_byte(self):
        try:
            img = Image.open(BytesIO(self.buff))
            img = np.array(img)
            cv2.imshow('img', cv2.cvtColor(img,cv2.COLOR_RGB2BGR))
            cv2.waitKey(1)
        except:
            return

    def processReceivedData(self, transfer):
        if transfer.getStatus() != usb1.TRANSFER_COMPLETED:
            # transfer.close()
            return

        data = transfer.getBuffer()[:transfer.getActualLength()]
        # Process data...
        self.pare_data(data)
        # Resubmit transfer once data is processed.
        transfer.submit()

    def connect(self):
        '''Get open devices'''
        self.handle = self.get_rlease_device(self.device_id, get=True)
        if self.handle is None:
            print('\rPlease plug in the device!')
            return False
        with self.handle.claimInterface(2):
            self.handle.setInterfaceAltSetting(2, 0)
            self.handle.controlRead(0x01 << 5, request=0x22, value=0x01, index=2, length=2048, timeout=1000)
            print('device is connected')
        return True

    def disconnect(self):
        try:
            print('Resetting device...')
            with usb1.USBContext() as context:
                handle = context.getByVendorIDAndProductID(VendorId, self.ProductId[self.device_id],
                                                           skip_on_error=False).open()
                handle.controlRead(0x01 << 5, request=0x22, value=0x00, index=2, length=2048, timeout=1000)
                handle.close()
                print('Device has been reset!')
            return True
        except:
            return False

    def get_rlease_device(self, did, get=True):
        '''Turn the device on or off'''
        tmp = 0
        print('*' * 50)
        print('looking for device!')
        for device in self.context.getDeviceIterator(skip_on_error=True):
            product_id = device.getProductID()
            vendor_id = device.getVendorID()
            device_addr = device.getDeviceAddress()
            bus = '->'.join(str(x) for x in ['Bus %03i' % (device.getBusNumber(),)] + device.getPortNumberList())
            if vendor_id == VendorId and product_id in ProductId and tmp == did:
                self.ProductId.append(product_id)
                print('\r' + f'\033[4;31mID {vendor_id:04x}:{product_id:04x} {bus} Device {device_addr} \033[0m',
                      end='')
                if get:
                    return device.open()
                else:
                    device.close()
                    print(
                        '\r' + f'\033[4;31mID {vendor_id:04x}:{product_id:04x} {bus} Device {device_addr} CLOSED\033[0m',
                        flush=True)
            elif vendor_id == VendorId and product_id in ProductId:
                self.ProductId.append(product_id)
                print(f'\033[0;31mID {vendor_id:04x}:{product_id:04x} {bus} Device {device_addr}\033[0m')
                tmp = tmp + 1
            else:
                print(
                    f'ID {vendor_id:04x}:{product_id:04x} {bus} Device {device_addr}')


def implement(arg, device):
    rr = Receive_Mess(arg, device)
    time.sleep(1)
    rr.start()


if __name__ == '__main__':
    opt = argparse.ArgumentParser()
    opt.add_argument('--unsave', action='store_true', help='whether save pictures')
    opt.add_argument('--unshow', action='store_true', help='whether show pictures')
    opt.add_argument('--device-num', type=int, default=1, help='Number of devices that need to be connected')
    opt.add_argument('--interval', type=int, default=300, help='ms,Minimum time interval for saving pictures')
    arg = opt.parse_args()
    if arg.device_num == 1:
        implement(arg, 0)
    elif arg.device_num <= 0:
        raise 'The number of devices must be at least one!'
    else:
        pro_ls = []
        for i in range(arg.device_num):
            pro_ls.append(Thread(target=implement, args=(arg, i,)))
        for i in pro_ls:
            i.start()


