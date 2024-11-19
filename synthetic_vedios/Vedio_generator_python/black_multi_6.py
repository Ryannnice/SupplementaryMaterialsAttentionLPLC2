import cv2
import os

# Path
image_path = r"C:\A_RESEARCH\Off_Line\C\LPLC2\synthetic_vedios\Vedio_dynamic_complex_bg\outdoor_3.jpg"
output_video = image_path.replace(".jpg", "_black_expand_multi_6.avi")

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
square_size_1 = 0
square_size_2 = 0
square_size_3 = 0
square_size_4 = 0
square_size_5 = 0
square_size_6 = 0

# Object appear time (frame)
time_appear_object_1 = 5
time_appear_object_2 = 15 
time_appear_object_3 = 25
time_appear_object_4 = 45
time_appear_object_5 = 55 
time_appear_object_6 = 65

time_disappear_object_1 = 55
time_disappear_object_2 = 65
time_disappear_object_3 = 75
time_disappear_object_4 = 95
time_disappear_object_5 = 105
time_disappear_object_6 = 115

time_end = 120

for start_x in range(0, time_end, scroll_speed):
    frame_count = start_x
    frame = img[:, start_x:start_x + frame_width].copy()

    # Object 1 
    if frame_count >= time_appear_object_1 and frame_count < time_disappear_object_1:
        center_x1, center_y1 = frame_width * 3 // 4, frame_height // 4
        top_left1 = (center_x1 - square_size_1 // 2, center_y1 - square_size_1 // 2)
        bottom_right1 = (center_x1 + square_size_1 // 2, center_y1 + square_size_1 // 2)
        cv2.rectangle(frame, top_left1, bottom_right1, (0, 0, 0), -1)
        if square_size_1 < max_square_size:
            square_size_1 += 2

    # Object 2
    if frame_count >= time_appear_object_2 and frame_count < time_disappear_object_2:
        center_x2, center_y2 = frame_width // 4, frame_height // 2
        top_left2 = (center_x2 - square_size_2 // 2, center_y2 - square_size_2 // 2)
        bottom_right2 = (center_x2 + square_size_2 // 2, center_y2 + square_size_2 // 2)
        cv2.rectangle(frame, top_left2, bottom_right2, (0, 0, 0), -1)
        if square_size_2 < max_square_size:
            square_size_2 += 2

    # Object 3
    if frame_count >= time_appear_object_3 and frame_count < time_disappear_object_3:
        center_x3, center_y3 = frame_width * 3 // 5, frame_height * 3 // 4
        top_left3 = (center_x3 - square_size_3 // 2, center_y3 - square_size_3 // 2)
        bottom_right3 = (center_x3 + square_size_3 // 2, center_y3 + square_size_3 // 2)
        cv2.rectangle(frame, top_left3, bottom_right3, (0, 0, 0), -1)
        if square_size_3 < max_square_size:
            square_size_3 += 2

    # Object 4 (Mirror of Object 1)
    if frame_count >= time_appear_object_4 and frame_count < time_disappear_object_4:
        center_x4, center_y4 = frame_width // 4, frame_height // 4  # Mirror center_x1
        top_left4 = (center_x4 - square_size_4 // 2, center_y4 - square_size_4 // 2)
        bottom_right4 = (center_x4 + square_size_4 // 2, center_y4 + square_size_4 // 2)
        cv2.rectangle(frame, top_left4, bottom_right4, (0, 0, 0), -1)
        if square_size_4 < max_square_size:
            square_size_4 += 2

    # Object 5 (Mirror of Object 2)
    if frame_count >= time_appear_object_5 and frame_count < time_disappear_object_5:
        center_x5, center_y5 = frame_width * 3 // 4, frame_height // 2  # Mirror center_x2
        top_left5 = (center_x5 - square_size_5 // 2, center_y5 - square_size_5 // 2)
        bottom_right5 = (center_x5 + square_size_5 // 2, center_y5 + square_size_5 // 2)
        cv2.rectangle(frame, top_left5, bottom_right5, (0, 0, 0), -1)
        if square_size_5 < max_square_size:
            square_size_5 += 2

    # Object 6 (Mirror of Object 3)
    if frame_count >= time_appear_object_6 and frame_count < time_disappear_object_6:
        center_x6, center_y6 = frame_width * 2 // 5, frame_height * 3 // 4  # Mirror center_x3
        top_left6 = (center_x6 - square_size_6 // 2, center_y6 - square_size_6 // 2)
        bottom_right6 = (center_x6 + square_size_6 // 2, center_y6 + square_size_6 // 2)
        cv2.rectangle(frame, top_left6, bottom_right6, (0, 0, 0), -1)
        if square_size_6 < max_square_size:
            square_size_6 += 2

    video_writer.write(frame)

video_writer.release()
cv2.destroyAllWindows()




#def print_all_objects_info():
center_positions = [
        (320 * 3 // 4, 240 // 4),  # Object 1
        (320 // 4, 240 // 2),      # Object 2
        (320 * 3 // 5, 240 * 3 // 4),  # Object 3
        (320 // 4, 240 // 4),      # Object 4 (mirror 1)
        (320 * 3 // 4, 240 // 2),  # Object 5 (mirror 2)
        (320 * 2 // 5, 240 * 3 // 4)   # Object 6 (mirror 3)
    ]



time_appear_object_1 = 5
time_appear_object_2 = 15 
time_appear_object_3 = 25
time_appear_object_4 = 45
time_appear_object_5 = 55 
time_appear_object_6 = 65
#colors = ['#1f77b4', '#ff7f0e', '#2ca02c', '#d62728', '#9467bd', '#8c564b']

for i, (center_x, center_y) in enumerate(center_positions, start=1):
    print(f"Object {i}  Initial Position: ({center_x}, {center_y})")



print(f'\n\nOutput Video to:   {output_video}\n\n')
