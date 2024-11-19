import cv2
import numpy as np

# 读取视频文件
input_path = "C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_dynamic_complex_bg\\standared_bird_attack_2.avi"
output_path = "C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_dynamic_complex_bg\\standared_bird_attack_2_slow.avi"

# 设置对比度增强参数
'''
alpha = 5  # 对比度增益（1.0表示不变，1.5增加对比度）
beta = -100     # 亮度偏移（0表示不变）
'''
alpha = 1  # 对比度增益（1.0表示不变，1.5增加对比度）
beta = 0     # 亮度偏移（0表示不变）

# 打开视频文件
cap = cv2.VideoCapture(input_path)

# 获取视频的帧率
fps = cap.get(cv2.CAP_PROP_FPS)

# 设置输出视频的参数
fourcc = cv2.VideoWriter_fourcc(*'XVID')
out = cv2.VideoWriter(output_path, fourcc, fps, (int(cap.get(cv2.CAP_PROP_FRAME_WIDTH)), int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT))))

frames = []

# 读取所有帧
while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break
    frames.append(frame)

cap.release()

# 插入平均帧
new_frames = []
for i in range(len(frames) - 1):
    new_frames.append(frames[i])
    avg_frame = cv2.addWeighted(frames[i], 0.5, frames[i + 1], 0.5, 0)
    new_frames.append(avg_frame)
new_frames.append(frames[-1])


# 写入新视频
for frame in new_frames:
    # 增强对比度
    frame_contrast = cv2.convertScaleAbs(frame, alpha=alpha, beta=beta)
    out.write(frame_contrast)

out.release()
cv2.destroyAllWindows()
