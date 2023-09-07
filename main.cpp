// http://datasheets.chipdb.org/Intel/MCS-4/datashts/intel-4004.pdf
// http://e4004.szyc.org/iset.html
// https://www.inf.pucrs.br/~calazans/undergrad/orgcomp_EC/mat_microproc/Intel-4004_InstructionSet.pdf
// http://e4004.szyc.org/asm.html

#include <iostream>
#include <fstream>
#include <string>
#include <stdio.h>
#include <stdlib.h>
#include <map>

using word = unsigned char;
using byte = unsigned char;
using chunk64 = unsigned long long;

class ROM {
    int byteSize;
protected:
    word *memory;
public:
    explicit ROM(word* memory) {
        this->memory = memory;
        this->byteSize = sizeof(this->memory);
    }
    word fetch(unsigned short address)
    {
        if (address > 255) return '\0';
        return memory[address];
    }
};

class RAM {
public:
    byte memory[40];
    byte fetch(unsigned short address) {
        if (address > 39) return '\0';
        return memory[address];
    }
    void store(unsigned short address, byte data) {
        if (address > 39) return;
        memory[address] = data;
    }

};

class CPU {


    word registerPairs[8];
    chunk64 REGISTERS = 0;  // R15 least significant, R0 most significant
    chunk64 C         : 1;
    chunk64 TEST      : 1;
    chunk64 ACC       : 4;
    chunk64 OPA       : 4;
    chunk64 OPR       : 4;
    chunk64 PC        : 12;
    chunk64 PC1       : 12;
    chunk64 PC2       : 12;
    chunk64 PC3       : 12;
    ROM* programRom;
    RAM* dataRam;


