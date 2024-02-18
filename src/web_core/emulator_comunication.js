// Using wasm32

var memory = new WebAssembly.Memory({
    initial: 256,  // initial size in pages, (1 page = 64 KiB)
    maximum: 512   // maximum size in pages
})
var exports;
WebAssembly.instantiateStreaming(fetch("main.wasm"), {
    js : {
        mem : memory
    },
    env : {
        emscripten_resize_heap : function (delta) {
                memory.grow(delta);
            }
    }
}).then(results => {
    exports = results.instance.exports;
    memory = results.instance.exports.memory;
});

function encodeArray(arr, len, sizeof=1) {
    var ptr;
    var out;
    if (sizeof == 8) {
        ptr = exports.wasmMalloc(8 * len);
        out = new BigUint64Array(memory.buffer, ptr);
    }
    else if (sizeof == 4) {
        ptr = exports.wasmMalloc(len * 4);
        out = new Uint32Array(memory.buffer, ptr);
    }
    else {
        ptr = exports.wasmMalloc(len);
        out = new Uint8Array(memory.buffer, ptr);                
    }
    for (var i = 0; i < len; i++) {
        out[i] = arr[i];
    }
    return ptr;
}


function decodeArray(ptr, len)
{
    return new Uint8Array(memory.buffer).slice(ptr, ptr + len);
}


function decodeString(ptr, len) {
    return new TextDecoder("utf8").decode(decodeArray(ptr, len));
}


function decodeString(ptr) {
    var bytes = new Uint8Array(memory.buffer, ptr);
    var strlen = 0;
    while (bytes[strlen] != 0) {
        strlen++;
    }
    return new TextDecoder("utf8").decode(
        bytes.slice(0, strlen)
    );
}

function decodeCPU(ptr) {
    var bytes = new Uint8Array(memory.buffer, ptr).slice(0, 57);
    var cpu = {
        C: bytes[0] & 0b0001,
        TEST: (bytes[0] & 0b0000_0100) >> 2,
        ACC: (bytes[0] & 0b0111_1000) >> 3,
        OPA: ((bytes[0] & 0b1000_0000) >> 7) | ((bytes[1] & 0b0000_0111) << 1),
        OPR: (bytes[1] & 0b0111_1000) >> 3,
        PC: ((bytes[1] & 0b1000_0000) >> 7) | (bytes[2] << 1) | ((bytes[3] & 0b0000_0111) << 9),
        STACK: ((bytes[3] & 0b1111_1000) >> 3) | (bytes[4] << 5) | (bytes[5] << 13) | (bytes[6] << 21) | ((bytes[7] & 0b0111_1111) << 29),
        registerPairs: [bytes[12], bytes[13], bytes[14], bytes[15], bytes[16], bytes[17], bytes[18], bytes[19]],
        pProgramRom: bytes[20] | (bytes[21] << 8) | (bytes[22] << 16) | (bytes[23] << 24),
        pRamBanks: [
            bytes[24] | (bytes[25] << 8) | (bytes[26] << 16) | (bytes[27] << 24),
            bytes[28] | (bytes[29] << 8) | (bytes[30] << 16) | (bytes[31] << 24),
            bytes[32] | (bytes[33] << 8) | (bytes[34] << 16) | (bytes[35] << 24),
            bytes[36] | (bytes[37] << 8) | (bytes[38] << 16) | (bytes[39] << 24),
            bytes[40] | (bytes[41] << 8) | (bytes[42] << 16) | (bytes[43] << 24),
            bytes[44] | (bytes[45] << 8) | (bytes[46] << 16) | (bytes[47] << 24),
            bytes[48] | (bytes[49] << 8) | (bytes[50] << 16) | (bytes[51] << 24),
            bytes[52] | (bytes[53] << 8) | (bytes[54] << 16) | (bytes[55] << 24)
        ],
        lastInstruction: bytes[56]
    }
    return cpu;
}

function decodeRAM(ptr) {
    var bytes = new Uint8Array(memory.buffer, ptr);
    ram = {
        memory: bytes.slice(0, 128),
        status: bytes.slice(128, 160),
        outputs: [bytes[160], bytes[161]]
    }
    return ram;

}


function initROM() {
    var arr = [
        0x40, 0x49, 0x73, 0x05, 0x62, 0xC0, 0x85, 0xb5, 0x1a, 0x0b, 0x64, 0xc0, 0xb5, 0x95, 0xb5, 0x1a,
        0x15, 0xd1, 0xb4, 0x94, 0xb4, 0xc0, 0xa0, 0x23, 0xe0, 0x50, 0x02, 0xa1, 0x23, 0xe0, 0x50, 0x02,
        0xc0, 0x26, 0x00, 0x25, 0xe9, 0xb7, 0xd2, 0x50, 0x06, 0x25, 0xe9, 0x87, 0xb1, 0x26, 0x00, 0x1a,
        0x33, 0x26, 0x01, 0xd3, 0x50, 0x0c, 0x25, 0xe9, 0xb6, 0xd2, 0x50, 0x06, 0x25, 0xe9, 0x86, 0x12,
        0x5d, 0x87, 0x12, 0x5d, 0xb0, 0xd1, 0x50, 0x06, 0xc0, 0x20, 0x00, 0x22, 0x00, 0x24, 0x01, 0x26,
        0x00, 0x50, 0x16, 0x20, 0x01, 0x50, 0x16, 0x50, 0x21, 0x50, 0x16, 0x40, 0x57, 0x40, 0x5d];
    var ptr = encodeArray(arr, arr.length, 1);
    exports.initROM(ptr, arr.length);
    exports.wasmFree(ptr);
}

function stepCPU(steps) {
    var ptr = exports.stepCPU(steps);

    var result = decodeString(ptr);
    console.log(result);
    if (result.length <= 3) {
        result += "&nbsp;[&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;]"
    }

    const opcode = document.getElementById("opcode");
    opcode.innerHTML = result;
        
}

function resetCPU() {
    initROM();
    exports.resetCPU();
}

function initEmulator() {
    exports.mainInit();
}


var sleepSetTimeout_ctrl;

function sleep(ms) {
    clearInterval(sleepSetTimeout_ctrl);
    return new Promise(resolve => sleepSetTimeout_ctrl = setTimeout(resolve, ms));
}

// sleep(1000).then(() => {
//     initEmulator();
//     console.log("Emulator initialized");
//     initROM();
//     console.log("ROM initialized");
//     resetCPU();
//     console.log("CPU reset");
// });

function getOpcode(instruction) {
    const OPR = (instruction >> 4) & 0x0F;
    const OPA = instruction & 0x0F;
    if (OPR <= 0xD) {
        switch (OPR) {
            case 0x0: return "NOP";
            case 0x1: return "JCN";
            case 0x2: 
                if (OPA % 2 === 0) return "FIM";
                else return "SRC";
            case 0x3: 
                if (OPA % 2 === 0) return "FIN";
                else return "JIN";
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
    if (OPR === 0xE) {
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
    
}