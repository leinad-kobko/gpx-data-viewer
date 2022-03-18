'use strict'; // C library API

function _slicedToArray(arr, i) { return _arrayWithHoles(arr) || _iterableToArrayLimit(arr, i) || _nonIterableRest(); }

function _nonIterableRest() { throw new TypeError("Invalid attempt to destructure non-iterable instance"); }

function _iterableToArrayLimit(arr, i) { if (!(Symbol.iterator in Object(arr) || Object.prototype.toString.call(arr) === "[object Arguments]")) { return; } var _arr = []; var _n = true; var _d = false; var _e = undefined; try { for (var _i = arr[Symbol.iterator](), _s; !(_n = (_s = _i.next()).done); _n = true) { _arr.push(_s.value); if (i && _arr.length === i) break; } } catch (err) { _d = true; _e = err; } finally { try { if (!_n && _i["return"] != null) _i["return"](); } finally { if (_d) throw _e; } } return _arr; }

function _arrayWithHoles(arr) { if (Array.isArray(arr)) return arr; }

function ownKeys(object, enumerableOnly) { var keys = Object.keys(object); if (Object.getOwnPropertySymbols) { var symbols = Object.getOwnPropertySymbols(object); if (enumerableOnly) symbols = symbols.filter(function (sym) { return Object.getOwnPropertyDescriptor(object, sym).enumerable; }); keys.push.apply(keys, symbols); } return keys; }

function _objectSpread(target) { for (var i = 1; i < arguments.length; i++) { var source = arguments[i] != null ? arguments[i] : {}; if (i % 2) { ownKeys(source, true).forEach(function (key) { _defineProperty(target, key, source[key]); }); } else if (Object.getOwnPropertyDescriptors) { Object.defineProperties(target, Object.getOwnPropertyDescriptors(source)); } else { ownKeys(source).forEach(function (key) { Object.defineProperty(target, key, Object.getOwnPropertyDescriptor(source, key)); }); } } return target; }

function _defineProperty(obj, key, value) { if (key in obj) { Object.defineProperty(obj, key, { value: value, enumerable: true, configurable: true, writable: true }); } else { obj[key] = value; } return obj; }

var ffi = require('ffi-napi'); // Express App (Routes)


var express = require("express");

var app = express();

var path = require("path");

var fileUpload = require('express-fileupload');

app.use(fileUpload());
app.use(express["static"](path.join(__dirname + '/uploads'))); // Minimization

var fs = require('fs');

var JavaScriptObfuscator = require('javascript-obfuscator'); // Important, pass in port as in `npm run dev 1234`, do not change


var portNum = process.argv[2]; // Send HTML at root, do not change

app.get('/', function (req, res) {
  res.sendFile(path.join(__dirname + '/public/index.html'));
}); // Send Style, do not change

app.get('/style.css', function (req, res) {
  //Feel free to change the contents of style.css to prettify your Web app
  res.sendFile(path.join(__dirname + '/public/style.css'));
}); // Send obfuscated JS, do not change

app.get('/index.js', function (req, res) {
  fs.readFile(path.join(__dirname + '/public/index.js'), 'utf8', function (err, contents) {
    var minimizedContents = JavaScriptObfuscator.obfuscate(contents, {
      compact: true,
      controlFlowFlattening: true
    });
    res.contentType('application/javascript');
    res.send(minimizedContents._obfuscatedCode);
  });
}); //Respond to POST requests that upload files to uploads/ directory

app.post('/upload', function (req, res) {
  if (!req.files) {
    return res.status(400).send('No files were uploaded.');
  }

  var uploadFile = req.files.uploadFile; // Use the mv() method to place the file somewhere on your server

  uploadFile.mv('uploads/' + uploadFile.name, function (err) {
    if (err) {
      return res.status(500).send(err);
    }

    res.redirect('/');
  });
}); //Respond to GET requests for files in the uploads/ directory

app.get('/uploads/:name', function (req, res) {
  fs.stat('uploads/' + req.params.name, function (err, stat) {
    if (err == null) {
      res.sendFile(path.join(__dirname + '/uploads/' + req.params.name));
    } else {
      console.log('Error in file downloading route: ' + err);
      res.send('');
    }
  });
}); //******************** Your code goes here ******************** 

