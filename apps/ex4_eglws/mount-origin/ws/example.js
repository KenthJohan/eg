


function create_onopen(obj) {
	return function () {
		console.log("ws.onopen", obj.wsurl);
		document.getElementById("r").disabled = 0;
		document.getElementById("open").disabled = true;
		document.getElementById("close").disabled = false;
	};
}


function create_onmessage(obj) {
	return function(msg) {
		console.log("ws.onmessage", msg);
		var n, s = "";
		let max = 5;
		obj.ring[obj.head] = msg.data + "\n";
		obj.head = (obj.head + 1) % max;
		if (obj.tail === obj.head) {
			obj.tail = (obj.tail + 1) % max;
		}
		n = obj.tail;
		do {
			s = s + obj.ring[n];
			n = (n + 1) % max;
		} while (n !== obj.head);
		document.getElementById("r").value = s;
		document.getElementById("r").scrollTop = document.getElementById("r").scrollHeight;
	};
}

function create_onclose(obj) {
	return function () {
		console.log("ws.onclose", obj.wsurl);
		document.getElementById("open").disabled = false;
		document.getElementById("close").disabled = true;
	};	
}


function create_websocket(desc)
{
	let obj = {};
	obj.ws = null;
	obj.head = 0;
	obj.tail = 0;
	obj.ring = new Array();
	obj.counter = 0;

	obj.open = function() {
		obj.wsurl = "ws://localhost:7681/";
		console.log("WebSocket open", obj.wsurl);
		console.assert(obj.ws == null);
		obj.ws = new WebSocket(obj.wsurl, "lws-minimal");
		obj.ws.onopen = create_onopen(obj);
		obj.ws.onmessage = create_onmessage(obj);
		obj.ws.onclose = create_onclose(obj);
	}

	obj.close = function() {
		console.log("Close");
		obj.ws.close();
		obj.ws = null;
	}

	obj.send_dummy = function() {
		obj.ws.send(`Dummy ${obj.counter++}.`);
	}


	return obj;
}



document.addEventListener("DOMContentLoaded", function () {
	let desc = {};
	let obj = create_websocket(desc);

	document.getElementById("close").addEventListener("click", obj.close);
	document.getElementById("open").addEventListener("click", (e) => {
		obj.open();
	});
	document.getElementById("send_dummy").addEventListener("click", obj.send_dummy);
}, false);



