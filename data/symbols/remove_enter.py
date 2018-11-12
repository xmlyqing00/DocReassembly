in_file = open('symbols_vert.txt', 'r')
data = in_file.read()
in_file.close()

arr = data.split('\n')
print(arr)

out_file = open('symbols.txt', 'w')
for i in range(len(arr)):
    out_file.write(arr[i])
    out_file.write('   ')
    if (i+1) % 16 == 0:
        out_file.write('\n\n')
out_file.close()

