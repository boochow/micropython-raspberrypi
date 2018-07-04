class RAMBlockDev:
    def __init__(self, block_size, num_blocks):
        self.block_size = block_size
        self.data = bytearray(block_size * num_blocks)

    def readblocks(self, block_num, buf):
        for i in range(len(buf)):
            buf[i] = self.data[block_num * self.block_size + i]

    def writeblocks(self, block_num, buf):
        for i in range(len(buf)):
            self.data[block_num * self.block_size + i] = buf[i]

    def ioctl(self, op, arg):
        if op == 4: # get number of blocks
            return len(self.data) // self.block_size
        if op == 5: # get block size
            return self.block_size

def _mount_initrd(num_blocks):
    try:
        import uos
        bdev = RAMBlockDev(512, num_blocks)
        uos.VfsFat.mkfs(bdev)
        vfs = uos.VfsFat(bdev)
        uos.mount(vfs, '/')
    except:
        print("error: _mount_initrd")

def _create_ramdisk():
    _mount_initrd(2048)
    f=open('RAM disk.txt', 'w')
    f.write('''
Size   = 512 bytes * 2048 blocks
Format = FAT16
''')
    f.close()

def set_fb_console(on=True):
    global theScreen
    from FBConsole import FBConsole, RPiScreen
    import os
    if on:
        theScreen = FBConsole(RPiScreen(480,270))
        os.dupterm(theScreen)
    else:
        theScreen = os.dupterm(None)

def _boot_main():
    _create_ramdisk()
    import machine
    try:
        if (machine.usb_mode() == 'host'):
            set_fb_console()
    except:
        pass

print("_boot.py")
_boot_main()
