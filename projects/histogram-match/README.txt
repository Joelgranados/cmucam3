About
---------

This is a sample program that shows how you can use histogram matching to
distinguish between images.  This is a very simple form of template based
object recognition.  In order for this approach to work well the scene needs
to have consistent positioning as well as constant lighting. These types of
strict assumptions are typically found in inspection applications where the
camera, objects and lighting are all fixed.


Operation
---------
Load the histogram-match firmware.  The firmware is set to grab the first
TEMPLATE_IMGS (by default 3 for the LEDs) images as template images to 
compare with future images.  Upon startup, press the button each time you 
want to train a new template image.  Once all templates have been filled, 
the program will continously grab frames and compare them to the templates. 

By default the region of interest in the image is set to (160, 50, 200, 240)
but can be easily changed.

