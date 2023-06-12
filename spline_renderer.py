from PIL import Image, ImageDraw

LOCKED = 0
TRACKING = 1

# Renders the files produced by random_spline_chain.c into videos of the  developing spline.

# The mximum number of frames to render.
frames = 24*96

# Change in the x value up to which the spline is drawn per frame.
dx = 0.008

# Number of line segments to evaluate per frame.
sub_steps = 2

# Fraction of the distance between the camera and the function that the camera moves towards each frame.
cam_t = 0.04

# Camera view width and height.
cam_w = 1.9
cam_h = 1

# Factor to scale the result down by before saving.
supersampled = 2

# Width and height of the output frames in pixels.
out_w = 1920 * supersampled
out_h = 1080 * supersampled

# Whether the camera moves up and down to follow the curve.
# LOCKED: The camera's y position never changes.
# TRACKING: The camera's y position follows the curve.
y_track_mode = TRACKING

line_width = 6

point_radius = 9

# ---- End Parameters ---- #

# Read the points and coefficients from file.
with open("spline.txt", "r") as fin:
	data = fin.read().split("\n")

# Split the data into two lists.
points = []
coefs = []

points.append(data[0].split(" "))
for l in range(1, len(data[1:])):
	if l % 2 == 1:
		coefs.append(data[l].split(" "))
	else:
		points.append(data[l].split(" "))

# Convert the data into numbers
for i, p in enumerate(points):
	for j, v in enumerate(p):
		points[i][j] = float(v)

for i, c in enumerate(coefs):
	for j, v in enumerate(c):
		coefs[i][j] = float(v)

cx = points[0][0]
cy = points[0][1]
x = 0
p_i = 0

# Holds x, y pairs where the spline has been evaluated, for drawing.
lines = [(points[0][0], points[0][1])]

# Holds the index of the segment that line "i" is in.
seg_i = [0]

for f in range(frames):
	print("%.2f < %.2f < %.2f (%.2f%%)" % (points[0][0], x, points[-1][0], ((x - points[0][0]) / (points[-1][0] - points[0][0]) * 100)))
	
	for s in range(sub_steps):
		# Increase x, evaluate the spline at x to get y,
		# and add the point to the lines list.
		x += dx / sub_steps
		
		if x > points[p_i+1][0]:
			p_i+=1
			
			if p_i == len(points) - 1:
				exit()
		
		y = coefs[p_i][0] * x**3 + coefs[p_i][1] * x**2 + coefs[p_i][2] * x + coefs[p_i][3]
		
		# Move the camera
		cx = (x - cx) * cam_t + cx
		if y_track_mode == TRACKING:
			cy = (y - cy) * cam_t + cy
		
		lines.append((x, y))
		seg_i.append(p_i)
	
	# The lines array in screen space. Lines that are off-screen are skipped.
	s_lines = []
	for l in lines:
		if (l[0] > cx - cam_w / 2 - 20 and l[0] < cx + cam_w / 2 + 20):
			s_x = int((l[0] - cx) / cam_w * out_w + out_w / 2)
			s_y = int((l[1] - cy) / cam_h * out_h + out_h / 2)
			
			s_lines.append( (s_x, s_y) )
	
	# The points array in screen space. Points that are off-screen are skipped.
	s_points = []
	for p in points:
		if (p[0] > cx - cam_w / 2 - 10 and p[0] < cx + cam_w / 2 + 10):
			s_x = int((p[0] - cx) / cam_w * out_w + out_w / 2)
			s_y = int((p[1] - cy) / cam_h * out_h + out_h / 2)
			
			s_points.append( (s_x, s_y) )
	
	im = Image.new("RGB", (out_w, out_h), (226, 220, 205))
	drw = ImageDraw.Draw(im)
	
	for i in range(1, len(s_lines)):
		drw.line((s_lines[i-1], s_lines[i]), (10, 10, 10), line_width)
	
	for p in s_points:
		drw.ellipse((p[0] - point_radius, p[1] - point_radius, p[0] + point_radius, p[1] + point_radius), (10, 10, 10))
	
	im = im.resize((int(out_w/supersampled), int(out_h/supersampled)))
	
	im.save("out/%04d.png" % f)
