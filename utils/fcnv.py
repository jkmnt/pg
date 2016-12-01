#
# Diz
#
import sys, os
import Image, ImageFont, ImageDraw
from struct import *

DEF_SIZE = 8
FIRST_CHAR = ord(' ')
LAST_CHAR = ord('~')
X_SYMBOL = 'x'

def fcnv ( inFile, outFile, Size = DEF_SIZE, charRange = ( FIRST_CHAR, LAST_CHAR)):

    Fnt = ImageFont.truetype( inFile, Size) 

    imgName = os.path.splitext(os.path.basename( outFile))[0]

    # Collect widths of all chars
    St = ''
    wSizes = []
    hSizes = []
    for i in range(charRange[0], charRange[1] + 1):
        St = St + chr(i)
        charSize = Fnt.getsize( chr(i))
        wSizes.append( charSize[0]) 
        hSizes.append( charSize[1])
    
    # Box of largest char
    charBox = (max( wSizes), max( hSizes))
    # Get image dimensions
    imgBox = ((charRange[1] - charRange[0] + 1) * charBox[0], charBox[1])
    
    Img = Image.new("1", imgBox, 0) 
    Draw = ImageDraw.Draw(Img)
    
    # Print all chars
    for i in range(charRange[0], charRange[1] + 1):
        Draw.text( ( (i - charRange[0]) * charBox[0], 0), chr(i), font = Fnt, fill = 1)
    del Draw
    
    # Bounding box for image
    Bbox = Img.getbbox()
    # new charBox, exactly fitting largest char
    cropBox = ( charBox[0], Bbox[3] - Bbox[1])
    
    # Crop image
    Img = Img.crop( ( 0, Bbox[1], imgBox[0], Bbox[3] ))
    Img = Img.rotate(-90).transpose( Image.FLIP_LEFT_RIGHT)
    
    # Get xbm string
    Xbm = Img.tobitmap( imgName)    

    # Get x-height, baseline and middleline positions
    Img = Image.new("1", charBox, 0);
    Draw = ImageDraw.Draw(Img)
    Draw.text( (0,0), X_SYMBOL, font = Fnt, fill = 1)
    del Draw
    xBox = Img.getbbox()
    del Img

    Baseline = xBox[3] - Bbox[1] - 1                       # Get vertical offset of lowest pixel in 'x'
    Meanline = Baseline - (xBox[3] - xBox[1] - 1)/2        # Shift up to the middle of 'x'
       
    # Font should be const to hopefully go to program memory
    Xbm = Xbm.replace( 'static char %s_bits' % imgName, 'static const uchar %s_bits' % imgName)
    
    F = open( outFile, 'w')

    F.write('/** Converted by fconv.py from %s (size %d) */\n\n' % (inFile, Size))

    F.write(Xbm)

    F.write( '''

#define %s_font_w %d
#define %s_font_h %d
#define %s_font_asciioffset %d
#define %s_font_baseline %d
#define %s_font_meanline %d
    ''' % (imgName, cropBox[0], imgName, cropBox[1], imgName, charRange[0], imgName, Baseline, imgName, Meanline))
    
    F.write( '\nstatic const uchar %s_font_spaces[] = {\n' % imgName)
    Str = ''.join(['%d, ' % i for i in wSizes ])
    F.write( Str + '};')
    F.close()

def main ():
    print 'Font conversion utility for pg lib'

    Args = sys.argv[1:]
    aLen = len(Args)
    if aLen !=3:
        print 'usage: python fcnv.py fontname.ttf outfile fontsize'
        sys.exit(-1)    

    inFile, outFile, Size = Args[0:3]

    if inFile == outFile:
        print 'Files should be different'
        sys.exit(-1)

    fcnv( inFile, outFile, int(Size))

#    inFilename, outFilename, outXmlFilename, outPdfFilename = Args[0:4]


#Main
if __name__ == '__main__':
    main()
    print 'Done'
