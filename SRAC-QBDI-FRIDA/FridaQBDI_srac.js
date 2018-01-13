// QBDI
const qbdi = require('/usr/local/share/qbdi/frida-qbdi'); // import QBDI bindings
qbdi.import(); // Set bindings to global environment

// Initialize QBDI
var vm = new QBDI();
var state = vm.getGPRState();
var stack = vm.allocateVirtualStack(state, 0x100000);

var funcPtr = Module.findExportByName(null, "prog_main");
if (!funcPtr) {
    funcPtr = DebugSymbol.fromName("prog_main");
}
vm.addInstrumentedModuleFromAddr(funcPtr);


// Callback on call instructions
var cb_call = vm.newInstCallback(function(vm, gpr, fpr, retStack) {
    inst = vm.getInstAnalysis();

    // Calculate the fall instr addr right after the call instr. This is where the return jumps back to
    var fallInstrAddr = inst.address + inst.instSize;

    // Push the calculate addr to the shadow stack
    retStack.push(fallInstrAddr);

    console.log("CALL Detected! Addr: 0x" + inst.address.toString(16) + " Fall Addr: 0x" + fallInstrAddr.toString(16) + " " + retStack.toString());
    return VMAction.CONTINUE;
});

// Callback on ret instructions
var cb_ret = vm.newInstCallback(function(vm, gpr, fpr, retStack) {
    inst = vm.getInstAnalysis();

    if (retStack.length == 0){
        return VMAction.CONTINUE;
    }

    var rspVal = gpr.getRegister("RSP");
    var savedIp = Memory.readPointer(rspVal);

    console.log("Ret Detected! IP on Stack: " + savedIp.toString());

    while (retStack.length > 0) {
        if (savedIp == retStack.pop()){
            return VMAction.CONTINUE;
        }
    }

    console.log("Saved IP is not valid. Exiting program");
    return VMAction.STOP;
});

var retStack = [];

vm.addMnemonicCB("CALL64pcrel32", InstPosition.POSTINST, cb_call, retStack);
vm.addMnemonicCB("RETQ", InstPosition.PREINST, cb_ret, retStack);


// Allocate a string in remote process memory
var arg = Memory.allocUtf8String("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");

// Call the Secret function using QBDI and with our string as argument
vm.call(funcPtr, [2, arg]);
