clear_graphics();

--tkr = tracker_new(195, 0, 0, 255, 45, 45); -- red square
tkr = tracker_new(35, 100, 95, 85, 155, 145); -- blue square

tkr:set_noise_filter(2);
pixbuf_set_coi(CC3_CHANNEL_ALL);

take_picture();
tracker_track_color(tkr);
print_color_tracker(tkr);

print("printing picture...");
print_picture();