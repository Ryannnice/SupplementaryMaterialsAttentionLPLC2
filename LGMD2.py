import cv2
import cv2
import numpy as np
import matplotlib.pyplot as plt
import math
import time
from numba import njit, prange

K_values = []
Col_values = []



@njit(parallel=True)
def Spatiotemporal_Convolution(input_pre, input_cur, Kernel, dt, t_delay):
    
    row, col = input_cur.shape
    output = input_cur.copy()  # 深拷贝以保证不改变 input

    for i in prange(1, row-1):
        for j in range(1, col-1):
            Delay = np.zeros((3, 3))
            
            for h in range(-1, 2):
                for w in range(-1, 2):
                    Delay[h + 1, w + 1] = LP_filter(input_pre[i + h, j + w], input_cur[i + h, j + w], dt, t_delay[h + 1, w + 1])
            
            output[i, j] = np.sum(np.multiply(Delay, Kernel))  # Convolution
    
    return output

@njit
def isolated_excitations_filter(input, coefficient, threshold):
    
    row, col = input.shape
    
    for i in range(row):
        for j in range(col):
            if input[i, j]*coefficient < threshold:
                input[i, j] = 0
    
    return input

@njit
def LP_filter(input_pre, input_cur, dt, t_delay):
    a = dt / (dt + t_delay)

    return a * input_cur + (1 - a) * input_pre  # first-order low-pass filtering


