import gpu
import framebuf

class RPiScreen(framebuf.FrameBuffer):
    def __init__(self, width, height):
        self.width = width
        self.height = height
        gpu.fb_init(width,height,screen_w=1920,screen_h=1080)
        super().__init__(gpu.fb_data(),width,height,framebuf.RGB565)
        self

    def show(self):
        pass
