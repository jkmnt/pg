from Tkinter import *
import socket
import threading
import Image, ImageFile, ImageTk, ImageDraw, ImageOps
import Queue

BORD_W = 1
LCD_CHAR_COLOR = (0, 0, 0)
LCD_BACKLIGHT_COLOR = (240, 240, 240)
#LCD_BACKLIGHT_COLOR = (220, 220, 255)
#LCD_BACKLIGHT_COLOR = (255, 217, 122)

BORD_COLOR = (0, 127, 0)
TITLE_PREFIX = 'LCD emulator'
MAGNIFY = 2

root = Tk()
root.title(TITLE_PREFIX)
Lbl0 = Label(root)
Lbl0.pack()
if MAGNIFY:
    Lbl1 = Label(root)
    Lbl1.pack()

Mq = Queue.Queue()

#def close_handler ():
#    root.destroy() ;
#    exit() ;

#root.protocol("WM_DELETE_WINDOW", close_handler)

def process_bmp ( Data):
    Size = [0,0]    
    Size[0] = ord(Data[0]) | (ord(Data[1]) << 8)
    Size[1] = ord(Data[2]) | (ord(Data[3]) << 8)
    Im = Image.fromstring( "1",  (Size[1], Size[0]), Data[4:], "raw", "1;IR", 0, 1)

    Im = Im.rotate(90).transpose(Image.FLIP_TOP_BOTTOM).convert('L')
    Im = ImageOps.colorize( Im, LCD_CHAR_COLOR, LCD_BACKLIGHT_COLOR)
    Size = (Im.size[0] + BORD_W*2, Im.size[1] + BORD_W*2)             # Add space for border
    newIm = Image.new( 'RGBA', Size)
    newIm.paste( Im, (1,1))
    del Im
    Draw = ImageDraw.Draw(newIm)
    Draw.rectangle([(0,0), (Size[0] - 1, Size[1] - 1)], outline = BORD_COLOR)
    del Draw    
    return newIm


def make_tk_bitmap( Im, Scale):            
    Bmp = ImageTk.PhotoImage( Im.resize( (Im.size[0] * Scale, Im.size[1] * Scale)))
    return Bmp

def attach_bmp ( Obj, Bmp):
    Obj.image = Bmp
    Obj.config( image = Bmp)


def check_bmp_update():    
    global Mq    
    if Mq.qsize():
        Data = Mq.get()
        Im = process_bmp( Data)
        del Data
                              
        root.title( TITLE_PREFIX + ' (%d x %d)' % (Im.size[1] - BORD_W*2 , Im.size[0] - BORD_W*2))        
    
        attach_bmp( Lbl0, make_tk_bitmap( Im, 1))
        if MAGNIFY:
            attach_bmp( Lbl1, make_tk_bitmap( Im, MAGNIFY))               
    root.after( 5,check_bmp_update)

def get_size(Data):
    Size = [0,0]    
    Size[0] = ord(Data[0]) | (ord(Data[1]) << 8)
    Size[1] = ord(Data[2]) | (ord(Data[3]) << 8)
    return (Size[0], Size[1])

def get_new_bmp (sock):
    # There could be delay in socket, and data may be splitted 
    try:
        indata = sock.recv(4096)    
    except:
        return None           
    expect = get_size(indata)
    expect = expect[0] * (expect[1] / 8) + 4    
    expect -= len(indata);

    while expect:        
        try:
            new_data = sock.recv(4096)
        except:
            return None                       
        expect -= len(new_data)
        indata += new_data
        assert expect >= 0, 'Negative expect!'        

    return indata

def open_socket():
    HOST = '127.0.0.1'        # Local host
    PORT = 50007              # Arbitrary port
    s = socket.socket( socket.AF_INET, socket.SOCK_STREAM)
    s.bind((HOST, PORT))
    s.listen(1)
    
    conn, addr = s.accept()
    return conn

check_bmp_update()
guiThread = threading.Thread( None, root.mainloop)
guiThread.start()

while 1:
    sock = open_socket()
    print 'Waiting'
    while 1:             
        Bmp = get_new_bmp(sock)
        if not Bmp:
            print 'Closed'
            sock.close()
            break
        Mq.put(Bmp)
