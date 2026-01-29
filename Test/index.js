
console.log("=========================================");
console.log("   Novadesk Multi-Addon Test Suite       ");
console.log("=========================================");

// 1. Hello World Addon
console.log("\n[1/3] Loading Hello World...");
const hello = system.loadAddon("../dist/x64/Debug/hello_world/hello_world.dll");
if (hello) {
    console.log("  Success! Version: " + hello.version);
    console.log("  Addon says: " + hello.hello());
    
    hello.onEvent(function(msg) {
        console.log("  [Hello Event]: " + msg);
    });
} else {
    console.error("  Failed to load hello_world.dll");
}

// 2. Math Utilities Addon
console.log("\n[2/3] Loading Math Utilities...");
const math = system.loadAddon("../dist/x64/Debug/math_utils/math_utils.dll");
if (math) {
    console.log("  Success!");
    console.log("  Test: 15 + 25 = " + math.sum(15, 25));
    console.log("  Test: 100 - 42 = " + math.subtract(100, 42));
    console.log("  Test: 12 * 12 = " + math.multiply(12, 12));
    
    // Testing error handling
    try {
        console.log("  Testing Error: " + math.sum(10, "not a number"));
    } catch (e) {
        console.log("  Caught expected error: " + e.message);
    }
} else {
    console.error("  Failed to load math_utils.dll");
}

// 3. CPU Monitor Addon
console.log("\n[3/3] Loading CPU Monitor...");
const cpu = system.loadAddon("../dist/x64/Debug/cpu_monitor/cpu_monitor.dll");
if (cpu) {
    console.log("  Success! Initializing real-time tracker...");
    cpu.start(function(usage) {
        console.log("  [CPU]: " + usage.toFixed(2) + "%");
    });
} else {
    console.error("  Failed to load cpu_monitor.dll");
}

console.log("\nAll tests initialized. System ready.");
console.log("Will auto-unload addons in 10 seconds...");
console.log("=========================================");

// 4. Auto-Unload After 10 Seconds
setTimeout(function() {
    console.log("\n[Cleanup] Unloading all addons...");
    
    var addons = [
        "../dist/x64/Debug/hello_world/hello_world.dll",
        "../dist/x64/Debug/math_utils/math_utils.dll",
        "../dist/x64/Debug/cpu_monitor/cpu_monitor.dll"
    ];

    addons.forEach(function(path) {
        if (system.unloadAddon(path)) {
            console.log("  Successfully unloaded: " + path);
        } else {
            console.error("  Failed to unload: " + path);
        }
    });

    console.log("\n[Cleanup] Finished. You can close Novadesk now.");
}, 10000);