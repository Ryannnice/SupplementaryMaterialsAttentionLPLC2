# 文件路径和名称
#define VEDIO_PATH "C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_dynamic_complex_bg\\standared_UAV_outdoor.avi"
#define OUTPUT_VEDIO_NAME ".\\Output_Vedio\\standared_UAV_outdoor.avi"

#define OUTPUT_FILE_NAME_SingleAF ".\\Output_Txt\\4.txt"
#define OUTPUT_FILE_NAME_Existed ".\\Output_Txt\\standared_UAV_outdoor_Existed.txt"
#define OUTPUT_FILE_NAME_Existing ".\\Output_Txt\\standared_UAV_outdoor_Existing.txt"
#define OUTPUT_FILE_NAME_Fatal ".\\Output_Txt\\standared_UAV_outdoor_Fatal.txt"
import numpy as np
import matplotlib.pyplot as plt
import cv2
from matplotlib import gridspec

# 视频路径
video_path = r'C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\Output_Vedio\\standared_bear_attack.avi'

Total_Frame = 100

# 指定要提取的帧（0到120）
#frame_numbers = list(range(0, 120, 14))
frame_numbers = list(range(0, Total_Frame, 18))

frames = []

cap = cv2.VideoCapture(video_path)
for num in frame_numbers:
    cap.set(cv2.CAP_PROP_POS_FRAMES, num)
    ret, frame = cap.read()
    if ret:
        frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)  # 转换为 RGB 格式
        frames.append(frame)
cap.release()

# 拼接所有提取的帧为一个长条图片
concatenated_image = np.concatenate(frames, axis=1)

# 文件路径和名称
file_name = 'standared_bear_attack_Fatal.txt'
file_path = r'C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\Output_Txt\\' + file_name


# 读取文件
with open(file_path, 'r') as file:
    lines = file.readlines()

# 创建图形，设置整体大小
fig = plt.figure(figsize=(8, 7))
gs = gridspec.GridSpec(2, 1, height_ratios=[0.5, 1])  # 设置两行的比例

# 添加拼接的帧作为第一个子图
ax0 = fig.add_subplot(gs[0])
ax0.imshow(concatenated_image)
ax0.axis('off')  # 隐藏坐标轴

# 在图像下方添加 Frame 信息
for i, num in enumerate(frame_numbers[:9]):  # 只为前八幅图像添加标注
    ax0.text(
        (i + 0.5) * concatenated_image.shape[1] // 6,  # 计算每个子图的中心位置
        concatenated_image.shape[0] + 10 ,  # 将 y 坐标设置为图像下方，15为垂直偏移值，可根据需要调整
        f'Frame {num}',  # 标注帧数
        color='black', 
        fontsize=12, 
        va='top',  # 设置文字的垂直对齐为顶部
        ha='center'  # 设置文字的水平对齐为居中
    )

# 添加第二个子图
ax1 = fig.add_subplot(gs[1])

# 寻找全局最大值：
y_max = 0
for idx, line in enumerate(lines):
    data = list(map(float, line.split()))
    y_values = data[5:]
    y = np.max(y_values)
    if y > y_max:
        y_max = y


# 遍历每一行数据，绘制折线
for idx, line in enumerate(lines):
    data = list(map(float, line.split()))
    y_annotation = int(data[1])  # Ordinate
    x_annotation = int(data[2])  # Abscissa
    data[3] = int(data[3])  # Ordinate
    data[4] = int(data[4])  # Abscissa
    y_values = data[5:]  
    #y_values_normalized = y_values / np.max(y_values)  # 归一化_way_1
    y_values_normalized = y_values / y_max  # 归一化_way_2
    x_values = np.arange(Total_Frame)  # abscissa range
    
    # 绘制折线
    ax1.plot(x_values, y_values_normalized, label=f'AF {idx + 1}:({x_annotation},{240-y_annotation}), {data[3]}~{data[4]} frames', linewidth=1)
    
# 设置图形标题和轴标签
#ax1.set_title('mLPLC2 Response in Fatal Attention Fields',fontsize=17)
#ax1.set_xlabel('Time in Frames',fontsize=18)
#ax1.set_ylabel('Normalized Membrane Potential',fontsize=17)

# 添加图例，放在左上角
ax1.legend(title="Center and Duration of AFs", loc='upper left',fontsize=15)

# 设置整体布局，紧贴子图
plt.tight_layout(h_pad=0)

# 保存图形
save_path = r'C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\Output_Plot\\' + 'bear_attack.png'
plt.savefig(save_path, dpi=300, bbox_inches='tight')

# 显示图形
plt.show()
