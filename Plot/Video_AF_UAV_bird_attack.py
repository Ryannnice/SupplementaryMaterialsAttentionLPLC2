import cv2
import numpy as np
import matplotlib.pyplot as plt

# 文件和路径定义
video_path = r'C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_dynamic_complex_bg\\standared_bird_attack.avi'
output_path = r'C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\Output_Vedio\\standared_bird_attack.avi'
txt_file_path = r'C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\Output_Txt\\bird_attack_Fatal.txt'

# 参数设置
box_radius = 75  # 正方形框的半径
total_frames = 27

# 读取文本文件内容
box_data = []
with open(txt_file_path, 'r') as file:
    for line in file:
        values = line.strip().split()
        x = int(float(values[2])) #      x  , from left to right
        y = int(float(values[1])) # Ordinate, from top to bottom
        frame_presence = [float(val) for val in values[5:]]  # 每帧的存在状态
        box_data.append((x, y, frame_presence))

# 定义颜色列表
colors = plt.cm.tab10.colors  # 使用 tab10 颜色表

# 打开视频进行读取
cap = cv2.VideoCapture(video_path)
fourcc = cv2.VideoWriter_fourcc(*'XVID')
fps = int(cap.get(cv2.CAP_PROP_FPS))
width = int(cap.get(cv2.CAP_PROP_FRAME_WIDTH))
height = int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))
out = cv2.VideoWriter(output_path, fourcc, fps, (width, height))

frame_index = 0
while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    # 检查每个框在当前帧是否存在，并绘制对应颜色的框
    for i, (x, y, frame_presence) in enumerate(box_data):
        if frame_index < len(frame_presence) and frame_presence[frame_index] != 0:
            # y: from top to bottom
            top_left = (x - box_radius, y - box_radius)
            bottom_right = (x + box_radius, y + box_radius)
            # 选定颜色并转换为 BGR 格式
            color_rgb = colors[i % len(colors)]
            box_color = (int(color_rgb[2] * 255), int(color_rgb[1] * 255), int(color_rgb[0] * 255))  # RGB 转换为 BGR
            cv2.rectangle(frame, top_left, bottom_right, box_color, 2)

    # 写入输出视频
    out.write(frame)
    frame_index += 1

# 释放资源
cap.release()
out.release()
