last_line = [1]
line_len = 1
for i in range(input("How many lines you want!?: ")):
    if i == 0:
        print last_line
        continue
    line_len += 1
    new_line = []
    for a in range(line_len):
        if a == 0 or a == line_len - 1:
            new_line.append(1)
            continue
        new_line.append(last_line[a] + last_line[a-1])
    print new_line
    last_line = new_line
