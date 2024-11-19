# center : arrroach --> stop --> recede

import cv2
import os

# Path
image_path = r"C:\A_RESEARCH\Off_Line\C\LPLC2\synthetic_vedios\Vedio_dynamic_complex_bg\outdoor_2.jpg"
output_video = image_path.replace(".jpg", "_black_expand_recede_center.avi")

# Vedio properties
frame_width = 320
frame_height = 240
fps = 30


img = cv2.imread(image_path)

#  Raw img operation
img = img[5:245, :]

img_height, img_width = img.shape[:2]

fourcc = cv2.VideoWriter_fourcc(*'XVID')
video_writer = cv2.VideoWriter(output_video, fourcc, fps, (frame_width, frame_height))

# Bg moving speed
scroll_speed = 1  

# Square size
max_square_size = 80 # size of a single RF
square_size = 0

# Object appear time (frame)
time_appear = 0 
time_stop = 40
time_recede = 60
time_disappear = 100



for start_x in range(0, 110 , scroll_speed):

    frame_count = start_x


    frame = img[:, start_x:start_x + frame_width].copy()
    
    # Add Square to Frame
    if frame_count < time_stop :
        # 计算正方形的左上角和右下角坐标
        center_x, center_y = frame_width // 2, frame_height // 2
        top_left = (center_x - square_size // 2, center_y - square_size // 2)
        bottom_right = (center_x + square_size // 2, center_y + square_size // 2)

        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)

        # Square expanding speed
        square_size += 2

    if frame_count >= time_stop and frame_count < time_recede :
        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)

    if frame_count >= time_recede and frame_count < time_disappear : 
        center_x, center_y = frame_width // 2, frame_height // 2
        top_left = (center_x - square_size // 2, center_y - square_size // 2)
        bottom_right = (center_x + square_size // 2, center_y + square_size // 2)

        cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)

        # Square expanding speed
        square_size -= 2



    video_writer.write(frame)

video_writer.release()
cv2.destroyAllWindows()

print(f'\n\nOutput Vedio to:   {output_video}\n\n')
