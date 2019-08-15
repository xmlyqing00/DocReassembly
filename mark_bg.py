import os
import cv2
import numpy as np

def mark_bg(img):
    
    color_thres = 200
    block_h = 20
    left_x = 0
    block_cnt = 0
    out_width = 0

    height, width, channels = img.shape

    mask = np.ones(img.shape[:2]) * 255

    for y in range(block_h, height - block_h, block_h):

        x = 0
        while x < width:
            c = img[y][x]
            if c[0] > color_thres and c[1] > color_thres and c[2] > color_thres:
                break
            x += 1
        
        if y == block_h:
            left_x = x
        else:
            left_x = int(round(0.8 * left_x + 0.2 * x))

        x = width - 1
        while x >= 0:
            c = img[y][x]
            if c[0] > color_thres and c[1] > color_thres and c[2] > color_thres:
                break
            x -= 1
        
        if y == block_h:
            right_x = x
        else:
            right_x = int(round(0.8 * right_x + 0.2 * x))
        
        for y_ in range(y - block_h, y):
            for x_ in range(0, left_x):
                mask[y_][x_] = 0
            for x_ in range(right_x + 1, width):
                mask[y_][x_] = 0
    
    return mask

if __name__ == '__main__':

    dataset_folder = 'data/stripes/real1_27'
    stripe_list = os.listdir(dataset_folder)
    stripe_list.sort(key = lambda x: (len(x), x))

    print(stripe_list)

    for file_name in stripe_list:
        if file_name[-4:] != '.png':
            continue
        
        print('Processing', file_name)
        stripe_path = os.path.join(dataset_folder, file_name)
        img = cv2.imread(stripe_path)
        mark_bg(img)

        # cv2.imshow('dd', img)
        # cv2.waitKey()



    # x = np.load('/Ship01/Downloads/D00201.npy')
    # print(x.shape)
    