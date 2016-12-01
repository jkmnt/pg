# python
# script to convert image to pg-compatible xbm
from PIL import Image, ImageOps, ImagePalette
import sys
import os
import glob
import math
import argparse

SYS_COLORS = [
    ('CL_BLACK', 0, 0, 0),
    ('CL_WHITE', 255, 255, 255),
    ('CL_RED',   255, 0, 0),
    ('CL_GREEN', 0, 255, 0),
    ('CL_BLUE',  0, 0, 255),
    ('CL_YELLOW', 0, 255, 255),
    ('CL_GRAY',  135, 135, 135),
    ('CL_BRAND_BLUE', 0, 174, 255),
    ('CL_BRAND_GRAY', 224, 224, 224),
    ('CL_BRAND_DARK_GRAY', 128, 128, 128),
]

TRANSPARENT_COLOR = (0, 255, 0)
MAX_COLORS = 16
TRANSPARENT_COLOR_IDX = MAX_COLORS - 1

# color lut class
class Clut:
    def __init__(self, predefined_colors=[]):
        self.clut = predefined_colors

    def _color_values(self):
        return [c[1:] for c in self.clut]

    def add(self, color):
        if color == TRANSPARENT_COLOR:
            return True
        if color in self._color_values():
            return True
        if len(self._color_values()) > MAX_COLORS:
            print 'Color table overflow'
            return False
        val = tuple(['CL_%d_%d_%d' % (color[0], color[1], color[2])] + list(color))
        self.clut += [val]

    def add_palette(self, palette):
        for c in palette:
            self.add(c)

    def get_idx_by_color(self, color):
        if color == TRANSPARENT_COLOR:
            return TRANSPARENT_COLOR_IDX
        if color not in self._color_values():
            return None
        else:
            return self._color_values().index(color)

    def tostring(self):
        out = ''
        out += 'static const u16 clut[] = {\n'
        out += ''.join(['\t[%s] = FROM_RGB_256(%d, %d, %d),\n' % (c[0], c[1], c[2], c[3]) for c in self.clut])
        out += '};'
        return out

    def cl_names_enum(self):
        out = ''
        out += 'enum cl_names_e {\n'
        out += ''.join(['\t%s,\n' % (c[0]) for c in self.clut])
        out += '};'
        return out

def chunker(seq, size):
    return (seq[pos:pos + size] for pos in range(0, len(seq), size))

# read all images into memory
def read_images(names):
    images = []
    for name in names:
        im = Image.open(name)
        images += [{'name':name, 'orig':im}]
    return images

def process(imds):
    max_w = max([imd['orig'].size[0] for imd in imds])
    max_h = max([imd['orig'].size[1] for imd in imds])
    # box fitting largest image
    max_box = (max_w, max_h)
    # length of common square box side
    nsquare = int(math.ceil(math.sqrt(len(imds))))
    # create overview image
    ov = Image.new('RGB', (nsquare * max_w, nsquare * max_h), TRANSPARENT_COLOR)
    # paste all images
    for j in range(nsquare):
        for i in range(nsquare):
            idx = j * nsquare + i
            if idx >= len(imds):
                continue
            ov_pos = (i * max_w, j * max_h)
            imds[idx]['ov_pos'] = ov_pos
            ov.paste(imds[idx]['orig'], ov_pos)
    # convert overview to paletted, resulting in common palette
    ov_paletted = ov.convert('P', dither=Image.NONE, palette=Image.ADAPTIVE, colors=MAX_COLORS)
    # cut paletted images back
    for imd in imds:
        box = (imd['ov_pos'][0], imd['ov_pos'][1], imd['ov_pos'][0] + imd['orig'].size[0], imd['ov_pos'][1] + imd['orig'].size[1])
        imd['paletted'] = ov_paletted.crop(box)
        imd['paletted'] = imd['paletted'].convert('P', dither=Image.NONE, palette=Image.ADAPTIVE, colors=len(imd['paletted'].getcolors()))
        assert imd['orig'].size == imd['paletted'].size
    return ov_paletted

# read palette of image
def get_palette(im):
    colors = im.getcolors()
    pal = im.getpalette()
    pal = [tuple(s) for s in chunker(pal, 3)]
    return pal
#   colors = zip(*colors)[1]
#   return [pal[c] for c in colors]


