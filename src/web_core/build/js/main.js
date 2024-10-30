// Using wasm32

import ec from "./emulator_communication.js" 

const CPU = ec.CPU;
const RAM = ec.RAM;

const hex = ["0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B", "C", "D", "E", "F"];

function topRowRegPairs() {
    let topRow = document.createElement("tr");
    topRow.className = "leading-3";
    for (let i = -1; i < 2; i++) {
        let cell = document.createElement("td");
        cell.className = "font-bold";
        if (i == -1)
            cell.innerHTML = "&nbsp;";
        else
            cell.innerHTML = i;
        topRow.appendChild(cell);
    }
    return topRow;
}


function adjustNInput() {
    let step_count = document.getElementById("step_count");
    let steps = parseInt(step_count.value);
    if (steps < step_count.min) step_count.value = 1;
    else if (steps > step_count.max) step_count.value = step_count.max;
    else if (isNaN(steps)) step_count.value = 1;
}




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

function decodeArray(ptr, len) {
    return new Uint8Array(memory.buffer).slice(ptr, ptr + len);
}

function initROM(arr) {
    let ptr = encodeArray(arr, arr.length, 1);
    exports.initROM(ptr, arr.length);
    exports.wasmFree(ptr);
}


var sleepSetTimeout_ctrl;
function sleep(ms) {
    clearInterval(sleepSetTimeout_ctrl);
    return new Promise(resolve => sleepSetTimeout_ctrl = setTimeout(resolve, ms));
}

var run = false;
function runCPU(cpu, steps) {
    if (((cpu.programRom.memory[cpu.PC] << 8) | cpu.programRom.memory[cpu.PC+1]) == (0x4000 | cpu.PC)) run = false;
    if (!run) return;
    cpu.stepCPU(steps > 40 ? steps / 40 : 1);
    sleep(steps > 40 ? 25 : 1000 / steps).then(() => {
        runCPU(cpu, steps);
    });
}


export function initEmulator(arr = []) {
    initROM(arr);
    run = false;
    let pCPU = exports.resetCPU();
    let cpu = new CPU(pCPU, exports, memory.buffer);
    // cpu.stepCPU(1);
    ec.updateGUI(cpu);
    document.getElementById("step_button").onclick = function() {
        cpu.stepCPU(1);
    }
    document.getElementById("step_button_n").onclick = function() {
        adjustNInput();
        cpu.stepCPU(parseInt(document.getElementById("step_count").value));
    }
    document.getElementById("run_button").onclick = function() {
        run = true;
        adjustNInput();
        runCPU(cpu, document.getElementById("step_count").value);
    }
    document.getElementById("stop_button").onclick = function() {
        run = false;
    }
    document.getElementById("reset_button").onclick = function() {
        initEmulator();
    }
    document.getElementById("rom_page_select").onchange = function() {
        ec.updateRomView(cpu);
    }
    document.getElementById("ram_bank_select").onchange = function() {
        ec.updateRamView(cpu);
    }
    document.getElementById("ram_chip_select").onchange = function() {
        ec.updateRamView(cpu);
    }
}

window.initEmulator = initEmulator;

var memory;
var exports;

