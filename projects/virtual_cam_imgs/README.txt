virtual-hal
===========

This directory contains sample images that will be feed into the virtual camera frame buffer.  The images need to be in PPM format and number "IMGxxxxx.PPM" where the x's represent the frame number.  The virtual-cam hal will read in images each time cc3_load_frame() is called in increasing order starting from "IMG00000.PPM". Once no more images are available, the virtual-hal will panic and stop. 

