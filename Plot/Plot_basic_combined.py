import numpy as np
import matplotlib.pyplot as plt
import os
import cv2

# 文件夹路径
folder_path = r'C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\Output_Txt'
output_folder_path = r'C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\Output_Plot'
video_folder_path = r'C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_basic_test'

# 所有测试类型
test_types = [
    "test_black_approach", "test_black_recede", 
    "test_white_approach", "test_white_recede", 
    "test_translating", "test_grating"
]

# 实验类型
experiment_types = ["LPLC2", "aLPLC2", "mLPLC2"]

# 定义颜色列表
colors = ['#4b4453', '#845ec2', '#ff8066']  # 分别对应 LPLC2, aLPLC2, mLPLC2

# 测试名称列表
test_names = [
    "Dark Object Outward", "Dark Object Inward", 
    "White Object Outward", "White Object Inward", 
    "Translating Motion", "Grating Motion"
]

# 获取所有文件的最大值用于归一化
global_max = 0
mLPLC2_max = 0  # 用于存储 mLPLC2 的最大值

# 首先计算 LPLC2 和 aLPLC2 的最大值
for experiment_type in ["LPLC2", "aLPLC2"]:
    for test_type in test_types:
        file_name = f"{test_type}_{experiment_type}.txt"
        file_path = os.path.join(folder_path, file_name)
        try:
            data = np.loadtxt(file_path, skiprows=4, encoding='gbk')
            if data.size == 0:
                continue
            max_value = np.max(data) if data.size > 0 else 0
            if max_value > global_max:
                global_max = max_value
        except Exception as e:
            print(f"Error reading {file_name}: {e}")

# 然后计算 mLPLC2 的最大值
for test_type in test_types:
    file_name = f"{test_type}_mLPLC2.txt"
    file_path = os.path.join(folder_path, file_name)
    try:
        with open(file_path, 'r') as file:
            lines = file.readlines()
        if len(lines) == 0:
            continue
        values = list(map(float, lines[0].split()[5:]))  # 读取第一行
        max_value = np.max(values) if values else 0
        if max_value > mLPLC2_max:
            mLPLC2_max = max_value
    except Exception as e:
        print(f"Error reading {file_name}: {e}")

# 创建子图，调整子图顺序
fig, axs = plt.subplots(len(experiment_types) + 1, len(test_types), figsize=(9, 6))
fig.subplots_adjust(hspace=0, wspace=0)

# 提取每个视频的第15帧并分别显示
frame_numbers = [15]  # 我们只提取第15帧
video_names = [
    "test_black_approach.avi", "test_black_recede.avi", 
    "test_white_approach.avi", "test_white_recede.avi", 
    "test_translating.avi", "test_grating.avi"
]

for col_idx, video_name in enumerate(video_names):
    video_path = os.path.join(video_folder_path, video_name)
    cap = cv2.VideoCapture(video_path)
    
    for num in frame_numbers:
        cap.set(cv2.CAP_PROP_POS_FRAMES, num - 1)
        ret, frame = cap.read()
        if ret:
            frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)  # 转换为 RGB 格式
            axs[0, col_idx].imshow(frame)  # 在第一行的每个子图中显示对应的帧
            axs[0, col_idx].axis('off')  # 关闭坐标轴
            # axs[0, col_idx].set_title(f"Frame {num} from {video_name}", fontsize=10)  # 添加标题
    
    cap.release()

# 绘制每个子图
for col_idx, test_type in enumerate(test_types):
    for row_idx, experiment_type in enumerate(experiment_types):
        file_name = f"{test_type}_{experiment_type}.txt"
        file_path = os.path.join(folder_path, file_name)
        
        ax = axs[row_idx + 1, col_idx]  # 下面的子图

        try:
            if experiment_type == "mLPLC2":
                with open(file_path, 'r') as file:
                    lines = file.readlines()
                if len(lines) == 0:
                    y_values = np.zeros(50)  # 空文件处理
                else:
                    values = list(map(float, lines[0].split()[5:]))  # 读取第一行
                    y_values = np.array(values) / mLPLC2_max if mLPLC2_max != 0 else np.array(values)
            else:
                data = np.loadtxt(file_path, skiprows=4, encoding='gbk')
                y_values = data / global_max if global_max != 0 else data

            x_values = np.arange(len(y_values))
            ax.plot(x_values, y_values, color=colors[row_idx])  # 使用不同颜色绘图

            # 统一纵坐标范围
            ax.set_ylim([-0.1, 1.1])
        
        except Exception as e:
            print(f"Error reading {file_name}: {e}")
            y_values = np.zeros(50)  # 出错时绘制全为0的图
            x_values = np.arange(len(y_values))
            ax.plot(x_values, y_values, color='black')
        
        # 取消子图坐标轴显示
        ax.set_xticks([])
        ax.set_yticks([])

        # 在每行最右侧的子图添加图例
        if col_idx == len(test_types) - 1:
            labels = ['LPLC2', 'aLPLC2', 'mLPLC2']
            ax.legend(labels[row_idx:row_idx+1], loc='upper right')

# 设置列标题
for col_idx, test_name in enumerate(test_names):
    axs[0, col_idx].set_title(test_name, fontsize=7.5)  # 使用 test_names 设置列标题

# 为最左下角的子图添加坐标轴标签和刻度
ax_bottom_left = axs[-1, 0]  # 最左下角的子图
# 设置横坐标刻度和对应的标签
ax_bottom_left.set_xticks(np.arange(0, 51, 10))  # 设置横坐标刻度
ax_bottom_left.set_xticklabels(np.arange(0, 51, 10), fontsize=6)  # 设置横坐标刻度的字体大小


ax_bottom_left.set_yticks(np.linspace(-0.1, 1.1, 3) )  # 设置纵坐标刻度
ax_bottom_left.set_yticklabels(np.round(np.linspace(-0.1, 1.1, 3), 2), fontsize=6)  # 设置纵坐标刻度的字体大小

ax_bottom_left.set_xlabel("Time in Frames", fontsize=13)  # 横坐标标签，设置字体大小
# 设置纵坐标标签及其位置
ax_bottom_left.set_ylabel("Normalized Membrane Potential", fontsize=13, labelpad=5)  # labelpad调整标签与坐标轴的距离

ax_bottom_left.yaxis.set_label_coords(-0.2, 0.9)  # 例如将标签移动到左侧并居中

# 保存图形
save_path = os.path.join(output_folder_path, 'basic_combined.png')
plt.savefig(save_path, dpi=300, bbox_inches='tight')

# 显示图形
# plt.show()
