import numpy as np
import matplotlib.pyplot as plt

# 文件路径和名称
file_name = 'outdoor_3_black_expand_multi_6_Existing.txt'
#define OUTPUT_FILE_NAME ".\\Output_Txt\\outdoor_3_black_expand_multi_6_Existing.txt"

file_path = r'C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\Output_Txt\\' + file_name
Total_Frame = 120

# 读取文件，每一行作为一条折线，数据以空格隔开
with open(file_path, 'r') as file:
    lines = file.readlines()

plt.figure(figsize=(10, 6))

# 遍历每一行数据，绘制折线
for idx, line in enumerate(lines):
    # 将每行数据分割成数值
    data = list(map(float, line.split()))
    
    # Attention Centroid : 
    y_annotation = int(data[1]) # data[1] : Ordinate
    x_annotation = int(data[2]) # data[2] : Abscissa

    y_values = data[5:]  
    x_values = np.arange(Total_Frame)  # abscissa range
    
    # 绘制折线，并将“Centroid: (x, y)”作为图例名称
    plt.plot(x_values, y_values, label=f'({x_annotation},{y_annotation}),(In:{data[3]},Out:{data[4]})', linewidth=1)

# 设置图形标题和轴标签
plt.title('Current Existing Attention Fields Response')
plt.xlabel('Frames')
plt.ylabel('Membrane Potential')

# 添加图例，图例放置在图的右侧并设置图例标题为“Centroid Coordinate”
plt.legend(title="Centroid Coordinate", loc='center left', bbox_to_anchor=(1, 0.5))

# 设置布局以防止图形元素重叠
plt.tight_layout()

# 保存图形
save_path = r'C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\Output_Plot\\' + 'outdoor_3_black_expand_multi_6_Existing.png'
plt.savefig(save_path, dpi=300, bbox_inches='tight')

# 显示图形
plt.show()
