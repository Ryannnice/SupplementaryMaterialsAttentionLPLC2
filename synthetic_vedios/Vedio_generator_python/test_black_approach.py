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
output_file = os.path.join(output_directory, 'test_black_approach.avi')

# 创建一个视频写入对象
fourcc = cv2.VideoWriter_fourcc(*'XVID')  # 使用 XVID 编码
video_writer = cv2.VideoWriter(output_file, fourcc, fps, (width, height))

# 初始正方形大小和位置
square_size = 1
center = (width // 2, height // 2)

# 创建视频帧
for i in range(total_frames):
    # 创建纯白色背景
    frame = np.ones((height, width, 3), dtype=np.uint8) * 255
    
    # 计算当前帧中正方形的左上角和右下角坐标
    top_left = (center[0] - square_size // 2, center[1] - square_size // 2)
    bottom_right = (center[0] + square_size // 2, center[1] + square_size // 2)
    
    # 画出纯黑色正方形
    cv2.rectangle(frame, top_left, bottom_right, (0, 0, 0), -1)
    
    # 写入帧到视频
    video_writer.write(frame)
    
    # 更新正方形大小，每帧扩大1像素
    square_size += 2

    print('Frame', i, top_left, bottom_right, '\n')

# 释放视频写入对象
video_writer.release()
