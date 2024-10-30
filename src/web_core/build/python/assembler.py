one_word_instructions: set[str] = {
    "NOP",
    "SRC",
    "FIN",
    "JIN",
    "INC",
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
}

two_word_instructions: set[str] = {
    "JCN",
    "FIM",
    "JUN",
    "JMS",
    "ISZ",
}

instructions: set[str] = one_word_instructions.union(two_word_instructions)

data: list[int] = []
address_labels: dict[str, int] = {}
label_address_counter = 0

def handle_lines(line: str) -> bool:
    line.strip()
    tokens = line.split(';')[0].replace("\t", " ")
    if len(sp := tokens.split(",")) != 1:
        tokens = sp[-1].split(" ")
    else:
        tokens = tokens.strip().split(" ")
    tokens = remove_empty_str(tokens)
    if len(tokens) == 0 or tokens[0] == "\n":
        return True
    instruction = tokens[0].strip()
    tokens = [token.strip() for token in tokens[1:]]
    if len(instruction) == 0:
        return True

    match instruction:
        case "NOP":
            data.append(0x00)
        case "JCN":
            data.append(0x10 + conv_int(tokens[0], 0xF))
            data.append(address_labels[tokens[1]] & 0xFF)
        case "FIM":
            pair = converte_pair(tokens[0])
            data.append(0x20 + (conv_int(pair, 0x8) << 1))
            data.append(conv_int(tokens[1]))
        case "SRC":
            pair = converte_pair(tokens[0])
            data.append(0x21 + (conv_int(pair, 0x8) << 1))
        case "FIN":
            pair = converte_pair(tokens[0])
            data.append(0x30 + (conv_int(pair, 0x8) << 1))
        case "JIN":
            pair = converte_pair(tokens[0])
            data.append(0x31 + (conv_int(pair, 0x8) << 1))
        case "JUN":
            address = address_labels[tokens[0]]
            data.append(0x40 + (address >> 8))
            data.append(address & 0xFF)
        case "JMS":
            address = address_labels[tokens[0]]
            data.append(0x50 + (address >> 8))
            data.append(address & 0xFF)
        case "INC":
            reg = converte_register(tokens[0])
            data.append(0x60 + conv_int(reg, 0xF))
        case "ISZ":
            reg = converte_register(tokens[0])
            data.append(0x70 + conv_int(reg, 0xF))
            data.append(address_labels[tokens[1]] & 0xFF)
        case "ADD":
            reg = converte_register(tokens[0])
            data.append(0x80 + conv_int(reg, 0xF))
        case "SUB":
            reg = converte_register(tokens[0])
            data.append(0x90 + conv_int(reg, 0xF))
        case "LD" :
            reg = converte_register(tokens[0])
            data.append(0xA0 + conv_int(reg, 0xF))
        case "XCH":
            reg = converte_register(tokens[0])
            data.append(0xB0 + conv_int(reg, 0xF))
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
        sp = remove_empty_str(sp)
        if len(sp) == 0:
            label_address_counter -= 1
        elif sp[0] in two_word_instructions:
            label_address_counter += 1
        return True
    if (line[0] in instructions) or (line[0] in address_labels):
        return False
    if line[0].isnumeric():
        return False
    for i in range(len(line)-1):
        address_labels[line[i]] = label_address_counter - 1
    rest = line[1].split(" ")
    rest = remove_empty_str(rest)
    if len(rest) == 0:
        label_address_counter -= 1
        return True
    if rest[0] in two_word_instructions:
        label_address_counter += 1
    return True


def conv_int(number: str | int, size_limit: int = 0xFF) -> int:
    if type(number) is str:
        len_num = len(number)
        if len_num > 2 and number[:2] == "0x":
            data = int(number, 16)
        elif len_num > 2 and number[:2] == "0b":
            data = int(number, 2)
        else:
            data = int(number, 10)
    elif type(number) is int:
        data = number
    else:
        raise TypeError
    if 0 <= data <= size_limit:
        return data
    else:
        print(f"Error: {number} is out of range")
        raise ValueError


def converte_register(reg: str) -> int | str:
    if len(reg) == 2 and reg[0] == "R" and 0 <= (reg_index := int(reg[1])) <= 9:
        return reg_index
    elif len(reg) == 3 and reg[0] == "R" and reg[1] == "1" and 0 <= (reg_index := int(reg[2])) <= 5:
        return reg_index + 10
    else:
        return reg


def converte_pair(p: str) -> int | str:
    if p[0] == "P":
        p = p.split("P")
        if 0 <= int(p[1]) < 8:
            return p[1]
        else:
            print(f"Error: {p[1]} is out of range")
            raise ValueError()
    elif p[0] == "R" and p[2] == "R" or p[3] == "R":
        p: list[int] = list(map(int, p.split("R")[1:3]))
        if p[0] % 2 == 0 and p[0] <= 14 and p[1] == p[0] + 1:
            return p[0] // 2
    else:
        return p


def remove_empty_str(tokens: list[str]) -> list[str]:
    return [token for token in tokens if token != ""]


def assemble_from_str(code: str, binary_destination: str = None) -> list[int]:
    lines = code.split("\n")
    for line in lines:
        if not add_label(line):
            print("Error: Invalid line")
            return None
    for line in lines:
        if not handle_lines(line):
            print("Error: Invalid line")
            return None
    if binary_destination is not None:
        with open(binary_destination, "wb") as f:
            f.write(bytearray(data))
    return data


def assemble_from_file(source_path: str, bin_path: str = None) -> list[int]:
    with open(source_path, "r") as f:
        content = f.read()
    return assemble_from_str(content, bin_path)


if __name__ == "__main__":
    assemble_from_file("src/web_core/build/files/fib_nums.txt", "src/web_core/build/files/fib_nums.bin")
    print("Done")