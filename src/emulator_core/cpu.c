#include "headers/cpu.h"

void initializeCPU(CPU *cpu, ROM *rom, uint16 startAddress, word startPage)
{
    cpu->C = cpu->C0 = cpu->ACC = cpu->OPA = cpu->OPR = cpu->STACK = cpu->CM = cpu->TEST = 0;
    cpu->PC = startAddress + (startPage >> 8);
    for (int i = 0; i < CPU_RAM_BANKS_N; i++)
    {
        cpu->ramBanks[i] = malloc(sizeof(RAM));
        initializeRAM(cpu->ramBanks[i]);
    }
    cpu->programRom = rom;
}

void NOP(CPU *cpu)
{
    cpu->PC++;
}

void JCN(CPU *cpu)
{
    // printf("[JUMP %d %d %d] ", cpu->ACC, cpu->C, cpu->TEST);
    bool jump = ((cpu->OPA & 0b0100) == 0b0100 && cpu->ACC == 0) ||
                ((cpu->OPA & 0b0010) == 0b0010 && cpu->C == 1) ||
                ((cpu->OPA & 0b0001) == 0b0001 && cpu->TEST == 0);
    if ((cpu->OPA & 0b1000) == 0b1000)
        jump = !jump;
    if (jump)
    {
        cpu->PC = read(cpu->programRom, ++cpu->PC);
    }
    else
        cpu->PC += 2;
}

void FIM(CPU *cpu)
{
    cpu->regPair = cpu->OPA >> 1; // may be changed to a local variable and remove cpu->regPair
    cpu->registerPairs[cpu->regPair] = read(cpu->programRom, ++cpu->PC);
    cpu->PC++;
}

void SRC(CPU *cpu)
{
    cpu->regPair = cpu->OPA >> 1;
    cpu->address8 = cpu->registerPairs[cpu->regPair];
    select(cpu->ramBanks[cpu->CM], cpu->address8);
    cpu->PC++;
}

void FIN(CPU *cpu)
{
    cpu->regPair = cpu->OPA >> 1;
    cpu->address12 = ((cpu->PC + 1) & 0xF00) + cpu->registerPairs[0];
    cpu->registerPairs[cpu->regPair] = read(cpu->programRom, cpu->address12);
    cpu->PC++;
}

void JIN(CPU *cpu)
{
    cpu->regPair = cpu->OPA >> 1;
    cpu->PC = cpu->registerPairs[cpu->regPair];
}

void JUN(CPU *cpu)
{
    cpu->address12 = (cpu->OPA << 8) + read(cpu->programRom, cpu->PC + 1);
    cpu->PC = cpu->address12;
}

void JMS(CPU *cpu)
{
    cpu->STACK <<= 12;
    cpu->STACK |= cpu->PC + 2;
    cpu->PC = (cpu->OPA << 8) + read(cpu->programRom, cpu->PC + 1);
}

void INC(CPU *cpu)
{
    cpu->regPair = cpu->OPA >> 1;
    if (cpu->OPA % 2 == 0)
    {
        cpu->reg = cpu->registerPairs[cpu->regPair] >> 4;
        cpu->reg++;
        cpu->registerPairs[cpu->regPair] &= 0x0F;
        cpu->registerPairs[cpu->regPair] |= cpu->reg << 4;
    }
    else
    {
        cpu->reg = cpu->registerPairs[cpu->regPair];
        cpu->reg++;
        cpu->registerPairs[cpu->regPair] &= 0xF0;
        cpu->registerPairs[cpu->regPair] |= cpu->reg;
    }
    cpu->PC++;
}

void ISZ(CPU *cpu)
{
    cpu->regPair = cpu->OPA >> 1;
    if (cpu->OPA % 2 == 0)
        cpu->reg = cpu->registerPairs[cpu->regPair] >> 4;
    else
        cpu->reg = cpu->registerPairs[cpu->regPair];
    cpu->reg++;
    if (cpu->reg == 0)
        cpu->PC += 2;
    else
        cpu->PC = read(cpu->programRom, cpu->PC + 1);
    if (cpu->OPA % 2 == 0)
    {
        cpu->registerPairs[cpu->regPair] &= 0x0F;
        cpu->registerPairs[cpu->regPair] += cpu->reg << 4;
    }
    else
    {
        cpu->registerPairs[cpu->regPair] &= 0xF0;
        cpu->registerPairs[cpu->regPair] += cpu->reg;
    }
}

