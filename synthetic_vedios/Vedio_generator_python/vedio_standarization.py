import cv2

# 读取视频文件
input_path = "C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_dynamic_complex_bg\\car_attack.mp4"
output_path = "C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_dynamic_complex_bg\\standared_car_attack.avi"
gray_output_path = "C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_dynamic_complex_bg\\gray_standared_car_attack.avi"

# 打开视频文件
cap = cv2.VideoCapture(input_path)

# 获取视频的帧率
fps = cap.get(cv2.CAP_PROP_FPS)

# 设置输出视频的参数
fourcc = cv2.VideoWriter_fourcc(*'XVID')
out = cv2.VideoWriter(output_path, fourcc, 30.0, (320, 240))
gray_out = cv2.VideoWriter(gray_output_path, fourcc, 30.0, (320, 240), isColor=False)

# 获取视频的总帧数
total_frames = int(cap.get(cv2.CAP_PROP_FRAME_COUNT))
print("Raw Video Total Frame : ", total_frames)

# 定义截取的帧范围，包含整个视频所有帧
FRAME_BEGIN = 0  # 起始帧
FRAME_END = 81   # total_frames - 1  # 结束帧

current_frame = 0

# 设置对比度增强参数
alpha = 1  # 对比度增益（1.0表示不变，1.5增加对比度）
beta = 0  # 不改变亮度

while cap.isOpened():
    ret, frame = cap.read()
    if not ret:
        break

    # 只处理指定范围内的帧
    if FRAME_BEGIN <= current_frame <= FRAME_END:
        # 调整帧的大小
        frame_resized = cv2.resize(frame, (320, 240))
        # 增强对比度，不改变亮度
        frame_contrast = cv2.convertScaleAbs(frame_resized, alpha=alpha, beta=beta)
        # 写入对比度增强的帧到彩色输出视频
        out.write(frame_contrast)

        # 转换为灰度图并写入到灰度输出视频
        gray_frame = cv2.cvtColor(frame_contrast, cv2.COLOR_BGR2GRAY)
        gray_out.write(gray_frame)

    current_frame += 1

# 释放资源
cap.release()
out.release()
gray_out.release()
cv2.destroyAllWindows()
