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