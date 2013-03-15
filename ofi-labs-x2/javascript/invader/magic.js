function Image()
{
    var width;
    var height;
    var src;
    var onload;

    this.__defineSetter__("onload", function(callback){
        onload = callback;
    });

    this.__defineGetter__("src", function(){
        return src;
    });

    this.__defineSetter__("src", function(url){
        src = url;
        var dim = invader.preloadImage(url);
        this.width = dim[0];
        this.height = dim[1];
        setTimeout(onload, 400);
    });
}

function ImageData(w, h, data)
{
    this.width = w;
    this.height = h;
    this.data = data;
}

var context = {

    width: invader.width,
    height: invader.height,

    globalAlpha: 1.0,
    strokeStyle: "#000000",
    fillStyle: "#000000",

    beginPath: function () {
        invader.beginPath();
    },

    arc: function(x, y, rad, sa, ea, ac) {
        invader.arc(x, y, rad);
    },

    stroke: function () {
        invader.stroke(this.strokeStyle, this.globalAlpha);
    },

    fill: function () {
        invader.fill(this.fillStyle, this.globalAlpha);
    },

    fillRect: function (x, y, w, h) {
        invader.fillRect(x, y, w, h, this.fillStyle, this.globalAlpha);
    },

    clearRect: function (x, y, w, h) {
        invader.fillRect(x, y, w, h, "#000", 1);
    },

    drawImage: function (image, x, y) {
        invader.drawImage(image.src, x, y);
    },

    getImageData: function (x, y, w, h) {
        return new ImageData(w, h, invader.getPixels(x, y, w, h));
    },

    putImageData: function (d, x, y) {
        invader.putPixels(x, y, d.width, d.height, d.data)
    },

    state: [],

    save: function () {
        this.state.push(this.globalAlpha);
        this.state.push(this.strokeStyle);
        this.state.push(this.fillStyle);
        this.globalAlpha = 1.0;
        this.strokeStyle = "#000000";
        this.fillStyle = "#000000";
    },

    restore: function () {
        this.fillStyle = this.state.pop();
        this.strokeStyle = this.state.pop();
        this.globalAlpha = this.state.pop();
    }

};

var canvas = {
    width: invader.width,
    height: invader.height,

    getContext: function (id) {
        if (id === "2d") {
            return context;
        }
    },
};

function Element() {
    innerHTML: []
}

function ScoreElement() {
    this.__defineGetter__("innerHTML", function(){
        return 'Invader: ';
    }),
    this.__defineSetter__("innerHTML", function(text){
        invader.windowTitle = text;
    });
}

function GameOverElement() {
    this.__defineGetter__("innerHTML", function(){
        return 'Invader: ';
    }),
    this.__defineSetter__("innerHTML", function(text){
        invader.windowTitle = text;
    });
}

var score = new ScoreElement;
var gameOver = new GameOverElement;

var document = {
    getElementById: function (id) {
        if (id === "V")
            return canvas;
        if (id === "S")
            return score;
        if (id === "O")
            return gameOver;
    },
};

var window = {
    onkeyup: [],
    onkeydown: []
}