class LGMD2_Node:
    def __init__(self, first_frame, dt):
        self.dt = self.t_i = dt            # t_i = dt(每两帧图像之间的时间间隔)，单位：毫秒(ms)

        self.a_i = 0.01 # 1 / (1 + math.exp(1))   # decay coefficient
        self.a1 = 0.1 # coefficient in half-wave rectifying
        
        self.w3 = 0.1  # bias baseline in ON channels : max(w3, PM^(t)/Tpm)
        self.w4 = 0.3  # bias baseline in OFF channels : max(w4, PM^(t)/Tpm)
        self.Theta_1 = 0.5  # Theta_1∈[1 , 2]
        self.Theta_2 = 1.0  # Theta_2∈[0.5 , 1]
        self.Theta_3 = 1.0  # Theta_3∈[0 , 0.6]
        self.T_pm = 2.6    #20.0  # threshold in PM way(运小球现实场景就调小点，曲线更不集成)
        self.t_3 = 90  # 90.0
        self.t_4 = 800.0  # 800.0  #t_4∈[500, 1000]
        self.a5 = 2      #0.3  # LGMD2 Cell(运小球现实场景就调大一点，调脉冲大一点)
        self.a6 = self.t_4 / (self.t_4 + self.dt)   #500  # time constant in SFA
        self.a7 = 1
        self.C_w = 4.0
        self.delta_c = 0.01
        self.C_de = 0.5
        self.T_de = 15
        self.T_sfa = 0.003  #0.003  # small threshold in SFA(运小球现实场景就调小点，曲线摆动幅度越大)
        self.T_spi = 0.75
        self.N_sp = 3.0     # SFA中一个判断阈值，N_sp∈[4 , 8]
        self.N_t = 4        # SFA中累计求和S_spike(t)+S_spike(t-1)+...+S_spike(t-N_t)的个数
        
        self.W_1 = np.array([[0.25, 0.5, 0.25], [0.5, 2, 0.5], [0.25, 0.5, 0.25]])          # the kernel to gain Ion
        self.W_2 = np.array([[0.125, 0.25, 0.125], [0.25, 1, 0.25], [0.125, 0.25, 0.125]])  # the kernel to gain Iff
        self.W_g = np.ones((3, 3), dtype=np.float32) / 9                                    # the kernel to gain coefficient Matrix Ce
        self.t_1 = np.array([[45.0, 30.0, 45.0], [30.0, 15.0, 30.0], [45.0, 30.0, 45.0]])
        self.t_2 = np.array([[180.0, 120.0, 180.0], [120.0, 60.0, 120.0], [180.0, 120.0, 180.0]])
        
        self.L = [None, first_frame.copy()]
        self.L_P = [np.zeros(first_frame.shape) for _ in range(2)]
        self.P_on = [np.zeros(first_frame.shape) for _ in range(2)]
        self.P_off = [np.zeros(first_frame.shape) for _ in range(2)]
        
        self.E_on = np.zeros(first_frame.shape)
        self.E_off = np.zeros(first_frame.shape)
        self.I_on = np.zeros(first_frame.shape)
        self.I_off = np.zeros(first_frame.shape)
        self.S_on = np.zeros(first_frame.shape)
        self.S_off = np.zeros(first_frame.shape)
        self.L_S = np.zeros(first_frame.shape)
        self.L_G = np.zeros(first_frame.shape)
        
        self.S_spike = np.zeros(self.N_t)
        self.PM = [0.0, 0.0]
        self.K = [0.0, 0.0]
        self.K_t = [0.0, 0.0]
        self.row = self.L[1].shape[0]
        self.col = self.L[1].shape[1]
        self.n_cell = self.row * self.col
    
    
    def layer_P(self, gray_image):
        # Photoreceptors
        self.L[0] = self.L[1].copy()
        self.L[1] = gray_image.copy()
        
        self.L_P[0] = self.L_P[1].copy()
        self.L_P[1] = self.L[1] - self.L[0] + self.a_i * self.L_P[0]
    
    def ON_layer_EI(self):
        # half-wave rectifier(ON pathway)
        self.P_on[0] = self.P_on[1].copy()
        self.P_on[1] = (self.L_P[1] + np.abs(self.L_P[1])) / 2.0 + self.a1 * self.P_on[0]
        
        self.E_on = self.P_on[1].copy()

        self.I_on = Spatiotemporal_Convolution(self.P_on[0], self.P_on[1], self.W_1, self.dt, self.t_1)
    
    def OFF_layer_EI(self):
        # half-wave rectifier(OFF pathway)
        self.P_off[0] = self.P_off[1].copy()
        self.P_off[1] = np.abs(self.L_P[1] - np.abs(self.L_P[1])) / 2.0 + self.a1 * self.P_off[0]
        
        self.E_off = self.P_off[1].copy()
        
        self.I_off = Spatiotemporal_Convolution(self.P_off[0], self.P_off[1], self.W_2, self.dt, self.t_2)
    
    def AIM(self):
        # Adaptive Inhibition Mechanism(AIM)
        self.PM[0] = self.PM[1]
        self.PM[1] = np.sum(np.abs(self.L_P[1])) / self.n_cell
        
        self.PM_Delay = LP_filter(self.PM[0], self.PM[1], self.dt, self.t_3)
        
        self.w1 = max(self.w3, self.PM_Delay/self.T_pm)  # Suppress sudden changes in visual scenes
        self.w2 = max(self.w4, self.PM_Delay/self.T_pm)  # Suppress sudden changes in visual scenes
    
    def layer_SG(self):
        # Summation and Grouping
        self.S_on = ((self.E_on - self.w1 * self.I_on) + np.abs(self.E_on - self.w1 * self.I_on)) / 2.0
        self.S_off = ((self.E_off - self.w2 * self.I_off) + np.abs(self.E_off - self.w2 * self.I_off)) / 2.0
        self.L_S = self.Theta_1 * self.S_on + self.Theta_2 * self.S_off + self.Theta_3 * np.multiply(self.S_on, self.S_off)
        
        self.Ce = cv2.filter2D(self.L_S, -1, self.W_g)
        w = np.max(self.Ce) / self.C_w + self.delta_c
        self.L_G = np.multiply(self.L_S, self.Ce) / w

        self.L_G = isolated_excitations_filter(self.L_G, self.C_de, self.T_de)  # smaller isolated (or decayed) excitations are eliminated

    def layer_LGMD_2(self):
        # LGMD2 cell
        self.MP = np.sum(self.L_G)
        
        self.K[0] = self.K[1]
        self.K[1] = 1 / (1 + math.exp(-self.MP / (self.n_cell * self.a5)))
    
    def SFA(self):
        # Spike Frequency Adaptation mechanism(SFA) --> further enhance the visual looming selectivity
        # K^(t) = a6 * (K^(t-1) + K(t) - K(t-1))
        # k represents K(t), k_pre represents K^(t-1), k_cur represents K^(t), k1 represents K(t-1)
        self.K_t[0] = self.K_t[1]
        
        if self.K[1]-self.K[0] <= self.T_sfa:
            self.K_t[1] = self.a6 * (self.K_t[0] + self.K[1] - self.K[0])
        
        else:
            self.K_t[1] = self.a6 * self.K[1]

        # 更新 S_spike 数组
        for i in range(self.N_t-1):
            self.S_spike[i] = self.S_spike[i+1]
        
        self.S_spike[self.N_t-1] = math.exp(self.a7 * (self.K_t[1] - self.T_spi))

        # 检查 S_spike 总和
        if sum(self.S_spike) >= self.N_sp:
            self.Col_t = 1
        else:
            self.Col_t = 0
        
        K_values.append(self.K[1])
        Col_values.append(self.Col_t)

    def NO_SFA(self):
        # Spike Frequency Adaptation mechanism(SFA) --> further enhance the visual looming selectivity
        # K^(t) = a6 * (K^(t-1) + K(t) - K(t-1))
        # k represents K(t), k_pre represents K^(t-1), k_cur represents K^(t), k1 represents K(t-1)
        self.K_t[0] = self.K_t[1]
        
        '''
        if self.K[1]-self.K[0] <= self.T_sfa:
            self.K_t[1] = self.a6 * (self.K_t[0] + self.K[1] - self.K[0])
        
        else:
            self.K_t[1] = self.a6 * self.K[1]
        '''
        
        # 更新 S_spike 数组
        for i in range(self.N_t-1):
            self.S_spike[i] = self.S_spike[i+1]
        
        self.S_spike[self.N_t-1] = math.exp(self.a7 * (self.K_t[1] - self.T_spi))

        # 检查 S_spike 总和
        if sum(self.S_spike) >= self.N_sp:
            self.Col_t = 1
        else:
            self.Col_t = 0
        
        K_values.append(self.K[1])
        Col_values.append(self.Col_t)


