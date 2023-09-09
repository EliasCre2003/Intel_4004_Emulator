with open("test3.txt", "r") as f:
    lines = f.readlines()
data: list[int] = []

for line in lines:
    line = line[5:]
    i = 0
    while i < len(line):
        if line[i] == "\n" or line[i] == "\t" or line[i] == " ":
            line = line[:i] + line[i + 1:]
        else:
            i += 1
    print(line)
    for i in range(0, len(line), 2):
        n = line[i] + line[i + 1]
        data.append(int(n, 16))
with open("test2.bin", "wb") as f:
    f.write(bytearray(data))