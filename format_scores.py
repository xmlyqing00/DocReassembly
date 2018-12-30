import os
import numpy as np

folder_name = 'data/scores_ori/'
dst_folder_name = 'data/scores/'
scores_list = os.listdir(folder_name)
nums_len = 6
method_n = 4
data = np.zeros([4, 6])

for file_name in scores_list:

    print(folder_name + file_name)

    score_file = open(folder_name + file_name, 'r')
    for i in range(int(method_n / 2)):
        x = score_file.readline()
        scores = x.split()
        if (len(scores) == 0):
            continue
        
        for j in range(nums_len):
            data[i * 2][j] = float(scores[j * 2])
            data[i * 2 + 1][j] = float(scores[j * 2  + 1])

    score_file.close()

    score_file = open(dst_folder_name + file_name, 'w')
    for i in range(method_n):
        for j in range(nums_len):
            if j == 2 or j == 3 or j == 5:
                continue
            score_file.write(str(data[i][j]) + ' ')
        score_file.write('\n')

    score_file.close()


    print(data)
