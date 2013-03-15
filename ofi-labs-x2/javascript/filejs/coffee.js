// Drive the CoffeeScript compiler to convert CoffeeScript to JavaScript

// Important: full path to the compiler
var compiler = '/home/yourusername/bin/coffee-script.js';

if (system.args.length !== 2) {
    system.print("Usage: coffee.js inputfile");
    system.exit(-1);
}

if (!fs.exists(compiler)) {
    system.print("CoffeeScript compiler " + compiler + " is not available!");
    system.exit(-1);
}

var loadFile = function(fname)
{
    var content = '',
        f, s;

    f = fs.open(fname, 'r');
    while (true) {
        s = f.readLine();
        if (s.length === 0) {
            break;
        }
        content += s;
    }
    f.close();

    return content;
};

eval(loadFile(compiler));
if (typeof this.CoffeeScript !== 'object' || typeof this.CoffeeScript.compile !== 'function') {
    system.print("Something is wrong with the CoffeeScript compiler (" + compiler + ").");
    system.print("Hint: make sure you reformat it, e.g. using jsbeautifier.org");
    system.exit(-1);
}

var input = loadFile(system.args[1]);
if (input.length === 0) {
    system.print('Nothing to convert!');
    system.exit(1);
}

system.print(this.CoffeeScript.compile(input));
