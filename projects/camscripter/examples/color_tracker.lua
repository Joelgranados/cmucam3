camera_set_resolution(CC3_CAMERA_RESOLUTION_LOW);

print("Starting color tracking!");
tkr = tracker_new(195, 0, 0, 255, 45, 45); -- red square
--tkr2 = tracker_new(35, 100, 95, 85, 155, 145); -- blue square
while(true) do
	take_picture();
	tracker_track_color(tkr);
	--tracker_track_color(tkr2);
	clear_graphics();
	print_color_tracker(tkr);
	--print_color_tracker(tkr2);
	wait(25);
end