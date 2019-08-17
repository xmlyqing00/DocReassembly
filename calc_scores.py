import os
import numpy as np
import matplotlib.pyplot as plt
import json

folder_name = 'data/scores/'
scores_list = os.listdir(folder_name)
scores_list.sort(key = lambda x: (len(x), x))
nums = [20, 30, 40, 60]
method_names = ['DNN', 'GA', 'P-Greedy', 'C-Greedy', 'W-Greedy', 'W-GCOM']
# method_colors = ['teal', 'olive', 'aqua', 'orchid', 'orange']
method_colors = ['lightsalmon', 'pink', 'lightblue', 'lightgreen', 'dodgerblue', 'slateblue']
score_files = ['doc0', 'doc3', 'doc7', 'doc10', 'doc12', 'doc13', 'doc14', 'doc15', 'doc16', 'doc17', 'doc18', 'doc20', 'doc21', 'doc23', 'doc24']
order = [1, 0, 2, 3, 4, 5]

test_cnt = 0
test_n = len(nums)
method_n = len(method_names)
regular_method_n = 4
data = []

for i in range(test_n):
    method_data = []
    for j in range(method_n):
        method_data.append([])
    data.append(method_data)

######################
CNN_result_file = open(os.path.join(folder_name, 'CNN_result.json'), 'r')
CNN_result = json.load(CNN_result_file)
CNN_result_file.close()

dnn_time = [0,0,0,0]
for name, result in CNN_result.items():
    
    print(name)
    arr = name.split('/')[2].split('_')
    puzzle_name, stripes_n = arr[0], int(arr[1])

    print(result['solution'])
    if puzzle_name == 'doc60' or stripes_n == 36 or puzzle_name == 'real1':
        continue

    puzzle_idx = score_files.index(puzzle_name)
    num_idx = nums.index(stripes_n)

    dnn_time[num_idx] += float(result['time'])

    gt_file = open('data/stripes/' + puzzle_name + '_' + str(stripes_n) + '/order.txt')
    gt_order = []
    for i in range(stripes_n):
        stripe_idx = int(gt_file.readline())
        gt_order.append(stripe_idx)
    gt_file.close()

    correct_cnt = 0
    for i in range(1, len(result['solution'])):

        for j in range(1, len(gt_order)):
            if gt_order[j] != result['solution'][i]:
                continue
            if gt_order[j - 1] == result['solution'][i - 1]:
                correct_cnt += 1
            break

    acc = float(correct_cnt) / (stripes_n - 1)
    print('Acc:', acc)

    data[num_idx][0].append(acc)

for i in range(test_n):
    dnn_time[i] /= len(scores_list)

print('dnn time', dnn_time)

for i in range(nums):
    print('median', i, np.median(data[i][0]))

#######################
our_data = None
for file_name in scores_list:
    if file_name[:-4] not in score_files:
        continue

    print(folder_name + file_name)
    test_cnt += 1

    score_file = open(folder_name + file_name, 'r')
    for i in range(1, regular_method_n):
        scores = score_file.readline().split()
        if (len(scores) == 0):
            continue
            
        for j in range(test_n):
            data[j][i].append(float(scores[j]))

    scores = score_file.readline().split()
    next_scores = score_file.readline().split()
    scores = scores + next_scores
    our_score = []
    for j in range(test_n):
        data[j][regular_method_n].append(float(scores[j*2]))
        data[j][regular_method_n+1].append(float(scores[j*2+1]))
        our_score.append(float(scores[j*2+1]))
        
    if our_data is None:
        our_data = np.array(our_score)
    else:
        our_data = np.vstack([our_data, np.array(our_score)])

    score_file.close()

# print(data)
# print(our_data)

for j in range(test_n):
    x = np.array(our_data[:,j]).astype(np.float)
    print(x.min(), x.max(), x.mean(), x.std(), np.median(x))
    # print(np.min(x), np.max(x))

label_name = []
for i in range(method_n):
    label_name.append(method_names[order[i]])

for i in range(test_n):
    new_data_item = []
    for j in range(method_n):
        new_data_item.append(data[i][order[j]])
    data[i] = new_data_item

for i in range(test_n):

    plt.figure(figsize=(9,6))
    bplot = plt.boxplot(data[i], patch_artist=True, labels=label_name)

    plt.xlabel('Algorithms')
    plt.ylabel('Accuracy')
    plt.yticks(np.arange(0, 1.1, 0.2))

    for patch, color in zip(bplot['boxes'], method_colors):
        patch.set_facecolor(color)

plt.show()