    // instruction set
    constexpr static word NOP = 0x00;
    constexpr static word JCN = 0x10;
    constexpr static word FIM = 0x20;
    constexpr static word FIN = 0x30;
    constexpr static word JIN = 0x40;
    constexpr static word JUN = 0x50;
    constexpr static word JMS = 0x60;
    constexpr static word INC = 0x70;
    constexpr static word ISZ = 0x80;
    constexpr static word ADD = 0x90;
    constexpr static word SUB = 0xA0;
    constexpr static word LD  = 0xB0;
    constexpr static word XCH = 0xC0;
    constexpr static word BBL = 0xD0;
    constexpr static word LDM = 0xE0;
    constexpr static word CLB = 0xF0;
    constexpr static word CLC = 0xF1;
    constexpr static word IAC = 0xF2;
    constexpr static word CMC = 0xF3;
    constexpr static word CMA = 0xF4;
    constexpr static word RAL = 0xF5;
    constexpr static word RAR = 0xF6;
    constexpr static word TCC = 0xF7;
    constexpr static word DAC = 0xF8;
    constexpr static word TCS = 0xF9;
    constexpr static word STC = 0xFA;
    constexpr static word DAA = 0xFB;
    constexpr static word KBP = 0xFC;
    constexpr static word DCL = 0xFD;
public:
    void runProgram(int cycles, unsigned short startAddress) {
        PC = startAddress;
        word instruction;
        word C0;
        while (cycles > 0) {
            instruction = programRom->fetch(PC);
            OPR = instruction >> 4;
            OPA = instruction & 0x0F;
            if (OPR == 0xF) {
                switch (OPA) {
                    case 0x0:
                        ACC = C = 0;
                        break;
                    case 0x1:
                        C = 0;
                        break;
                    case 0x2:
                        ACC++;
                        if (ACC == 0) C = 1;
                        break;
                    case 0x3:
                        C = ~C;
                        break;
                    case 0x4:
                        ACC = ~ACC;
                        break;
                    case 0x5:
                        C0 = C;
                        C = ACC >> 3;
                        ACC = ACC << 1;
                        ACC += C0;
                        break;
                    case 0x6:
                        C0 = C;
                        C = ACC & 0b1;
                        ACC = ACC >> 1;
                        ACC += C0 << 3;
                        break;
                    case 0x7:
                        ACC = C;
                        C = 0;
                        break;
                    case 0x8:
                        if (ACC == 0) C = 1;
                        ACC--;
                        break;
                    case 0x9:
                        if (C == 0) ACC = 0b1001;
                        else ACC = 0b1010;
                        C = 0;
                        break;
                    case 0xA:
                        C = 1;
                        break;
                    case 0xB:
                        if (ACC > 0b1001 || C == 1) ACC += 0b0110;
                        if (ACC < 0b0110) C = 1;
                        break;
                    case 0xC:
                        if (!(ACC == 0 || ACC == 1 || ACC == 2 || ACC == 4 || ACC == 8)) ACC = 0xF;
                        break;
                    case 0xD:
                        break;
                    default:
                        break;
                }
                PC++;
                cycles--;
            } else {
                bool jump;
                word reg, regPair, wAddress;
                short sAddress;
                chunk64 regMask;
                unsigned short pcMask;
                switch (OPR) {
                    case 0x0:
                        PC++;
                        cycles--;
                        break;
                    case 0x1:
                        jump = false;
                        if (((OPA & 0b0100) == 0b0100 && ACC == 0) ||
                            ((OPA & 0b0010) == 0b0010 && C == 1) ||
                            ((OPA & 0b0001) == 0b0001 && TEST == 0)) jump = true;
                        if ((OPA & 0b1000) == 0b1000) jump = !jump;
                        if (jump) {
                            PC = programRom->fetch(++PC);
                        } else {
                            PC += 2;
                        }
                        cycles -= 2;
                        break;
                    case 0x2:
                        if (OPA % 2 == 0) {
                            regPair = OPA >> 1;
                            registerPairs[regPair] = programRom->fetch(++PC);
                            PC++;
                            cycles--;
                        } else {
                            // Instruction: SRC
                        }
                        cycles--;
                        break;
                    case 0x3:
                        if (OPA % 2 == 0) {
                            regPair = OPA >> 1;
                            registerPairs[regPair] = dataRam->fetch(registerPairs[0]);
                            PC++;
                        } else {
                            regPair = OPA >> 1;
                            PC = registerPairs[regPair];
                        }
                    case 0x4:
                        sAddress = ((short) OPA << 8) + programRom->fetch(PC + 1);
                        PC = sAddress;
                        cycles -= 2;
                        break;
                    case 0x5:
                        PC3 = PC2;
                        PC2 = PC1;
                        PC1 = PC+1;
                        PC = (OPA << 8) + programRom->fetch(PC + 1);
                        cycles -= 2;
                    case 0x6:
                        regPair = OPA >> 1;
                        reg = registerPairs[regPair];
                        if (OPA % 2 == 0) {
                            reg >>= 4;
                            reg = ++reg % 16;
                            registerPairs[regPair] &= 0x0F;
                            registerPairs[regPair] += reg << 4;
                        } else {
                            reg &= 0xF;
                            reg = ++reg % 16;
                            registerPairs[regPair] &= 0xF0;
                            registerPairs[regPair] += reg;
                        }
                        PC++;
                        cycles--;
                        break;
                    case 0x7:
                        regPair = OPA >> 1;
                        reg = registerPairs[regPair];
                        if (OPA % 2 == 0) {
                            reg >>= 4;
                        } else {
                            reg &= 0xF;
                        }
                        reg = ++reg % 16;
                        if (reg == 0) {
                            PC += 2;
                        } else {
                            PC = programRom->fetch(PC + 1);
                        }
                        if (OPA % 2 == 0) {
                            registerPairs[regPair] &= 0x0F;
                            registerPairs[regPair] += reg << 4;
                        } else {
                            registerPairs[regPair] &= 0xF0;
                            registerPairs[regPair] += reg;
                        }
                        cycles -= 2;
                        break;
                    case 0x8:
                        regPair = OPA >> 1;
                        reg = registerPairs[regPair];
                        if (OPA % 2 == 0) {
                            reg >>= 4;
                        } else {
                            reg &= 0xF;
                        }
                        ACC += reg;
                        if (ACC < reg) C = 1;
                        PC++;
                        cycles--;
                        break;
                    case 0x9:
                        regPair = OPA >> 1;
                        reg = registerPairs[regPair];
                        if (OPA % 2 == 0) {
                            reg >>= 4;
                        } else {
                            reg &= 0xF;
                        }
                        reg = -reg;
                        reg += ACC + C;
                        if (reg >= 16) C = 0;
                        ACC = reg;
                        PC++;
                        cycles--;
                        break;
                    case 0xA:
                        regPair = OPA >> 1;
                        reg = registerPairs[regPair];
                        if (OPA % 2 == 0) {
                            ACC = reg >> 4;
                        } else {
                            ACC = reg & 0xF;
                        }
                        PC++;
                        cycles--;
                        break;
                    case 0xB:
                        pcMask = ACC; //tempACC
                        regPair = OPA >> 1;
                        reg = registerPairs[regPair];
                        if (OPA % 2 == 0) {
                            ACC = reg >> 4;
                        } else {
                            ACC = reg & 0xF;
                        }
                        if (OPA % 2 == 0) {
                            registerPairs[regPair] &= 0x0F;
                            registerPairs[regPair] += pcMask << 4;
                        } else {
                            registerPairs[regPair] &= 0xF0;
                            registerPairs[regPair] += pcMask;
                        }
                        PC++;
                        cycles--;
                        break;
                    case 0xC:
                        ACC = OPA;
                        PC = PC1;
                        PC1 = PC2;
                        PC2 = PC3;
                        cycles--;
                    case 0x0D:
                        ACC = OPA;
                        PC++;
                        cycles--;
                        break;
                    default:
                        break;
                }
            }
        }
    }

public:
    CPU() = default;
    CPU(RAM* dataRam)
    {
        this->dataRam = dataRam;
    }
    CPU(ROM* programRom)
    {
        this->programRom = programRom;
    }
    CPU(RAM* dataRam, ROM* programRom)
    {
        this->dataRam = dataRam;
        this->programRom = programRom;
    }
    void connectDataRam(RAM* memory) {
        dataRam = memory;
    }
    void connectProgramRom(ROM* memory) {
        programRom = memory;
    }

};

