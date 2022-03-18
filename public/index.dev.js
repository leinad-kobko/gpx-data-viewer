"use strict";

var routeOtherDataList = [];
var trackOtherDataList = []; // Put all onload AJAX calls here, and event listeners

jQuery(document).ready(function () {
  $("#login-button").submit(function (e) {
    e.preventDefault();
    $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/login',
      data: {
        username: $("#un").val(),
        password: $("#pwd").val(),
        dbname: $("#dbn").val()
      },
      success: function success(_ref) {
        var data = _ref.data;

        if (data === "success") {
          $("#loginUIWrapper").toggleClass("invisible");
          $("#dataBaseUI").toggleClass("invisible");
        } else {
          alert("Login failed. Please check entries and try again");
        }
      },
      fail: function fail(err) {
        console.log(err);
      }
    });
  }); // Fill the File Log Panel

  populateFileLog(); // When the dropdown list of files has a changed selection

  $("#filedropdown").change(function (e) {
    if ($("#filedropdown").val() === "") {
      $("#gpxview").html("No File selected");
    } else {
      populateGPXView();
    }
  }); // When the Show other data button is clicked

  $('#otherdata-button').submit(function (e) {
    e.preventDefault();

    if ($("input[name='component']:checked").val() === undefined) {
      alert("No route was selected");
    } else {
      var component = $("input[name='component']:checked").attr('id');
      var index = new Number(component.slice((component.length - 5) * -1)) - 1;

      if (component.slice(0, 5) === "Track") {
        var dataString = "";

        if (trackOtherDataList[index].length === 0) {
          dataString += "No data to be shown for Track " + (index + 1);
        } else {
          dataString += "Other Data for Track " + (index + 1) + ":\n";
          trackOtherDataList[index].forEach(function (e) {
            var data = e.name + ": " + e.value + "\n";
            dataString += data;
          });
        }

        alert(dataString);
      } else if (component.slice(0, 5) === "Route") {
        var _dataString = "";

        if (routeOtherDataList[index].length === 0) {
          _dataString += "No data to be shown for Route " + (index + 1);
        } else {
          _dataString += "Other Data for Route " + (index + 1) + ":\n";
          routeOtherDataList[index].forEach(function (e) {
            var data = e.name + ": " + e.value + "\n";
            _dataString += data;
          });
        }

        alert(_dataString);
      }
    }
  }); // Rename

  $('#rename-button').submit(function (e) {
    e.preventDefault();
    length = $("input[name='component']:checked").attr("id").length;

    if ($("input[name='component']:checked").val() === undefined) {
      alert("No route was selected for renaming.");
    } else {
      $.ajax({
        type: 'get',
        dataType: 'json',
        url: '/rename',
        data: {
          filename: $("#filedropdown").val(),
          type: $("input[name='component']:checked").attr("id").slice(0, 5),
          id: $("input[name='component']:checked").attr("id").slice((length - 5) * -1, length),
          newname: $("#entryBox").val()
        },
        success: function success(_ref2) {
          var data = _ref2.data;
          $("#creationSuccess").html("File successfully created.");
        },
        fail: function fail(err) {
          console.log(err);
        }
      });
      populateGPXView();
    }

    $("#entryBox").val("");
  }); // Add Route

  $('#newroute-button').submit(function (e) {
    e.preventDefault();
    $('#newRoute').html(addRouteHTML());
    $('#addPt-button').submit(function (e) {
      e.preventDefault();
      addPoint();
    });
    $('#undo-button').submit(function (e) {
      e.preventDefault();
      popPoint();
    });
    $('#addRt-button').submit(function (e) {
      e.preventDefault();
      addRoute();
    });
  }); // Create GPX

  $("#addgpx-button").submit(function (e) {
    e.preventDefault();
    $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/createGPX',
      data: {
        creator: $("#Creator").val(),
        version: $("#Version").val(),
        filename: $("#newGPXFile").val()
      },
      success: function success(_ref3) {
        var data = _ref3.data;
        populateFileLog();
        populateGPXView();
        clearAll();
        storeAll();
        alert("File successfully created.");
      },
      fail: function fail(err) {
        console.log(err);
      }
    });
  }); // Find Routes Between

  $("#between-button").submit(function (e) {
    e.preventDefault();
    $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/findBetween',
      data: {
        lon1: $("#longitude1").val(),
        lat1: $("#latitude1").val(),
        lon2: $("#longitude2").val(),
        lat2: $("#latitude2").val(),
        delta: $("#delta").val()
      },
      success: function success(_ref4) {
        var data = _ref4.data;
        $("#componentsBetween").html("");
        var num = 1;
        data.forEach(function (file) {
          file.routes.routes.forEach(function (route) {
            $("#componentsBetween").append("<div>" + "<b>File</b>: " + file.routes.name + " Route " + num++ + "- <b>Name: </b>" + route.name + ", <b># of Points: </b>" + route.numPoints + ", <b>Length: </b>" + route.len + "m" + ", <b>Loop: </b>" + route.loop + "</div>");
          });
        });
        num = 1;
        data.forEach(function (file) {
          file.tracks.tracks.forEach(function (track) {
            $("#componentsBetween").append("<div>" + "<b>File</b>: " + file.tracks.name + " Route " + num++ + "- <b>Name: </b>" + track.name + ", <b># of Points: </b>" + track.numPoints + ", <b>Length: </b>" + track.len + "m" + ", <b>Loop: </b>" + track.loop + "</div>");
          });
        });
      },
      fail: function fail(err) {
        console.log(err);
      }
    });
  });
  $("#store-all-button").submit(function (e) {
    e.preventDefault();
    clearAll();
    storeAll();
  });
  $("#clear-all-button").submit(function (e) {
    e.preventDefault();
    clearAll();
    displayStatus();
  });
  $("#display-status-button").submit(function (e) {
    e.preventDefault();
    displayStatus();
  });
  $("#execute-query-button").submit(function (e) {
    e.preventDefault();
  });
});

