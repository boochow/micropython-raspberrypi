import gpu
import framebuf
import uio

class RPiScreen(framebuf.FrameBuffer):
    def __init__(self, width, height):
        self.width = width
        self.height = height
        gpu.fb_init(width,height,screen_w=1920,screen_h=1080)
        super().__init__(gpu.fb_data(),width,height,framebuf.RGB565)
        self

    def show(self):
        pass

class FBConsole(uio.IOBase):
    def __init__(self, framebuf, bgcolor=0, fgcolor=-1, width=-1, height=-1, readobj=None):
        self.readobj = readobj
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
        self._draw_cursor()

    def _putc(self, c):
        c = chr(c)
        if c == '\n':
            self.x = 0
            self._newline()
        elif c == '\x08':
            self._backspace()
        elif c >= ' ':
            self.fb.text(c, self.x * 8, self.y * self.lineheight, self.fgcolor)
            self.x += 1
            if self.x >= self.w:
                self._newline()
                self.x = 0

    def write(self, buf):
        self._erase_current()
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
        self._draw_cursor()
        try:
            self.fb.show()
        except:
            pass
        return len(buf)

    def readinto(self, buf, nbytes=0):
        if readobj != None:
            return readobj.readinto(buf, nbytes)
        else:
            return None
        
    def _newline(self):
        self.y += 1
        if self.y >= self.h:
            self.fb.scroll(0, -8)
            self.fb.fill_rect(0, self.height - self.lineheight, self.width, self.lineheight, self.bgcolor)
            self.y = self.h - 1

    def _erase_current(self):
        self.fb.fill_rect(self.x * 8, self.y * self.lineheight, 8, self.lineheight, self.bgcolor)

    def _backspace(self):
        if self.x == 0:
            if self.y > 0:
                self.y -= 1
                self.x = self.w - 1
        else:
            self.x -= 1
        self.fb.fill_rect(self.x * 8, self.y * self.lineheight, 8, self.lineheight, self.bgcolor)

    def _draw_cursor(self):
        self.fb.hline(self.x * 8, self.y * self.lineheight + 7, 8, self.fgcolor)
