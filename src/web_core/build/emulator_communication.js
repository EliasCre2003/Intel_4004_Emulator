const hex = ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"];
const twoWordInstructions = ["JUN", "JMS", "JCN", "ISZ", "FIM"]

class CPU {
    constructor(ptrStruct, exports, buffer) {
        this.ptr = ptrStruct;
        this.ramBanks = [];
        this.buffer = buffer;
        this.exports = exports;
        var bytes = new Uint8Array(this.buffer, this.ptr).slice(0, 58);
        for (var i = 0; i < 8; i++) {
            this.ramBanks.push(new RAM(toPtr(bytes, 24 + 4*i), this.buffer));
        }
        this.update(bytes);   
    }

    update(bytes=NaN) {
        if (isNaN(bytes)) {
            bytes = new Uint8Array(this.buffer, this.ptr).slice(0, 58);
        }
        this.C = bytes[0] & 0b0001;
        this.TEST = (bytes[0] & 0b0000_0100) >> 2;
        this.ACC = (bytes[0] & 0b0111_1000) >> 3;
        // this.OPA = ((bytes[0] & 0b1000_0000) >> 7) | ((bytes[1] & 0b0000_0111) << 1);
        // this.OPR = (bytes[1] & 0b0111_1000) >> 3;
        this.PC = ((bytes[1] & 0b1000_0000) >> 7) | (bytes[2] << 1) | ((bytes[3] & 0b0000_0111) << 9);
        this.STACK = ((bytes[3] & 0b1111_1000) >> 3) | (bytes[4] << 5) | (bytes[5] << 13) | (bytes[6] << 21) | ((bytes[7] & 0b0111_1111) << 29);
        this.CM = bytes[8] & 0b0000_1111;
        this.registerPairs = [bytes[12], bytes[13], bytes[14], bytes[15], bytes[16], bytes[17], bytes[18], bytes[19]];
        this.programRom = this.decodeROM(toPtr(bytes, 20));
        for (const r of this.ramBanks) {r.update();}
        this.lastInstruction = bytes[56];
        this.lastData = bytes[57];
    }
    stepCPU(steps=1) {
        this.exports.stepCPU(steps);
        this.update();
        updateGUI(this);
    }

    decodeROM(ptrStruct) {
        var bytes = new Uint8Array(this.buffer, ptrStruct).slice(0, 4097);
        var rom = {
            memory: bytes.slice(0, 4096),
            ioPorts: bytes[4096] & 0b0000_1111,
        }
        return rom;
    }

}


class RAM {
    constructor(ptrStruct, buffer) {
        this.ptr = ptrStruct;
        this.buffer = buffer;
        this.update();
    }

    update() {
        var bytes = new Uint8Array(this.buffer, this.ptr).slice(0, 164);
        this.memory = bytes.slice(0, 128);
        this.status = bytes.slice(128, 160);
        this.outputs = [bytes[160], bytes[161]];
        this.chip = bytes[162] & 0b0000_0011;
        this.reg = (bytes[162] & 0b0000_1100) >> 2;
        this.character = (bytes[162] & 0b1111_0000) >> 4;
        this.index = bytes[163];
    }
}


function updateGUI(cpu) {
    const opcode = document.getElementById("opcode");
    
    // TODO: Replace cpu.lastInstruction with cpu.lastPC as index to the ROM
    opcode.innerHTML = getInstruction(cpu);
    getInstruction(cpu);

    const acc = document.getElementById("acc");
    const accText = `0x${cpu.ACC.toString(16).toUpperCase()} 0b${cpu.ACC.toString(2).padStart(4, "0")} ${cpu.ACC.toString().padStart(2, "0")}`;
    acc.innerHTML = accText;

    const pc = document.getElementById("pc");
    const pcText = `0x${cpu.PC.toString(16).toUpperCase().padStart(3, "0")}
                    0x${((cpu.STACK) & 0xFFF).toString(16).toUpperCase().padStart(3, "0")}
                    0x${((cpu.STACK >> 12) & 0xFFF).toString(16).toUpperCase().padStart(3, "0")}
                    0x${((cpu.STACK >> 24) & 0xFFF).toString(16).toUpperCase().padStart(3, "0")}`;
    pc.innerHTML = pcText;

    for (var i = 0; i < 8; i++) {
        let reg = document.getElementById(`r${2*i}`);
        reg.innerHTML = `${((cpu.registerPairs[i] >> 4) & 0xF).toString(16).toUpperCase()}`;
        reg = document.getElementById(`r${2*i+1}`);
        reg.innerHTML = `${(cpu.registerPairs[i] & 0xF).toString(16).toUpperCase()}`;
    }

    const cFlag = document.getElementById("cflag");
    cFlag.innerHTML = `${cpu.C}`;

    const testFlag = document.getElementById("tflag");
    testFlag.innerHTML = `${cpu.TEST}`;

    const cm = document.getElementById("CM");
    cm.innerHTML = `0x${cpu.CM.toString(16).toUpperCase()}`;
    

    updateRomView(cpu);
    updateRamView(cpu);
}

function toPtr(array, startIndex) {
    return array[startIndex] | (array[startIndex + 1] << 8) | (array[startIndex + 2] << 16) | (array[startIndex + 3] << 24);
}


