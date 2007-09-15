----------------------------- security_cam.lua --------------------------------
-- Script to make the CMUcam3 perform as a security camera.                  --
-- Continually take pictures, comparing each picture to the previous picture --
-- using our framediff_scanline functions. If there is a noticeable enough   --
-- difference between the two pictures, store the new (different) picture on --
-- the MMC card to be looked at later.  In debug mode, the image will also   --
-- be sent through the serial port and displayed in Eclipse.                 --
-------------------------------------------------------------------------------
THRESHOLD = 10; -- threshold to determine if new pic should be stored
MAX_IMAGES = 100; -- Max number of images to be stored on the MMC card
take_picture(); -- load pixbuf so we can use width, height
wait(1000);
camera_set_resolution(0);
-- get our objects
img = image_new(pixbuf_get_width(), 1);
fd = framediff_new(16, 16);

fd:set_coi(CC3_CHANNEL_SINGLE);
pixbuf_set_coi(fd:get_coi());
fd:set_total_x(pixbuf_get_width());
fd:set_total_y(pixbuf_get_height());
fd:set_load_frame(true);
fd:set_threshold(THRESHOLD);
img:set_channels(1);

image_count = 0;
file_count = 0;
repeat
	print("start of loop");
	wait(2000); -- give the change time (2 secs) to stabilize
	take_picture();
	fd:set_load_frame(true);
	if (framediff_scanline_start(fd) > 0) then
		while pixbuf_read_rows(img, 1) > 0 do
			framediff_scanline(img, fd);		
		end
		framediff_scanline_finish(fd);
	else 
		print("frame diff startup error");
	end
	
	fd:set_load_frame(false);

	-- keep taking pictures until we find enough difference
	repeat
		take_picture();
		if (framediff_scanline_start(fd) > 0) then
			while pixbuf_read_rows(img, 1) > 0 do
				framediff_scanline(img, fd);		
			end
			framediff_scanline_finish(fd);
		else 
			print("Frame Diff start Error #2!");
		end
		
		fd:swap_templates(); -- put current template in previous template spot
		print("diff from last frame: " .. fd:get_num_pixels());
	until fd:get_num_pixels() >= fd:get_threshold(); 
	
	print("Change detected!");
	-- get a filename that doesn't overwrite existing files
	repeat 
		filename = "ScCam" .. file_count .. ".ppm";
		file_count = file_count + 1;
	until (	not file_exists(filename));

	pixbuf_set_coi(CC3_CHANNEL_ALL); -- store/send pictures with all colors
	image_count =  image_count + 1;
	print_picture(); -- send pic back to CamScripter debugger
	print("saving picture " .. filename);
	save_picture(filename); -- save pic on the SD/MMC card
	pixbuf_set_coi(1);
until image_count >= MAX_IMAGES;

img:dispose();
fd:dispose();