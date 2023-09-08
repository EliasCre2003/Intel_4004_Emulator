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

using word = unsigned __int8;
using byte = unsigned __int8;
using uint8 = unsigned __int8;
using uint16 = unsigned __int16;
using uint32 = unsigned __int32;
using uint64 = unsigned __int64;

class ROM {
protected:
    word pageCount : 4;
    word* memory[16]{};
public:
    explicit ROM(word* page) {
        this->memory[0] = page;
        pageCount++;
    }
    word read(uint16 address)
    {
        word page = address << 8;
        address %= 256;
        return memory[page][address];
    }
    void addPage(word* page) {
        if (pageCount > 0) {
            this->memory[pageCount] = page;
            pageCount++;
        }
    }
    word ioPorts : 4;
};

class RAM {
protected:
    byte memory[128];
    byte status[32];
    byte outputs[2];
    word chip      : 2;
    word reg       : 2;
    word character : 4;
    word index = 0;
public:
    void select(word address) {
        chip = address >> 6;
        reg = (address >> 4) & 0b11;
        character = address & 0b1111;
    }
    void writeMain(byte data) {
        index = chip << 5 | reg << 3 | character >> 1;
        if (character % 2 == 0) {
            data <<= 4;
            memory[index] &= 0x0F;
        } else {
            memory[index] &= 0xF0;
        }
        memory[index] |= data;
    }
    byte readMain() {
        return memory[chip << 6 | reg << 4 | character];
    }
    void writeStatus0(byte data) {
        index = chip << 3 | reg << 1 | 0;
        status[index] &= 0x0F;
        status[index] |= data << 4;
    }
    byte readStatus0() {
        index = chip << 3 | reg << 1 | 0;
        return status[index] >> 4;
    }
    void writeStatus1(byte data) {
        index = chip << 3 | reg << 1 | 0;
        status[index] &= 0xF0;
        status[index] |= data;
    }
    byte readStatus1() {
        index = chip << 3 | reg << 1 | 0;
        return status[index] & 0x0F;
    }
    void writeStatus2(byte data) {
        index = chip << 3 | reg << 1 | 1;
        status[index] &= 0x0F;
        status[index] |= data << 4;
    }
    byte readStatus2() {
        index = chip << 3 | reg << 1 | 1;
        return status[index] >> 4;
    }
    void writeStatus3(byte data) {
        index = chip << 3 | reg << 1 | 1;
        status[index] &= 0xF0;
        status[index] |= data;
    }
    byte readStatus3() {
        index = chip << 3 | reg << 1 | 1;
        return status[index] & 0x0F;
    }
    void writeOutput(byte data) {
        if (chip % 2 == 0) {
            outputs[chip >> 1] &= 0x0F;
            outputs[chip >> 1] |= data << 4;
        } else {
            outputs[chip >> 1] &= 0xF0;
            outputs[chip >> 1] |= data;
        }
    }

};

class CPU {

    word registerPairs[8]{};
    uint64 C    : 1;
    uint64 C0   : 1;
    uint64 TEST : 1;
    uint64 ACC  : 4;
    uint64 OPA  : 4;
    uint64 OPR  : 4;
    uint64 PC   : 12;
    uint64 PC1  : 12;
    uint64 PC2  : 12;
    uint64 PC3  : 12;
    ROM* programRom{};
    RAM* dataRam{};

