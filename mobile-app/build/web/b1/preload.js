let app = {};

app.version = "0.0.1 developer preview";
app.name = "KrendelCam";

app.preloadUpdate = []


app.fs = localStorage;


window.addEventListener("DOMContentLoaded", ()=>{

	app.fs.log = "";

	log("Start Init App");

	log("Starting Local Storage Cameras");

	if (typeof app.fs.cams == "undefined") {
		app.fs.cams = "{}"
		log("Creating val[app.fs.cams]")
	} else {
		log("val[app.fs.cams] checked")
	}

	loadCams();
	log("Loading List Cams")

	log("Setting Auto Update Preload")

	setInterval(updatePreload, 5000)

})

function updatePreload() {
	for (let i = 0; i < app.preloadUpdate.length; i++) {
		dqs("#camID-" + app.preloadUpdate[i].camID + " .preview img.img").src = ""
		dqs("#camID-" + app.preloadUpdate[i].camID + " .preview img.img").src = "http://" + app.preloadUpdate[i].ip + "/screen";
	}
}


async function loadCams() {

	let db = JSON.parse(app.fs.cams);

	let html = "";

	for (let i = 0; i < db.length; i++) {

		html += "<div class='box' id='camID-" + db[i].camID + "'>";
		html += "<div class='name'>" + db[i].cameraName + "</div>";
		html += "<div class='preview'>";
		html += "<img class='img' border='0'>";
		html += "<div class='img-blur'></div>";
		html += "<div class='text'>";
		html += "<div class='quality'>" + db[i].quality + "</div>";
		html += "<div class='status'>Загрузка...</div>";
		html += "</div>";
		html += "</div>";
		html += "<div class='info'>";
		html += "<div class='wifi'><div></div><span> " + db[i].wifi + "</span></div>";
		html += "<div class='ping'><div></div><span> not support</span></div>";
		html += "<div class='version'><div></div><span> " + db[i].version + "</span></div>";
		html += "<div class='web'><div></div><span> not support</span></div>"
		html += "</div>";
		html += "</div>";
		loadPreload("camID-" + db[i].camID, db[i].ip)
		

	} 
	dqs("#home .boxs").innerHTML = html

}

async function loadPreload(camID, ip) {

	let srv = new XMLHttpRequest;
	srv.timeout = 5000;
	srv.onreadystatechange = await function(){  
		if (srv.readyState == 4) {
			if (srv.response != "") {
				dqs("#" + camID + " .preview .img-blur").style.display = "block"
				dqs("#" + camID + " .preview img.img").src = "http://" + ip + "/screen"
				dqs("#" + camID + " .preview .text .status").innerHTML = ""
				dqs("#" + camID + " .preview .text .quality").style.color = "#F9F7F7"
				dqs("#" + camID + " .preview img.img").style.display = "block"
				app.preloadUpdate.push({
					"camID":db[i].camID,
					"ip":db[i].ip
				})
			} else {
				dqs("#" + camID + " .preview .text .status").innerHTML = "Недоступно <br>в данной сети!"
				dqs("#" + camID + " .preview img.img").style.display = "none"
				dqs("#" + camID + " .preview .text .quality").style.display = "none"

			}
		} else {
			dqs("#" + camID + " .preview img.img").style.display = "none"
			dqs("#" + camID + " .preview .text .status").innerHTML = "Загрузка..."
			dqs("#" + camID + " .preview .img-blur").style.display = "none"
		}
		
	}
	srv.open("GET", "http://" + ip + "/screen", true);
	srv.send()

}