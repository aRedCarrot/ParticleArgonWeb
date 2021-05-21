let argonValues = {};
const serverURL = 'http://192.168.0.103:3000';
let fetchingLoop = setInterval(() => {
  fetch(serverURL+"/argonValues").then(response => response.json()).then(data => 
  {
    argonValues = data;
  });
}, 1000);

let updateUI = setInterval(() => {
  let hasValues = false;
  const parent = document.getElementById('values');
  parent.innerHTML = '';
  Object.entries(argonValues).forEach(([key,value])=> {
    const newSpan = document.createElement('span');
    newSpan.innerText = key + " : " + value + "\n";
    parent.appendChild(newSpan);
    hasValues = true;
  });
  if(hasValues){
    document.getElementById('init').innerText = '';
  } else {
    document.getElementById('init').innerText = 'No argon values found';
  }
}, 1000);

function initMap() {
  fetch(serverURL+"/argonLocation").then(response => response.json()).then(data => 
  {
    console.log("Location : " , data);
    if(Object.keys(data).includes('lat')){
      const loc = { lat: data['lat'], lng: data['lon'] };
      const map = new google.maps.Map(document.getElementById("map"), {
        zoom: 15,
        center: loc,
      });
      // The marker, positioned
      const marker = new google.maps.Marker({
        position: loc,
        map: map,
      });
    }
  });
}

let updateMap = setInterval(() => {
  initMap();
},1000*60*5); // Reload every 5 minutes