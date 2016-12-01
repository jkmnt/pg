import socket
import array
import threading
import random
import Image, ImageFile, ImageTk, ImageDraw, ImageOps, ImageChops, ImageFont

CMD_CLEAR = 0
CMD_SHOW = 1
CMD_SET_CLIP = 2
CMD_DRAW_VLINE  = 3
CMD_DRAW_HLINE  = 4
CMD_DRAW_LINE = 5
CMD_DRAW_FILL = 6
CMD_DRAW_FRAME = 7
CMD_DRAW_TEXTLINE = 8

def get_size(Data):
    Size = [0,0]    
    Size[0] = ord(Data[0]) | (ord(Data[1]) << 8)
    Size[1] = ord(Data[2]) | (ord(Data[3]) << 8)
    return (Size[0], Size[1])

def unpack_image(Data):    
    size = get_size(Data)
    Im = Image.fromstring( "1",  (size[1], size[0]), Data[4:], "raw", "1;IR", 0, 1)
    Im = Im.rotate(90).transpose(Image.FLIP_TOP_BOTTOM)
    return Im

def connect():
    HOST = '127.0.0.1'        # Local host
    PORT = 50007              # Server port
    s = socket.socket( socket.AF_INET, socket.SOCK_STREAM)
    s.connect((HOST, PORT))
    return s

# cmd Id and args are 32-bits
def send_cmd(socket, cmdId, args = []):    
    cmdReq = array.array('l', [cmdId] + args)
    socket.send(cmdReq)

def get_ack(sock):
    ack = sock.recv(1024)    
    assert ack == 'Ok\0', 'ACK error !'

def draw_hline(img, args):
    draw = ImageDraw.Draw(img)    
    x0, x1, y0, y1 = args[0:3] + [args[2]]
    draw.line([x0, y0, x1, y1], fill = not args[3])
    del draw

def draw_vline(img, args):
    draw = ImageDraw.Draw(img)    
    x0, x1, y0, y1 = [args[0]] + args[0:3]
    draw.line([x0, y0, x1, y1], fill = not args[3])
    del draw

def draw_line(img, args):
    draw = ImageDraw.Draw(img)            
    draw.line(args[0:4], fill = not args[4])    
    del draw

def draw_fill(img, args):
    draw = ImageDraw.Draw(img)            
    draw.rectangle(args[0:4], fill = not args[4], outline = not args[4])    
    del draw

def draw_frame(img, args):
    draw = ImageDraw.Draw(img)            
    draw.rectangle(args[0:4], fill = 1, outline = not args[4])
    del draw

def draw_textline(img, args):
    fnt = ImageFont.truetype('f04b_11.ttf', 8) 
    draw = ImageDraw.Draw(img)    
    # This font have a different offset in TTF and xbm        
    draw.text([args[0], args[1] - 1 ], 'Test string', font = fnt, fill = not args[2])
    del draw

def compare_img(im0, im1):    
    return im0.convert('1').tostring() == im1.convert('1').tostring()

def reset_test(sock, clip = [0, 0, 250, 131]):
    send_cmd(sock, CMD_SET_CLIP, clip)
    get_ack(sock)
    send_cmd(sock, CMD_CLEAR)
    get_ack(sock)
    return Image.new( '1', (251, 160), color = 1)

def get_result(sock):
    send_cmd(sock, CMD_SHOW)    
    # There could be delay in socket, and data may be splitted 
    indata = sock.recv(8192)
    expect = get_size(indata)
    expect = expect[0] * (expect[1] / 8) + 4    
    expect -= len(indata);

    while expect:        
        new_data = sock.recv(2048)
        expect -= len(new_data)
        indata += new_data
        assert expect >= 0, 'Negative expect!'        
                
    return unpack_image(indata)

def test_vline(im, sock, args):
    send_cmd(sock, CMD_DRAW_VLINE, args)    
    get_ack(sock)
    draw_vline(im, args)

def test_hline(im, sock, args):
    send_cmd(sock, CMD_DRAW_HLINE, args)    
    get_ack(sock)
    draw_hline(im, args)

def test_fill(im, sock, args):
    send_cmd(sock, CMD_DRAW_FILL, args)    
    get_ack(sock)
    draw_fill(im, args)

def test_frame(im, sock, args):
    send_cmd(sock, CMD_DRAW_FRAME, args)    
    get_ack(sock)
    draw_frame(im, args)

def test_textline(im, sock, args):
    send_cmd(sock, CMD_DRAW_TEXTLINE, args)    
    get_ack(sock)
    draw_textline(im, args)

