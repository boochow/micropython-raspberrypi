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

class FBConsole:
    def __init__(self, framebuf, bgcolor=0, fgcolor=-1, width=-1, height=-1):
        self.fb = framebuf
        if width > 0:
            self.width=width
        else:
            try:
                self.width=framebuf.width
            except:
                raise ValueError
        if height > 0:
            self.height=height
        else:
            try:
                self.height=framebuf.height
            except:
                raise ValueError
        self.bgcolor = bgcolor
        self.fgcolor = fgcolor
        self.line_height(8)

    def cls(self):
        self.x = 0
        self.y = 0
        self.fb.fill(self.bgcolor)
        try:
            self.fb.show()
        except:
            pass

    def line_height(self, px):
        self.lineheight = px
        self.w =  self.width // px
        self.h =  self.height // px
        self.cls()

    def _putc(self, c):
        c = chr(c)
        if c == '\n':
            self.x = 0
            self._newline()
        elif c == '\x08':
            self._backspace()
        elif c >= ' ':
            self.fb.text(c, self.x * 8, self.y * 8, self.fgcolor)
            self.x += 1
            if self.x >= self.w:
                self._newline()
                self.x = 0

    def write(self, buf):
        i = 0
        while i < len(buf):
            c = buf[i]
            if c == 0x1b: # skip escape sequence
                i += 1
                while chr(buf[i]) in '[;0123456789':
                    i += 1
            else:
                self._putc(c)
            i += 1
        try:
            self.fb.show()
        except:
            pass

    def readinto(self, buf, nbytes=0):
        return None
        
    def _newline(self):
        self.y += 1
        if self.y >= self.h:
            self.fb.scroll(0, -8)
            self.fb.fill_rect(0, self.height - 8, self.width, 8, 0)
            self.y = self.h - 1

    def _erase_current(self):
        self.fb.fill_rect(self.x * 8, self.y * 8, 8, 8, 0)

    def _backspace(self):
        if self.x == 0:
            if self.y > 0:
                self.y -= 1
                self.x = self.w - 1
        else:
            self.x -= 1
        self.fb.fill_rect(self.x * 8, self.y * 8, 8, 8, 0)
