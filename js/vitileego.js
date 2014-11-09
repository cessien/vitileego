var debugMode = true;
var VitileegoModule;
var slipslidevar = 1;
var active = false;

var pastLoadedFiles = JSON.parse(localStorage.getItem("past_files"));
if(!pastLoadedFiles) {
	pastLoadedFiles = {};
	localStorage.setItem("past_files", JSON.stringify(pastLoadedFiles));
}
			
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

function update_all(){
	if(active) {
		console.log("pausing");
		return;
	}
	active = true;
	var data = {
		fn:"adjust",
		threshold: parseInt($('#threshold').val())||0,
		ratio: parseInt($('#ratio').val())||1,
		kernel_size: parseInt($('#kernel-size').val())||3,
		min_slope: parseFloat($('#min-slope').val())||1,
		step1: $('#step1').prop('checked'),
		step2: $('#step2').prop('checked'),
		step3: $('#step3').prop('checked'),
		use_mask: $('#use-mask').prop('checked'),
		slipslide: slipslidevar
	};
	console.log(data);
	VitileegoModule.postMessage(data);
	setTimeout(function(){active = false;},1);
}

function addPicture(file, picture) {
	pastLoadedFiles[file.name] = file.name;
	localStorage.setItem("files",JSON.stringify(pastLoadedFiles));
}

function loadPicture(e){
	var file = $("#files")[0].files[0];
	
	var reader = new FileReader();

	// Closure to capture the file information.
	reader.onload = function(e) {
		var pictureData = reader.result;
		addPicture(file, pictureData);
		pictureData = pictureData.substring(pictureData.indexOf(',')+1);
		VitileegoModule.postMessage({fn:"picture",picture: pictureData, size: pictureData.length});
	};

	// Read in the image file as a data URL.
	reader.readAsDataURL(file);
}

function loadStoredPicture(name){
	var pictureData = pastLoadedFiles[name];
	pictureData = pictureData.substring(pictureData.indexOf(',')+1);
	VitileegoModule.postMessage({fn:"picture",picture: pictureData, size: pictureData.length});
}

$(document).ready(function(){
	$( "#threshold-slider" ).slider({
      min: 0,
      max: 100,
			value: 30,
			range: "min",
      change: function (){},
			slide: function (){
				$('#threshold').val($('#threshold-slider').slider("value"));
				update_all();
			}
	});
	
	$( "#kernel-slider" ).slider({
      min: 3,
      max: 7,
			value: 3,
			step: 2,
			range: "min",
      change: function (){
				$('#kernel-size').val($('#kernel-slider').slider("value"));
				update_all();
			}
	});
	
	$( "#slipslide" ).slider({
      min: 1,
      max: 100,
			value: 1,
			range: "min",
			slide: function (){
				slipslidevar = parseInt($("#slipslide").slider("value"));
				update_all();
			}
	});
	
	$("input").keyup(function(){
		update_all();
	});
});