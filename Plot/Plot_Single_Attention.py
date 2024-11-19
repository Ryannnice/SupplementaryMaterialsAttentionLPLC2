import numpy as np
import matplotlib.pyplot as plt

file_name = 'outdoor_2_black_4_period_attention.txt'
file_path = r'C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\Output_Txt\\' + file_name 

data = np.loadtxt(file_path, skiprows=4, encoding='gbk')

x_values = np.arange(len(data))

plt.figure(figsize=(8, 2))
plt.plot(x_values, data, color='black')

plt.title(r'Expanding and Receding in 4 Periods', loc='center')
plt.xlabel('Frames')
plt.ylabel('Membrane potential')
plt.ylim([-max(data)*0.1, max(data)*1.1])

plt.tight_layout()

save_path = r'C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\Output_Plot\\' + 'outdoor_2_black_4_period_attention.png'
plt.savefig(save_path, dpi=1000)

plt.show()