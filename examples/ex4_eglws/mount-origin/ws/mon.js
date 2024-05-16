/*
function createElement_button(txt) {
	let btn = document.createElement('button');
	btn.innerText = txt;
	btn.addEventListener("click", (e) => {
			
	});
	return btn;
}

*/



function fetch_component(component) {
	fetch(`http://127.0.0.1:27750/entity/${component}&type_info=true`, { method: "GET" })
	.then((res) => res.json())
	.then((json) => {
		let size = json.type_info[0].size;
		console.log(size);
	})
	.catch((err) => {});
}



function set_keyup(r) {
	let a = r.querySelector('input[name=component]');
	a.addEventListener("keyup", (event) => {
		let component = a.value;
		console.log(component);
		fetch_component(component);
	});
	return a;
}


function test_monitor()
{
	//let m1 = create_monitor({});

	let myTemplate = document.getElementById("my-template");
	//console.log(myTemplate.content);

	let t1 = document.getElementById("t1");
	document.getElementById("add_row").addEventListener("click", (e) => {
		const r = document.importNode(myTemplate.content, true);
		set_keyup(r);
		t1.tBodies[0].appendChild(r);
	});


}