void ADD(CPU *cpu)
{
    cpu->regPair = cpu->OPA >> 1;
    if (cpu->OPA % 2 == 0)
        cpu->reg = cpu->registerPairs[cpu->regPair] >> 4;
    else
        cpu->reg = cpu->registerPairs[cpu->regPair];
    cpu->ACC += cpu->reg;
    cpu->C = cpu->ACC < cpu->reg;
    cpu->PC++;
}

void SUB(CPU *cpu)
{
    cpu->regPair = cpu->OPA >> 1;
    if (cpu->OPA % 2 == 0)
        cpu->reg = cpu->registerPairs[cpu->regPair] >> 4;
    else
        cpu->reg = cpu->registerPairs[cpu->regPair] & 0x0F;
    cpu->address8 = cpu->ACC;
    cpu->address8 -= cpu->reg;
    cpu->C = cpu->address8 >= 16;
    cpu->ACC = cpu->address8;
    cpu->PC++;
}

void LD(CPU *cpu)
{
    cpu->regPair = cpu->OPA >> 1;
    if (cpu->OPA % 2 == 0)
        cpu->ACC = cpu->registerPairs[cpu->regPair] >> 4;
    else
        cpu->ACC = cpu->registerPairs[cpu->regPair];
    cpu->PC++;
}

void XCH(CPU *cpu)
{
    cpu->address8 = cpu->ACC;
    cpu->regPair = cpu->OPA >> 1;
    if (cpu->OPA % 2 == 0)
        cpu->reg = cpu->registerPairs[cpu->regPair] >> 4;
    else
        cpu->reg = cpu->registerPairs[cpu->regPair];
    cpu->ACC = cpu->reg;
    if (cpu->OPA % 2 == 0)
    {
        cpu->registerPairs[cpu->regPair] &= 0x0F;
        cpu->registerPairs[cpu->regPair] += cpu->address8 << 4;
    }
    else
    {
        cpu->registerPairs[cpu->regPair] &= 0xF0;
        cpu->registerPairs[cpu->regPair] += cpu->address8;
    }
    cpu->PC++;
}

void BBL(CPU *cpu)
{
    cpu->ACC = cpu->OPA;
    cpu->PC = cpu->STACK;
    cpu->STACK >>= 12;
}

void LDM(CPU *cpu)
{
    cpu->ACC = cpu->OPA;
    cpu->PC++;
}

void WRM(CPU *cpu)
{
    writeMain(cpu->ramBanks[cpu->CM], cpu->ACC);
    cpu->PC++;
}

void WMP(CPU *cpu)
{
    writeOutput(cpu->ramBanks[cpu->CM], cpu->ACC);
}

void WRR(CPU *cpu)
{
    cpu->programRom->ioPorts = cpu->ACC;
    cpu->PC++;
}

void WPM(CPU *cpu) // (only used for special peripherals) (not yet implemented)
{
    cpu->PC++;
}

void WR0(CPU *cpu)
{
    writeStatus0(cpu->ramBanks[cpu->CM], cpu->ACC);
    cpu->PC++;
}

void WR1(CPU *cpu)
{
    writeStatus1(cpu->ramBanks[cpu->CM], cpu->ACC);
    cpu->PC++;
}

void WR2(CPU *cpu)
{
    writeStatus2(cpu->ramBanks[cpu->CM], cpu->ACC);
    cpu->PC++;
}

void WR3(CPU *cpu)
{
    writeStatus3(cpu->ramBanks[cpu->CM], cpu->ACC);
    cpu->PC++;
}

void SBM(CPU *cpu)
{
    cpu->address8 = ~readMain(cpu->ramBanks[cpu->CM]);
    cpu->address8 += cpu->ACC + cpu->C;
    cpu->C = !(cpu->address8 >= 16);
    cpu->ACC = cpu->address8;
    cpu->PC++;
}

void RDM(CPU *cpu)
{
    cpu->ACC = readMain(cpu->ramBanks[cpu->CM]);
    cpu->PC++;
}

void RDR(CPU *cpu)
{
    cpu->ACC = cpu->programRom->ioPorts;
    cpu->PC++;
}

void ADM(CPU *cpu)
{
    cpu->reg = cpu->ACC;
    cpu->ACC = readMain(cpu->ramBanks[cpu->CM]);
    cpu->C = cpu->ACC < cpu->reg;
    cpu->PC++;
}

void RR0(CPU *cpu)
{
    cpu->ACC = readStatus0(cpu->ramBanks[cpu->CM]);
    cpu->PC++;
}

