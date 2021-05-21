const express = require('express');
const path = require('path');
const app = express();
var cors = require('cors')

app.use(express.json());
app.use(express.static('public'))
app.use(cors());
const port = 3000;

let argonValues = {};

app.get('/', function(req, res) {
  res.sendFile(path.join(__dirname, '/public/index.html'));
});

app.get('/argonValues', (req,res) => {
  res.send(argonValues);
});

app.post('/json', (req, res) => {
  console.log("received req " , req.body);
  argonValues = { ...argonValues, ...req.body };
  res.send(req.body);
});

app.listen(port, () => {
  console.log(`Example app listening at http://localhost:${port}`)
})