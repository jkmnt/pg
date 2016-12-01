# python
import glob
import fcnv2
import sys
import os

SIZES = [10, 12, 14, 16, 24]

for file in glob.glob('*.ttf'):
    basename = os.path.splitext(file)[0]
    for size in SIZES:
        name = basename + '_' + str(size) + '.h'
        print 'Converting %s' % name
        fcnv2.main([file, name, size])