def test_line(im, sock, args):
    send_cmd(sock, CMD_DRAW_LINE, args)    
    get_ack(sock)
    x0, y0, x1, y1, c = args    
    dx = abs(x1 - x0)
    dy = abs(y1 - y0)

    if dy < dx:     
        if x0 < x1:
            if y0 < y1:
                octant = 1
            else:
                octant = 8
        else:
            if y0 < y1:
                octant = 4
            else:
                octant = 5
    else:
        if x0 < x1:
            if y0 < y1:
                octant = 2
            else:
                octant = 7
        else:
            if y0 < y1:
                octant = 3
            else:
                octant = 6

    if octant in [8, 3, 2, 1]:    
        draw_line(im, [x1, y1, x0, y0, c])
    else:
        draw_line(im, [x0, y0, x1, y1, c])

def test_hlines(sock):
    for x0 in range(250):
        for x1 in range(250):
            pim = reset_test(sock)
            res = test_hline(pim, sock, [x0, x1, random.randint(0, 130), 1])
            dut = get_result(sock)
            if not compare_img(pim, dut):
                return 'x0 = %d, x1 = %d' % (x0, x1)
    return 'Ok'

def test_vlines(sock):        
    for y0 in range(130):
        for y1 in range(130):
            pim = reset_test(sock)
            res = test_vline(pim, sock, [random.randint(0, 250), y0, y1, 1])
            dut = get_result(sock)                        
            if not compare_img(pim, dut):
                return 'y0 = %d y1 = %d' % (y0, y1)    
    return 'Ok'

def vdiff(im0, im1):
    im0 = ImageOps.colorize( im0.convert('L'), (255, 0, 0), (255, 255, 255))
    im1 = ImageOps.colorize( im1.convert('L'), (0, 255, 0), (255, 255, 255))            
    ImageChops.multiply(im0, im1).show()

def test_lines(sock):    
    for i in range(130*130):
        x0 = random.randint(0, 250)
        x1 = random.randint(0, 250)
        y0 = random.randint(0, 130)
        y1 = random.randint(0, 130)        
        pim = reset_test(sock)
        res = test_line(pim, sock, [x0, y0, x1, y1, 1])
        dut = get_result(sock)       
        if not compare_img(pim, dut):
            vdiff(pim, dut)
            return 'x0 = %d y0 = %d x1 = %d y1 = %d i = %d' % (x0, y0, x1, y1, i)
    return 'Ok'

def test_fills(sock):
    for y0 in range(130):
        for y1 in range(130):            
            x0 = random.randint(0, 250)
            x1 = random.randint(0, 250)
            pim = reset_test(sock)
            res = test_fill(pim, sock, [x0, y0, x1, y1, 1])
            dut = get_result(sock)            
            if not compare_img(pim, dut):
                return 'x0 = %d, y0 = %d, x1 = %d, y1 = %d' % (x0, y0, y1, y1)  
    return 'Ok'

def test_frames(sock):
    for y0 in range(130):
        for y1 in range(130):                    
            x0 = random.randint(0, 250)
            x1 = random.randint(0, 250)            
            # PIL generates no frame for x0 = x1. We do generate it.
            if x0 == x1:
                x1 += 1
            pim = reset_test(sock)
            res = test_frame(pim, sock, [x0, y0, x1, y1, 1])
            dut = get_result(sock)      
            if not compare_img(pim, dut):
                vdiff(pim, dut)
                return 'x0 = %d, y0 = %d, x1 = %d, y1 = %d' % (x0, y0, x1, y1)
    return 'Ok'

def test_textlines(sock):
    for x0 in range(250):
        for y0 in range(120):            
            pim = reset_test(sock)
            res = test_textline(pim, sock, [x0, y0, 1])
            dut = get_result(sock)
            if not compare_img(pim, dut):
                vdiff(pim, dut)
                return 'x0 = %d, y0 = %d' % (x0, y0)  
    return 'Ok'

ts = [
    ['hlines', test_hlines],
    ['vlines', test_vlines],
    ['lines', test_lines],
    ['fills', test_fills],
    ['frames', test_frames],
    ['textlines', test_textlines],
]

sock = connect()
failed = []

for test in ts:
    print 'Testing %s' % test[0]
    res = test[1](sock)
    if res != 'Ok':
        print '\tFailed: %s' % res
        failed.append([test[0], res])

if not failed:
    print 'All tests passed'
else:
    print '\nFailed tests:'
    for i in failed:
        print '%s (%s)' % (i[0], i[1])

sock.close()
