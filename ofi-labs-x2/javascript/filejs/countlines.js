// Count non-empty lines in a file

if (system.args.length !== 2) {
    system.print("Usage: countlines.js inputfile");
    system.exit(-1);
}

try {
    var input = fs.open(system.args[1], 'r');
    var count = 0;
    while (true) {
        var line = input.readLine();
        if (line.length === 0)
            break;
        if (line.trim().length)
            count++;
    }
    system.print(count);
}
catch (err) {
    system.print("FAIL:", err);
    system.exit(-1);
}
finally {
    input.close();
}

