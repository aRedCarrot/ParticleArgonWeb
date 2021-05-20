const express = require('express');
const app = express();
app.use(express.json());
const port = 3000;


app.get('/', (req, res) => {
  res.send('Hello World!')
  console.log("got request at /");
})

app.post('/json', (req, res) => {
  console.log("received req " , req.body);
  res.send(req.body);
});

app.listen(port, () => {
  console.log(`Example app listening at http://localhost:${port}`)
})