function populateFileLog() {
  jQuery.ajax({
    type: 'get',
    dataType: 'json',
    url: '/filelog',
    success: function success(_ref5) {
      var data = _ref5.data;

      if (data.length === 0) {
        $('#filelog').html("<p>No Files.</p>");
        $('#filedropdown').html("");
      } else {
        $('#filelog').html("<tr>" + "<th>File name (click to download)</th>" + "<th>Version</th>" + "<th>Creator</th>" + "<th>Number of Waypoints</th>" + "<th>Number of Routes</th>" + "<th>Number of tracks</th>" + "</tr>");
        $('#filedropdown').html("<option value=\"\"></option>");
        data.forEach(function (gpx) {
          $('#filedropdown').append("<option value=\"" + gpx.name + "\">" + gpx.name.slice(0, -4) + "</option>");
          $('#filelog').append("<tr>" + "<td>" + "<a href=\"" + gpx.name + "\" download \">" + gpx.name + "</a>" + "</td>" + "<td>" + gpx.version + "</td>" + "<td>" + gpx.creator + "</td>" + "<td>" + gpx.numWaypoints + "</td>" + "<td>" + gpx.numRoutes + "</td>" + "<td>" + gpx.numTracks + "</td>" + "</tr>");
        });

        if ($("#store-all-button").hasClass("invisible")) {
          $("#store-all-button").toggleClass("invisible");
        }
      }
    },
    fail: function fail(error) {
      console.log(error);
    }
  });
}

function populateGPXView() {
  $.ajax({
    type: 'get',
    dataType: 'json',
    url: '/dropdown',
    data: {
      filename: $("#filedropdown").val()
    },
    success: function success(_ref6) {
      var data = _ref6.data;

      if (JSON.stringify(data) != "{}") {
        var routes = JSON.parse(data.routes);
        var tracks = JSON.parse(data.tracks);
        $("#gpxview").html("");
        var num = 1;
        routes.forEach(function (route) {
          routeOtherDataList[num - 1] = route.otherData;
          $("#gpxview").append("<div>" + "<input type=\"radio\" name=\"component\" id=\"Route" + num + "\" value=\"" + route.route.name + "\">" + "  <b>Route " + num++ + "</b>" + "- <b>Name: </b>" + route.route.name + ", <b># of Points: </b>" + route.route.numPoints + ", <b>Length: </b>" + route.route.len + "m" + ", <b>Loop: </b>" + route.route.loop + "</div>");
        });
        num = 1;
        tracks.forEach(function (track) {
          trackOtherDataList[num - 1] = track.otherData;
          $("#gpxview").append("<div>" + "<input type=\"radio\" name=\"component\" id=\"Track" + num + "\" value=\"" + track.track.name + "\">" + "  <b>Track " + num++ + "</b>" + " - <b>Name: </b>" + track.track.name + ", <b># of Points: </b>" + track.track.numPoints + ", <b>Length: </b>" + track.track.len + "m" + ", <b>Loop: </b>" + track.track.loop + "</div>");
        });
      } else {
        $("#gpxview").html("");
      }
    },
    fail: function fail(error) {
      console.log(error);
    }
  });
}

