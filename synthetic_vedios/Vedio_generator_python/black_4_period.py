import cv2
import os

# Path
image_path = r"C:\A_RESEARCH\Off_Line\C\LPLC2\synthetic_vedios\Vedio_dynamic_complex_bg\outdoor_2.jpg"
output_video = image_path.replace(".jpg", "_black_4_period.avi")

# Video properties
frame_width = 320
frame_height = 240
fps = 30

img = cv2.imread(image_path)

# Raw img operation
img = img[5:245, :]

img_height, img_width = img.shape[:2]

fourcc = cv2.VideoWriter_fourcc(*'XVID')
video_writer = cv2.VideoWriter(output_video, fourcc, fps, (frame_width, frame_height))

# Bg moving speed
scroll_speed = 1  

# Square size
max_square_size = 80  # size of a single RF
square_size = 0

# Object appear time (frame)
period_1_appear = 0 
period_1_stop = 40
period_1_recede = 60
period_1_disappear = 100  

period_2_appear = 100 
period_2_stop = 140
period_2_recede = 160
period_2_disappear = 200  

period_3_appear = 200 
period_3_stop = 240
period_3_recede = 260
period_3_disappear = 300      

period_4_appear = 300 
period_4_stop = 340
period_4_recede = 360
period_4_disappear = 400

# Main loop to generate video frames
for frame_count in range(0, period_4_disappear, scroll_speed):
    frame = img[:, frame_count:frame_count + frame_width].copy()
    
    # Period 1
    if frame_count < period_1_stop:
        center_x, center_y = frame_width // 2, frame_height // 2
        top_left = (center_x - square_size // 2, center_y - square_size // 2)
        bottom_right = (center_x + square_size // 2, center_y + square_size // 2)
        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)
        square_size += 2

    elif period_1_stop <= frame_count < period_1_recede:
        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)

    elif period_1_recede <= frame_count < period_1_disappear:
        square_size -= 2
        top_left = (center_x - square_size // 2, center_y - square_size // 2)
        bottom_right = (center_x + square_size // 2, center_y + square_size // 2)
        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)

    # Period 2: Object starts from the center of the first quadrant
    if period_2_appear <= frame_count < period_2_stop:
        center_x, center_y = frame_width - (frame_width // 4), frame_height // 4  # First quadrant center
        top_left = (center_x - square_size // 2, center_y - square_size // 2)
        bottom_right = (center_x + square_size // 2, center_y + square_size // 2)
        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)
        square_size += 2

    elif period_2_stop <= frame_count < period_2_recede:
        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)

    elif period_2_recede <= frame_count < period_2_disappear:
        square_size -= 2
        top_left = (center_x - square_size // 2, center_y - square_size // 2)
        bottom_right = (center_x + square_size // 2, center_y + square_size // 2)
        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)

    # Period 3: Object starts from the center of the third quadrant
    if period_3_appear <= frame_count < period_3_stop:
        center_x, center_y = frame_width // 4 , frame_height - (frame_height // 4)  # Third quadrant center
        top_left = (center_x - square_size // 2, center_y - square_size // 2)
        bottom_right = (center_x + square_size // 2, center_y + square_size // 2)
        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)
        square_size += 2

    elif period_3_stop <= frame_count < period_3_recede:
        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)

    elif period_3_recede <= frame_count < period_3_disappear:
        square_size -= 2
        top_left = (center_x - square_size // 2, center_y - square_size // 2)
        bottom_right = (center_x + square_size // 2, center_y + square_size // 2)
        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)

    # Period 4: Two objects from the centers of the second and fourth quadrants
    if period_4_appear <= frame_count < period_4_stop:
        # Second quadrant
        center_x_1, center_y_1 = frame_width // 4, frame_height // 4  # Second quadrant center
        top_left_1 = (center_x_1 - square_size // 2, center_y_1 - square_size // 2)
        bottom_right_1 = (center_x_1 + square_size // 2, center_y_1 + square_size // 2)
        cv2.rectangle(frame, top_left_1, bottom_right_1, (0, 0, 0), -1)
        #square_size += 2

        # Fourth quadrant
        center_x_2, center_y_2 = frame_width - (frame_width // 4), frame_height - (frame_height // 4)  # Fourth quadrant center
        top_left_2 = (center_x_2 - square_size // 2, center_y_2 - square_size // 2)
        bottom_right_2 = (center_x_2 + square_size // 2, center_y_2 + square_size // 2)
        cv2.rectangle(frame, top_left_2, bottom_right_2, (0, 0, 0), -1)
        square_size += 2

    elif period_4_stop <= frame_count < period_4_recede:
        cv2.rectangle(frame, top_left_1, bottom_right_1, (0, 0, 0), -1)
        cv2.rectangle(frame, top_left_2, bottom_right_2, (0, 0, 0), -1)

    elif period_4_recede <= frame_count < period_4_disappear:
        square_size -= 2
        top_left_1 = (center_x_1 - square_size // 2, center_y_1 - square_size // 2)
        bottom_right_1 = (center_x_1 + square_size // 2, center_y_1 + square_size // 2)
        cv2.rectangle(frame, top_left_1, bottom_right_1, (0, 0, 0), -1)

        top_left_2 = (center_x_2 - square_size // 2, center_y_2 - square_size // 2)
        bottom_right_2 = (center_x_2 + square_size // 2, center_y_2 + square_size // 2)
        cv2.rectangle(frame, top_left_2, bottom_right_2, (0, 0, 0), -1)

    # Write the frame to video
    video_writer.write(frame)

# Release resources
video_writer.release()
cv2.destroyAllWindows()

print(f'\n\nOutput Video to: {output_video}\n\n')