function initWebAssembly() {
    memory = new WebAssembly.Memory({
        initial: 64,  // initial size in pages, (1 page = 64 KiB)
        maximum: 512   // maximum size in pages
    })
    return WebAssembly.instantiateStreaming(fetch("wasm/emulator.wasm"), {
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
}

initWebAssembly().then(() => initEmulator());


let reg_pairs = document.getElementById("reg_pair_view");
let table = document.createElement("table");
// table.appendChild(topRowRegPairs());
for (let i = 0; i < 4; i++) {
    let row = document.createElement("tr");
    row.className = "leading-3";
    for (let j = -1; j < 2; j++) {
        let col = document.createElement("td");
        if (j == -1) {
            col.className = "font-bold";
            col.innerHTML = `R${2*i}R${2*i+1}&nbsp;`;
        }
        else {
            col.id = `r${2*i+j}`;
        }
        row.appendChild(col);
    }
    table.appendChild(row);
}
reg_pairs.appendChild(table);

table = document.createElement("table");
// table.appendChild(topRowRegPairs());
for (let i = 4; i < 8; i++) {
    let row = document.createElement("tr");
    row.className = "leading-3";
    for (let j = -1; j < 3; j++) {
        let col = document.createElement("td");
        if (j == -1) {
            col.className = "font-bold";
            if (i == 4) col.innerHTML = `&nbsp;R${2*i} R${2*i+1}&nbsp;`;
            else col.innerHTML = `&nbsp;R${2*i}R${2*i+1}&nbsp;`;
        }
        else if (j == 2) {
            col.innerHTML = "&nbsp;";
        }
        else {
            col.id = `r${2*i+j}`;
        }
        row.appendChild(col);
    }
    table.appendChild(row);
}
reg_pairs.appendChild(table);

adjustNInput();

let rom_table = document.getElementById("rom_view");

var topRow = document.createElement("tr");
topRow.className = "leading-4";

var topCell = document.createElement("td");
topCell.className = "address";
topCell.innerHTML = "&nbsp;&nbsp;";
topRow.appendChild(topCell);

for (var i = 0; i < 16; i++) {
    var cell = document.createElement("td");
    cell.className = "address";
    cell.innerHTML = "0" + hex[i];
    topRow.appendChild(cell);
}
rom_table.appendChild(topRow);

for (var i = 0; i < 16; i++) {
    var row = document.createElement("tr");
    row.className = "leading-4";

    var cell = document.createElement("td");
    cell.className = "address";
    cell.innerHTML = hex[i] + "0";
    row.appendChild(cell);
    for (var j = 0; j < 16; j++) {
        var cell = document.createElement("td");
        cell.id = "rom" + hex[i] + hex[j];
        cell.className = "byte";
        row.appendChild(cell);
    }
    rom_table.appendChild(row);
}

var rom_page_select = document.getElementById("rom_page_select");
for (var i = 0; i < 16; i++) {
    var option = document.createElement("option");
    if (i == 0) option.selected = true;
    option.value = i;
    option.innerHTML = `Page ${i.toString().toUpperCase()}`;
    rom_page_select.appendChild(option);
}



var ram_table = document.getElementById("ram_view");
var topRow = document.createElement("tr");
topRow.className = "leading-4";

var topCell = document.createElement("td");
topCell.className = "address";
topCell.innerHTML = "&nbsp;";
topRow.appendChild(topCell);

for (var i = 0; i < 16; i++) {
    var cell = document.createElement("td");
    cell.className = "address";
    cell.innerHTML = hex[i];
    topRow.appendChild(cell);
}
ram_table.appendChild(topRow);

for (var i = 0; i < 4; i++) {
    var row = document.createElement("tr");
    row.className = "leading-4";

    var cell = document.createElement("td");
    cell.className = "address";
    cell.innerHTML = hex[i];
    row.appendChild(cell);
    for (var j = 0; j < 16; j++) {
        var cell = document.createElement("td");
        cell.id = "ram" + hex[i] + hex[j];
        cell.className = "byte";
        row.appendChild(cell);
    }
    ram_table.appendChild(row);
}



let status_table = document.getElementById("status_view");
topRow = document.createElement("tr");
topRow.className = "leading-4";

topCell = document.createElement("td");
topCell.className = "address";
topCell.innerHTML = "&nbsp;";
topRow.appendChild(topCell);

for (let i = 0; i < 4; i++) {
    let cell = document.createElement("td");
    cell.className = "address";
    cell.innerHTML = i;
    topRow.appendChild(cell);
}
status_table.appendChild(topRow);

for (let i = 0; i < 4; i++) {
    let row = document.createElement("tr");
    row.className = "leading-4";

    let cell = document.createElement("td");
    cell.className = "address";
    cell.innerHTML = i;
    row.appendChild(cell);
    for (let j = 0; j < 4; j++) {
        let cell = document.createElement("td");
        cell.id = "status" + i + j;
        cell.className = "byte";
        row.appendChild(cell);
    }
    status_table.appendChild(row);
}


let ram_bank_select = document.getElementById("ram_bank_select");
for (let i = 0; i < 8; i++) {
    let option = document.createElement("option");
    if (i == 0) option.selected = true;
    option.value = i;
    option.innerHTML = `Bank ${i}`;
    ram_bank_select.appendChild(option);
}

let ram_chip_select = document.getElementById("ram_chip_select");
for (let i = 0; i < 4; i++) {
    let option = document.createElement("option");
    if (i == 0) option.selected = true;
    option.value = i;
    option.innerHTML = `Chip ${i}`;
    ram_chip_select.appendChild(option);
}

// sleep(500).then(() => {
//     initEmulator();
// });