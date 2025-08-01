  var ss = SpreadsheetApp.openById('yoursheetid');  // Replace with your Sheet ID
var sheet = ss.getSheetByName('Sheet1');
var summarySheet = ss.getSheetByName('Summary') || ss.insertSheet('Summary');
var timezone = "Asia/Kolkata";

function doGet(e) {
  if (!e || !e.parameter || !e.parameter.name) {
    return ContentService.createTextOutput("Received data is undefined");
  }
 var name;
  var id = stripQuotes(e.parameter.name); 
  if(id ==1){
    name = "Mathew";
  }else if (id == 2){
    name = "Thambu";
  }else if (id == 3){
    name = "Dhanush";
  }else if(id==4){
    name = "Fayaz";
  }else if(id == 5){
    name = "Navaneetha";
  }else if (id == 11){
    name = "Andrew";
  }
  var currDate = new Date();
  var currDateString = Utilities.formatDate(currDate, timezone, 'yyyy-MM-dd');
  var currTime = Utilities.formatDate(currDate, timezone, 'HH:mm:ss');
  var nextRow = sheet.getLastRow() + 1;

  // Log all scans
  sheet.getRange("A" + nextRow).setValue(name);
  sheet.getRange("B" + nextRow).setValue(currDateString);
  sheet.getRange("C" + nextRow).setValue(currTime);

  // Determine IN or OUT
  var status = updateSummary(name, currDateString, currTime);

  return ContentService.createTextOutput("status").setMimeType(ContentService.MimeType.TEXT);
}

function stripQuotes(value) {
  return value ? value.toString().replace(/^['"]|['"]$/g, "") : "";
}

function updateSummary(name, date, time) {
  var lastRow = summarySheet.getLastRow();
  var nameCol = 1;
  var dateCol = 2;

  if (lastRow === 0) {
    summarySheet.appendRow(["Name", "Date", "Start Time", "End Time", "Duration"]);
  }

  var lastEntryRow = null;
  var lastStatus = "";

  for (var i = lastRow; i >= 2; i--) {  // Start from the last entry
    var existingName = summarySheet.getRange(i, nameCol).getValue().toString();
    var existingDate = Utilities.formatDate(summarySheet.getRange(i, dateCol).getValue(), timezone, 'yyyy-MM-dd');
    
    if (existingName === name && existingDate === date) {
      lastEntryRow = i;
      //lastStatus = summarySheet.getRange(i, 4).getValue() ? "OUT" : "IN";
      break;
    }
  }
  // return "IN"
  if (lastEntryRow) {  
    if (lastStatus === "IN") {
      // Set OUT time and calculate duration
      summarySheet.getRange(lastEntryRow, 4).setValue(time);
      var startTime = summarySheet.getRange(lastEntryRow, 3).getValue();
      //var duration = calculateDuration(startTime, time);
      //summarySheet.getRange(lastEntryRow, 5).setValue(duration);
      lastStatus = "OUT";
      return "OUT";
    }else if (lastStatus === "OUT"){
      lastStatus ="IN"
      return "IN";
    }
  }

  // If no previous entry or last status was OUT, add new IN entry
  summarySheet.appendRow([name, date, time, "", ""]);
  return "IN";
}


function calculateDuration(startTime, endTime) {
  var startParts = startTime.split(":").map(Number);
  var endParts = endTime.split(":").map(Number);

  var startDate = new Date(1970, 0, 1, startParts[0], startParts[1], startParts[2]);
  var endDate = new Date(1970, 0, 1, endParts[0], endParts[1], endParts[2]);

  if (endDate < startDate) {
    endDate.setDate(endDate.getDate() + 1);  // Handle cases where end time is after midnight
  }

  var durationMs = endDate - startDate;
  var hours = Math.floor(durationMs / (1000 * 60 * 60));
  var minutes = Math.floor((durationMs % (1000 * 60 * 60)) / (1000 * 60));
  var seconds = Math.floor((durationMs % (1000 * 60)) / 1000);

  return hours + "h " + minutes + "m " + seconds + "s";
}

