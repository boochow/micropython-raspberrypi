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

def _boot_main():
    _create_ramdisk()

_boot_main()

# uncomment below if you want to use frame buffer console
"""
from FBConsole import FBConsole, RPiScreen
import gpu, os
gpu.fb_init(480,270,screen_w=1920,screen_h=1080)
theScreen = FBConsole(RPiScreen(480,270))
os.dupterm(theScreen)
"""