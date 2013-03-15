/*
  This file is part of the Ofi Labs X2 project.

  Copyright (C) 2010 Ariya Hidayat <ariya.hidayat@gmail.com>

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
  ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
  THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

function Box(width, height) {
    this.width = width;
    this.height = height;
    this.dim = 15;
    this.marbles = [];

    var worldAABB = new b2AABB();
    worldAABB.minVertex.Set(-width * 4, -height * 4);
    worldAABB.maxVertex.Set(width * 4, height * 4);
    this.world = new b2World(worldAABB, new b2Vec2(0, 1 * 1000), true);

    this.resize(width, height);
}

Box.prototype.resize = function (width, height) {
    function createBox(world, x, y, w, h) {
        var boxSd = new b2BoxDef(),
            boxBd = new b2BodyDef();
        boxSd.extents.Set(w, h);
        boxBd.AddShape(boxSd);
        boxBd.position.Set(x, y);
        return world.CreateBody(boxBd);
    }

    if (typeof this.topWall !== 'undefined') {
        this.world.DestroyBody(this.topWall);
        this.world.DestroyBody(this.bottomWall);
        this.world.DestroyBody(this.leftWall);
        this.world.DestroyBody(this.rightWall);
    }

    var slab = 100;
    this.width = width;
    this.height = height;
    this.topWall = createBox(this.world, -width / 2, -height / 2 - slab, width, slab);
    this.bottomWall = createBox(this.world, -width / 2, height / 2 + slab, width, slab);
    this.leftWall = createBox(this.world, -width / 2 - slab, -height / 2, slab, height);
    this.rightWall = createBox(this.world, width / 2 + slab, -height / 2, slab, height);

    return this;
};

Box.prototype.addMarble = function (x, y, c) {
    var circleSd = new b2CircleDef(),
        circleBd = new b2BodyDef(),
        marble = {};

    circleSd.density = 1.0;
    circleSd.radius = this.dim;
    circleSd.restitution = 0.6;
    circleSd.friction = 0;
    circleBd.AddShape(circleSd);
    circleBd.position.Set(x + Math.random() * 20 - 10, y);

    marble.body = this.world.CreateBody(circleBd);
    marble.radius = circleSd.radius;
    marble.color = c;
    marble.id = 'm' + (this.marbles.length + 1);
    this.marbles.push(marble);
    return marble;
};

Box.prototype.run = function (update) {
    var that = this,
        frameRate = ('ontouchstart' in window) ? 30 : 60;

    window.setInterval(function () {
        that.world.Step(1 / frameRate, 1);
        that.marbles.forEach(function (m) {
            m.x = Math.round(m.body.m_position.x);
            m.y = Math.round(m.body.m_position.y);
            update(m);
        });
    }, 1000 / frameRate);
};

// http://mjijackson.com/2008/02/rgb-to-hsl-and-rgb-to-hsv-color-model-conversion-algorithms-in-javascript

/**
 * Converts an HSL color value to RGB. Conversion formula
 * adapted from http://en.wikipedia.org/wiki/HSL_color_space.
 * Assumes h, s, and l are contained in the set [0, 1] and
 * returns r, g, and b in the set [0, 255].
 *
 * @param   Number  h       The hue
 * @param   Number  s       The saturation
 * @param   Number  l       The lightness
 * @return  Array           The RGB representation
 */

function hslToRgb(h, s, l) {
    var r, g, b;

    if (s == 0) {
        r = g = b = l; // achromatic
    } else {
        function hue2rgb(p, q, t) {
            if (t < 0) t += 1;
            if (t > 1) t -= 1;
            if (t < 1 / 6) return p + (q - p) * 6 * t;
            if (t < 1 / 2) return q;
            if (t < 2 / 3) return p + (q - p) * (2 / 3 - t) * 6;
            return p;
        }

        var q = l < 0.5 ? l * (1 + s) : l + s - l * s;
        var p = 2 * l - q;
        r = hue2rgb(p, q, h + 1 / 3);
        g = hue2rgb(p, q, h);
        b = hue2rgb(p, q, h - 1 / 3);
    }
    return [r * 255, g * 255, b * 255];
}

