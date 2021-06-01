var pageCache = new Array();

function debounce(func, wait) {
    var timer;
    return function() {
        var context = this,
            args = arguments;
        clearTimeout(timer)
        timer = setTimeout(function() {
            func.apply(context, args)
        }, wait)
    }
}
var Cookie = function() {}
Cookie.set = function(key, value) {
    document.cookie = key + '=' + value;
}
Cookie.get = function(key) {
    return document.cookie.substring(key.length + 1);
}
Cookie.del = function(key) {
    document.cookie = key + '=; expires=Thu, 01 Jan 1970 00:00:00 UTC;';
}
Element.prototype.css = function(key, value) {
    switch (key) {
        case 'color':
            this.style.color = value;
            break;
        case 'background-color':
            this.style.backgroundColor = value;
            break;
        case 'width':
            this.style.width = value;
            console.log(this);
            break;
        case 'height':
            this.style.height = value;
            break;
        case 'border-radius':
            this.style.borderRadius = value;
            break;
        case 'display':
            this.style.display = value;
            break;
        case 'opacity':
            this.style.opacity = value;
            break;
        case 'position':
            this.style.position = value;
            break;
        case 'top':
            this.style.top = value;
            break;
        case 'right':
            this.style.right = value;
            break;
        case 'bottom':
            this.style.bottom = value;
            break;
        case 'left':
            this.style.left = value;
            break;
        case 'margin':
            this.style.margin = value;
            break;
        case 'margin-top':
            this.style.marginTop = value;
            break;
        case 'margin-right':
            this.style.marginRight = value;
            break;
        case 'margin-bottom':
            this.style.marginBottom = value;
            break;
        case 'margin-left':
            this.style.marginLeft = value;
            break;
        case 'transition':
            this.style.transition = value;
            break;
        case 'transform':
            this.style.transform = value;
            break;
        case 'font-size':
            this.style.fontSize = value;
            break;

        default:
            break;
    }
    return this;
}
Element.prototype.mover = function(func) {
    this.onmouseover = function() {
        func && func();
    }
}
Element.prototype.mout = function(func) {
    this.onmouseout = function() {
        func && func();
    }
}
Element.prototype.mclick = function(func) {
    this.onclick = function() {
        func && func();
    }
}
HTMLCollection.prototype.mclick = function(func) {
    for (var i = 0; i < this.length; i++) {
        this[i].onclick = function() {
            func && func(this);
        }
    }
}
Element.prototype.getSize = function(key) {
    switch (key) {
        case 'width':
            return parseInt(this.offsetWidth);
            break;
        case 'height':
            return parseInt(this.offsetHeight);
            break;

        default:
            break;
    }
}
Element.prototype.go = function(url) {
    this.onclick = function() {
        if (!document.cookie.substring(10) == '') {
            pageCache.push(document.cookie.substring(10));
        }
        pageCache.push(window.location.href);
        Cookie.set('pageCache', pageCache);
        setTimeout(function() {
            window.location.href = url;
        }, 200);
    }
}
Element.prototype.back = function() {
    this.onclick = function() {
        if (!Cookie.get('pageCache').substring(10) == '') {
            pageCache = document.cookie.substring(10).split(',');
        }
        var buff = pageCache[pageCache.length - 1];
        if (pageCache.length == 1) {
            Cookie.del('pageCache');
        } else {
            pageCache.pop();
            Cookie.set('pageCache', pageCache);
        }

        window.location.href = buff;
    }
}

function $(el) {
    return query(el);
}


function query (el) {
    if (typeof el === 'string') {
      var selected = document.querySelectorAll(el);
      if (!selected) {
        warn(
          'Cannot find element: ' + el
        );
        return document.createElement('div')
      }
      if (selected.length > 1) {
          return selected
      }
      return selected[0]
    } else {
      return el
    }
  }

function q(query) {
    return document.querySelector(query);
}

function qa(query) {
    return document.querySelectorAll(query);
}

function canvas(obj) {
    var type = obj[0];
    var id = obj.substr(1, obj.length - 1);
    if (obj[0] == '#') {
        var result = document.getElementById(obj.substr(1, obj.length - 1));
    }
    return result.getContext('2d');
}

function Person() {
    this.setBirthday = function(birthday) {
        this.birthday = birthday;
    }
    this.setAge = function(age) {
        this.age = age;
    }
    this.getAgeByBirthday = function() {
        var birthday = new Date(this.birthday);
        var today = new Date();
        var age = today.getFullYear() - birthday.getFullYear();
        if (today.getMonth() <= birthday.getMonth()) {
            if (today.getDate() < birthday.getDate()) {
                age--;
            }
        }
        this.age = age;
        return age;
    }
}

function Ajax() {
    this.get = function(url, func) {
        var xmlhttp;
        if (window.XMLHttpRequest) {
            //  IE7+, Firefox, Chrome, Opera, Safari 浏览器执行代码
            xmlhttp = new XMLHttpRequest();
        } else {
            // IE6, IE5 浏览器执行代码
            xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
        }
        xmlhttp.onreadystatechange = function() {
            if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
                func && func(xmlhttp.responseText);
            }
        }
        xmlhttp.open('GET', url, true);
        xmlhttp.send();
    }

    this.post = function(url, data, func) {
        var xmlhttp;
        if (window.XMLHttpRequest) {
            //  IE7+, Firefox, Chrome, Opera, Safari 浏览器执行代码
            xmlhttp = new XMLHttpRequest();
        } else {
            // IE6, IE5 浏览器执行代码
            xmlhttp = new ActiveXObject("Microsoft.XMLHTTP");
        }
        xmlhttp.open('POST', url);
        xmlhttp.setRequestHeader('Content-type', 'application/x-www-form-urlencoded');
        xmlhttp.send(data);
        xmlhttp.onreadystatechange = function() {
            if (xmlhttp.readyState == 4 && xmlhttp.status == 200) {
                func && func(xmlhttp.responseText);
            }
        }

    }
}

function Environment() {
    var inBrowser = typeof window !== 'undefined';
    var inWeex = typeof WXEnvironment !== 'undefined' && !!WXEnvironment.platform;
    var weexPlatform = inWeex && WXEnvironment.platform.toLowerCase();
    UA = inBrowser && window.navigator.userAgent.toLowerCase();
    this.isIE = UA && /msie|trident/.test(UA);
    this.isIE9 = UA && UA.indexOf('msie 9.0') > 0;
    this.isEdge = UA && UA.indexOf('edge/') > 0;
    this.isAndroid = (UA && UA.indexOf('android') > 0) || (weexPlatform === 'android');
    this.isIOS = (UA && /iphone|ipad|ipod|ios/.test(UA)) || (weexPlatform === 'ios');
    this.isChrome = UA && /chrome\/\d+/.test(UA) && !this.isEdge;
    this.isPhantomJS = UA && /phantomjs/.test(UA);
    this.isFF = UA && UA.match(/firefox\/(\d+)/);
}