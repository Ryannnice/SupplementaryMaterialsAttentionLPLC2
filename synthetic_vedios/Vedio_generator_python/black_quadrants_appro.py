import cv2
import os

# Path
image_path = r"C:\A_RESEARCH\Off_Line\C\LPLC2\synthetic_vedios\Vedio_dynamic_complex_bg\outdoor_2.jpg"
output_video = image_path.replace(".jpg", "_black_expand_four_quadrants.avi")

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
max_square_size = 80
square_size = 0

# Object appear time (frame)
time_appear_quadrant_1 = 0 
time_appear_quadrant_2 = 50 
time_appear_quadrant_3 = 100 
time_appear_quadrant_4 = 150
time_end = 200

for start_x in range(0, time_end, scroll_speed):
    frame_count = start_x
    frame = img[:, start_x:start_x + frame_width].copy()

    # Quadrant 1
    if frame_count < time_appear_quadrant_2 - 10:
        center_x, center_y = frame_width * 3 // 4, frame_height // 4
        top_left = (center_x - square_size // 2, center_y - square_size // 2)
        bottom_right = (center_x + square_size // 2, center_y + square_size // 2)

        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)
        square_size += 2

    elif frame_count >= time_appear_quadrant_2 - 10 and frame_count < time_appear_quadrant_2:
        # Ensure top_left and bottom_right are updated for Quadrant 1
        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)

    # Quadrant 2
    elif frame_count >= time_appear_quadrant_2 and frame_count < time_appear_quadrant_3 - 10:
        if frame_count == time_appear_quadrant_2 :
            square_size = 0
        center_x, center_y = frame_width // 4, frame_height // 4
        top_left = (center_x - square_size // 2, center_y - square_size // 2)
        bottom_right = (center_x + square_size // 2, center_y + square_size // 2)

        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)
        square_size += 2

    elif frame_count >= time_appear_quadrant_3 - 10 and frame_count < time_appear_quadrant_3:
        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)

    # Quadrant 3
    elif frame_count >= time_appear_quadrant_3 and frame_count < time_appear_quadrant_4 - 10:
        if frame_count == time_appear_quadrant_3 :
            square_size = 0
        center_x, center_y = frame_width // 4, frame_height * 3 // 4
        top_left = (center_x - square_size // 2, center_y - square_size // 2)
        bottom_right = (center_x + square_size // 2, center_y + square_size // 2)

        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)
        square_size += 2

    elif frame_count >= time_appear_quadrant_4 - 10 and frame_count < time_appear_quadrant_4:
        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)

    # Quadrant 4
    elif frame_count >= time_appear_quadrant_4 and frame_count < time_end - 10:
        if frame_count == time_appear_quadrant_4 :
            square_size = 0
        center_x, center_y = frame_width * 3 // 4, frame_height * 3 // 4
        top_left = (center_x - square_size // 2, center_y - square_size // 2)
        bottom_right = (center_x + square_size // 2, center_y + square_size // 2)

        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)
        square_size += 2

    elif frame_count >= time_end - 10 and frame_count < time_end:
        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)





    video_writer.write(frame)

video_writer.release()
cv2.destroyAllWindows()

print(f'\n\nOutput Video to:   {output_video}\n\n')
