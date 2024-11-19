import cv2
import numpy as np
import os

# 确保目录存在
output_directory = r'C:\A_RESEARCH\Off_Line\C\LPLC2\synthetic_vedios\Vedio_basic_test'
if not os.path.exists(output_directory):
    os.makedirs(output_directory)

# 视频参数
width, height = 100, 100
fps = 30
total_frames = 50
output_file = os.path.join(output_directory, 'test_translating.avi')

# 创建一个视频写入对象
fourcc = cv2.VideoWriter_fourcc(*'XVID')  # 使用 XVID 编码
video_writer = cv2.VideoWriter(output_file, fourcc, fps, (width, height))

# 黑色物体的初始大小和位置
rect_width, rect_height = 20, 40
start_x = 0
start_y = (height - rect_height) // 2
move_per_frame = 2

# 创建视频帧
for i in range(total_frames):
    # 创建纯白色背景
    frame = np.ones((height, width, 3), dtype=np.uint8) * 255
    
    # 计算当前帧中矩形的左上角和右下角坐标
    top_left = (start_x + i * move_per_frame, start_y)
    bottom_right = (top_left[0] + rect_width, top_left[1] + rect_height)
    
    # 画出黑色矩形
    cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)
    
    # 写入帧到视频
    video_writer.write(frame)

# 释放视频写入对象
video_writer.release()