void RR1(CPU *cpu)
{
    cpu->ACC = readStatus1(cpu->ramBanks[cpu->CM]);
    cpu->PC++;
}

void RR2(CPU *cpu)
{
    cpu->ACC = readStatus2(cpu->ramBanks[cpu->CM]);
    cpu->PC++;
}

void RR3(CPU *cpu)
{
    cpu->ACC = readStatus3(cpu->ramBanks[cpu->CM]);
    cpu->PC++;
}

void CLB(CPU *cpu)
{
    cpu->C = cpu->ACC = 0;
    cpu->PC++;
}

void CLC(CPU *cpu)
{
    cpu->C = 0;
    cpu->PC++;
}

void IAC(CPU *cpu)
{
    cpu->ACC++;
    cpu->C = cpu->ACC == 0;
    cpu->PC++;
}

void CMC(CPU *cpu)
{
    cpu->C = !cpu->C;
    cpu->PC++;
}

void CMA(CPU *cpu)
{
    cpu->ACC = ~cpu->ACC;
    cpu->PC++;
}

void RAL(CPU *cpu)
{
    cpu->C0 = cpu->C;
    cpu->C = cpu->ACC >> 3;
    cpu->ACC <<= 1;
    cpu->ACC += cpu->C0;
    cpu->PC++;
}

void RAR(CPU *cpu)
{
    cpu->C0 = cpu->C;
    cpu->C = cpu->ACC & 1;
    cpu->ACC >>= 1;
    cpu->ACC += cpu->C0 << 3;
    cpu->PC++;
}

void TCC(CPU *cpu)
{
    cpu->ACC = cpu->C;
    cpu->C = 0;
    cpu->PC++;
}

void DAC(CPU *cpu)
{
    cpu->C = cpu->ACC == 0;
    cpu->ACC--;
    cpu->PC++;
}

void TCS(CPU *cpu)
{
    if (cpu->C == 0)
        cpu->ACC = 0b1001;
    else
        cpu->ACC = 0b1010;
    cpu->C = 0;
    cpu->PC++;
}

void STC(CPU *cpu)
{
    cpu->C = 1;
    cpu->PC++;
}

void DAA(CPU *cpu)
{
    if (cpu->ACC > 0b1001 || cpu->C == 1)
        cpu->ACC += 0b0110;
    cpu->C = cpu->ACC > 0b0110;
    cpu->PC++;
}

void KBP(CPU *cpu)
{
    if (!(cpu->ACC == 0 || cpu->ACC == 1 || cpu->ACC == 2 || cpu->ACC == 4 || cpu->ACC == 8))
        cpu->ACC = 0x0F;
    cpu->PC++;
}

void DCL(CPU *cpu)
{
    cpu->CM = cpu->ACC;
    cpu->PC++;
}

