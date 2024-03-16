



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





document.addEventListener("DOMContentLoaded", function () {
	test_ws();
	test_monitor();
}, false);