    uint32 reg : 4;
    uint32 regPair : 3;
    uint32 pcMask : 12;
    uint32 address12 : 12;
    word address8 = 0;


public:
    void runProgram(int cycles, uint16 startAddress, word startPage) {
        PC = startAddress + (startPage >> 8);
        word instruction;
        while (cycles > 0) {
            instruction = programRom->read(PC);
            OPR = instruction >> 4;
            OPA = instruction & 0x0F;
            if (OPR == 0xF) {
                switch (OPA) {
                    case 0x0: // Instruction: CLB
                        ACC = C = 0;
                        break;
                    case 0x1: // Instruction: CLC
                        C = 0;
                        break;
                    case 0x2: // Instruction: IAC
                        ACC++;
                        if (ACC == 0) C = 1;
                        break;
                    case 0x3: // Instruction: CMC
                        C = ~C;
                        break;
                    case 0x4: // Instruction: CMA
                        ACC = ~ACC;
                        break;
                    case 0x5: // Instruction: RAL
                        C0 = C;
                        C = ACC >> 3;
                        ACC = ACC << 1;
                        ACC += C0;
                        break;
                    case 0x6: // Instruction: RAR
                        C0 = C;
                        C = ACC & 0b1;
                        ACC = ACC >> 1;
                        ACC += C0 << 3;
                        break;
                    case 0x7: // Instruction: TCC
                        ACC = C;
                        C = 0;
                        break;
                    case 0x8: // Instruction: DAC
                        if (ACC == 0) C = 1;
                        ACC--;
                        break;
                    case 0x9: // Instruction: TCS
                        if (C == 0) ACC = 0b1001;
                        else ACC = 0b1010;
                        C = 0;
                        break;
                    case 0xA: // Instruction: STC
                        C = 1;
                        break;
                    case 0xB: // Instruction: DAA
                        if (ACC > 0b1001 || C == 1) ACC += 0b0110;
                        if (ACC < 0b0110) C = 1;
                        break;
                    case 0xC: // Instruction: KBP
                        if (!(ACC == 0 || ACC == 1 || ACC == 2 || ACC == 4 || ACC == 8)) ACC = 0xF;
                        break;
                    case 0xD: // Instruction: DCL
                        break;
                    default:
                        break;
                }
                PC++;
                cycles--;
            } else {
                bool jump;
                switch (OPR) {
                    case 0x0: // Instruction: NOP
                        PC++;
                        cycles--;
                        break;
                    case 0x1: // Instruction: JCN
                        jump = ((OPA & 0b0100) == 0b0100 && ACC  == 0) ||
                               ((OPA & 0b0010) == 0b0010 && C    == 1) ||
                               ((OPA & 0b0001) == 0b0001 && TEST == 0);
                        if ((OPA & 0b1000) == 0b1000) jump = !jump;
                        if (jump) {
                            PC = programRom->read(++PC);
                        } else {
                            PC += 2;
                        }
                        cycles -= 2;
                        break;
                    case 0x2:
                        if (OPA % 2 == 0) { // Instruction: FIM
                            regPair = OPA >> 1;
                            registerPairs[regPair] = programRom->read(++PC);
                            PC++;
                            cycles--;
                        } else { // Instruction: SRC
                            regPair = OPA >> 1;
                            address8 = registerPairs[regPair];
                            dataRam->select(address8);
                            PC++;
                            cycles--;
                        }
                        cycles--;
                        break;
                    case 0x3:
                        if (OPA % 2 == 0) { // Instruction: FIN
                            regPair = OPA >> 1;
                            address12 = PC & 0xF00;
                            registerPairs[regPair] = programRom->read(address12 + registerPairs[0]);
                            PC++;
                        } else { // Instruction: JIN
                            regPair = OPA >> 1;
                            PC = registerPairs[regPair];
                        }
                    case 0x4: // Instruction: JUN
                        address12 = (OPA << 8) + programRom->read(PC + 1);
                        PC = address12;
                        cycles -= 2;
                        break;
                    case 0x5: // Instruction: JMS
                        PC3 = PC2;
                        PC2 = PC1;
                        PC1 = PC+1;
                        PC = (OPA << 8) + programRom->read(PC + 1);
                        cycles -= 2;
                    case 0x6: // Instruction: INC
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
                    case 0x7: // Instruction: ISZ
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
                            PC = programRom->read(PC + 1);
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
                    case 0x8: // Instruction: ADD
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
                    case 0x9: // Instruction: SUB
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
                    case 0xA: // Instruction: LD
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
                    case 0xB: // Instruction: XCH
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
                    case 0xC: // Instruction: BBL
                        ACC = OPA;
                        PC = PC1;
                        PC1 = PC2;
                        PC2 = PC3;
                        cycles--;
                    case 0x0D: // Instruction: LDM
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
    CPU()
    {
        C = ACC = OPA = OPR = PC = PC1 = PC2 = PC3 = TEST = 0;
    }
    explicit CPU(RAM* dataRam) : CPU()
    {
        this->dataRam = dataRam;
    }
    explicit CPU(ROM* programRom) : CPU()
    {
        this->programRom = programRom;
    }
    CPU(RAM* dataRam, ROM* programRom) : CPU()
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
    cpu->runProgram(5000000, 0, 0);

    return 0;
}
