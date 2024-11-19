import numpy as np
import matplotlib.pyplot as plt
import cv2
from matplotlib import gridspec

#define VEDIO_PATH ".\\synthetic_vedios\\Vedio_dynamic_complex_bg\\outdoor_2_black_4_period.avi"
#define OUTPUT_VEDIO_NAME ".\\Output_Vedio\\outdoor_2_black_4_period.avi"

#define OUTPUT_FILE_NAME_SingleAF ".\\Output_Txt\\4.txt"
#define OUTPUT_FILE_NAME_Existed ".\\Output_Txt\\outdoor_2_black_4_period_Existed.txt"
#define OUTPUT_FILE_NAME_Existing ".\\Output_Txt\\outdoor_2_black_4_period_Existing.txt"
#define OUTPUT_FILE_NAME_Fatal ".\\Output_Txt\\outdoor_2_black_4_period_Fatal.txt"
# 文件名和路径
file_name_1 = 'outdoor_2_black_4_period_no_attention.txt'
file_name_2 = 'outdoor_2_black_4_period_attention.txt'
file_name_3 = 'outdoor_2_black_4_period_Fatal.txt'
file_path = r'C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\Output_Txt\\'
video_path = r'C:\A_RESEARCH\Off_Line\C\LPLC2\synthetic_vedios\Vedio_dynamic_complex_bg\outdoor_2_black_4_period.avi'

# 提取视频帧
frame_numbers = [1, 50, 100, 150, 200, 250, 300, 350, 400]
frames = []

cap = cv2.VideoCapture(video_path)
for num in frame_numbers:
    cap.set(cv2.CAP_PROP_POS_FRAMES, num - 1)
    ret, frame = cap.read()
    if ret:
        frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)  # 转换为 RGB 格式
        frames.append(frame)
cap.release()

# 拼接所有提取的帧为一个长条图片
concatenated_image = np.concatenate(frames, axis=1)

# 读取数据
data_1 = np.loadtxt(file_path + file_name_1, skiprows=4, encoding='gbk')
data_2 = np.loadtxt(file_path + file_name_2, skiprows=4, encoding='gbk')

# 读取第三个文件的数据
with open(file_path + file_name_3, 'r') as file:
    lines_3 = file.readlines()

# 获取横坐标
x_values_1 = np.arange(len(data_1))
x_values_2 = np.arange(len(data_2))
total_frames = 400
x_values_3 = np.arange(total_frames)

# 归一化
data_1_normalized = data_1 / np.max(data_1)
data_2_normalized = data_2 / np.max(data_2)

# 计算第三个文件中所有行的最大值，用于归一化
max_val_3 = max([np.max(np.array(list(map(float, line.split()))[5:])) for line in lines_3])

# 创建图形，设置整体大小
fig = plt.figure(figsize=(8, 6))
gs = gridspec.GridSpec(4, 1, height_ratios=[0.7, 0.7, 0.7, 0.7])  # 设置每个子图的高度比例

# 添加拼接的帧作为第一个子图
ax0 = fig.add_subplot(gs[0])
ax0.imshow(concatenated_image)
ax0.axis('off')  # 隐藏坐标轴

# 设置偏移量，用于右移文本
offset = 15  
# 在图像下方添加 Frame 信息
for i, num in enumerate(frame_numbers):
    ax0.text(
        i * concatenated_image.shape[1] // len(frame_numbers) + concatenated_image.shape[1] // (2 * len(frame_numbers)) + offset, 
        concatenated_image.shape[0] + 10,  # 将 y 坐标设置为图像下方
        f'Frame {num}', 
        color='black', 
        fontsize=8, 
        va='top',  # 设置文字的垂直对齐为顶部
        ha='center'  # 设置文字的水平对齐为居中
    )

# 设置共享的y范围
y_min, y_max = -0.1, 1.1

# 添加第二个子图
ax1 = fig.add_subplot(gs[1])
ax1.plot(x_values_1, data_1_normalized, color='#4b4453', linewidth=2, label='LPLC2')
ax1.set_ylim(y_min, y_max)
ax1.legend(loc='upper right', fontsize=10)

# 隐藏第二个子图的横坐标刻度值
ax1.xaxis.set_visible(False)

# 添加第三个子图
ax2 = fig.add_subplot(gs[2])
ax2.plot(x_values_2, data_2_normalized, color='#845ec2', linewidth=2, label='aLPLC2')
ax2.set_ylim(y_min, y_max)
ax2.legend(loc='upper right', fontsize=10)

# 隐藏第三个子图的横坐标刻度值
ax2.xaxis.set_visible(False)

# 添加第四个子图
ax3 = fig.add_subplot(gs[3])
color_list = ['#00c9a7', '#ff8066', '#005b44', '#009efa', '#d53624']  # 可以根据需要扩展颜色列表
num_lines = len(lines_3)

for idx, line in enumerate(lines_3):
    data = list(map(float, line.split()))
    y_values = np.array(data[5:])
    y_values_normalized = y_values / max_val_3
    
    # 选择颜色，循环使用颜色列表
    color = color_list[idx % len(color_list)]
    ax3.plot(x_values_3, y_values_normalized, color=color, linewidth=2, label=f'mLPLC2 AF {idx + 1}')

ax3.set_ylim(y_min, y_max)
ax3.legend(loc='upper right', fontsize=8)

# 设置第四个子图的横坐标刻度值
ax3.set_xticks(x_values_3[::50])  # 设置刻度间隔
ax3.set_xticklabels(x_values_3[::50], fontsize=12 )  # 显示刻度标签并旋转

# 设置横纵坐标标签，放置在子图之外
fig.text(0.5, -0.02, 'Time in Frames', ha='center', fontsize=18)  # 横坐标
fig.text(-0.01, 0.5, 'Normalized Membrane Potential', va='center', rotation='vertical', fontsize=17)  # 纵坐标

# 设置整体布局
plt.tight_layout(rect=[0, 0, 1, 1])

# 保存图形，按原比例
save_path = r'C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\Output_Plot\\' + '4_period_final.png'
plt.savefig(save_path, dpi=300, bbox_inches='tight')  # 使用bbox_inches='tight'

# plt.show()