# pack data into C-source array
def pack_bytes(data, by=2):
    out = ''
    if isinstance(data, str):
        data = [ord(c) for c in data]
    for d in chunker(data, by):
        byte = d[0]
        if len(d) == 2:
            byte |= d[1] << 4
        out += '0x%02X, ' % byte
    return out

# repack image, replacing local palette with global clut
def repalette_img(im):
    global clut
    pal = get_palette(im)
    out = []
    for px in im.getdata():
        color = pal[px]
        idx = clut.get_idx_by_color(color)
        assert idx != None, 'pixel not in clut'
        out += [idx]
    return out

# check is image could be stored as monochrome bitmap
def is_mono(im):
    colors = zip(*im.getcolors())[1]
    ncolors = len(colors)
    if ncolors == 1:
        return True
    if ncolors > 2:
        return False
    pal = get_palette(im)
    if pal[colors[0]] == TRANSPARENT_COLOR or pal[colors[1]] == TRANSPARENT_COLOR:
        return True
    return False

def img_to_bin(im):
    return pack_bytes(repalette_img(im), by=2)

def bitmap_to_bin(im):
    pxs = im.load()
    pal = get_palette(im)
    # replace transparent color with black, active color with white
    for y in range(im.size[1]):
        for x in range(im.size[0]):
            px = pxs[x, y]
            color = pal[px]
            if color == TRANSPARENT_COLOR:
                pxs[x, y] = 0
            else:
                pxs[x, y] = 1
    # put black/white palette, convert to bw
    bw = im.copy()
    bw.putpalette([0, 0, 0, 255, 255, 255] + [0] * 762)
    bw = bw.convert('1', dither=Image.NONE)
    out = bw.tostring('raw', '1;R')
    out = pack_bytes(out, 1)
    return out

def convert_image(im, resname, bw):
    im = im.rotate(-90).transpose(Image.FLIP_LEFT_RIGHT)

    out = 'const u8 %s_bits[] = {' % resname

    if bw:
        out += bitmap_to_bin(im)
    else:
        out += img_to_bin(im)
    out += '};\n'
    out += '''
const pg_xbm_t %s =
{
    .w = %d,
    .h = %d,
    .mode = %s,
    .transp_color = %d,
    .data = %s_bits
};
''' % ( resname,
        im.size[1],
        im.size[0],
        'PG_XBM_1_BIT' if bw else 'PG_XBM_4_BIT',
        TRANSPARENT_COLOR_IDX,
        resname)
    return out


if __name__ == '__main__':
    parser = argparse.ArgumentParser(description='PG batch image converter')
    parser.add_argument('-i', '--input', type=str, default='*.png', help='input files path and wildcard (default is %s)' % '*.png')
    parser.add_argument('-o', '--outdir', type=str, default='.', help='output dir (default is %s)' % '.')
    parser.add_argument('--overview', type=str, default='overview.png', help='overview image name (default is %s)' % 'overview.png')
    parser.add_argument('--clut', type=str, default='clut', help='clut (color LUT) path and name w/o extension (default is %s)' % 'clut')
    parser.add_argument('--pic_prefix', type=str, default='pic_', help='prefix to append to multicolor image names (default is %s)' % 'pic_')
    parser.add_argument('--icon_prefix', type=str, default='icon_', help='prefix to append to monochrome icons (default is %s)' % 'icon_')

    args = parser.parse_args()

    globbed = [s for s in glob.glob(args.input) if s != args.overview]
    imds = read_images(globbed)
    ov = process(imds)
    pal = get_palette(ov)

    clut = Clut(SYS_COLORS)
    clut.add_palette(pal)

    for imd in imds:
        inname = imd['name']
        basename = os.path.basename(inname)
        basename = os.path.splitext(basename)[0]
        basename = basename.lower().replace('-','_')
        bw = is_mono(imd['paletted'])
        if bw:
            resname = args.icon_prefix + basename
        else:
            resname = args.pic_prefix + basename
        outname = args.outdir + '/' + resname + '.h'
        print '%s -> %s' % (inname, outname)
        data = convert_image(imd['paletted'], resname, bw)
        open(outname, 'w').write(data)

    print 'saving overview image %s' % args.overview
    ov.save(args.overview)

    print 'saving clut to %s.c/%s.h' % (args.clut, args.clut)
    open(args.clut + '.c', 'w').write(clut.tostring())
    open(args.clut + '.h', 'w').write(clut.cl_names_enum())
