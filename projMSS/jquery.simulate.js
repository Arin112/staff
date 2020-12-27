 /*!
 * jQuery Simulate v0.0.1 - simulate browser mouse and keyboard events
 * https://github.com/jquery/jquery-simulate
 *
 * Copyright 2012 jQuery Foundation and other contributors
 * Released under the MIT license.
 * http://jquery.org/license
 *
 * Date: Sun Dec 9 12:15:33 2012 -0500
 */

;(function( $, undefined ) {
	"use strict";

var rkeyEvent = /^key/,
	rmouseEvent = /^(?:mouse|contextmenu)|click/,
	rdocument = /\[object (?:HTML)?Document\]/;

function isDocument(ele) {
	return rdocument.test(Object.prototype.toString.call(ele));
}

function windowOfDocument(doc) {
	for (var i=0; i < window.frames.length; i+=1) {
		try{
		if (window.frames[i] && window.frames[i].document === doc) {
			return window.frames[i];
		}
		}catch{}
	}
	return window;
}

$.fn.simulate = function( type, options ) {
	return this.each(function() {
		new $.simulate( this, type, options );
	});
};

$.simulate = function( elem, type, options ) {
	var method = $.camelCase( "simulate-" + type );

	this.target = elem;
	this.options = options || {};

	if ( this[ method ] ) {
		this[ method ]();
	} else {
		this.simulateEvent( elem, type, this.options );
	}
};

$.extend( $.simulate, {

	keyCode: {
		BACKSPACE: 8,
		COMMA: 188,
		DELETE: 46,
		DOWN: 40,
		END: 35,
		ENTER: 13,
		ESCAPE: 27,
		HOME: 36,
		LEFT: 37,
		NUMPAD_ADD: 107,
		NUMPAD_DECIMAL: 110,
		NUMPAD_DIVIDE: 111,
		NUMPAD_ENTER: 108,
		NUMPAD_MULTIPLY: 106,
		NUMPAD_SUBTRACT: 109,
		PAGE_DOWN: 34,
		PAGE_UP: 33,
		PERIOD: 190,
		RIGHT: 39,
		SPACE: 32,
		TAB: 9,
		UP: 38
	},

	buttonCode: {
		LEFT: 0,
		MIDDLE: 1,
		RIGHT: 2
	}
});

$.extend( $.simulate.prototype, {

	simulateEvent: function( elem, type, options ) {
		var event = this.createEvent( type, options );
		this.dispatchEvent( elem, type, event, options );
	},

	createEvent: function( type, options ) {
		if ( rkeyEvent.test( type ) ) {
			return this.keyEvent( type, options );
		}

		if ( rmouseEvent.test( type ) ) {
			return this.mouseEvent( type, options );
		}
	},

	mouseEvent: function( type, options ) {
		var event,
			eventDoc,
			doc = isDocument(this.target)? this.target : (this.target.ownerDocument || document),
			docEle,
			body;
		
		
		options = $.extend({
			bubbles: true,
			cancelable: (type !== "mousemove"),
			view: windowOfDocument(doc),
			detail: 0,
			screenX: 0,
			screenY: 0,
			clientX: 1,
			clientY: 1,
			ctrlKey: false,
			altKey: false,
			shiftKey: false,
			metaKey: false,
			button: 0,
			relatedTarget: undefined
		}, options );

		
		
		if ( doc.createEvent ) {
			event = doc.createEvent( "MouseEvents" );
			event.initMouseEvent( type, options.bubbles, options.cancelable,
				options.view, options.detail,
				options.screenX, options.screenY, options.clientX, options.clientY,
				options.ctrlKey, options.altKey, options.shiftKey, options.metaKey,
				options.button, options.relatedTarget || doc.body.parentNode );

			// IE 9+ creates events with pageX and pageY set to 0.
			// Trying to modify the properties throws an error,
			// so we define getters to return the correct values.
			if ( event.pageX === 0 && event.pageY === 0 && Object.defineProperty ) {
				eventDoc = isDocument(event.relatedTarget)? event.relatedTarget : (event.relatedTarget.ownerDocument || document);
				docEle = eventDoc.documentElement;
				body = eventDoc.body;

				Object.defineProperty( event, "pageX", {
					get: function() {
						return options.clientX +
							( docEle && docEle.scrollLeft || body && body.scrollLeft || 0 ) -
							( docEle && docEle.clientLeft || body && body.clientLeft || 0 );
					}
				});
				Object.defineProperty( event, "pageY", {
					get: function() {
						return options.clientY +
							( docEle && docEle.scrollTop || body && body.scrollTop || 0 ) -
							( docEle && docEle.clientTop || body && body.clientTop || 0 );
					}
				});
			}
		} else if ( doc.createEventObject ) {
			event = doc.createEventObject();
			$.extend( event, options );
			// standards event.button uses constants defined here: http://msdn.microsoft.com/en-us/library/ie/ff974877(v=vs.85).aspx
			// old IE event.button uses constants defined here: http://msdn.microsoft.com/en-us/library/ie/ms533544(v=vs.85).aspx
			// so we actually need to map the standard back to oldIE
			event.button = {
				0: 1,
				1: 4,
				2: 2
			}[ event.button ] || event.button;
		}

		return event;
	},

	keyEvent: function( type, options ) {
		var event, doc;
		options = $.extend({
			bubbles: true,
			cancelable: true,
			view: windowOfDocument(doc),
			ctrlKey: false,
			altKey: false,
			shiftKey: false,
			metaKey: false,
			keyCode: 0,
			charCode: undefined
		}, options );

		doc = isDocument(this.target)? this.target : (this.target.ownerDocument || document);
		if ( doc.createEvent ) {
			try {
				event = doc.createEvent( "KeyEvents" );
				event.initKeyEvent( type, options.bubbles, options.cancelable, options.view,
					options.ctrlKey, options.altKey, options.shiftKey, options.metaKey,
					options.keyCode, options.charCode );
			// initKeyEvent throws an exception in WebKit
			// see: http://stackoverflow.com/questions/6406784/initkeyevent-keypress-only-works-in-firefox-need-a-cross-browser-solution
			// and also https://bugs.webkit.org/show_bug.cgi?id=13368
			// fall back to a generic event until we decide to implement initKeyboardEvent
			} catch( err ) {
				event = doc.createEvent( "Events" );
				event.initEvent( type, options.bubbles, options.cancelable );
				$.extend( event, {
					view: options.view,
					ctrlKey: options.ctrlKey,
					altKey: options.altKey,
					shiftKey: options.shiftKey,
					metaKey: options.metaKey,
					keyCode: options.keyCode,
					charCode: options.charCode
				});
			}
		} else if ( doc.createEventObject ) {
			event = doc.createEventObject();
			$.extend( event, options );
		}

		if ( !!/msie [\w.]+/.exec( navigator.userAgent.toLowerCase() ) || (({}).toString.call( window.opera ) === "[object Opera]") ) {
			event.keyCode = (options.charCode > 0) ? options.charCode : options.keyCode;
			event.charCode = undefined;
		}

		return event;
	},

	dispatchEvent: function( elem, type, event, options ) {
		if (options.jQueryTrigger === true) {
			$(elem).trigger($.extend({}, event, options, {type: type}));
		}
		else if ( elem.dispatchEvent ) {
			elem.dispatchEvent( event );
		} else if ( elem.fireEvent ) {
			elem.fireEvent( "on" + type, event );
		}
	},

	simulateFocus: function() {
		var focusinEvent,
			triggered = false,
			$element = $( this.target );

		function trigger() {
			triggered = true;
		}

		$element.bind( "focus", trigger );
		$element[ 0 ].focus();

		if ( !triggered ) {
			focusinEvent = $.Event( "focusin" );
			focusinEvent.preventDefault();
			$element.trigger( focusinEvent );
			$element.triggerHandler( "focus" );
		}
		$element.unbind( "focus", trigger );
	},

	simulateBlur: function() {
		var focusoutEvent,
			triggered = false,
			$element = $( this.target );

		function trigger() {
			triggered = true;
		}

		$element.bind( "blur", trigger );
		$element[ 0 ].blur();

		// blur events are async in IE
		setTimeout(function() {
			// IE won't let the blur occur if the window is inactive
			if ( $element[ 0 ].ownerDocument.activeElement === $element[ 0 ] ) {
				$element[ 0 ].ownerDocument.body.focus();
			}

			// Firefox won't trigger events if the window is inactive
			// IE doesn't trigger events if we had to manually focus the body
			if ( !triggered ) {
				focusoutEvent = $.Event( "focusout" );
				focusoutEvent.preventDefault();
				$element.trigger( focusoutEvent );
				$element.triggerHandler( "blur" );
			}
			$element.unbind( "blur", trigger );
		}, 1 );
	}
});



/** complex events **/

function findCenter( elem ) {
	var offset,
		$document,
		$elem = $( elem );
	
	if ( isDocument($elem[0]) ) {
		$document = $elem;
		offset = { left: 0, top: 0 };
	}
	else {
		$document = $( $elem[0].ownerDocument || document );
		offset = $elem.offset();
	}
	
	return {
		x: offset.left + $elem.outerWidth() / 2 - $document.scrollLeft(),
		y: offset.top + $elem.outerHeight() / 2 - $document.scrollTop()
	};
}

function findCorner( elem ) {
	var offset,
		$document,
		$elem = $( elem );
	
	if ( isDocument($elem[0]) ) {
		$document = $elem;
		offset = { left: 0, top: 0 };
	}
	else {
		$document = $( $elem[0].ownerDocument || document );
		offset = $elem.offset();
	}

	return {
		x: offset.left - $document.scrollLeft(),
		y: offset.top - $document.scrollTop()
	};
}

$.extend( $.simulate.prototype, {
	simulateDrag: function() {
		var i = 0,
			target = this.target,
			options = this.options,
			center = options.handle === "corner" ? findCorner( target ) : findCenter( target ),
			x = Math.floor( center.x ),
			y = Math.floor( center.y ),
			coord = { clientX: x, clientY: y },
			dx = options.dx || ( options.x !== undefined ? options.x - x : 0 ),
			dy = options.dy || ( options.y !== undefined ? options.y - y : 0 ),
			moves = options.moves || 3;

		this.simulateEvent( target, "mousedown", coord );

		for ( ; i < moves ; i++ ) {
			x += dx / moves;
			y += dy / moves;

			coord = {
				clientX: Math.round( x ),
				clientY: Math.round( y )
			};

			this.simulateEvent( target.ownerDocument, "mousemove", coord );
		}

		if ( $.contains( document, target ) ) {
			this.simulateEvent( target, "mouseup", coord );
			this.simulateEvent( target, "click", coord );
		} else {
			this.simulateEvent( document, "mouseup", coord );
		}
	}
});

})( jQuery );