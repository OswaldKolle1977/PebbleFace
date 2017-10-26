
// This will hold the battery manager object
var m_Battery;
// Simple var to save if the battery manager is available
var m_b_Battery_Available;
// Store the battery init status
var m_b_Battery_IsInit;

// debugging
var m_b_Debug;

function GetData()
{   
   // Did we init yet?
   if ((typeof m_b_Battery_IsInit === "undefined") || (m_b_Battery_IsInit === false))
   {
      if (m_b_Debug)
         console.log("[JS:BATT] Battery is not yet initialized!");
      return;
   }
   
   if (!m_b_Battery_Available)
   {
     console.log("[JS:BATT] m_b_Battery_Available FALSE!");
     return;
   }
     
   
   if (m_b_Debug)
         console.log("[JS:BATT] Getting battery data");
   
   var dictionary = 
       {
          'BATTERY_CHARGE': m_Battery.level * 100,
          'BATTERY_STATE': m_Battery.charging ? 1 : 0
       };

   // Send to Pebble
   Pebble.sendAppMessage(dictionary,
                         function(e)
                         {
                            if (m_b_Debug)
                               console.log("[JS:BATT] Sent message with ID " + e.data.transactionId);
                         },
                         function(e)
                         {
                            if (m_b_Debug)
                               console.log("[JS:BATT] Could not send message with ID " + e.data.transactionId + " Error is: " + e.error);
                         });
   
}

function InitSuccess(batteryManager) 
{
  // Assign batteryManager to globally 
  //   available `battery` variable.
  if (m_b_Debug)
    console.log("[JS:BATT] Init OK");
  m_Battery = batteryManager;
  m_b_Battery_Available = true;
  GetData();
}

function InitFailure()
{
   if (m_b_Debug)
         console.log("[JS:BATT] Init failed");
}

function Init(b_Debug)
{
   m_b_Debug = b_Debug;
   
   if (m_b_Debug)
         console.log("[JS:BATT] Init...");
   
   m_b_Battery_Available = false;
   
   
   if("getBattery" in navigator) 
   {
      // Request battery manager object.
      
      navigator.getBattery().then(function(battery) 
      {
        battery.addEventListener('chargingchange', function(){ updateChargeInfo();});
        function updateChargeInfo(){GetData();}
      
        battery.addEventListener('levelchange', function(){updateLevelInfo();});
        function updateLevelInfo(){GetData();}
      
        battery.addEventListener('chargingtimechange', function(){updateChargingInfo();});
        function updateChargingInfo(){GetData();}
      
        battery.addEventListener('dischargingtimechange', function(){updateDischargingInfo();});
        function updateDischargingInfo(){ GetData();} 
      });
      
      navigator.getBattery().then(InitSuccess, InitFailure);
   } 
   else 
   {
      // API is not supported, fail gracefully.
      if (m_b_Debug)
         console.log("[JS:BATT] Battery API not supported!");
   }
   
   m_b_Battery_IsInit = true;
   
}

module.exports.Init = Init;
module.exports.GetData = GetData;


///////
// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {
    // console.log('[JS:index.js]PebbleKit JS ready!');
    Init(false);
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    var b_Found = false;
    if (m_b_Debug)
    {
      console.log('[JS:index.js]AppMessage received!');
      console.log(e.payload);
      console.log('[JS:index.js] Das war die Payload...');
    }
    if ('BATTERY_CHARGE' in e.payload)
    {
      if (m_b_Debug)
        console.log("[JS:index.js] Battery info requested...");
      GetData();
      b_Found = true;
    }
    if (!b_Found && m_b_Debug)
               console.log("[JS:index.js] Unkown info requested..." +e.payload.toString());
  }                     
);

Pebble.addEventListener('showConfiguration', function() {
  var url = 'http://pebble.w-wer.eu/index.html';
  if (m_b_Debug)
    console.log('Showing configuration page: ' + url);
  
  Pebble.openURL(url);
});
///////

Pebble.addEventListener('webviewclosed', function(e) {
  var configData = JSON.parse(decodeURIComponent(e.response));
  console.log('Configuration page returned: ' + JSON.stringify(configData));

  // var backgroundColor = configData['background_color'];

  var dict = {};
  dict['VBC'] = configData['vib_bt_c'] ? 1 : 0;  // Send a boolean as an integer
  dict['VBD'] = configData['vib_bt_d'] ? 1 : 0;  // Send a boolean as an integer
  // dict['COLOR_RED'] = parseInt(backgroundColor.substring(2, 4), 16);
  // dict['COLOR_GREEN'] = parseInt(backgroundColor.substring(4, 6), 16);
  // dict['COLOR_BLUE'] = parseInt(backgroundColor.substring(6), 16);
  // dict['BG_COLOR'] = backgroundColor;
  dict['DATE_FORMAT'] = configData['selDate'];
  dict['LANG_SEL'] = configData['selLang'];
  dict['PHONE_BAT'] = configData['phoneBat'] ? 1 : 0;

  // Send to watchapp
  Pebble.sendAppMessage(dict, function() {
    console.log('Send successful: ' + JSON.stringify(dict));
  }, function() {
    console.log('Send failed!');
  });
});