void step(CPU *cpu, bool debug)
{
    word instruction = read(cpu->programRom, cpu->PC);
    cpu->OPR = (instruction >> 4) & 0x0F;
    cpu->OPA = instruction & 0x0F;
    if (cpu->OPR <= 0xD)
    {
        switch (cpu->OPR)
        {
        case 0x0:
            NOP(cpu);
            cpu->lastInstruction = "NOP";
            break;
        case 0x1:
            JCN(cpu);
            cpu->lastInstruction = "JCN";
            break;
        case 0x2:
            if (cpu->OPA % 2 == 0)
            {
                FIM(cpu);
                cpu->lastInstruction = "FIM";
            }
            else
            {
                SRC(cpu);
                cpu->lastInstruction = "SRC";
            }
            break;
        case 0x3:
            if (cpu->OPA % 2 == 0)
            {
                FIN(cpu);
                cpu->lastInstruction = "FIN";
            }
            else
            {
                JIN(cpu);
                cpu->lastInstruction = "JIN";
            }
            break;
        case 0x4:
            JUN(cpu);
            cpu->lastInstruction = "JUN";
            break;
        case 0x5:
            JMS(cpu);
            cpu->lastInstruction = "JMS";
            break;
        case 0x6:
            INC(cpu);
            cpu->lastInstruction = "INC";
            break;
        case 0x7:
            ISZ(cpu);
            cpu->lastInstruction = "ISZ";
            break;
        case 0x8:
            ADD(cpu);
            cpu->lastInstruction = "ADD";
            break;
        case 0x9:
            SUB(cpu);
            cpu->lastInstruction = "SUB";
            break;
        case 0xA:
            LD(cpu);
            cpu->lastInstruction = "LD ";
            break;
        case 0xB:
            XCH(cpu);
            cpu->lastInstruction = "XCH";
            break;
        case 0xC:
            BBL(cpu);
            cpu->lastInstruction = "BBL";
            break;
        case 0xD:
            LDM(cpu);
            cpu->lastInstruction = "LDM";
            break;
        default:
            break;
        }
    }
    else if (cpu->OPR == 0xE)
    {
        switch (cpu->OPA)
        {
        case 0x0:
            WRM(cpu);
            cpu->lastInstruction = "WRM";
            break;
        case 0x1:
            WMP(cpu);
            cpu->lastInstruction = "WMP";
            break;
        case 0x2:
            WRR(cpu);
            cpu->lastInstruction = "WRR";
            break;
        case 0x3:
            WPM(cpu);
            cpu->lastInstruction = "WPM";
            break;
        case 0x4:
            WR0(cpu);
            cpu->lastInstruction = "WR0";
            break;
        case 0x5:
            WR1(cpu);
            cpu->lastInstruction = "WR1";
            break;
        case 0x6:
            WR2(cpu);
            cpu->lastInstruction = "WR2";
            break;
        case 0x7:
            WR3(cpu);
            cpu->lastInstruction = "WR3";
            break;
        case 0x8:
            SBM(cpu);
            cpu->lastInstruction = "SBM";
            break;
        case 0x9:
            RDM(cpu);
            cpu->lastInstruction = "RDM";
            break;
        case 0xA:
            RDR(cpu);
            cpu->lastInstruction = "RDR";
            break;
        case 0xB:
            ADM(cpu);
            cpu->lastInstruction = "ADM";
            break;
        case 0xC:
            RR0(cpu);
            cpu->lastInstruction = "RR0";
            break;
        case 0xD:
            RR1(cpu);
            cpu->lastInstruction = "RR1";
            break;
        case 0xE:
            RR2(cpu);
            cpu->lastInstruction = "RR2";
            break;
        case 0xF:
            RR3(cpu);
            cpu->lastInstruction = "RR3";
            break;
        default:
            break;
        }
    }
    else if (cpu->OPR == 0xF)
    {
        switch (cpu->OPA)
        {
        case 0x0:
            CLB(cpu);
            cpu->lastInstruction = "CLB";
            break;
        case 0x1:
            CLC(cpu);
            cpu->lastInstruction = "CLC";
            break;
        case 0x2:
            IAC(cpu);
            cpu->lastInstruction = "IAC";
            break;
        case 0x3:
            CMC(cpu);
            cpu->lastInstruction = "CMC";
            break;
        case 0x4:
            CMA(cpu);
            cpu->lastInstruction = "CMA";
            break;
        case 0x5:
            RAL(cpu);
            cpu->lastInstruction = "RAL";
            break;
        case 0x6:
            RAR(cpu);
            cpu->lastInstruction = "RAR";
            break;
        case 0x7:
            TCC(cpu);
            cpu->lastInstruction = "TCC";
            break;
        case 0x8:
            DAC(cpu);
            cpu->lastInstruction = "DAC";
            break;
        case 0x9:
            TCS(cpu);
            cpu->lastInstruction = "TCS";
            break;
        case 0xA:
            STC(cpu);
            cpu->lastInstruction = "STC";
            break;
        case 0xB:
            DAA(cpu);
            cpu->lastInstruction = "DAA";
            break;
        case 0xC:
            KBP(cpu);
            cpu->lastInstruction = "KBP";
            break;
        case 0xD:
            DCL(cpu);
            cpu->lastInstruction = "DCL";
            break;
        default:
            break;
        }
    }
    // if (debug)
    // printf("Instruction: %s (0x%02X), OPR: (0x%02X), OPA: (0x%02X)\n", cpu->lastInstruction, instruction & 0xFF, cpu->OPR, cpu->OPA);
    // printf("Instruction: (0x%02X), OPR: (0x%02X), OPA: (0x%02X)\n", instruction & 0xFF, cpu->OPR, cpu->OPA);
}

void execute(CPU *cpu, int steps, bool debug)
{
    for (int i = 0; i < steps; i++)
    {
        step(cpu, debug);
    }
}

void freeCPU(CPU *cpu)
{
    for (int i = 0; i < CPU_RAM_BANKS_N; i++)
    {
        free(cpu->ramBanks[i]);
    }
    free(cpu);
}