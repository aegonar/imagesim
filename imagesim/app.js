var express = require('express');
var app = express();
var path = require('path');
var formidable = require('formidable');
var fs = require('fs');
const spawn = require('child_process').spawn;

var redis = require('redis');
var redisClient = redis.createClient({host : 'localhost', port : 6379});

redisClient.on('ready',function() {
 console.log("Redis is ready");
});

redisClient.on('error',function(err) {
 console.log("Error in Redis");
console.log(err);
});

// Write data (remember to send only strings or numbers, otherwhise python wont understand)
var filename = "";


app.use(express.static(path.join(__dirname, 'public')));

app.get('/', function(req, res){
  res.sendFile(path.join(__dirname, 'views/index.html'));
});

app.post('/upload', function(req, res){

  // create an incoming form object
  var form = new formidable.IncomingForm();

  // specify that we want to allow the user to upload multiple files in a single request
  form.multiples = true;

  // store all uploads in the /uploads directory
  form.uploadDir = path.join(__dirname, '/uploads');

  // every time a file has been uploaded successfully,
  // rename it to it's orignal name

var filename;

  form.on('file', function(field, file) {
    fs.rename(file.path, path.join(form.uploadDir, file.name));
    
    console.log("filename".concat(JSON.stringify(file)));
filename=file.name;
    const scriptExecution = spawn('python', ["imagesimscript.py"]); //Hace spawn del script de phyton siempre que se hace un upload
    scriptExecution.stdout.on('data', (data) => {
        console.log(String.fromCharCode.apply(null, data));
    });
    scriptExecution.stdin.write(file.name); // Se le pasa el nombre del archivo al script de python 
    scriptExecution.stdin.end();

    redisClient.lrange(filename,0,1000,function(err,reply) {
     console.log(err);
     console.log(reply);
    res.send(reply);
    });

  });

  // log any errors that occur
  form.on('error', function(err) {
    console.log('An error has occured: \n' + err);
  });

  // once all the files have been uploaded, send a response to the client
  form.on('end', function() {
    //res.send('success');
  });

  // parse the incoming request containing the form data
  form.parse(req);
  


});

var server = app.listen(3000, function(){
  console.log('Server listening on port 3000');
});