def plot_results():
    # Ensure U_values and Col_values are populated
    curve_color = (0/255, 0/255, 234/255)
    bar_color = (10/255, 5/255, 5/255)
    dashed_line_color = (214/255, 226/255, 228/255)
    dashed_color = (164/255, 166/255, 168/255)

    x = list(range(len(K_values)))  # X-axis for K_values

    fig, ax = plt.subplots(figsize=(10, 6))

    ax.axhline(y=0.78, color=dashed_color, linestyle='--', linewidth=2.5)
    ax.axhline(y=1, color=dashed_line_color)

    # Plotting spikes
    for i in range(len(Col_values)):
        ax.plot([i, i], [0, Col_values[i]], color='r', linestyle='--', linewidth=2.3, alpha=0.6)
        ax.plot(i, Col_values[i], marker='o', markersize=6, color='r')  # Adding points

    # Plotting the U[2] curve
    ax.plot(x, K_values, color=curve_color, linewidth=2.7, label='LGMD2')
    ax.set_ylim([0.45, 1.1])
    plt.yticks([0.5, 0.8, 1.0], fontsize=30)
    plt.xticks(fontsize=30) 
    ax.set_ylabel('Membrane Potential', fontsize=32)

    ax.legend(loc='upper right', fontsize=32)

    plt.tight_layout()
    plt.show()


    # Writing K_values to the file with two decimal places
    output_path = r'C:\A_RESEARCH\Off_Line\C\LPLC2\Output_Txt\test_grating_LGMD2.txt'

    with open(output_path, 'w') as f:
        for value in K_values:
            f.write(f"{(value-0.5):.2f}\n")




def main():

    video = cv2.VideoCapture(r"C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_basic_test\\test_black_approach.avi")
    video = cv2.VideoCapture(r"C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_basic_test\\test_black_recede.avi")
    video = cv2.VideoCapture(r"C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_basic_test\\test_white_approach.avi")
    video = cv2.VideoCapture(r"C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_basic_test\\test_white_recede.avi")
    video = cv2.VideoCapture(r"C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_basic_test\\test_translating.avi")
    video = cv2.VideoCapture(r"C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_basic_test\\test_grating.avi")
    video = cv2.VideoCapture(r"C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_dynamic_complex_bg\\outdoor_2_black_4_period.avi")
    video = cv2.VideoCapture(r"C:\\A_RESEARCH\\Off_Line\\C\\LPLC2\\synthetic_vedios\\Vedio_basic_test\\test_grating.avi")


    if not video.isOpened():
        print("Failed to open video.")
        return -1

    ret, first_frame = video.read()
    if not ret:
        print("Failed to read first frame")
        return -1

    first_frame = cv2.resize(first_frame, (320, 320), interpolation=cv2.INTER_NEAREST)
    first_frame = first_frame.astype(np.float32)  # Convert to float32
    first_frame = cv2.cvtColor(first_frame, cv2.COLOR_RGB2GRAY)
    
    #lgmd_2 = LGMD2_Node(first_frame, 1000 / video.get(cv2.CAP_PROP_FPS))
    lgmd_2 = LGMD2_Node(first_frame, 10 )

    start_time = time.time()  # 记录开始时间
    
    framecount = 0
    while True:
        ret, image = video.read()
        if not ret:
            end_time = time.time()  # 记录结束时间
            elapsed_time = end_time - start_time
            print(f"\nTotal runtime: {elapsed_time:f} seconds")
            
            plot_results()
            print("Image is empty")
            break
        
        framecount += 1
        image = cv2.resize(image, first_frame.shape, interpolation=cv2.INTER_NEAREST)
        cv2.imshow("image", image)
        image = image.astype(np.float32)  # Convert to float32
        grayimage = cv2.cvtColor(image, cv2.COLOR_RGB2GRAY)
        
        # Process the image through the LGMD2
        lgmd_2.layer_P(grayimage)
        lgmd_2.ON_layer_EI()
        lgmd_2.OFF_layer_EI()
        lgmd_2.AIM()
        lgmd_2.layer_SG()
        lgmd_2.layer_LGMD_2()
        #lgmd_2.SFA()
        lgmd_2.SFA()
        
        
        # print(lgmd_2.S_spike)
        print(f"Frame: {framecount} MP: {lgmd_2.K[1]:<8} Collision: {lgmd_2.Col_t:<6}")  # Print membrane potential and collision judgment
        
        # print(lgmd_2.MP)
        # print(lgmd_2.w1)
        # print(np.sum(lgmd_2.L[1]))
        # print(np.sum(lgmd_2.L_P[1]))
        # print(np.sum(lgmd_2.I_off))
        
        # cv2.waitKey(0)
        if cv2.waitKey(20) == 27:  # Press Esc to exit
            # elapsed_time = end_time - start_time
            # print(f"Total runtime: {elapsed_time:f} seconds")
            break

    video.release()


if __name__ == "__main__":
    main()

