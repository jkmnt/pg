#
# Diz
#
import sys, os
from PIL import Image, ImageFont, ImageDraw
from _fnt import generate_font

CPS =  [0]
CPS += range(0x0020, 0x007E+1)      # Ascii + latin-1
CPS += range(0x0400, 0x04FF+1)      # Cyrillic

def measure_glyph(font, code):
    return font.getsize(unichr(code))

def create_glyph(code, font, max_w, max_h):
    w, h = measure_glyph(font, code)
    if w:
        im = Image.new('1', (w, max_h), 0)
        draw = ImageDraw.Draw(im)
        draw.text( (0, 0), unichr(code), font = font, fill = 1)
    else:
        print 'No glyph', code, w, h, max_h
        im = Image.new('1', (max_h / 2, max_h), 0)
    im = im.rotate(-90).transpose(Image.FLIP_LEFT_RIGHT)
    return {'width':w, 'height':max_h, 'bytes':im.tostring('raw', '1;R')}

def measure_glyphset(font, codes):
    w_sizes = []
    h_sizes = []

    for code in codes:
        w, h = measure_glyph(font, code)
        w_sizes += [w]
        h_sizes += [h]

    w = max(w_sizes)
    h = max(h_sizes)

    im = Image.new('1', (len(codes) * w, h * 2), 0)
    draw = ImageDraw.Draw(im)
    a = ''.join([unichr(code) for code in codes])
    draw.text( (0, 0), a, font=font, fill=1)
    bbox = im.getbbox()
    h = bbox[3] - bbox[1] + 1
    return (w, h)

def main(args):
    if len(args) != 3:
        print 'Font conversion utility for pg lib'
        print 'usage: python fcnv.py fontname.ttf outfile fontsize'
        sys.exit(-1)
    infont, outfile, font_size = args[0:3]

    if infont == outfile:
        print 'Files should be different'
        sys.exit(-1)

    font_size = int(font_size)

    font = ImageFont.truetype(infont, font_size)
    max_w, max_h = measure_glyphset(font, CPS)
    name = os.path.splitext(outfile)[0]
    res = generate_font(CPS, create_glyph, name, max_h, max_h / 2, 0, font, max_w, max_h)
    f = open(outfile, 'w')
    hdr = '/* converted from %s at size %d */\n' % (infont, font_size)
    f.write(hdr)
    f.write(res)

if __name__ == '__main__':
    main(sys.argv[1:])
