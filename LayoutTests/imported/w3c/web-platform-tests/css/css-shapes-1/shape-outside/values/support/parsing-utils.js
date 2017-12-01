function testComputedStyle(value, expected) {
    var div = document.createElement('div');
    div.style.setProperty('shape-outside', value);
    document.body.appendChild(div);
    var style = getComputedStyle(div);
    var actual = style.getPropertyValue('shape-outside');
    actual = roundResultStr(actual);
    document.body.removeChild(div);

    assert_equals(actual, typeof expected !== 'undefined' ? expected : value);
}

function setUnit(str, unit1) {
    return str.replace(new RegExp('u1', 'g'), unit1);
}

function convertToPx(origValue) {

    var valuesToConvert = origValue.match(/[0-9]+(\.[0-9]+)?([a-z]{2,4}|%)/g);
    if(!valuesToConvert)
        return origValue;

    var retStr = origValue;
    for(var i = 0; i < valuesToConvert.length; i++) {
        var unit = valuesToConvert[i].match(/[a-z]{2,4}|%/).toString();
        var numberStr = valuesToConvert[i].match(/[0-9]+(\.[0-9]+)?/)[0];

        var number = parseFloat(numberStr);
        var convertedUnit = 'px';
        if( typeof number !== 'NaN' )
        {
             if (unit == 'in') {
                 number = (96 * number);
             } else if (unit == 'cm') {
                 number = (37.795275591 * number);
             } else if (unit == 'mm') {
                 number = (3.779527559 * number);
             } else if (unit == 'pt') {
                 number = (1.333333333333 * number);
             } else if (unit == 'pc') {
                 number = (16 * number);
             } else if (unit == 'em') {
                 number = (16 * number);
             } else if (unit == 'ex') {
                 number = (12.8 * number);
             } else if (unit == 'ch') {
                 number = (16 * number);
             } else if (unit == 'rem') {
                 number = (16 * number);
             } else if (unit == 'vw') {
                 number = ((.01 * window.innerWidth) * number);
             } else if (unit == 'vh') {
                 number = ((.01 * window.innerHeight) * number);
             } else if (unit == 'vmin') {
                 number = Math.min( (.01 * window.innerWidth), (.01 * window.innerHeight) ) * number;
             } else if (unit == 'vmax') {
                number = Math.max( (.01 * window.innerWidth), (.01 * window.innerHeight) ) * number;
             }
             else {
                 convertedUnit = unit;
             }
            number = Math.round(number * 1000) / 1000;
            var find = valuesToConvert[i];
            var replace = number.toString() + convertedUnit;
            retStr = retStr.replace(valuesToConvert[i], number.toString() + convertedUnit);
      }
    }
    return retStr.replace(',,', ',');
}

function roundResultStr(str) {
    if(Object.prototype.toString.call( str ) !== '[object String]')
        return str;

    var numbersToRound = str.match(/[0-9]+\.[0-9]+/g);
    if(!numbersToRound)
        return str;

    var retStr = str;
    for(var i = 0; i < numbersToRound.length; i++) {
        num = parseFloat(numbersToRound[i]);
        if( !isNaN(num) ) {
            roundedNum = Math.round(num*1000)/1000;
            retStr = retStr.replace(numbersToRound[i].toString(), roundedNum.toString());
        }
    }

    return retStr;
}

function each(object, func) {
    for (var prop in object) {
        if (object.hasOwnProperty(prop)) {
            func(prop, object[prop]);
        }
    }
}

function setupFonts(func) {
    return function () {
        var fontProperties = {
            'font-family': 'Ahem',
            'font-size': '16px',
            'line-height': '1'
        };
        var savedValues = { };
        each(fontProperties, function (key, value) {
            savedValues[key] = document.body.style.getPropertyValue(key);
            document.body.style.setProperty(key, value);
        });
        func.apply(this, arguments);
        each(fontProperties, function (key, value) {
            if (value) {
                document.body.style.setProperty(key, value);
            }
            else {
                document.body.style.removeProperty(key);
            }
        });
    };
}

var ParsingUtils = {
    testComputedStyle: setupFonts(testComputedStyle),
    roundResultStr: roundResultStr,
    setupFonts: setupFonts
};
