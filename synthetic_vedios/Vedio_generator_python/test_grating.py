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
output_file = os.path.join(output_directory, 'test_grating.avi')

# 创建一个视频写入对象
fourcc = cv2.VideoWriter_fourcc(*'XVID')  # 使用 XVID 编码
video_writer = cv2.VideoWriter(output_file, fourcc, fps, (width, height))

# 光栅的宽度和运动速度
grating_width = 20
move_per_frame = 1

# 创建视频帧
for i in range(total_frames):
    # 创建纯白色背景
    frame = np.ones((height, width, 3), dtype=np.uint8) * 255
    
    # 计算光栅的起始位置（从左侧不断进入画面）
    start_x = (i * move_per_frame) % (grating_width * 2)
    for x in range(-grating_width + start_x, width, grating_width * 2):
        # 画出黑色光栅
        cv2.rectangle(frame, (x, 0), (x + grating_width, height), (0, 0, 0), -1)
    
    # 写入帧到视频
    video_writer.write(frame)

# 释放视频写入对象
video_writer.release()