window.setTimeout(function () {
    var b = new Box(window.innerWidth, window.innerHeight);
    b.addMarble(0, 0, 'yellow');

    function randomColor() {
        var rgb = hslToRgb(Math.random(), 0.8, 0.5);
        return 'rgba(' + Math.round(rgb[0]) + ',' + Math.round(rgb[1]) + ',' + Math.round(rgb[2]) + ', 1)';
    }

    function addMarble(x, y) {
        var angle = +window.orientation,
            color = randomColor(),
            el = document.getElementById('tap');
        if (angle === 0 || typeof window.orientation === 'undefined') {
            b.addMarble(x, y, color);
        } else if (angle == 90) {
            b.addMarble(-y, x, color);
        } else if (angle == 180) {
            b.addMarble(-x, -y, color);
        } else if (angle == -90) {
            b.addMarble(y, -x, color);
        }
        if (el) {
            document.body.removeChild(el);
        }
        el = document.createElement('div');
        el.className = 'tap';
        el.id = 'mtap';
        el.style.borderColor = color;
        el.style.left = x + window.innerWidth / 2;
        el.style.top = y + window.innerHeight / 2;
        document.body.appendChild(el);
        window.setTimeout(function (){
            var t = 'translate(-100px,-100px) scale(0, 0)';
            el.style.webkitTransform = el.style.OTransfrom = el.style.MozTransform = t;
        }, 0);
        window.setTimeout(function (){
            document.body.removeChild(document.getElementById('mtap'));
        }, 150);
    }

    if ('ontouchstart' in window) {
        var el = document.getElementById('text');
        el.addEventListener('touchstart', function (e) {
            if (e.targetTouches.length >= 1) {
                var x = e.targetTouches[0].clientX - window.innerWidth / 2;
                var y = e.targetTouches[0].clientY - window.innerHeight / 2;
                addMarble(x, y);
            }
            e.preventDefault();
            e.stopPropagation();
        }, false);
    } else {
        window.addEventListener('mousedown', function (e) {
            var x = e.clientX - window.innerWidth / 2;
            var y = e.clientY - window.innerHeight / 2;
            addMarble(x, y);
            e.preventDefault();
            e.stopPropagation();
        }, false);
    }

    window.addEventListener('resize', function (e) {
        var angle = +window.orientation;
        if (angle === 0 || angle === 180 || typeof window.orientation === 'undefined') {
            b.resize(window.innerWidth, window.innerHeight);
        } else {
            b.resize(window.innerHeight, window.innerWidth);
        }
    }, false);

    if ('ondevicemotion' in window) {
        window.addEventListener('devicemotion', function (e) {
            b.world.m_gravity.x = 1 *e.accelerationIncludingGravity.x * 40;
            b.world.m_gravity.y = 1 * -e.accelerationIncludingGravity.y * 40;
        }, false);
    } else {
        if ('ondeviceorientation' in window) {
            window.addEventListener('deviceorientation', function (e) {
                b.world.m_gravity.x = e.gamma * 8;
                b.world.m_gravity.y = e.beta * 14;
            }, false);
        }
    }

    b.run(function(m) {
        var el = document.getElementById(m.id),
            angle = +window.orientation;

        if (el === null) {
            el = document.createElement('div');
            el.style.position = 'absolute';
            el.style.width = 2 * m.radius + 'px';
            el.style.height = 2 * m.radius + 'px';
            el.style.backgroundColor = m.color;
            el.style.borderRadius = m.radius + 'px';
            el.style.webkitTransform = 'translate3d(0,0,0)';
            el.id = m.id;
            document.body.appendChild(el);
        }

        if (angle === 0 || typeof window.orientation === 'undefined') {
            el.style.left = Math.round(-m.radius + window.innerWidth / 2 + m.x) + 'px';
            el.style.top = Math.round(-m.radius + window.innerHeight / 2 + m.y) + 'px';
        } else if (angle == 90) {
            el.style.left = Math.round(-m.radius + window.innerWidth / 2 + m.y) + 'px';
            el.style.top = Math.round(-m.radius + window.innerHeight / 2 - m.x) + 'px';
        } else if (angle == 180) {
            el.style.left = Math.round(-m.radius + window.innerWidth / 2 - m.x) + 'px';
            el.style.top = Math.round(-m.radius + window.innerHeight / 2 - m.y) + 'px';
        } else if (angle == -90) {
            el.style.left = Math.round(-m.radius + window.innerWidth / 2 - m.y) + 'px';
            el.style.top = Math.round(-m.radius + window.innerHeight / 2 + m.x) + 'px';
        }
    });

    window.setTimeout(function () {
        var el = document.getElementById('hint');
        el.style.opacity = 0.0;
        el.style.visibility = 'hidden';
    }, 1000);
}, 0);
