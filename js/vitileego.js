var VitileegoModule;
			
function moduleInit() {
	VitileegoModule = document.getElementById("vitileegocv");
	VitileegoModule.addEventListener("message", handleMessage, true);
	console.log("vitileego module successfully loaded.");
	
	VitileegoModule.postMessage({fn: "init"});
}

function handleMessage(e) {
	if (e.data) {
		if (e.data.type === "log") {
			console.info(e.data.message);
		} else {
			console.log(e.data.message);
		}
	} else console.error("No message returned.");
}