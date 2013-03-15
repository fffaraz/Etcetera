// Encrypt a text file using ROT-13 substitution cipher

// http://www.google.com/search?q=rot13+javascript+stackoverflow
String.prototype.rot13 = function(s)
 {
    return (s ? s : this).split('').map(function(_)
     {
        if (!_.match(/[A-za-z]/)) return _;
        c = Math.floor(_.charCodeAt(0) / 97);
        k = (_.toLowerCase().charCodeAt(0) - 83) % 26 || 26;
        return String.fromCharCode(k + ((c == 0) ? 64 : 96));
     }).join('');
 }

if (system.args.length !== 3) {
    system.print("Usage: rot13.js inputfile outputfile");
    system.exit(-1);
}

if (system.args[1] === system.args[2]) {
    system.print("Output file can't be the same as input file");
    system.exit(-1);
}

try {
    var input = fs.open(system.args[1], 'r');
    var output = fs.open(system.args[2], 'w');
    try {
        while (true)
            output.writeLine(input.next().rot13());
    }
    catch (e) { } // EOF exception
}
catch (err) {
    system.print("FAIL:", err);
    system.exit(-1);
}
finally {
    input.close();
    output.close();
}