function updateRomView(cpu) {
    let romData = cpu.programRom.memory;
    const offset = document.getElementById("rom_page_select").selectedIndex * 0xFF;
    let colorNext = false;
    for (let i = 0; i < 16; i++) {
        for (let j = 0; j < 16; j++) {
            const bitAddress = i * 16 + j + offset;
            let cell = document.getElementById("rom" + hex[i] + hex[j]);
            let byteText = romData[bitAddress].toString(16).toUpperCase().padStart(2, "0");
            cell.innerHTML = byteText;
            if (colorNext) {
                cell.style = "background-color: #FFFF00";
                colorNext = false;
            } 
            else if (cpu.PC == bitAddress) {
                cell.style = "background-color: #00FF00";
                if (twoWordInstructions.includes(getOpcode(romData[bitAddress]))) colorNext = true;
            }
            else if (cell.style != "") cell.style = "";
        }
    }
}

function updateRamView(cpu) {
    let chip = document.getElementById("ram_chip_select").selectedIndex;
    let bank = document.getElementById("ram_bank_select").selectedIndex;
    let ramData = cpu.ramBanks[bank].memory;
    for (var i = 0; i < 4; i++) {
        for (var j = 0; j < 8; j++) {
            var byte = ramData[(i + 4 * chip) * 8 + j]

            var cell = document.getElementById("ram" + hex[i] + hex[2*j]);
            var byteText = (byte >> 4).toString(16).toUpperCase();
            cell.innerHTML = byteText;

            cell = document.getElementById("ram" + hex[i] + hex[2*j+1]);
            byteText = (byte & 0xF).toString(16).toUpperCase();
            cell.innerHTML = byteText;
        }
    }
}

function getOpcode(cpu) {
    if (cpu instanceof CPU)
    {
        var instruction = cpu.programRom.memory[cpu.PC];
    }
    else
        var instruction = cpu;
    const OPR = (instruction >> 4) & 0x0F;
    const OPA = instruction & 0x0F;
    if (OPR <= 0xD) {
        switch (OPR) {
            case 0x0: return "NOP";
            case 0x1: return "JCN";
            case 0x2: 
                if (OPA % 2 === 0) return "FIM";
                else  return "SRC";
            case 0x3: 
                if (OPA % 2 === 0) return "FIN";
                else  return "JIN";
            case 0x4: return "JUN";
            case 0x5: return "JMS";
            case 0x6: return "INC";
            case 0x7: return "ISZ";
            case 0x8: return "ADD";
            case 0x9: return "SUB";
            case 0xA: return "LD ";
            case 0xB: return "XCH";
            case 0xC: return "BBL";
            case 0xD: return "LDM";
        }
    }
    else if (OPR === 0xE) {
        switch (OPA) {
            case 0x0: return "WRM";
            case 0x1: return "WMP";
            case 0x2: return "WRR";
            case 0x3: return "WPM";
            case 0x4: return "WR0";
            case 0x5: return "WR1";
            case 0x6: return "WR2";
            case 0x7: return "WR3";
            case 0x8: return "SBM";
            case 0x9: return "RDM";
            case 0xA: return "RDR";
            case 0xB: return "ADM";
            case 0xC: return "RR0";
            case 0xD: return "RR1";
            case 0xE: return "RR2";
            case 0xF: return "RR3";
        }
    }
    else if (OPR === 0xF) {
        switch (OPA) {
            case 0x0: return "CLB";
            case 0x1: return "CLC";
            case 0x2: return "IAC";
            case 0x3: return "CMC";
            case 0x4: return "CMA";
            case 0x5: return "RAL";
            case 0x6: return "RAR";
            case 0x7: return "TCC";
            case 0x8: return "DAC";
            case 0x9: return "TCS";
            case 0xA: return "STC";
            case 0xB: return "DAA";
            case 0xC: return "KBP";
            case 0xD: return "DCL";
            default : return "???";
        }
    }
    
}

function getInstruction(cpu) {
    let data;
    let opcode = getOpcode(cpu);
    let OPA = cpu.programRom.memory[cpu.PC] & 0x0F;
    let second = cpu.programRom.memory[cpu.PC + 1];
    switch (opcode) {
        case "JCN": 
            data = `0b${OPA.toString(2).toUpperCase().padStart(4, "0")} ${second.toString(16).toUpperCase().padStart(2, "0")}`;
            break;
        case "FIM":
            data = `P${OPA >> 1} 0x${second.toString(16).toUpperCase().padStart(2, "0")}`;
            break;
        case "SRC":
            data = `P${OPA >> 1}`;
            break;
        case "FIN":
            data = `P${OPA >> 1}`;
            break;
        case "JIN":
            data = `P${OPA >> 1}`;
            break;
        case "JUN":
            data = `0x${((OPA << 8) | second).toString(16).toUpperCase().padStart(3, "0")}`;
            break;
        case "JMS":
            data = `0x${((OPA << 8) | second).toString(16).toUpperCase().padStart(3, "0")}`;
            break;
        case "INC":
            data = `R${OPA}`;
            break;
        case "ISZ":
            data = `R${OPA} 0x${second.toString(16).toUpperCase().padStart(2, "0")}`;
            break;
        case "ADD":
            data = `R${OPA}`;
            break;
        case "SUB":
            data = `R${OPA}`;
            break;
        case "LD ":
            data = `R${OPA}`;
            break;
        case "XCH":
            data = `R${OPA}`;
            break;
        case "BBL":
            data = `0x${OPA.toString(16).toUpperCase()}`;
            break;
        case "LDM":
            data = `0x${OPA.toString(16).toUpperCase()}`;
            break;
        default:
            data = "";
    }
    return `${opcode}&nbsp;${data}`;
}

export default {CPU, RAM, updateGUI, updateRomView, updateRamView};