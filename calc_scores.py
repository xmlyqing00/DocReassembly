import os
import numpy as np
import matplotlib.pyplot as plt

folder_name = 'data/scores/'
scores_list = os.listdir(folder_name)
scores_list.sort(key = lambda x: (len(x), x))
nums = [20, 30, 40, 60]
method_names = ['P-Greedy', 'C-Greedy', 'W-Greedy', 'W-GCOM', 'P-GA']
method_colors = ['teal', 'olive', 'aqua', 'orchid', 'orange']
score_files = ['doc0', 'doc3', 'doc7', 'doc10', 'doc12', 'doc13', 'doc14', 'doc15', 'doc16', 'doc17', 'doc18', 'doc20', 'doc21', 'doc23', 'doc24']
# order = [4, 0, 1, 2, 3]
order = [0, 1, 2, 3]

adjust_flag = True
test_cnt = 0
test_n = len(nums)
# method_n = len(method_names)
method_n = 4
data = np.zeros(method_n * test_n)

our_data = None
for file_name in scores_list:
    if file_name[:-4] not in score_files:
        continue

    print(folder_name + file_name)
    test_cnt += 1

    score_file = open(folder_name + file_name, 'r')
    for i in range(2):
        scores = score_file.readline().split()
        if (len(scores) == 0):
            continue
            
        for j in range(test_n):
            data[i * test_n + j] += float(scores[j])

    scores = score_file.readline().split()
    next_scores = score_file.readline().split()
    scores = scores + next_scores
    print(scores)
    our_score = []
    for j in range(test_n):
        data[2 * test_n + j] += float(scores[j*2])
        data[3 * test_n + j] += float(scores[j*2+1])
        our_score.append(float(scores[j*2+1]))
    if our_data is None:
        our_data = np.array(our_score)
    else:
        our_data = np.vstack([our_data, np.array(our_score)])

    score_file.close()

data /= test_cnt
print(data)
print(our_data)

for j in range(test_n):
    x = np.array(our_data[:,j]).astype(np.float)
    print(x.min(), x.max(), x.mean(), x.std())
    # print(np.min(x), np.max(x))

x = list(range(len(nums)))
total_width = 0.8
width = total_width / method_n

for i in range(method_n):
    order_idx = order[i]
    if i > 0:
        for j in range(test_n):
            x[j] += width
    plt.bar(x, data[order_idx * test_n: (order_idx+1) * test_n], width=width, label=method_names[order_idx], facecolor=method_colors[order_idx])

ticks = [str(x) for x in nums]
plt.xticks(np.arange(test_n), ticks)
plt.legend(loc='lower left', prop={'size': 8})
plt.xlabel('Stripes Number')
plt.ylabel('Accuracy')
plt.show()
