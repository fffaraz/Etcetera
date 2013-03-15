var mouseX = 0;
var mouseY = 0;
var canvas = null;
var canvasContext = null;
var spritesPerElement = 3;
var elements = [];
var emitSpriteInterval = null;
var emitCount = 0;
var bugCount = 0;
var bugs = [];
var spriteSizeFactor = 0.65;
var vignette = null;
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
    var trailDistance = Math.random() * 50 + 70;
    var trailAngle = Math.random() * 360;
    var rotate = Math.random() * 180;
    var elementX = Math.round(Math.cos(trailAngle) * trailDistance);
    var elementY = Math.round(Math.sin(trailAngle) * trailDistance);
    var sprite = flowers_elements[elements[elementIndex]].sprites[spriteIndex];
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

function drawBug()
{
    var bugIndex = bugCount % bugs.length;
    bugCount++;
    var bug = bugs[bugIndex];
    canvasContext.save();
    canvasContext.translate(bug.bounds.width / 2 + Math.random() * canvas.width, bug.bounds.height / 2 + Math.random() * canvas.height);
    canvasContext.rotate(Math.random() * 360 * Math.PI/180);
    canvasContext.translate(-bug.bounds.width / 2, -bug.bounds.height / 2);
    bugs_draw(canvasContext, bug.id);
    canvasContext.restore();
}

function initializeScene()
{
    canvas.height = window.innerHeight;
    canvas.width = window.innerWidth;
    canvasContext.fillStyle = 'rgb(0, 256, 0)';
    canvasContext.fillRect(0, 0, canvas.width, canvas.height);
    var grassElement = grass_elements['grass'];
    var verticalOverlap = 0.7;
    var columns = Math.ceil(canvas.width / (grassElement.bounds.width * 0.8)) + 2;
    var rows = Math.ceil(canvas.height / (grassElement.bounds.height * (1 - verticalOverlap))) + 3;
    for (var row = 0; row < rows; row++) {
        var rotation = (-20 / rows) * row;
        for (var column = 0; column < columns; column++) {
            var x = (column - 1) * grassElement.bounds.width * 0.7;
            x += grassElement.bounds.width * (1.3 - Math.random() * 0.3);
            var y = (row - 1) * grassElement.bounds.height * (1 - verticalOverlap);
            y += grassElement.bounds.height * (1 - verticalOverlap) * (1.3 - Math.random() * 0.3);
            canvasContext.save();
            canvasContext.translate(x, y);
            canvasContext.rotate((rotation + 5 - Math.random() * 10) * Math.PI / 180);
            canvasContext.translate(-grassElement.bounds.width / 2, -grassElement.bounds.height);
            grass_draw(canvasContext, grassElement.id);
            canvasContext.restore();
        }
    }
    vignette.width = canvas.width;
    vignette.height = canvas.height;
    var vignetteContext = vignette.getContext('2d');
    var gradient = vignetteContext.createRadialGradient(vignette.width / 2, vignette.height / 2, 0,
                                                        vignette.width / 2, vignette.height / 2, vignette.width / 1.7);
    gradient.addColorStop(0, 'rgba(0, 0, 0, 0)');
    gradient.addColorStop(0.3, 'rgba(0, 0, 0, 0)');
    gradient.addColorStop(0.6, 'rgba(0, 0, 0, 0.2)');
    gradient.addColorStop(1, 'rgba(0, 0, 0, 0.5)');
    vignetteContext.fillStyle = gradient;
    vignetteContext.beginPath();
    vignetteContext.rect(0, 0, vignette.width, vignette.height);
    vignetteContext.fill();
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
    flowers_draw(canvasContext, this.element.id);
    canvasContext.restore();

    var spriteStyle = this.style;
    spriteStyle[accelerated.transform] = '';
    spriteStyle[accelerated.transitionDuration] = '';
    spriteStyle.opacity = 0;
    this.data = null;
}

function createSprites()
{
    for (var i in flowers_elements) {
        var element = flowers_elements[i];
        elements.push(element.id);
        var sprites = [];
        for (var j = 0; j < spritesPerElement; ++j) {
            var sprite = null;
            if (j == 0) {
                sprite = document.createElement('canvas');
                sprite.width = Math.ceil(element.bounds.width * spriteSizeFactor);
                sprite.height = Math.ceil(element.bounds.height * spriteSizeFactor);
                var context = sprite.getContext('2d');
                context.scale(spriteSizeFactor, spriteSizeFactor);
                flowers_draw(context, element.id);
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

function initFlowerPower(parent)
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

    vignette = document.createElement('canvas');
    parent.appendChild(vignette);
    var vignetteStyle = vignette.style;
    vignetteStyle.position = 'absolute';
    vignetteStyle.left = canvas.offsetLeft + 'px';
    vignetteStyle.top = canvas.offsetTop + 'px';
    vignetteStyle.zIndex = 9999999;

    initializeScene();

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

    for (var i in bugs_elements)
        bugs.push(bugs_elements[i]);
    setInterval(drawBug, 3000);
}