function addRouteHTML() {
  return "<p>Adding route to " + $("#filedropdown").val() + "</p>" + "<p id='ptsJSON'>{\"name\":\"\",\"longitudes\":[],\"latitudes\":[]}</p>" + "<form ref='addPt-button' id='addPt-button'>" + "<div class=\"form-group\">" + "<input type=\"text\" id=\"routeName\"class=\"form-control\" value=\"\" placeholder=\"Route Name ...\">" + "<input type=\"text\" id=\"newRouteLon\"class=\"form-control\" value=\"\" placeholder=\"Longitude ...\">" + "<input type=\"text\" id=\"newRouteLat\"class=\"form-control\" value=\"\" placeholder=\"Latitude ...\">" + "</div>" + "<div class=\"form-group\">" + "<input type='submit' class=\"btn btn-secondary\" value=\"+ Add Point\">" + "</div>" + "</form>" + "<form ref='undo-button' id='undo-button'>" + "<div class=\"form-group\">" + "<input type='submit' class=\"btn btn-secondary\" value=\"- Undo\">" + "</div>" + "</form>" + "<form ref='addRt-button' id='addRt-button'>" + "<div class=\"form-group\">" + "<input type='submit' class=\"btn btn-secondary\" value=\"+ Add Route\">" + "</div>" + "</form>" + "<hr>";
}

function addPoint() {
  if (!isNaN($("#newRouteLon").val()) && !isNaN($("#newRouteLat").val())) {
    var points = JSON.parse($("#ptsJSON").text());
    var _length = points.longitudes.length;
    points.longitudes[_length] = new Number($("#newRouteLon").val());
    points.latitudes[_length] = new Number($("#newRouteLat").val());
    $("#ptsJSON").html(JSON.stringify(points));
  } else {
    alert("Values entered were not all valid");
  }
}

function popPoint() {
  var points = JSON.parse($("#ptsJSON").text());

  if (points.latitudes.length != 0) {
    points.latitudes.pop();
    points.longitudes.pop();
  }

  $("#ptsJSON").html(JSON.stringify(points));
}

function addRoute() {
  if ($("#filedropdown").val() === "") {
    alert("No file selected");
  } else {
    var ptsJSON = JSON.parse($("#ptsJSON").text());
    ptsJSON.name = $("#routeName").val();
    $("#ptsJSON").html(JSON.stringify(ptsJSON));
    $.ajax({
      type: 'get',
      dataType: 'json',
      url: '/addroute',
      data: {
        filename: $("#filedropdown").val(),
        JSONstring: $("#ptsJSON").text()
      },
      success: function success(_ref7) {
        var data = _ref7.data;
        populateFileLog();
        populateGPXView();
        clearAll();
        storeAll();
      },
      fail: function fail(error) {
        console.log(error);
      }
    });
  }
}

function displayStatus() {
  $.ajax({
    type: 'get',
    dataType: 'json',
    url: '/displaystatus',
    success: function success(_ref8) {
      var data = _ref8.data;

      if (data === "fail") {
        alert("Something went wrong when displaying database status");
      } else {
        alert(data);
      }
    },
    fail: function fail(err) {
      console.log(err);
    }
  });
}

function clearAll() {
  $.ajax({
    type: 'get',
    dataType: 'json',
    url: '/cleartables',
    success: function success(_ref9) {
      var data = _ref9.data;
    },
    fail: function fail(err) {
      console.log(err);
    }
  });
}

function storeAll() {
  $.ajax({
    type: 'get',
    dataType: 'json',
    url: '/storeall',
    success: function success(_ref10) {
      var data = _ref10.data;

      if (data === "success") {
        displayStatus();
      } else {
        alert("Something went wrong when adding files to the database");
      }
    },
    fail: function fail(err) {
      console.log(err);
    }
  });
}