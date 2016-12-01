# python
# script to convert GNU unifont to our custom format

from PIL import Image, ImageFont, ImageDraw
from _fnt import generate_font

MIN_LEFT_GAP = 1

# required code points (all glyphs)
CPS =  [0]
CPS += range(0x0020, 0x007E+1)      # Ascii + latin-1
#CPS += range(0x00A0, 0x00FF+1)      # Latin-1 supplement
#CPS += range(0x0100, 0x017F+1)      # Latin extended A
#CPS += range(0x0180, 0x024F+1)      # Latin extended B
CPS += range(0x0400, 0x04FF+1)      # Cyrillic
#CPS += range(0x0500, 0x052F+1)      # Cyrillic supplement
#CPS += range(0x0530, 0x058F+1)      # Armenian
#CPS += range(0x0590, 0x05FF+1)      # Hebrew
#CPS += range(0x0600, 0x06FF+1)      # Arabic
#CPS += range(0x0700, 0x074F+1)      # Syriac
#CPS += range(0x0750, 0x077F+1)      # Arabic Supplement
#CPS += range(0x08A0, 0x08FF+1)      # Arabic Extended-A
CPS += [0x21B5] #   CRLF
CPS += [0x23B5] #   space bracket

def parse_uni_glyph(gl):
    bytes = ''
    idx, raw = gl.split(':')
    for i in range(len(raw) / 2):
        char = (int(raw[i*2], 16) << 4) | (int(raw[i*2 + 1], 16))
        bytes += chr(char)
    return (int(idx, 16), bytes)

def parse_unifont(lines):
    out = {}
    for g in lines:
        u = parse_uni_glyph(g)
        out[u[0]] = u[1]
    return out

def create_glyph(code, uni, is_monospaced=False):
    src = uni[code]
    width = len(src) / 2 # there are only 16x16 or 32x16 byte glyphs in unifont, string lenght is width * 2
    im = Image.fromstring( '1',  (width, 16), src, 'raw', '1', 0, 1)
#   im.show()
    if not is_monospaced:
        bbox = im.getbbox()
        if bbox:
            x0, y0, x1, y1 = bbox
            x0 = max(0, x0 - MIN_LEFT_GAP)      # preserve spacing
            width = x1 - x0
            im = im.crop((x0, 0, x1, 16))
    im = im.rotate(-90).transpose(Image.FLIP_LEFT_RIGHT)
    return {'width':width, 'height':16, 'bytes':im.tostring('raw', '1;R')}

uni = open('unifont-7.0.03.hex', 'r').readlines()
uni = parse_unifont(uni)

cps = [c for c in CPS if c in uni]
res = generate_font(cps, create_glyph, 'pg_unifont', 16, 8, 0, uni)

open('unifont.h', 'w').write(res)
