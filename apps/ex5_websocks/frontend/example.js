var head = 0, tail = 0, ring = new Array();

function get_appropriate_ws_url(extra_url)
{
	var pcol;
	var u = document.URL;

	/*
	 * We open the websocket encrypted if this page came on an
	 * https:// url itself, otherwise unencrypted
	 */

	if (u.substring(0, 5) === "https") {
		pcol = "wss://";
		u = u.substr(8);
	} else {
		pcol = "ws://";
		if (u.substring(0, 4) === "http")
			u = u.substr(7);
	}

	u = u.split("/");

	/* + "/xxx" bit is for IE10 workaround */

	return pcol + u[0] + "/" + extra_url;
}

function new_ws(urlpath, protocol)
{
	return new WebSocket(urlpath, protocol);
}




function fx(obj, a) {
	return function () {
		if(a == true) {
			console.log("Open");
			obj.ws = new_ws(get_appropriate_ws_url(""), "lws-minimal");
			try {
				var ws = obj.ws;
				ws.onopen = function() {
					document.getElementById("r").disabled = 0;
				};
			
				ws.onmessage = function got_packet(msg) {
					console.log("msg", msg);
					var n, s = "";
					let max = 30;
			
					ring[head] = msg.data + "\n";
					head = (head + 1) % max;
					if (tail === head)
						tail = (tail + 1) % max;
			
					n = tail;
					do {
						s = s + ring[n];
						n = (n + 1) % max;
					} while (n !== head);
			
					document.getElementById("r").value = s; 
					document.getElementById("r").scrollTop =
					document.getElementById("r").scrollHeight;
				};
			
				ws.onclose = function(){
					document.getElementById("r").disabled = 1;
				};
			} catch(exception) {
				alert("<p>Error " + exception);  
			}
		} else if (a == false){
			console.log("Close");
			obj?.ws?.close();
		}
	}
}

document.addEventListener("DOMContentLoaded", function() {
	let obj = {};
	document.getElementById("close").addEventListener("click", fx(obj, false));
	document.getElementById("open").addEventListener("click", fx(obj, true));
}, false);
