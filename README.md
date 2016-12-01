# pg
GUI library for the small embedded devices

This repository is a dump of old, but still useful library. Do not expect to find shiny commented code, documentation and examples.

Library was designed for a simple windowed GUIs, the ones used in coffemachines, instruments, old cellphones.
Rendering is deferred - client creates and destroys objects, changes properties of existing objects.
Later client explicitly calls render(), library diffs changes and posts bitmap to the framebuffer.
It is the client duty to transfer framebuffer to the actual hardware.

Input (buttons/touch/etc) is a duty of client. Library duty is output (think View in the MVC model).

Features:
* C99 without external dependencies
* amiga-like vararg interface to the objects settings
* overlapping rectangular windows with z-order rendering
* proportional fonts (generated from ttf by the python script)
* bitmaps (generated from images by the python script)
* animated bitmaps (generated from gifs by the python script)
* unicode support
* a number of basic widgets - icon, textlabel, listbox, texteditor, progress bar, etc.
