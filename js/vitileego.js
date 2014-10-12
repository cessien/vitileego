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
	var file = $("#files")[0].files[0];
	
	var reader = new FileReader();

	// Closure to capture the file information.
	reader.onload = function(file) {
	var pictureData = new Uint8Array(reader.result);
		//pictureData = atob(pictureData.substring(pictureData.indexOf(',')+1));
		//pictureData =  atob('iVBORw0KGgoAAAANSUhEUgAAABAAAAAQCAYAAAAf8/9hAAAAGXRFWHRTb2Z0d2FyZQBBZG9iZSBJbWFnZVJlYWR5ccllPAAAAp1JREFUeNqEU21IU1EYfu7unW5Ty6aBszYs6MeUjGVYokHYyH5E1B9rZWFEFPQnAwmy6Hc/oqhfJsRKSSZGH1JIIX3MNCsqLTD9o1Oj6ebnnDfvvefezrnbdCHhCw/n433P8z7nPe/hBEEAtX0U7hc164uwuvVSXKwZLoOmaRDim+7m9vZa0WiEKSUFFpNpCWlmMyypqTDRuYn6t3k8vmQ2gRDCxs0t9fW45F52aBTROJLtZl7nEZad2m+KtoQCQ0FBARyOCGRZ/q92I1WgqqXlfdd95VsrK8/pChIEqqpCkiQsiCII0aBQZZoWl8lzFDwsFjMl0DBLY8Lj41hBwK4jSQrWOIphL6xYyhwJDWGo6wFSaH1Y3PTCAsITE1oyAa8flhWkbSiCLX8vun11eiGIpiJ/z2nYdx5HqLdVV7elrOzsuqysL3rmBIGiKPizKCHHWY4PLVeQbnXAdegqdhy+hu8dDTBnbqQJZJ1A7u+vz7RaiymWCZgCRSF6Edk8b9cx+B/W6WuVxPaZnyiqXoPpyUmVYvkKTIFClHigEieKjYuSvETUllaF4GAUM1NT6ooaJDKx+aDfC9fByxj90REb+9ppmIoAscH/6leg8MS9DJXPAM9xHCM443K57C6biMjcHDaVVCHw9RmCA2/RGC5C00AqXk/m4p20HZK4CM/J3Zk9n0ecMBhDQnJHcrTisyMfdQXOilrdMfxcwoHq/fg5R59TiQV3hYGKo6X2J/c7LyQIjOx9GXhOw/zoJ8wEevRGyp53o/lGMNYsBgPtEwLecwov7/jGDKa1twT6o3KpL4MdZgGsWZLtfPr7f1q58k1JNHy7YYaM+J+K3Y2PmAIbRavX66229hrGVvvL5uzsHDEUvUu+NT1my78CDAAMK1a8/QaZCgAAAABJRU5ErkJggg==');
		
		console.log("client data: "+pictureData);
		console.log("client legth: "+pictureData.length);
		VitileegoModule.postMessage({fn:"picture",picture: pictureData, size: pictureData.length});
	};

	// Read in the image file as a data URL.
	reader.readAsArrayBuffer(file);
}