// resetCPU();


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


function initROM() {
    var arr = [
        0x40, 0x49, 0x73, 0x05, 0x62, 0xC0, 0x85, 0xb5, 0x1a, 0x0b, 0x64, 0xc0, 0xb5, 0x95, 0xb5, 0x1a,
        0x15, 0xd1, 0xb4, 0x94, 0xb4, 0xc0, 0xa0, 0x23, 0xe0, 0x50, 0x02, 0xa1, 0x23, 0xe0, 0x50, 0x02,
        0xc0, 0x26, 0x00, 0x25, 0xe9, 0xb7, 0xd2, 0x50, 0x06, 0x25, 0xe9, 0x87, 0xb1, 0x26, 0x00, 0x1a,
        0x33, 0x26, 0x01, 0xd3, 0x50, 0x0c, 0x25, 0xe9, 0xb6, 0xd2, 0x50, 0x06, 0x25, 0xe9, 0x86, 0x12,
        0x5d, 0x87, 0x12, 0x5d, 0xb0, 0xd1, 0x50, 0x06, 0xc0, 0x20, 0x00, 0x22, 0x00, 0x24, 0x01, 0x26,
        0x00, 0x50, 0x16, 0x20, 0x01, 0x50, 0x16, 0x50, 0x21, 0x50, 0x16, 0x40, 0x57, 0x40, 0x5d];
    var ptr = encodeArray(arr, arr.length, 1);
    var result = exports.initROM(ptr, arr.length);
    // exports.wasmFree(ptr);

    // document.querySelector("#ret")
    //     .innerHTML += `${result}<br>`;
}

function stepCPU(steps) {
    var ptr = exports.stepCPU(steps);

    var result = decodeString(ptr);
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