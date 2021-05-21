const express = require('express');
const fs = require('fs');
const path = require('path');
const app = express();
var cors = require('cors')

app.use(express.json());
app.use(express.static('public'))
app.use(cors());
const port = 3000;

let argonValues = {};
let currentLocation = {};

app.get('/', function(req, res) {
  res.sendFile(path.join(__dirname, '/public/index.html'));
});

app.get('/argonValues', (req,res) => {
  res.send(argonValues);
});

app.get('/argonLocation', (req,res) => {
  res.send(currentLocation);
});

app.post('/json', (req, res) => {
  console.log("received req " , req.body);
  argonValues = { ...argonValues, ...req.body };
  res.send(req.body);
});

app.post('/location', (req, res) => {
  console.log("received new location" , req.body);
  currentLocation = {...req.body};
});

app.listen(port, () => {
  console.log(`Example app listening at http://localhost:${port}`)
})

const writeFileInterval = setInterval(() => {
  try{
    if(Object.keys(argonValues).length === 0) {
      return;
    }
    const ts = getCurrentTime();
    fs.appendFileSync(__dirname + '/logs/argonValues.json',JSON.stringify({...argonValues, timestamp : ts}) + ",\n");
  } catch (err) {
    console.error(err);
  }
},1000);

const readLogFile = () => {
  try {
    return fs.readFileSync(__dirname + '/logs/argonValues.json', 'utf8')
  } catch (err) {
    console.error(err)
    return false
  }
}

const getCurrentTime = () => {
  let date_ob = new Date();
  // current date
  // adjust 0 before single digit date
  let date = ("0" + date_ob.getDate()).slice(-2);
  // current month
  let month = ("0" + (date_ob.getMonth() + 1)).slice(-2);
  // current year
  let year = date_ob.getFullYear();
  // current hours
  let hours = date_ob.getHours();
  // current minutes
  let minutes = date_ob.getMinutes();
  // current seconds
  let seconds = date_ob.getSeconds();
  // prints date in YYYY-MM-DD format
  //console.log(year + "-" + month + "-" + date);
  // prints date & time in YYYY-MM-DD HH:MM:SS format
  return year + "-" + month + "-" + date + " " + hours + ":" + minutes + ":" + seconds;
}

// Check if the log file is empty or not
let fileContent = readLogFile();
if(fileContent.length === 0){
  fs.appendFileSync(__dirname + '/logs/argonValues.json',"[");
} else {
  if(fileContent.length !== 0){
    fileContent = fileContent.slice(0,-1);
    fileContent += ',\n';
    try{
      fs.writeFileSync(__dirname + '/logs/argonValues.json',fileContent);
    } catch (err) {
      console.error(err);
    }
  }
}

// catching signals and do something before exit
['SIGHUP', 'SIGINT', 'SIGQUIT', 'SIGILL', 'SIGTRAP', 'SIGABRT',
  'SIGBUS', 'SIGFPE', 'SIGUSR1', 'SIGSEGV', 'SIGUSR2', 'SIGTERM'
].forEach(function (signal) {
  process.on(signal, function () {
      terminator(signal);
      //console.log('signal: ' + sig);
  });
});

function terminator(signal) {
  if (typeof signal === "string") {
      // call your async task here and then call process.exit() after async task is done
      let fileContent = readLogFile();
      if(fileContent.length !== 0 && fileContent !== '['){
        fileContent = fileContent.slice(0,-2);
        fileContent += ']';
        try{
          fs.writeFileSync(__dirname + '/logs/argonValues.json',fileContent);
        } catch (err) {
          console.error(err);
        }
      }
      if(fileContent === '['){
        try{
          fs.writeFileSync(__dirname + '/logs/argonValues.json','');
        } catch (err) {
          console.error(err);
        }
      }
      process.exit(1);
  }
  //console.log('Node server stopped.');
}