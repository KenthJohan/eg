


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







// https://stackoverflow.com/questions/18333427/how-to-insert-a-row-in-an-html-table-body-in-javascript
// https://developer.mozilla.org/en-US/docs/Web/API/HTMLTableElement
function create_table(desc)
{
	let obj = {
		table : desc.table
	};
	obj.colgroup = obj.table.children[0];

	//console.log(obj.colgroup.childElementCount);

	obj.add_row = function() {
		//console.log(obj.table.tBodies[0]);
		let r = obj.table.tBodies[0].insertRow();
		console.log(`Inserting cells: ${obj.colgroup.childElementCount}`);
		for (var c = 0; c < obj.colgroup.childElementCount; c++) {
			let cell = r.insertCell(c);
			cell.innerText = c;
		}
	}


	obj.add_col = function(index) {
		let col = document.createElement('col');
		obj.colgroup.insertBefore(col, obj.colgroup.children[index]);
		//console.log(obj.colgroup.children[index]);
		let rowsh = obj.table.tHead.rows;
		let rowsb = obj.table.tBodies[0].rows;
		//console.log(rowsh);

		console.log(`Inserting cells in thead: ${rowsh.length}`);
		for (var r = 0; r < rowsh.length; r++) {
			let cell = rowsh[r].insertCell(index);
			cell.innerText = r;
		}
		console.log(`Inserting cells in tBodies: ${rowsb.length}`);
		for (var r = 0; r < rowsb.length; r++) {
			let cell = rowsb[r].insertCell(index);
			cell.innerText = r;
		}
	}


	return obj;
}












function create_monitor(desc)
{
	let obj = {};
	obj.rows = [];

	obj.add = function(name, component, value) {
		let r = {
			name : name,
			component : component,
			value : value,
			e : null
		};
		obj.rows.push(r);

		fetch(`http://127.0.0.1:27750/entity/${component}&type_info=true`, { method: "GET" })
		.then((res) => res.json())
		.then((json) => {
			console.log(json);
		})
		.catch((err) => console.error("error:", err));
	}
	return obj;
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

function test_monitor()
{
	let m1 = create_monitor({});
	let t1 = create_table({
		table : document.getElementById("t1")
	});

	document.getElementById("add_row").addEventListener("click", (e) => {
		t1.add_row();
	});

	document.getElementById("add_col").addEventListener("click", (e) => {
		t1.add_col(1);
	});

	document.getElementById("add").addEventListener("click", (e) => {
		m1.add("123", "Comp1", "");
	});
}




document.addEventListener("DOMContentLoaded", function () {
	test_ws();
	test_monitor();
}, false);



