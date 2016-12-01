# font creation support

def pack_glyphs(glyphs):
    bitmaps = ''
    for g in glyphs:
        g['data']['offset'] = len(bitmaps)
        bitmaps += g['data']['bytes']
    return bitmaps

def hexy(data):
    return ','.join(['0x%02x' % ord(x) for x in data])

def generate_resource(name, glyphs, bitmaps, height, meanline, pad):
    out = ''

    out += 'const uint8_t %s_glyphs_bimap[] = {' % (name)
    out += hexy(bitmaps)
    out += '};\n'

    out += 'const pg_glyph_t %s_glyphs[] = {' % (name)
    out += ',\n'.join(['{%d, %d, %d, &%s_glyphs_bimap[%d]}' % (g['code'], g['data']['width'], g['data']['height'], name, g['data']['offset']) for g in glyphs])
    out += '};\n'

    out += 'const pg_utf8_font_t %s = { ' % (name)
    out += '.h = %d, .meanline = %d, .pad = %d, .nglyphs = %d,' % (height, meanline, pad, len(glyphs))
    out += '.glyphs = %s_glyphs };' % (name)
    return out

def generate_font(cps, glyph_factory, name, height, meanline, pad, *args, **kwargs):
    cps.sort()
    glyphs = []
    for code in cps:
        glyphs += [{'code':code, 'data':glyph_factory(code, *args, **kwargs)}]
    bitmaps = pack_glyphs(glyphs)
    return generate_resource(name, glyphs, bitmaps, height, meanline, pad)