var sharedLib = ffi.Library('./libgpxparser.so', {
  'fileToJSON': ['string', ['string']],
  'getRoutesComponent': ['string', ['string']],
  'getTracksComponent': ['string', ['string']],
  'renameComponent': ['void', ['string', 'string', 'string', 'string']],
  'validateFile': ['string', ['string', 'string']],
  'addRouteFromApp': ['void', ['string', 'string', 'string']],
  'createGPXFromApp': ['void', ['string', 'string']],
  'componentsBetween': ['string', ['string', 'string', 'string', 'string', 'string', 'string']],
  'getRoutesBetweenJSON': ['string', ['string', 'string', 'string', 'string', 'string', 'string', 'string']],
  'getTracksBetweenJSON': ['string', ['string', 'string', 'string', 'string', 'string', 'string', 'string']],
  'getPoints': ['string', ['string', 'string']]
});
app.get('/filelog', function (req, res) {
  files: fs.readdir(path.join(__dirname + "/uploads/"), function (err, files) {
    if (!files.length) {
      res.send({
        data: []
      });
    }

    res.send({
      data: files.filter(function (name) {
        return name.slice(-4) === ".gpx";
      }).filter(function (filename) {
        return sharedLib.validateFile("".concat(__dirname, "/uploads/").concat(filename), __dirname) === "true";
      }).map(function (gpxName) {
        var filename = "".concat(__dirname, "/uploads/").concat(gpxName);
        var gpx = JSON.parse(sharedLib.fileToJSON(filename));
        return _objectSpread({
          name: gpxName
        }, gpx);
      })
    });
  });
});
app.get('/dropdown', function (req, res) {
  if (req.query.filename === '') {
    res.send({
      data: {}
    });
  } else {
    var filename = "".concat(__dirname, "/uploads/").concat(req.query.filename);
    var routesJSON = sharedLib.getRoutesComponent(filename);
    var tracksJSON = sharedLib.getTracksComponent(filename);
    var components = {
      routes: routesJSON,
      tracks: tracksJSON
    };
    res.send({
      data: components
    });
  }
});
app.get('/rename', function (req, res) {
  var filename = "".concat(__dirname, "/uploads/").concat(req.query.filename);
  sharedLib.renameComponent(filename, req.query.type, req.query.id, req.query.newname);
  res.send({
    data: "Component rename successful."
  });
});
app.get('/createGPX', function (req, res) {
  var filename = "".concat(__dirname, "/uploads/").concat(req.query.filename);
  sharedLib.createGPXFromApp(filename, JSON.stringify({
    "version": new Number(req.query.version),
    "creator": req.query.creator
  }));
  res.send({
    data: filename
  });
});
app.get('/findBetween', function (req, res) {
  files: fs.readdir(path.join(__dirname + "/uploads/"), function (err, files) {
    if (!files.length) {
      res.send({
        data: []
      });
    } else {
      res.send({
        data: files.filter(function (name) {
          return name.slice(-4) === ".gpx";
        }).filter(function (filename) {
          return sharedLib.validateFile("".concat(__dirname, "/uploads/").concat(filename), __dirname) === "true";
        }).map(function (gpxName) {
          var filename = "".concat(__dirname, "/uploads/").concat(gpxName);
          var routesJSON = JSON.parse(sharedLib.getRoutesBetweenJSON(filename, gpxName, req.query.lon1, req.query.lat1, req.query.lon2, req.query.lat2, req.query.delta));
          var tracksJSON = JSON.parse(sharedLib.getTracksBetweenJSON(filename, gpxName, req.query.lon1, req.query.lat1, req.query.lon2, req.query.lat2, req.query.delta));
          return {
            routes: routesJSON,
            tracks: tracksJSON
          };
        })
      });
    }
  });
});
app.get('/addroute', function (req, res) {
  var addRouteFile = "".concat(__dirname, "/uploads/addingroute.gpx");
  var filename = "".concat(__dirname, "/uploads/").concat(req.query.filename);
  fs.rename(filename, addRouteFile, function () {
    console.log("Adding route to " + "".concat(req.query.filename));
  });
  sharedLib.addRouteFromApp(addRouteFile, filename, req.query.JSONstring);
  fs.unlink(addRouteFile, function (err) {
    if (err) {
      console.error(err);
    }
  });
  res.send({
    data: "File created successfully"
  });
}); //******************** A4 code ******************** 

var mysql = require('mysql2/promise');

