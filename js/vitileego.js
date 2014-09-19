var debugMode = true;
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

function loadPicture(e){
	console.log(e);
	var file = $("#files")[0].files[0];
	//var file = file.substring(file.indexOf("fakepath\\")+9);
	
	$("#vitileegocv").hide();
	$("#loaded-image").hide();
	$("#loading").show();
	
	setTimeout(function(){
		var reader = new FileReader();

		// Closure to capture the file information.
		reader.onload = function(theFile) {
			var elem = $("#loaded-image");
			$(elem).attr("src",reader.result);
			
			//console.log(reader.result);
			
			//var result = reader.result.replace(/data:.*,/,"");
			
			var uarray = new Uint8Array(reader.result);
			console.log(uarray);
			VitileegoModule.postMessage({fn:"picture",picture:uarray});
			$("#loading").hide();
			//elem.show();
		};
		
      // Read in the image file as a data URL.
    reader.readAsArrayBuffer(file);
		
		//$("#loading").attr("src",file);
		//console.log(file.substring(file.indexOf("fakepath\\")+9) + " loaded.");
	},500);
	
	
	
	
}