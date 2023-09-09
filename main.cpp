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
    word memory[256 * 16]{};
public:
    explicit ROM(word* page) {
        for (int i = 0; i< 256; i++) {
            this->memory[i] = page[i];
        }
        pageCount++;
    }
    word read(uint16 address)
    {
        return memory[address];
    }
//    void addPage(word* page) {
//        if (pageCount > 0) {
//            this->memory[pageCount] = page;
//            pageCount++;
//        }
//    }
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
        index = chip << 5 | reg << 3 | character >> 1;
        if (character % 2 == 0) {
            return memory[index] >> 4;
        }
        return memory[index] & 0x0F;
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

    RAM* ramBanks[8]{};
    word registerPairs[8]{};
    ROM* programRom{};
    uint64 C     : 1;
    uint64 C0    : 1;
    uint64 TEST  : 1;
    uint64 ACC   : 4;
    uint64 OPA   : 4;
    uint64 OPR   : 4;
    uint64 PC    : 12;
    uint64 PC1   : 12;
    uint64 PC2   : 12;
    uint64 PC3   : 12;
    uint16 CM : 4;

    uint32 reg : 4;
    uint32 regPair : 3;
    uint32 pcMask : 12;
    uint32 address12 : 12;
    uint16 address8 : 8;


public:
    void runProgram(int cycles, uint16 startAddress, word startPage) {
        PC = startAddress + (startPage >> 8);
        word instruction;
        while (cycles > 0) {
            instruction = programRom->read(PC);
            OPR = instruction >> 4;
            OPA = instruction & 0x0F;
            if (OPR <= 0xD) {
                bool jump;
                switch (OPR) {
                    case 0x0: // Instruction: NOP
                        PC++;
                        cycles--;
                        break;
                    case 0x1: // Instruction: JCN
                        jump = ((OPA & 0b0100) == 0b0100 && ACC == 0) ||
                               ((OPA & 0b0010) == 0b0010 && C == 1) ||
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
                            ramBanks[CM]->select(address8);
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
                        PC1 = PC + 1;
                        PC = (OPA << 8) + programRom->read(PC + 1);
                        cycles -= 2;
                    case 0x6: // Instruction: INC
                        regPair = OPA >> 1;
                        if (OPA % 2 == 0) {
                            reg = registerPairs[regPair] >> 4;
                            reg++;
                            registerPairs[regPair] &= 0x0F;
                            registerPairs[regPair] |= reg << 4;
                        } else {
                            reg = registerPairs[regPair];
                            reg = ++reg;
                            registerPairs[regPair] &= 0xF0;
                            registerPairs[regPair] |= reg;
                        }
                        PC++;
                        cycles--;
                        break;
                    case 0x7: // Instruction: ISZ
                        regPair = OPA >> 1;
                        if (OPA % 2 == 0) {
                            reg = registerPairs[regPair] >> 4;
                        } else {
                            reg = registerPairs[regPair];
                        }
                        reg++;
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
                        if (OPA % 2 == 0) {
                            reg = registerPairs[regPair] >> 4;
                        } else {
                            reg = registerPairs[regPair];
                        }
                        ACC += reg;
                        if (ACC < reg) C = 1;
                        PC++;
                        cycles--;
                        break;
                    case 0x9: // Instruction: SUB
                        regPair = OPA >> 1;
                        if (OPA % 2 == 0) {
                            address8 = ~registerPairs[regPair] >> 4;
                        } else {
                            address8 = ~registerPairs[regPair];
                        }
                        address8 += ACC + C;
                        if (address8 >= 16) {
                            C = 0;
                        } else {
                            C = 1;
                        }
                        ACC = address8;
                        PC++;
                        cycles--;
                        break;
                    case 0xA: // Instruction: LD
                        regPair = OPA >> 1;
                        if (OPA % 2 == 0) {
                            reg = registerPairs[regPair] >> 4;
                        } else {
                            reg = registerPairs[regPair];
                        }
                        PC++;
                        cycles--;
                        break;
                    case 0xB: // Instruction: XCH
                        address8 = ACC; //tempACC
                        regPair = OPA >> 1;
                        if (OPA % 2 == 0) {
                            reg = registerPairs[regPair] >> 4;
                        } else {
                            reg = registerPairs[regPair];
                        }
                        ACC = reg;
                        if (OPA % 2 == 0) {
                            registerPairs[regPair] &= 0x0F;
                            registerPairs[regPair] += address8 << 4;
                        } else {
                            registerPairs[regPair] &= 0xF0;
                            registerPairs[regPair] += address8;
                        }
                        PC++;
                        cycles--;
                        break;
                    case 0xC: // Instruction: BBL
                        ACC = OPA;
                        PC = PC1;
                        PC1 = PC2;
                        PC2 = PC3;
                        PC++;
                        cycles--;
                    case 0x0D: // Instruction: LDM
                        ACC = OPA;
                        PC++;
                        cycles--;
                        break;
                    default:
                        break;
                }
            } else if (OPR == 0xE) {
                switch (OPA) {
                    case 0x0: // Instruction: WRM
                        ramBanks[CM]->writeMain(ACC);
                        PC++;
                        cycles--;
                        break;
                    case 0x1: // Instruction: WMP
                        ramBanks[CM]->writeOutput(ACC);
                        PC++;
                        cycles--;
                        break;
                    case 0x2: // Instruction: WRR
                        programRom->ioPorts = ACC;
                        PC++;
                        cycles--;
                        break;
                    case 0x3: // Instruction: WPM (only used for special peripherals)
                        PC++;
                        cycles--;
                        break;
                    case 0x4: // Instruction: WR0
                        ramBanks[CM]->writeStatus0(ACC);
                        PC++;
                        cycles--;
                        break;
                    case 0x5: // Instruction: WR1
                        ramBanks[CM]->writeStatus1(ACC);
                        PC++;
                        cycles--;
                        break;
                    case 0x6: // Instruction: WR2
                        ramBanks[CM]->writeStatus2(ACC);
                        PC++;
                        cycles--;
                        break;
                    case 0x7: // Instruction: WR3
                        ramBanks[CM]->writeStatus3(ACC);
                        PC++;
                        cycles--;
                        break;
                    case 0x8: // Instruction: SBM
                        address8 = ~ramBanks[CM]->readMain(); // memory character
                        address8 += ACC + C;
                        if (address8 >= 16) {
                            C = 0;
                        } else {
                            C = 1;
                        }
                        ACC = address8;
                        PC++;
                        cycles--;
                        break;
                    case 0x9: // Instruction: RDM
                        ACC = ramBanks[CM]->readMain();
                        PC++;
                        cycles--;
                        break;
                    case 0xA: // Instruction: RDR
                        ACC = programRom->ioPorts;
                        PC++;
                        cycles--;
                        break;
                    case 0xB: // Instruction: ADM
                        reg = ACC; //tempACC
                        ACC += ramBanks[CM]->readMain();
                        if (ACC < reg) {
                            C = 1;
                        }
                        PC++;
                        cycles--;
                        break;
                    case 0xC: // Instruction: RD0
                        ACC = ramBanks[CM]->readStatus0();
                        PC++;
                        cycles--;
                        break;
                    case 0xD: // Instruction: RD1
                        ACC = ramBanks[CM]->readStatus1();
                        PC++;
                        cycles--;
                        break;
                    case 0xE: // Instruction: RD2
                        ACC = ramBanks[CM]->readStatus2();
                        PC++;
                        cycles--;
                        break;
                    case 0xF: // Instruction: RD3
                        ACC = ramBanks[CM]->readStatus3();
                        PC++;
                        cycles--;
                        break;
                }
            } else if (OPR == 0xF) {
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
                        CM = ACC;
                        PC++;
                        cycles--;
                        break;
                    default:
                        break;
                }
                PC++;
                cycles--;
            }
        }
    }

public:
    CPU()
    {
        C = ACC = OPA = OPR = PC = PC1 = PC2 = PC3 = CM = TEST = 0;
        for (auto & ram : ramBanks) {
            ram = new RAM();
        }
    }
    explicit CPU(ROM* programRom) : CPU()
    {
        this->programRom = programRom;
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
    word programMemArray[256];
    for (word & i : programMemArray) {
        i = 0;
    }
    loadProgram("C:\\Users\\elias\\CLionProjects\\Intel_4004_Emulator\\test.bin", programMemArray);
    auto* programMemory = new ROM(programMemArray);
    auto* cpu = new CPU(programMemory);
    cpu->runProgram(5000000, 0, 0);

    return 0;
}