var connection;
app.get('/login', function _callee(req, res) {
  return regeneratorRuntime.async(function _callee$(_context) {
    while (1) {
      switch (_context.prev = _context.next) {
        case 0:
          _context.prev = 0;
          console.log("connecting to db");
          _context.next = 4;
          return regeneratorRuntime.awrap(mysql.createConnection({
            host: 'dursley.socs.uoguelph.ca',
            user: req.query.username,
            password: req.query.password,
            database: req.query.dbname
          }));

        case 4:
          connection = _context.sent;
          _context.next = 7;
          return regeneratorRuntime.awrap(connection.execute("CREATE TABLE IF NOT EXISTS FILE " + "(gpx_id INT AUTO_INCREMENT, " + "file_name VARCHAR(60) NOT NULL, " + "ver DECIMAL(2,1) NOT NULL, " + "creator VARCHAR(256) NOT NULL, " + "PRIMARY KEY(gpx_id));"));

        case 7:
          _context.next = 9;
          return regeneratorRuntime.awrap(connection.execute("CREATE TABLE IF NOT EXISTS ROUTE " + "(route_id INT AUTO_INCREMENT, " + "route_name VARCHAR(256), " + "route_len FLOAT(15,7) NOT NULL, " + "gpx_id INT NOT NULL, " + "PRIMARY KEY(route_id), " + "FOREIGN KEY(gpx_id) REFERENCES FILE(gpx_id) ON DELETE CASCADE);"));

        case 9:
          _context.next = 11;
          return regeneratorRuntime.awrap(connection.execute("CREATE TABLE IF NOT EXISTS POINT " + "(point_id INT AUTO_INCREMENT, " + "point_index INT NOT NULL, " + "latitude DECIMAL(11,7) NOT NULL, " + "longitude DECIMAL(11,7), " + "point_name VARCHAR(256), " + "route_id INT NOT NULL, " + "PRIMARY KEY(point_id), " + "FOREIGN KEY(route_id) REFERENCES ROUTE(route_id) ON DELETE CASCADE);"));

        case 11:
          res.send({
            data: "success"
          });
          _context.next = 18;
          break;

        case 14:
          _context.prev = 14;
          _context.t0 = _context["catch"](0);
          console.log(_context.t0);
          res.send({
            data: _context.t0
          });

        case 18:
        case "end":
          return _context.stop();
      }
    }
  }, null, null, [[0, 14]]);
});
app.get('/storeall', function _callee6(req, res) {
  return regeneratorRuntime.async(function _callee6$(_context6) {
    while (1) {
      switch (_context6.prev = _context6.next) {
        case 0:
          try {
            files: fs.readdir(path.join(__dirname + "/uploads/"), function _callee5(err, files) {
              var gpxFiles, dbFiles;
              return regeneratorRuntime.async(function _callee5$(_context5) {
                while (1) {
                  switch (_context5.prev = _context5.next) {
                    case 0:
                      // store all gpx file JSONs into an array
                      gpxFiles = files.filter(function (name) {
                        return name.slice(-4) === ".gpx";
                      }).filter(function (filename) {
                        return sharedLib.validateFile("".concat(__dirname, "/uploads/").concat(filename), __dirname) === "true";
                      }).map(function (gpxName) {
                        var filename = "".concat(__dirname, "/uploads/").concat(gpxName);
                        var gpx = JSON.parse(sharedLib.fileToJSON(filename));
                        return _objectSpread({
                          path: filename,
                          name: gpxName
                        }, gpx);
                      });
                      _context5.next = 3;
                      return regeneratorRuntime.awrap(connection.execute("SELECT * FROM FILE"));

                    case 3:
                      dbFiles = _context5.sent;
                      gpxFiles.forEach(function _callee4(gpx) {
                        var gpxFileExists, gpxID, routes, routeidx;
                        return regeneratorRuntime.async(function _callee4$(_context4) {
                          while (1) {
                            switch (_context4.prev = _context4.next) {
                              case 0:
                                gpxFileExists = 0;
                                dbFiles.forEach(function (dbfile) {
                                  if (dbfile.file_name === gpx.name && dbfile.ver === gpx.version && dbfile.creator === gpx.creator) {
                                    gpxFileExists++;
                                  }
                                });

                                if (!(gpxFileExists === 0)) {
                                  _context4.next = 9;
                                  break;
                                }

                                _context4.next = 5;
                                return regeneratorRuntime.awrap(connection.execute("INSERT INTO FILE (file_name, ver, creator) VALUES (\"".concat(gpx.name, "\", ").concat(gpx.version, ", \"").concat(gpx.creator, "\");")));

                              case 5:
                                gpxID = _context4.sent;
                                routes = JSON.parse(sharedLib.getRoutesComponent(gpx.path));
                                routeidx = 0;
                                routes.forEach(function _callee3(route) {
                                  var routeID, points, pointidx;
                                  return regeneratorRuntime.async(function _callee3$(_context3) {
                                    while (1) {
                                      switch (_context3.prev = _context3.next) {
                                        case 0:
                                          _context3.next = 2;
                                          return regeneratorRuntime.awrap(connection.execute("INSERT INTO ROUTE (route_name, route_len, gpx_id) VALUES (\"".concat(route.route.name, "\", ").concat(route.route.len, ", ").concat(gpxID[0].insertId, ");")));

                                        case 2:
                                          routeID = _context3.sent;
                                          points = JSON.parse(sharedLib.getPoints(gpx.path, "".concat(routeidx++)));
                                          pointidx = 0;
                                          points.forEach(function _callee2(point) {
                                            return regeneratorRuntime.async(function _callee2$(_context2) {
                                              while (1) {
                                                switch (_context2.prev = _context2.next) {
                                                  case 0:
                                                    _context2.next = 2;
                                                    return regeneratorRuntime.awrap(connection.execute("INSERT INTO POINT (point_index, latitude, longitude, point_name, route_id) VALUES (".concat(pointidx++, ", ").concat(point.latitude, ", ").concat(point.longitude, ", \"").concat(point.name, "\", ").concat(routeID[0].insertId, ");")));

                                                  case 2:
                                                  case "end":
                                                    return _context2.stop();
                                                }
                                              }
                                            });
                                          });

                                        case 6:
                                        case "end":
                                          return _context3.stop();
                                      }
                                    }
                                  });
                                });

                              case 9:
                              case "end":
                                return _context4.stop();
                            }
                          }
                        });
                      });
                      res.send({
                        data: "success"
                      });

                    case 6:
                    case "end":
                      return _context5.stop();
                  }
                }
              });
            });
          } catch (e) {
            console.log(e);
            res.send({
              data: "fail"
            });
          }

        case 1:
        case "end":
          return _context6.stop();
      }
    }
  });
});
app.get('/displaystatus', function _callee7(req, res) {
  var _ref, _ref2, _ref2$, num_files, _ref3, _ref4, _ref4$, num_routes, _ref5, _ref6, _ref6$, num_points, output;

  return regeneratorRuntime.async(function _callee7$(_context7) {
    while (1) {
      switch (_context7.prev = _context7.next) {
        case 0:
          _context7.prev = 0;
          _context7.next = 3;
          return regeneratorRuntime.awrap(connection.execute("SELECT COUNT(*) FROM FILE;"));

        case 3:
          _ref = _context7.sent;
          _ref2 = _slicedToArray(_ref, 1);
          _ref2$ = _slicedToArray(_ref2[0], 1);
          num_files = _ref2$[0];
          _context7.next = 9;
          return regeneratorRuntime.awrap(connection.execute("SELECT COUNT(*) FROM ROUTE;"));

        case 9:
          _ref3 = _context7.sent;
          _ref4 = _slicedToArray(_ref3, 1);
          _ref4$ = _slicedToArray(_ref4[0], 1);
          num_routes = _ref4$[0];
          _context7.next = 15;
          return regeneratorRuntime.awrap(connection.execute("SELECT COUNT(*) FROM POINT;"));

        case 15:
          _ref5 = _context7.sent;
          _ref6 = _slicedToArray(_ref5, 1);
          _ref6$ = _slicedToArray(_ref6[0], 1);
          num_points = _ref6$[0];
          output = "Database has " + num_files["COUNT(*)"] + " files, " + num_routes["COUNT(*)"] + " routes and " + num_points["COUNT(*)"] + " points";
          res.send({
            data: output
          });
          _context7.next = 27;
          break;

        case 23:
          _context7.prev = 23;
          _context7.t0 = _context7["catch"](0);
          console.log(_context7.t0);
          res.send({
            data: "fail"
          });

        case 27:
        case "end":
          return _context7.stop();
      }
    }
  }, null, null, [[0, 23]]);
});
app.get('/cleartables', function _callee8(req, res) {
  return regeneratorRuntime.async(function _callee8$(_context8) {
    while (1) {
      switch (_context8.prev = _context8.next) {
        case 0:
          _context8.next = 2;
          return regeneratorRuntime.awrap(connection.execute("DELETE FROM FILE;"));

        case 2:
          _context8.next = 4;
          return regeneratorRuntime.awrap(connection.execute("DELETE FROM ROUTE;"));

        case 4:
          _context8.next = 6;
          return regeneratorRuntime.awrap(connection.execute("DELETE FROM POINT;"));

        case 6:
          res.send({
            data: "cleared"
          });

        case 7:
        case "end":
          return _context8.stop();
      }
    }
  });
});
app.listen(portNum);
console.log('Running app at localhost: ' + portNum);