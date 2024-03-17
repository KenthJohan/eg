



function test_ws()
{
	let desc = {};
	let obj = create_websocket(desc);
	document.getElementById("close").addEventListener("click", obj.close);
	document.getElementById("open").addEventListener("click", (e) => {
		obj.open();
	});
	document.getElementById("send_dummy").addEventListener("click", obj.send_dummy);
	document.getElementById("send").addEventListener("click", (event) =>  {
		let value = document.getElementById("sendval").value;
		//console.log(value);
		obj.send(value);
	});
}





document.addEventListener("DOMContentLoaded", function () {
	test_ws();
	test_monitor();
}, false);



