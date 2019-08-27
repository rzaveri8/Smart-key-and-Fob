//modules
var Engine = require('tingodb')();
var assert = require('assert');
var express = require('express');
var app = require('express')();
var http = require('http').Server(app);
var io = require('socket.io')(http);

var db = new Engine.Db('./', {});

var Iconv  = require('iconv').Iconv,
    fs = require("fs");

var buffer = fs.readFileSync("smoke.txt"),
    iconv = new Iconv( "UTF-16", "UTF-8");

//var array = iconv.convert(buffer).toString("utf8").split("\n");
var fob_data ;
var data;
var data_for_database = [];
var smoke;
const net = require('net');
var collection = db.collection("smoke_collection");
const server = net.createServer((socket) => {
  console.log('\n\n\---client connected---');
  socket.on('data',(data)=>{
      console.log("Msg from client :"+data.toString());
      fob_data = data.toString();
      data_for_database = fob_data.split("\t");
  });
console.log(data_for_database[2]);
  if(data_for_database[2]=='5039'){
    socket.end('1\n'); // writes the given data and sends FIN packet
    smoke = 1;
  }
  else {
    socket.end('0\n');
    smoke = 0;
  }




  collection.insert([{fob_id:parseFloat(data_for_database[0]),hub_id:data_for_database[1],smoke:smoke}], function(err,result){
       assert.equal(null,err);
       collection.find().toArray(function(err,item){
         assert.equal(null, err);
         data = item;
         console.log(data_for_database[1]);
         console.log(smoke);
       })
 })
}).on('error', (err) => {
  console.log("err:"+err);
  throw err;
});

console.log(smoke);


server.listen({
  host: '0.0.0.0',
  port: 3010,
  exclusive: true
},() => {
  console.log('TCP server started');
});


// var array = fs.readFileSync('smoke.txt','utf8').toString().split("\n");
// // Fetch a collection to insert document into

//  }

// Points to index.html to serve webpage
app.get('/', function(req, res){
  res.sendFile(__dirname + '/index.html');
  //Points to stocks.txt
  app.get('/login.html', function(req, res) {
    res.sendFile(__dirname + '/login.html');
  });
});

// var collection = db.collection("smoke_collection");
//
//
//   collection.insert([{fob_id:1,hub_id:2,smoke:0}], function(err,result){
//        assert.equal(null,err);
//  })
//  collection.insert([{fob_id:2,hub_id:2,smoke:1}], function(err,result){
//       assert.equal(null,err);
//  })
//  collection.find().toArray(function(err,item){
//    assert.equal(null, err);
//    data = item;
//  })
// User socket connection
io.on('connection', function(socket){
  console.log('a user connected');
  //console.log(data[0]);
  io.emit('data_transmit', data);
  socket.on('disconnect', function(){
    console.log('user disconnected');
  });
});

// Listening on localhost:3000
http.listen(3000, function() {
  console.log('listening on *:3000');
});
