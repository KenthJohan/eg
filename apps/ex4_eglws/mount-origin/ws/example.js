


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
		if (msg.data instanceof ArrayBuffer) {
			const view = new DataView(msg.data);
			console.log(view.getInt32(0, true));
		} else if (typeof(msg.data) === 'string') {
			console.log(msg.data);
		}
		/*
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
		*/
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
		obj.ws.binaryType = "arraybuffer";
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





function create_table(desc)
{
	let obj = {};
	obj.rows = [];

	obj.add = function() {
		obj.rows.push();
	}
}







function test_ws()
{
	let desc = {};
	let obj = create_websocket(desc);
	document.getElementById("close").addEventListener("click", obj.close);
	document.getElementById("open").addEventListener("click", (e) => {
		obj.open();
	});
	document.getElementById("send_dummy").addEventListener("click", obj.send_dummy);
}

function test_table()
{
	let desc = {};
	let obj = create_table(desc);
	document.getElementById("add").addEventListener("click", (e) => {
		let tr = document.createElement("tr");
		let td1 = document.createElement("td");
		let td2 = document.createElement("td");
		let td3 = document.createElement("td");
		let in1 = document.createElement(`input`);
		let in2 = document.createElement(`input`);
		let in3 = document.createElement(`input`);
		in1.setAttribute("type", "text");
		td1.appendChild(in1);
		td2.appendChild(in2);
		td3.appendChild(in3);
		tr.appendChild(td1);
		tr.appendChild(td2);
		tr.appendChild(td3);
		document.getElementById("rows").appendChild(tr);
	});
}



document.addEventListener("DOMContentLoaded", function () {
	test_ws();
	test_table();
}, false);



