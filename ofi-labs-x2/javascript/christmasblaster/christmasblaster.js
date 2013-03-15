var mouseX = 0;
var mouseY = 0;
var canvas = null;
var canvasContext = null;
var spritesPerElement = 3;
var elements = [];
var flakesPerDraw = 0;
var emitSpriteInterval = null;
var emitCount = 0;
var spriteSizeFactor = 0.65; // Save some GPU memory
var flakes = [
    { file: "flake1.png" },
    { file: "flake2.png" },
    { file: "flake3.png" },
    { file: "flake4.png" }
];
var accelerated = {};

function moveEmission(x, y)
{
    mouseX = x;
    mouseY = y;
}

function onMouseMove(event)
{
    moveEmission(event.clientX, event.clientY);
}

function onTouchMove(event)
{
    event.preventDefault();
    moveEmission(event.targetTouches[0].clientX, event.targetTouches[0].clientY);
}

function startEmission(x, y)
{
    mouseX = x;
    mouseY = y;
    emitElementSprite();
}

function onMouseDown(event)
{
    startEmission(event.clientX, event.clientY);
}

function onTouchStart(event)
{
    event.preventDefault();
    if (event.targetTouches.length != 1)
        return false;
    startEmission(event.targetTouches[0].clientX, event.targetTouches[0].clientY);
}

function onMouseUp(event)
{
    window.clearInterval(emitSpriteInterval);
    emitSpriteInterval = null;
}

function emitElementSprite()
{
    var elementIndex = emitCount % elements.length;
    var spriteIndex = Math.floor(emitCount / elements.length) % spritesPerElement;
    emitCount++;
    var trailDistance = Math.random() * 50 + 100;
    var trailAngle = Math.random() * 360;
    var rotate = Math.random() * 180;
    var elementX = Math.round(Math.cos(trailAngle) * trailDistance);
    var elementY = Math.round(Math.sin(trailAngle) * trailDistance);
    var sprite = graphics_elements[elements[elementIndex]].sprites[spriteIndex];
    var spriteStyle = sprite.style;
    var offsetX = Math.round(mouseX - sprite.width / 2);
    var offsetY = Math.round(mouseY - sprite.height / 2);
    spriteStyle.left = offsetX + 'px';
    spriteStyle.top = offsetY + 'px';
    spriteStyle[accelerated.transform] = 'translate(' + elementX + 'px, ' + elementY + 'px) scale(' + (1 / spriteSizeFactor) + ') rotate(' + rotate + 'deg)';
    spriteStyle[accelerated.transitionDuration] = '0.7s';
    spriteStyle.opacity = 1;
    spriteStyle.zIndex = emitCount;
    sprite.data = { elementX: elementX, offsetX: offsetX, elementY: elementY, offsetY: offsetY, rotate: rotate };
    if (emitSpriteInterval == null)
        emitSpriteInterval = window.setInterval(emitElementSprite, 80);
}

function drawFlakes()
{
    for (var i = 0; i < flakesPerDraw; i++) {
        var flakeImage = flakes[Math.floor(Math.random() * flakes.length)].image;
        canvasContext.drawImage(flakeImage,
                                Math.round(Math.random() * (canvas.width + 2 * flakeImage.width) - flakeImage.width),
                                Math.round(Math.random() * (canvas.height + 2 * flakeImage.height) - flakeImage.height));
    }
}

function fitToWindow()
{
    canvas.height = window.innerHeight;
    canvas.width = window.innerWidth;
    canvasContext.fillStyle = "#000";
    flakesPerDraw = Math.ceil(canvas.width * canvas.height / 800000.0);
    canvasContext.fillRect(0, 0, canvas.width, canvas.height);
}

function onWebkitTransitionEnd(event)
{
    if (this.data === null)
        return;

    canvasContext.save();
    canvasContext.translate(this.width / 2 + this.data.elementX + this.data.offsetX,
                            this.height / 2 + this.data.elementY + this.data.offsetY);
    canvasContext.rotate(this.data.rotate * Math.PI/180);
    canvasContext.translate(-this.width / 2 / spriteSizeFactor, -this.height / 2 / spriteSizeFactor);
    graphics_draw(canvasContext, this.element.id);
    canvasContext.restore();

    var spriteStyle = this.style;
    spriteStyle[accelerated.transform] = '';
    spriteStyle[accelerated.transitionDuration] = '';
    spriteStyle.opacity = 0;
    this.data = null;
}

function createSprites()
{
    for (var i in graphics_elements) {
        var element = graphics_elements[i];
        elements.push(element.id);
        var sprites = [];
        for (var j = 0; j < spritesPerElement; ++j) {
            var sprite = null;
            if (j == 0) {
                sprite = document.createElement("canvas");
                sprite.width = Math.ceil(element.bounds.width * spriteSizeFactor);
                sprite.height = Math.ceil(element.bounds.height * spriteSizeFactor);
                var context = sprite.getContext('2d');
                context.scale(spriteSizeFactor, spriteSizeFactor);
                graphics_draw(context, element.id);
            } else {
                sprite = sprites[0].cloneNode(false);
                var context = sprite.getContext('2d');
                context.drawImage(sprites[0], 0, 0);
            }
            sprite.element = element;
            var spriteStyle = sprite.style;
            spriteStyle.position = 'absolute';
            spriteStyle.opacity = 0;
            spriteStyle[accelerated.transitionProperty] = accelerated.transformCssAttribute + ', opacity';
            document.body.appendChild(sprite);
            sprite.addEventListener(accelerated.transitionEnd, onWebkitTransitionEnd, false);
            sprites.push(sprite);
        }
        element.sprites = sprites;
    }
}

function initChristmasBlaster(parent)
{
    var testStyle = parent.style;
    if (typeof(testStyle.webkitTransform) !== 'undefined') {
        accelerated = {
            transform: 'webkitTransform',
            transitionDuration: 'webkitTransitionDuration',
            transitionProperty: 'webkitTransitionProperty',
            transitionEnd: 'webkitTransitionEnd',
            transformCssAttribute: '-webkit-transform'
        };
    } else if (typeof(testStyle.MozTransform) !== 'undefined') {
        accelerated = {
            transform: 'MozTransform',
            transitionDuration: 'MozTransitionDuration',
            transitionProperty: 'MozTransitionProperty',
            transitionEnd: 'transitionend',
            transformCssAttribute: '-Moz-transform'
        };
    } else if (typeof(testStyle.OTransform) !== 'undefined') {
        accelerated = {
            transform: 'OTransform',
            transitionDuration: 'OTransitionDuration',
            transitionProperty: 'OTransitionProperty',
            transitionEnd: 'OTransitionEnd',
            transformCssAttribute: '-O-transform'
        };
    }

    canvas = document.createElement('canvas');
    parent.appendChild(canvas);
    createSprites();
    canvasContext = canvas.getContext('2d');
    fitToWindow();

    if ('ontouchstart' in document.documentElement) {
        document.addEventListener('touchmove', onTouchMove, false);
        document.addEventListener('touchstart', onTouchStart, false);
        document.addEventListener('touchend', onMouseUp, false);
        document.addEventListener('gesturechange', function(e){e.preventDefault();}, false);
    } else {
        document.body.addEventListener('mousemove', onMouseMove, false);
        document.body.addEventListener('mouseup', onMouseUp, false);
        document.body.addEventListener('mousedown', onMouseDown, false);
    }

    for (var i in flakes) {
        flakes[i].image = new Image();
        flakes[i].image.src = flakes[i].file;
    }

    setInterval(drawFlakes, 20);
}

