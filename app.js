'use strict'

// C library API
const ffi = require('ffi-napi');

// Express App (Routes)
const express = require("express");
const app     = express();
const path    = require("path");
const fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express.static(path.join(__dirname+'/uploads')));

// Minimization
const fs = require('fs');
const JavaScriptObfuscator = require('javascript-obfuscator');

// Important, pass in port as in `npm run dev 1234`, do not change
const portNum = process.argv[2];

// Send HTML at root, do not change
app.get('/',function(req,res){
  res.sendFile(path.join(__dirname+'/public/index.html'));
});

// Send Style, do not change
app.get('/style.css',function(req,res){
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname+'/public/style.css'));
});

// Send obfuscated JS, do not change
app.get('/index.js',function(req,res){
  fs.readFile(path.join(__dirname+'/public/index.js'), 'utf8', function(err, contents) {
    const minimizedContents = JavaScriptObfuscator.obfuscate(contents, {compact: true, controlFlowFlattening: true});
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
});

//Respond to POST requests that upload files to uploads/ directory
app.post('/upload', function(req, res) {
  if(!req.files) {
    return res.status(400).send('No files were uploaded.');
  }
 
  let uploadFile = req.files.uploadFile;
 
  // Use the mv() method to place the file somewhere on your server
  uploadFile.mv('uploads/' + uploadFile.name, function(err) {
    if(err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
});

//Respond to GET requests for files in the uploads/ directory
app.get('/uploads/:name', function(req , res){
  fs.stat('uploads/' + req.params.name, function(err, stat) {
    if(err == null) {
      res.sendFile(path.join(__dirname+'/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: '+err);
      res.send('');
    }
  });
});

//******************** Your code goes here ******************** 

let sharedLib = ffi.Library('./libgpxparser.so', {
  'fileToJSON': ['string', ['string']],
  'getRoutesComponent': ['string', ['string']],
  'getTracksComponent': ['string', ['string']],
  'renameComponent': ['void', ['string','string','string','string']],
  'validateFile': ['string',['string','string']],
  'addRouteFromApp': ['void',['string','string','string']],
  'createGPXFromApp': ['void',['string','string']],
  'componentsBetween': ['string', ['string','string','string','string','string','string']],
  'getRoutesBetweenJSON': ['string', ['string','string','string','string','string','string','string']],
  'getTracksBetweenJSON': ['string', ['string','string','string','string','string','string','string']],
  'getPoints':['string',['string','string']]
});

app.get('/filelog', function(req , res){
  files: fs.readdir(path.join(__dirname + "/uploads/"), (err, files) => {
    if(!files.length){
      res.send({
        data: []
      });
    }

    res.send({
      data: files.filter(name => name.slice(-4) === ".gpx").filter(filename => sharedLib.validateFile(`${__dirname}/uploads/${filename}`,__dirname) === "true").map(gpxName => {
        let filename = `${__dirname}/uploads/${gpxName}`;
        let gpx = JSON.parse(sharedLib.fileToJSON(filename));
        return ({
          name:gpxName,
          ...gpx
        });
      })
    });
  });
});

app.get('/dropdown', function(req , res){
  if(req.query.filename === ''){
    res.send({
      data:{}
    });
  }else{
    let filename = `${__dirname}/uploads/${req.query.filename}`;
    let routesJSON = sharedLib.getRoutesComponent(filename);
    let tracksJSON = sharedLib.getTracksComponent(filename);
    let components = {routes:routesJSON,tracks:tracksJSON};
    res.send({
      data:components
    });
  }
});

app.get('/rename', function(req , res){

  let filename = `${__dirname}/uploads/${req.query.filename}`;
  sharedLib.renameComponent(filename, req.query.type, req.query.id, req.query.newname);

  res.send({
    data:"Component rename successful."
  });
});

app.get('/createGPX', function(req , res){
  let filename = `${__dirname}/uploads/${req.query.filename}`;
  sharedLib.createGPXFromApp(filename, JSON.stringify({"version":(new Number(req.query.version)),"creator":req.query.creator}));
  res.send({
    data:filename
  });
});

app.get('/findBetween', function(req , res){
  files: fs.readdir(path.join(__dirname + "/uploads/"), (err, files) => {
    if(!files.length){
      res.send({
        data: []
      });
    }else{
      res.send({
        data: files.filter(name => name.slice(-4) === ".gpx").filter(filename => sharedLib.validateFile(`${__dirname}/uploads/${filename}`,__dirname) === "true").map(gpxName => {
          let filename = `${__dirname}/uploads/${gpxName}`;
          let routesJSON = JSON.parse(sharedLib.getRoutesBetweenJSON(filename, gpxName, req.query.lon1, req.query.lat1, req.query.lon2, req.query.lat2, req.query.delta));
          let tracksJSON = JSON.parse(sharedLib.getTracksBetweenJSON(filename, gpxName, req.query.lon1, req.query.lat1, req.query.lon2, req.query.lat2, req.query.delta));
          return ({
            routes:routesJSON,
            tracks:tracksJSON
          });
        })
      });
    }
  });
});

app.get('/addroute', function(req , res){
  let addRouteFile = `${__dirname}/uploads/addingroute.gpx`;
  let filename = `${__dirname}/uploads/${req.query.filename}`;

  fs.rename(filename, addRouteFile, () => {
    console.log("Adding route to " + `${req.query.filename}`);
  });
  sharedLib.addRouteFromApp(addRouteFile, filename, req.query.JSONstring);
  fs.unlink(addRouteFile, (err) => {
    if (err) {
      console.error(err);
    }
  });
  res.send({
    data:"File created successfully"
  });
});

//******************** A4 code ******************** 

const mysql = require('mysql2/promise');
let connection;
app.get('/login', async function(req , res){
  try{ 
    console.log("connecting to db");
    connection = await mysql.createConnection({
      host: 'dursley.socs.uoguelph.ca',
      user: req.query.username,
      password: req.query.password,
      database: req.query.dbname
    });

    await connection.execute("CREATE TABLE IF NOT EXISTS FILE " +
                       "(gpx_id INT AUTO_INCREMENT, " +
                       "file_name VARCHAR(60) NOT NULL, " +
                       "ver DECIMAL(2,1) NOT NULL, " +
                       "creator VARCHAR(256) NOT NULL, " +
                       "PRIMARY KEY(gpx_id));"
                       );
    await connection.execute("CREATE TABLE IF NOT EXISTS ROUTE "+
                       "(route_id INT AUTO_INCREMENT, "+
                       "route_name VARCHAR(256), "+
                       "route_len FLOAT(15,7) NOT NULL, "+
                       "gpx_id INT NOT NULL, "+
                       "PRIMARY KEY(route_id), "+
                       "FOREIGN KEY(gpx_id) REFERENCES FILE(gpx_id) ON DELETE CASCADE);"
                       );
    await connection.execute("CREATE TABLE IF NOT EXISTS POINT "+
                       "(point_id INT AUTO_INCREMENT, "+
                       "point_index INT NOT NULL, "+
                       "latitude DECIMAL(11,7) NOT NULL, "+
                       "longitude DECIMAL(11,7), "+
                       "point_name VARCHAR(256), "+
                       "route_id INT NOT NULL, "+
                       "PRIMARY KEY(point_id), "+
                       "FOREIGN KEY(route_id) REFERENCES ROUTE(route_id) ON DELETE CASCADE);"
                       );

    res.send({
      data:"success"
    });
  }catch(e){
    console.log(e);
    res.send({
      data:e
    });
  }
});

app.get('/storeall', async function(req , res){
  try{
    files: fs.readdir(path.join(__dirname + "/uploads/"), async (err, files) => {
      // store all gpx file JSONs into an array
      let gpxFiles = files.filter(name => name.slice(-4) === ".gpx").filter(filename => sharedLib.validateFile(`${__dirname}/uploads/${filename}`,__dirname) === "true").map(gpxName => {
        let filename = `${__dirname}/uploads/${gpxName}`;
        let gpx = JSON.parse(sharedLib.fileToJSON(filename));
        return ({
          path:filename,
          name:gpxName,
          ...gpx
        });
      });
  
      let dbFiles = await connection.execute("SELECT * FROM FILE");

      gpxFiles.forEach(async (gpx) =>{
        let gpxFileExists = 0;
        dbFiles.forEach((dbfile) => {
          if(
            dbfile.file_name === gpx.name &&
            dbfile.ver === gpx.version &&
            dbfile.creator === gpx.creator
          ) 
          {
            gpxFileExists++;
          }          
        });
        

        if(gpxFileExists === 0){
          let gpxID = await connection.execute(`INSERT INTO FILE (file_name, ver, creator) VALUES ("${gpx.name}", ${gpx.version}, "${gpx.creator}");`);
          let routes = JSON.parse(sharedLib.getRoutesComponent(gpx.path));
          let routeidx = 0;
          routes.forEach(async (route) => {
            let routeID = await connection.execute(`INSERT INTO ROUTE (route_name, route_len, gpx_id) VALUES ("${route.route.name}", ${route.route.len}, ${gpxID[0].insertId});`);
            let points = JSON.parse(sharedLib.getPoints(gpx.path, `${routeidx++}`));
            let pointidx = 0;
            points.forEach(async (point) => {
              await connection.execute(`INSERT INTO POINT (point_index, latitude, longitude, point_name, route_id) VALUES (${pointidx++}, ${point.latitude}, ${point.longitude}, "${point.name}", ${routeID[0].insertId});`);            
            });
          });
        }        
      });

      res.send({
        data:"success"
      });
    });
  }catch(e){
    console.log(e);
    res.send({
      data:"fail"
    });
  }
});

app.get('/displaystatus', async function(req , res){
  try{
    let [[num_files]] = await connection.execute("SELECT COUNT(*) FROM FILE;");
    let [[num_routes]] = await connection.execute("SELECT COUNT(*) FROM ROUTE;");
    let [[num_points]] = await connection.execute("SELECT COUNT(*) FROM POINT;");

    let output = "Database has " +
                 num_files["COUNT(*)"] + " files, " +
                 num_routes["COUNT(*)"] + " routes and " +
                 num_points["COUNT(*)"] + " points";
    res.send({
      data: output
    });
  }catch(e){
    console.log(e);
    res.send({
      data:"fail"
    });
  }
});

app.get('/cleartables', async function(req , res){
  await connection.execute("DELETE FROM FILE;");
  await connection.execute("DELETE FROM ROUTE;");
  await connection.execute("DELETE FROM POINT;");
  res.send({
    data:"cleared"
  });
});

app.listen(portNum);
console.log('Running app at localhost: ' + portNum);