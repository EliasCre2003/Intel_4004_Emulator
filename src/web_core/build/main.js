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

function decodeArray(ptr, len)
{
    return new Uint8Array(memory.buffer).slice(ptr, ptr + len);
}

function initROM() {
    // let arr = [
    //     0x40, 0x49, 0x73, 0x05, 0x62, 0xC0, 0x85, 0xb5, 0x1a, 0x0b, 0x64, 0xc0, 0xb5, 0x95, 0xb5, 0x1a,
    //     0x15, 0xd1, 0xb4, 0x94, 0xb4, 0xc0, 0xa0, 0x23, 0xe0, 0x50, 0x02, 0xa1, 0x23, 0xe0, 0x50, 0x02,
    //     0xc0, 0x26, 0x00, 0x25, 0xe9, 0xb7, 0xd2, 0x50, 0x06, 0x25, 0xe9, 0x87, 0xb1, 0x26, 0x00, 0x1a,
    //     0x33, 0x26, 0x01, 0xd3, 0x50, 0x0c, 0x25, 0xe9, 0xb6, 0xd2, 0x50, 0x06, 0x25, 0xe9, 0x86, 0x12,
    //     0x5d, 0x87, 0x12, 0x5d, 0xb0, 0xd1, 0x50, 0x06, 0xc0, 0x20, 0x00, 0x22, 0x00, 0x24, 0x01, 0x26,
    //     0x00, 0x50, 0x16, 0x20, 0x01, 0x50, 0x16, 0x50, 0x21, 0x50, 0x16, 0x40, 0x57, 0x40, 0x5d       ];

    let arr = [64, 55, 117, 17, 100, 164, 248, 159, 18, 17, 105, 169, 152, 26, 71, 169, 253, 192, 162, 37, 224, 80, 2, 163, 37, 224, 80, 2, 192, 113, 32, 96, 192, 240, 179, 128, 26, 39, 98, 179, 192, 34, 0, 38, 0, 161, 151, 20, 54, 80, 33, 103, 64, 45, 192, 32, 0, 36, 0, 40, 32, 46, 15, 80, 41, 80, 29, 80, 18, 20, 63, 64, 71]
    let ptr = encodeArray(arr, arr.length, 1);
    exports.initROM(ptr, arr.length);
    exports.wasmFree(ptr);
}


var sleepSetTimeout_ctrl;
function sleep(ms) {
    clearInterval(sleepSetTimeout_ctrl);
    return new Promise(resolve => sleepSetTimeout_ctrl = setTimeout(resolve, ms));
}

var run = false
function runCPU(cpu, steps) {
    if (!run) return;
    if (steps > 40) {
        cpu.stepCPU(steps / 40);
        sleep(25).then(() => {
            runCPU(cpu, steps);
        });
    } else {
        cpu.stepCPU(1);
        sleep(1000 / steps).then(() => {
            runCPU(cpu, steps);
        });
    }
}


function initEmulator() {
    initROM();
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
        cpu.stepCPU(document.getElementById("step_count").value);
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



var memory = new WebAssembly.Memory({
    initial: 64,  // initial size in pages, (1 page = 64 KiB)
    maximum: 512   // maximum size in pages
})
var exports;
WebAssembly.instantiateStreaming(fetch("emulator.wasm"), {
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



var reg_pairs = document.getElementById("reg_pair_view");

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

var rom_table = document.getElementById("rom_view");

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
topCell.innerHTML = "&nbsp;&nbsp;";
topRow.appendChild(topCell);

for (let i = 0; i < 4; i++) {
    let cell = document.createElement("td");
    cell.className = "address";
    cell.innerHTML = "0" + i;
    topRow.appendChild(cell);
}
status_table.appendChild(topRow);

for (let i = 0; i < 4; i++) {
    let row = document.createElement("tr");
    row.className = "leading-4";

    let cell = document.createElement("td");
    cell.className = "address";
    cell.innerHTML = i + "0";
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


sleep(500).then(() => {
    initEmulator();
});