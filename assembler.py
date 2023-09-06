instructions = [
    "NOP",
    "JCN",
    "FIM",
    "SRC",
    "FIN",
    "JIN",
    "JUN",
    "JMS",
    "INC",
    "ISZ",
    "ADD",
    "SUB",
    "LD",
    "XCH",
    "BBL",
    "LDM",
    "WRM",
    "WMP",
    "WRR",
    "WR0",
    "WR1",
    "WR2",
    "WR3",
    "SBM",
    "RDM",
    "RDR",
    "ADM",
    "RD0",
    "RD1",
    "RD2",
    "RD3",
    "CLB",
    "CLC",
    "IAC",
    "CMC",
    "CMA",
    "RAL",
    "RAR",
    "TCC",
    "DAC",
    "TCS",
    "STC",
    "DAA",
    "KBP",
    "DCL"
]

two_word_instructions = [
    "JCN",
    "FIM",
    "JUN",
    "JMS",
    "ISZ",
]

data: list[int] = []
labels: dict[str, int] = {}
label_address_counter = 0


def handle_lines(line: str) -> bool:
    line.strip()
    tokens = line.split(';')[0].replace("\t", " ")
    if len(sp := tokens.split(",")) != 1:
        tokens = sp[1].split(" ")
    else:
        tokens = tokens.strip().split(" ")
    tokens = trim(tokens)
    if len(tokens) == 0:
        return True
    instruction = tokens[0].strip()
    tokens = list(map(str.strip, tokens[1:]))
    match instruction:
        case "NOP":
            data.append(0x00)
        case "JCN":
            data.append(0x10 + conv_int(tokens[0], 0xF))
            data.append(labels[tokens[1]])
        case "FIM":
            data.append(0x20 + (conv_int(tokens[0], 0x8) << 1))
            data.append(conv_int(tokens[1]))
        case "SRC":
            data.append(0x21 + (conv_int(tokens[0], 0x8) << 1))
        case "FIN":
            data.append(0x30 + (conv_int(tokens[0], 0x8) << 1))
        case "JIN":
            data.append(0x31 + (conv_int(tokens[0], 0x8) << 1))
        case "JUN":
            address = labels[tokens[0]]
            data.append(0x40 + (address >> 8))
            data.append(address & 0xFF)
        case "JMS":
            address = labels[tokens[0]]
            data.append(0x50 + (address >> 8))
            data.append(address & 0xFF)
        case "INC":
            data.append(0x60 + conv_int(tokens[0], 0xF))
        case "ISZ":
            data.append(0x70 + conv_int(tokens[0], 0xF))
            data.append(labels[tokens[1]] & 0xFF)
        case "ADD":
            data.append(0x80 + conv_int(tokens[0], 0xF))
        case "SUB":
            data.append(0x90 + conv_int(tokens[0], 0xF))
        case "LD":
            data.append(0xA0 + conv_int(tokens[0], 0xF))
        case "XCH":
            data.append(0xB0 + conv_int(tokens[0], 0xF))
        case "BBL":
            data.append(0xC0 + conv_int(tokens[0], 0xF))
        case "LDM":
            data.append(0xD0 + conv_int(tokens[0], 0xF))
        case "WRM":
            data.append(0xE0)
        case "WMP":
            data.append(0xE1)
        case "WRR":
            data.append(0xE2)
        case "WPM":
            data.append(0xE3)
        case "WR0":
            data.append(0xE4)
        case "WR1":
            data.append(0xE5)
        case "WR2":
            data.append(0xE6)
        case "WR3":
            data.append(0xE7)
        case "SBM":
            data.append(0xE8)
        case "RDM":
            data.append(0xE9)
        case "RDR":
            data.append(0xEA)
        case "ADM":
            data.append(0xEB)
        case "RD0":
            data.append(0xEC)
        case "RD1":
            data.append(0xED)
        case "RD2":
            data.append(0xEE)
        case "RD3":
            data.append(0xEF)
        case "CLB":
            data.append(0xF0)
        case "CLC":
            data.append(0xF1)
        case "IAC":
            data.append(0xF2)
        case "CMC":
            data.append(0xF3)
        case "CMA":
            data.append(0xF4)
        case "RAL":
            data.append(0xF5)
        case "RAR":
            data.append(0xF6)
        case "TCC":
            data.append(0xF7)
        case "DAC":
            data.append(0xF8)
        case "TCS":
            data.append(0xF9)
        case "STC":
            data.append(0xFA)
        case "DAA":
            data.append(0xFB)
        case "KBP":
            data.append(0xFC)
        case "DCL":
            data.append(0xFD)
        case _:
            print(f"Error: {instruction} is not a valid instruction")
            return False
    return True


def add_label(line: str) -> bool:
    global label_address_counter
    line = line.split(';')[0].replace("\t", " ")
    label_address_counter += 1
    line = line.strip().split(",")
    if len(line) < 2:
        sp = line[0].split(" ")
        sp = trim(sp)
        if len(sp) == 0:
            label_address_counter -= 1
        elif sp[0] in two_word_instructions:
            label_address_counter += 1
        return True
    elif len(line) > 2:
        print("Error: Invalid line")
        return False
    if (line[0] in instructions) or (line[0] in labels):
        return False
    if line[0].isnumeric():
        return False
    labels[line[0]] = label_address_counter-1
    rest = line[1].split(" ")
    rest = trim(rest)
    if rest[0] in two_word_instructions:
        label_address_counter += 1
    return True


def conv_int(num: str, size_limit: int = 0xFF) -> int:
    len_num = len(num)
    if len_num > 2 and num[:2] == "0x":
        dat = int(num, 16)
    elif len_num > 2 and num[:2] == "0b":
        dat = int(num, 2)
    else:
        dat = int(num, 10)
    if 0 <= dat <= size_limit:
        return dat
    else:
        print(f"Error: {num} is out of range")
        raise ValueError


def trim(tokens: list[str]) -> list[str]:
    i = 0
    while i < len(tokens):
        if tokens[i] == "":
            tokens.pop(i)
        else:
            i += 1
    return tokens


def main():
    with open("test2.txt", "r") as f:
        lines = f.readlines()
    for line in lines:
        if not add_label(line):
            print("Error: Invalid line")
            break
    for line in lines:
        if not handle_lines(line):
            print("Error: Invalid line")
            break
    print(data)
    with open("test.bin", "wb") as f:
        f.write(bytearray(data))


if __name__ == "__main__":
    print()
    main()
