import os
import cv2
import numpy as np

def mark_bg(img):
    
    color_thres = 200
    block_h = 20
    shift_x = 0
    block_cnt = 0
    out_width = 0

    height, width, channels = img.shape

    for y in range(block_h, height - block_h, block_h):

        x = 0
        while x < width:
            c = img[y][x]
            if c[0] > color_thres and c[1] > color_thres and c[2] > color_thres:
                break
            x += 1
        
        if y == block_h:
            shift_x = x
        else:
            shift_x = int(round(0.8 * shift_x + 0.2 * x))

        for y_ in range(y - block_h, y):
            

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
    