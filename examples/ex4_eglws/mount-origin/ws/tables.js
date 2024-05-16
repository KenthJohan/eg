// https://stackoverflow.com/questions/18333427/how-to-insert-a-row-in-an-html-table-body-in-javascript
// https://developer.mozilla.org/en-US/docs/Web/API/HTMLTableElement
function create_table(desc)
{
	let obj = {
		table : desc.table
	};
	obj.colgroup = obj.table.children[0];

	//console.log(obj.colgroup.childElementCount);

	obj.add_row = function(index) {
		//console.log(obj.table.tBodies[0]);
		let r = obj.table.tBodies[0].insertRow(index);
		console.log(`Inserting cells: ${obj.colgroup.childElementCount}`);
		for (var c = 0; c < obj.colgroup.childElementCount; c++) {
			let cell = r.insertCell(c);
			//cell.innerText = c;
		}
		return r;
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


	obj.add_row_input = function() {
		let r = obj.add_row(0);
		for (var i = 0; i < r.children.length; i++) {
			let input = document.createElement('input');
			r.children[i].appendChild(input);
		}
	}


	return obj;
}