constexpr char wordToChar[16] {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
                                 'A', 'B', 'C', 'D', 'E', 'F'};

//void generateProgram(char* path, word* characters) {
//    int numOfCharacters = 0;
//    std::ifstream stream(path);
//    std::string line;
//    if (!stream.is_open()) {
//        std::cout << "Unable to open file";
//        exit(1);
//    }
//    while (std::getline(stream, line)) {
//        numOfCharacters += line.length();
//    }
//    stream.close();
//    if (numOfCharacters % 2 == 1) {
//        numOfCharacters = numOfCharacters / 2 + 1;
//    } else {
//        numOfCharacters = numOfCharacters / 2;
//    }
//    characters = new word[numOfCharacters];
//    stream.open(path);
//    std::map<char, word> charToWord;
//    for (int i = 0; i < 16; i++) {
//        charToWord[wordToChar[i]] = i;
//    }
//    int i = 0;
//    char c;
//    while (stream >> c) {
//        word byte = charToWord[c];
//        if (stream >> c) {
//            byte = byte << 4;
//            byte += charToWord[c];
//        }
//        characters[i++] = byte;
//    }
//}

int sizeOfFile(char* path) {
    std::ifstream in_file(path, std::ios::binary);
    in_file.seekg(0, std::ios::end);
    return in_file.tellg();
}

void loadProgram(char* path, word* characters) {
    std::ifstream file(path, std::ios::in | std::ios::binary);
    file.read((char*) characters, sizeOfFile(path));
    file.close();
}

int main() {
    auto* dataMemory = new RAM();
    word programMemArray[256];
    for (word & i : programMemArray) {
        i = 0;
    }
    loadProgram("C:\\Users\\elias\\CLionProjects\\Intel 4004 Emulator\\test.bin", programMemArray);
    auto* programMemory = new ROM(programMemArray);
    auto* cpu = new CPU(dataMemory, programMemory);
    cpu->runProgram(5000000, 0);

    return 0;
}
